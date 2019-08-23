file(GLOB DIRECTTK_SRC "*.cpp" "*.c")
include_directories(${LearnDX12Project}/include)
SET(CMAKE_BUILD_TYPE "Release")
add_library(DirectXTK STATIC ${DIRECTTK_SRC})
