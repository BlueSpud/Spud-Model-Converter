#include "MFile.h"

std::hash<std::string> hasher;

/******************************************************************************
 *  Functions for generic file                                                *
 ******************************************************************************/

size_t MFile::getVertexIndex(glm::vec3& _position, glm::vec3& _normal, glm::vec2& _tex_coord) {

    // Hash the vertex
    size_t hash = hasher(std::to_string(_position.x) + std::to_string(_position.y) + std::to_string(_position.z) +
                         std::to_string(_normal.x) + std::to_string(_normal.y) + std::to_string(_normal.z) +
                         std::to_string(_tex_coord.x) + std::to_string(_tex_coord.y));

    if (vertices.count(hash))
        return hash;

    // Didnt find, add new vertex
    MVertex new_vertex;
    new_vertex.position = glm::vec3(_position);
    new_vertex.normal = glm::vec3(_normal);
    new_vertex.tex_coord = glm::vec2(_tex_coord);

    // Check this vertex against the min and max
    mins.x = glm::min(_position.x, mins.x);
    mins.y = glm::min(_position.y, mins.y);
    mins.z = glm::min(_position.z, mins.z);

    maxes.x = glm::max(_position.x, maxes.x);
    maxes.y = glm::max(_position.y, maxes.y);
    maxes.z = glm::max(_position.z, maxes.z);

    vertices[hash] = new_vertex;

    return hash;

}

size_t MFile::getVertexIndex(glm::vec3& _position, glm::vec3& _normal, glm::vec2& _tex_coord, glm::vec4& _bone_indicies, glm::vec4& _bone_weights) {

    // Hash the vertex (with skinning information)
    size_t hash = hasher(std::to_string(_position.x) + std::to_string(_position.y) + std::to_string(_position.z) +
                         std::to_string(_normal.x) + std::to_string(_normal.y) + std::to_string(_normal.z) +
                         std::to_string(_tex_coord.x) + std::to_string(_tex_coord.y) +
                         std::to_string(_bone_indicies.x) + std::to_string(_bone_indicies.y) + std::to_string(_bone_indicies.z) + std::to_string(_bone_indicies.w) +
                         std::to_string(_bone_weights.x) + std::to_string(_bone_weights.y) + std::to_string(_bone_weights.z) + std::to_string(_bone_weights.w));

    if (vertices.count(hash))
        return hash;

    // Didnt find, add new vertex
    MVertex new_vertex;
    new_vertex.position = glm::vec3(_position);
    new_vertex.normal = glm::vec3(_normal);
    new_vertex.tex_coord = glm::vec2(_tex_coord);
    new_vertex.bone_indicies = glm::vec4(_bone_indicies);
    new_vertex.bone_weights = glm::vec4(_bone_weights);

    // Check this vertex against the min and max
    mins.x = glm::min(_position.x, mins.x);
    mins.y = glm::min(_position.y, mins.y);
    mins.z = glm::min(_position.z, mins.z);

    maxes.x = glm::max(_position.x, maxes.x);
    maxes.y = glm::max(_position.y, maxes.y);
    maxes.z = glm::max(_position.z, maxes.z);

    vertices[hash] = new_vertex;

    return hash;


}

void MFile::calculateTangent(MIndex& index) {

    glm::vec3 edge1 = vertices[index.y].position - vertices[index.x].position;
    glm::vec3 edge2 = vertices[index.z].position - vertices[index.x].position;

    glm::vec2 d_UV1 = vertices[index.y].tex_coord - vertices[index.x].tex_coord;
    glm::vec2 d_UV2 = vertices[index.z].tex_coord - vertices[index.y].tex_coord;

    float f = 1.0f / (d_UV1.x * d_UV2.y - d_UV1.y * d_UV2.x);

    glm::vec3 tangent = glm::normalize((edge1 * d_UV2.y - edge2 * d_UV1.y) * f);

    if (!std::isfinite(glm::length(tangent)))
        tangent = edge1;

    // Add the tangent on for all of the vertices
    vertices[index.x].tangent += tangent;
    vertices[index.y].tangent += tangent;
    vertices[index.z].tangent += tangent;

}

void MFile::finalizeTangents() {

    // Tangents are just averages, normalize them
    for (std::map<size_t, MVertex>::iterator i = vertices.begin(); i != vertices.end(); i++)
        i->second.tangent = glm::normalize(i->second.tangent);

}

