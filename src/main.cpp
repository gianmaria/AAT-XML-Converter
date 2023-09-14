#include <cstdint>
#include <exception>
#include <iostream>
#include <string>
#include <format>

#include "pugixml/pugixml.hpp"

using std::cout;
using std::endl;
using std::string;

static constexpr auto indent_1{ "   " };
static constexpr auto indent_2{ "      " };


bool is_section_empty(const pugi::xml_node& node)
{
    bool res = true;

    for (const auto& elem : node)
    {
        res = res and elem.children().empty();
    }

    return res;
}

bool strings_are_eq(const char* a, const char* b)
{
    return strcmp(a, b) == 0;
}

void handle_sequence_summary(const pugi::xml_node& node)
{
    for (const auto& child : node.children())
    {
        cout << indent_2 << child.name() << ": ";
        
        if (child.attribute("id") and
            child.attribute("refid") and
            not strings_are_eq(child.attribute("refid").value(), "Message"))
        {
            // special case
            cout << child.attribute("id").value() << " (" << child.attribute("refid").value()
                << " (\"" << child.attribute("idnumeric").value() << "\"))";
            cout << endl;

            continue;
        }

        for (const auto& attr : child.attributes())
        {
            if (strings_are_eq(attr.name(), "refid") or
                strings_are_eq(attr.name(), "id"))
            {
                if (strings_are_eq(attr.value(), "Message"))
                {
                    // skip
                }
                else
                {
                    cout << attr.value() << " ";
                }
            }
            else if (strings_are_eq(attr.name(), "idnumeric"))
            {
                cout << "(\"" << attr.value() << "\")" << " ";
            }
            else if (strings_are_eq(attr.name(), "oms-id"))
            {
                // skip completely 
            }
            else
            {
                cout << attr.name() << " (\"" << attr.value() << "\") ";
            }
        }
        cout << endl;
    }
}

void handle_step_for_AAT(const pugi::xml_document& doc,
                         string refid, string idnumeric)
{
    string path = std::format("/Local/StepDef[@id='{}' and @idnumeric='{}']",
                              refid, idnumeric);
    auto steps_def = doc.select_node(path.c_str());

    if (!steps_def)
    {
        cout << "[ERROR] Cannot find path: " << path << endl;
        std::exit(-1);
    }

    if (is_section_empty(steps_def.node()))
    {
        cout << indent_2 << "<NONE>";
        cout << endl;
    }
    else
    {
        for (const auto& child : steps_def.node())
        {
            handle_sequence_summary(child);
        }
    }
    cout << endl;
}

void handle_step_def(const pugi::xml_document& doc,
                     const pugi::xml_node& node)
{
    auto steps = node.select_nodes("StepDefs/StepDef");

    for (const auto& step : steps)
    {
        auto id = step.node().attribute("id").as_string();
        auto refid = step.node().attribute("refid").as_string();
        auto idnumeric = step.node().attribute("idnumeric").as_string();

        cout << indent_1 << id << ":";
        cout << endl;

        handle_step_for_AAT(doc, refid, idnumeric);
    }
}

void handle_sequence_def(const pugi::xml_document& doc)
{
    auto sequences = doc.select_nodes("/Local/SequenceDef");
    //cout << "[INFO] Found " << sequences.size() << " AAT Sequences" << endl;

    for (const auto& sequence : sequences)
    {
        auto node = sequence.node();

        auto id = node.attribute("id").as_string();
        auto oms_id = node.attribute("oms-id").as_string();

        cout << "## " << id << " (oms-id=\"" << oms_id << "\")";
        cout << endl;
        cout << endl;
        cout << "### Requirements Summary";
        cout << endl;
        cout << endl;
        cout << "TBD";
        cout << endl;
        cout << endl;
        cout << "### Sequence Summary";
        cout << endl;
        cout << endl;

        handle_step_def(doc, node);
    }
}

int main(int argc, const char* argv[])
{
    try
    {
        if (argc != 2)
        {
            cout << "[ERROR] Missing input file" << endl;
            return 1;
        }

        const char* filename = argv[1];

        pugi::xml_document doc{};
        if (auto result = doc.load_file(filename); !result)
        {
            cout << "[ERROR] Cannot load xml file (" << filename << "): " << result.description() << endl;
            return -1;
        }

        handle_sequence_def(doc);
    }
    catch (const std::exception& e)
    {
        cout << "[EXCEP] " << e.what() << endl;
        return -1;
    }

    return 0;
}
