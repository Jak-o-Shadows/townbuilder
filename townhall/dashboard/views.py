import sqlite3
import re
import json
import random
import hashlib
import os
import functools
from turtle import width

import pandas as pd
import altair as alt

from . import plots
from . import models

from django.shortcuts import render
from django.http import JsonResponse, HttpResponse
from django.template.loader import render_to_string
from django.core.cache import cache

alt.data_transformers.enable("vegafusion")

DEFAULT_FILEPATH_DB =  "../build/Release/database_backup.sqlite3"
DEFAULT_FILEPATH_LOG = "../build/Release/logs.log"


def hx_or_full(template_name="dashboard/full.html"):
    """Decorator for view functions that return an HTML fragment.
    If the request contains the HTMX header "HX-Target", return the fragment as-is.
    Otherwise render the fragment into `template_name` under `context_key` so a
    full page is produced.
    """
    def decorater(view_func):
        @functools.wraps(view_func)
        def _wrapped(request, *args, **kwargs):
            # Check for HTMX header
            is_hx = request.headers.get("HX-Request", None) is not None
            print(f"URL: {request.path} | HTMX request: {is_hx}")
            fragment_html = view_func(request, *args, **kwargs)
            if is_hx:
                content = fragment_html
            else:
                content = render_to_string(template_name, {'content': fragment_html})
            return HttpResponse(content)
        return _wrapped
    return decorater



def index(request):
    """Render the main dashboard page. The first plot is loaded via an htmx request from the template."""
    return render(request, "dashboard/full.html")

######################## Plots #######################

@hx_or_full()
def plot_pawn_utility(request):
    """Return an HTML fragment containing the Vega chart for the utility plot."""
    # filepath: preference order -> GET param, session value, default constant
    filepath_db = request.GET.get("filepath_db") or request.session.get('db_path') or DEFAULT_FILEPATH_DB

    con = None
    chart = alt.Chart().mark_text(text="Could not generate plot 'utility'").properties(title="Error")
    try:
        con = sqlite3.connect(filepath_db)
        pawn_name = request.GET.get("pawn_name", "Pawn 1")

        query = "SELECT time, pawn_name, state_name, utility FROM pawn_state_utility WHERE pawn_name = ?"
        df = pd.read_sql_query(query, con, params=(pawn_name,))
        df['time'] = pd.to_numeric(df['time'], errors='coerce')
        df = df.dropna(subset=['time'])
        chart = plots.pawn_utility(df, pawn_name)
        
        content_paragraphs = [f"This plot shows the utility values over time for the pawn named '{pawn_name}'."]

    except Exception as e:
        content_paragraphs = [f"An error occurred while generating the plot: {e}"]
    finally:
        if con:
            con.close()

    

    spec = chart.to_dict(format="vega")
    html = render_to_string('dashboard/partial_single_altair_plot.html', {'spec_json': json.dumps(spec),
                                                                          'content_header': "Pawn State Machine state utility",
                                                                          'content_paragraphs': content_paragraphs
                                                                          })
        
    return html

@hx_or_full()
def plot_entity_positions(request):
    """Return an HTML fragment containing the Vega chart for the entity positions plot."""
    # filepath: preference order -> GET param, session value, default constant
    filepath_db = request.GET.get("filepath_db") or request.session.get('db_path') or DEFAULT_FILEPATH_DB

    con = None
    chart = alt.Chart().mark_text(text="Could not generate plot 'entity_positions'").properties(title="Error")
    try:
        con = sqlite3.connect(filepath_db)
        query = "SELECT time, entity_name, Grid_x, Grid_y, Cell_x, Cell_y FROM entity_map_position"
        df = pd.read_sql_query(query, con)
        df['time'] = pd.to_numeric(df['time'], errors='coerce')
        df = df.dropna(subset=['time'])
        chart = plots.entity_positions(df)

        content_paragraphs = ["This plot shows the positions of entities on the map over time. Each point represents an entity's location at a specific timestamp."]
    except Exception as e:
        content_paragraphs = [f"An error occurred while generating the plot: {e}"]
    finally:
        if con:
            con.close()

    spec = chart.to_dict(format="vega")
    html = render_to_string('dashboard/partial_single_altair_plot.html', {'spec_json': json.dumps(spec),
                                                                          'content_header': "Entity Positions on Map Over Time",
                                                                          'content_paragraphs': content_paragraphs
                                                                          })
        
    return html

