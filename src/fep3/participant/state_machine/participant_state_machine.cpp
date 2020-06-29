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

// if defined, the boost participant state machine implementation is used
// if not defined, a non-functional (but compilable) dummy state machine implementation is used, that has no depenedency on boost
#define USE_BOOST_PARTICIPANT_STATE_MACHINE_IMPLEMENTATION

#ifdef USE_BOOST_PARTICIPANT_STATE_MACHINE_IMPLEMENTATION
// Boost MSM produces warning "conditional expression is constant"
#pragma warning(disable:4127)

#include <boost/msm/front/euml/euml.hpp>
#include <boost/msm/front/euml/state_grammar.hpp>
#include <boost/msm/back/state_machine.hpp>
#endif // USE_BOOST_PARTICIPANT_STATE_MACHINE_IMPLEMENTATION

#include <fep3/participant/element_manager/element_manager.h>
#include "participant_state_machine.h"
#include <a_util/result/result_util.h>
#include <iostream>


#define LOG_INFO_OR_ERROR_TO_LOGGER_OR_COUT(logger_used, log_message, res_code) \
{ \
if (fep3::isFailed(res_code)) \
{ \
    if (logger_used) \
    { \
        if (logger_used->isErrorEnabled()) \
        { \
            logger_used->logError(std::string("Error while ") + std::string(log_message) + a_util::result::toString(res_code)); \
        } \
    } \
    else \
    { \
        std::cerr << std::string("Error while ") << log_message; \
        std::cerr << a_util::result::toString(res_code); \
        std::cerr << std::endl; \
    } \
} \
else \
{ \
    if (logger_used) \
    { \
        if (logger_used->isInfoEnabled()) \
        { \
            logger_used->logInfo(std::string("Successfully ") + std::string(log_message)); \
        } \
    } \
    else \
    { \
        std::cout << std::string("Successfully ") + log_message; \
        std::cerr << std::endl; \
    } \
} \
}

namespace fep3
{
#ifdef USE_BOOST_PARTICIPANT_STATE_MACHINE_IMPLEMENTATION
namespace participant_state_machine
{
namespace arya
{

namespace msm = boost::msm;
using namespace boost::msm::front::euml;

template<typename StateMachine>
std::string getStateName(int id)
{
    return StateMachine::getStateName(id);
}

// attributes
class CurrentStateNameGetter;
BOOST_MSM_EUML_DECLARE_ATTRIBUTE(CurrentStateNameGetter, _current_state_name_getter)
BOOST_MSM_EUML_DECLARE_ATTRIBUTE(fep3::arya::ElementManager, _element_manager)
BOOST_MSM_EUML_DECLARE_ATTRIBUTE(std::shared_ptr<ComponentRegistry>, _component_registry)
BOOST_MSM_EUML_DECLARE_ATTRIBUTE(std::shared_ptr<ILoggingService::ILogger>, _participant_logger)

// generic actions
/**
 * Class providing an action to update the current state name of the templated StateMachine
 *
*/
template<typename StateMachine>
struct UpdateCurrentStateName
{
    BOOST_MSM_EUML_ACTION(action)
    {
        template<typename Event, typename Fsm, typename SourceState, typename TargetState>
        void operator()(const Event&, Fsm& fsm, SourceState&, TargetState&) const
        {
            fsm.get_attribute(_current_state_name_getter).update(getStateName<StateMachine>(*fsm.current_state()));
        }
    };
};

// events
BOOST_MSM_EUML_EVENT(update_current_state_name)
BOOST_MSM_EUML_EVENT(load)
BOOST_MSM_EUML_EVENT(unload)
BOOST_MSM_EUML_EVENT(exit)
BOOST_MSM_EUML_EVENT(initialize)
BOOST_MSM_EUML_EVENT(deinitialize)
BOOST_MSM_EUML_EVENT(start)
BOOST_MSM_EUML_EVENT(pause)
BOOST_MSM_EUML_EVENT(stop)

/**
 * Class to get the current state name
 * @note The mechanism to get the state name is a bit complicated, because states of the eUML front end of Boost MSM are not visitable.
 *       If ever the front end is changed to the functor front end, the state names can be get easier by using visitors of (current) states.
 *
*/
class CurrentStateNameGetter
{
private:
    class InternalGetter
    {
    public:
        template<typename StateMachine>
        void operator()(StateMachine& state_machine)
        {
            state_machine.process_event(update_current_state_name);
        }
        void update(const std::string& new_name);
        std::shared_ptr<std::string> createString(const std::string& default_name);
    private:
        // note: no lifetime control of the state name string
        std::weak_ptr<std::string> _state_name_pointer;
    };

public:
    CurrentStateNameGetter();

