import os
import sys

def main():
    os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'townhall.config.settings')
    from django.core.management import execute_from_command_line
    execute_from_command_line(sys.argv)

def run():
    os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'townhall.config.settings')
    from django.core.management import execute_from_command_line
    execute_from_command_line(["manage.py", "runserver"])

if __name__ == '__main__':
    main()