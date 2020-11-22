#include "object.h"

int zip_verts_tris(object_info* object) {
    if (object->tris.size() % 3) return -1; //Return error due to incorrect size of tris list
    if (object->verts.size() % 3) return -2; //Return error due to incorrect size of verts list
    bool textured = object->has_texture;
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
        if (textured) {
            target = object->ttris[q * 3];
            tri.tx.x = object->tverts[target * 2];
            tri.ty.x = 1 - object->tverts[target * 2 + 1];

            target = object->ttris[q * 3 + 1];
            tri.tx.y = object->tverts[target * 2];
            tri.ty.y = 1 - object->tverts[target * 2 + 1];

            target = object->ttris[q * 3 + 2];
            tri.tx.q = object->tverts[target * 2];
            tri.ty.q = 1 - object->tverts[target * 2 + 1];
        }
        object->model_mesh.push_back(tri);
    }
    object->set_zipped();
    return 0;
}

int load_from_obj(object_info* object, std::string file_name, int type) {
    std::ifstream file(file_name);
    if (!file.is_open()) std::cout << "Could not open file: " << file_name << '\n';
    std::string line;
    std::vector <std::string> sub_line;
    std::vector <std::vector<std::string>> sub_sub_line;
    object->has_texture = true;
    switch (type) {
    case 0:
        while (getline(file, line)) {
            if (line[0] == 'v' && line[1] == 't') {
                sub_line = split(line, ' ');
                object->tverts.push_back(stof(sub_line[1]));
                object->tverts.push_back(stof(sub_line[2]));
            }
            else if (line[0] == 'v' && line[1] == 'n') {
            }
            else if (line[0] == 'v') {
                sub_line = split(line, ' ');
                object->verts.push_back(stof(sub_line[1]));
                object->verts.push_back(stof(sub_line[2]));
                object->verts.push_back(stof(sub_line[3]));
            }
            else if (line[0] == 'f') {
                sub_line = split(line, ' ');
                sub_sub_line = split(sub_line, '/');
                object->tris.push_back(stoi(sub_sub_line[1][0]) - 1);
                object->tris.push_back(stoi(sub_sub_line[2][0]) - 1);
                object->tris.push_back(stoi(sub_sub_line[3][0]) - 1);

                object->ttris.push_back(stoi(sub_sub_line[1][1]) - 1);
                object->ttris.push_back(stoi(sub_sub_line[2][1]) - 1);
                object->ttris.push_back(stoi(sub_sub_line[3][1]) - 1);

            }
        }
        break;
     case 1:
         object->ttris = {};
         object->tverts = {};
         object->has_texture = false;
         while (getline(file, line)) {
             if (line[0] == 'v') {
                 sub_line = split(line, ' ');
                 object->verts.push_back(stof(sub_line[1]));
                 object->verts.push_back(stof(sub_line[2]));
                 object->verts.push_back(stof(sub_line[3]));
             }
             else if (line[0] == 'f') {
                 sub_line = split(line, ' ');
                 sub_sub_line = split(sub_line, '/');
                 object->tris.push_back(stoi(sub_sub_line[1][0]) - 1);
                 object->tris.push_back(stoi(sub_sub_line[2][0]) - 1);
                 object->tris.push_back(stoi(sub_sub_line[3][0]) - 1);
             }
         }
         break;
    }
    return 0;
}