#include "MDAEFile.h"

void MDAEFile::loadFile(const QString& path) {

    std::cout << "Parsing file\n";

    std::ifstream in_file_stream(path.toStdString());
    std::string xml, line;

    while (std::getline(in_file_stream, line, '\n'))
        xml = xml + line;

    // Parse the XML
    MXMLParser parser;
    parser.parseXML(xml);

    // Start parsing, get the geometry first
    MXMLNode* mesh_node = parser.nodes[0].getChildrenOfType("library_geometries")[0]->getChildrenOfType("geometry")[0]->getChildrenOfType("mesh")[0];

    // Get the polylist, this will tell us where we need to get everything from
    std::vector<MXMLNode*> polylists = mesh_node->getChildrenOfType("polylist");

    unsigned int total_vertex_count = 0;

    for (int p = 0; p < polylists.size(); p++) {

        std::vector<glm::vec3>_positions;
        std::vector<glm::vec3>_normals;
        std::vector<glm::vec2>_tex_coords;

        // Find the sources and organize them
        std::vector<MXMLNode*> polylist_input = polylists[0]->getChildrenOfType("input");

        // The input for the positions is not in the polylist, so we find it
        for (int i = 0; i < polylist_input.size(); i++)
            if (!polylist_input[i]->getArgument("semantic").compare("VERTEX"))
                polylist_input.erase(polylist_input.begin() + i);

        polylist_input.push_back(mesh_node->getChildrenOfType("vertices")[0]->getChildrenOfType("input")[0]);

        std::vector<MXMLNode*> sources = mesh_node->getChildrenOfType("source");
        std::map<std::string, MXMLNode*> source_buffers;

        for (int i = 0; i < polylist_input.size(); i++) {

            // This comes with a # on it, remove that first
            std::string source_name = polylist_input[i]->getArgument("source");
            source_name = source_name.substr(1, source_name.size() - 1);

            // Match it with a source node
            for (int j = 0; j < sources.size(); j++) {
                if (!source_name.compare(sources[j]->getArgument("id"))) {

                    source_buffers[polylist_input[i]->getArgument("semantic")] = sources[j];
                    break;

                }

            }

        }

        // Parse the sources, starting with vectors. We can assume that we have normals and tex coords and their sizes
        MXMLNode* array = source_buffers["POSITION"]->getChildrenOfType("float_array")[0];
        std::stringstream stream = std::stringstream(array->contents);
        int size = atoi(array->getArgument("count").c_str()) / 3;

        for (int i = 0; i < size; i++) {

            glm::vec3 position;

            // X
            std::getline(stream, line, ' ');
            position.x = atof(line.c_str());

            // Y
            std::getline(stream, line, ' ');
            position.y = atof(line.c_str());

            // Z
            std::getline(stream, line, ' ');
            position.z = atof(line.c_str());

            _positions.push_back(position);

        }

        // Parse normals
        array = source_buffers["NORMAL"]->getChildrenOfType("float_array")[0];
        stream = std::stringstream(array->contents);
        size = atoi(array->getArgument("count").c_str()) / 3;

        for (int i = 0; i < size; i++) {

           glm::vec3 normal;

           // X
           std::getline(stream, line, ' ');
           normal.x = atof(line.c_str());

           // Y
           std::getline(stream, line, ' ');
           normal.y = atof(line.c_str());

           // Z
           std::getline(stream, line, ' ');
           normal.z = atof(line.c_str());

           _normals.push_back(normal);

       }

        // Parse tex coords
        array = source_buffers["TEXCOORD"]->getChildrenOfType("float_array")[0];
        stream = std::stringstream(array->contents);
        size = atoi(array->getArgument("count").c_str()) / 2;

        for (int i = 0; i < size; i++) {

           glm::vec2 tex_coord;

           // X
           std::getline(stream, line, ' ');
           tex_coord.x = atof(line.c_str());

           // Y
           std::getline(stream, line, ' ');
           tex_coord.y = atof(line.c_str());

           _tex_coords.push_back(tex_coord);

       }

       // Now we parse the indicies, found in the polylist, we assume this was triangulated
       MXMLNode* polylist_p = polylists[p]->getChildrenOfType("p")[0];
       stream = std::stringstream(polylist_p->contents);
       total_face_count = atoi(polylists[p]->getArgument("count").c_str());
       std::vector<MIndex> indicies_vector;

       // Create a material for the list
       MMaterial new_material;

       std::string domain_name = polylists[p]->getArgument("material");
       if (!domain_name.length())
            domain_name = "Generated_Material_0";

       new_material.material_domain_name = domain_name;
       materials.push_back(new_material);

       for (int i = 0; i < total_face_count; i++) {

           // First pase the 3 verticies
            glm::vec3 _verticies[3];

            for (int j = 0; j < 3; j++) {

               // Position
               std::getline(stream, line, ' ');
               _verticies[j].x = atoi(line.c_str());

               // Normal
               std::getline(stream, line, ' ');
               _verticies[j].y = atoi(line.c_str());

               // Tex coord
               std::getline(stream, line, ' ');
               _verticies[j].z = atoi(line.c_str());

            }

            // Resolve the indexes
            MIndex index;
            index.x = getVertexIndex(_positions[_verticies[0].x], _normals[_verticies[0].y], _tex_coords[_verticies[0].z]);
            index.y = getVertexIndex(_positions[_verticies[1].x], _normals[_verticies[1].y], _tex_coords[_verticies[1].z]);
            index.z = getVertexIndex(_positions[_verticies[2].x], _normals[_verticies[2].y], _tex_coords[_verticies[2].z]);

             indicies_vector.push_back(index);

            // Calculate the tangnet
            calculateTangent(index);

       }

       // Add the new indicies to the final list
       indicies.push_back(indicies_vector);

       total_vertex_count = total_vertex_count + _positions.size();

    }

   // Finalize tangents
   finalizeTangents();

   std::cout << "Read " << total_vertex_count << " vertex positions from .dae\n";
   std::cout << "Needed a total of " << vertices.size() << " unique verticies\n";
   std::cout << "Had " << indicies.size() << " materials\n";
   std::cout << "Had " << total_face_count << " faces\n";

}
