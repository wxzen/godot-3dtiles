#ifndef _HTTPCLIENT_TEST_
#define _HTTPCLIENT_TEST_

#include <godot_cpp/classes/http_request.hpp>
#include <godot_cpp/core/version.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace CesiumForGodot
{

    namespace HttpUtils
    {
        void request( godot::String url );

    } // namespace HttpUtils

} // namespace CesiumForGodot

#endif