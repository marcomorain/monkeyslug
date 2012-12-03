#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <float.h>
#include <cassert>
#include "json/json.h"

using namespace std;

static void fatal(const char* message, ...)
{
    va_list argp;
    va_start(argp, message);
    fprintf(stderr, "Fatal error: ");
    vfprintf(stderr, message, argp);
    fputs("\n", stderr);
    va_end(argp);
    exit(EXIT_FAILURE);
}

static char* read_entire_file(const char* filename)
{
    FILE* input = fopen(filename, "rb");
    if (!input) fatal("Error reading %s\n", filename);
    fseek(input, 0, SEEK_END);
    long size = ftell(input);
    char* data = (char*)malloc(size);
    rewind(input);
    fread(data, size, 1, input);
    fclose(input);
    return data;
}

<<<<<<< HEAD
=======


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

>>>>>>> 25083b3ed21f9b0c0f0168ddab413e0fbad057e2
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
    int16_t plane_id;  // The plane in which the face lies
    //           must be in [0,numplanes[
    int16_t side;      // 0 if in front of the plane, 1 if behind the plane
    int ledge_id;       // first edge in the List of edges
    //           must be in [0,numledges[
    int16_t ledge_num; // number of edges in the List of edges
    int16_t texinfo_id;// index of the Texture info the face is part of
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

typedef struct                 // Bounding Box, Float values
{ vertex_t   min;                // minimum values of X,Y,Z
    vertex_t   max;                // maximum values of X,Y,Z
} boundbox_t;

