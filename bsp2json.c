#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>


#include "utils.c"

typedef struct                 // A Directory entry
{
    int  offset;                // Offset to entry, in bytes, from start of file
    int  size;                  // Size of entry in file, in bytes
} dentry_t;


typedef struct                 // The BSP file header
{ int  version;               // Model version, must be 0x17 (23).
  dentry_t entities;           // List of Entities.
  dentry_t planes;             // Map Planes.
                               // numplanes = size/sizeof(plane_t)
  dentry_t miptex;             // Wall Textures.
  dentry_t vertices;           // Map Vertices.
                               // numvertices = size/sizeof(vertex_t)
  dentry_t visilist;           // Leaves Visibility lists.
  dentry_t nodes;              // BSP Nodes.
                               // numnodes = size/sizeof(node_t)
  dentry_t texinfo;            // Texture Info for faces.
                               // numtexinfo = size/sizeof(texinfo_t)
  dentry_t faces;              // Faces of each surface.
                               // numfaces = size/sizeof(face_t)
  dentry_t lightmaps;          // Wall Light Maps.
  dentry_t clipnodes;          // clip nodes, for Models.
                               // numclips = size/sizeof(clipnode_t)
  dentry_t leaves;             // BSP Leaves.
                               // numlaves = size/sizeof(leaf_t)
  dentry_t lface;              // List of Faces.
  dentry_t edges;              // Edges of faces.
                               // numedges = Size/sizeof(edge_t)
  dentry_t ledges;             // List of Edges.
  dentry_t models;             // List of Models.
                               // nummodels = Size/sizeof(model_t)
} dheader_t;

typedef struct
{
    float x;                    // X,Y,Z coordinates of the vertex
    float y;                    // usually some integer value
    float z;                    // but coded in floating point
} vertex_t;

static void vertex_to_json(FILE* output, vertex_t* vertex)
{
    fprintf(output, "  { x: %g y: %g z: %g }",
        vertex->x,
        vertex->y,
        vertex->z);
}

static void to_json(FILE* output, const char* file)
{
    char* data = read_entire_file(file);

    dheader_t* header = (dheader_t*)data;
    printf("Reading %s BSP version %x\n", file, header->version);
    
    const int num_vertices = (header->vertices.size / sizeof(vertex_t));
    vertex_t* vertices = (vertex_t*)(data + header->vertices.offset);
    
    printf("num vertices: %d\n", num_vertices);
    
    fprintf(output, "{ \"vertices\" = [ ");

    for (int i=0; i<num_vertices; i++)
    {
        vertex_to_json(output, vertices + i);
        if (i < (num_vertices - 1)) fprintf(output, ", ");
        if ((i%3)==0) fprintf(output, "\n");
    }
    fprintf(output, "]\n}\n");  
    free(data);
}

int main(int argc, char** argv)
{
    if (argc < 2) fatal("Usage: %s <filename.bsp>\n", argv[0]);
    

    for (int i=1; i<argc; i++)
    {
        char* output_file_name;
        int error = asprintf(&output_file_name, "%s.json", argv[i]);
        if (error < 0) fatal("Unable to compute output filename");
        puts(output_file_name);
        FILE* output = fopen(output_file_name, "w");
        if(!output) fatal("Error opening %s", output_file_name);
        to_json(output, argv[i]);
    }
    
    return EXIT_SUCCESS;
}