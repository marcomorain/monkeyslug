#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <float.h>
#include <string>
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


#define FIELD_STR(s) FIELD_STR_(s)
#define FIELD_STR_(s) #s
#define ASSIGN_FIELD(field) result[FIELD_STR(field)] = object.field

Json::Value object_to_json(uint16_t object)
{
    Json::Value result = object;
    return result;
}

Json::Value object_to_json(const bboxshort_t& object)
{
    Json::Value result;
    
    Json::Value minArray(Json::arrayValue);
    Json::Value maxArray(Json::arrayValue);
    for (int i=0; i<3; i++)
    {
        minArray.append(object.min[i]);
        maxArray.append(object.max[i]);
    }
    result["max"] = maxArray;
    result["min"] = minArray;
    return result;
}

Json::Value object_to_json(const vertex_t& object)
{
    Json::Value result(Json::arrayValue);
    result.append(object.x);
    result.append(object.y);
    result.append(object.z);
    return result;
}

Json::Value object_to_json(const boundbox_t& object)
{
    Json::Value result;
    result["min"] = object_to_json(object.min);
    result["max"] = object_to_json(object.max);
    return result;
}

Json::Value object_to_json(const plane_t& object)
{
    Json::Value result;
    ASSIGN_FIELD(dist);
    ASSIGN_FIELD(type);
    result["normal"] = object_to_json(object.normal);
    return result;
}

Json::Value object_to_json(const texinfo_t& object)
{
    Json::Value result;
    ASSIGN_FIELD(animated);
    ASSIGN_FIELD(distS);
    ASSIGN_FIELD(distT);
    ASSIGN_FIELD(texture_id);
    result["vectorS"] = object_to_json(object.vectorS);
    result["vectorT"] = object_to_json(object.vectorT);
    return result;
}

Json::Value object_to_json(const edge_t& object)
{
    Json::Value result(Json::arrayValue);
    result.append(object.vertex0);
    result.append(object.vertex1);
    return result;
}

Json::Value object_to_json(const model_t& object)
{
    Json::Value result;
    result["bound"] = object_to_json(object.bound);
    ASSIGN_FIELD(face_id);
    ASSIGN_FIELD(face_num);
    ASSIGN_FIELD(node_id0);
    ASSIGN_FIELD(node_id1);
    ASSIGN_FIELD(node_id2);
    ASSIGN_FIELD(node_id3);
    ASSIGN_FIELD(numleafs);
    result["origin"] = object_to_json(object.origin);
    return result;
}

Json::Value object_to_json(const face_t& object)
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

Json::Value object_to_json(const node_t& object)
{
    Json::Value result;
    ASSIGN_FIELD(back);
    result["box"] = object_to_json(object.box);
    ASSIGN_FIELD(face_id);
    ASSIGN_FIELD(face_num);
    ASSIGN_FIELD(front);
    ASSIGN_FIELD(plane_id);
    return result;
}

 char   name[16];             // Name of the texture.
    uint32_t width;                // width of picture, must be a multiple of 8
    uint32_t height;               // height of picture, must be a multiple of 8
    uint32_t offset1;              // offset to u_char Pix[width   * height]
    uint32_t offset2;              // offset to u_char Pix[width/2 * height/2]
    uint32_t offset4;              // offset to u_char Pix[width/4 * height/4]
    uint32_t offset8;              // offset to u_char Pix[width/8 * height/8]

Json::Value object_to_json(const miptex_t& object)
{
    Json::Value result;
    
    std::string name;
    for (int i=0; i<16; i++)
    {
        name.push_back(object.name[i]);
    }
    result["name"] = name;
    ASSIGN_FIELD(width);
    ASSIGN_FIELD(height);
    ASSIGN_FIELD(offset1);
    ASSIGN_FIELD(offset2);
    ASSIGN_FIELD(offset4);
    ASSIGN_FIELD(offset8);
    return result;
}

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
    
    std::string to_string(void)
    {
        return std::string(this->m_data, this->m_count);
    }
};

static void to_json(const char* file)
{
    Json::Value json;
    
    char* data = read_entire_file(file);
    
    dheader_t* header = (dheader_t*)data;
    
    json["version"] = header->version;
    
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
    
    //Array<uint8_t> lightmaps(data, header->lightmaps);
    //json["lightmaps"] = lightmaps.to_json();
    
    Array<char> entities(data, header->entities);
    json["entities"] = entities.to_string();
    
    cout << json;
    
    free(data);
}

int main(int argc, char** argv)
{
    if (argc < 2) fatal("Usage: %s <filename.bsp>\n", argv[0]);
    for (int i=1; i<argc; i++) to_json(argv[i]);
    return EXIT_SUCCESS;
}

