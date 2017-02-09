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

    // Skinning
    glm::vec4 bone_indicies;
    glm::vec4 bone_weights;

};

/******************************************************************************
 *  Declaration for bone struct                                               *
 ******************************************************************************/

struct MBone {

    int parent_index;
    glm::mat4 matrix;
    glm::mat4 bind_matrix;

    std::string name;

};

/******************************************************************************
 *  Declaration for animation data                                            *
 ******************************************************************************/

enum MAnimationTweening {

    MAnimationTweeningNormal

};

struct MAnimationKeyFrame {

    MAnimationTweening tweener;
    float time = 0.0;

    glm::mat4 matrix;

};

/******************************************************************************
 *  Declaration for tokens                                                    *
 ******************************************************************************/

#define END_OF_FILE_TOKEN 0x00
#define COLLISION_TOKEN 0x01
#define SKINNED_TOKEN 0x02
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
        virtual void saveAnimation(const QString& path);

        const std::vector<std::vector<MAnimationKeyFrame>> getAnimation() { return animation; }

    protected:

        size_t getVertexIndex(glm::vec3& _position, glm::vec3& _normal, glm::vec2& _tex_coord);
        size_t getVertexIndex(glm::vec3& _position, glm::vec3& _normal, glm::vec2& _tex_coord, glm::vec4& _bone_indicies, glm::vec4& _bone_weights);
        void calculateTangent(MIndex& index);
        void finalizeTangents();

        int findBoneNamed(const std::string& name);

        std::map<size_t, MVertex> vertices;
        unsigned int total_face_count = 0;

        std::vector<MMaterial> materials;

        glm::vec3 mins, maxes;
        std::vector<std::vector<MIndex>> indicies;

        // Data for skinning
        bool skinned = false;
        std::vector<std::string> bone_names;
        std::vector<MBone> bones;
        glm::mat4 global_bind_pos;

        std::vector<std::vector<MAnimationKeyFrame>> animation;
};

#endif // MFILE_H
