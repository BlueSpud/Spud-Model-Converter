#ifndef MSMDLFILE_H
#define MSMDLFILE_H

#include "MFile.h"

/******************************************************************************
 *  Declaration for OBJ file                                                  *
 ******************************************************************************/

class MSMDLFile : public MFile {

    public:

        virtual void loadFile(const QString& path);

};

#endif // MSMDLFILE_H
