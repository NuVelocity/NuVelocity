#ifndef NVE_PROPERTY_H
#define NVE_PROPERTY_H

#include "LogCategory.h"
#include <string>
#include <cstring>

namespace nuvelocity
{
    class Property
    {
    protected:
        std::string mName;
        std::string mDescription;

        size_t mOffset;
        size_t mSize;

    public:
        Property(const std::string& name, size_t offset, size_t size, const std::string& description = "")
            : mName(name)
            , mOffset(offset)
            , mSize(size)
            , mDescription(description) {}

        virtual ~Property() = default;

        const std::string& GetName() const { return mName; }
        const std::string& GetDescription() const { return mDescription; }

        void SetDescription(const std::string& description) { mDescription = description; }

        inline void* GetValuePtr(void* obj) const {
            char* base = (char*)obj;
            return (void*)(base + mOffset);
        }

        virtual void* GetValue(void* obj) const {
            return GetValuePtr(obj);
        }

        virtual void SetValue(void* obj, const void* valuePtr) {
            char* dest = (char*)obj + mOffset;
            memcpy(dest, valuePtr, mSize);
        }

        virtual void SetValue(void* obj, const std::string& value) {
            SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                "String assignment not supported for property '%s'", mName.c_str());
        }

        virtual void DumpValue(void* obj) const {
            void* valuePtr = GetValue(obj);
            SDL_Log("  %s: %p", mName.c_str(), valuePtr);
        }

        void DumpMetadata() const {
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
            : Property(name, offset, size) {}

        int GetIntValue(void* obj) const {
            return *(int*)GetValuePtr(obj);
        }

        void SetIntValue(void* obj, int value) {
            *(int*)GetValuePtr(obj) = value;
        }

        void SetValue(void* obj, const void* valuePtr) override {
            *(int*)GetValuePtr(obj) = *(const int*)valuePtr;
        }

        void SetValue(void* obj, const std::string& value) override {
            try {
                int intValue = std::stoi(value);
                SetIntValue(obj, intValue);
            } catch (const std::exception& e) {
                SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                    "Failed to convert '%s' to int for property '%s': %s",
                    value.c_str(), mName.c_str(), e.what());
            }
        }

