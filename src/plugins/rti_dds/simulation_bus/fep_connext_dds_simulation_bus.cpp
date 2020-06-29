/**
 * @file
 * Copyright &copy; AUDI AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 */
#include "fep_connext_dds_simulation_bus.h"
#include <fep3/base/streamtype/default_streamtype.h>
#include <fep3/base/binary_info/binary_info.h>
#include <fep3/base/environment_variable/environment_variable.h>
#include <fep3/base/properties/properties.h>
#include <fep3/components/configuration/configuration_service_intf.h>
#include <fep3/fep3_participant_version.h>

#include <a_util/result.h>
#include <a_util/filesystem.h>

#include <vector>
#include <cstring>
#include <regex>

#include <dds/dds.hpp>    

#include <plugins/rti_dds/simulation_bus/converter.h>
#include <plugins/rti_dds/simulation_bus/stream_item_topic/stream_item_topic.h>
#include <plugins/rti_dds/simulation_bus/bus_info/bus_info.h>

#include <plugins/rti_dds/simulation_bus/internal_topic/internal_topic.h>

#ifdef WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

using namespace fep3;
using namespace dds::domain;
using namespace dds::core;
using namespace dds::domain::qos;

a_util::filesystem::Path getFilePath()
{
    a_util::filesystem::Path current_binary_file_path;
#ifdef WIN32
    HMODULE hModule = nullptr;
    if(GetModuleHandleEx
        (GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
        , (LPCTSTR)getFilePath
        , &hModule
        ))
    {
        std::vector<wchar_t> file_path_buffer;
        DWORD number_of_copied_characters = 0;
        // note: to support paths with length > MAX_PATH we have do trial-and-error
        // because GetModuleFileName does not indicate if the path was truncated
        while(number_of_copied_characters >= file_path_buffer.size())
        {
            file_path_buffer.resize(file_path_buffer.size() + MAX_PATH);
            number_of_copied_characters = GetModuleFileNameW(hModule, &file_path_buffer[0], static_cast<DWORD>(file_path_buffer.size()));
        }
        file_path_buffer.resize(number_of_copied_characters);
        current_binary_file_path = std::string(file_path_buffer.cbegin(), file_path_buffer.cend());
    }
#else   // WIN32
    Dl_info dl_info;
    dladdr(reinterpret_cast<void*>(getFilePath), &dl_info);
    current_binary_file_path = dl_info.dli_fname;
#endif
    return current_binary_file_path.getParent();
}

class ConnextDDSSimulationBus::Impl
{
public:
    std::unique_ptr<DomainParticipant> _participant;
    std::map<std::string, std::shared_ptr<ITopic>> _topics;
    std::shared_ptr<dds::core::QosProvider> _qos_provider;

    std::unique_ptr<BusInfo> _bus_info;
    
public:
    Impl()
    {

    }

    ~Impl()
    {
        _topics.clear();
        _bus_info.reset();
       
        if (_qos_provider)
        {
            _qos_provider->extensions().unload_profiles();
        }
        if (_participant)
        {
            _participant->close();
        }
    }



    std::shared_ptr<ITopic> getOrCreateTopic(const std::string & topic_name, const IStreamType& stream_type)
    {
        auto entity = _topics.find(topic_name);
        if (entity != _topics.end())
        {
            //@TODO Check IStreamType
            return entity->second;
        }


        auto topic = std::make_shared<StreamItemTopic>(*_participant, topic_name, stream_type, _qos_provider);
        _topics[topic_name] = topic;
        return topic;
    }


    std::shared_ptr<QosProvider> LoadQosProfile()
    {
        auto qos_file_beside_the_binary = getFilePath().append("USER_QOS_PROFILES.xml");
        auto qos_file_beside_the_binary_clean = std::regex_replace(qos_file_beside_the_binary.toString(), std::regex("\\\\"), "/");

        auto qos_libraries = QosProvider::Default().extensions().qos_profile_libraries();
        // If we have already found the fep3 qos library we use it
        if (std::find(qos_libraries.begin(), qos_libraries.end(), "fep3") != qos_libraries.end())
        {
            _qos_provider = std::make_shared<QosProvider>(dds::core::QosProvider::Default());
        }
        // If not we search beside the simulation bus binary 
        else if (a_util::filesystem::exists(qos_file_beside_the_binary_clean))
        {
            _qos_provider = std::make_shared<QosProvider>(qos_file_beside_the_binary_clean);
        }
        else
        {
            _qos_provider = std::make_shared<QosProvider>(dds::core::QosProvider::Default());
        }
        return _qos_provider;
    }
    
