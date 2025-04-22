#ifndef CESIUM_H
#define CESIUM_H

#include "godot_cpp/classes/object.hpp"

namespace godot
{
    class ClassDB;
};

class Cesium : public godot::Object
{
    GDCLASS( Cesium, godot::Object )

public:
    static godot::String version();
    static godot::String godotCPPVersion();

private:
    static void _bind_methods();
};

#endif
