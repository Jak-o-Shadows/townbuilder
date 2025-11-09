

import pandas as pd
import altair as alt

alt.data_transformers.enable("vegafusion")

def pawn_utility(utility_data, pawn_name):
    """
    Plots the utility values of different states for a given pawn over time.

    Parameters:
    - utility_data: DataFrame with columns ['time', 'pawn_name', 'state_name', 'utility']
    - pawn_name: Name of the pawn to filter data for

    Returns:
    - Altair Chart object
    """
    # Filter data for the specified pawn
    pawn_data = utility_data[utility_data['pawn_name'] == pawn_name]

    # Create the Altair chart
    selection = alt.selection_multi(fields=['state_name'], bind='legend')
    chart = alt.Chart(pawn_data).mark_line().encode(
        x='time:T',
        y='utility:Q',
        color='state_name:N',
        tooltip=['time:T', 'state_name:N', 'utility:Q'],
        opacity=alt.condition(selection, alt.value(1), alt.value(0.2))
    ).add_selection(
        selection
    ).properties(
        title=f'Utility Values Over Time for {pawn_name}',
        width="container",
        height="container"
    ).interactive()

    return chart

def coordGrid_to_map(df, grid_size_x=20, grid_size_y=20):
    """
    Converts grid and cell coordinates to map coordinates.

    Parameters:
    - df: DataFrame with columns ['Grid_x', 'Grid_y', 'Cell_x', 'Cell_y']
    - grid_size_x: Size of each grid cell in the x direction
    - grid_size_y: Size of each grid cell in the y direction

    Returns:
    - DataFrame with additional columns ['map_x', 'map_y']
    """
    df['map_x'] = df['Cell_x'] + (df['Grid_x'] * grid_size_x)
    df['map_y'] = df['Cell_y'] + (df['Grid_y'] * grid_size_y)
    return df

def entity_positions(position_data):
    """
    Plots the positions of entities on the map over time.

    Parameters:
    - position_data: DataFrame with columns ['time', 'entity_name', 'Grid_x', 'Grid_y', 'Cell_x', 'Cell_y']

    Returns:
    - Altair Chart object
    """

    # Calculate the "actual" map positin based on the cell & grid coordinates
    position_data = coordGrid_to_map(position_data)

    # Create the altair chart
    chart = alt.Chart(position_data).mark_line().encode(
        x='map_x:Q',
        y='map_y:Q',
        color='entity_name:N',
        tooltip=['time:T', 'entity_name:N', 'map_x:Q', 'map_y:Q']
    ).add_selection(
        alt.selection_multi(fields=['entity_name'], bind='legend')
    ).properties(
        title='Entity Positions Over Time',
        width="container",
        height="container"
    ).interactive()

    return chart

def grid_heatmap(position_data):
    """
    Plots a heatmap of entity density on the map.

    There is a slider for the current timestep;
    There is a slider for the grid cell size - Goes between exactly 1 cell, and 4 cells across.

    Parameters:
    - position_data: DataFrame with columns ['time', 'Grid_x', 'Grid_y']
    
    Returns:
    - Altair Chart object
    """

    if position_data.empty:
        return alt.Chart().mark_text(text="No data to display").properties(title='Entity Density on the Map')

    # Note that we don't care about cell position here, as we're only going down to the resolution of the grid
    
    # Slider for time
    min_time = int(position_data['time'].min())
    max_time = int(position_data['time'].max())
    time_slider = alt.binding_range(min=min_time, max=max_time, step=1, name='Time: ')
    time_select = alt.selection_single(name="time_selection", fields=['time'], bind=time_slider, init={'time': min_time})

    # Slider for grid aggregation level
    grid_agg_slider = alt.binding_range(min=1, max=4, step=1, name='Grid Aggregation: ')
    grid_agg_param = alt.param(name="grid_agg", value=1, bind=grid_agg_slider)

    chart = alt.Chart(position_data).add_params(
        grid_agg_param,
    ).add_selection(
        time_select
    ).transform_filter(
        time_select
    ).transform_calculate(
        Grid_x_binned='floor(datum.Grid_x / grid_agg) * grid_agg',
        Grid_y_binned='floor(datum.Grid_y / grid_agg) * grid_agg'
    ).mark_rect().encode(
        x=alt.X('Grid_x_binned:O', title='Grid X'),
        y=alt.Y('Grid_y_binned:O', title='Grid Y'),
        color=alt.Color('count():Q', title='Entity Count'),
        tooltip=['Grid_x_binned:O', 'Grid_y_binned:O', 'count():Q']
    ).properties(
        title='Entity Density on the Map',
        width="container",
        height="container"
    ).interactive()

    return chart


