#ifndef MFILE_H
#define MFILE_H

#include <iostream>
#include <vector>
#include <map>
#include <fstream>

#include <QString>

#include "glm/glm.hpp"

/******************************************************************************
 *  Declaration for index struct                                              *
 ******************************************************************************/

struct MIndex {

    size_t x = 0, y = 0, z = 0;

};

/******************************************************************************
 *  Declaration for vertex struct                                             *
 ******************************************************************************/

struct MVertex {

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coord;
    glm::vec3 tangent;
    int vertex_index;

};

/******************************************************************************
 *  Declaration for tokens                                                    *
 ******************************************************************************/

#define END_OF_FILE_TOKEN 0x00
#define COLLISION_TOKEN 0x01
#define NEW_MATERIAL_TOKEN 0xFF

/******************************************************************************
 *  Declaration for material struct                                           *
 ******************************************************************************/

struct MMaterial {

    std::string material_domain_name;
    std::string material_path;

};

/******************************************************************************
 *  Declaration for generic file                                              *
 ******************************************************************************/

class MFile {

    public:

        virtual ~MFile() {}

        int getMaterialCount();
        MMaterial* getMaterial(int material);

        MFile* collision_model = nullptr;

        virtual void loadFile(const QString& path) = 0;
        virtual void saveFile(const QString& path);

    protected:

        size_t getVertexIndex(glm::vec3& _position, glm::vec3& _normal, glm::vec2& _tex_coord);
        void calculateTangent(MIndex& index);
        void finalizeTangents();

        std::map<size_t, MVertex> vertices;
        unsigned int total_face_count = 0;

        std::vector<MMaterial> materials;

        glm::vec3 mins, maxes;
        std::vector<std::vector<MIndex>> indicies;
};

#endif // MFILE_H
