cmake_minimum_required(VERSION 3.28)
project(ParallelComputing_TerrainGenerator LANGUAGES C CXX CUDA)

set(CMAKE_CUDA_ARCHITECTURES "75")
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

find_package(CUDAToolkit REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories("C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.5/include")

add_executable(ParallelComputing_TerrainGenerator
        src/main.cpp
        src/cpu_terrain.cpp
        src/utils.cpp
        src/gpu_terrain.cu
        include/cpu_terrain.h
        include/utils.h
        include/gpu_terrain.cuh
        include/constants.h
)

set_target_properties(ParallelComputing_TerrainGenerator PROPERTIES
        CUDA_SEPARABLE_COMPILATION OFF)
set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} --ptxas-options=-v")