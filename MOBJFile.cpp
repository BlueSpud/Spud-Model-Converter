#include "MOBJFile.h"

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
              calculateTangent(new_indicies);

              indicies_vector.push_back(new_indicies);

           }

           indicies.push_back(indicies_vector);
           i++;

       }

       // Finalize the tangents
       finalizeTangents();

       std::cout << "Read " << _positions.size() << " vertex positions from .obj\n";
       std::cout << "Needed a total of " << vertices.size() << " unique verticies\n";
       std::cout << "Had " << indicies.size() << " materials\n";
       std::cout << "Had " << total_face_count << " faces\n";

}
