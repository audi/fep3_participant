

/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from stream_types.idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Connext distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Connext manual.
*/

#ifndef stream_types_783819995_hpp
#define stream_types_783819995_hpp

#include <iosfwd>

#if (defined(RTI_WIN32) || defined (RTI_WINCE) || defined(RTI_INTIME)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, start exporting symbols.
*/
#undef RTIUSERDllExport
#define RTIUSERDllExport __declspec(dllexport)
#endif

#pragma warning( push )
#pragma warning( disable : 4245)
#include "dds/domain/DomainParticipant.hpp"
#pragma warning( pop )

#include "dds/topic/TopicTraits.hpp"
#include "dds/core/SafeEnumeration.hpp"
#include "dds/core/String.hpp"
#include "dds/core/array.hpp"
#include "dds/core/vector.hpp"
#include "dds/core/Optional.hpp"
#include "dds/core/xtypes/DynamicType.hpp"
#include "dds/core/xtypes/StructType.hpp"
#include "dds/core/xtypes/UnionType.hpp"
#include "dds/core/xtypes/EnumType.hpp"
#include "dds/core/xtypes/AliasType.hpp"
#include "rti/core/array.hpp"
#include "rti/core/BoundedSequence.hpp"
#include "rti/util/StreamFlagSaver.hpp"
#include "rti/domain/PluginSupport.hpp"
#include "rti/core/LongDouble.hpp"
#include "dds/core/External.hpp"
#include "rti/core/Pointer.hpp"
#include "rti/topic/TopicTraits.hpp"

#if (defined(RTI_WIN32) || defined (RTI_WINCE) || defined(RTI_INTIME)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, stop exporting symbols.
*/
#undef RTIUSERDllExport
#define RTIUSERDllExport
#endif

#if (defined(RTI_WIN32) || defined (RTI_WINCE) || defined(RTI_INTIME)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, start exporting symbols.
*/
#undef NDDSUSERDllExport
#define NDDSUSERDllExport __declspec(dllexport)
#endif

namespace fep3 {
    namespace ddstypes {

        class NDDSUSERDllExport Property {
          public:
            Property();

            Property(
                const std::string& name,
                const std::string& type,
                const std::string& value);

            #ifdef RTI_CXX11_RVALUE_REFERENCES
            #ifndef RTI_CXX11_NO_IMPLICIT_MOVE_OPERATIONS
            Property (Property&&) = default;
            Property& operator=(Property&&) = default;
            Property& operator=(const Property&) = default;
            Property(const Property&) = default;
            #else
            Property(Property&& other_) OMG_NOEXCEPT;  
            Property& operator=(Property&&  other_) OMG_NOEXCEPT;
            #endif
            #endif 

            std::string& name() OMG_NOEXCEPT {
                return m_name_;
            }

            const std::string& name() const OMG_NOEXCEPT {
                return m_name_;
            }

            void name(const std::string& value) {
                m_name_ = value;
            }

            std::string& type() OMG_NOEXCEPT {
                return m_type_;
            }

            const std::string& type() const OMG_NOEXCEPT {
                return m_type_;
            }

            void type(const std::string& value) {
                m_type_ = value;
            }

            std::string& value() OMG_NOEXCEPT {
                return m_value_;
            }

            const std::string& value() const OMG_NOEXCEPT {
                return m_value_;
            }

            void value(const std::string& value) {
                m_value_ = value;
            }

            bool operator == (const Property& other_) const;
            bool operator != (const Property& other_) const;

            void swap(Property& other_) OMG_NOEXCEPT ;

          private:

            std::string m_name_;
            std::string m_type_;
            std::string m_value_;

        };

        inline void swap(Property& a, Property& b)  OMG_NOEXCEPT 
        {
            a.swap(b);
        }

        NDDSUSERDllExport std::ostream& operator<<(std::ostream& o, const Property& sample);

        #if (defined(RTI_WIN32) || defined (RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
        // On Windows, dll-export template instantiations of standard types used by
        // other dll-exported types
        template class NDDSUSERDllExport std::allocator< fep3::ddstypes::Property >;
        template class NDDSUSERDllExport std::vector< fep3::ddstypes::Property >;
        #endif
        class NDDSUSERDllExport StreamType {
          public:
            StreamType();

