make bsp2json && ./bsp2json output/maps/$1.bsp && \
cp output/maps/*.vertices.json public && \
cp output/maps/*.indices.json  public && \
python entities2json.py output/maps/$1.bsp.entities.json > public/$1.bsp.entities.json
