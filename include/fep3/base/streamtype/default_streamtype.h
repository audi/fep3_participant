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
 //Guideline - FEP System Library API Exception
#ifndef _FEP3_COMP_STREAMTYPE_DEFAULT_H_
#define _FEP3_COMP_STREAMTYPE_DEFAULT_H_

#include "streamtype.h"
#include <map>
#include <string>
#include <list>

namespace fep3
{
namespace arya
{
/**
 * @brief the meta type for plain old datatype
 *
 * name: "plain-ctype"
 * properties: "datatype"
 *
 * @see StreamTypePlain
 */
const StreamMetaType meta_type_plain{"plain-ctype", std::list<std::string>{ "datatype" }};
/**
 * @brief the meta type for plain old datatype as array
 *
 * name: "plain-array-ctype"
 * properties: "datatype", "max_array_size"
 *
 * @see StreamTypePlain
 */
const StreamMetaType meta_type_plain_array{"plain-array-ctype", std::list<std::string>{ "datatype", "max_array_size" }};
/**
 * @brief the meta type for strings
 *
 * name: "ascii-string"
 * properties: "max_size"
 *
 *
 */
const StreamMetaType meta_type_string{"ascii-string", std::list<std::string>{ "max_size" }};
/**
 * @brief the meta type for video
 *
 * name: "video"
 * properties: "max_size", "height", "width", "pixelformat"
 *
 *
 */
const StreamMetaType meta_type_video{"video", std::list<std::string>{ "height", "width", "pixelformat", "max_size" }};
/**
* @brief the meta type for audio
*
* name: "audio"
* properties: not yet defined
*
*
*/
const StreamMetaType meta_type_audio{"audio", std::list<std::string>{}};

/**
 * @brief the meta type for raw memory types which are not typed!!
 *
 * name: "anonymous"
 * properties:
 *
 * @see StreamTypeRaw
 */
const StreamMetaType meta_type_raw{"anonymous", std::list<std::string>{ }};


/// value to define the struct type within the @ref meta_type_ddl 
const std::string    meta_type_prop_name_ddlstruct = "ddlstruct";
///value to define the whole type definition within the @ref meta_type_ddl 
const std::string    meta_type_prop_name_ddldescription = "ddldescription";
///value to define a file reference to the whole type definition within the @ref meta_type_ddl 
const std::string    meta_type_prop_name_ddlfileref = "ddlfileref";
///value to define max element amount within the @ref meta_type_ddl_array 
const std::string    meta_type_prop_name_size_max = "size_max";
/**
 * @brief Meta type for structured memory types which are described by DDL. Description has to be loaded from a file.
 *
 * name: "ddl-fileref"
 * properties: "ddlstruct", "ddlfileref"
 *
 * @see meta_type_prop_name_ddlstruct, meta_type_prop_name_ddlfileref
 * @see StreamTypeDDLFileRef
 */
const StreamMetaType meta_type_ddl_fileref{"ddl-fileref", std::list<std::string>{ meta_type_prop_name_ddlstruct, meta_type_prop_name_ddlfileref }};

/**
 * @brief Meta type for structured memory types which are described by DDL. Description is shipped within a StreamType property.
 *
 * name: "ddl"
 * properties: "ddlstruct", "ddldescription"
 *
 * @see meta_type_prop_name_ddlstruct, meta_type_prop_name_ddldescription
 * @see StreamTypeDDL
 */
const StreamMetaType meta_type_ddl{"ddl", std::list<std::string>{ meta_type_prop_name_ddlstruct, meta_type_prop_name_ddldescription }};

/**
 * @brief The meta type for structured array memory types
 *
 * name: "ddl"
 * properties: "ddlstruct", "ddlfileref", "size_max"
 *
 * @see meta_type_prop_name_ddlstruct, meta_type_prop_name_ddlfileref, meta_type_prop_name_size_max
 * @see fep::StreamTypeArrayDDL
 */
const StreamMetaType meta_type_ddl_array_fileref{"ddl-fileref-array", std::list<std::string>{ meta_type_prop_name_ddlstruct, meta_type_prop_name_ddlfileref, meta_type_prop_name_size_max }};

/**
 * @brief The meta type for structured array memory types
 *
 * name: "ddl"
 * properties: "ddlstruct", "ddldescription", "size_max"
 *
 * @see meta_type_prop_name_ddlstruct, meta_type_prop_name_ddldescription, meta_type_prop_name_size_max
 * @see fep::StreamTypeArrayDDLFileRef
 */
const StreamMetaType meta_type_ddl_array{"ddl-array", std::list<std::string>{ meta_type_prop_name_ddlstruct, meta_type_prop_name_ddldescription, meta_type_prop_name_size_max }};

/**
 * @brief Instance of a raw meta type.
 *
 * @see meta_type_raw
 */
class StreamTypeRaw : public StreamType
{
public:
    /**
     * @brief Construct a new Stream Type Raw object
     *
     */
    StreamTypeRaw() : StreamType(meta_type_raw)
    {
    }
};

/**
 * @brief StreamType class for meta_type_ddl_fileref
 * @see meta_type_ddl_fileref
 * @see StreamTypeDDL
 */
class StreamTypeDDLFileRef : public StreamType
{
public:
    /**
     * @brief Construct a new Stream Type DDL object
     *
     * @param ddlstruct Value for ddl struct this StreamType describes
     * @param fileref Reference to a file which contains the whole ddl description content. Your @p ddlstruct has to be included in this description.
     */
    StreamTypeDDLFileRef(const std::string& ddlstruct, const std::string& fileref) : StreamType(meta_type_ddl_fileref)
    {
        setProperty(meta_type_prop_name_ddlstruct, ddlstruct, "string");
        setProperty(meta_type_prop_name_ddlfileref, fileref, "string");
    }
};

/**
 * @brief StreamType class for meta_type_ddl
 * @see meta_type_ddl
 * @see StreamTypeDDLFileRef
 */
class StreamTypeDDL : public StreamType
{
public:
    /**
     * @brief Construct a new Stream Type DDL object
     *
     * @param ddlstruct Value for ddl struct this StreamType describes
     * @param ddldescription Contains the whole ddl description content. Your @p ddlstruct has to be included in this description.
     */
    StreamTypeDDL(const std::string& ddlstruct, const std::string& ddldescription) : StreamType(meta_type_ddl)
    {
        setProperty(meta_type_prop_name_ddlstruct, ddlstruct, "string");
        setProperty(meta_type_prop_name_ddldescription, ddldescription, "string");
    }
};

/**
 * @brief StreamType class for meta_type_ddl_array
 * @see meta_type_ddl_array
 */
class StreamTypeDDLArrayFileRef : public StreamType
{
public:
    /**
     * @brief Construct an Array Stream Type which uses a single struct from a DDL
     *
     * @param ddlstruct An element of your array
     * @param fileref Contains the whole ddl description content. Your @p ddlstruct has to be included in this description.
     * @param size_max Max element amount
     */
    StreamTypeDDLArrayFileRef(const std::string& ddlstruct, const std::string& fileref, uint32_t size_max) : StreamType(meta_type_ddl_array_fileref)
    {
        setProperty(meta_type_prop_name_ddlstruct, ddlstruct, "string");
        setProperty(meta_type_prop_name_ddlfileref, fileref, "string");
        setProperty(meta_type_prop_name_size_max, std::to_string(size_max), "int");
    }
};

/**
 * @brief StreamType class for meta_type_ddl_array
 * @see meta_type_ddl_array
 */
class StreamTypeDDLArray : public StreamType
{
public:
    /**
     * @brief Construct an Array Stream Type which uses a single struct from a DDL
     *
     * @param ddlstruct An element of your array
     * @param ddldescription Contains the whole ddl description content. Your @p ddlstruct has to be included in this description.
     * @param size_max Max element amount
     */
    StreamTypeDDLArray(const std::string& ddlstruct, const std::string& ddldescription, uint32_t size_max) : StreamType(meta_type_ddl_array)
    {
        setProperty(meta_type_prop_name_ddlstruct, ddlstruct, "string");
        setProperty(meta_type_prop_name_ddldescription, ddldescription, "string");
        setProperty(meta_type_prop_name_size_max, std::to_string(size_max), "int");
    }
};

/**
 * @brief Base StreamType class for any plain c meta type
 *
 * @tparam T the plain c type
 * @see meta_type_raw
 */
template<typename T>
class StreamTypePlain
{
};

/**
 * @brief specialized StreamTypePlain for type int8_t
 *
 * @tparam is set to int8_t
 */
template<>
class StreamTypePlain<int8_t> : public StreamType
{
public:
    /**
     * @brief Construct a new Stream Type Plain object
     *
     */
    StreamTypePlain() : StreamType(meta_type_plain)
    {
        setProperty("datatype", "int8_t", "string");
    }
};

/**
 * @brief specialized StreamTypePlain for type int16_t
 *
 * @tparam is set to int16_t
 */
template<>
class StreamTypePlain<int16_t> : public StreamType
{
public:
    /**
     * @brief Construct a new Stream Type Plain object
     *
     */
    StreamTypePlain() : StreamType(meta_type_plain)
    {
        setProperty("datatype", "int16_t", "string");
    }
};

/**
 * @brief specialized StreamTypePlain for type int32_t
 *
 * @tparam is set to int32_t
 */
template<>
class StreamTypePlain<int32_t> : public StreamType
{
public:
    /**
     * @brief Construct a new Stream Type Plain object
     *
     */
    StreamTypePlain() : StreamType(meta_type_plain)
    {
        setProperty("datatype", "int32_t", "string");
    }
};

/**
 * @brief specialized StreamTypePlain for type int64_t
 *
 * @tparam is set to int64_t
 */
template<>
class StreamTypePlain<int64_t> : public StreamType
{
public:
    /**
     * @brief Construct a new Stream Type Plain object
     *
     */
    StreamTypePlain() : StreamType(meta_type_plain)
    {
        setProperty("datatype", "int64_t", "string");
    }
};

/**
 * @brief specialized StreamTypePlain for type uint8_t
 *
 * @tparam is set to uint8_t
 */
template<>
class StreamTypePlain<uint8_t> : public StreamType
{
public:
    /**
     * @brief Construct a new Stream Type Plain object
     *
     */
    StreamTypePlain() : StreamType(meta_type_plain)
    {
        setProperty("datatype", "uint8_t", "string");
    }
};

/**
 * @brief specialized StreamTypePlain for type uint16_t
 *
 * @tparam is set to uint16_t
 */
template<>
class StreamTypePlain<uint16_t> : public StreamType
{
public:
    /**
     * @brief Construct a new Stream Type Plain object
     *
     */
    StreamTypePlain() : StreamType(meta_type_plain)
    {
        setProperty("datatype", "uint16_t", "string");
    }
};

/**
 * @brief specialized StreamTypePlain for type uint32_t
 *
 * @tparam is set to uint32_t
 */
template<>
class StreamTypePlain<uint32_t> : public StreamType
{
public:
    /**
     * @brief Construct a new Stream Type Plain object
     *
     */
    StreamTypePlain() : StreamType(meta_type_plain)
    {
        setProperty("datatype", "uint32_t", "string");
    }
};

/**
 * @brief specialized StreamTypePlain for type uint64_t
 *
 * @tparam is set to uint64_t
 */
template<>
class StreamTypePlain<uint64_t> : public StreamType
{
public:
    /**
     * @brief Construct a new Stream Type Plain object
     *
     */
    StreamTypePlain() : StreamType(meta_type_plain)
    {
        setProperty("datatype", "uint64_t", "string");
    }
};
/**
 * @brief specialized StreamTypePlain for type uint64_t
 *
 * @tparam is set to uint64_t
 */
class StreamTypeString : public StreamType
{
public:
    /**
     * @brief Construct a new Stream Type Plain object
     * @param max_size maximum size of a string
     * @remark max_size=0 means bus default (or dynamic)
     */
    StreamTypeString(size_t max_size = 0) : StreamType(meta_type_string)
    {
        setProperty("max_size", std::to_string(max_size), "uint32_t");
    }
};
}

using arya::StreamTypeRaw;
using arya::StreamTypeDDL;
using arya::StreamTypeDDLFileRef;
using arya::StreamTypeDDLArray;
using arya::StreamTypeDDLArrayFileRef;
using arya::StreamTypePlain;
using arya::StreamTypeString;
}

#endif //_FEP3_COMP_STREAMTYPE_DEFAULT_H_