    template<typename StateMachine>
    std::string operator()(const std::string& default_name, StateMachine& state_machine)
    {
        std::shared_ptr<std::string> state_name_pointer;
        if(_internal_getter)
        {
            state_name_pointer = _internal_getter->createString(default_name);
            _internal_getter->operator()(state_machine);
        }
        return state_name_pointer ? *state_name_pointer : default_name;
    }
    void update(const std::string& new_name);
private:
    // the internal getter shared among (copied) instances of CurrentStateNameGetter
    std::shared_ptr<InternalGetter> _internal_getter;
};

/**
 * state machine "main" performing module operations (loading, unloading , etc.)
 *
*/
class MainStateMachine
{
private:
    using this_type = MainStateMachine;

    // states
    BOOST_MSM_EUML_STATE((), Unloaded)
    BOOST_MSM_EUML_STATE((), Loaded)
    BOOST_MSM_EUML_STATE((), Initialized)
    BOOST_MSM_EUML_STATE((), Running)
    BOOST_MSM_EUML_STATE((), Paused)
    BOOST_MSM_EUML_TERMINATE_STATE((), Final)

    // actions
    BOOST_MSM_EUML_ACTION(load_element)
    {
        template<typename Event, typename Fsm, typename SourceState, typename TargetState>
        bool operator()(const Event&, Fsm& fsm, SourceState&, TargetState&) const
        {
            if(const auto& component_registry = fsm.get_attribute(_component_registry))
            {
                const auto& logger = fsm.get_attribute(_participant_logger);
                // pass reference to the components to the element manager factory
                // Note: The element must not store the components as a whole, but it may
                //       pick certain components (via IComponents::getComponent) and store a pointer to those.
                auto res = fsm.get_attribute(_element_manager).loadElement(*component_registry.get());
                LOG_INFO_OR_ERROR_TO_LOGGER_OR_COUT(logger, "loading element ", res);
                return isOk(res);
            }
            return false;
        }
    };
    BOOST_MSM_EUML_ACTION(unload_element)
    {
        template<typename Event, typename Fsm, typename SourceState, typename TargetState>
        void operator()(const Event&, Fsm& fsm, SourceState&, TargetState&) const
        {
            const auto& logger = fsm.get_attribute(_participant_logger);
            fsm.get_attribute(_element_manager).unloadElement();
            LOG_INFO_OR_ERROR_TO_LOGGER_OR_COUT(logger, "unloading element ", fep3::Result());
        }
    };
    BOOST_MSM_EUML_ACTION(initialize_element)
    {
        template<typename Event, typename Fsm, typename SourceState, typename TargetState>
        bool operator()(const Event&, Fsm& fsm, SourceState&, TargetState&) const
        {
            const auto& logger = fsm.get_attribute(_participant_logger);
            auto res = fsm.get_attribute(_element_manager).initializeElement();
            LOG_INFO_OR_ERROR_TO_LOGGER_OR_COUT(logger, "initializing element ", res);
            return isOk(res);
        }
    };
    BOOST_MSM_EUML_ACTION(initialize_components)
    {
        template<typename Event, typename Fsm, typename SourceState, typename TargetState>
        bool operator()(const Event&, Fsm& fsm, SourceState&, TargetState&) const
        {
            const auto& logger = fsm.get_attribute(_participant_logger);
            if(const auto& component_registry = fsm.get_attribute(_component_registry))
            {
                auto res = component_registry->initialize();
                LOG_INFO_OR_ERROR_TO_LOGGER_OR_COUT(logger, "initializing components ", res);
                return isOk(res);
            }
            return false;
        }
    };
    BOOST_MSM_EUML_ACTION(tense_components)
    {
        template<typename Event, typename Fsm, typename SourceState, typename TargetState>
        bool operator()(const Event&, Fsm& fsm, SourceState&, TargetState&) const
        {
            const auto& logger = fsm.get_attribute(_participant_logger);
            if(const auto& component_registry = fsm.get_attribute(_component_registry))
            {
                auto res = component_registry->tense();
                LOG_INFO_OR_ERROR_TO_LOGGER_OR_COUT(logger, "tensing components ", res);
                return isOk(res);
            }
            return false;
        }
    };
    BOOST_MSM_EUML_ACTION(relax_components)
    {
        template<typename Event, typename Fsm, typename SourceState, typename TargetState>
        bool operator()(const Event&, Fsm& fsm, SourceState&, TargetState&) const
        {
            const auto& logger = fsm.get_attribute(_participant_logger);
            if (const auto& component_registry = fsm.get_attribute(_component_registry))
            {
                auto res = component_registry->relax();
                LOG_INFO_OR_ERROR_TO_LOGGER_OR_COUT(logger, "relaxing components ", res);
                return isOk(res);
            }
            return false;
        }
    };
    BOOST_MSM_EUML_ACTION(deinitialize_element)
    {
        template<typename Event, typename Fsm, typename SourceState, typename TargetState>
        void operator()(const Event&, Fsm& fsm, SourceState&, TargetState&) const
        {
            const auto& logger = fsm.get_attribute(_participant_logger);
            fsm.get_attribute(_element_manager).deinitializeElement();
            LOG_INFO_OR_ERROR_TO_LOGGER_OR_COUT(logger, "deinitializing element ", fep3::Result());
        }
    };
    BOOST_MSM_EUML_ACTION(deinitialize_components)
    {
        template<typename Event, typename Fsm, typename SourceState, typename TargetState>
        void operator()(const Event&, Fsm& fsm, SourceState&, TargetState&) const
        {
            const auto& logger = fsm.get_attribute(_participant_logger);
            if(const auto& component_registry = fsm.get_attribute(_component_registry))
            {
                auto res = component_registry->deinitialize();
                LOG_INFO_OR_ERROR_TO_LOGGER_OR_COUT(logger, "deinitializing components ", res);
            }
        }
    };
    BOOST_MSM_EUML_ACTION(run_element)
    {
        template<typename Event, typename Fsm, typename SourceState, typename TargetState>
        bool operator()(const Event&, Fsm& fsm, SourceState&, TargetState&) const
        {
            const auto& logger = fsm.get_attribute(_participant_logger);
            auto res = fsm.get_attribute(_element_manager).runElement();
            LOG_INFO_OR_ERROR_TO_LOGGER_OR_COUT(logger, "running element ", res);
            return isOk(res);
        }
    };
    BOOST_MSM_EUML_ACTION(start_components)
    {
        template<typename Event, typename Fsm, typename SourceState, typename TargetState>
        bool operator()(const Event&, Fsm& fsm, SourceState&, TargetState&) const
        {
            const auto& logger = fsm.get_attribute(_participant_logger);
            if(const auto& component_registry = fsm.get_attribute(_component_registry))
            {
                auto res = component_registry->start();
                LOG_INFO_OR_ERROR_TO_LOGGER_OR_COUT(logger, "starting components ", res);
                return isOk(res);
            }
            return false;
        }
    };
    BOOST_MSM_EUML_ACTION(pause_components)
    {
        template<typename Event, typename Fsm, typename SourceState, typename TargetState>
        bool operator()(const Event&, Fsm& fsm, SourceState&, TargetState&) const
        {
            const auto& logger = fsm.get_attribute(_participant_logger);
            if(const auto& component_registry = fsm.get_attribute(_component_registry))
            {
                auto res = component_registry->pause();
                LOG_INFO_OR_ERROR_TO_LOGGER_OR_COUT(logger, "pausing components ", res);
                return isOk(res);
            }
            return false;
        }
    };
    BOOST_MSM_EUML_ACTION(stop_element)
    {
        template<typename Event, typename Fsm, typename SourceState, typename TargetState>
        void operator()(const Event&, Fsm& fsm, SourceState&, TargetState&) const
        {
            const auto& logger = fsm.get_attribute(_participant_logger);
            fsm.get_attribute(_element_manager).stopElement();
            LOG_INFO_OR_ERROR_TO_LOGGER_OR_COUT(logger, "stopping element ", fep3::Result());
        }
    };
    BOOST_MSM_EUML_ACTION(stop_components)
    {
        template<typename Event, typename Fsm, typename SourceState, typename TargetState>
        void operator()(const Event&, Fsm& fsm, SourceState&, TargetState&) const
        {
            const auto& logger = fsm.get_attribute(_participant_logger);
            if(const auto& component_registry = fsm.get_attribute(_component_registry))
            {
                auto res = component_registry->stop();
                LOG_INFO_OR_ERROR_TO_LOGGER_OR_COUT(logger, "stopping components ", res);
            }
        }
    };

