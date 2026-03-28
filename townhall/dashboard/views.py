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
from . import matplotlib_plots
from . import forms

from django.shortcuts import render
from django.http import HttpResponse
from django.template.loader import render_to_string
from django.core.cache import cache
from django.conf import settings
from django.middleware.csrf import get_token

alt.data_transformers.enable("vegafusion")

DEFAULT_FILEPATH_DB =  "../build/Release/asdfdatabase_backup.sqlite3"
DEFAULT_FILEPATH_LOG = "../build/Release/logs.log"


def get_active_filepath(dataset_id=None):
    """Get the path of a specific dataset, or fallback to default."""
    if dataset_id:
        try:
            file_obj = models.InputDatabaseFile.objects.get(id=dataset_id, is_open=True)
            if file_obj.exists:
                return file_obj.path
        except models.InputDatabaseFile.DoesNotExist:
            pass
    return DEFAULT_FILEPATH_DB


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
                # Get dataset_id from query params
                dataset_id = request.GET.get('dataset_id', None)
                dataset_id = int(dataset_id) if dataset_id else None
                active_dataset = None
                if dataset_id:
                    try:
                        active_dataset = models.InputDatabaseFile.objects.get(id=dataset_id, is_open=True)
                    except models.InputDatabaseFile.DoesNotExist:
                        pass
                content = render_to_string(template_name, {
                    'content': fragment_html,
                    'dataset_id': dataset_id,
                    'active_dataset': active_dataset
                })
            return HttpResponse(content)
        return _wrapped
    return decorater



def index(request):
    """Render the main dashboard page. The first plot is loaded via an htmx request from the template."""
    dataset_id = request.GET.get('dataset_id', None)
    dataset_id = int(dataset_id) if dataset_id else None
    active_dataset = None
    if dataset_id:
        try:
            active_dataset = models.InputDatabaseFile.objects.get(id=dataset_id, is_open=True)
        except models.InputDatabaseFile.DoesNotExist:
            pass
    return render(request, "dashboard/full.html", {
        'dataset_id': dataset_id,
        'active_dataset': active_dataset
    })


def datasets_list(request):
    """Render the datasets management page."""
    open_files = models.InputDatabaseFile.objects.filter(is_open=True)
    csrf_token = get_token(request)
    folder_browse_form = forms.DatasetFolderBrowseForm()
    dataset_upload_form = forms.DatasetUploadForm()
    return render(request, "dashboard/datasets.html", {
        'available_datasets': open_files,
        'csrf_token': csrf_token,
        'folder_browse_form': folder_browse_form,
        'dataset_upload_form': dataset_upload_form
    })


def datasets_nav(request):
    """Return partial HTML for the datasets dropdown in the navbar."""
    open_files = models.InputDatabaseFile.objects.filter(is_open=True)
    html = render_to_string('dashboard/partial_nav_datasets.html', {'available_datasets': open_files})
    return HttpResponse(html)

######################## Plots #######################

@hx_or_full()
def plot_matplotlib_example(request):
    """Return an HTML fragment containing the matplotlib plot."""
    chart_html = matplotlib_plots.example_plot()
    html = render_to_string('dashboard/partial_single_matplotlib_plot.html', {'chart_html': chart_html,
                                                                          'content_header': "Matplotlib Example Plot",
                                                                          'content_paragraphs': ["This is an example of a matplotlib plot rendered with mpld3."]
                                                                          })
    return html


@hx_or_full()
def plot_pawn_utility(request):
    """Return an HTML fragment containing the Vega chart for the utility plot."""
    # Get dataset_id from query params
    dataset_id = request.GET.get('dataset_id')
    dataset_id = int(dataset_id) if dataset_id else None
    filepath_db = get_active_filepath(dataset_id)

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
    # Get dataset_id from query params
    dataset_id = request.GET.get('dataset_id')
    dataset_id = int(dataset_id) if dataset_id else None
    filepath_db = get_active_filepath(dataset_id)

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
    # Get dataset_id from query params
    dataset_id = request.GET.get('dataset_id')
    dataset_id = int(dataset_id) if dataset_id else None
    filepath_db = get_active_filepath(dataset_id)
    
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
    # Get dataset_id from query params
    dataset_id = request.GET.get('dataset_id')
    dataset_id = int(dataset_id) if dataset_id else None
    filepath_db = get_active_filepath(dataset_id)
    
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






########################### Log Viewer ###########################

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


########################### Database File Management ###########################

@hx_or_full()
def open_database_files(request):
    """Handle opening multiple database files."""
    if request.method == 'POST':
        uploaded_files = request.FILES.getlist('files')
        temp_dir = os.path.join(settings.BASE_DIR, 'temp_dbs')
        os.makedirs(temp_dir, exist_ok=True)
        for uploaded_file in uploaded_files:
            temp_path = os.path.join(temp_dir, uploaded_file.name)
            with open(temp_path, 'wb') as f:
                for chunk in uploaded_file.chunks():
                    f.write(chunk)
            obj, created = models.InputDatabaseFile.objects.get_or_create(
                path=temp_path,
                defaults={'is_open': True, 'is_temporary': True, 'label': uploaded_file.name}
            )
            if not created:
                obj.is_open = True
                obj.save()
    open_files = models.InputDatabaseFile.objects.filter(is_open=True)
    html = render_to_string('dashboard/partial_dataset_manager.html', {'available_datasets': open_files, 'csrf_token': get_token(request)})
    return html


def set_active_file(request, file_id):
    """Redirect to dashboard with the specified dataset. Used for opening datasets in new tabs."""
    from django.http import HttpResponseRedirect
    from django.urls import reverse
    return HttpResponseRedirect(f"{reverse('dashboard:index')}?dataset_id={file_id}")


def close_file(request, file_id):
    """Close a database file."""
    try:
        file_obj = models.InputDatabaseFile.objects.get(id=file_id)
        file_obj.is_open = False
        file_obj.save()
        # If temporary (uploaded), delete the file
        if file_obj.is_temporary:
            os.remove(file_obj.path)
    except models.InputDatabaseFile.DoesNotExist:
        pass
    open_files = models.InputDatabaseFile.objects.filter(is_open=True)
    html = render_to_string('dashboard/partial_open_files.html', {'open_files': open_files, 'csrf_token': get_token(request)})
    return HttpResponse(html)

