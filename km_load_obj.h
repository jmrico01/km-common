#pragma once

#include "km_array.h"
#include "km_math.h"
#include "km_memory.h"
#include "km_string.h"

struct ObjVertex
{
    Vec3 pos;
    Vec2 uv;
};

struct ObjTriangle
{
    ObjVertex v[3];
    uint32 materialIndex;
};

struct ObjQuad
{
    ObjVertex v[4];
    uint32 materialIndex;
};

struct ObjMaterial
{
    string name;
};

struct ObjModel
{
    string name;
    Array<ObjTriangle> triangles;
    Array<ObjQuad> quads;
};

struct LoadObjResult
{
    Array<uint8> file;
    Array<ObjModel> models;
    Array<ObjMaterial> materials;
};

bool LoadObj(const_string filePath, Vec3 offset, float32 scale, LoadObjResult* result, LinearAllocator* allocator);
