

import numpy as np
import pandas as pd
import altair as alt

#alt.data_transformers.enable("vegafusion")


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

def active_states(active_states_data):
    """
    Plots the active states of a given pawn over time as a stacked area chart.

    Parameters
    ----------
    - active_states_data: DataFrame with columns ['time', 'state_name', 'is_active']

    Returns
    -------
    - Altair Chart object
    """

    if active_states_data.empty:
        return alt.Chart().mark_text(text="No data to display").properties(
            title='Active States Over Time'
        )

    # Ensure time is parsed as temporal. If numeric, assume it's elapsed seconds
    # (relative game time). If values look like epoch milliseconds, handle that.
    if pd.api.types.is_numeric_dtype(active_states_data['time']):
        maxv = active_states_data['time'].abs().max()
        # If values are very large (e.g. epoch ms ~1e12), treat as milliseconds,
        # otherwise treat as seconds (relative game time).
        unit = 'ms' if maxv > 1e11 else 's'
        active_states_data['time'] = pd.to_datetime(active_states_data['time'], unit=unit, errors='coerce')
    else:
        active_states_data['time'] = pd.to_datetime(active_states_data['time'], errors='coerce')


    df = active_states_data.sort_values(["state_name", "time"])

    # For each state, shift the time column to get the next timestamp
    df["end_time"] = df.groupby("state_name")["time"].shift(-1)

    # Keep only rows where active == True
    #   Interval segments are where is_active is True and end_time is not NaN
    #   Single points still need to be shown
    segments = df[df["is_active"] == True].dropna(subset=["end_time"])
    single_points = df[(df["is_active"] == True) & ((df["end_time"].isna()) | (df["end_time"] == df["time"]))]

    chart = (
        alt.Chart(segments).mark_rule(size=4   # a thick line segment
            ).encode(
                x="time:T",
                x2="end_time:T",
                y=alt.Y("state_name:N", sort=None),
                color="state_name:N",
                tooltip=["state_name", "time"]
            ) + \
        alt.Chart(single_points).mark_circle(
                size=80
            ).encode(
                x="time:T",
                y=alt.Y("state_name:N", sort=None),
                color="state_name:N",
                tooltip=["state_name", "time"]
            )
        ).properties(
            title='Active States Over Time',
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
    df['map_x'] = df['Grid_x'] + df['Cell_x']/2
    df['map_y'] = df['Grid_y'] + df['Cell_y']/2
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
    time_slider = alt.binding_range(min=min_time, max=max_time, step=1, name='Time:N')
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


def pawn_state_duration_plot(intervals_data, bin_period_s=30, max_time=None):
    """
    Produce an Altair chart that shows the the mean/median (toggleable) duration of each state,
    across all pawns, for all states, binned into time periods.

    Axis:
     * The y-axis is the time period - latest time starting at the bottom, similar to a waterfall graph. 
       This should be binned into time periods (configurable in code, not via the altair plot).
     * The x-axis is each state.
     * The colour of each cell is the mean/median duration of that state for that time period.

    However as aggregate metrics can be misleading, depending on the distribution of the data, 
    clicking on a cell will show the histogram of the durations for that state in that time period, across all pawns.
    This will be shown in a separate facet/chart below.

    Parameters
    ----------
    - intervals_data: DataFrame matching the `pawn_state_intervals` table
      (columns pawn_name, state_name, entered_at, exited_at, duration_s).
      Times are game-time seconds; exited_at may be NULL if still open.
    - bin_period_s: width of each time bin in seconds (Python-side binning
      on entered_at).
    - max_time: reference "now" used to close open intervals. Defaults to
      the max observed entered_at/exited_at.

    Returns
    -------
    - Altair Chart (heatmap vconcat'ed with drill-down histogram).
    """
    if intervals_data is None or intervals_data.empty:
        return alt.Chart(pd.DataFrame({"text": ["No data to display"]})).mark_text().encode(
            text="text:N"
        ).properties(title="State Duration by Time Period")

    df = intervals_data.copy()
    df["entered_at"] = pd.to_numeric(df["entered_at"], errors="coerce")
    df["exited_at"] = pd.to_numeric(df["exited_at"], errors="coerce")
    df["duration_s"] = pd.to_numeric(df["duration_s"], errors="coerce")
    df = df.dropna(subset=["entered_at", "state_name"])
    if df.empty:
        return alt.Chart(pd.DataFrame({"text": ["No data to display"]})).mark_text().encode(
            text="text:N"
        ).properties(title="State Duration by Time Period")

    if max_time is None:
        cands = [float(df["entered_at"].max())]
        if df["exited_at"].notna().any():
            cands.append(float(df["exited_at"].max()))
        max_time = float(np.nanmax(cands))
    open_mask = df["duration_s"].isna() | df["exited_at"].isna()
    df.loc[open_mask, "exited_at"] = df.loc[open_mask, "exited_at"].fillna(max_time)
    df.loc[open_mask, "duration_s"] = df.loc[open_mask, "exited_at"] - df.loc[open_mask, "entered_at"]
    df = df.dropna(subset=["duration_s"])
    df = df[df["duration_s"] >= 0]
    if df.empty:
        return alt.Chart(pd.DataFrame({"text": ["No data to display"]})).mark_text().encode(
            text="text:N"
        ).properties(title="State Duration by Time Period")

    df["bin_start"] = np.floor(df["entered_at"] / bin_period_s) * bin_period_s
    df["bin_end"] = df["bin_start"] + bin_period_s
    df["bin_label"] = df["bin_start"].astype(int).astype(str) + "-" + df["bin_end"].astype(int).astype(str) + "s"
    agg = df.groupby(["bin_start", "bin_label", "state_name"], as_index=False).agg(
        mean_s=("duration_s", "mean"), median_s=("duration_s", "median"), n=("duration_s", "size"))
    bin_order = agg.sort_values("bin_start")["bin_label"].drop_duplicates().tolist()
    metric = alt.param(name="metric", value="mean",
        bind=alt.binding_radio(options=["mean", "median"], name="Aggregate: "))
    heat = alt.Chart(agg).add_params(metric).transform_calculate(
        agg_value="metric == 'median' ? datum.median_s : datum.mean_s"
    ).mark_rect().encode(
        x=alt.X("state_name:N", title="State"),
        y=alt.Y("bin_label:N", title="Time period (entry)", sort=bin_order),
        color=alt.Color("agg_value:Q", title="Duration (s)", scale=alt.Scale(scheme="blues")),
        tooltip=["state_name:N", "bin_label:N",
            alt.Tooltip("mean_s:Q", title="Mean (s)", format=".2f"),
            alt.Tooltip("median_s:Q", title="Median (s)", format=".2f"),
            alt.Tooltip("n:Q", title="Samples")],
    ).properties(title="Mean/Median State Duration by Time Period",
        width="container", height="container")
    sel = alt.selection_point(name="cell_select", fields=["state_name", "bin_label"])
    heat = heat.add_params(sel).encode(
        strokeOpacity=alt.condition(sel, alt.value(1.0), alt.value(0.0)), stroke=alt.value("red"))
    hist = alt.Chart(df).transform_filter(sel).mark_bar().encode(
        x=alt.X("duration_s:Q", bin=alt.Bin(maxbins=30), title="Duration (s)"),
        y=alt.Y("count():Q", title="Count"),
    ).properties(title="Duration distribution for selected cell (click a cell above)",
        width="container")
    return alt.vconcat(heat, hist).resolve_scale(color="independent")