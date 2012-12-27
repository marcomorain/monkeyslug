# ./bsp2json/build/Release/bsp2json output/maps/start.bsp > start.bsp.json
# iconv -f UTF8 -t ASCII start.bsp.json | python entities2json.py > public/start.bsp.processed.json 
import sys
import json

data = sys.stdin.read()
data = json.loads(data)

def process_entities(raw_entities):
  entities = []
  entity = {}

  input = raw_entities.split("\n")
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
  return entities

data['entities'] = process_entities(data['entities'])

print(json.dumps(data, indent=2))

