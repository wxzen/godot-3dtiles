# SPDX-License-Identifier: Unlicense

add_custom_target( templates
    SOURCES
        template.debug.gdextension.in
        template.release.gdextension.in
)

add_dependencies( ${PROJECT_NAME} templates )

# We shouldn't be relying on CMAKE_BUILD_TYPE (see https://github.com/asmaloney/GDExtensionTemplate/issues/25)
# But until we fix it here and in godot-cpp, ensure it's one we expect.
set ( ALLOWED_BUILDS "Debug;Release" )
if ( NOT "${CMAKE_BUILD_TYPE}" IN_LIST ALLOWED_BUILDS )
    message( FATAL_ERROR "CMAKE_BUILD_TYPE must be set to Debug or Release" )
endif()

# Get our gdextension input file name based on build type
string( TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE )
set( GD_EXTENSION_FILE_INPUT template.${BUILD_TYPE}.gdextension.in )
# set( GD_EXTENSION_FILE_INPUT template.gdextension.in )

# Workaround to add the "lib" prefix to the library in our template file if using MSYS2.
if ( MINGW )
    set( LIB_PREFIX "lib")
endif()

# Generate our project's .gdextension file from the template
set( GD_EXTENSION_FILE ${PROJECT_NAME}.gdextension )
configure_file( ${GD_EXTENSION_FILE_INPUT} ${PROJECT_BINARY_DIR}/${GD_EXTENSION_FILE} )

# Debug message to verify the paths
message(STATUS "GD_EXTENSION_FILE_INPUT: ${GD_EXTENSION_FILE_INPUT}")
message(STATUS "GD_EXTENSION_FILE: ${PROJECT_BINARY_DIR}/${GD_EXTENSION_FILE}")
message(STATUS "BUILD_OUTPUT_DIR: ${BUILD_OUTPUT_DIR}")

# Install the gdextension file from the build directory
install(
    FILES ${PROJECT_BINARY_DIR}/${GD_EXTENSION_FILE}
    DESTINATION ${INSTALL_DIR}
)

unset( ALLOWED_BUILDS )
unset( BUILD_TYPE )
unset( GD_EXTENSION_FILE )
unset( GD_EXTENSION_FILE_INPUT )
unset( LIB_PREFIX )
