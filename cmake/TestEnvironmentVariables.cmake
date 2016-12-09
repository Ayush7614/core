#
# Test Environment Variables by Parra Studios
# Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
# Utility for defining cross-platform environment variables in tests
#

if(TEST_ENVIRONMENT_VARIABLES_FOUND)
    return()
endif()

set(TEST_ENVIRONMENT_VARIABLES_FOUND YES)

get_filename_component(TEST_ENVIRONMENT_VARIABLES_PATH ${CMAKE_CURRENT_LIST_FILE} PATH)

function(test_environment_variables target env_vars)

    if(MSVC)

        set(MSVC_LOCAL_DEBUGGER_ENVIRONMENT ${env_vars})

        string(REGEX REPLACE "=${PROJECT_BINARY_DIR}/;" "=$(OutDir);" MSVC_LOCAL_DEBUGGER_ENVIRONMENT "${MSVC_LOCAL_DEBUGGER_ENVIRONMENT}")
        string(REGEX REPLACE ";" "\n" MSVC_LOCAL_DEBUGGER_ENVIRONMENT "${MSVC_LOCAL_DEBUGGER_ENVIRONMENT}")

        if(MSVC_VERSION GREATER 1800)
            set(MSVC_TOOLS_VERSION "12.0")
        elseif(MSVC_VERSION GREATER 1500)
            set(MSVC_TOOLS_VERSION "4.0")
        elseif(MSVC_VERSION GREATER 1400)
            set(MSVC_TOOLS_VERSION "3.5")
        elseif(MSVC_VERSION GREATER 1301)
            set(MSVC_TOOLS_VERSION "3.0")
        elseif(MSVC_VERSION GREATER 1300)
            set(MSVC_TOOLS_VERSION "1.1")
        else()
            set(MSVC_TOOLS_VERSION "1.0")
        endif()

        set(MSVC_LOCAL_PROPERTY_GROUPS "")

        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(MSVC_PLATFORM "x64")
        else()
            set(MSVC_PLATFORM "Win32")
        endif()

        if(MSVC_VERSION GREATER 1500)

            set(MSVC_PROPERTY_GROUP_TEMPLATE " \
                <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='@MSVC_BUILD_CONFIG@|@MSVC_PLATFORM@'\">\n \
                    <LocalDebuggerEnvironment>\n@MSVC_LOCAL_DEBUGGER_ENVIRONMENT@\n$(LocalDebuggerEnvironment)\n \
                    </LocalDebuggerEnvironment>\n \
                    <LocalDebuggerWorkingDirectory>$(OutDir)</LocalDebuggerWorkingDirectory>\n \
                    <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>\n \
                </PropertyGroup>\n \
            ")

            set(MSVC_PROJECT_TEMPLATE "${TEST_ENVIRONMENT_VARIABLES_PATH}/ProjectTemplate.vcxproj.user.in")
            set(MSVC_PROJECT_OUTPUT_EXT "vcxproj")

            foreach(MSVC_BUILD_CONFIG ${CMAKE_CONFIGURATION_TYPES})

                string(CONFIGURE ${MSVC_PROPERTY_GROUP_TEMPLATE} MSVC_PROPERTY_GROUP)

                set(MSVC_LOCAL_PROPERTY_GROUPS "${MSVC_LOCAL_PROPERTY_GROUPS}${MSVC_PROPERTY_GROUP}\n")

            endforeach()

        else()

        endif()

        configure_file(${MSVC_PROJECT_TEMPLATE} ${CMAKE_CURRENT_BINARY_DIR}/${target}.${MSVC_PROJECT_OUTPUT_EXT}.user @ONLY)

        set(MSVC_LOCAL_CTEST_ENVIRONMENT ${env_vars})

        string(REGEX REPLACE "=${PROJECT_BINARY_DIR}/;" "=${PROJECT_BINARY_DIR}/$<CONFIGURATION>/;" MSVC_LOCAL_CTEST_ENVIRONMENT "${MSVC_LOCAL_CTEST_ENVIRONMENT}")
        string(REGEX REPLACE ";" "\\\\;" MSVC_LOCAL_CTEST_ENVIRONMENT "${MSVC_LOCAL_CTEST_ENVIRONMENT}")
        string(REGEX REPLACE "\\\\;" ";" MSVC_LOCAL_CTEST_ENVIRONMENT "${MSVC_LOCAL_CTEST_ENVIRONMENT}")

        set_property(TEST ${target}
            PROPERTY ENVIRONMENT "${MSVC_LOCAL_CTEST_ENVIRONMENT}"
        )
    else()
        set_property(TEST ${target}
            PROPERTY ENVIRONMENT "${env_vars}"
        )
    endif()

endfunction()
