/**
 * Implementation of ElementManager
 *
 * @file
 * Copyright &copy; AUDI AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 */

#include <functional>
#include <memory>

#include "element_manager.h"

namespace fep3
{
namespace arya
{

ElementManager::ElementManager(const std::shared_ptr<const IElementFactory>& element_factory)
    : _element_factory(element_factory)
{}
    
ElementManager::~ElementManager()
{}

Result ElementManager::loadElement(const IComponents& components)
{
    try
    {
        if (_element_factory)
        {
            _element = _element_factory->createElement(components);
            if (_element)
            {
                return _element->loadElement(components);
            }
            else
            {
                RETURN_ERROR_DESCRIPTION(ERR_POINTER, "creating element failed");
            }
        }
        else
        {
            RETURN_ERROR_DESCRIPTION(ERR_POINTER, "the element factory is invalid");
        }
    }
    catch (const std::exception& ex)
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, ex.what());
    }
}

void ElementManager::unloadElement()
{
    try
    {
        _element->unloadElement();
    }
    catch (const std::exception&)
    {
        //this should be transformed to a error to get a good message out there
        //RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, ex.what());
    }
    _element.reset();
}

Result ElementManager::initializeElement()
{
    try
    {
        return _element->initialize();
    }
    catch (const std::exception& ex)
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, ex.what());
    }
}

void ElementManager::deinitializeElement()
{
    try
    {
        _element->deinitialize();
    }
    catch (const std::exception&)
    {
        //this should be transformed to a error to get a good message out there
        //RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, ex.what());
    }
}

Result ElementManager::runElement()
{
    try
    {
        return _element->run();
    }
    catch (const std::exception& ex)
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, ex.what());
    }
}

void ElementManager::stopElement()
{
    try
    {
        _element->stop();
    }
    catch (const std::exception&)
    {
        //this should be transformed to a error to get a good message out there
        //RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, ex.what());
    }
}


} // namespace arya
} // namespace fep3