    // transition table
    BOOST_MSM_EUML_TRANSITION_TABLE
        ((Unloaded + update_current_state_name / UpdateCurrentStateName<this_type>::action
        , Unloaded + exit == Final
        , Unloaded + load [load_element] == Loaded
        , Loaded + update_current_state_name / UpdateCurrentStateName<this_type>::action
        , Loaded + unload / unload_element == Unloaded
        // the element must be initialized before the component registry is being initialized
        , Loaded + initialize [initialize_element && initialize_components && tense_components] == Initialized
        , Initialized + update_current_state_name / UpdateCurrentStateName<this_type>::action
        , Initialized + start [run_element && start_components] == Running
        , Initialized + pause [run_element && pause_components] == Paused
        , Initialized + deinitialize / (relax_components, deinitialize_components, deinitialize_element) == Loaded
        , Running + update_current_state_name / UpdateCurrentStateName<this_type>::action
        // the component registry must be stopped before the element is being stopped
        , Running + stop / (stop_components, stop_element) == Initialized
        , Running + pause [pause_components] == Paused
        , Paused + update_current_state_name / UpdateCurrentStateName<this_type>::action
        , Paused + stop / (stop_components, stop_element) == Initialized
        , Paused + start [start_components] == Running
        ), main_transition_table)
public:
    static std::string getStateName(int id)
    {
        switch(id)
        {
            // numbering according to appearance in BOOST_MSM_EUML_TRANSITION_TABLE
            case 0: return "Unloaded";
            case 1: return "Loaded";
            case 2: return "Initialized";
            case 3: return "Running";
            case 4: return "Paused";
            default: return "Unknown";
        }
    }

public:
    // state machine
    BOOST_MSM_EUML_DECLARE_STATE_MACHINE
        ((main_transition_table
        , init_ << Unloaded
        , no_action
        , no_action
        , attributes_ << _current_state_name_getter << _element_manager << _component_registry << _participant_logger
        ), StateMachine)
};

void CurrentStateNameGetter::InternalGetter::update(const std::string& new_name)
{
    if(const auto& state_name_pointer = _state_name_pointer.lock())
    {
        *state_name_pointer = new_name;
    }
}
std::shared_ptr<std::string> CurrentStateNameGetter::InternalGetter::createString(const std::string& default_name)
{
    const auto& state_name_pointer = std::make_shared<std::string>(default_name);
    _state_name_pointer = state_name_pointer;
    return state_name_pointer;
}

CurrentStateNameGetter::CurrentStateNameGetter()
    : _internal_getter(std::make_shared<InternalGetter>())
{}

auto CurrentStateNameGetter::update(const std::string& new_name) -> void
{
    if(_internal_getter)
    {
        _internal_getter->update(new_name);
    }
}

} // namespace arya
using arya::MainStateMachine;
// state machine attributes
using arya::_element_manager;
using arya::_component_registry;
using arya::_current_state_name_getter;
using arya::_participant_logger;
// events
using arya::load;
using arya::unload;
using arya::exit;
using arya::initialize;
using arya::deinitialize;
using arya::start;
using arya::pause;
using arya::stop;
} // namespace participant_state_machine
#endif // USE_BOOST_PARTICIPANT_STATE_MACHINE_IMPLEMENTATION

namespace arya
{

#ifdef USE_BOOST_PARTICIPANT_STATE_MACHINE_IMPLEMENTATION
class ParticipantStateMachine::Impl
    : public boost::msm::back::state_machine<participant_state_machine::MainStateMachine::StateMachine>
{
public:
    using BackEnd = boost::msm::back::state_machine<participant_state_machine::MainStateMachine::StateMachine>;
};

ParticipantStateMachine::ParticipantStateMachine
    (ElementManager element_manager
    , const std::shared_ptr<ComponentRegistry>& component_registry
    , const std::shared_ptr<ILoggingService::ILogger>& participant_logger
    )
    : _impl(std::make_unique<Impl>())
{
    _impl->get_attribute(participant_state_machine::_element_manager) = std::move(element_manager);
    _impl->get_attribute(participant_state_machine::_component_registry) = component_registry;
    _impl->get_attribute(participant_state_machine::_participant_logger) = participant_logger;
}

ParticipantStateMachine::~ParticipantStateMachine() = default;

bool ParticipantStateMachine::isFinalized() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _finalized;
}

std::string ParticipantStateMachine::getCurrentStateName() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _impl->get_attribute(participant_state_machine::_current_state_name_getter)("Unknown", static_cast<Impl::BackEnd&>(*(_impl.get())));
}