@hx_or_full()
def plot_example_state(request):
    """Return a HTML fragment containing the Vega chart for the example state plot."""
    filepath_db = request.GET.get("filepath_db") or request.session.get('db_path') or DEFAULT_FILEPATH_DB
    
    con = None
    chart = alt.Chart().mark_text(text="Could not generate plot 'entity_positions'").properties(title="Error")

    try:
        con = sqlite3.connect(filepath_db)
        query = "SELECT * FROM example_active_states"
        df = pd.read_sql_query(query, con)
        entity_name = df['entity_name'].iloc[0]
        df_melted = df.melt(id_vars=['time', 'entity_name'], var_name='state_name', value_name='is_active')
        df_melted['time'] = pd.to_numeric(df_melted['time'], errors='coerce')
        df_melted = df_melted.dropna(subset=['time'])
        # Filter data for the specified entity
        entity_data = df_melted[df_melted['entity_name'] == entity_name]
        #plot
        chart = plots.active_states(entity_data)

        content_paragraphs = [f"This plot shows the active states over time for the entity named '{entity_name}'. This is purely an example that serves as an example of how to use a HFSM2 state machine, and how to log the active state."]
    except Exception as e:
        content_paragraphs = [f"An error occurred while generating the plot: {e}"]
    finally:
        if con:
            con.close()

    spec = chart.to_dict(format="vega")
    html = render_to_string('dashboard/partial_single_altair_plot.html', {'spec_json': json.dumps(spec),
                                                                          'content_header': "Example Active States",
                                                                          'content_paragraphs': content_paragraphs
                                                                          })

    return html

@hx_or_full()
def plot_pawn_state(request):
    """Return a HTML fragment containing the Vega chart for the pawn state plot."""
    filepath_db = request.GET.get("filepath_db") or request.session.get('db_path') or DEFAULT_FILEPATH_DB
    
    con = None
    chart = alt.Chart().mark_text(text="Could not generate plot 'pawn_state'").properties(title="Error")

    try:
        con = sqlite3.connect(filepath_db)
        query = "SELECT * FROM pawn_active_states"
        df = pd.read_sql_query(query, con)
        pawn_name = df['pawn_name'].iloc[0]
        df_melted = df.melt(id_vars=['time', 'pawn_name'], var_name='state_name', value_name='is_active')
        df_melted['time'] = pd.to_numeric(df_melted['time'], errors='coerce')
        df_melted = df_melted.dropna(subset=['time'])
        # Filter data for the specified pawn
        pawn_data = df_melted[df_melted['pawn_name'] == pawn_name]
        # Plot
        chart = plots.active_states(pawn_data)

        content_paragraphs = [f"This plot shows the active states over time for the pawn named '{pawn_name}'."]

    except Exception as e:
        content_paragraphs = [f"An error occurred while generating the plot: {e}"]
    finally:
        if con:
            con.close()

    spec = chart.to_dict(format="vega")
    html = render_to_string('dashboard/partial_single_altair_plot.html', {'spec_json': json.dumps(spec),
                                                                          'content_header': "Pawn Active States",
                                                                          'content_paragraphs': content_paragraphs
                                                                          })

    return html

@hx_or_full()
def log_viewer(request):
    # TODO: Finish log viewer, following the example in the hypermedia.systems book
    return render_to_string('dashboard/partial_logs.html')

