import sqlite3
import re
import json
import random
import hashlib
import os

import pandas as pd
import altair as alt

from . import plots

from django.shortcuts import render
from django.http import JsonResponse, HttpResponse
from django.template.loader import render_to_string
from django.core.cache import cache

alt.data_transformers.enable("vegafusion")

DEFAULT_FILEPATH_DB = "../build/Release/database_backup.sqlite3"


# Create your views here.
def chart_view(request):
    # If HTMX issued the request, return only the chart fragment so HTMX swaps
    # the #vis container with the fragment. This keeps HTMX/anchor flows
    # consistent (anchors use chart_view; HTMX will get a fragment).
    if request.headers.get('HX-Request') == 'true':
        return chart_partial(request)
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
    """Return an HTML fragment containing the Vega chart for the selected plot type.
    This is intended to be requested via HTMX and swapped into the page.
    """
    plot_type = request.GET.get("plot_type", "utility")

    # filepath: preference order -> GET param, session value, default constant
    filepath_db = request.GET.get("filepath_db") or request.session.get('db_path') or DEFAULT_FILEPATH_DB

    con = None
    chart = alt.Chart().mark_text(text=f"Could not generate plot '{plot_type}'").properties(title="Error")
    try:
        con = sqlite3.connect(filepath_db)
        
        if plot_type == 'utility':
            pawn_name = request.GET.get("pawn_name", "Pawn 1")

            query = "SELECT time, pawn_name, state_name, utility FROM pawn_state_utility WHERE pawn_name = ?"
            df = pd.read_sql_query(query, con, params=(pawn_name,))
            df['time'] = pd.to_numeric(df['time'], errors='coerce')
            df = df.dropna(subset=['time'])
            chart = plots.pawn_utility(df, pawn_name)
        
        elif plot_type == 'entity_positions':
            query = "SELECT time, entity_name, Grid_x, Grid_y, Cell_x, Cell_y FROM entity_map_position"
            df = pd.read_sql_query(query, con)
            df['time'] = pd.to_numeric(df['time'], errors='coerce')
            df = df.dropna(subset=['time'])
            chart = plots.entity_positions(df)

        elif plot_type == 'grid_heatmap':
            query = "SELECT time, Grid_x, Grid_y FROM pawn_positions"
            df = pd.read_sql_query(query, con)
            df['time'] = pd.to_numeric(df['time'], errors='coerce')
            df = df.dropna(subset=['time'])
            chart = plots.grid_heatmap(df)
        
        else:
            chart = alt.Chart().mark_text(text=f"Unknown plot type: {plot_type}").properties(title="Error")

    except Exception as e:
        # Log the error
        pass
    finally:
        if con:
            con.close()

    spec = chart.to_dict(format="vega")
    html = render_to_string('dashboard/_chart.html', {'spec_json': json.dumps(spec)})
        
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
