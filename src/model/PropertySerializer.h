#ifndef NVE_PROPERTY_SERIALIZER_H
#define NVE_PROPERTY_SERIALIZER_H

#include "API.h"
#include "ClassInfo.h"
#include "Object.h"
#include "ObjectRegistry.h"
#include "Utils.h"
#include "system/LogCategory.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace nuvelocity
{
    static constexpr std::string_view kTokenOpenBrace = "{";
    static constexpr std::string_view kTokenCloseBrace = "}";

    static constexpr std::string_view kKeyDynamicProperties = "Dynamic Properties";
    static constexpr std::string_view kKeyArrayCount = "Array Count";
    static constexpr std::string_view kKeyItemCount = "Item Count";
    static constexpr std::string_view kKeyArrayItem = "Array Item";
    static constexpr std::string_view kKeyArray = "Array";

    enum class ParserScope : uint8_t
    {
        None,
        Root,
        Child,
        Container,
        ByteArray,
        DynamicProperties
    };

    class PropertySerializer
    {
    public:
        template <typename T>
        static void Serialize(const T& value, std::string& output);

        template <typename T>
        static bool Deserialize(const std::string& input, T*& output, ClassInfo*& info);

        template <typename T>
        static bool Deserialize(std::istream& input, T*& output, ClassInfo*& info);

        template <typename T>
        static bool Deserialize(const std::string& input, T*& output);

    private:
        struct DeserializeContext
        {
            std::stack<ClassInfo*> infoStack = {};
            std::stack<void*> objectStack = {};
            std::stack<Property*> containerStack = {};
            std::stack<size_t> arrayCountStack = {};
            std::stack<ParserScope> scopeStack = {};

            ByteArrayTarget byteArrayTarget = {};
            size_t byteArraySize = 0; // Current size of data written
            ClassInfo* currentInfo = nullptr;
            void* currentObject = nullptr;
            ParserScope scope = ParserScope::None;

            // Set when an object is assigned but we haven't seen '{' yet.
            // Scope is pushed only if '{' follows; cleared otherwise.
            ClassInfo* pendingChildInfo = nullptr;
            void* pendingChildObject = nullptr;
        };

        static std::pair<ClassInfo*, void*> InstantiateObject(const std::string& className,
                                                              const ByteArrayInfo& byteArrayInfo)
        {
            ClassInfo* childInfo = ObjectRegistry::Get().Find(className);
            if (childInfo != nullptr)
            {
                void* childObject = childInfo->mFactoryFunction();

                // For byte array classes, initialize immediately with cols, rows, bits
                if (childInfo->mSerializationMode == SerializationMode::ByteArray)
                {
                    if (childInfo->mByteArrayInitFunction == nullptr)
                    {
                        throw std::runtime_error(std::string("Byte array class '") + className +
                                                 "' is missing mByteArrayInitFunction");
                    }

                    if (!byteArrayInfo.IsValid())
                    {
                        throw std::runtime_error(
                            std::string("Invalid byte array info for class '") + className +
                            "': expected (cols,rows,bits) with positive values and bits divisible "
                            "by 8");
                    }

                    childInfo->mByteArrayInitFunction(childObject, byteArrayInfo);
                }

                return {childInfo, childObject};
            }
            return {nullptr, nullptr};
        }

        static void ParseByteLine(const std::string& line, DeserializeContext& context)
        {
            if (context.byteArrayTarget.data == nullptr)
            {
                throw std::runtime_error("Byte array target pointer not initialized");
            }

            // Parse pairs of hex characters and write directly to target array
            for (size_t i = 0; i < line.length(); i += 2)
            {
                if (i + 1 >= line.length())
                {
                    throw std::runtime_error(std::string("Invalid byte array data length (") +
                                             "expected even number of characters)");
                }

                std::string hexPair = line.substr(i, 2);
                try
                {
                    uint8_t byte = static_cast<uint8_t>(std::stoul(hexPair, nullptr, 16));
                    context.byteArrayTarget.data[context.byteArraySize++] = byte;
                }
                catch (const std::exception& e)
                {
                    throw std::runtime_error(std::string("Failed to parse hex pair '") + hexPair +
                                             "': " + e.what());
                }
            }
        }

        static void InitializeByteArrayData(DeserializeContext& context)
        {
            if (context.currentInfo == nullptr || context.currentObject == nullptr)
            {
                throw std::runtime_error(
                    "Invalid object or class info for byte array deserialization");
            }

            if (context.currentInfo->mByteArrayProperty == nullptr)
            {
                throw std::runtime_error(std::string("No byte array property defined for class '") +
                                         context.currentInfo->mName + "'");
            }

            if (context.currentInfo->mByteArrayInfoFunction == nullptr)
            {
                throw std::runtime_error(std::string("Byte array class '") +
                                         context.currentInfo->mName +
                                         "' is missing mByteArrayInfoFunction");
            }

            // Get pointer to the array pointer held by the property
            uint8_t** byteArrayData = static_cast<uint8_t**>(
                context.currentInfo->mByteArrayProperty->GetValue(context.currentObject));
            if (byteArrayData == nullptr || *byteArrayData == nullptr)
            {
                throw std::runtime_error(
                    std::string("Failed to get byte array pointer for property '") +
                    context.currentInfo->mByteArrayProperty->GetName() + "'");
            }

            context.byteArrayTarget.info =
                context.currentInfo->mByteArrayInfoFunction(context.currentObject);
            context.byteArrayTarget.data = *byteArrayData;
            context.byteArraySize = 0;
        }

        static void FinalizeByteArrayData(DeserializeContext& context)
        {
            if (context.byteArraySize > 0)
            {
                SDL_LogWarn(NVE_LOG_CATEGORY_PROPSYS,
                            "Deserialized %zu bytes of data for class '%s' into property '%s'",
                            context.byteArraySize,
                            context.currentInfo->mName.c_str(),
                            context.currentInfo->mByteArrayProperty->GetName().c_str());
            }

            context.byteArrayTarget = {};
            context.byteArraySize = 0;
        }

        static void PushScope(DeserializeContext& context, ParserScope newScope)
        {
            context.scopeStack.push(context.scope);
            context.infoStack.push(context.currentInfo);
            context.objectStack.push(context.currentObject);
            context.scope = newScope;
        }

        static void PopScope(DeserializeContext& context)
        {
            if (context.scopeStack.empty())
            {
                context.scope = ParserScope::None;
                return;
            }

            context.scope = context.scopeStack.top();
            context.scopeStack.pop();

            context.currentInfo = context.infoStack.empty() ? nullptr : context.infoStack.top();
            if (!context.infoStack.empty())
            {
                context.infoStack.pop();
            }

            context.currentObject =
                context.objectStack.empty() ? nullptr : context.objectStack.top();
            if (!context.objectStack.empty())
            {
                context.objectStack.pop();
            }
        }

        static void ValidateAndPopContainer(DeserializeContext& context)
        {
            if (context.containerStack.empty())
            {
                return;
            }

            Property* containerProp = context.containerStack.top();

            if (containerProp->GetType() == PropertyType::Array && !context.arrayCountStack.empty())
            {
                size_t expectedCount = context.arrayCountStack.top();
                size_t actualCount = containerProp->GetArraySize(context.currentObject);
                if (expectedCount != actualCount)
                {
                    throw std::runtime_error(std::string("Array item count mismatch: expected ") +
                                             std::to_string(expectedCount) + ", got " +
                                             std::to_string(actualCount));
                }
                context.arrayCountStack.pop();
            }

            context.containerStack.pop();
        }

        static void HandleCloseBrace(DeserializeContext& context)
        {
            if (context.scope == ParserScope::ByteArray)
            {
                FinalizeByteArrayData(context);
            }
            else if (context.scope == ParserScope::Container)
            {
                ValidateAndPopContainer(context);
            }

            PopScope(context);
        }

        static bool HandleArrayContainerMetadata(Property* prop,
                                                 const std::string& key,
                                                 const std::string& value,
                                                 DeserializeContext& context)
        {
            if (prop->GetType() != PropertyType::Array)
            {
                return false;
            }

            const std::string& itemKey = prop->GetItemKey();
            const std::string& countKey =
                itemKey.empty() ? std::string(kKeyArrayCount) : std::string(kKeyItemCount);
            const std::string& expectedItemKey =
                itemKey.empty() ? std::string(kKeyArrayItem) : itemKey;

            if (key == countKey)
            {
                try
                {
                    context.arrayCountStack.push(std::stoull(value));
                }
                catch (...)
                {
                    throw std::runtime_error(std::string("Invalid item count: ") + value);
                }
                return true;
            }

            if (key != expectedItemKey)
            {
                throw std::runtime_error(std::string("Invalid array item key '") + key +
                                         "', expected '" + expectedItemKey + "'");
            }

            return false;
        }

        static void HandleContainerElement(Property* prop,
                                           const std::string& key,
                                           const std::string& value,
                                           DeserializeContext& context)
        {
            PropertyType containerType = prop->GetType();
            bool hasObjectElements =
                (containerType == PropertyType::Array && prop->IsObjectArray()) ||
                ((containerType == PropertyType::Map ||
                  containerType == PropertyType::UnorderedMap) &&
                 prop->IsObjectMapValue());

            if (hasObjectElements)
            {
                auto [className, byteArrayInfo] = ParseObjectValue(value);
                auto [childInfo, childObject] = InstantiateObject(className, byteArrayInfo);

                if (childInfo == nullptr || childObject == nullptr)
                {
                    throw std::runtime_error(
                        std::string("Failed to instantiate container element '") + value + "'");
                }

                if (containerType == PropertyType::Array)
                {
                    prop->AddArrayObjectEntry(context.currentObject, childObject);
                }
                else
                {
                    prop->AddMapObjectEntry(context.currentObject, key, childObject);
                }

                // Defer scope push until we confirm a '{' block follows.
                context.pendingChildInfo = childInfo;
                context.pendingChildObject = childObject;
                return;
            }

            if (containerType == PropertyType::Array)
            {
                prop->AddArrayEntry(context.currentObject, value);
            }
            else
            {
                prop->AddMapEntry(context.currentObject, key, value);
            }
        }

        static void HandleDynamicPropertyAssignment(const std::string& key,
                                                    const std::string& value,
                                                    DeserializeContext& context)
        {
            if (context.currentObject == nullptr)
            {
                SDL_LogWarn(NVE_LOG_CATEGORY_PROPSYS,
                            "Cannot assign dynamic property: empty object state.");
                return;
            }

            auto [className, byteArrayInfo] = ParseObjectValue(value);
            auto [childInfo, childObject] = InstantiateObject(className, byteArrayInfo);

            if (childObject)
            {
                static_cast<ObjectBase*>(context.currentObject)
                    ->SetDynamicProperty(key, childObject, true, childInfo);
                context.pendingChildInfo = childInfo;
                context.pendingChildObject = childObject;
            }
            else
            {
                std::string* strVal = new std::string(value);
                static_cast<ObjectBase*>(context.currentObject)
                    ->SetDynamicProperty(key, strVal, false, nullptr);
            }
        }

        static void HandlePropertyAssignment(const std::string& key,
                                             const std::string& value,
                                             DeserializeContext& context)
        {

            if (context.scope == ParserScope::None || context.currentInfo == nullptr ||
                context.currentObject == nullptr)
            {
                throw std::runtime_error(
                    std::string("Missing object context for property assignment: ") + key + "=" +
                    value);
            }

            Property* prop = nullptr;
            if (context.scope == ParserScope::Container)
            {
                if (context.containerStack.empty())
                {
                    SDL_LogWarn(NVE_LOG_CATEGORY_PROPSYS,
                                "Container scope but no container property on stack for key '%s'",
                                key.c_str());
                    return;
                }

                prop = context.containerStack.top();
                if (HandleArrayContainerMetadata(prop, key, value, context))
                {
                    return;
                }
            }
            else if (key == kKeyDynamicProperties)
            {
                PushScope(context, ParserScope::DynamicProperties);
                return;
            }
            else
            {
                prop = context.currentInfo->GetProperty(key);
                if (prop == nullptr)
                {
                    throw std::runtime_error(std::string("Unknown property '") + key +
                                             "' for class '" + context.currentInfo->mName + "'");
                }
            }

            switch (prop->GetType())
            {
            case PropertyType::Array:
            case PropertyType::Map:
            case PropertyType::UnorderedMap:
                if (value == kKeyArray)
                {
                    context.containerStack.push(prop);
                    PushScope(context, ParserScope::Container);
                }
                else if (context.scope == ParserScope::Container)
                {
                    HandleContainerElement(prop, key, value, context);
                }
                break;
            case PropertyType::Object:
            {
                auto [className, byteArrayInfo] = ParseObjectValue(value);
                auto [childInfo, childObject] = InstantiateObject(className, byteArrayInfo);
                if (childInfo == nullptr)
                {
                    throw std::runtime_error(std::string("Unknown class '") + className +
                                             "' for property '" + key + "'");
                }

                prop->SetValue(context.currentObject, &childObject);
                // Defer scope push until we confirm a '{' block follows.
                context.pendingChildInfo = childInfo;
                context.pendingChildObject = childObject;
                break;
            }
            default:
                if (context.scope == ParserScope::Container)
                {
                    HandleContainerElement(prop, key, value, context);
                }
                else
                {
                    prop->SetValue(context.currentObject, value);
                }
                break;
            }
        }

        template <typename T>
        static void HandleObjectInstantiation(const std::string& line,
                                              T*& output,
                                              ClassInfo*& info,
                                              DeserializeContext& context)
        {
            auto [className, byteArrayInfo] = ParseObjectValue(line);
            auto [classInfo, obj] = InstantiateObject(className, byteArrayInfo);
            if (classInfo == nullptr)
            {
                throw std::runtime_error(std::string("Unknown class: ") + className);
            }

            info = classInfo;
            output = static_cast<T*>(obj);
            SDL_Log("Created object of type: %s", classInfo->mName.c_str());
            context.currentInfo = classInfo;
            context.currentObject = obj;

            if (context.scopeStack.empty())
            {
                context.scope = ParserScope::Root;
            }
        }

        static std::pair<std::string, ByteArrayInfo> ParseObjectValue(const std::string& value)
        {
            std::string className = value;
            ByteArrayInfo byteArrayInfo = {};

            size_t parenPos = className.find('(');
            if (parenPos != std::string::npos)
            {
                size_t closeParenPos = className.find(')');
                if (closeParenPos != std::string::npos)
                {
                    std::string argsStr =
                        className.substr(parenPos + 1, closeParenPos - parenPos - 1);
                    argsStr = trim(argsStr);
                    if (!argsStr.empty())
                    {
                        std::stringstream argStream(argsStr);
                        std::string arg;
                        std::vector<std::string> arguments;
                        while (std::getline(argStream, arg, ','))
                        {
                            arguments.push_back(trim(arg));
                        }

                        if (arguments.size() != 3)
                        {
                            throw std::runtime_error(
                                "Byte array object values must use exactly 3 arguments: "
                                "(cols,rows,bits)");
                        }

                        try
                        {
                            byteArrayInfo.cols = std::stoi(arguments[0]);
                            byteArrayInfo.rows = std::stoi(arguments[1]);
                            byteArrayInfo.bits = std::stoi(arguments[2]);
                        }
                        catch (const std::exception& e)
                        {
                            throw std::runtime_error(
                                std::string("Failed to parse byte array args (cols,rows,bits): ") +
                                e.what());
                        }
                    }
                    className = className.substr(0, parenPos);
                    className = trim(className);
                }
            }

            return {className, byteArrayInfo};
        }
    };

    // Helper function for serializing objects
    inline static std::string GetIndent(int level)
    {
        return std::string(level, '\t');
    }

    inline static void AppendLine(std::string& output, int indentLevel, const std::string& line)
    {
        output += GetIndent(indentLevel) + line + "\n";
    }

    inline static std::string GetPropertyValueAsString(const void* object, Property* prop)
    {
        PropertyType type = prop->GetType();

        switch (type)
        {
        case PropertyType::Int:
        {
            IntProperty* intProp = static_cast<IntProperty*>(prop);
            return std::to_string(intProp->GetIntValue(const_cast<void*>(object)));
        }
        case PropertyType::UInt:
        {
            UIntProperty* uintProp = static_cast<UIntProperty*>(prop);
            return std::to_string(uintProp->GetUIntValue(const_cast<void*>(object)));
        }
        case PropertyType::Float:
        {
            FloatProperty* floatProp = static_cast<FloatProperty*>(prop);
            float val = floatProp->GetFloatValue(const_cast<void*>(object));
            std::ostringstream oss;
            oss << val;
            return oss.str();
        }
        case PropertyType::Double:
        {
            DoubleProperty* doubleProp = static_cast<DoubleProperty*>(prop);
            double val = doubleProp->GetDoubleValue(const_cast<void*>(object));
            std::ostringstream oss;
            oss << val;
            return oss.str();
        }
        case PropertyType::Bool:
        {
            BoolProperty* boolProp = static_cast<BoolProperty*>(prop);
            return boolProp->GetBoolValue(const_cast<void*>(object)) ? "1" : "0";
        }
        case PropertyType::String:
        {
            StringProperty* strProp = static_cast<StringProperty*>(prop);
            return strProp->GetStringValue(const_cast<void*>(object));
        }
        case PropertyType::CString:
        {
            CStringProperty* cstrProp = static_cast<CStringProperty*>(prop);
            const char* val = cstrProp->GetCStringValue(const_cast<void*>(object));
            return val ? std::string(val) : "";
        }
        case PropertyType::Enum:
        {
            EnumProperty* enumProp = static_cast<EnumProperty*>(prop);
            return enumProp->GetSerializedValue(const_cast<void*>(object));
        }
        case PropertyType::Polygon:
        {
            PolygonProperty* polyProp = static_cast<PolygonProperty*>(prop);
            const std::vector<SDL_FPoint>& points = polyProp->GetPoints(const_cast<void*>(object));
            std::ostringstream oss;
            for (size_t i = 0; i < points.size(); ++i)
            {
                oss << points[i].x << "," << points[i].y;
                if (i + 1 < points.size())
                {
                    oss << ",";
                }
            }
            return oss.str();
        }
        case PropertyType::Point:
        {
            PointProperty* pointProp = static_cast<PointProperty*>(prop);
            SDL_FPoint point = pointProp->GetPointValue(const_cast<void*>(object));
            std::ostringstream oss;
            oss << point.x << "," << point.y;
            return oss.str();
        }
        default:
            return "";
        }
    }

    inline static void
    SerializeObject(const void* object, ClassInfo* classInfo, std::string& output, int indentLevel)
    {
        if (classInfo == nullptr || object == nullptr)
        {
            return;
        }

        if (classInfo->mSerializationMode == SerializationMode::ByteArray)
        {
            ByteArrayInfo byteArrayInfo = {};

            // Get constructor info from byte array info function
            if (classInfo->mByteArrayInfoFunction != nullptr)
            {
                byteArrayInfo = classInfo->mByteArrayInfoFunction(object);
            }

            if (byteArrayInfo.IsValid())
            {
                std::ostringstream header;
                header << classInfo->mName << "(";
                header << byteArrayInfo.cols << "," << byteArrayInfo.rows << ","
                       << byteArrayInfo.bits;
                header << ")";
                AppendLine(output, indentLevel, header.str());
            }
            else
            {
                AppendLine(output, indentLevel, classInfo->mName);
            }

            AppendLine(output, indentLevel, std::string(kTokenOpenBrace));

            if (classInfo->mByteArrayProperty != nullptr)
            {
                auto* pixelData = static_cast<uint8_t**>(
                    classInfo->mByteArrayProperty->GetValue(const_cast<void*>(object)));

                const bool hasExpectedSize = byteArrayInfo.IsValid();
                const size_t expectedSize = hasExpectedSize ? byteArrayInfo.ByteCount() : 0;

                if (pixelData != nullptr && *pixelData != nullptr && hasExpectedSize)
                {
                    constexpr size_t kBytesPerLine = 32;
                    for (size_t offset = 0; offset < expectedSize; offset += kBytesPerLine)
                    {
                        const size_t chunk = std::min(kBytesPerLine, expectedSize - offset);
                        std::ostringstream hexLine;
                        for (size_t i = 0; i < chunk; ++i)
                        {
                            hexLine << std::hex << std::setfill('0') << std::setw(2)
                                    << static_cast<unsigned int>((*pixelData)[offset + i]);
                        }
                        AppendLine(output, indentLevel + 1, hexLine.str());
                    }
                }
            }

            AppendLine(output, indentLevel, std::string(kTokenCloseBrace));
            return;
        }

        AppendLine(output, indentLevel, classInfo->mName);
        AppendLine(output, indentLevel, std::string(kTokenOpenBrace));

        Property* prop = classInfo->GetFirstProperty();
        while (prop != nullptr)
        {
            PropertyType type = prop->GetType();
            std::string propName = prop->GetName();

            switch (type)
            {
            case PropertyType::Array:
            {
                size_t arraySize = prop->GetArraySize(const_cast<void*>(object));
                AppendLine(output, indentLevel + 1, propName + "=" + std::string(kKeyArray));
                AppendLine(output, indentLevel + 1, std::string(kTokenOpenBrace));
                AppendLine(output,
                           indentLevel + 2,
                           std::string(kKeyItemCount) + "=" + std::to_string(arraySize));

                std::string itemKey = prop->GetItemKey();
                if (itemKey.empty())
                {
                    itemKey = std::string(kKeyArrayItem);
                }

                SDL_LogWarn(NVE_LOG_CATEGORY_PROPSYS,
                            "Array serialization for '%s' requires element type at compile-time",
                            propName.c_str());

                AppendLine(output, indentLevel + 1, std::string(kTokenCloseBrace));
                break;
            }
            case PropertyType::Map:
            case PropertyType::UnorderedMap:
            {
                AppendLine(output, indentLevel + 1, propName + "=" + std::string(kKeyArray));
                AppendLine(output, indentLevel + 1, std::string(kTokenOpenBrace));

                SDL_LogWarn(NVE_LOG_CATEGORY_PROPSYS,
                            "Map serialization for '%s' requires key/value type at compile-time",
                            propName.c_str());

                AppendLine(output, indentLevel + 1, std::string(kTokenCloseBrace));
                break;
            }
            case PropertyType::Object:
            {
                void* childObject = *(void**)prop->GetValue(const_cast<void*>(object));
                if (childObject != nullptr)
                {
                    ClassInfo* childInfo = prop->GetChildClassInfo();

                    if (childInfo != nullptr)
                    {
                        // Recursively serialize the child object
                        SerializeObject(childObject, childInfo, output, indentLevel + 1);
                    }
                    else
                    {
                        SDL_LogWarn(NVE_LOG_CATEGORY_PROPSYS,
                                    "No ClassInfo for object property '%s'",
                                    propName.c_str());
                    }
                }
                break;
            }
            default:
            {
                std::string value = GetPropertyValueAsString(object, prop);
                AppendLine(output, indentLevel + 1, propName + "=" + value);
                break;
            }
            }

            prop = prop->mNext;
        }

        if (const void* dynPropsMapPtr =
                static_cast<const ObjectBase*>(object)->GetDynamicPropertiesMap())
        {
            const auto* dynProps = static_cast<
                const std::unordered_map<std::string, std::tuple<bool, void*, ClassInfo*>>*>(
                dynPropsMapPtr);
            if (!dynProps->empty())
            {
                AppendLine(output,
                           indentLevel + 1,
                           std::string(kKeyDynamicProperties) + "=" +
                               std::to_string(dynProps->size()));
                AppendLine(output, indentLevel + 1, std::string(kTokenOpenBrace));
                for (const auto& [dynKey, tupleVal] : *dynProps)
                {
                    void* valPtr = std::get<1>(tupleVal);
                    if (valPtr == nullptr)
                        continue;

                    if (!std::get<0>(tupleVal)) // string
                    {
                        std::string* strVal = static_cast<std::string*>(valPtr);
                        AppendLine(output, indentLevel + 2, dynKey + "=" + *strVal);
                    }
                    else // object
                    {
                        ClassInfo* childInfo = std::get<2>(tupleVal);
                        if (childInfo != nullptr)
                        {
                            SerializeObject(valPtr, childInfo, output, indentLevel + 1);
                        }
                    }
                }
                AppendLine(output, indentLevel + 1, std::string(kTokenCloseBrace));
            }
        }

        AppendLine(output, indentLevel, std::string(kTokenCloseBrace));
    }

    template <typename T>
    void PropertySerializer::Serialize(const T& value, std::string& output)
    {
        // T must be an Object subclass with GetClassInfo() method
        ClassInfo* classInfo = T::GetClassInfo();
        SerializeObject(&value, classInfo, output, 0);
    }

    template <typename T>
    bool PropertySerializer::Deserialize(const std::string& input, T*& output, ClassInfo*& info)
    {
        std::stringstream stream(input);
        return Deserialize(stream, output, info);
    }

    template <typename T>
    bool PropertySerializer::Deserialize(std::istream& stream, T*& output, ClassInfo*& info)
    {
        std::string line;

        info = nullptr;
        output = nullptr;

        DeserializeContext context;

        while (std::getline(stream, line))
        {
            line = trim(line);
            if (line.empty())
            {
                continue;
            }

            if (line == kTokenOpenBrace)
            {
                if (context.pendingChildInfo != nullptr)
                {
                    ParserScope childScope =
                        context.pendingChildInfo->mSerializationMode == SerializationMode::ByteArray
                            ? ParserScope::ByteArray
                            : ParserScope::Child;
                    PushScope(context, childScope);
                    context.currentInfo = context.pendingChildInfo;
                    context.currentObject = context.pendingChildObject;
                    context.pendingChildInfo = nullptr;
                    context.pendingChildObject = nullptr;
                    if (childScope == ParserScope::ByteArray)
                    {
                        InitializeByteArrayData(context);
                    }
                }
                else if (context.scope == ParserScope::Root &&
                         context.currentInfo->mSerializationMode == SerializationMode::ByteArray)
                {
                    InitializeByteArrayData(context);
                    PushScope(context, ParserScope::ByteArray);
                }
                continue;
            }

            if (line == kTokenCloseBrace)
            {
                HandleCloseBrace(context);

                // Stop at the first fully parsed root object.
                if (output != nullptr && context.scope == ParserScope::None &&
                    context.scopeStack.empty())
                {
                    break;
                }
                continue;
            }

            if (context.scope == ParserScope::ByteArray)
            {
                ParseByteLine(line, context);
                continue;
            }

            // A non-'{' line means the previous pending object has no block; discard pending.
            context.pendingChildInfo = nullptr;
            context.pendingChildObject = nullptr;

            if (auto* kvp = parseKeyValuePair(line); kvp != nullptr)
            {
                const std::string key = kvp->first;
                const std::string value = kvp->second;
                delete kvp;
                if (context.scope == ParserScope::DynamicProperties)
                {
                    HandleDynamicPropertyAssignment(key, value, context);
                }
                else
                {
                    HandlePropertyAssignment(key, value, context);
                }
            }
            else
            {
                HandleObjectInstantiation(line, output, info, context);
            }
        }

        assert(context.scopeStack.empty());

        if (output == nullptr)
        {
            SDL_LogError(NVE_LOG_CATEGORY_PROPSYS,
                         "Failed to deserialize object: No class information found.");
            return false;
        }

        return true;
    }

    template <typename T>
    bool PropertySerializer::Deserialize(const std::string& input, T*& output)
    {
        ClassInfo* info = nullptr;
        return Deserialize(input, output, info);
    }
} // namespace nuvelocity

#endif // NVE_PROPERTY_SERIALIZER_H
