from django.db import models
import os
from django.utils import timezone

# Create your models here.

class DatasetFileModel(models.Model):
    filepath = models.CharField(max_length=500, unique=True)  # Absolute path to the DB file
    label = models.CharField(max_length=100, blank=True, null=True)  # Friendly name
    is_open = models.BooleanField(default=True)
    last_seen = models.DateTimeField(default=timezone.now)
    created_at = models.DateTimeField(auto_now_add=True)

    class Meta:
        ordering = ['-last_seen']

    def __str__(self):
        return self.label or os.path.basename(self.filepath)

    @property
    def exists(self):
        return os.path.exists(self.filepath)

    @property
    def status(self):
        if self.exists:
            return 'ok'
        return 'missing'