def log_entries(request):
    # Read and parse the log file, support filtering and pagination.
    log_file_path = DEFAULT_FILEPATH_LOG
    log_pattern = re.compile(r'^\[(.*?)\]\s+\[(.*?)\]\s+\[(.*?)\]\s+\[(.*?)\]\s+(.*)$')

    messages = []
    try:
        with open(log_file_path, 'r', encoding='utf-8', errors='replace') as f:
            for line in f:
                m = log_pattern.match(line.strip())
                if not m:
                    continue
                timestamp, game_timestamp, level, logger, message = m.groups()
                messages.append({
                    'timestamp': timestamp,
                    'game_timestamp': game_timestamp,
                    'level': level,
                    'logger': logger,
                    'message': message
                })
    except FileNotFoundError:
        return HttpResponse('<div class="error">Log file not found</div>', status=404)

    # Apply filters
    level = request.GET.get('level', '').strip()
    logger = request.GET.get('logger', '').strip()
    q = request.GET.get('q', '').strip()

    if level:
        messages = [m for m in messages if m['level'] == level]
    if logger:
        messages = [m for m in messages if logger in m['logger']]
    if q:
        qlower = q.lower()
        messages = [m for m in messages if qlower in m['message'].lower()]

    # Range-based pagination using integer start_message and end_message indexes.
    # If end_message is omitted, we use a default count to form the range [start, start+count).
    DEFAULT_COUNT = 100

    total = len(messages)
    # Parse start and end as integers if present; default start=0
    start_param = request.GET.get('start_message', "0")
    end_param = request.GET.get('end_message', str(DEFAULT_COUNT))

    start = int(start_param)
    end = int(end_param)

    # Bound start/end
    start = max(0, min(start, total))
    end = max(start, min(end, total))

    page_msgs = messages[start:end]
    has_more = end < total

    # build next_url preserving existing query params; advance start to `end` for the next chunk
    from urllib.parse import urlencode
    params = dict(request.GET.items())
    params["start_message"] = str(end)
    params["end_message"] = str(end + DEFAULT_COUNT)
    next_url = request.path + '?' + urlencode(params)

    # Return full table wrapper only when starting at 0; otherwise return rows-only for append
    if start <= 0:
        html = render_to_string('dashboard/partial_log_results.html', {
            'logs': page_msgs,
            'has_more': has_more,
            'next_url': next_url,
        })
    else:
        html = render_to_string('dashboard/partial_log_rows.html', {
            'logs': page_msgs,
            'has_more': has_more,
            'next_url': next_url,
        })

    return HttpResponse(html)

def log_levels(request):
    levels = ["trace", "debug", "info", "warning", "error", "critical"]
    html = render_to_string('dashboard/partial_log_levels.html', {'log_levels': levels})
    return HttpResponse(html)

def log_loggers(request):
    log_file_path = DEFAULT_FILEPATH_LOG
    log_pattern = re.compile(r'^\[(.*?)\]\s+\[(.*?)\]\s+\[(.*?)\]\s+\[(.*?)\]\s+(.*)$')
    loggers = set()
    try:
        with open(log_file_path, 'r', encoding='utf-8', errors='replace') as f:
            for line in f:
                m = log_pattern.match(line.strip())
                if not m:
                    continue
                _, _, _, logger, _ = m.groups()
                loggers.add(logger)
    except FileNotFoundError:
        return HttpResponse('<select name="logger"><option value="">All</option></select>')

    sorted_loggers = sorted(loggers)
    html = render_to_string('dashboard/partial_log_loggers.html', {'loggers': sorted_loggers})
    return HttpResponse(html)


########################### Log Viewer ###########################








########################## Setting what database to open ##########################

def dataset(request):
    """Set the dataset (database filepath and game id) in the user's session (POST from HTMX)
    and return a small fragment that displays the currently-set dataset. HTMX will swap this
    fragment into the page; after swap the client-side HTMX afterSwap handler will reload
    the pawn select.
    """
    if request.method == 'POST':
        new_filepath_db = request.POST.get('filepath_db', '').strip()
        
        if new_filepath_db:
            # Load the game_id from the database
            # TODO: Put this logic in properly
            new_game_id = "placeholder"

            settings, created = models.DatasetModel.objects.get_or_create()
            settings.filepath_db = new_filepath_db
            settings.game_id = new_game_id
            settings.save()

    # TODO: THIS ISN"T FINISHED. NOT SURE HOW TO WORK WITH HTMX AND DJANGO MODELS YET






# @hx_or_full()
# def plot_grid_heatmap(request):
#     """Return an HTML fragment containing the Vega chart for the grid heatmap plot."""
#     # filepath: preference order -> GET param, session value, default constant
#     filepath_db = request.GET.get("filepath_db") or request.session.get('db_path') or DEFAULT_FILEPATH_DB

