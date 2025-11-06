from django.urls import path
app_name = "dashboard"
from . import views

urlpatterns = [
    path('', views.chart_view, name='chart_view'),
    path('chart_data/', views.get_chart_data, name='get_chart_data'),
    path('chart_partial/', views.chart_partial, name='chart_partial'),
    path('set_db_path/', views.set_db_path, name='set_db_path'),
    path('pawn_select_partial/', views.pawn_select_partial, name='pawn_select_partial'),
    path('pawn_names/', views.get_pawn_names, name='get_pawn_names'),
    path('logs/', views.log_viewer, name='log_viewer'),
    path('log_data/', views.get_log_data, name='get_log_data'),
    path('counter_partial/', views.counter_partial, name='counter_partial'),
]