            StreamType(
                const std::string& metatype,
                const std::vector< fep3::ddstypes::Property >& properties);

            #ifdef RTI_CXX11_RVALUE_REFERENCES
            #ifndef RTI_CXX11_NO_IMPLICIT_MOVE_OPERATIONS
            StreamType (StreamType&&) = default;
            StreamType& operator=(StreamType&&) = default;
            StreamType& operator=(const StreamType&) = default;
            StreamType(const StreamType&) = default;
            #else
            StreamType(StreamType&& other_) OMG_NOEXCEPT;  
            StreamType& operator=(StreamType&&  other_) OMG_NOEXCEPT;
            #endif
            #endif 

            std::string& metatype() OMG_NOEXCEPT {
                return m_metatype_;
            }

            const std::string& metatype() const OMG_NOEXCEPT {
                return m_metatype_;
            }

            void metatype(const std::string& value) {
                m_metatype_ = value;
            }

            std::vector< fep3::ddstypes::Property >& properties() OMG_NOEXCEPT {
                return m_properties_;
            }

            const std::vector< fep3::ddstypes::Property >& properties() const OMG_NOEXCEPT {
                return m_properties_;
            }

            void properties(const std::vector< fep3::ddstypes::Property >& value) {
                m_properties_ = value;
            }

            bool operator == (const StreamType& other_) const;
            bool operator != (const StreamType& other_) const;

            void swap(StreamType& other_) OMG_NOEXCEPT ;

          private:

            std::string m_metatype_;
            std::vector< fep3::ddstypes::Property > m_properties_;

        };

        inline void swap(StreamType& a, StreamType& b)  OMG_NOEXCEPT 
        {
            a.swap(b);
        }

        NDDSUSERDllExport std::ostream& operator<<(std::ostream& o, const StreamType& sample);

        #if (defined(RTI_WIN32) || defined (RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
        // On Windows, dll-export template instantiations of standard types used by
        // other dll-exported types
        template class NDDSUSERDllExport std::allocator< uint8_t >;
        template class NDDSUSERDllExport std::vector< uint8_t >;
        #endif
        class NDDSUSERDllExport Sample {
          public:
            Sample();

            explicit Sample(
                const std::vector< uint8_t >& data);

            #ifdef RTI_CXX11_RVALUE_REFERENCES
            #ifndef RTI_CXX11_NO_IMPLICIT_MOVE_OPERATIONS
            Sample (Sample&&) = default;
            Sample& operator=(Sample&&) = default;
            Sample& operator=(const Sample&) = default;
            Sample(const Sample&) = default;
            #else
            Sample(Sample&& other_) OMG_NOEXCEPT;  
            Sample& operator=(Sample&&  other_) OMG_NOEXCEPT;
            #endif
            #endif 

            std::vector< uint8_t >& data() OMG_NOEXCEPT {
                return m_data_;
            }

            const std::vector< uint8_t >& data() const OMG_NOEXCEPT {
                return m_data_;
            }

            void data(const std::vector< uint8_t >& value) {
                m_data_ = value;
            }

            bool operator == (const Sample& other_) const;
            bool operator != (const Sample& other_) const;

            void swap(Sample& other_) OMG_NOEXCEPT ;

          private:

            std::vector< uint8_t > m_data_;

        };

        inline void swap(Sample& a, Sample& b)  OMG_NOEXCEPT 
        {
            a.swap(b);
        }

        NDDSUSERDllExport std::ostream& operator<<(std::ostream& o, const Sample& sample);

        struct StreamItemType_def {
            enum type {
                SAMPLE,      
                STREAMTYPE     
            };
            static type get_default(){ return SAMPLE;}
        };

        typedef ::dds::core::safe_enum< StreamItemType_def > StreamItemType;
        NDDSUSERDllExport std::ostream& operator << (std::ostream& o,const StreamItemType& sample);

