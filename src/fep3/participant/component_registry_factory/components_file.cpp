/**
 * @file
 * @copyright AUDI AG
 *            All right reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 */

#include "components_file.h"
#include <a_util/xml.h>
#include <a_util/filesystem.h>

namespace fep3
{
namespace arya
{

    ComponentsFile::ComponentsFile()
    {
    }
    void ComponentsFile::load(const std::string& file)
    {
        { //reset
            _items.clear();
            _current_path.clear();
        }

        a_util::xml::DOM loaded_file;
        if (!loaded_file.load(file))
        {
            auto error_message = a_util::strings::format("can not loaded %s - Error : %s",
                file.c_str(),
                loaded_file.getLastError().c_str());
            throw std::runtime_error(error_message);
        }
            
        a_util::xml::DOMElement schema_version;
        if (loaded_file.getRoot().findNode("schema_version", schema_version))
        {
            auto schema_version_string = schema_version.getData();
            if (a_util::strings::trim(schema_version_string) == "1.0.0")
            {
               
            }
            else
            {
                auto error_message = a_util::strings::format("can not loaded %s - Error : wrong schema version found : expect %s - found %s",
                    file.c_str(),
                    "1.0.0",
                    schema_version_string.c_str());
                throw std::runtime_error(error_message);
            }
        }
        else
        {
            auto error_message = a_util::strings::format("can not loaded %s - Error : %s",
                file.c_str(),
                "no schema version tag found");
            throw std::runtime_error(error_message);
        }


        { //sets the path
            a_util::filesystem::Path filepath = file;
            if (filepath.isRelative())
            {
                filepath = a_util::filesystem::getWorkingDirectory().append(filepath);
                filepath.makeCanonical();
            }
            _current_path = filepath;
        }

        a_util::xml::DOMElementList comps;
        if (loaded_file.getRoot().findNodes("component", comps))
        {
            { //validate it 

                for (const auto& comp_node : comps)
                {
                    auto source_node = comp_node.getChild("source");
                    auto comp_iid = comp_node.getChild("iid");
                   
                    if (source_node.isNull())
                    {
                        auto error_message = a_util::strings::format("can not loaded %s - Error : %s",
                            file.c_str(),
                            "no iid node for component tag found");
                        throw std::runtime_error(error_message);
                    }
                    if (comp_iid.isNull())
                    {
                        auto error_message = a_util::strings::format("can not loaded %s - Error : %s",
                            file.c_str(),
                            "no source node for component tag found");
                        throw std::runtime_error(error_message);
                    }
                }
            }
            { // fill items
                for (const auto& comp_node : comps)
                {
                    auto source_node = comp_node.getChild("source");
                    auto comp_iid = comp_node.getChild("iid").getData();
                    auto source_type = fep3::arya::getComponentSourceType(source_node.getAttribute("type"));
                    auto source_file_string = source_node.getData();
                    a_util::strings::trim(source_file_string);
                    a_util::filesystem::Path source_file;
                    if (!source_file_string.empty())
                    {
                        source_file = source_file_string;
                    }
                    if (!source_file.isEmpty() && source_file.isRelative())
                    {
                        //we make it relative to the File! (not the workingdirectory!!)
                        source_file = a_util::filesystem::Path(_current_path).getParent().append(source_file);
                        source_file.makeCanonical();
                    }
                    _items.push_back({ comp_iid, source_type, source_file });
                }
            }
        }
        else
        {
            //everything is fine, but there are no components in the file defined
        }
    }
    std::vector<std::string> ComponentsFile::getFiles(ComponentSourceType source_type) const
    {
        std::vector<std::string> files;
        for (const auto& item : _items)
        {
            if (item._comp_source_type == source_type)
            {
                if (files.size() == 0 ||
                    (files.end() == std::find(files.begin(), files.end(), item._comp_source_file_reference)))
                {
                    files.push_back(item._comp_source_file_reference);
                } //else it is already in the list ... no need to load it again
            }
        }
        return files;
    }

    const std::vector<ComponentsFile::ComponentItem> ComponentsFile::getItems() const
    {
        return _items;
    }
    std::string ComponentsFile::getCurrentPath() const
    {
        return _current_path;
    }
}
}