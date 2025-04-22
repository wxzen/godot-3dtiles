#ifndef _CESIUM_FOR_GODOT_ENUMS_
#define _CESIUM_FOR_GODOT_ENUMS_

namespace CesiumForGodot
{
    enum class GLTFComponentType : unsigned int
    {
        BYTE = 5120,
        UNSIGNED_BYTE = 5121,
        SHORT = 5122,
        UNSIGNED_SHORT = 5123,
        UNSIGNED_INT = 5125,
        FLOAT = 5126
    };

    enum class GLTFAccessorType : unsigned int
    {
        SCALAR = 0,
        VEC2 = 1,
        VEC3 = 2,
        VEC4 = 3,
        MAT2 = 4,
        MAT3 = 5,
        MAT4 = 6
    };

} // namepace CesiumForGodot

#endif