        class NDDSUSERDllExport StreamItem {
          public:
            StreamItem();

            #ifdef RTI_CXX11_RVALUE_REFERENCES
            #ifndef RTI_CXX11_NO_IMPLICIT_MOVE_OPERATIONS
            StreamItem (StreamItem&&) = default;
            StreamItem& operator=(StreamItem&&) = default;
            StreamItem& operator=(const StreamItem&) = default;
            StreamItem(const StreamItem&) = default;
            #else
            StreamItem(StreamItem&& other_) OMG_NOEXCEPT;  
            StreamItem& operator=(StreamItem&&  other_) OMG_NOEXCEPT;
            #endif
            #endif 

            fep3::ddstypes::StreamItemType& _d()  {
                return m_d_;
            }

            const fep3::ddstypes::StreamItemType& _d() const  {
                return m_d_;
            }

            void _d(const fep3::ddstypes::StreamItemType& value) {
                m_d_ = value;
            }

            fep3::ddstypes::StreamType& streamtype()  {
                if ( _d() != (fep3::ddstypes::StreamItemType::STREAMTYPE)) {
                    throw ::dds::core::PreconditionNotMetError(
                        "fep3::ddstypes::StreamItem::streamtype not selected by the discriminator" );
                }
                return m_u_.m_streamtype_;
            }

            const fep3::ddstypes::StreamType& streamtype() const  {
                if ( _d() != (fep3::ddstypes::StreamItemType::STREAMTYPE)) {
                    throw ::dds::core::PreconditionNotMetError(
                        "fep3::ddstypes::StreamItem::streamtype not selected by the discriminator" );
                }
                return m_u_.m_streamtype_;
            }

            void streamtype(const fep3::ddstypes::StreamType& value) {
                m_u_.m_streamtype_ = value;
                m_d_= (fep3::ddstypes::StreamItemType::STREAMTYPE);
            }

            fep3::ddstypes::Sample& sample()  {
                if ( _d() != (fep3::ddstypes::StreamItemType::SAMPLE)) {
                    throw ::dds::core::PreconditionNotMetError(
                        "fep3::ddstypes::StreamItem::sample not selected by the discriminator" );
                }
                return m_u_.m_sample_;
            }

            const fep3::ddstypes::Sample& sample() const  {
                if ( _d() != (fep3::ddstypes::StreamItemType::SAMPLE)) {
                    throw ::dds::core::PreconditionNotMetError(
                        "fep3::ddstypes::StreamItem::sample not selected by the discriminator" );
                }
                return m_u_.m_sample_;
            }

            void sample(const fep3::ddstypes::Sample& value) {
                m_u_.m_sample_ = value;
                m_d_= (fep3::ddstypes::StreamItemType::SAMPLE);
            }

            bool operator == (const StreamItem& other_) const;
            bool operator != (const StreamItem& other_) const;

            static fep3::ddstypes::StreamItemType default_discriminator();  

            void swap(StreamItem& other_) OMG_NOEXCEPT ;

          private:

            fep3::ddstypes::StreamItemType m_d_;
            struct NDDSUSERDllExport Union_ {
                fep3::ddstypes::StreamType m_streamtype_;
                fep3::ddstypes::Sample m_sample_;
                Union_();
                Union_(
                    const fep3::ddstypes::StreamType& streamtype,
                    const fep3::ddstypes::Sample& sample);
            };
            Union_ m_u_;

        };

        inline void swap(StreamItem& a, StreamItem& b)  OMG_NOEXCEPT 
        {
            a.swap(b);
        }

        NDDSUSERDllExport std::ostream& operator<<(std::ostream& o, const StreamItem& sample);

    } // namespace ddstypes  
} // namespace fep3  

namespace rti {
    namespace flat {
        namespace topic {
        }
    }
}
namespace dds {
    namespace topic {

        template<>
        struct topic_type_name< fep3::ddstypes::Property > {
            NDDSUSERDllExport static std::string value() {
                return "fep3::ddstypes::Property";
            }
        };

        template<>
        struct is_topic_type< fep3::ddstypes::Property > : public ::dds::core::true_type {};

