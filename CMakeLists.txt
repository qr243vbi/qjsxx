cmake_minimum_required(VERSION 3.16)

project(qjsxx VERSION 1.0 LANGUAGES CXX)

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

if (${CPP_23_SUPPORTED})
    set(CMAKE_CXX_STANDARD 23)
else()
    set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB  QUICKJS_SOURCES  CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/quickjs_*.cpp"
)

add_library(qjsxx
    ${QUICKJS_SOURCES}
)

add_executable(qjsxx_test test_library.cpp)

find_package(qjs CONFIG QUIET)
if(qjs_FOUND)
    message(STATUS "Using system QuickJS-NG")
else()
    message(STATUS "System QuickJS-NG not found, using bundled version")
    add_subdirectory("${PROJECT_SOURCE_DIR}/qjs")
endif()

message(STATUS "Using -std=c++${CMAKE_CXX_STANDARD}")

target_link_libraries(qjsxx PRIVATE qjs)
if (${EXP_SUPPORTED})
    target_link_libraries(qjsxx PRIVATE stdc++exp)
endif() 
target_compile_features(qjsxx PUBLIC cxx_std_${CMAKE_CXX_STANDARD})

if (${CPP_23_SUPPORTED})
    target_compile_definitions(qjsxx PUBLIC QJS_USE_STACKTRACE)

    message(STATUS "Compiling tests")
    target_link_libraries(qjsxx_test PRIVATE qjsxx)
    set_target_properties(qjsxx_test PROPERTIES
        CXX_STANDARD ${CMAKE_CXX_STANDARD}
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
    add_custom_command(TARGET qjsxx_test POST_BUILD
        COMMAND $<TARGET_FILE:qjsxx_test>
    )

    if (${EXP_SUPPORTED})
        target_link_libraries(qjsxx_test PRIVATE stdc++exp)
    endif() 
else()
  message(STATUS "Skip tests")
endif()