from django.urls import path
app_name = "dashboard"
from . import views

urlpatterns = [
    path("", views.index, name="index"),
    path("datasets/", views.datasets_list, name="datasets"),
    path("datasets/nav/", views.datasets_nav, name="datasets_nav"),

    path("remote_file_browser/", views.remote_file_browser, name="remote_file_browser"),


    # Various entries for the main panel
    path("plot/pawn_utility/", views.plot_pawn_utility, name="plot_pawn_utility"),
    path("plot/entity_positions/", views.plot_entity_positions, name="plot_entity_positions"),
    path("plot/example_state", views.plot_example_state, name="plot_example_state"),
    path("plot/pawn_state", views.plot_pawn_state, name="plot_pawn_state"),
    #path("plot/grid_heatmap/", views.plot_grid_heatmap, name="plot_grid_heatmap"),
    path("logs", views.log_viewer, name="log_viewer"),

    #path("set_db_path/", views.set_db_path, name="set_db_path"),
    #path("pawn_select_partial/", views.pawn_select_partial, name="pawn_select_partial"),
    #path("logs/", views.log_viewer, name="log_viewer"),
    #path("log_data/", views.get_log_data, name="get_log_data"),
    #path("counter_partial/", views.counter_partial, name="counter_partial"),

    


    # The logger has a separate endpoint, so we can stream log entries
    path("log_entries/", views.log_entries, name="log_entries"),
    path("log_levels/", views.log_levels, name="log_levels"),
    path("log_loggers/", views.log_loggers, name="log_loggers"),

    # Database file management
    path("scan_dataset_folder", views.scan_dataset_folder, name="scan_dataset_folder"),
    path("upload_datasets/", views.upload_datasets, name="upload_datasets"),
    #path("set_active/<int:file_id>/", views.set_active_file, name="set_active"),
    #path("close_file/<int:file_id>/", views.close_file, name="close_file"),
]