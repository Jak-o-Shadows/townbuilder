from django.urls import path
from . import views

urlpatterns = [
    path('', views.chart_view, name='chart_view'),
    path('chart_data/', views.get_chart_data, name='get_chart_data'),
    path('pawn_names/', views.get_pawn_names, name='get_pawn_names'),
]