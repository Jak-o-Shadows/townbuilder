import sqlite3
import re

import pandas as pd
import altair as alt

from django.shortcuts import render
from django.http import JsonResponse

# Create your views here.
def chart_view(request):
    return render(request, "dashboard/index.html")

def get_chart_data(request):
    pawn_name = request.GET.get("pawn_name", "Pawn 1")   # Default to Pawn 1
    filepath_db = request.GET.get("filepath_db", "../build/Release/database.db")

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

    return JsonResponse(chart.to_dict())

def get_pawn_names(request):
    filepath_db = request.GET.get("filepath_db", "../build/Release/database.db")
    con = sqlite3.connect(filepath_db)
    query = "SELECT DISTINCT pawn_name FROM pawn_state_utility"
    df = pd.read_sql_query(query, con)
    con.close()
    return JsonResponse(df.to_dict(orient='list'))

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
