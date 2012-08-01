#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>


#include "utils.c"

FILE* create_output_file(const char* base, const char* description)
{
    char* filename;
    int error = asprintf(&filename, "%s.%s.json", base, description);
    if (error < 0) fatal("Unable to compute output filename");
    puts(filename);
    FILE* result = fopen(filename, "w");
    if(!result) fatal("Error opening %s", filename);
    return result;
}

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
    uint16_t ledge_num; // number of edges in the List of edges
    uint16_t texinfo_id;// index of the Texture info the face is part of
                        //           must be in [0,numtexinfos[ 
    uint8_t typelight;  // type of lighting, for the face
    uint8_t baselight;  // from 0xFF (dark) to 0 (bright)
    uint8_t light[2];   // two additional light models  
    int lightmap;       // Pointer inside the general light map, or -1
                        // this define the start of the face light map
} face_t;


typedef struct
{
    float x;                    // X,Y,Z coordinates of the vertex
    float y;                    // usually some integer value
    float z;                    // but coded in floating point
} vertex_t;

typedef struct                 // Bounding Box, Short values
{
    int16_t   min;                 // minimum values of X,Y,Z
    int16_t   max;                 // maximum values of X,Y,Z
} bboxshort_t;

typedef struct
{
    vertex_t normal;    // Vector orthogonal to plane (Nx,Ny,Nz)
                        // with Nx2+Ny2+Nz2 = 1
    float dist;         // Offset to plane, along the normal vector.
                        // Distance from (0,0,0) to the plane
    int type;           // Type of plane, depending on normal vector.
} plane_t;

typedef struct
{
    uint16_t vertex0;   // index of the start vertex
                        //  must be in [0,numvertices[
    uint16_t vertex1;   // index of the end vertex
                        //  must be in [0,numvertices[
} edge_t;

typedef struct
{
    int plane_id;       // The plane that splits the node
                        //           must be in [0,numplanes[
    uint16_t front;     // If bit15==0, index of Front child node
                        // If bit15==1, ~front = index of child leaf
    uint16_t back;      // If bit15==0, id of Back child node
                        // If bit15==1, ~back =  id of child leaf
    bboxshort_t box;    // Bounding box of node and all childs
    uint16_t face_id;   // Index of first Polygons in the node
    uint16_t face_num;   // Number of faces in the node
} node_t;

typedef struct
{ int type;                   // Special type of leaf
  int vislist;                // Beginning of visibility lists
                               //     must be -1 or in [0,numvislist[
  bboxshort_t bound;           // Bounding box of the leaf
  uint16_t lface_id;            // First item of the list of faces
                               //     must be in [0,numlfaces[
  uint16_t lface_num;           // Number of faces in the leaf  
  uint8_t sndwater;             // level of the four ambient sounds:
  uint8_t sndsky;               //   0    is no sound
  uint8_t sndslime;             //   0xFF is maximum volume
  uint8_t sndlava;              //
} dleaf_t;


typedef struct                 // Mip Texture
{ char   name[16];             // Name of the texture.
  uint32_t width;                // width of picture, must be a multiple of 8
  uint32_t height;               // height of picture, must be a multiple of 8
  uint32_t offset1;              // offset to u_char Pix[width   * height]
  uint32_t offset2;              // offset to u_char Pix[width/2 * height/2]
  uint32_t offset4;              // offset to u_char Pix[width/4 * height/4]
  uint32_t offset8;              // offset to u_char Pix[width/8 * height/8]
} miptex_t;

static float*       vertices        = NULL;
static int          num_vertices    = 0;
static face_t*      faces           = NULL;
static int          num_faces       = 0;
static edge_t*      edges           = NULL;
static int          num_edges       = 0;
static int32_t*     list_edges      = NULL;
static int          num_list_edges  = 0;
static plane_t*     planes          = NULL;
static int          num_planes      = 0;
static miptex_t*    miptextures     = NULL;
static int          num_miptextures = 0;
static node_t*      nodes           = NULL;
static int          num_nodes       = 0;

