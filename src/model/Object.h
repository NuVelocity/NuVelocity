#ifndef NVE_OBJECT_H
#define NVE_OBJECT_H

#include "API.h"
#include "ClassInfo.h"
#include "Property.h"
#include <cstdint>
#include <type_traits>
#include <typeinfo>

namespace nuvelocity
{
    // FIXME: This is inefficient because this means every object has a member for each
    // "dynamic" property. We should move this to a separate component or use a more efficient
    // storage method for storing dynamic properties. This will all go away eventually.
    class PaletteHolder; // Forward declaration
    class ObjectBase
    {
    private:
        std::string mComment;
        PaletteHolder* mPaletteHolder;

    public:
        virtual ~ObjectBase() = default;
    };

    // CRTP template providing automatic reflection with optional customization hook
    template <typename Derived, typename Base = ObjectBase>
    class Object : public Base
    {
    private:
        // SFINAE to detect if Derived has static InitClassInfo method
        template <typename T>
        static auto HasInitClassInfo(int)
            -> decltype(T::InitClassInfo(std::declval<ClassInfo&>()), std::true_type{});

        template <typename T>
        static std::false_type HasInitClassInfo(...);

        static constexpr bool HasInitClassInfoMethod =
            decltype(HasInitClassInfo<Derived>(0))::value;

        static ClassInfo GetClassInfoInternal()
        {
            ClassInfo info;
            info.mName = typeid(Derived).name();

            // Set base class info if Base is not ObjectBase
            if constexpr (std::is_same_v<Base, ObjectBase>)
            {
                info.mBaseClassInfo = nullptr;
            }
            else
            {
                info.mBaseClassInfo = Base::GetClassInfo();
            }

            info.mFactoryFunction = []() -> void* { return new Derived(); };

            // Call derived class's InitClassInfo
            Derived::InitClassInfo(info);

            return info;
        }

    protected:
        // Helper to detect if a type is a std::vector
        template <typename T>
        struct is_vector : std::false_type
        {
        };

        template <typename T, typename A>
        struct is_vector<std::vector<T, A>> : std::true_type
        {
            using element_type = T;
        };

        // Helper to detect if a type is a std::map
        template <typename T>
        struct is_map : std::false_type
        {
        };

        template <typename K, typename V, typename C>
        struct is_map<std::map<K, V, C>> : std::true_type
        {
            using key_type = K;
            using value_type = V;
        };

        // Helper to detect if a type is a std::unordered_map
        template <typename T>
        struct is_unordered_map : std::false_type
        {
        };

        template <typename K, typename V, typename H, typename E>
        struct is_unordered_map<std::unordered_map<K, V, H, E>> : std::true_type
        {
            using key_type = K;
            using value_type = V;
        };

        template <typename MemberType>
        static void AddPropertyImpl(ClassInfo& info, const char* name,
                                    MemberType Derived::* memberPtr,
                                    const std::string& arrayItemKey)
        {
            size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<Derived*>(0)->*memberPtr));
            size_t size = sizeof(MemberType);

            // Type dispatch to create appropriate Property subclass
            Property* prop = nullptr;
            if constexpr (std::is_same_v<MemberType, std::string>)
            {
                prop = new StringProperty(name, offset, size);
            }
            else if constexpr (std::is_same_v<MemberType, bool>)
            {
                prop = new BoolProperty(name, offset, size);
            }
            else if constexpr (std::is_same_v<MemberType, int> ||
                               std::is_same_v<MemberType, int16_t>)
            {
                prop = new IntProperty(name, offset, size);
            }
            else if constexpr (std::is_same_v<MemberType, unsigned int> ||
                               std::is_same_v<MemberType, uint16_t>)
            {
                prop = new UIntProperty(name, offset, size);
            }
            else if constexpr (std::is_same_v<MemberType, float>)
            {
                prop = new FloatProperty(name, offset, size);
            }
            else if constexpr (std::is_same_v<MemberType, double>)
            {
                prop = new DoubleProperty(name, offset, size);
            }
            else if constexpr (is_vector<MemberType>::value)
            {
                using ElementType = typename is_vector<MemberType>::element_type;
                auto* arrayProp = new ArrayProperty<ElementType>(name, offset, size, arrayItemKey);
                prop = arrayProp;
            }
            else if constexpr (is_map<MemberType>::value)
            {
                using KeyType = typename is_map<MemberType>::key_type;
                using ValueType = typename is_map<MemberType>::value_type;
                prop = new MapProperty<KeyType, ValueType>(name, offset, size);
            }
            else if constexpr (is_unordered_map<MemberType>::value)
            {
                using KeyType = typename is_unordered_map<MemberType>::key_type;
                using ValueType = typename is_unordered_map<MemberType>::value_type;
                prop = new UnorderedMapProperty<KeyType, ValueType>(name, offset, size);
            }
            else
            {
                // Fallback to generic Property
                prop = new Property(name, offset, size);
            }

            info.AddProperty(prop);
        }

        // Helper to automatically register properties using member pointers
        template <typename MemberType>
        static void AddProperty(ClassInfo& info, const char* name, MemberType Derived::* memberPtr)
        {
            AddPropertyImpl(info, name, memberPtr, "");
        }

        // arrayItemKey is only valid for vector properties
        template <typename MemberType>
        static void AddProperty(ClassInfo& info, const char* name, MemberType Derived::* memberPtr,
                                const std::string& arrayItemKey)
        {
            static_assert(is_vector<MemberType>::value,
                          "arrayItemKey can only be provided for std::vector properties");
            AddPropertyImpl(info, name, memberPtr, arrayItemKey);
        }

    public:
        static ClassInfo* GetClassInfo()
        {
            static_assert(
                HasInitClassInfoMethod,
                "Derived class must implement static void InitClassInfo(ClassInfo& aInfo)");
            static ClassInfo classInfo = GetClassInfoInternal();
            return &classInfo;
        }
    };
} // namespace nuvelocity

#endif // NVE_OBJECT_H
