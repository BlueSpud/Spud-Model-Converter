#ifndef MXMLPARSER_H
#define MXMLPARSER_H

#include <iostream>
#include <sstream>
#include <vector>
#include <map>

// Forward declaration
class MXMLParser;

/******************************************************************************
 *  Definition for XML node                                                   *
 ******************************************************************************/

struct MXMLNode {

    friend class MXMLParser;

    std::string name;
    std::string contents;

    std::string getArgument(const std::string& arg);
    std::vector<MXMLNode*> getChildrenOfType(const std::string& type);

    private:

        std::map<std::string, std::string> arguments;
        std::vector<MXMLNode> children;

};

/******************************************************************************
 *  Definition for XML parser                                                 *
 ******************************************************************************/

class MXMLParser {

    public:

        void parseXML(std::string& xml);
        std::vector<MXMLNode> nodes;

    private:

        MXMLNode parseNode(std::string xml, int& i);

};

#endif // MXMLPARSER_H