typedef struct                 // Bounding Box, Short values
{
    int16_t   min[3];                 // minimum values of X,Y,Z
    int16_t   max[3];                 // maximum values of X,Y,Z
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

typedef struct
{
    boundbox_t bound;            // The bounding box of the Model
    vertex_t origin;               // origin of model, usually (0,0,0)
    int node_id0;               // index of first BSP node
    int node_id1;               // index of the first Clip node
    int node_id2;               // index of the second Clip node
    int node_id3;               // usually zero
    int numleafs;               // number of BSP leaves
    int face_id;                // index of Faces
    int face_num;               // number of Faces
} model_t;


typedef struct
{
    vertex_t vectorS;       // S vector, horizontal in texture space)
    float    distS;         // horizontal offset in texture space
    vertex_t vectorT;       // T vector, vertical in texture space
    float    distT;         // vertical offset in texture space
    uint32_t   texture_id;    // Index of Mip Texture
    //           must be in [0,numtex[
    uint32_t   animated;      // 0 for ordinary textures, 1 for water
} texinfo_t;

<<<<<<< HEAD
template <typename Type> Json::Value object_to_json(const Type& object)
{
    return Json::Value(Json::nullValue);
}

template <typename Type> class Array
{
private:
    const int   m_count;
    const Type* m_data;
    
public:
    
    Array(const char* base, const dentry_t& entry)
    : m_count(entry.size/sizeof(Type))
    , m_data((Type*)(base + entry.offset))
    {
    }
    
    const Type& operator[](int i)
    {
        assert(i>0);
        assert(i<m_count);
        return m_data[i];
    }
    
    int count(void) const
    {
        return this->count;
    }
    
    Json::Value to_json(void)
    {
        Json::Value result(Json::arrayValue);
        for (int i=0; i<m_count; i++)
        {
            result.append(object_to_json(m_data[i]));
        }
        return result;
    }
};


=======
static face_t*      _faces          = NULL;
static int          _num_faces      = 0;
static int32_t*     list_edges      = NULL;
static int          num_list_edges  = 0;
static plane_t*     planes          = NULL;
static int          num_planes      = 0;
static miptex_t*    miptextures     = NULL;
static int          num_miptextures = 0;
static node_t*      nodes           = NULL;
static int          num_nodes       = 0;
static model_t*     models          = NULL;
static int          num_models      = 0;
static uint8_t*     lightmaps       = NULL;
static int          num_lightmaps   = 0;
static texinfo_t*   _texinfos       = NULL;
static int          _num_texinfos   = 0;
static char*        entities        = NULL;
static int          num_entities    = 0;

static face_t* get_face(int index)
{
    if (index >= _num_faces) fatal("Invalid face index %d", index);
    if (index < 0) fatal("Negative face index");
    return _faces + index;
}

static texinfo_t* get_texinfo(int index)
{
    //printf("Get texture [%ld] %d of %d\n", sizeof(texinfo_t), index, _num_texinfos);
    if (index >= _num_texinfos) fatal("Invalid texinfo index %d\n", index);
    if (index < 0) fatal("negative texinfo index %d", index);
    return &_texinfos[index];
}
void set_min(vertex_t* out, vertex_t* a, vertex_t* b)
{
    out->x = MIN(a->x, b->x);
    out->y = MIN(a->y, b->y);
    out->z = MIN(a->z, b->z);
}

void set_max(vertex_t* out, vertex_t* a, vertex_t* b)
{
    out->x = MAX(a->x, b->x);
    out->y = MAX(a->y, b->y);
    out->z = MAX(a->z, b->z);
}


float dotproduct(vertex_t a, vertex_t b)
{
    float result = 0;
    result += a.x * b.x;
    result += a.y * b.y;
    result += a.z * b.z;
    return result;
}

static void node_to_json(int node_id, int* index_base, Json::Value& json);

static void node_leaf_index_to_json(int index, int* index_base, Json::Value& json)
{
    const static unsigned leaf_mask = 0x8000;
    if (index & leaf_mask) return;
    if (index == 0) return;
    node_to_json(index, index_base, json);
}

static void node_to_json(int node_id, int* index_base, Json::Value& json)
{
    printf("Processing node %d\n", node_id);
    
    node_t* node = nodes + node_id;
    
    //printf("Node: plane %08x faces: %d first: %08x front %08x back %08x\n",
    //node->plane_id, node->face_num, node->face_id, node->front, node->back);
    
    node_leaf_index_to_json(node->front, index_base, json);
    
    for (int i = 0; i< node->face_num; i++)
    {
        //face_to_json(node->face_id + i, index_base, json);
    }
    
    node_leaf_index_to_json(node->back, index_base, json);
}

static void nodes_to_json(Json::Value& json)
{
    json["vertices"] = Json::Value(Json::arrayValue);
    json["indices"]  = Json::Value(Json::arrayValue);
    
    printf("Model[0] origin: %g %g %g\n",
           models[0].origin.x,
           models[0].origin.y,
           models[0].origin.z);
    
    int bsp_root = models[0].node_id0;\
    
    int index_base = 0;
    
    node_to_json(bsp_root, &index_base, json);
}


static Json::Value vertex_to_json(vertex_t& v)
{
    Json::Value result;
    result["x"] = v.x;
    result["y"] = v.y;
    result["z"] = v.z;
    return result;
}

static Json::Value edge_to_json(edge_t& e)
{
    Json::Value result;
    result["vertex0"] = e.vertex0;
    result["vertex1"] = e.vertex1;
    return result;
}

>>>>>>> 25083b3ed21f9b0c0f0168ddab413e0fbad057e2
static void to_json(const char* file)
{
    Json::Value json;
    
    char* data = read_entire_file(file);
    
    dheader_t* header = (dheader_t*)data;
    
    json["version"] = header->version;
<<<<<<< HEAD
    
    Array<vertex_t> vertices(data, header->vertices);
    json["vertices"] = vertices.to_json();
    
    Array<edge_t> edges(data, header->edges);
    json["edges"] = edges.to_json();
    
    Array<uint16_t> ledges(data, header->ledges);
    json["ledges"] = ledges.to_json();
    
    Array<plane_t> planes(data, header->planes);
    json["planes"] = planes.to_json();
    
    Array<face_t> faces(data, header->faces);
    json["faces"] = faces.to_json();
    
    Array<node_t> nodes(data, header->nodes);
    json["nodes"] = nodes.to_json();
    
    Array<model_t> models(data, header->models);
    json["models"] = models.to_json();
    
    Array<miptex_t> miptex(data, header->miptex);
    json["miptex"] = miptex.to_json();
    
    Array<texinfo_t> texinfos(data, header->texinfo);
    json["texinfo"] = texinfos.to_json();
    
    Array<uint8_t> lightmaps(data, header->lightmaps);
    json["lightmaps"] = lightmaps.to_json();
    
    Array<char> entities(data, header->entities);
    json["entities"] = entities.to_json();
=======

    int num_vertices = header->vertices.size / sizeof(vertex_t);
    vertex_t* vertices = (vertex_t*)(data + header->vertices.offset);
    Json::Value jsonVertices = Json::Value(Json::arrayValue);
    for (int i=0; i<num_vertices; i++)
    {
        jsonVertices.append(vertex_to_json(vertices[i]));
    }
    json["vertices"] = jsonVertices;
    
    
    int num_edges = header->edges.size / sizeof(edge_t);
    edge_t* edges = (edge_t*)(data + header->edges.offset);
    Json::Value jEdges(Json::arrayValue);
    for (int i=0; i<num_edges; i++)
    {
        jEdges.append(edge_to_json(edges[i]));
    }
    json["edges"] = jEdges;
    
    num_list_edges = header->ledges.size / sizeof(uint16_t);
    list_edges = (int32_t*)(data + header->ledges.offset);
    
    num_planes = header->planes.size / sizeof(plane_t);
    planes = (plane_t*)(data + header->planes.offset);
    
    _num_faces = header->faces.size / sizeof(face_t);
    _faces = (face_t*)(data + header->faces.offset);
    
    num_nodes = header->nodes.size / sizeof(node_t);
    nodes = (node_t*)(data + header->nodes.offset);
    
    num_models = header->models.size / sizeof(model_t);
    models = (model_t*)(data + header->models.offset);
    
    // Bug here? wrong size of miptex struct?
    num_miptextures = header->miptex.size / sizeof(miptex_t);
    miptextures = (miptex_t*)(data + header->miptex.offset);
    
    for (int i=0; i<num_miptextures; i++)
    {
        char name_data[17] = {};
        strncpy(name_data, miptextures[i].name, 16);
        //printf("Texture: %s\n", name_data);
    }
    
    _num_texinfos = header->texinfo.size / sizeof(texinfo_t);
    _texinfos = (texinfo_t*)(data + header->texinfo.offset);
    
    num_lightmaps = header->lightmaps.size / sizeof(uint8_t);
    lightmaps = (uint8_t*)(data + header->lightmaps.offset);
    
    num_entities = header->entities.size / sizeof(char);
    entities = (char*)(data + header->entities.offset);
>>>>>>> 25083b3ed21f9b0c0f0168ddab413e0fbad057e2
    
    //nodes_to_json(json);
    
    //textures_to_json();
    
    cout << json;
    
    free(data);
}

<<<<<<< HEAD
#define FIELD_STR(s) FIELD_STR_(s)
#define FIELD_STR_(s) #s
#define ASSIGN_FIELD(field) result[FIELD_STR((field))] = object.field

Json::Value object_to_json(const vertex_t& object)
{
    Json::Value result;
    ASSIGN_FIELD(x);
    ASSIGN_FIELD(y);
    ASSIGN_FIELD(z);
    return result;
}

Json::Value object_to_json(const edge_t object)
{
    Json::Value result;
    ASSIGN_FIELD(vertex0);
    ASSIGN_FIELD(vertex1);
    return result;
}

    int16_t plane_id;  // The plane in which the face lies
    //           must be in [0,numplanes[
    int16_t side;      // 0 if in front of the plane, 1 if behind the plane
    int ledge_id;       // first edge in the List of edges
    //           must be in [0,numledges[
    int16_t ledge_num; // number of edges in the List of edges
    int16_t texinfo_id;// index of the Texture info the face is part of
    //           must be in [0,numtexinfos[
    uint8_t typelight;  // type of lighting, for the face
    uint8_t baselight;  // from 0xFF (dark) to 0 (bright)
    uint8_t light[2];   // two additional light models
    int lightmap;       // Pointer inside the general light map, or -1

Json::Value object_to_json(const face_t object)
{
    Json::Value result;
    ASSIGN_FIELD(plane_id);
    ASSIGN_FIELD(side);
    ASSIGN_FIELD(ledge_id);
    ASSIGN_FIELD(ledge_num);
    ASSIGN_FIELD(texinfo_id);
    ASSIGN_FIELD(typelight);
    ASSIGN_FIELD(baselight);
    
    Json::Value lights(Json::arrayValue);
    lights.append(object.light[0]);
    lights.append(object.light[1]);
    result["lights"] = lights;

    ASSIGN_FIELD(lightmap);

    return result;
}

=======
>>>>>>> 25083b3ed21f9b0c0f0168ddab413e0fbad057e2
int main(int argc, char** argv)
{
    if (argc < 2) fatal("Usage: %s <filename.bsp>\n", argv[0]);
    for (int i=1; i<argc; i++) to_json(argv[i]);
    return EXIT_SUCCESS;
}
<<<<<<< HEAD


=======
>>>>>>> 25083b3ed21f9b0c0f0168ddab413e0fbad057e2
