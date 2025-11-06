import sqlite3
import re
import json
import random
import hashlib
import os

import pandas as pd
import altair as alt

from django.shortcuts import render
from django.http import JsonResponse, HttpResponse
from django.template.loader import render_to_string
from django.core.cache import cache

alt.data_transformers.enable("vegafusion")

DEFAULT_FILEPATH_DB = "../build/Release/database_backup.sqlite3"


# Create your views here.
def chart_view(request):
    return render(request, "dashboard/index.html")

def get_chart_data(request):
    pawn_name = request.GET.get("pawn_name", "Pawn 1")   # Default to Pawn 1
    filepath_db = request.GET.get("filepath_db", DEFAULT_FILEPATH_DB)

    # Fetch the data
    con = sqlite3.connect(filepath_db)
    query = f"SELECT time, state_name, utility FROM pawn_state_utility WHERE pawn_name = '{pawn_name}'"
    df = pd.read_sql_query(query, con)
    con.close()

    # Create the interactive chart
    selection = alt.selection_multi(fields=['state_name'], bind='legend')

    chart = alt.Chart(df).mark_line().encode(
        x="time:Q",
        y="utility:Q",
        color="state_name:N",
        tooltip=["time", "utility", "state_name"],
        opacity=alt.condition(selection, alt.value(1), alt.value(0.2))
    ).add_selection(
        selection
    ).properties(
        title=f"Utility of states for {pawn_name}",
        width='container',
        height='container'
    ).interactive()

    return JsonResponse(chart.to_dict(format="vega"))

def get_pawn_names(request):
    filepath_db = request.GET.get("filepath_db", DEFAULT_FILEPATH_DB)
    con = sqlite3.connect(filepath_db)
    query = "SELECT DISTINCT pawn_name FROM pawn_state_utility"
    df = pd.read_sql_query(query, con)
    con.close()
    return JsonResponse(df.to_dict(orient='list'))


def chart_partial(request):
    """Return an HTML fragment containing the Vega chart for the selected pawn.
    This is intended to be requested via HTMX and swapped into the page.
    """
    pawn_name = request.GET.get("pawn_name", "Pawn 1")
    plot_type = request.GET.get("plot_type", "utility")
    # additional filters for non-utility plots (these come from the new per-plot inputs)
    region = request.GET.get('region')
    category = request.GET.get('category')
    metric = request.GET.get('metric')
    # normalized filter value to use for seeding/cache when plot_type != 'utility'
    seed_filter = region or category or metric or pawn_name
    # filepath: preference order -> GET param, session value, default constant
    filepath_db = request.GET.get("filepath_db") or request.session.get('db_path') or DEFAULT_FILEPATH_DB

    # Build cache key including DB mtime so cache invalidates when DB changes
    try:
        mtime = int(os.path.getmtime(filepath_db))
    except Exception:
        mtime = 0

    # Build cache key including plot_type and any filter value so cached fragments
    # are distinct per-filter and auto-invalidate when DB mtime changes.
    if plot_type == 'utility':
        cache_key = f"chart_partial:utility:{pawn_name}:{mtime}"
    else:
        # use the normalized seed_filter (first non-empty of region/category/metric or pawn_name)
        cache_key = f"chart_partial:{plot_type}:{seed_filter}:{mtime}"
    cached = cache.get(cache_key)
    if cached:
        return HttpResponse(cached)

    con = None
    try:
        con = sqlite3.connect(filepath_db)
        # For the 'utility' plot type try to read real data; for synthetic plots we will generate data below
        df = None
        if plot_type == 'utility':
            try:
                query = "SELECT time, state_name, utility FROM pawn_state_utility WHERE pawn_name = ?"
                df = pd.read_sql_query(query, con, params=(pawn_name,))
            except Exception:
                # fallback to basic sqlite fetch
                cur = con.cursor()
                cur.execute("SELECT time, state_name, utility FROM pawn_state_utility WHERE pawn_name = ?", (pawn_name,))
                rows = cur.fetchall()
                try:
                    import pandas as _pd
                    df = _pd.DataFrame(rows, columns=['time', 'state_name', 'utility'])
                except Exception:
                    df = None
    finally:
        try:
            if con:
                con.close()
        except Exception:
            pass

    selection = alt.selection_multi(fields=['state_name'], bind='legend')

    chart = alt.Chart(df).mark_line().encode(
        x="time:Q",
        y="utility:Q",
        color="state_name:N",
        tooltip=["time", "utility", "state_name"],
        opacity=alt.condition(selection, alt.value(1), alt.value(0.2))
    ).add_selection(
        selection
    ).properties(
        title=f"Utility of states for {pawn_name}",
        width='container',
        height='container'
    ).interactive()

    # If requested plot type is not 'utility' or if df is empty, generate a synthetic dataset
    if plot_type != 'utility' or df is None or df.empty:
        # Seed from either pawn_name (for utility) or the normalized seed_filter so
        # non-utility plots are deterministic per-filter value.
        if plot_type == 'utility':
            seed_src = f"{pawn_name}|{plot_type}"
        else:
            seed_src = f"{seed_filter}|{plot_type}"
        seed = int(hashlib.md5(seed_src.encode('utf-8')).hexdigest()[:8], 16)
        rnd = random.Random(seed)

        if plot_type == 'state_bar':
            # Generate counts for a few state names
            states = [f"State {i+1}" for i in range(6)]
            counts = [rnd.randint(5, 50) for _ in states]
            import pandas as _pd
            df = _pd.DataFrame({'state_name': states, 'count': counts})
            title_val = ("State counts for " + region) if region else "State counts"
            chart = alt.Chart(df).mark_bar().encode(
                x='state_name:N',
                y='count:Q',
                color='state_name:N'
            ).properties(title=title_val)

        elif plot_type == 'reward_scatter':
            # Scatter of reward vs time
            n = 50
            xs = list(range(n))
            ys = [rnd.uniform(0, 100) for _ in range(n)]
            import pandas as _pd
            df = _pd.DataFrame({'time': xs, 'reward': ys})
            title_val = ("Reward scatter for " + category) if category else "Reward scatter"
            chart = alt.Chart(df).mark_point().encode(
                x='time:Q',
                y='reward:Q',
                tooltip=['time', 'reward']
            ).properties(title=title_val)

        elif plot_type == 'histogram':
            # Histogram of synthetic metric
            n = 200
            vals = [rnd.gauss(50 + rnd.uniform(-10,10), 15) for _ in range(n)]
            import pandas as _pd
            df = _pd.DataFrame({'value': vals})
            title_val = ("Histogram for " + metric) if metric else "Histogram"
            chart = alt.Chart(df).mark_bar().encode(
                alt.X('value:Q', bin=alt.Bin(maxbins=30)),
                y='count():Q'
            ).properties(title=title_val)

        else:
            # default fallback: simple line using random walk
            n = 60
            vals = []
            v = rnd.uniform(0, 50)
            for _ in range(n):
                v += rnd.uniform(-5, 5)
                vals.append(max(0, v))
            import pandas as _pd
            df = _pd.DataFrame({'time': list(range(n)), 'value': vals})
            title_val = "Random trend for " + seed_filter
            chart = alt.Chart(df).mark_line().encode(
                x='time:Q',
                y='value:Q'
            ).properties(title=title_val)
    
    spec = chart.to_dict(format="vega")
    html = render_to_string('dashboard/_chart.html', {'spec_json': json.dumps(spec)})
    # cache the rendered HTML for a short period (key includes mtime)
    try:
        cache.set(cache_key, html, timeout=300)
    except Exception:
        pass
    return HttpResponse(html)


