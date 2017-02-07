#ifndef MOBJFILE_H
#define MOBJFILE_H

#include "MFile.h"

/******************************************************************************
 *  Declaration for OBJ file                                                  *
 ******************************************************************************/

class MOBJFile : public MFile {

    public:

        virtual void loadFile(const QString& path);

};

#endif // MOBJFILE_H
