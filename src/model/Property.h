#ifndef NVE_PROPERTY_H
#define NVE_PROPERTY_H

#include "LogCategory.h"
#include <SDL3/SDL.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace nuvelocity
{
    class ClassInfo;
    class Property;

    inline void ParseCSVToPoints(const std::string& csv, std::vector<SDL_FPoint>& points)
    {
        points.clear();
        std::stringstream stream(csv);
        std::string segment;
        std::vector<float> coords;

        while (std::getline(stream, segment, ','))
        {
            try
            {
                coords.push_back(std::stof(segment));
            }
            catch (...)
            {
            }
        }

        for (size_t i = 0; i + 1 < coords.size(); i += 2)
        {
            points.push_back(SDL_FPoint{.x = coords[i], .y = coords[i + 1]});
        }
    }

    enum class PropertyType : uint8_t
    {
        Object,
        Int,
        UInt,
        Float,
        Double,
        Bool,
        String,
        CString,
        Enum,
        Array,
        Map,
        UnorderedMap,
        Color,
        Polygon,
        Point
    };

    class Property
    {
    protected:
        std::string mName;
        std::string mDescription;

        size_t mOffset;
        size_t mSize;
        bool mIsDeprecated = false;

    public:
        Property(const std::string& name,
                 size_t offset,
                 size_t size,
                 const std::string& description = "")
                : mName(name)
                , mOffset(offset)
                , mSize(size)
                , mDescription(description)
                , mNext(nullptr)
        {
        }

        virtual ~Property() = default;

        Property* mNext;

        const std::string& GetName() const
        {
            return mName;
        }
        const std::string& GetDescription() const
        {
            return mDescription;
        }

        void SetDescription(const std::string& description)
        {
            mDescription = description;
        }

        bool IsDeprecated() const
        {
            return mIsDeprecated;
        }

        void SetDeprecated(bool deprecated)
        {
            mIsDeprecated = deprecated;
        }

    protected:
        inline void* GetValuePtr(void* obj) const
        {
            char* base = (char*)obj;
            return (void*)(base + mOffset);
        }

    public:
        virtual void* GetValue(void* obj) const
        {
            return GetValuePtr(obj);
        }

        virtual void SetValue(void* obj, const void* valuePtr)
        {
            char* dest = (char*)obj + mOffset;
            memcpy(dest, valuePtr, mSize);
        }

        virtual void SetValue(void* obj, const std::string& value)
        {
            SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                        "String assignment not supported for property '%s'",
                        mName.c_str());
        }

        virtual PropertyType GetType() const
        {
            return PropertyType::Object;
        }

    protected:
        // Single check for whether a container holds object pointers
        virtual bool ContainsObjects() const
        {
            return false;
        }

    public:
        virtual bool IsObjectArray() const
        {
            return ContainsObjects();
        }

        virtual bool IsObjectMapValue() const
        {
            return ContainsObjects();
        }

        virtual size_t GetArraySize(void* obj) const
        {
            return 0;
        }

        virtual const std::string& GetItemKey() const
        {
            static const std::string empty;
            return empty;
        }

        virtual const std::string& GetExpectedClassName() const
        {
            static const std::string empty;
            return empty;
        }

        virtual ClassInfo* GetChildClassInfo() const
        {
            return nullptr;
        }

        virtual void AddMapEntry(void* obj, const std::string& key, const std::string& value)
        {
            SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                        "Map entry insertion not supported for property '%s'",
                        mName.c_str());
        }

        virtual void AddArrayEntry(void* obj, const std::string& value)
        {
            SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                        "Array entry insertion not supported for property '%s'",
                        mName.c_str());
        }

        virtual void AddArrayObjectEntry(void* obj, void* valueObj)
        {
            SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                        "Array object entry insertion not supported for property '%s'",
                        mName.c_str());
        }

        virtual void AddMapObjectEntry(void* obj, const std::string& key, void* valueObj)
        {
            SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                        "Map object entry insertion not supported for property '%s'",
                        mName.c_str());
        }

        virtual void DumpValue(void* obj) const
        {
            void* valuePtr = GetValue(obj);
            SDL_Log("  %s: %p", mName.c_str(), valuePtr);
        }

        void DumpMetadata() const
        {
            SDL_Log("Property: %s", mName.c_str());
            SDL_Log("  Description: %s", mDescription.c_str());
            SDL_Log("  Offset: %zu", mOffset);
            SDL_Log("  Size: %zu", mSize);
        }
    };

    class IntProperty : public Property
    {
    public:
        IntProperty(const std::string& name, size_t offset, size_t size)
                : Property(name, offset, size)
        {
        }

        int GetIntValue(void* obj) const
        {
            return *(int*)GetValuePtr(obj);
        }

        void SetIntValue(void* obj, int value)
        {
            *(int*)GetValuePtr(obj) = value;
        }

        void SetValue(void* obj, const void* valuePtr) override
        {
            *(int*)GetValuePtr(obj) = *(const int*)valuePtr;
        }

        void SetValue(void* obj, const std::string& value) override
        {
            try
            {
                int intValue = std::stoi(value);
                SetIntValue(obj, intValue);
            }
            catch (const std::exception& e)
            {
                SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                            "Failed to convert '%s' to int for property '%s': %s",
                            value.c_str(),
                            mName.c_str(),
                            e.what());
            }
        }

        void DumpValue(void* obj) const override
        {
            int intValue = GetIntValue(obj);
            SDL_Log("  %s: %d", mName.c_str(), intValue);
        }

        PropertyType GetType() const override
        {
            return PropertyType::Int;
        }
    };

    class UIntProperty : public Property
    {
    public:
        UIntProperty(const std::string& name, size_t offset, size_t size)
                : Property(name, offset, size)
        {
        }

        unsigned int GetUIntValue(void* obj) const
        {
            return *(unsigned int*)GetValuePtr(obj);
        }

        void SetUIntValue(void* obj, unsigned int value)
        {
            *(unsigned int*)GetValuePtr(obj) = value;
        }

        void SetValue(void* obj, const void* valuePtr) override
        {
            *(unsigned int*)GetValuePtr(obj) = *(const unsigned int*)valuePtr;
        }

        void SetValue(void* obj, const std::string& value) override
        {
            try
            {
                unsigned int uintValue = std::stoul(value);
                SetUIntValue(obj, uintValue);
            }
            catch (const std::exception& e)
            {
                SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                            "Failed to convert '%s' to unsigned int for property '%s': %s",
                            value.c_str(),
                            mName.c_str(),
                            e.what());
            }
        }

        void DumpValue(void* obj) const override
        {
            unsigned int uintValue = GetUIntValue(obj);
            SDL_Log("  %s: %u", mName.c_str(), uintValue);
        }

        PropertyType GetType() const override
        {
            return PropertyType::UInt;
        }
    };

    class FloatProperty : public Property
    {
    public:
        FloatProperty(const std::string& name, size_t offset, size_t size)
                : Property(name, offset, size)
        {
        }

        float GetFloatValue(void* obj) const
        {
            return *(float*)GetValuePtr(obj);
        }

        void SetFloatValue(void* obj, float value)
        {
            *(float*)GetValuePtr(obj) = value;
        }

        void SetValue(void* obj, const void* valuePtr) override
        {
            *(float*)GetValuePtr(obj) = *(const float*)valuePtr;
        }

        void SetValue(void* obj, const std::string& value) override
        {
            try
            {
                float floatValue = std::stof(value);
                SetFloatValue(obj, floatValue);
            }
            catch (const std::exception& e)
            {
                SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                            "Failed to convert '%s' to float for property '%s': %s",
                            value.c_str(),
                            mName.c_str(),
                            e.what());
            }
        }

        void DumpValue(void* obj) const override
        {
            float floatValue = GetFloatValue(obj);
            SDL_Log("  %s: %f", mName.c_str(), floatValue);
        }

        PropertyType GetType() const override
        {
            return PropertyType::Float;
        }
    };

    class DoubleProperty : public Property
    {
    public:
        DoubleProperty(const std::string& name, size_t offset, size_t size)
                : Property(name, offset, size)
        {
        }

        double GetDoubleValue(void* obj) const
        {
            return *(double*)GetValuePtr(obj);
        }

        void SetDoubleValue(void* obj, double value)
        {
            *(double*)GetValuePtr(obj) = value;
        }

        void SetValue(void* obj, const void* valuePtr) override
        {
            *(double*)GetValuePtr(obj) = *(const double*)valuePtr;
        }

        void SetValue(void* obj, const std::string& value) override
        {
            try
            {
                double doubleValue = std::stod(value);
                SetDoubleValue(obj, doubleValue);
            }
            catch (const std::exception& e)
            {
                SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                            "Failed to convert '%s' to double for property '%s': %s",
                            value.c_str(),
                            mName.c_str(),
                            e.what());
            }
        }

        void DumpValue(void* obj) const override
        {
            double doubleValue = GetDoubleValue(obj);
            SDL_Log("  %s: %f", mName.c_str(), doubleValue);
        }

        PropertyType GetType() const override
        {
            return PropertyType::Double;
        }
    };

    class BoolProperty : public Property
    {
    public:
        BoolProperty(const std::string& name, size_t offset, size_t size)
                : Property(name, offset, size)
        {
        }

        bool GetBoolValue(void* obj) const
        {
            return *(bool*)GetValuePtr(obj);
        }

        void SetBoolValue(void* obj, bool value)
        {
            *(bool*)GetValuePtr(obj) = value;
        }

        void SetValue(void* obj, const void* valuePtr) override
        {
            *(bool*)GetValuePtr(obj) = *(const bool*)valuePtr;
        }

        void SetValue(void* obj, const std::string& value) override
        {
            if (value == "1")
            {
                SetBoolValue(obj, true);
            }
            else if (value == "0")
            {
                SetBoolValue(obj, false);
            }
            else
            {
                SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                            "Failed to convert '%s' to bool for property '%s': expected '1'/'0'",
                            value.c_str(),
                            mName.c_str());
            }
        }

        void DumpValue(void* obj) const override
        {
            bool boolValue = GetBoolValue(obj);
            SDL_Log("  %s: %s", mName.c_str(), boolValue ? "true" : "false");
        }

        PropertyType GetType() const override
        {
            return PropertyType::Bool;
        }
    };

    class StringProperty : public Property
    {
    public:
        StringProperty(const std::string& name, size_t offset, size_t size)
                : Property(name, offset, size)
        {
        }

        std::string GetStringValue(void* obj) const
        {
            return *(std::string*)GetValuePtr(obj);
        }

        void SetStringValue(void* obj, const std::string& value)
        {
            *(std::string*)GetValuePtr(obj) = value;
        }

        void SetValue(void* obj, const void* valuePtr) override
        {
            *(std::string*)GetValuePtr(obj) = *(const std::string*)valuePtr;
        }

        void SetValue(void* obj, const std::string& value) override
        {
            SetStringValue(obj, value);
        }

        void DumpValue(void* obj) const override
        {
            std::string strValue = GetStringValue(obj);
            SDL_Log("  %s: %s", mName.c_str(), strValue.c_str());
        }

        PropertyType GetType() const override
        {
            return PropertyType::String;
        }
    };

    class CStringProperty : public Property
    {
    public:
        CStringProperty(const std::string& name, size_t offset, size_t size)
                : Property(name, offset, size)
        {
        }

        const char* GetCStringValue(void* obj) const
        {
            return *(const char**)GetValuePtr(obj);
        }

        void SetCStringValue(void* obj, const char* value)
        {
            *(const char**)GetValuePtr(obj) = value;
        }

        void SetValue(void* obj, const void* valuePtr) override
        {
            *(const char**)GetValuePtr(obj) = *(const char* const*)valuePtr;
        }

        void SetValue(void* obj, const std::string& value) override
        {
            SetCStringValue(obj, value.c_str());
        }

        void DumpValue(void* obj) const override
        {
            const char* cstrValue = GetCStringValue(obj);
            SDL_Log("  %s: %s", mName.c_str(), cstrValue ? cstrValue : "null");
        }

        PropertyType GetType() const override
        {
            return PropertyType::CString;
        }
    };

    class EnumProperty : public Property
    {
    private:
        std::unordered_map<int, std::string> mValueToText;
        std::unordered_map<std::string, int> mTextToValue;
        bool mStorageIsInt32;

    public:
        EnumProperty(const std::string& name,
                     size_t offset,
                     size_t size,
                     bool storageIsInt32 = true)
                : Property(name, offset, size)
                , mStorageIsInt32(storageIsInt32)
        {
        }

        void AddSerializedValue(int value, const std::string& text)
        {
            mValueToText[value] = text;
            mTextToValue[text] = value;
        }

        int GetIntValue(void* obj) const
        {
            void* valuePtr = GetValuePtr(obj);
            if (mStorageIsInt32)
            {
                return *(int*)valuePtr;
            }

            if (mSize == 1)
            {
                return static_cast<int>(*(uint8_t*)valuePtr);
            }
            if (mSize == 2)
            {
                return static_cast<int>(*(uint16_t*)valuePtr);
            }
            if (mSize == 4)
            {
                return static_cast<int>(*(uint32_t*)valuePtr);
            }

            SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                        "Unsupported enum storage size (%zu) for property '%s'",
                        mSize,
                        mName.c_str());
            return 0;
        }

        void SetIntValue(void* obj, int value)
        {
            void* valuePtr = GetValuePtr(obj);
            if (mStorageIsInt32)
            {
                *(int*)valuePtr = value;
                return;
            }

            if (mSize == 1)
            {
                *(uint8_t*)valuePtr = static_cast<uint8_t>(value);
                return;
            }
            if (mSize == 2)
            {
                *(uint16_t*)valuePtr = static_cast<uint16_t>(value);
                return;
            }
            if (mSize == 4)
            {
                *(uint32_t*)valuePtr = static_cast<uint32_t>(value);
                return;
            }

            SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                        "Unsupported enum storage size (%zu) for property '%s'",
                        mSize,
                        mName.c_str());
        }

        void SetValue(void* obj, const void* valuePtr) override
        {
            SetIntValue(obj, *(const int*)valuePtr);
        }

        void SetValue(void* obj, const std::string& value) override
        {
            const auto namedIt = mTextToValue.find(value);
            if (namedIt != mTextToValue.end())
            {
                SetIntValue(obj, namedIt->second);
                return;
            }

            try
            {
                int enumValue = std::stoi(value);
                SetIntValue(obj, enumValue);
            }
            catch (const std::exception& e)
            {
                SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                            "Failed to convert '%s' to enum for property '%s': %s",
                            value.c_str(),
                            mName.c_str(),
                            e.what());
            }
        }

        std::string GetSerializedValue(void* obj) const
        {
            const int intValue = GetIntValue(obj);
            const auto textIt = mValueToText.find(intValue);
            if (textIt != mValueToText.end())
            {
                return textIt->second;
            }

            return std::to_string(intValue);
        }

        void DumpValue(void* obj) const override
        {
            SDL_Log("  %s: %s", mName.c_str(), GetSerializedValue(obj).c_str());
        }

        PropertyType GetType() const override
        {
            return PropertyType::Enum;
        }
    };

    class ColorProperty : public Property
    {
    public:
        ColorProperty(const std::string& name, size_t offset, size_t size)
                : Property(name, offset, size)
        {
        }

        SDL_Color GetColorValue(void* obj) const
        {
            return *(SDL_Color*)GetValuePtr(obj);
        }

        void SetColorValue(void* obj, SDL_Color value)
        {
            *(SDL_Color*)GetValuePtr(obj) = value;
        }

        void SetValue(void* obj, const void* valuePtr) override
        {
            *(SDL_Color*)GetValuePtr(obj) = *(const SDL_Color*)valuePtr;
        }

        void SetValue(void* obj, const std::string& value) override
        {
            int r, g, b, a = 255;
            int count = sscanf(value.c_str(), "%d,%d,%d,%d", &r, &g, &b, &a);
            if (count >= 3)
            {
                SDL_Color color;
                color.r = static_cast<uint8_t>(r);
                color.g = static_cast<uint8_t>(g);
                color.b = static_cast<uint8_t>(b);
                color.a = static_cast<uint8_t>(a);
                SetColorValue(obj, color);
            }
            else
            {
                SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                            "Failed to convert '%s' to color for property '%s': expected 'R,G,B' "
                            "or 'R,G,B,A'",
                            value.c_str(),
                            mName.c_str());
            }
        }

        void DumpValue(void* obj) const override
        {
            SDL_Color color = GetColorValue(obj);
            SDL_Log("  %s: %d,%d,%d,%d", mName.c_str(), color.r, color.g, color.b, color.a);
        }

        PropertyType GetType() const override
        {
            return PropertyType::Color;
        }
    };

    class ObjectProperty : public Property
    {
    private:
        ClassInfo* mChildClassInfo;

    public:
        ObjectProperty(const std::string& name,
                       size_t offset,
                       size_t size,
                       ClassInfo* childClassInfo)
                : Property(name, offset, size)
                , mChildClassInfo(childClassInfo)
        {
        }

        ClassInfo* GetChildClassInfo() const override
        {
            return mChildClassInfo;
        }

        PropertyType GetType() const override
        {
            return PropertyType::Object;
        }
    };

    class PolygonProperty : public Property
    {
    public:
        PolygonProperty(const std::string& name, size_t offset, size_t size)
                : Property(name, offset, size)
        {
        }

        std::vector<SDL_FPoint>& GetPoints(void* obj) const
        {
            return *(std::vector<SDL_FPoint>*)GetValuePtr(obj);
        }

        void SetValue(void* obj, const std::string& value) override
        {
            ParseCSVToPoints(value, GetPoints(obj));
        }

        void DumpValue(void* obj) const override
        {
            size_t size = GetPoints(obj).size();
            SDL_Log("  %s: [polygon with %zu points]", mName.c_str(), size);
        }

        PropertyType GetType() const override
        {
            return PropertyType::Polygon;
        }
    };
 
    class PointProperty : public Property
    {
    public:
        PointProperty(const std::string& name, size_t offset, size_t size)
                : Property(name, offset, size)
        {
        }
 
        SDL_FPoint GetPointValue(void* obj) const
        {
            return *(SDL_FPoint*)GetValuePtr(obj);
        }
 
        void SetPointValue(void* obj, SDL_FPoint value)
        {
            *(SDL_FPoint*)GetValuePtr(obj) = value;
        }
 
        void SetValue(void* obj, const void* valuePtr) override
        {
            *(SDL_FPoint*)GetValuePtr(obj) = *(const SDL_FPoint*)valuePtr;
        }
 
        void SetValue(void* obj, const std::string& value) override
        {
            float x, y;
            if (sscanf(value.c_str(), "%f,%f", &x, &y) == 2)
            {
                SetPointValue(obj, SDL_FPoint{.x = x, .y = y});
            }
            else
            {
                SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                            "Failed to convert '%s' to point for property '%s': expected 'X,Y'",
                            value.c_str(),
                            mName.c_str());
            }
        }
 
        void DumpValue(void* obj) const override
        {
            SDL_FPoint point = GetPointValue(obj);
            SDL_Log("  %s: %f,%f", mName.c_str(), point.x, point.y);
        }
 
        PropertyType GetType() const override
        {
            return PropertyType::Point;
        }
    };

    template <typename T>
    class VectorProperty : public Property
    {
    private:
        std::string mItemKey; // Custom key name for array items (e.g., "Round")

        // SFINAE to detect if T is a pointer
        template <typename U>
        static constexpr bool is_pointer_v = std::is_pointer_v<U>;

    public:
        VectorProperty(const std::string& name,
                       size_t offset,
                       size_t size,
                       const std::string& itemKey = "")
                : Property(name, offset, size)
                , mItemKey(itemKey)
        {
        }

        std::vector<T>& GetVector(void* obj) const
        {
            return *(std::vector<T>*)GetValuePtr(obj);
        }

        size_t GetArraySize(void* obj) const override
        {
            return GetVector(obj).size();
        }

        void ResizeArray(void* obj, size_t newSize)
        {
            GetVector(obj).resize(newSize);
        }

        void ClearArray(void* obj)
        {
            GetVector(obj).clear();
        }

        T& GetElement(void* obj, size_t index) const
        {
            return GetVector(obj)[index];
        }

        void SetElement(void* obj, size_t index, const T& value)
        {
            GetVector(obj)[index] = value;
        }

        void PushElement(void* obj, const T& value)
        {
            GetVector(obj).push_back(value);
        }

        void SetValue(void* obj, const void* valuePtr) override
        {
            *(std::vector<T>*)GetValuePtr(obj) = *(const std::vector<T>*)valuePtr;
        }

        void SetValue(void* obj, const std::string& value) override
        {
            SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                        "String assignment not directly supported for array property '%s'",
                        mName.c_str());
        }

        void AddArrayEntry(void* obj, const std::string& value) override
        {
            if constexpr (std::is_pointer_v<T>)
            {
                SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                            "String array entry insertion not supported for pointer array '%s'",
                            mName.c_str());
            }
            else
            {
                try
                {
                    T convertedValue;
                    if constexpr (std::is_constructible_v<T, const std::string&>)
                    {
                        convertedValue = T(value);
                    }
                    else
                    {
                        std::istringstream iss(value);
                        iss >> convertedValue;
                        if (iss.fail())
                        {
                            throw std::runtime_error("failed stream conversion");
                        }
                    }

                    PushElement(obj, convertedValue);
                }
                catch (const std::exception& e)
                {
                    SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                                "Failed to add array entry for property '%s': %s",
                                mName.c_str(),
                                e.what());
                }
            }
        }

        void AddArrayObjectEntry(void* obj, void* valueObj) override
        {
            if constexpr (std::is_pointer_v<T>)
            {
                PushElement(obj, static_cast<T>(valueObj));
            }
            else
            {
                SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                            "AddArrayObjectEntry called on non-pointer array property '%s'",
                            mName.c_str());
            }
        }

        void DumpValue(void* obj) const override
        {
            size_t size = GetArraySize(obj);
            SDL_Log("  %s: [array of %zu elements]", mName.c_str(), size);
        }

        PropertyType GetType() const override
        {
            return PropertyType::Array;
        }

        bool ContainsObjects() const override
        {
            return is_pointer_v<T>;
        }

        const std::string& GetItemKey() const override
        {
            return mItemKey;
        }

        void SetItemKey(const std::string& itemKey)
        {
            mItemKey = itemKey;
        }
    };

    // Base template for shared map/unordered_map logic
    template <typename K, typename V, typename ContainerType>
    class BaseMapPropertyTemplate : public Property
    {
    public:
        BaseMapPropertyTemplate(const std::string& name, size_t offset, size_t size)
                : Property(name, offset, size)
        {
        }

    public:
        // Pure virtual - derived classes implement GetMapContainer() specific to their
        // container type
        virtual ContainerType& GetMapContainer(void* obj) const = 0;

        size_t GetMapSize(void* obj) const
        {
            return GetMapContainer(obj).size();
        }

        void ClearMap(void* obj)
        {
            GetMapContainer(obj).clear();
        }

        V& GetValue(void* obj, const K& key) const
        {
            return GetMapContainer(obj)[key];
        }

        void SetMapEntry(void* obj, const K& key, const V& value)
        {
            GetMapContainer(obj)[key] = value;
        }

        bool HasKey(void* obj, const K& key) const
        {
            auto& map = GetMapContainer(obj);
            return map.find(key) != map.end();
        }

        void SetValue(void* obj, const std::string& value) override
        {
            SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                        "String assignment not directly supported for map property '%s'",
                        mName.c_str());
        }

        void AddMapEntry(void* obj, const std::string& key, const std::string& value) override
        {
            // For maps with string key/value types, insert the entry directly
            // For other types, handle conversion
            try
            {
                // Attempt to construct K and V from strings and insert
                K strKey;
                V strValue;

                // For std::string types, direct construction works
                if constexpr (std::is_constructible_v<K, const std::string&>)
                {
                    strKey = K(key);
                }
                else
                {
                    // Try string stream conversion for numeric types
                    std::istringstream iss(key);
                    iss >> strKey;
                }

                if constexpr (std::is_constructible_v<V, const std::string&>)
                {
                    strValue = V(value);
                }
                else
                {
                    // Try string stream conversion for numeric types
                    std::istringstream iss(value);
                    iss >> strValue;
                }

                SetMapEntry(obj, strKey, strValue);
            }
            catch (const std::exception& e)
            {
                SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                            "Failed to add map entry for property '%s': %s",
                            mName.c_str(),
                            e.what());
            }
        }

        bool ContainsObjects() const override
        {
            // Check if V is a pointer type (indicates object value)
            return std::is_pointer_v<V>;
        }

        void AddMapObjectEntry(void* obj, const std::string& key, void* valueObj) override
        {
            // Add an object value to the map
            // Only works if V is a pointer type
            if constexpr (std::is_pointer_v<V>)
            {
                try
                {
                    K strKey;
                    if constexpr (std::is_constructible_v<K, const std::string&>)
                    {
                        strKey = K(key);
                    }
                    else
                    {
                        std::istringstream iss(key);
                        iss >> strKey;
                    }

                    GetMapContainer(obj)[strKey] = static_cast<V>(valueObj);
                }
                catch (const std::exception& e)
                {
                    SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                                "Failed to add object map entry for property '%s': %s",
                                mName.c_str(),
                                e.what());
                }
            }
            else
            {
                SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                            "AddMapObjectEntry called on non-pointer-value map property '%s'",
                            mName.c_str());
            }
        }
    };

    template <typename K, typename V>
    class MapProperty : public BaseMapPropertyTemplate<K, V, std::map<K, V>>
    {
    public:
        using Base = BaseMapPropertyTemplate<K, V, std::map<K, V>>;
        using Base::GetMapSize;
        using Base::GetValuePtr;
        using Base::mName;

        MapProperty(const std::string& name, size_t offset, size_t size)
                : BaseMapPropertyTemplate<K, V, std::map<K, V>>(name, offset, size)
        {
        }

        std::map<K, V>& GetMapContainer(void* obj) const override
        {
            return *(std::map<K, V>*)GetValuePtr(obj);
        }

        std::map<K, V>& GetMap(void* obj) const
        {
            return GetMapContainer(obj);
        }

        void SetValue(void* obj, const void* valuePtr) override
        {
            *(std::map<K, V>*)GetValuePtr(obj) = *(const std::map<K, V>*)valuePtr;
        }

        void DumpValue(void* obj) const override
        {
            size_t size = GetMapSize(obj);
            SDL_Log("  %s: [map with %zu entries]", mName.c_str(), size);
        }

        PropertyType GetType() const override
        {
            return PropertyType::Map;
        }
    };

    template <typename K, typename V>
    class UnorderedMapProperty : public BaseMapPropertyTemplate<K, V, std::unordered_map<K, V>>
    {
    public:
        using Base = BaseMapPropertyTemplate<K, V, std::unordered_map<K, V>>;
        using Base::GetMapSize;
        using Base::GetValuePtr;
        using Base::mName;

        UnorderedMapProperty(const std::string& name, size_t offset, size_t size)
                : BaseMapPropertyTemplate<K, V, std::unordered_map<K, V>>(name, offset, size)
        {
        }

        std::unordered_map<K, V>& GetMapContainer(void* obj) const override
        {
            return *(std::unordered_map<K, V>*)GetValuePtr(obj);
        }

        std::unordered_map<K, V>& GetMap(void* obj) const
        {
            return GetMapContainer(obj);
        }

        void SetValue(void* obj, const void* valuePtr) override
        {
            *(std::unordered_map<K, V>*)GetValuePtr(obj) =
                *(const std::unordered_map<K, V>*)valuePtr;
        }

        void DumpValue(void* obj) const override
        {
            size_t size = GetMapSize(obj);
            SDL_Log("  %s: [unordered_map with %zu entries]", mName.c_str(), size);
        }

        PropertyType GetType() const override
        {
            return PropertyType::UnorderedMap;
        }
    };
} // namespace nuvelocity

#endif // NVE_PROPERTY_H
