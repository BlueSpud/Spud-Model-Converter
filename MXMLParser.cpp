#include "MXMLParser.h"

/******************************************************************************
 *  Implementation for XML node                                               *
 ******************************************************************************/

std::string MXMLNode::getArgument(const std::string& arg) {

    if (arguments.count(arg))
        return arguments[arg];

    std::cout << "Could not find argument on XML node " << arg << std::endl;
    return "";

}

std::vector<MXMLNode*> MXMLNode::getChildrenOfType(const std::string& type) {

    std::vector<MXMLNode*> search_results;
    for (int i = 0; i < children.size(); i++)
        if (!children[i].name.compare(type))
            search_results.push_back(&children[i]);

    return search_results;

}

/******************************************************************************
 *  Implementation for XML parser                                             *
 ******************************************************************************/

 void MXMLParser::parseXML(std::string& xml) {

    int i = 0;
    while (true) {

        MXMLNode node = parseNode(xml, i);

        // Make sure the node had a name and was not the XML declaration
        if (node.name.length()) {

            if (node.name.compare("?xml"))
                nodes.push_back(node);

        } else break;

    }

 }

 MXMLNode MXMLParser::parseNode(std::string xml, int& i) {

    MXMLNode node;

    size_t search = xml.find("<", i);

    // Check that this was the opening to a node
    if (search != std::string::npos) {

        i = search + 1;

        while (!xml.substr(i, 1).compare(" ") || !xml.substr(i, 1).compare("\t"))
            i++;

        // Parse the name of the node
        while (xml.substr(i, 1).compare(" ") && xml.substr(i, 1).compare(">")) {

            node.name = node.name + xml.substr(i, 1);
            i++;

        }

        if (node.name.find("/") != std::string::npos)
            return node;

        // Xml declaration shouldnt be parsed
        if (!node.name.compare("?xml"))
            return node;

        // Parse the arguments of the node
        while (!xml.substr(i, 1).compare(" ") || !xml.substr(i, 1).compare("\t")) {

            std::string arg_name;
            i++;

            while (xml.substr(i, 1).compare("=")) {

                arg_name = arg_name + xml.substr(i, 1);
                i++;

            }

            // We know that there was an equal sign because we looked for it before, just incremenet i by 2 because there will also be quotes
            i = i + 2;

            std::string arg_contents;

            while (xml.substr(i, 1).compare("\"")) {

                arg_contents = arg_contents + xml.substr(i, 1);
                i++;

            }

            // Assume closing ", increment by one more
            i++;

            // Save the arg
            node.arguments[arg_name] = arg_contents;

        }

        // Take out any blank space that was at the end
        while (!xml.substr(i, 1).compare(" ") || !xml.substr(i, 1).compare("\t"))
            i++;

        // If we have a slash at the end of the node, it means that we didnt have a body
        if (!xml.substr(i, 1).compare("/"))
            return node;

        if (!xml.substr(i, 1).compare(">")) {

            i++;

            // Now we parse the body
            while (xml.substr(i, 1).compare("<")) {

                node.contents = node.contents + xml.substr(i, 1);
                i++;

            }

            // Parse the body, this can be text or more nodes
            while (true) {

                MXMLNode child = parseNode(xml, i);

                if (child.name.length()) {

                    if (!child.name.compare("/" + node.name))
                        break;

                    node.children.push_back(child);

                } else break;

            }

            // Node parsing finished
            i--;

        } else return MXMLNode();

    }

    return node;

 }
