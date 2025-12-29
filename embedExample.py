import sys
import pprint

print(f"My Script: {__file__}")

sys.path.append("build/src/pythonEcsBinding/Release")
import pythonEcsBinding

pprint.pprint(dir(pythonEcsBinding))

