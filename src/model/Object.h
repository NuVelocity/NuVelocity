#ifndef NVE_OBJECT_H
#define NVE_OBJECT_H

#include "API.h"
#include "ClassInfo.h"
#include "Property.h"
#include <cstdint>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

namespace nuvelocity
{
    class ObjectBase
    {
    private:
        void* mDynamicProperties = nullptr;

    public:
        virtual ~ObjectBase()
        {
            if (mDynamicProperties)
            {
                auto* map = static_cast<
                    std::unordered_map<std::string, std::tuple<bool, void*, ClassInfo*>>*>(
                    mDynamicProperties);
                for (auto& pair : *map)
                {
                    if (std::get<1>(pair.second) != nullptr)
                    {
                        if (!std::get<0>(pair.second)) // false means string
                        {
                            delete static_cast<std::string*>(std::get<1>(pair.second));
                        }
                        else
                        {
                            // Other dynamic properties like Palette are derived from ObjectBase
                            delete static_cast<ObjectBase*>(std::get<1>(pair.second));
                        }
                    }
                }
                delete map;
            }
        }

        void* GetDynamicProperty(const std::string& key) const
        {
            if (!mDynamicProperties)
                return nullptr;
            auto* map =
                static_cast<std::unordered_map<std::string, std::tuple<bool, void*, ClassInfo*>>*>(
                    mDynamicProperties);
            auto it = map->find(key);
            if (it != map->end())
            {
                return std::get<1>(it->second);
            }
            return nullptr;
        }

        void SetDynamicProperty(const std::string& key,
                                void* value,
                                bool isObject = false,
                                ClassInfo* typeInfo = nullptr)
        {
            if (!mDynamicProperties)
            {
                mDynamicProperties =
                    new std::unordered_map<std::string, std::tuple<bool, void*, ClassInfo*>>();
            }
            auto* map =
                static_cast<std::unordered_map<std::string, std::tuple<bool, void*, ClassInfo*>>*>(
                    mDynamicProperties);
            (*map)[key] = std::make_tuple(isObject, value, typeInfo);
        }

        const void* GetDynamicPropertiesMap() const
        {
            return mDynamicProperties;
        }

        // Optional vector-based hook for argument-based initialization.
        virtual void InitFromArgs(const std::vector<std::string>& args)
        {
            (void)args;
        }
    };

    // CRTP template providing automatic reflection with optional customization hook
    template <typename Derived, typename Base = ObjectBase>
    class Object : public Base
    {
    private:
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
        // Helper to detect if a type is a pointer to an Object-derived class
        template <typename T>
        struct is_object_ptr : std::false_type
        {
        };

