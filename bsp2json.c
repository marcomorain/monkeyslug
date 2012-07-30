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
    uint16_t plane_id;  // The plane in which the face lies
                        //           must be in [0,numplanes[ 
    uint16_t side;      // 0 if in front of the plane, 1 if behind the plane
    int ledge_id;       // first edge in the List of edges
                        //           must be in [0,numledges[
  uint16_t ledge_num;   // number of edges in the List of edges
  uint16_t texinfo_id;  // index of the Texture info the face is part of
                        //           must be in [0,numtexinfos[ 
  uint8_t typelight;    // type of lighting, for the face
  uint8_t baselight;    // from 0xFF (dark) to 0 (bright)
  uint8_t light[2];     // two additional light models  
  int lightmap;         // Pointer inside the general light map, or -1
                        // this define the start of the face light map
} face_t;


typedef struct
{
    float x;                    // X,Y,Z coordinates of the vertex
    float y;                    // usually some integer value
    float z;                    // but coded in floating point
} vertex_t;

typedef struct
{
    vertex_t normal;    // Vector orthogonal to plane (Nx,Ny,Nz)
                        // with Nx2+Ny2+Nz2 = 1
    float dist;         // Offset to plane, along the normal vector.
                        // Distance from (0,0,0) to the plane
    int type;           // Type of plane, depending on normal vector.
} plane_t;

static void planes_to_json(FILE* output, plane_t* planes, int num_planes)
{
    printf("Num planes: %d\n", num_planes);

    fprintf(output, "\"planes\" = [ ");

    for (int i=0; i<num_planes; i++)
    {
    }

    fprintf(output, "];\n");
}

static void faces_to_json(FILE* output, const face_t* faces, int num_faces)
{
    printf("Num faces: %d\n", num_faces);

    fprintf(output, "\"faces\" = [ ");

    for (int i=0; i<num_faces; i++)
    {
        const face_t* face = faces + i;
        fprintf(output, "  {\n");
        fprintf(output, "    plane_id:   %d,\n",       face->plane_id);
        fprintf(output, "    side:       %d,\n",           face->side);
        fprintf(output, "    ledge_id:   %d,\n",       face->ledge_id);
        fprintf(output, "    ledge_num:  %d,\n",      face->ledge_num);
        fprintf(output, "    texinfo_id: %d,\n",     face->texinfo_id);
        fprintf(output, "    typelight:  %d,\n",      face->typelight);
        fprintf(output, "    baselight:  %d,\n",      face->baselight);
        fprintf(output, "    light: [%d, %d],\n",    face->light[0], face->light[1]);
        fprintf(output, "    lightmap:   %d\n",       face->lightmap);
        fprintf(output, "  },\n");
    }

    fprintf(output, "];\n");    
}

static void vertices_to_json(FILE* output, const float* vertices, int num_vertices)
{
    printf("num vertices: %d\n", num_vertices);
    
    fprintf(output, "\"vertices\" = [ ");

    for (int i=0; i<num_vertices; i++)
    {
        fprintf(output, "%g", vertices[i]);
        if (i < (num_vertices - 1)) fprintf(output, ", ");
        if ((i%8)==0) fprintf(output, "\n");
    }
    fprintf(output, "];\n");
}
static void to_json(FILE* output, const char* file)
{
    char* data = read_entire_file(file);

    dheader_t* header = (dheader_t*)data;
    printf("Reading %s BSP version %x\n", file, header->version);
    
    const int num_vertices = header->vertices.size / sizeof(float);
    float* vertices = (float*)(data + header->vertices.offset);
    vertices_to_json(output, vertices, num_vertices - num_vertices);
    
    const int num_faces = header->faces.size / sizeof(face_t);
    face_t* faces = (face_t*)(data + header->faces.offset);
    faces_to_json(output, faces, num_faces);
    
    const int num_planes = header->planes.size / sizeof(plane_t);
    plane_t* planes = (plane_t*)(data + header->planes.offset);
    planes_to_json(output, planes, num_planes);
    
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