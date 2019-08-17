cmake_minimum_required(VERSION 2.8)

project(LearnDX12)

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()

add_definitions(-DWIN_MAIN)
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /subsystem:windows") 

set(LearnDX12Project ${PROJECT_SOURCE_DIR})
set(INCLUDE ${LearnDX12Project}/include ${LearnDX12Project}/base)
include_directories(${INCLUDE})
add_definitions(-DSK_DATA_DIR_W=L\"${CMAKE_SOURCE_DIR}/data/\")
add_definitions(-DSK_DATA_DIR=\"${CMAKE_SOURCE_DIR}/data/\")
link_directories(${LearnDX12Project}/lib)


IF( CMAKE_SIZEOF_VOID_P EQUAL 8)
    #64位静态链接库
    link_directories(${LearnDX12Project}/lib)
ELSE( CMAKE_SIZEOF_VOID_P EQUAL 8)
    #32位静态链接库
    link_directories(${LearnDX12Project}/lib32)  
ENDIF( CMAKE_SIZEOF_VOID_P EQUAL 8)

link_libraries(d3d12.lib)
link_libraries(dxgi.lib)
link_libraries(d3dcompiler.lib)
link_libraries(assimp.lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${LearnDX12Project}/bin)   
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${LearnDX12Project}/bin)   
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LearnDX12Project}/bin)   
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LearnDX12Project}/bin) 


add_subdirectory(${LearnDX12Project}/base)
link_libraries(base)
add_subdirectory(${LearnDX12Project}/src)