    void initBusInfo(DomainParticipantQos &participant_qos, const std::string participant_name)
    {
        // Create BusInfo to collect for bus informations
        _bus_info = std::make_unique<BusInfo>();
        _bus_info->getOwnParticipantInfo()->setParticipantName(participant_name);

        BusInfo::Version version;
        version.major = FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR;
        version.minor = FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR;
        version.patch = FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH;

        _bus_info->getOwnParticipantInfo()->setFepVersion(version);
        _bus_info->registerUserData(participant_qos);

        // Create build in topic to make bus informations available via ISimulationBus
        auto buildin_topic_businfo = std::make_shared<InternalTopic>("_buildin_topic_businfo");
        _topics["_buildin_topic_businfo"] = buildin_topic_businfo;
        _bus_info->setUpdateCallback([this, buildin_topic_businfo]()
        {
            if (_bus_info)
            {
                buildin_topic_businfo->write(_bus_info->asJson());
            }
        });
    }
};

ConnextDDSSimulationBus::ConnextDDSSimulationBus() : _impl(std::make_unique<ConnextDDSSimulationBus::Impl>())
{

}

ConnextDDSSimulationBus::~ConnextDDSSimulationBus()
{
}

fep3::Result ConnextDDSSimulationBus::create()
{
    std::shared_ptr<const IComponents> components = _components.lock();
    if (components)
    {
        auto logging_service = components->getComponent<ILoggingService>();
        if (logging_service)
        {
            _logger = logging_service->createLogger("connext_dds_simulation_bus.component");
        }

        auto configuration_service = components->getComponent<IConfigurationService>();
        if (configuration_service)
        {
            _simulation_bus_configuration.initConfiguration(*configuration_service);
        }
    }
    return {};
}

fep3::Result ConnextDDSSimulationBus::destroy()
{
    _simulation_bus_configuration.deinitConfiguration();
    return {};
}

fep3::Result ConnextDDSSimulationBus::initialize()
{
    auto qos_libraries = _impl->LoadQosProfile()->extensions().qos_profile_libraries();
    if (std::find(qos_libraries.begin(), qos_libraries.end(), "fep3") == qos_libraries.end())
    {
        RETURN_ERROR_DESCRIPTION(fep3::ERR_NOT_FOUND, "Could not find fep3 library in USER_QOS_PROFILES.xml. \n"
            "Please make sure your application has access to the predefined USER_QOS_PROFILES.xml from fep3. \n"
            "See documentation for more information");
    }
    _simulation_bus_configuration.updatePropertyVariables();
    uint32_t domain_id = _simulation_bus_configuration._participant_domain;
    
    auto participant_qos = _impl->_qos_provider->participant_qos("fep3::participant");

    _impl->initBusInfo(participant_qos, _simulation_bus_configuration._participant_name);

#ifdef WIN32
    //in windows the rtimonitoring is loaded lazy ... so we need to change working dir here for
    //creating time
    auto orig_wd = a_util::filesystem::getWorkingDirectory();
    auto res = a_util::filesystem::setWorkingDirectory(getFilePath());
    if (isFailed(res))
    {
        orig_wd = getFilePath();
    }
#endif
    try
    {
        _impl->_participant = std::make_unique<DomainParticipant>
            (domain_id
             , participant_qos);
        _impl->_bus_info->registerParticipant(*_impl->_participant);
    }
    catch (const std::exception& ex)
    {
#ifdef WIN32
        a_util::filesystem::setWorkingDirectory(orig_wd);
#endif
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, ex.what());
    }
    
#ifdef WIN32
    a_util::filesystem::setWorkingDirectory(orig_wd);
#endif
        
    return {};
}

fep3::Result ConnextDDSSimulationBus::deinitialize()
{
    _impl->_bus_info->unregisterParticipant(*_impl->_participant);
    _impl->_bus_info = nullptr;

    _impl->_topics.clear();
    if (_impl->_qos_provider)
    {
        _impl->_qos_provider->extensions().unload_profiles();
    }
    
    if (_impl->_participant)
    {
        _impl->_participant->close();
    }

    _impl->_participant.reset();

    return {};
}

bool ConnextDDSSimulationBus::isSupported(const IStreamType& stream_type) const
{
    return
        (meta_type_raw == stream_type
        || (meta_type_audio == stream_type)
        || (meta_type_ddl == stream_type)
        || (meta_type_plain == stream_type)
        || (meta_type_string == stream_type)
        || (meta_type_plain == stream_type)
        );
}

