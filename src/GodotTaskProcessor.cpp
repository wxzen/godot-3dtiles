#pragma once

#include <future>
#include <functional>

#include "GodotTaskProcessor.h"


namespace CesiumForGodot {

  void GodotTaskProcessor::startTask(std::function<void()> f) {
    std::async(std::launch::async, f);    
  }

} // namespace CesiumForGodot