static edge_t* get_edge(int index)
{
    if (index >= num_edges) fatal("Invalid edge index %d", index);
    if (index < 0) fatal("Negative edge index");
    return edges + index;
}

static void face_to_json(const face_t* face, int* index_base, FILE* vertices_out, FILE* indices_out)
{
    vertex_t* verts = (vertex_t*)vertices;    
    int32_t* first_edge = list_edges +  face->ledge_id;
    
    plane_t* plane = planes + face->plane_id;
    
    int last_face = 0; // to: recompute this

    for (int e=0; e<face->ledge_num; e++)
    {
        int32_t edge_index = first_edge[e];
        int v0, v1;
        if (edge_index > 0)
        {
            edge_t* edge = get_edge(edge_index);
            v0 = edge->vertex0;
            v1 = edge->vertex1;
        }
        else // swap winding
        {
            edge_t* edge = get_edge(-edge_index);
            v0 = edge->vertex1;
            v1 = edge->vertex0;
        }
        
        int last_vertex = (e == face->ledge_num - 1) && last_face;

        fprintf(vertices_out, "%g, %g, %g, %g, %g, %g%c \n",
            verts[v0].x,
            verts[v0].y,
            verts[v0].z,
            plane->normal.x,
            plane->normal.y,
            plane->normal.z,
            last_vertex ? ' ' : ',');

    }
    
    int base = *index_base;
    
    for (int f=1; f < face->ledge_num - 1; f++)
    {
        int last_index = (f == face->ledge_num - 2) && last_face;
        fprintf(indices_out, "%d, %d, %d%c \n",
            base,
            base + f,
            base + f + 1,
            last_index ? ' ' : ',');
    }
    (*index_base) = base + face->ledge_num;
    
}

static void faces_to_json(FILE* vertices_out, FILE* indices_out)
{
    printf("Num faces: %d\n", num_faces);

    fprintf(vertices_out, "{ \"vertices\" : [ ");
    fprintf(indices_out,  "{ \"indices\"  : [ ");

    int index_base = 0;

    for (int i=0; i<num_faces; i++)
    {
        const face_t* face = faces + i;
        
        face_to_json(face, &index_base, vertices_out, indices_out);
    }
  
    fprintf(vertices_out, "] }\n");
    fprintf(indices_out,  "] }\n");    
}
/*
static void textures_to_json()
{
    for (int i=0; i<num_miptextures; i++)
    {
        miptex_t* texture = miptextures + i;
        printf("%c\n", texture->name[0]);
    }
}
*/

static void to_json(const char* file)
{
    char* data = read_entire_file(file);

    dheader_t* header = (dheader_t*)data;
    printf("Reading %s BSP version %d\n", file, header->version);
    
    num_vertices = header->vertices.size / sizeof(float);
    vertices = (float*)(data + header->vertices.offset);

    num_edges = header->edges.size / sizeof(edge_t);
    edges = (edge_t*)(data + header->edges.offset);
    
    num_list_edges = header->ledges.size / sizeof(uint16_t);
    list_edges = (int32_t*)(data + header->ledges.offset);
    
    num_planes = header->planes.size / sizeof(plane_t);
    planes = (plane_t*)(data + header->planes.offset);

    num_faces = header->faces.size / sizeof(face_t);
    faces = (face_t*)(data + header->faces.offset);
    
    num_nodes = header->nodes.size / sizeof(node_t);
    nodes = (node_t*)(data + header->nodes.offset);
    
    // Bug here? wrong size of miptex struct?
    num_miptextures = header->miptex.size / sizeof(miptex_t);
    miptextures = (miptex_t*)data + header->miptex.offset;

    FILE* vertices_out = create_output_file(file, "vertices");
    FILE* indices_out  = create_output_file(file, "indices");
    faces_to_json(vertices_out, indices_out);
    fclose(vertices_out);
    fclose(indices_out);

    //textures_to_json();

    free(data);
}

int main(int argc, char** argv)
{
    if (argc < 2) fatal("Usage: %s <filename.bsp>\n", argv[0]);
    for (int i=1; i<argc; i++) to_json(argv[i]);
    return EXIT_SUCCESS;
}