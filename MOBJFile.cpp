#include "MOBJFile.h"

/******************************************************************************
 *  Functions for indexing                                                    *
 ******************************************************************************/

int MOBJFile::getVertexIndex(glm::vec3& _position, glm::vec3& _normal, glm::vec2& _tex_coord) {

    for (int i = 1; i < vertices.size(); i++) {

            // Check for position, normal and tex corod
            if (_position == vertices[i].position && _normal == vertices[i].normal && _tex_coord == vertices[i].tex_coord) {

                // Found the vertex, return its index
                return i;

            }

        }

        // If we didnt find the vertex, that means that this one was unique and we need to add it
        vertices.push_back(MVertex());
        MVertex& new_vertex = vertices.back();

        new_vertex.position = _position;
        new_vertex.normal = _normal;
        new_vertex.tex_coord = _tex_coord;

        // Return the index of the last vertex
        return (int)vertices.size() - 1;

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
           std::vector<glm::ivec3> indicies_vector;

           // Create a material
           MMaterial new_material;
           new_material.material_domain_name = i->first;
           materials.push_back(new_material);

           for (int j = 0; j < i->second.size(); j++) {

               glm::ivec3 new_indicies;

               // Get the data in .obj format for indicies
               glm::ivec3 indicies_vertex, indicies_normal, indicies_tex;
               sscanf(i->second[j].c_str(), "f %i/%i/%i %i/%i/%i %i/%i/%i", &indicies_vertex.x, &indicies_tex.x, &indicies_normal.x,
                      &indicies_vertex.y, &indicies_tex.y, &indicies_normal.y,
                      &indicies_vertex.z, &indicies_tex.z, &indicies_normal.z);

               // Get the verticies, these will be indexed
               new_indicies.x = getVertexIndex(_positions[indicies_vertex.x], _normals[indicies_normal.x], _tex_coords[indicies_tex.x]);
               new_indicies.y = getVertexIndex(_positions[indicies_vertex.y], _normals[indicies_normal.y], _tex_coords[indicies_tex.y]);
               new_indicies.z = getVertexIndex(_positions[indicies_vertex.z], _normals[indicies_normal.z], _tex_coords[indicies_tex.z]);

               std::cout << "Indexed a face\n";

               // Calculate the tangent
               glm::vec3 edge1 = _positions[indicies_vertex.y] - _positions[indicies_vertex.x];
               glm::vec3 edge2 = _positions[indicies_vertex.z] - _positions[indicies_vertex.x];

               glm::vec2 d_UV1 = _tex_coords[indicies_tex.y] - _tex_coords[indicies_tex.x];
               glm::vec2 d_UV2 = _tex_coords[indicies_tex.z] - _tex_coords[indicies_tex.x];

               float f = 1.0f / (d_UV1.x * d_UV2.y - d_UV1.y * d_UV2.x);

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
       for (int i = 0; i < vertices.size(); i++)
           vertices[i].tangent = glm::normalize(vertices[i].tangent);

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
        for (int j = 0; j < vertices.size(); j++) {

            // Write the position
            out_file_stream.write((char*)&vertices[j].position, sizeof(glm::vec3));

            // Write out the normal
            out_file_stream.write((char*)&vertices[j].normal, sizeof(glm::vec3));

            // Write out the tangent
            out_file_stream.write((char*)&vertices[j].tangent, sizeof(glm::vec3));

            // Write out the tex_coord
            out_file_stream.write((char*)&vertices[j].tex_coord, sizeof(glm::vec2));

        }

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
            for (int j = 0; j < indicies[i].size(); j++)
                out_file_stream.write((char*)&indicies[i][j], sizeof(glm::vec3));

        }

        // If we have a collision model, write out a collision token
        if (collision_model) {

            token = COLLISION_TOKEN;
            out_file_stream.write((char*)&token, sizeof(int));

            // Write out the verticies
            unsigned int collision_vertex_count = (unsigned int)collision_model->vertices.size();
            out_file_stream.write((char*)&collision_vertex_count, sizeof(unsigned int));

            for (int i = 0; i < collision_model->vertices.size(); i++)
                out_file_stream.write((char*)&collision_model->vertices[i].position, sizeof(glm::vec3));

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

}

int MOBJFile::getMaterialCount() { return indicies.size(); }
MMaterial* MOBJFile::getMaterial(int material) { return &materials[material]; }
