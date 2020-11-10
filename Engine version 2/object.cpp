#include "object.h"

int zip_verts_tris(object_info* object) {
    if (object->tris.size() % 3) return -1; //Return error due to incorrect size of tris list
    if (object->verts.size() % 3) return -2; //Return error due to incorrect size of verts list

    tri3d tri;
    int target = 0;
    for (size_t q = 0; q < floor(object->tris.size() / 3); q++) {
        target = object->tris[q * 3];
        tri.p1.x = object->verts[static_cast<__int64>(target) * 3];
        tri.p1.y = object->verts[static_cast<__int64>(target) * 3 + 1];
        tri.p1.z = object->verts[static_cast<__int64>(target) * 3 + 2];
        tri.p1.q = 1;

        target = object->tris[q * 3 + 1];
        tri.p2.x = object->verts[static_cast<__int64>(target) * 3];
        tri.p2.y = object->verts[static_cast<__int64>(target) * 3 + 1];
        tri.p2.z = object->verts[static_cast<__int64>(target) * 3 + 2];
        tri.p2.q = 1;

        target = object->tris[q * 3 + 2];
        tri.p3.x = object->verts[static_cast<__int64>(target) * 3];
        tri.p3.y = object->verts[static_cast<__int64>(target) * 3 + 1];
        tri.p3.z = object->verts[static_cast<__int64>(target) * 3 + 2];
        tri.p3.q = 1;
        object->model_mesh.push_back(tri);
    }
    object->set_zipped();
    return 0;
}