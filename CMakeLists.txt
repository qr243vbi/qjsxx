cmake_minimum_required(VERSION 3.16)

project(qjsxx VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include(CheckCXXSourceCompiles)

# =====================================================================
# --- FUNCTION FOR STDC++EXP VERIFICATION ---
# =====================================================================
function(check_exp out_boolean)    
    unset(COMPILER_CAN_LINK_STDCPPEXP CACHE)

    set(CMAKE_REQUIRED_LIBRARIES "stdc++exp")

    check_cxx_source_compiles("
        int main() { return 0; }
    " COMPILER_CAN_LINK_STDCPPEXP)

    
    set(${out_boolean} ${COMPILER_CAN_LINK_STDCPPEXP} PARENT_SCOPE)
endfunction()

# =====================================================================
# --- BUILT-IN STATE EXTRACTION (NO COMPILING REQUIRED) ---
# =====================================================================
function(check_standard std_version out_boolean)    
    set(FEATURE_TO_FIND "cxx_std_${std_version}")

    if(FEATURE_TO_FIND IN_LIST CMAKE_CXX_COMPILE_FEATURES)
        set(COMPILER_SUPPORTS_CPP TRUE)
    else()
        set(COMPILER_SUPPORTS_CPP FALSE)
    endif()

    set(${out_boolean} ${COMPILER_SUPPORTS_CPP} PARENT_SCOPE)
endfunction()

check_standard("23" CPP_23_SUPPORTED)
check_exp(EXP_SUPPORTED)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(QJS_USE_STACKTRACE_DEFAULT OFF)
else()
    set(QJS_USE_STACKTRACE_DEFAULT ON)
endif()

option(QJS_USE_STACKTRACE "Use stacktrace if available" ${QJS_USE_STACKTRACE_DEFAULT})
if (NOT QJS_USE_STACKTRACE)
    set(CPP_23_SUPPORTED OFF)
endif()

if (${CPP_23_SUPPORTED})
    set(CXX_STANDARD 23)
else()
    set(CXX_STANDARD 20)
endif()

file(GLOB  QUICKJS_SOURCES  CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/quickjs_*.cpp"
)


find_package(qjs CONFIG QUIET)
if(qjs_FOUND)
    message(STATUS "Using system QuickJS-NG")
else()
    message(STATUS "System QuickJS-NG not found, using bundled version")
    add_subdirectory("${PROJECT_SOURCE_DIR}/qjs")
endif()


add_library(qjsxx_static STATIC
    ${QUICKJS_SOURCES}
)
add_library(qjsxx SHARED
    ${QUICKJS_SOURCES}
)

message(STATUS "Using -std=c++${CXX_STANDARD}")

foreach(target qjsxx qjsxx_static)
    target_link_libraries(${target} PRIVATE qjs)
    target_compile_features(${target} PUBLIC cxx_std_${CXX_STANDARD})
    if (${CPP_23_SUPPORTED})
        target_compile_definitions(${target} PUBLIC QJS_USE_STACKTRACE)
        if (${EXP_SUPPORTED})
            target_link_libraries(${target} PRIVATE stdc++exp)
        endif() 

        target_compile_options(${target} PRIVATE
            $<$<CXX_COMPILER_ID:GNU,Clang>:-g>
            $<$<CXX_COMPILER_ID:MSVC>:/Zi>
        )

        target_link_options(${target} PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/DEBUG>
        )
    endif()
    set_target_properties(${target} PROPERTIES
        OUTPUT_NAME qjsxx
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
endforeach()

add_executable(qjsxx_test test_library.cpp)

message(STATUS "Compiling tests")
target_link_libraries(qjsxx_test PRIVATE qjsxx)
set_target_properties(qjsxx_test PROPERTIES
    CXX_STANDARD ${CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
add_custom_command(TARGET qjsxx_test POST_BUILD
    COMMAND $<TARGET_FILE:qjsxx_test>
)

if (${CPP_23_SUPPORTED})
    target_compile_definitions(qjsxx_test PUBLIC QJS_USE_STACKTRACE)
    if (${EXP_SUPPORTED})
        target_link_libraries(qjsxx_test PRIVATE stdc++exp)
    endif() 

    target_compile_options(qjsxx_test PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-g>
        $<$<CXX_COMPILER_ID:MSVC>:/Zi>
    )

    target_link_options(qjsxx_test PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/DEBUG>
    )
endif()