#ifndef GODOT_TASK_PROCESSOR_H
#define GODOT_TASK_PROCESSOR_H

#include <CesiumAsync/ITaskProcessor.h>

namespace CesiumForGodot
{
    class GodotTaskProcessor : public CesiumAsync::ITaskProcessor
    {
    public:
        virtual void startTask( std::function<void()> f ) override;
    };
} // namespace CesiumForGodot

#endif