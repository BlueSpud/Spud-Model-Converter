#include "MDAEFile.h"

void MDAEFile::loadFile(const QString& path) {

    std::cout << "Parsing file\n";

    std::ifstream in_file_stream(path.toStdString());
    std::string xml, line;

    while (std::getline(in_file_stream, line, '\n'))
        xml = xml + line;

    in_file_stream.close();

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
        std::vector<glm::vec4>_bone_indicies;
        std::vector<glm::vec4>_bone_weights;

        // Find the sources and organize them
        std::vector<MXMLNode*> polylist_input = polylists[0]->getChildrenOfType("input");

        // Get the skeleton
        std::vector<MXMLNode*> controllers = parser.nodes[0].getChildrenOfType("library_controllers")[0]->getChildrenOfType("controller");
        MXMLNode* skeleton = nullptr;

        for (int i = 0; i < controllers.size(); i++) {

             if (controllers[i]->getChildrenOfType("skin").size()) {

                 skeleton = controllers[i]->getChildrenOfType("skin")[0];
                 break;

             }

        }

        // Not all collada models are assumed to have skeletons
        if (skeleton) {

            // Make sure we remember that this is skinned
            skinned = true;

            std::vector<MXMLNode*> skeleton_sources = skeleton->getChildrenOfType("source");

            // Bind the global shape matrix, theres a good chance this is an identity matrix
            global_bind_pos = parseMatrix(skeleton->getChildrenOfType("bind_shape_matrix")[0]->contents);

            // Find the name source
            MXMLNode* name_node;
            MXMLNode* weight_content_node;
            MXMLNode* bind_poses;

            for (int i = 0; i < skeleton_sources.size(); i++) {

                if (skeleton_sources[i]->getArgument("id").find("joints") != std::string::npos)
                     name_node = skeleton_sources[i];

                if (skeleton_sources[i]->getArgument("id").find("weights") != std::string::npos)
                     weight_content_node = skeleton_sources[i];

                if (skeleton_sources[i]->getArgument("id").find("bind_poses") != std::string::npos)
                     bind_poses = skeleton_sources[i];

            }

            // Read the names
            std::stringstream stream = std::stringstream(name_node->getChildrenOfType("Name_array")[0]->contents);
            std::string name;

            while (std::getline(stream, name, ' '))
                bone_names.push_back(name);

            // Parse the bones themselves
            std::vector<MXMLNode*> scene_nodes =  parser.nodes[0].getChildrenOfType("library_visual_scenes")[0]->getChildrenOfType("visual_scene")[0]->getChildrenOfType("node");
            MXMLNode* root_parent = nullptr;

            // Figure out which node in the scene actually contained an rig
            for (int i = 0; i < scene_nodes.size(); i++) {

                 std::vector<MXMLNode*> node_children = scene_nodes[i]->getChildrenOfType("node");
                 if (node_children.size() && !node_children[0]->getArgument("type").compare("JOINT")) {

                     root_parent = scene_nodes[i];
                     break;

                 }

            }

            // Parse the bones
            parseBone(root_parent, bones);

            // Once the bones are parsed we make an array of indexes to turn collada indexes into the stored one
            std::vector<int> bone_indicies;

            for (int i = 0; i < bone_names.size(); i++)
                bone_indicies.push_back(findBoneNamed(bone_names[i]));

            // Lastly weget the bind poses for the matricies
            std::vector<glm::mat4> bind_matrices = parseMatrices(bind_poses->getChildrenOfType("float_array")[0]->contents, bone_names.size());

             // Give the bone the matrix
            for (int i = 0; i < bone_names.size(); i++)
                bones[bone_indicies[i]].bind_matrix = bind_matrices[i];

            // Get the weights
            std::vector<float> weight_values;
            stream = std::stringstream(weight_content_node->getChildrenOfType("float_array")[0]->contents);
            std::string weight_f;

            while (std::getline(stream, weight_f, ' '))
                weight_values.push_back(atof(weight_f.c_str()));

            MXMLNode* weight_node = skeleton->getChildrenOfType("vertex_weights")[0];

            MXMLNode* count_node = weight_node->getChildrenOfType("vcount")[0];
            std::stringstream count_stream = std::stringstream(count_node->contents);

            MXMLNode* weight_index_node = weight_node->getChildrenOfType("v")[0];
            std::stringstream weight_index_stream = std::stringstream(weight_index_node->contents);

            std::string count;
            std::string num;
            while (std::getline(count_stream, count, ' ')) {

                std::list<glm::vec2> _vertex_weights;

                // Get the number of weights that we are using
                int count_i = atoi(count.c_str());

                for (int i = 0; i < count_i; i++) {

                    glm::vec2 weight;

                    // Get the bone index
                    std::getline(weight_index_stream, num, ' ');
                    weight.x = bone_indicies[atoi(num.c_str())];

                    // Get the weight
                    std::getline(weight_index_stream, num, ' ');
                    weight.y = weight_values[atoi(num.c_str())];

                    // Do an insertion sort
                    std::list<glm::vec2>::iterator insert;
                    for (insert = _vertex_weights.begin(); insert != _vertex_weights.end(); insert++)
                        if ((*insert).y < weight.y)
                            break;

                    _vertex_weights.insert(insert, weight);

                }

                // Compile everything into vec4s
                glm::vec4 vertex_indicies = glm::vec4(-1);
                glm::vec4 vertex_weights = glm::vec4(0.0);

                std::list<glm::vec2>::iterator value = _vertex_weights.begin();
                for (int i = 0; i < 4 && i < _vertex_weights.size(); i++) {

                    glm::vec2& w = *value;

                    vertex_indicies[i] = w.x;
                    vertex_weights[i] = w.y;

                    value++;

                }

                _bone_indicies.push_back(vertex_indicies);
                _bone_weights.push_back(vertex_weights);

            }

            // Try to parse the animation if we can
            MXMLNode* animation_node = parser.nodes[0].getChildrenOfType("library_animations")[0];

            if (animation_node) {

                // There was an animation
                animation = std::vector<std::vector<MAnimationKeyFrame>>();

                std::vector<MXMLNode*> bone_animation_nodes = animation_node->getChildrenOfType("animation");
                std::vector<MXMLNode*> bone_animation_sorted;
                bone_animation_sorted.resize(bone_animation_nodes.size());

                for (int i = 0; i < bone_animation_nodes.size(); i++) {

                    // Sort the bones array for simplicity
                    for (int j = 0; j < bone_animation_nodes.size(); j++) {

                        if (bone_animation_nodes[j]->getArgument("id").find(bone_names[i] + "_pose_matrix") != std::string::npos) {

                            bone_animation_sorted[bone_indicies[i]] = bone_animation_nodes[j];
                            break;

                        }

                    }

                }

                // The nodes are sorted, now we can parse the key frames
                for (int i = 0; i < bone_animation_nodes.size(); i++) {

                    animation.push_back(std::vector<MAnimationKeyFrame>());

                    // Get all of the sources
                    std::vector<MXMLNode*> sources = bone_animation_sorted[i]->getChildrenOfType("source");

                    MXMLNode* time_node;
                    MXMLNode* matrix_node;

                    // Figure out which source is which
                    for (int j = 0; j < sources.size(); j++) {

                        if (sources[j]->getArgument("id").find("input") != std::string::npos)
                             time_node = sources[j]->getChildrenOfType("float_array")[0];

                        if (sources[j]->getArgument("id").find("output") != std::string::npos)
                             matrix_node = sources[j]->getChildrenOfType("float_array")[0];

                    }

                    // Parse the times
                    stream = std::stringstream(time_node->contents);
                    std::string time;

                    while (std::getline(stream, time, ' ')) {

                        // Make a new key frame and then set the time
                        animation[i].push_back(MAnimationKeyFrame());
                        animation[i].back().time = atof(time.c_str());

                    }

                    // Get all of the matrices
                    std::vector<glm::mat4> animation_matrices = parseMatrices(matrix_node->contents, animation[i].size());
                    for (int j = 0; j < animation[i].size(); j++)
                        animation[i][j].matrix = animation_matrices[j];

                }

            }

        }

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

            if (!skinned) {

                index.x = getVertexIndex(_positions[_verticies[0].x], _normals[_verticies[0].y], _tex_coords[_verticies[0].z]);
                index.y = getVertexIndex(_positions[_verticies[1].x], _normals[_verticies[1].y], _tex_coords[_verticies[1].z]);
                index.z = getVertexIndex(_positions[_verticies[2].x], _normals[_verticies[2].y], _tex_coords[_verticies[2].z]);

            } else {

                index.x = getVertexIndex(_positions[_verticies[0].x], _normals[_verticies[0].y], _tex_coords[_verticies[0].z], _bone_indicies[_verticies[0].x], _bone_weights[_verticies[0].x]);
                index.y = getVertexIndex(_positions[_verticies[1].x], _normals[_verticies[1].y], _tex_coords[_verticies[1].z], _bone_indicies[_verticies[1].x], _bone_weights[_verticies[1].x]);
                index.z = getVertexIndex(_positions[_verticies[2].x], _normals[_verticies[2].y], _tex_coords[_verticies[2].z], _bone_indicies[_verticies[2].x], _bone_weights[_verticies[2].x]);

            }

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

glm::mat4 MDAEFile::parseMatrix(const std::string& string) {

    std::stringstream stream = std::stringstream(string);
    std::string matrix_f;
    float matrix[16];

    // Get all the values then make the actual matrix
    for (int i = 0; i < 16; i++) {

        std::getline(stream, matrix_f, ' ');
        matrix[i] = atof(matrix_f.c_str());

    }

     return glm::make_mat4(matrix);

}

std::vector<glm::mat4> MDAEFile::parseMatrices(const std::string& string, int count) {

    // Lastly weget the bind poses for the matricies
    std::stringstream stream = std::stringstream(string);
    std::vector<glm::mat4> matrices;

    for (int i = 0; i < count; i++) {

        // Parse the matrix, we cant use the function because theyre all together
        std::string matrix_f;
        float matrix[16];

        for (int m = 0; m < 16; m++) {

            std::getline(stream, matrix_f, ' ');
            matrix[m] = atof(matrix_f.c_str());

        }

        // Give the bone the matrix
       matrices.push_back(glm::make_mat4(matrix));

    }

    return matrices;

}

 void MDAEFile::parseBone(MXMLNode* parent, std::vector<MBone>& bones) {

    std::vector<MXMLNode*> child_bones = parent->getChildrenOfType("node");
    int parent_index = -1;

    if (!parent->getArgument("type").compare("JOINT"))
        parent_index = findBoneNamed(parent->getArgument("sid"));

    for (int i = 0; i < child_bones.size(); i++) {

        // Create a new bone
        bones.push_back(MBone());
        MBone& bone = bones.back();

        bone.name = child_bones[i]->getArgument("sid");
        bone.parent_index = parent_index;
        bone.matrix = parseMatrix(child_bones[i]->getChildrenOfType("matrix")[0]->contents);

        parseBone(child_bones[i], bones);

    }

 }
