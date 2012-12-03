# iconv -f UTF8 -t ASCII start.bsp.json | python entities2json.py > start.bsp.processed.json 
import sys
import json


data = sys.stdin.read()
data = json.loads(data)

entities = []
entity = {}

input = data['entities'].split("\n")
input.reverse()

while len(input) > 1:
  item = input.pop().strip()
  #print(item)
  if item == "{":
    pass
  elif item == "}":
    entities.append(entity)
    entity = {}
  else:
    tokens = item.split('"')
    entity[tokens[1]] = tokens[3]

data['entities'] = entities

print(json.dumps(data, indent=2))

