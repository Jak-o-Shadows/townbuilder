from django.db import models
import os
from django.utils import timezone

# Create your models here.

class DatasetModel(models.Model):
    filepath_db = models.CharField(max_length=255)
    game_id = models.CharField(max_length=100)


class InputDatabaseFile(models.Model):
    path = models.CharField(max_length=500, unique=True)  # Absolute path to the DB file
    label = models.CharField(max_length=100, blank=True, null=True)  # Friendly name
    is_open = models.BooleanField(default=True)
    is_active = models.BooleanField(default=False)  # Only one should be True at a time
    is_temporary = models.BooleanField(default=False)  # Mark if from townbuilder temp output
    last_seen = models.DateTimeField(default=timezone.now)
    created_at = models.DateTimeField(auto_now_add=True)

    class Meta:
        ordering = ['-last_seen']

    def __str__(self):
        return self.label or os.path.basename(self.path)

    @property
    def exists(self):
        return os.path.exists(self.path)

    @property
    def status(self):
        if self.exists:
            return 'ok'
        return 'missing'

    def save(self, *args, **kwargs):
        # Ensure only one active file
        if self.is_active:
            InputDatabaseFile.objects.filter(is_active=True).exclude(pk=self.pk).update(is_active=False)
        super().save(*args, **kwargs)