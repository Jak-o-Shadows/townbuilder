import os
import sys
import sqlite3

import numpy as np
import pandas as pd
import altair as alt

sys.path.append("dashboard")
import plots

if __name__ == "__main__":
    filepath_db = "../build/Release/database_backup.sqlite3"
    dir_output = "output_plots"
    os.makedirs(dir_output, exist_ok=True)

    con = sqlite3.connect(filepath_db)


    query = "SELECT time, entity_name, Grid_x, Grid_y, Cell_x, Cell_y FROM entity_map_position"
    df = pd.read_sql_query(query, con)
    df['time'] = pd.to_numeric(df['time'], errors='coerce')
    df = df.dropna(subset=['time'])
    chart = plots.entity_positions(df)
    chart.save(os.path.join(dir_output, "entity_positions.vg.json"))
    chart.save(os.path.join(dir_output, "entity_positions.png"))



    query = "SELECT time, pawn_name, state_name, utility FROM pawn_state_utility WHERE pawn_name = ?"
    pawn_name = "::Pawn::pawnsParent::Pawn0"
    df = pd.read_sql_query(query, con, params=(pawn_name,))
    df['time'] = pd.to_numeric(df['time'], errors='coerce')
    df = df.dropna(subset=['time'])
    chart = plots.pawn_utility(df, pawn_name)
    chart.save(os.path.join(dir_output, "pawn_utility.vg.json"))
    chart.save(os.path.join(dir_output, "pawn_utility.png"))


    query = "SELECT * FROM pawn_active_states"
    df = pd.read_sql_query(query, con)
    print(df["pawn_name"].unique())
    df_melted = df.melt(id_vars=['time', 'pawn_name'], var_name='state_name', value_name='is_active')
    df_melted['time'] = pd.to_numeric(df_melted['time'], errors='coerce')
    df_melted = df_melted.dropna(subset=['time'])
    # Filter data for the specified pawn
    pawn_data = df_melted[df_melted['pawn_name'] == pawn_name]
    # Plot
    chart = plots.active_states(pawn_data)
    chart.save(os.path.join(dir_output, "pawn_active_states.vg.json"))
    chart.save(os.path.join(dir_output, "pawn_active_states.png"))

    query = "SELECT * FROM example_active_states"
    df = pd.read_sql_query(query, con)
    entity_name = df['entity_name'].iloc[0]
    df_melted = df.melt(id_vars=['time', 'entity_name'], var_name='state_name', value_name='is_active')
    df_melted['time'] = pd.to_numeric(df_melted['time'], errors='coerce')
    df_melted = df_melted.dropna(subset=['time'])
    # Filter data for the specified entity
    enity_data = df_melted[df_melted['entity_name'] == entity_name]
    #plot
    chart = plots.active_states(enity_data)
    chart.save(os.path.join(dir_output, "example_active_states.vg.json"))
    chart.save(os.path.join(dir_output, "example_active_states.png"))
    chart.save(os.path.join(dir_output, "example_active_states.html"))


