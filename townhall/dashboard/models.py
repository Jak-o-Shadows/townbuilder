from django.db import models

# Create your models here.

class DatasetModel(models.Model):
    filepath_db = models.CharField(max_length=255)
    game_id = models.CharField(max_length=100)