#include "km_load_obj.h"

#include "km_os.h"

union FaceIndices
{
    struct
    {
        int pos, uv;
    };
    int values[2];
};

bool StringToObjFaceInds(const_string str, FaceIndices* faceInds)
{
    int numElements;
    bool result = StringToElementArray(str, '/', false, StringToIntBase10, 2, faceInds->values, &numElements);
    if (numElements == 1) {
        faceInds->uv = -1;
    }
    return result && (numElements == 1 || numElements == 2);
}

bool LoadObj(const_string filePath, LoadObjResult* result, LinearAllocator* allocator)
{
    result->file = LoadEntireFile(filePath, allocator);
    if (result->file.data == nullptr) {
        return false;
    }

    DynamicArray<Vec3, LinearAllocator> positions(allocator);
    DynamicArray<Vec2, LinearAllocator> uvs(allocator);
    DynamicArray<ObjTriangle, LinearAllocator> triangles(allocator);
    DynamicArray<ObjQuad, LinearAllocator> quads(allocator);
    DynamicArray<uint32, LinearAllocator> startTriangleInds(allocator);
    DynamicArray<uint32, LinearAllocator> startQuadInds(allocator);


    string fileString = {
        .size = result->file.size,
        .data = (char*)result->file.data
    };
    string next;

    bool firstMesh = true;
    while (true) {
        next = NextSplitElement(&fileString, '\n');
        if (next.size > 0 && next[next.size - 1] == '\r') {
            next.size--;
        }
        if (fileString.size == 0 && next.size == 0) {
            break;
        }

        if (next.size < 2) continue;

        // Handle new model
        if (next[0] == 'o' && next[1] == ' ') {
            startTriangleInds.Append(triangles.size);
            startQuadInds.Append(quads.size);
            firstMesh = false;
        }
        // Handle new vertex position
        else if (next[0] == 'v' && next[1] == ' ') {
            if (firstMesh) {
                startTriangleInds.Append(triangles.size);
                startQuadInds.Append(quads.size);
                firstMesh = false;
            }

            next.data += 2;
            next.size -= 2;

            Vec3* p = positions.Append();
            int numElements;
            if (!StringToElementArray(next, ' ', false, StringToFloat32, 3, p->e, &numElements)) {
                LOG_ERROR("Failed to load vertex position with value: %.*s\n", (int)next.size, next.data);
                return false;
            }
            if (numElements != 3) {
                return false;
            }
        }
        // Handle new vertex UV
        else if (next.size > 2 && next[0] == 'v' && next[1] == 't' && next[2] == ' ') {
            if (firstMesh) {
                startTriangleInds.Append(triangles.size);
                startQuadInds.Append(quads.size);
                firstMesh = false;
            }

            next.data += 3;
            next.size -= 3;

            Vec2* uv = uvs.Append();
            int numElements;
            if (!StringToElementArray(next, ' ', false, StringToFloat32, 2, uv->e, &numElements)) {
                LOG_ERROR("Failed to load vertex UV with value: %.*s\n", (int)next.size, next.data);
                return false;
            }
            if (numElements != 2) {
                return false;
            }
        }
        // Handle new face
        else if (next[0] == 'f' && next[1] == ' ') {
            if (firstMesh) {
                startTriangleInds.Append(triangles.size);
                startQuadInds.Append(quads.size);
                firstMesh = false;
            }

            next.data += 2;
            next.size -= 2;

            FaceIndices indices[4];
            int numElements;
            if (!StringToElementArray(next, ' ', false, StringToObjFaceInds, 4, indices, &numElements)) {
                LOG_ERROR("Failed to load face with value: %.*s\n", (int)next.size, next.data);
                return false;
            }

            // NOTE obj files store faces in counter-clockwise order, but we want to return clockwise
            if (numElements == 3) {
                ObjTriangle* triangle = triangles.Append();
                triangle->v[0].pos = positions[indices[0].pos - 1];
                triangle->v[0].uv = indices[0].uv == -1 ? Vec2::zero : uvs[indices[0].uv - 1];
                triangle->v[1].pos = positions[indices[2].pos - 1];
                triangle->v[1].uv = indices[2].uv == -1 ? Vec2::zero : uvs[indices[2].uv - 1];
                triangle->v[2].pos = positions[indices[1].pos - 1];
                triangle->v[2].uv = indices[1].uv == -1 ? Vec2::zero : uvs[indices[1].uv - 1];

            }
            else if (numElements == 4) {
                ObjQuad* quad = quads.Append();
                quad->v[0].pos = positions[indices[0].pos - 1];
                quad->v[0].uv = indices[0].uv == -1 ? Vec2::zero : uvs[indices[0].uv - 1];
                quad->v[1].pos = positions[indices[3].pos - 1];
                quad->v[1].uv = indices[3].uv == -1 ? Vec2::zero : uvs[indices[3].uv - 1];
                quad->v[2].pos = positions[indices[2].pos - 1];
                quad->v[2].uv = indices[2].uv == -1 ? Vec2::zero : uvs[indices[2].uv - 1];
                quad->v[3].pos = positions[indices[1].pos - 1];
                quad->v[3].uv = indices[1].uv == -1 ? Vec2::zero : uvs[indices[1].uv - 1];
            }
            else {
                return false;
            }
        }
    }

    if (startTriangleInds.size != startQuadInds.size) {
        LOG_ERROR("Error in triangle/quad tracking when loading obj file %.*s\n", (int)filePath.size, filePath.data);
        return false;
    }

    result->models = allocator->NewArray<ObjModel>(startTriangleInds.size);
    for (uint32 i = 0; i < startTriangleInds.size; i++) {
        uint32 endTriangleInd = triangles.size;
        if (i != startTriangleInds.size - 1) {
            endTriangleInd = startTriangleInds[i + 1];
        }
        uint32 endQuadInd = quads.size;
        if (i != startTriangleInds.size - 1) {
            endQuadInd = startQuadInds[i + 1];
        }

        result->models[i].triangles = triangles.ToArray().Slice(startTriangleInds[i], endTriangleInd);
        result->models[i].quads = quads.ToArray().Slice(startQuadInds[i], endQuadInd);
    }

    return true;
}
