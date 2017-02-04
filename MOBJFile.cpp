#include "MOBJFile.h"

std::hash<std::string> hasher;

/******************************************************************************
 *  Functions for indexing                                                    *
 ******************************************************************************/

size_t MOBJFile::getVertexIndex(glm::vec3& _position, glm::vec3& _normal, glm::vec2& _tex_coord) {

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

    vertices[hash] = new_vertex;

    return hash;

}

/******************************************************************************
 *  Function for loading                                                      *
 ******************************************************************************/

void MOBJFile::loadFile(const QString& path) {

       std::ifstream in_file_stream(path.toStdString());

       std::string line;

       std::vector<glm::vec3>_positions;
       std::vector<glm::vec3>_normals;
       std::vector<glm::vec2>_tex_coords;

       vertices = std::map<size_t, MVertex>();

       // Storage and required objects for indicy parsing
       std::map<std::string, std::vector<std::string>> indices_map;

       // Create something if no materials were included
       indices_map["Generated_Material_0"] = std::vector<std::string>();

       std::string current_material = "Generated_Material_0";

       // WARNING:
       // .obj indicies start at one and therefore we need to add 1 to every one we parse
       _positions.push_back(glm::vec3());
       _normals.push_back(glm::vec3());
       _tex_coords.push_back(glm::vec2());

       // Itterate over every line in the file
       while (std::getline(in_file_stream, line, '\n')) {

           // Parse materials
           if (line.compare(0, 7, "usemtl ") == 0) {

               // The current material was changed here, so we get the name and hash it
               std::string material_name = line.substr(7, line.length() - 7);

               // If this is the first time add it to the map
               if (!indices_map.count(material_name))
                   indices_map[material_name] = std::vector<std::string>();

               current_material = material_name;

           }

           // Parse verticies
           if (line.compare(0, 2, "v ") == 0) {

               glm::vec3 v;
               sscanf(line.c_str(), "v %f %f %f",&v.x,&v.y,&v.z);

               // Check this vertex against the min and max
               mins.x = glm::min(v.x, mins.x);
               mins.y = glm::min(v.y, mins.y);
               mins.z = glm::min(v.z, mins.z);

               maxes.x = glm::max(v.x, maxes.x);
               maxes.y = glm::max(v.y, maxes.y);
               maxes.z = glm::max(v.z, maxes.z);

               _positions.push_back(v);
           }

           // Parse normals
           if (line.compare(0, 2, "vn") == 0) {

               glm::vec3 n;
               sscanf(line.c_str(), "vn %f %f %f",&n.x,&n.y,&n.z);

               _normals.push_back(n);
           }

           // Parse texture coordinates
           if (line.compare(0, 2, "vt") == 0) {

               glm::vec2 t;
               sscanf(line.c_str(), "vt %f %f",&t.x,&t.y);

               _tex_coords.push_back(t);
           }

           // Parse indicies
           if (line.compare(0, 1, "f") == 0) {

               // Add it to the current material
               indices_map[current_material].push_back(line);
               total_face_count++;

           }

       }

       // Close the file stream
       in_file_stream.close();

       // If we didnt use the default array scrap it
       if (!indices_map["Generated_Material_0"].size())
           indices_map.erase("Generated_Material_0");

       // Now we put together everything, we do it with an indexed mesh
       std::map<std::string, std::vector<std::string>>::iterator i = indices_map.begin();

       while (i != indices_map.end()) {

           // Create a new vector to store the indicies in
           std::vector<MIndex> indicies_vector;

           // Create a material
           MMaterial new_material;
           new_material.material_domain_name = i->first;
           materials.push_back(new_material);

           for (int j = 0; j < i->second.size(); j++) {

               MIndex new_indicies;

               // Get the data in .obj format for indicies
               glm::ivec3 indicies_vertex, indicies_normal, indicies_tex;
               sscanf(i->second[j].c_str(), "f %i/%i/%i %i/%i/%i %i/%i/%i", &indicies_vertex.x, &indicies_tex.x, &indicies_normal.x,
                      &indicies_vertex.y, &indicies_tex.y, &indicies_normal.y,
                      &indicies_vertex.z, &indicies_tex.z, &indicies_normal.z);

               // Get the verticies, these will be indexed
               new_indicies.x = getVertexIndex(_positions[indicies_vertex.x], _normals[indicies_normal.x], _tex_coords[indicies_tex.x]);
               new_indicies.y = getVertexIndex(_positions[indicies_vertex.y], _normals[indicies_normal.y], _tex_coords[indicies_tex.y]);
               new_indicies.z = getVertexIndex(_positions[indicies_vertex.z], _normals[indicies_normal.z], _tex_coords[indicies_tex.z]);

               // Calculate the tangent
               glm::vec3 edge1 = _positions[indicies_vertex.y] - _positions[indicies_vertex.x];
               glm::vec3 edge2 = _positions[indicies_vertex.z] - _positions[indicies_vertex.x];

               glm::vec2 d_UV1 = _tex_coords[indicies_tex.y] - _tex_coords[indicies_tex.x];
               glm::vec2 d_UV2 = _tex_coords[indicies_tex.z] - _tex_coords[indicies_tex.y];

               float f = 1.0f / (d_UV1.x * d_UV2.y - d_UV1.y * d_UV2.x);

               if (!std::isfinite(f))
                   f = 1.0;

               glm::vec3 tangent = glm::normalize((edge1 * d_UV2.y - edge2 * d_UV1.y) * f);

               // Add the tangent on for all of the vertices
               vertices[new_indicies.x].tangent += tangent;
               vertices[new_indicies.y].tangent += tangent;
               vertices[new_indicies.z].tangent += tangent;

               indicies_vector.push_back(new_indicies);

           }

           indicies.push_back(indicies_vector);
           i++;

       }

       // Tangents are just averages, normalize them
       for (std::map<size_t, MVertex>::iterator i = vertices.begin(); i != vertices.end(); i++)
           i->second.tangent = glm::normalize(i->second.tangent);

       std::cout << "Read " << _positions.size() - 1 << " vertex positions from .obj\n";
       std::cout << "Needed a total of " << vertices.size() << " unique verticies\n";
       std::cout << "Had " << indicies.size() << " materials\n";
       std::cout << "Had " << total_face_count << " faces\n";

}

void MOBJFile::saveFile(const QString& path) {

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
//        if (collision_model) {

//            token = COLLISION_TOKEN;
//            out_file_stream.write((char*)&token, sizeof(int));

//            // Write out the verticies
//            unsigned int collision_vertex_count = (unsigned int)collision_model->vertices.size();
//            out_file_stream.write((char*)&collision_vertex_count, sizeof(unsigned int));

//            for (int i = 0; i < collision_model->vertices.size(); i++)
//                out_file_stream.write((char*)&collision_model->vertices[i].position, sizeof(glm::vec3));

//            // For all the materials, write out their indieis
//            out_file_stream.write((char*)&collision_model->total_face_count, sizeof(unsigned int));
//            for (int i = 0; i < collision_model->indicies.size(); i++)
//                for (int j = 0; j < collision_model->indicies[i].size(); j++)
//                    out_file_stream.write((char*)&collision_model->indicies[i][j], sizeof(glm::vec3));

//        }

        // Write out an end of file token
        token = END_OF_FILE_TOKEN;
        out_file_stream.write((char*)&token, sizeof(int));

        out_file_stream.close();

}

int MOBJFile::getMaterialCount() { return indicies.size(); }
MMaterial* MOBJFile::getMaterial(int material) { return &materials[material]; }