        void DumpValue(void* obj) const override {
            int intValue = GetIntValue(obj);
            SDL_Log("  %s: %d", mName.c_str(), intValue);
        }
    };

    class UIntProperty : public Property
    {
    public:
        UIntProperty(const std::string& name, size_t offset, size_t size)
            : Property(name, offset, size) {}

        unsigned int GetUIntValue(void* obj) const {
            return *(unsigned int*)GetValuePtr(obj);
        }

        void SetUIntValue(void* obj, unsigned int value) {
            *(unsigned int*)GetValuePtr(obj) = value;
        }

        void SetValue(void* obj, const void* valuePtr) override {
            *(unsigned int*)GetValuePtr(obj) = *(const unsigned int*)valuePtr;
        }

        void SetValue(void* obj, const std::string& value) override {
            try {
                unsigned int uintValue = std::stoul(value);
                SetUIntValue(obj, uintValue);
            } catch (const std::exception& e) {
                SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                    "Failed to convert '%s' to unsigned int for property '%s': %s",
                    value.c_str(), mName.c_str(), e.what());
            }
        }

        void DumpValue(void* obj) const override {
            unsigned int uintValue = GetUIntValue(obj);
            SDL_Log("  %s: %u", mName.c_str(), uintValue);
        }
    };

    class FloatProperty : public Property
    {
    public:
        FloatProperty(const std::string& name, size_t offset, size_t size)
            : Property(name, offset, size) {}

        float GetFloatValue(void* obj) const {
            return *(float*)GetValuePtr(obj);
        }

        void SetFloatValue(void* obj, float value) {
            *(float*)GetValuePtr(obj) = value;
        }

        void SetValue(void* obj, const void* valuePtr) override {
            *(float*)GetValuePtr(obj) = *(const float*)valuePtr;
        }

        void SetValue(void* obj, const std::string& value) override {
            try {
                float floatValue = std::stof(value);
                SetFloatValue(obj, floatValue);
            } catch (const std::exception& e) {
                SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                    "Failed to convert '%s' to float for property '%s': %s",
                    value.c_str(), mName.c_str(), e.what());
            }
        }

        void DumpValue(void* obj) const override {
            float floatValue = GetFloatValue(obj);
            SDL_Log("  %s: %f", mName.c_str(), floatValue);
        }
    };

    class DoubleProperty : public Property
    {
    public:
        DoubleProperty(const std::string& name, size_t offset, size_t size)
            : Property(name, offset, size) {}

        double GetDoubleValue(void* obj) const {
            return *(double*)GetValuePtr(obj);
        }

        void SetDoubleValue(void* obj, double value) {
            *(double*)GetValuePtr(obj) = value;
        }

        void SetValue(void* obj, const void* valuePtr) override {
            *(double*)GetValuePtr(obj) = *(const double*)valuePtr;
        }

        void SetValue(void* obj, const std::string& value) override {
            try {
                double doubleValue = std::stod(value);
                SetDoubleValue(obj, doubleValue);
            } catch (const std::exception& e) {
                SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                    "Failed to convert '%s' to double for property '%s': %s",
                    value.c_str(), mName.c_str(), e.what());
            }
        }

        void DumpValue(void* obj) const override {
            double doubleValue = GetDoubleValue(obj);
            SDL_Log("  %s: %f", mName.c_str(), doubleValue);
        }
    };

    class BoolProperty : public Property
    {
    public:
        BoolProperty(const std::string& name, size_t offset, size_t size)
            : Property(name, offset, size) {}

        bool GetBoolValue(void* obj) const {
            return *(bool*)GetValuePtr(obj);
        }

        void SetBoolValue(void* obj, bool value) {
            *(bool*)GetValuePtr(obj) = value;
        }

        void SetValue(void* obj, const void* valuePtr) override {
            *(bool*)GetValuePtr(obj) = *(const bool*)valuePtr;
        }

        void SetValue(void* obj, const std::string& value) override {
            if (value == "1") {
                SetBoolValue(obj, true);
            } else if (value == "0") {
                SetBoolValue(obj, false);
            } else {
                SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                    "Failed to convert '%s' to bool for property '%s': expected '1'/'0'",
                    value.c_str(), mName.c_str());
            }
        }

        void DumpValue(void* obj) const override {
            bool boolValue = GetBoolValue(obj);
            SDL_Log("  %s: %s", mName.c_str(), boolValue ? "true" : "false");
        }
    };

    class StringProperty : public Property
    {
    public:
        StringProperty(const std::string& name, size_t offset, size_t size)
            : Property(name, offset, size) {}

        std::string GetStringValue(void* obj) const {
            return *(std::string*)GetValuePtr(obj);
        }

        void SetStringValue(void* obj, const std::string& value) {
            *(std::string*)GetValuePtr(obj) = value;
        }

        void SetValue(void* obj, const void* valuePtr) override {
            *(std::string*)GetValuePtr(obj) = *(const std::string*)valuePtr;
        }

        void SetValue(void* obj, const std::string& value) override {
            SetStringValue(obj, value);
        }

        void DumpValue(void* obj) const override {
            std::string strValue = GetStringValue(obj);
            SDL_Log("  %s: %s", mName.c_str(), strValue.c_str());
        }
    };

    class CStringProperty : public Property
    {
    public:
        CStringProperty(const std::string& name, size_t offset, size_t size)
            : Property(name, offset, size) {}

        const char* GetCStringValue(void* obj) const {
            return *(const char**)GetValuePtr(obj);
        }

        void SetCStringValue(void* obj, const char* value) {
            *(const char**)GetValuePtr(obj) = value;
        }

        void SetValue(void* obj, const void* valuePtr) override {
            *(const char**)GetValuePtr(obj) = *(const char* const*)valuePtr;
        }

        void SetValue(void* obj, const std::string& value) override {
            SetCStringValue(obj, value.c_str());
        }

        void DumpValue(void* obj) const override {
            const char* cstrValue = GetCStringValue(obj);
            SDL_Log("  %s: %s", mName.c_str(), cstrValue ? cstrValue : "null");
        }
    };

    class EnumProperty : public Property
    {
    public:
        EnumProperty(const std::string& name, size_t offset, size_t size)
            : Property(name, offset, size) {}

        int GetIntValue(void* obj) const {
            return *(int*)GetValuePtr(obj);
        }

        void SetIntValue(void* obj, int value) {
            *(int*)GetValuePtr(obj) = value;
        }

        void SetValue(void* obj, const void* valuePtr) override {
            *(int*)GetValuePtr(obj) = *(const int*)valuePtr;
        }

        void DumpValue(void* obj) const override {
            int intValue = GetIntValue(obj);
            SDL_Log("  %s: %d", mName.c_str(), intValue);
        }
    };
} // namespace nuvelocity

#endif // NVE_PROPERTY_H