std::unique_ptr<ISimulationBus::IDataReader> ConnextDDSSimulationBus::getReader
    (const std::string& name
    , const IStreamType& stream_type
)
{
    try
    {
        auto topic = _impl->getOrCreateTopic(name, stream_type);
        return topic->createDataReader(0);
    }
    catch (Exception & exception)
    {
        logError(convertExceptionToResult(exception));
    }
    return nullptr;
}

std::unique_ptr<ISimulationBus::IDataReader> ConnextDDSSimulationBus::getReader
    (const std::string& name
    , const IStreamType& stream_type
    , size_t queue_capacity
)
{
    try
    {
        auto topic = _impl->getOrCreateTopic(name, stream_type);
        return topic->createDataReader(queue_capacity);
    }
    catch (Exception & exception)
    {
        logError(convertExceptionToResult(exception));
    }
    catch (std::exception& exception)
    {
        logError(convertExceptionToResult(exception));
    }
    return nullptr;
}

std::unique_ptr<ISimulationBus::IDataReader> ConnextDDSSimulationBus::getReader(const std::string& name)
{
    try
    {
        auto topic = _impl->getOrCreateTopic(name, fep3::arya::StreamTypeRaw());
        return topic->createDataReader(0);
    }
    catch (Exception & exception)
    {
        logError(convertExceptionToResult(exception));
    }
    return nullptr;
}
std::unique_ptr<ISimulationBus::IDataReader> ConnextDDSSimulationBus::getReader(const std::string& name, size_t queue_capacity)
{
    try
    {
        auto topic = _impl->getOrCreateTopic(name, fep3::arya::StreamTypeRaw());
        return topic->createDataReader(queue_capacity);
    }
    catch (Exception & exception)
    {
        logError(convertExceptionToResult(exception));
    }
    return nullptr;
}
std::unique_ptr<ISimulationBus::IDataWriter> ConnextDDSSimulationBus::getWriter
    (const std::string& name
    , const IStreamType& stream_type
)
{
    try
    {
        auto topic = _impl->getOrCreateTopic(name, stream_type);
        return topic->createDataWriter(0);
    }
    catch (Exception & exception)
    {
        logError(convertExceptionToResult(exception));
    }
    return nullptr;
}
std::unique_ptr<ISimulationBus::IDataWriter> ConnextDDSSimulationBus::getWriter
    (const std::string& name
    , const IStreamType& stream_type
    , size_t queue_capacity
)
{
    try
    {
        auto topic = _impl->getOrCreateTopic(name, stream_type);
        return topic->createDataWriter(queue_capacity);
    }
    catch (Exception & exception)
    {
        logError(convertExceptionToResult(exception));
    }
    return nullptr;
}
std::unique_ptr<ISimulationBus::IDataWriter> ConnextDDSSimulationBus::getWriter(const std::string& name)
{
    try
    {
        auto topic = _impl->getOrCreateTopic(name, fep3::arya::StreamTypeRaw());
        return topic->createDataWriter(0);
    }
    catch (Exception & exception)
    {
        logError(convertExceptionToResult(exception));
    }
    return nullptr;
}
std::unique_ptr<ISimulationBus::IDataWriter> ConnextDDSSimulationBus::getWriter(const std::string& name, size_t queue_capacity)
{
    try
    {
        auto topic = _impl->getOrCreateTopic(name, fep3::arya::StreamTypeRaw());
        return topic->createDataWriter(queue_capacity);
    }
    catch (Exception & exception)
    {
        logError(convertExceptionToResult(exception));
    }
    return nullptr;
}

std::shared_ptr<dds::core::QosProvider> ConnextDDSSimulationBus::getQOSProfile() const
{
    return _impl->_qos_provider;
}

void ConnextDDSSimulationBus::logError(const fep3::Result& res)
{
    if (_logger)
    {
        if (_logger->isErrorEnabled())
        {
            _logger->logError(a_util::result::toString(res));
        }
    }
}

ConnextDDSSimulationBus::ConnextDDSSimulationBusConfiguration::ConnextDDSSimulationBusConfiguration()
    : Configuration("rti_dds_simulation_bus")
{
}

fep3::Result ConnextDDSSimulationBus::ConnextDDSSimulationBusConfiguration::registerPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(registerPropertyVariable(_participant_domain, "participant_domain"));
    FEP3_RETURN_IF_FAILED(registerPropertyVariable(_participant_name, "participant_name"));
    
    return {};
}

fep3::Result ConnextDDSSimulationBus::ConnextDDSSimulationBusConfiguration::unregisterPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(unregisterPropertyVariable(_participant_domain, "participant_domain"));
    FEP3_RETURN_IF_FAILED(unregisterPropertyVariable(_participant_name, "participant_name"));

    return {};
}