def pawn_select_partial(request):
    """Return a server-rendered select element (partial) with pawn names.
    Intended to be loaded by HTMX on page load so the UI is server-driven.
    """
    # filepath: preference order -> GET param, session value, default constant
    filepath_db = request.GET.get("filepath_db") or request.session.get('db_path') or DEFAULT_FILEPATH_DB
    pawns = []
    con = None
    try:
        con = sqlite3.connect(filepath_db)
        cur = con.cursor()
        cur.execute("SELECT DISTINCT pawn_name FROM pawn_state_utility")
        rows = cur.fetchall()
        pawns = [r[0] for r in rows]
    except Exception as e:
        # return a small error fragment so HTMX injects visible feedback
        return HttpResponse(f"<div class='error'>Failed to load pawn list: {e}</div>")
    finally:
        try:
            if con:
                con.close()
        except Exception:
            pass
    selected = request.GET.get('pawn_name', pawns[0] if pawns else '')
    return render(request, 'dashboard/_pawn_select.html', {'pawns': pawns, 'selected': selected})


def set_db_path(request):
    """Set the database filepath in the user's session (POST from HTMX) and return a small fragment
    that displays the currently-set path. HTMX will swap this fragment into the page; after swap
    the client-side HTMX afterSwap handler will reload the pawn select.
    """
    if request.method == 'POST':
        new_path = request.POST.get('db_path', '').strip()
        if new_path:
            request.session['db_path'] = new_path
            request.session.modified = True
            # return a small fragment showing the chosen path
            return HttpResponse(f"<div id=\"db-path-container\">Database file: <strong>{new_path}</strong></div>")
    # fallback: show current path
    current = request.session.get('db_path', DEFAULT_FILEPATH_DB)
    return HttpResponse(f"<div id=\"db-path-container\">Database file: <strong>{current}</strong></div>")

def log_viewer(request):
    return render(request, 'dashboard/log_viewer.html')

def get_log_data(request):
    log_file_path = "z:/Hayden/programming/projects/townbuilder/build/Release/logs.log"
    log_pattern = re.compile(r'^\[(.*?)\]\s+\[(.*?)\]\s+\[(.*?)\]\s+\[(.*?)\]\s+(.*)$')

    log_messages = []
    log_levels = set()
    logger_names = set()

    try:
        with open(log_file_path, 'r') as f:
            for line in f:
                match = log_pattern.match(line)
                if match:
                    timestamp, thread_id, level, logger, message = match.groups()
                    log_levels.add(level)
                    logger_names.add(logger)
                    log_messages.append({
                        'timestamp': timestamp,
                        'thread_id': thread_id,
                        'level': level,
                        'logger': logger,
                        'message': message
                    })
    except FileNotFoundError:
        return JsonResponse({'error': 'Log file not found'}, status=404)
    
    logs = pd.DataFrame(log_messages)

    level_filter = request.GET.get('level')
    logger_filter = request.GET.get('logger')

    if level_filter:
        logs = logs[logs['level'] == level_filter]
    if logger_filter:
        logs = logs[logs['logger'] == logger_filter]

    return JsonResponse({
        'logs': logs.to_dict(orient='records'),
        'levels': sorted(log_levels),
        'loggers': sorted(logger_names)
    })


def counter_partial(request):
    # Simple server-rendered partial used by HTMX examples. The partial contains Alpine attributes
    # so it becomes interactive once inserted into the DOM (Alpine must be loaded on the page).
    return render(request, 'dashboard/_counter.html')
