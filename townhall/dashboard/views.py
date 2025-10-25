import sqlite3

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