        template<>
        struct topic_type_support< fep3::ddstypes::Property > {
            NDDSUSERDllExport 
            static void register_type(
                ::dds::domain::DomainParticipant& participant,
                const std::string & type_name);

            NDDSUSERDllExport 
            static std::vector<char>& to_cdr_buffer(
                std::vector<char>& buffer, 
                const fep3::ddstypes::Property& sample,
                ::dds::core::policy::DataRepresentationId representation 
                = ::dds::core::policy::DataRepresentation::auto_id());

            NDDSUSERDllExport 
            static void from_cdr_buffer(fep3::ddstypes::Property& sample, const std::vector<char>& buffer);
            NDDSUSERDllExport 
            static void reset_sample(fep3::ddstypes::Property& sample);

            NDDSUSERDllExport 
            static void allocate_sample(fep3::ddstypes::Property& sample, int, int);

            static const ::rti::topic::TypePluginKind::type type_plugin_kind = 
            ::rti::topic::TypePluginKind::STL;
        };

        template<>
        struct topic_type_name< fep3::ddstypes::StreamType > {
            NDDSUSERDllExport static std::string value() {
                return "fep3::ddstypes::StreamType";
            }
        };

        template<>
        struct is_topic_type< fep3::ddstypes::StreamType > : public ::dds::core::true_type {};

        template<>
        struct topic_type_support< fep3::ddstypes::StreamType > {
            NDDSUSERDllExport 
            static void register_type(
                ::dds::domain::DomainParticipant& participant,
                const std::string & type_name);

            NDDSUSERDllExport 
            static std::vector<char>& to_cdr_buffer(
                std::vector<char>& buffer, 
                const fep3::ddstypes::StreamType& sample,
                ::dds::core::policy::DataRepresentationId representation 
                = ::dds::core::policy::DataRepresentation::auto_id());

            NDDSUSERDllExport 
            static void from_cdr_buffer(fep3::ddstypes::StreamType& sample, const std::vector<char>& buffer);
            NDDSUSERDllExport 
            static void reset_sample(fep3::ddstypes::StreamType& sample);

            NDDSUSERDllExport 
            static void allocate_sample(fep3::ddstypes::StreamType& sample, int, int);

            static const ::rti::topic::TypePluginKind::type type_plugin_kind = 
            ::rti::topic::TypePluginKind::STL;
        };

        template<>
        struct topic_type_name< fep3::ddstypes::Sample > {
            NDDSUSERDllExport static std::string value() {
                return "fep3::ddstypes::Sample";
            }
        };

        template<>
        struct is_topic_type< fep3::ddstypes::Sample > : public ::dds::core::true_type {};

        template<>
        struct topic_type_support< fep3::ddstypes::Sample > {
            NDDSUSERDllExport 
            static void register_type(
                ::dds::domain::DomainParticipant& participant,
                const std::string & type_name);

            NDDSUSERDllExport 
            static std::vector<char>& to_cdr_buffer(
                std::vector<char>& buffer, 
                const fep3::ddstypes::Sample& sample,
                ::dds::core::policy::DataRepresentationId representation 
                = ::dds::core::policy::DataRepresentation::auto_id());

            NDDSUSERDllExport 
            static void from_cdr_buffer(fep3::ddstypes::Sample& sample, const std::vector<char>& buffer);
            NDDSUSERDllExport 
            static void reset_sample(fep3::ddstypes::Sample& sample);

            NDDSUSERDllExport 
            static void allocate_sample(fep3::ddstypes::Sample& sample, int, int);

            static const ::rti::topic::TypePluginKind::type type_plugin_kind = 
            ::rti::topic::TypePluginKind::STL;
        };

        template<>
        struct topic_type_name< fep3::ddstypes::StreamItem > {
            NDDSUSERDllExport static std::string value() {
                return "fep3::ddstypes::StreamItem";
            }
        };

        template<>
        struct is_topic_type< fep3::ddstypes::StreamItem > : public ::dds::core::true_type {};