#     con = None
#     chart = alt.Chart().mark_text(text="Could not generate plot 'grid_heatmap'").properties(title="Error")
#     try:
#         con = sqlite3.connect(filepath_db)
#         query = "SELECT time, Grid_x, Grid_y FROM pawn_positions"
#         df = pd.read_sql_query(query, con)
#         df['time'] = pd.to_numeric(df['time'], errors='coerce')
#         df = df.dropna(subset=['time'])
#         chart = plots.grid_heatmap(df)

#     except Exception as e:
#         # Log the error
#         pass
#     finally:
#         if con:
#             con.close()

#     spec = chart.to_dict(format="vega")
#     html = render_to_string('dashboard/_chart.html', {'spec_json': json.dumps(spec)})
        
#     return html




# def pawn_select_partial(request):
#     """Return a server-rendered select element (partial) with pawn names.
#     Intended to be loaded by HTMX on page load so the UI is server-driven.
#     """
#     # filepath: preference order -> GET param, session value, default constant
#     filepath_db = request.GET.get("filepath_db") or request.session.get('db_path') or DEFAULT_FILEPATH_DB
#     pawns = []
#     con = None
#     try:
#         con = sqlite3.connect(filepath_db)
#         cur = con.cursor()
#         cur.execute("SELECT DISTINCT pawn_name FROM pawn_state_utility")
#         rows = cur.fetchall()
#         pawns = [r[0] for r in rows]
#     except Exception as e:
#         # return a small error fragment so HTMX injects visible feedback
#         return HttpResponse(f"<div class='error'>Failed to load pawn list: {e}</div>")
#     finally:
#         try:
#             if con:
#                 con.close()
#         except Exception:
#             pass
#     selected = request.GET.get('pawn_name', pawns[0] if pawns else '')
#     return render(request, 'dashboard/_pawn_select.html', {'pawns': pawns, 'selected': selected})


# def set_db_path(request):
#     """Set the database filepath in the user's session (POST from HTMX) and return a small fragment
#     that displays the currently-set path. HTMX will swap this fragment into the page; after swap
#     the client-side HTMX afterSwap handler will reload the pawn select.
#     """
#     if request.method == 'POST':
#         new_path = request.POST.get('db_path', '').strip()
#         if new_path:
#             request.session['db_path'] = new_path
#             request.session.modified = True
#             # return a small fragment showing the chosen path
#             return HttpResponse(f"<div id=\"db-path-container\">Database file: <strong>{new_path}</strong></div>")
#     # fallback: show current path
#     current = request.session.get('db_path', DEFAULT_FILEPATH_DB)
#     return HttpResponse(f"<div id=\"db-path-container\">Database file: <strong>{current}</strong></div>")


# def log_viewer(request):
#     return render(request, 'dashboard/log_viewer.html')


# def get_log_data(request):
#     log_file_path = "z:/Hayden/programming/projects/townbuilder/build/Release/logs.log"
#     log_pattern = re.compile(r'^\[(.*?)\]\s+\[(.*?)\]\s+\[(.*?)\]\s+\[(.*?)\]\s+(.*)$')

#     log_messages = []
#     log_levels = set()
#     logger_names = set()

#     try:
#         with open(log_file_path, 'r') as f:
#             for line in f:
#                 match = log_pattern.match(line)
#                 if match:
#                     timestamp, thread_id, level, logger, message = match.groups()
#                     log_levels.add(level)
#                     logger_names.add(logger)
#                     log_messages.append({
#                         'timestamp': timestamp,
#                         'thread_id': thread_id,
#                         'level': level,
#                         'logger': logger,
#                         'message': message
#                     })
#     except FileNotFoundError:
#         return JsonResponse({'error': 'Log file not found'}, status=404)
    
#     logs = pd.DataFrame(log_messages)

#     level_filter = request.GET.get('level')
#     logger_filter = request.GET.get('logger')

#     if level_filter:
#         logs = logs[logs['level'] == level_filter]
#     if logger_filter:
#         logs = logs[logs['logger'] == logger_filter]

#     return JsonResponse({
#         'logs': logs.to_dict(orient='records'),
#         'levels': sorted(log_levels),
#         'loggers': sorted(logger_names)
#     })


# def counter_partial(request):
#     # Simple server-rendered partial used by HTMX examples. The partial contains Alpine attributes
#     # so it becomes interactive once inserted into the DOM (Alpine must be loaded on the page).
#     return render(request, 'dashboard/_counter.html')
