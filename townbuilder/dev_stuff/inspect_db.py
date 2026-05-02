import os
import sqlite3

filepath_db = os.path.join("../build/Release/database.db")
con = sqlite3.connect(filepath_db)
cur = con.cursor()

cur.execute("SELECT name FROM sqlite_master WHERE type='table';")
tables = cur.fetchall()

for table_name in tables:
    table_name = table_name[0]  # Why?
    print(f"Table: {table_name}")
    cur.execute(f"PRAGMA table_info({table_name});")
    table_info = cur.fetchall()
    for column_info in table_info:
        column_name = column_info[1]
        data_type = column_info[2]
        print(f"  Column: {column_name} ({data_type})")

con.close()