        template <typename T>
        struct is_object_ptr<T*> : std::bool_constant<std::is_base_of_v<ObjectBase, T>>
        {
            using element_type = T;
        };

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
        static Property* AddPropertyImpl(ClassInfo& info,
                                         const char* name,
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
            else if constexpr (std::is_same_v<MemberType, int64_t>)
            {
                prop = new Int64Property(name, offset, size);
            }
            else if constexpr (std::is_same_v<MemberType, unsigned int> ||
                               std::is_same_v<MemberType, uint16_t>)
            {
                prop = new UIntProperty(name, offset, size);
            }
            else if constexpr (std::is_same_v<MemberType, uint64_t>)
            {
                prop = new UInt64Property(name, offset, size);
            }
            else if constexpr (std::is_same_v<MemberType, float>)
            {
                prop = new FloatProperty(name, offset, size);
            }
            else if constexpr (std::is_same_v<MemberType, double>)
            {
                prop = new DoubleProperty(name, offset, size);
            }
            else if constexpr (std::is_same_v<MemberType, SDL_Color>)
            {
                prop = new ColorProperty(name, offset, size);
            }
            else if constexpr (std::is_same_v<MemberType, SDL_FPoint>)
            {
                prop = new PointProperty(name, offset, size);
            }
            else if constexpr (is_vector<MemberType>::value)
            {
                using ElementType = typename is_vector<MemberType>::element_type;
                ClassInfo* elementInfo = nullptr;
                if constexpr (is_object_ptr<ElementType>::value)
                {
                    elementInfo = is_object_ptr<ElementType>::element_type::GetClassInfo();
                }
                auto* vectorProp =
                    new VectorProperty<ElementType>(name, offset, size, arrayItemKey, elementInfo);
                prop = vectorProp;
            }
            else if constexpr (is_map<MemberType>::value)
            {
                using KeyType = typename is_map<MemberType>::key_type;
                using ValueType = typename is_map<MemberType>::value_type;
                ClassInfo* valueInfo = nullptr;
                if constexpr (is_object_ptr<ValueType>::value)
                {
                    valueInfo = is_object_ptr<ValueType>::element_type::GetClassInfo();
                }
                prop = new MapProperty<KeyType, ValueType>(name, offset, size, valueInfo);
            }
            else if constexpr (is_unordered_map<MemberType>::value)
            {
                using KeyType = typename is_unordered_map<MemberType>::key_type;
                using ValueType = typename is_unordered_map<MemberType>::value_type;
                ClassInfo* valueInfo = nullptr;
                if constexpr (is_object_ptr<ValueType>::value)
                {
                    valueInfo = is_object_ptr<ValueType>::element_type::GetClassInfo();
                }
                prop = new UnorderedMapProperty<KeyType, ValueType>(name, offset, size, valueInfo);
            }
            else if constexpr (is_object_ptr<MemberType>::value)
            {
                using ObjectType = typename is_object_ptr<MemberType>::element_type;
                prop = new ObjectProperty(name, offset, size, ObjectType::GetClassInfo());
            }
            else
            {
                // Fallback to generic Property
                prop = new Property(name, offset, size);
            }

            info.AddProperty(prop);
            return prop;
        }

        // Helper to automatically register properties using member pointers
        template <typename MemberType>
        static void AddProperty(ClassInfo& info, const char* name, MemberType Derived::* memberPtr)
        {
            AddPropertyImpl(info, name, memberPtr, "");
        }

        template <typename MemberType>
        static void
        AddDeprecatedProperty(ClassInfo& info, const char* name, MemberType Derived::* memberPtr)
        {
            Property* prop = AddPropertyImpl(info, name, memberPtr, "");
            if (prop != nullptr)
            {
                prop->SetDeprecated(true);
            }
        }

        // arrayItemKey is only valid for vector properties
        template <typename MemberType>
        static void AddProperty(ClassInfo& info,
                                const char* name,
                                MemberType Derived::* memberPtr,
                                const std::string& arrayItemKey)
        {
            static_assert(is_vector<MemberType>::value,
                          "arrayItemKey can only be provided for std::vector properties");
            AddPropertyImpl(info, name, memberPtr, arrayItemKey);
        }

        static void AddPolygonProperty(ClassInfo& info,
                                       const char* name,
                                       std::vector<SDL_FPoint> Derived::* memberPtr)
        {
            size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<Derived*>(0)->*memberPtr));
            size_t size = sizeof(std::vector<SDL_FPoint>);
            Property* prop = new PolygonProperty(name, offset, size);
            info.AddProperty(prop);
        }

        // Helper to register an enum/int property with custom serialized value text.
        template <typename MemberType, typename MappingContainer>
        static void AddEnumProperty(ClassInfo& info,
                                    const char* name,
                                    MemberType Derived::* memberPtr,
                                    const MappingContainer& serializedValues)
        {
            static_assert(std::is_enum_v<MemberType> || std::is_integral_v<MemberType>,
                          "MemberType must be enum or integral for enum-backed properties");

            size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<Derived*>(0)->*memberPtr));
            size_t size = sizeof(MemberType);

            constexpr bool storageIsInt32 = std::is_integral_v<MemberType>;
            auto* prop = new EnumProperty(name, offset, size, storageIsInt32);
            for (const auto& [value, text] : serializedValues)
            {
                prop->AddSerializedValue(value, text);
            }

            info.AddProperty(prop);
        }

    public:
        static ClassInfo* GetClassInfo()
        {
            static_assert(
                requires(ClassInfo& info) { Derived::InitClassInfo(info); },
                "Derived class must implement static void InitClassInfo(ClassInfo& info)");
            static ClassInfo classInfo = GetClassInfoInternal();
            return &classInfo;
        }
    };
} // namespace nuvelocity

#endif // NVE_OBJECT_H