void MFile::saveFile(const QString& path) {

        /******************************************************************************
         *  Write out the Spud Model File                                             *
         ******************************************************************************/

        std::ofstream out_file_stream(path.toStdString(), std::ios::binary);

        // Write the header, this is mins and maxes
        out_file_stream.write((char*)&mins, sizeof(glm::vec3));
        out_file_stream.write((char*)&maxes, sizeof(glm::vec3));

         std::cout << "Saving extents: " << mins.x << " " << mins.y << " " << mins.z << ", " << maxes.x << " " << maxes.y << " " << maxes.z << std::endl;

        // Write the face count and then vert count
        unsigned int vertex_count = (int)vertices.size();
        out_file_stream.write((char*)&vertex_count, sizeof(unsigned int));

        // Write out the verticies
        int vertex_index = 0;
        for (std::map<size_t, MVertex>::iterator i = vertices.begin(); i != vertices.end(); i++) {

            // Write the position
            out_file_stream.write((char*)&i->second.position, sizeof(glm::vec3));

            // Write out the normal
            out_file_stream.write((char*)&i->second.normal, sizeof(glm::vec3));

            // Write out the tangent
            out_file_stream.write((char*)&i->second.tangent, sizeof(glm::vec3));

            // Write out the tex_coord
            out_file_stream.write((char*)&i->second.tex_coord, sizeof(glm::vec2));

            i->second.vertex_index = vertex_index;
            vertex_index++;

        }

        std::cout << "Wrote " << vertex_index << std::endl;

        int token;

        // Write out the indicies
        for (int i = 0; i < indicies.size(); i++) {

            // Write out a new material token
            int token = NEW_MATERIAL_TOKEN;
            out_file_stream.write((char*)&token, sizeof(int));

            // Write out the name of the material that belongs here
            unsigned int material_name_length = materials[i].material_path.length();
            out_file_stream.write((char*)&material_name_length, sizeof(unsigned int));
            out_file_stream.write(materials[i].material_path.c_str(), sizeof(char) * material_name_length);

            // Write out the count
            unsigned int index_count = (unsigned int)indicies[i].size();
            out_file_stream.write((char*)&index_count, sizeof(unsigned int));

            // Write the indicies
            for (int j = 0; j < indicies[i].size(); j++) {

                 MIndex& hashed = indicies[i][j];
                 glm::ivec3 unhashed = glm::ivec3(vertices[hashed.x].vertex_index, vertices[hashed.y].vertex_index, vertices[hashed.z].vertex_index);

                out_file_stream.write((char*)&unhashed, sizeof(glm::ivec3));
            }

        }

        // If we have a collision model, write out a collision token
        if (collision_model) {

            token = COLLISION_TOKEN;
            out_file_stream.write((char*)&token, sizeof(int));

            // Write out the verticies
            unsigned int collision_vertex_count = (unsigned int)collision_model->vertices.size();
            out_file_stream.write((char*)&collision_vertex_count, sizeof(unsigned int));

            for (std::map<size_t, MVertex>::iterator i = collision_model->vertices.begin(); i != collision_model->vertices.end(); i++)
                out_file_stream.write((char*)&i->second.position, sizeof(glm::vec3));

            // For all the materials, write out their indieis
            out_file_stream.write((char*)&collision_model->total_face_count, sizeof(unsigned int));
            for (int i = 0; i < collision_model->indicies.size(); i++)
                for (int j = 0; j < collision_model->indicies[i].size(); j++)
                    out_file_stream.write((char*)&collision_model->indicies[i][j], sizeof(glm::vec3));

        }

        // Write out an end of file token
        token = END_OF_FILE_TOKEN;
        out_file_stream.write((char*)&token, sizeof(int));

        out_file_stream.close();

        // If we are skinned we also write out a new file
        if (skinned) {

            std::string modified_path = path.toStdString();
            modified_path = modified_path.replace(modified_path.length() - 5, 5, ".srig");

            out_file_stream = std::ofstream(modified_path, std::ios::binary);

            // Write out the global bind pose
            out_file_stream.write((char*)&global_bind_pos, sizeof(glm::mat4));

            // Write out the bones
            unsigned int bone_count = (unsigned int)bones.size();
            out_file_stream.write((char*)&bone_count, sizeof(unsigned int));

            for (int i = 0; i < bone_count; i++) {

                // Write out the parent of the bone
                out_file_stream.write((char*)&bones[i].parent_index, sizeof(int));

                // Write out the matricies, bind matrix then transform
                out_file_stream.write((char*)&bones[i].bind_matrix, sizeof(glm::mat4));

                // Write out the matricies
                out_file_stream.write((char*)&bones[i].matrix, sizeof(glm::mat4));

            }

            // We dont need to write out the vertex count because we should already have it, but we do need to write out the weights
            for (std::map<size_t, MVertex>::iterator i = vertices.begin(); i != vertices.end(); i++) {

                // Write the indicies
                out_file_stream.write((char*)&i->second.bone_indicies, sizeof(glm::vec4));

                // Write out the weights
                out_file_stream.write((char*)&i->second.bone_weights, sizeof(glm::vec4));

            }

            out_file_stream.close();

        }


}

void MFile::saveAnimation(const QString& path) {

    std::ofstream out_file_stream(path.toStdString(), std::ios::binary);

    // Write out number of bones
    unsigned int bone_count = animation.size();
    out_file_stream.write((char*)&bone_count, sizeof(unsigned int));

    for (int i = 0; i < animation.size(); i++) {

        // Write out the number of keyframes
        unsigned int keyframe_count = animation[i].size();
        out_file_stream.write((char*)&keyframe_count, sizeof(unsigned int));

        // Write out each keyfram
        for (int j = 0; j < animation[i].size(); j++) {

             // Write out the tweening type
             out_file_stream.write((char*)&animation[i][j].tweener, sizeof(MAnimationTweening));

             // Write out what time this keyframe is
            out_file_stream.write((char*)&animation[i][j].time, sizeof(float));

            // Write out the matrix
            out_file_stream.write((char*)&animation[i][j].matrix, sizeof(glm::mat4));

        }

    }

}

int MFile::getMaterialCount() { return indicies.size(); }
MMaterial* MFile::getMaterial(int material) { return &materials[material]; }

int MFile::findBoneNamed(const std::string& name) {

    for (int i = 0; i < bones.size(); i++)
        if (!name.compare(bones[i].name))
            return i;

    return -1;

}
