#ifndef MDAEFILE_H
#define MDAEFILE_H

#include <QString>
#include <sstream>
#include <list>
#include <glm/gtc/type_ptr.hpp>

#include "MXMLParser.h"
#include "MFile.h"

class MDAEFile : public MFile {

    public:

        void loadFile(const QString& path);

    private:

        glm::mat4 parseMatrix(const std::string& string);
        std::vector<glm::mat4> parseMatrices(const std::string& string, int count);

        void parseBone(MXMLNode* parent, std::vector<MBone>& bones);
};

#endif // MDAEFILE_H
