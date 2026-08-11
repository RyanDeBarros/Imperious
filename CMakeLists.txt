cmake_minimum_required(VERSION 3.20)
project(imperious VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (POLICY CMP0141)
	cmake_policy(SET CMP0141 NEW)
	set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

add_library(imperious STATIC)

if (MSVC)
	target_compile_options(imperious PUBLIC /Zc:__cplusplus)
endif()

target_include_directories(imperious PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(imp)
