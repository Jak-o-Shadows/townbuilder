
from django import forms

class DatasetFolderBrowseForm(forms.Form):
    folder_path = forms.CharField(label='Dataset Folder Path',
                                  widget=forms.TextInput(attrs={
                                      'placeholder': 'Enter dataset folder path'
                                    })
                                  )
    
class DatasetUploadForm(forms.Form):
    dataset_file = forms.FileField(label='Upload Dataset File',
                                  widget=forms.ClearableFileInput(attrs={
                                      'accept': '.db',
                                      'class': 'dataset-upload-input'
                                    })
                                  )
    