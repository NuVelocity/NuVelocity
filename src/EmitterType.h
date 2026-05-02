#ifndef NVE_EMITTER_TYPE_H
#define NVE_EMITTER_TYPE_H

#include <string>

namespace nuvelocity
{
    enum class EmitterType
    {
        Burst,
        Continuous,
        Pulse
    };

    inline EmitterType StringToEmitterType(const std::string& str)
    {
        if (str == "Continuous")
        {
            return EmitterType::Continuous;
        }
        if (str == "Pulse")
        {
            return EmitterType::Pulse;
        }
        return EmitterType::Burst;
    }

    inline std::string EmitterTypeToString(EmitterType type)
    {
        switch (type)
        {
        case EmitterType::Continuous:
            return "Continuous";
        case EmitterType::Pulse:
            return "Pulse";
        case EmitterType::Burst:
        default:
            return "Burst";
        }
    }

    enum class EmitterShape
    {
        Cone,
        Circle,
        Rectangle
    };

    inline EmitterShape StringToEmitterShape(const std::string& str)
    {
        if (str == "Circle")
        {
            return EmitterShape::Circle;
        }
        if (str == "Rectangle")
        {
            return EmitterShape::Rectangle;
        }
        return EmitterShape::Cone;
    }

    inline std::string EmitterShapeToString(EmitterShape shape)
    {
        switch (shape)
        {
        case EmitterShape::Circle:
            return "Circle";
        case EmitterShape::Rectangle:
            return "Rectangle";
        case EmitterShape::Cone:
        default:
            return "Cone";
        }
    }
} // namespace nuvelocity

#endif // NVE_EMITTER_TYPE_H
