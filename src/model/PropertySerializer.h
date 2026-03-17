#ifndef NVE_PROPERTY_SERIALIZER_H
#define NVE_PROPERTY_SERIALIZER_H

#include "API.h"
#include "ClassInfo.h"
#include "ObjectRegistry.h"
#include "Utils.h"
#include "system/LogCategory.h"
#include <algorithm>
#include <cassert>
#include <cctype>
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

    enum class ParserScope
    {
        None,
        Root,
        Child,
        Container,
        HexArray
    };

    class PropertySerializer
    {
    public:
        template <typename T>
        static void Serialize(const T& value, std::string& output);

        template <typename T>
        static bool Deserialize(const std::string& input, T*& output, ClassInfo*& info);

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

            std::vector<uint8_t> hexArrayData = {};
            ClassInfo* currentInfo = nullptr;
            void* currentObject = nullptr;
            ParserScope scope = ParserScope::None;
        };

        static std::pair<ClassInfo*, void*> InstantiateObject(const std::string& className,
                                                              const std::vector<std::string>& args)
        {
            ClassInfo* childInfo = ObjectRegistry::Get().Find(className);
            if (childInfo != nullptr)
            {
                void* childObject = childInfo->mFactoryFunction();

                // If arguments are provided, assign them to properties in order
                if (args.size() > 0)
                {
                    // Assert that this class uses non-standard serialization
                    // (argument-based construction only makes sense for special classes like
                    // HexArray)
                    assert(childInfo->mSerializationMode != SerializationMode::Standard &&
                           "Arguments can only be used with non-Standard serialization modes");

                    // Assign arguments to properties in order
                    size_t argIndex = 0;
                    Property* prop = childInfo->GetFirstProperty();
                    while (prop != nullptr && argIndex < args.size())
                    {
                        prop->SetValue(childObject, args[argIndex]);
                        argIndex++;
                        prop = prop->mNext;
                    }
                }

                return {childInfo, childObject};
            }
            return {nullptr, nullptr};
        }

        static void ParseHexLine(const std::string& line, std::vector<uint8_t>& binaryData)
        {
            std::string cleanLine = trim(const_cast<std::string&>(line));

            if (cleanLine.empty())
            {
                return;
            }

            // Parse pairs of hex characters
            for (size_t i = 0; i < cleanLine.length(); i += 2)
            {
                if (i + 1 >= cleanLine.length())
                {
                    throw std::runtime_error(std::string("Invalid hex data length (") +
                                             "expected even number of characters)");
                }

                std::string hexPair = cleanLine.substr(i, 2);
                try
                {
                    uint8_t byte = static_cast<uint8_t>(std::stoul(hexPair, nullptr, 16));
                    binaryData.push_back(byte);
                }
                catch (const std::exception& e)
                {
                    throw std::runtime_error(std::string("Failed to parse hex pair '") + hexPair +
                                             "': " + e.what());
                }
            }
        }

        static void StoreHexArrayData(std::vector<uint8_t>& binaryData, void* object,
                                      ClassInfo* classInfo)
        {
            if (classInfo == nullptr || object == nullptr)
            {
                throw std::runtime_error(
                    "Invalid object or class info for hex array deserialization");
            }

            if (classInfo->mHexArrayProperty == nullptr)
            {
                throw std::runtime_error(std::string("No hex array property defined for class '") +
                                         classInfo->mName + "'");
            }

            // Set the parsed binary data to the designated hex array property
            // Convert vector<uint8_t> to string representation for property setting
            std::string binaryString(binaryData.begin(), binaryData.end());
            classInfo->mHexArrayProperty->SetValue(object, binaryString);

            SDL_LogWarn(
                NVE_LOG_CATEGORY_PROPSYS,
                "Deserialized %zu bytes of hex array data for class '%s' into property '%s'",
                binaryData.size(), classInfo->mName.c_str(),
                classInfo->mHexArrayProperty->GetName().c_str());
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

        static bool AccumulateHexArrayLine(const std::string& line, DeserializeContext& context)
        {
            if (context.scope != ParserScope::HexArray)
            {
                return false;
            }

            if (line != kTokenCloseBrace)
            {
                ParseHexLine(line, context.hexArrayData);
                return true;
            }

            return false;
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
            if (context.scope == ParserScope::HexArray)
            {
                StoreHexArrayData(context.hexArrayData, context.currentObject, context.currentInfo);
                context.hexArrayData.clear();
            }
            else if (context.scope == ParserScope::Container)
            {
                ValidateAndPopContainer(context);
            }

            PopScope(context);
        }

        static bool HandleArrayContainerMetadata(Property* prop, const std::string& key,
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

        static void HandleContainerElement(Property* prop, const std::string& key,
                                           const std::string& value, DeserializeContext& context)
        {
            PropertyType containerType = prop->GetType();
            bool hasObjectElements =
                (containerType == PropertyType::Array && prop->IsObjectArray()) ||
                ((containerType == PropertyType::Map ||
                  containerType == PropertyType::UnorderedMap) &&
                 prop->IsObjectMapValue());

            if (hasObjectElements)
            {
                auto [objValue, arguments] = ParseObjectValue(value);
                auto [childInfo, childObject] = InstantiateObject(objValue, arguments);
                if (childInfo == nullptr || childObject == nullptr)
                {
                    throw std::runtime_error(
                        std::string("Failed to instantiate container element '") + value + "'");
                }

                if (containerType == PropertyType::Array)
                {
                    prop->SetValue(context.currentObject, childObject);
                }
                else
                {
                    prop->AddMapObjectEntry(context.currentObject, key, childObject);
                }
                return;
            }

            if (containerType == PropertyType::Array)
            {
                prop->SetValue(context.currentObject, value);
            }
            else
            {
                prop->AddMapEntry(context.currentObject, key, value);
            }
        }

        static void HandlePropertyAssignment(const std::string& key, const std::string& value,
                                             DeserializeContext& context)
        {
            if (key == kKeyDynamicProperties)
            {
                return;
            }

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
                break;
            case PropertyType::Object:
            {
                auto [objValue, arguments] = ParseObjectValue(value);
                auto [childInfo, childObject] = InstantiateObject(objValue, arguments);
                if (childInfo == nullptr)
                {
                    throw std::runtime_error(std::string("Unknown class '") + objValue +
                                             "' for property '" + key + "'");
                }

                prop->SetValue(context.currentObject, childObject);
                PushScope(context, ParserScope::Child);
                context.currentInfo = childInfo;
                context.currentObject = childObject;
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
        static void HandleObjectInstantiation(const std::string& line, T*& output, ClassInfo*& info,
                                              DeserializeContext& context)
        {
            auto [classInfo, obj] = InstantiateObject(line, {});
            if (classInfo == nullptr)
            {
                throw std::runtime_error(std::string("Unknown class: ") + line);
            }

            info = classInfo;
            output = static_cast<T*>(obj);
            SDL_Log("Created object of type: %s", classInfo->mName.c_str());
            context.currentInfo = classInfo;
            context.currentObject = obj;

            if (context.scopeStack.empty())
            {
                context.scope = ParserScope::Root;

                if (context.currentInfo->mSerializationMode == SerializationMode::HexArray)
                {
                    PushScope(context, ParserScope::HexArray);
                }
            }
        }

        static std::pair<std::string, std::vector<std::string>>
        ParseObjectValue(const std::string& value)
        {
            std::string objValue = value;
            std::vector<std::string> arguments;

            size_t parenPos = objValue.find('(');
            if (parenPos != std::string::npos)
            {
                size_t closeParenPos = objValue.find(')');
                if (closeParenPos != std::string::npos)
                {
                    std::string argsStr =
                        objValue.substr(parenPos + 1, closeParenPos - parenPos - 1);
                    argsStr = trim(argsStr);
                    if (!argsStr.empty())
                    {
                        std::stringstream argStream(argsStr);
                        std::string arg;
                        while (std::getline(argStream, arg, ','))
                        {
                            arguments.push_back(trim(arg));
                        }
                    }
                    objValue = objValue.substr(0, parenPos);
                    objValue = trim(objValue);
                }
            }

            return {objValue, arguments};
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
            return std::to_string(enumProp->GetIntValue(const_cast<void*>(object)));
        }
        default:
            return "";
        }
    }

    inline static void SerializeObject(const void* object, ClassInfo* classInfo,
                                       std::string& output, int indentLevel)
    {
        if (classInfo == nullptr || object == nullptr)
        {
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
                AppendLine(output, indentLevel + 2,
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
                void* childObject = *(void**) prop->GetValue(const_cast<void*>(object));
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
                                    "No ClassInfo for object property '%s'", propName.c_str());
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

            if (AccumulateHexArrayLine(line, context))
            {
                continue;
            }

            if (line == kTokenOpenBrace)
            {
                continue;
            }

            if (line == kTokenCloseBrace)
            {
                HandleCloseBrace(context);
                continue;
            }

            if (auto* kvp = parseKeyValuePair(line); kvp != nullptr)
            {
                const std::string key = kvp->first;
                const std::string value = kvp->second;
                delete kvp;
                HandlePropertyAssignment(key, value, context);
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
