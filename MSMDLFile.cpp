#include "MSMDLFile.h"

/******************************************************************************
 *  Function for loading                                                      *
 ******************************************************************************/

void MSMDLFile::loadFile(const QString& path) {

    std::ifstream file(path.toStdString(), std::ios::binary);

    // Read the header, the min and max of the bounding box
    glm::vec3 extents[2];
    file.read((char*)&extents, sizeof(glm::vec3) * 2);
    mins = extents[0];
    maxes = extents[1];

    // Grab the face count and vertex count
    unsigned int vertex_count = 0;
    file.read((char*)&vertex_count, sizeof(unsigned int));

    // Allocate the arrays for the data
    glm::vec3* verts = new glm::vec3[vertex_count];
    glm::vec3* normals = new glm::vec3[vertex_count];
    glm::vec2* tex_coords = new glm::vec2[vertex_count];
    glm::vec3* tangents = new glm::vec3[vertex_count];

    // Read the verticies
    for (int i = 0; i < vertex_count; i++) {

        // Read position
        file.read((char*)&verts[i], sizeof(glm::vec3));

        // Read normal
        file.read((char*)&normals[i], sizeof(glm::vec3));

        // Read tangent
        file.read((char*)&tangents[i], sizeof(glm::vec3));

        // Read tex coord
        file.read((char*)&tex_coords[i], sizeof(glm::vec2));

        // Change to convert from old to new
        //tex_coords[i].y = 1.0 - tex_coords[i].y;

    }

    while (true) {

        // Read the next token
        int token;
        file.read((char*)(char*)&token, sizeof(int));

        // If end of file was reached, we are done
        if (token == END_OF_FILE_TOKEN)
            break;

        if (token == NEW_MATERIAL_TOKEN) {

            // Read the material name that we need
            unsigned int material_name_length;
            file.read((char*)&material_name_length, sizeof(unsigned int));

            // Get the path of the material
            char* material_path_s = new char[material_name_length + 1];
            file.read((char*)material_path_s, sizeof(char) * material_name_length);

            std::string material_path_std = material_path_s;
            material_path_std = material_path_std.substr(0, material_name_length);
            delete[] material_path_s;

            MMaterial mat;
            mat.material_domain_name = "Material_" + std::to_string(materials.size());
            mat.material_path = material_path_std;
            materials.push_back(mat);

            // Read the index count
            unsigned int index_count;
            file.read((char*)&index_count, sizeof(unsigned int));

            std::vector<MIndex> material_indicies;

            for (int j = 0; j < index_count; j++) {

                // Read the indicies
                glm::ivec3 index;
                file.read((char*)&index, sizeof(glm::ivec3));

                MIndex hashed;
                hashed.x = getVertexIndex(verts[index.x], normals[index.x], tex_coords[index.x]);
                hashed.y = getVertexIndex(verts[index.y], normals[index.y], tex_coords[index.y]);
                hashed.z = getVertexIndex(verts[index.z], normals[index.z], tex_coords[index.z]);

                calculateTangent(hashed);

                material_indicies.push_back(hashed);

            }

            indicies.push_back(material_indicies);

        }

        if (token == COLLISION_TOKEN) {

            // Read the number of verticies
            unsigned int collision_vertex_count;
            file.read((char*)&collision_vertex_count, sizeof(unsigned int));

            glm::vec3* collision_verticies = new glm::vec3[collision_vertex_count];

            for (int i = 0; i < collision_vertex_count; i++)
                file.read((char*)&collision_verticies[i], sizeof(glm::vec3));

            // Read the indicies
            unsigned int collision_index_count;
            file.read((char*)&collision_index_count, sizeof(unsigned int));

            glm::vec3* collision_indicies = new glm::vec3[collision_index_count];

            for (int i = 0; i < collision_index_count; i++)
                file.read((char*)&collision_indicies[i], sizeof(glm::vec3));

              // Clean up
            delete [] collision_verticies;
            delete [] collision_indicies;

        }

     }

}