        template<>
        struct topic_type_support< fep3::ddstypes::StreamItem > {
            NDDSUSERDllExport 
            static void register_type(
                ::dds::domain::DomainParticipant& participant,
                const std::string & type_name);

            NDDSUSERDllExport 
            static std::vector<char>& to_cdr_buffer(
                std::vector<char>& buffer, 
                const fep3::ddstypes::StreamItem& sample,
                ::dds::core::policy::DataRepresentationId representation 
                = ::dds::core::policy::DataRepresentation::auto_id());

            NDDSUSERDllExport 
            static void from_cdr_buffer(fep3::ddstypes::StreamItem& sample, const std::vector<char>& buffer);
            NDDSUSERDllExport 
            static void reset_sample(fep3::ddstypes::StreamItem& sample);

            NDDSUSERDllExport 
            static void allocate_sample(fep3::ddstypes::StreamItem& sample, int, int);

            static const ::rti::topic::TypePluginKind::type type_plugin_kind = 
            ::rti::topic::TypePluginKind::STL;
        };

    }
}

namespace rti { 
    namespace topic {

        #ifndef NDDS_STANDALONE_TYPE
        template<>
        struct dynamic_type< fep3::ddstypes::Property > {
            typedef ::dds::core::xtypes::StructType type;
            NDDSUSERDllExport static const ::dds::core::xtypes::StructType& get();
        };
        #endif

        template <>
        struct extensibility< fep3::ddstypes::Property > {
            static const ::dds::core::xtypes::ExtensibilityKind::type kind =
            ::dds::core::xtypes::ExtensibilityKind::EXTENSIBLE;                
        };

        #ifndef NDDS_STANDALONE_TYPE
        template<>
        struct dynamic_type< fep3::ddstypes::StreamType > {
            typedef ::dds::core::xtypes::StructType type;
            NDDSUSERDllExport static const ::dds::core::xtypes::StructType& get();
        };
        #endif

        template <>
        struct extensibility< fep3::ddstypes::StreamType > {
            static const ::dds::core::xtypes::ExtensibilityKind::type kind =
            ::dds::core::xtypes::ExtensibilityKind::EXTENSIBLE;                
        };

        #ifndef NDDS_STANDALONE_TYPE
        template<>
        struct dynamic_type< fep3::ddstypes::Sample > {
            typedef ::dds::core::xtypes::StructType type;
            NDDSUSERDllExport static const ::dds::core::xtypes::StructType& get();
        };
        #endif

        template <>
        struct extensibility< fep3::ddstypes::Sample > {
            static const ::dds::core::xtypes::ExtensibilityKind::type kind =
            ::dds::core::xtypes::ExtensibilityKind::EXTENSIBLE;                
        };

        #ifndef NDDS_STANDALONE_TYPE
        template<>
        struct dynamic_type< fep3::ddstypes::StreamItemType > {
            typedef ::dds::core::xtypes::EnumType type;
            NDDSUSERDllExport static const ::dds::core::xtypes::EnumType& get();
        };
        #endif

        template <>
        struct extensibility< fep3::ddstypes::StreamItemType > {
            static const ::dds::core::xtypes::ExtensibilityKind::type kind =
            ::dds::core::xtypes::ExtensibilityKind::EXTENSIBLE;                
        };

        #ifndef NDDS_STANDALONE_TYPE
        template<>
        struct dynamic_type< fep3::ddstypes::StreamItem > {
            typedef ::dds::core::xtypes::UnionType type;
            NDDSUSERDllExport static const ::dds::core::xtypes::UnionType& get();
        };
        #endif

        template <>
        struct extensibility< fep3::ddstypes::StreamItem > {
            static const ::dds::core::xtypes::ExtensibilityKind::type kind =
            ::dds::core::xtypes::ExtensibilityKind::EXTENSIBLE;                
        };

    }
}

#if (defined(RTI_WIN32) || defined (RTI_WINCE) || defined(RTI_INTIME)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, stop exporting symbols.
*/
#undef NDDSUSERDllExport
#define NDDSUSERDllExport
#endif

#endif // stream_types_783819995_hpp

