import sys
import json

input = open(sys.argv[1]).readlines()
input.reverse()

entities = []
entity = {}

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

print(json.dumps(entities, indent=2))

