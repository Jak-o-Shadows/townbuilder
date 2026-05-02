from django.db import models
import os
import sqlite3
from django.utils import timezone

# Create your models here.

class DatasetFileModel(models.Model):
    filepath = models.CharField(max_length=500, unique=True)  # Absolute path to the DB file
    label = models.CharField(max_length=100, blank=True, null=True)  # Friendly name
    is_open = models.BooleanField(default=True)
    last_seen = models.DateTimeField(default=timezone.now)
    created_at = models.DateTimeField(auto_now_add=True)

    class Meta:
        db_table = 'townhall_dataset_files'
        ordering = ['-last_seen']

    def __str__(self):
        return self.display_label

    @property
    def exists(self):
        return os.path.exists(self.filepath)

    @property
    def metadata(self):
        if not self.exists:
            return {}
        try:
            with sqlite3.connect(self.filepath) as con:
                cur = con.cursor()
                cur.execute("SELECT key, value FROM dataset_metadata")
                return dict(cur.fetchall())
        except sqlite3.DatabaseError:
            return {}

    @property
    def dataset_name(self):
        return self.metadata.get('dataset_name')

    @property
    def metadata_created_at(self):
        return self.metadata.get('created_at')

    @property
    def metadata_uuid(self):
        return self.metadata.get('dataset_uuid')

    @property
    def display_label(self):
        if self.label:
            return self.label
        if self.dataset_name:
            return self.dataset_name
        if self.metadata_created_at:
            return f"{os.path.basename(self.filepath)} ({self.metadata_created_at})"
        return os.path.basename(self.filepath)
