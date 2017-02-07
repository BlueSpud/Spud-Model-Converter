#ifndef MDAEFILE_H
#define MDAEFILE_H

#include <QString>
#include <sstream>

#include "MXMLParser.h"
#include "MFile.h"

class MDAEFile : public MFile {

    public:

        void loadFile(const QString& path);
};

#endif // MDAEFILE_H
