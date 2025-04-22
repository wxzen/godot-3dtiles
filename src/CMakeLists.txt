# SPDX-License-Identifier: Unlicense

file(GLOB_RECURSE godot_3dtiles_src CONFIGURE_DEPENDS 
        "*.h"
        "*.cpp")

target_sources( ${PROJECT_NAME}
    PRIVATE
        ${godot_3dtiles_src}
)

target_include_directories( ${PROJECT_NAME}
    PRIVATE
        "src"
)