bool ParticipantStateMachine::load()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return (boost::msm::back::HANDLED_TRUE == _impl->process_event(participant_state_machine::load));
}

bool ParticipantStateMachine::unload()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return (boost::msm::back::HANDLED_TRUE == _impl->process_event(participant_state_machine::unload));
}

bool ParticipantStateMachine::exit()
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (boost::msm::back::HANDLED_TRUE == _impl->process_event(participant_state_machine::exit))
    {
        _finalized = true;
    }
    return _finalized;
}

bool ParticipantStateMachine::initialize()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return (boost::msm::back::HANDLED_TRUE == _impl->process_event(participant_state_machine::initialize));
}

bool ParticipantStateMachine::deinitialize()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return (boost::msm::back::HANDLED_TRUE == _impl->process_event(participant_state_machine::deinitialize));
}

bool ParticipantStateMachine::stop()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return (boost::msm::back::HANDLED_TRUE == _impl->process_event(participant_state_machine::stop));
}

bool ParticipantStateMachine::start()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return (boost::msm::back::HANDLED_TRUE == _impl->process_event(participant_state_machine::start));
}

bool ParticipantStateMachine::pause()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return (boost::msm::back::HANDLED_TRUE == _impl->process_event(participant_state_machine::pause));
}

#else // USE_BOOST_PARTICIPANT_STATE_MACHINE_IMPLEMENTATION
class ParticipantStateMachine::Impl
{};

ParticipantStateMachine::ParticipantStateMachine
    (ElementManager
    , const std::shared_ptr<ComponentRegistry>&
    )
{}

ParticipantStateMachine::~ParticipantStateMachine() = default;

bool ParticipantStateMachine::isFinalized() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _finalized;
}

std::string ParticipantStateMachine::getCurrentStateName() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return "dummy";
}

bool ParticipantStateMachine::load()
{
    std::lock_guard<std::mutex> lock(_mutex);

    return false;
}

bool ParticipantStateMachine::unload()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return false;
}

bool ParticipantStateMachine::exit()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _finalized = true;
    return _finalized;
}

bool ParticipantStateMachine::initialize()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return false;
}

bool ParticipantStateMachine::deinitialize()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return false;
}

bool ParticipantStateMachine::stop()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return false;
}

bool ParticipantStateMachine::start()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return false;
}

bool ParticipantStateMachine::pause()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return false;
}

#endif // USE_BOOST_PARTICIPANT_STATE_MACHINE_IMPLEMENTATION

} // namespace arya
} // namespace fep3