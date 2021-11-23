cmake_minimum_required(VERSION 3.11...3.16)

project(hpfw
        VERSION 0.0.1
        DESCRIPTION "Audio HashPrint framework"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

add_library(hpfw INTERFACE)

find_package(Eigen3 3.3 REQUIRED NO_MODULE)
target_link_directories(hpfw INTERFACE /usr/local/lib)
target_include_directories(
        hpfw
        INTERFACE

        include/

        /usr/include
        # /usr/include/mkl
        /usr/local/include

        /opt/intel/oneapi/mkl/2021.4.0/include
        /usr/local/Cellar/fmt/8.0.1/include
        /usr/local/include/taglib

        thirdparty/cpp-taskflow-2.2.0
        thirdparty/annoy-1.16.3/src
        thirdparty
)

target_link_directories(
        hpfw
        INTERFACE

        /usr/lib
)

target_link_libraries(
        hpfw
        INTERFACE

        Eigen3::Eigen
        tbb

        essentia
        fftw3f
        fftw3
        yaml
        samplerate
        avcodec
        avdevice
        avfilter
        avformat
        avresample
        avutil
        tag
        chromaprint
        samplerate

        /usr/local/Cellar/fmt/8.0.1/lib/libfmt.a

        # MKL RELATED
        pthread
        m
        dl
        /opt/intel/oneapi/mkl/2021.4.0/lib/libmkl_intel_lp64.a
        /opt/intel/oneapi/mkl/2021.4.0/lib/libmkl_sequential.a
        /opt/intel/oneapi/mkl/2021.4.0/lib/libmkl_core.a
        /opt/intel/oneapi/mkl/2021.4.0/lib/libmkl_blacs_mpich_lp64.a
        /opt/intel/oneapi/mkl/2021.4.0/lib/libmkl_scalapack_ilp64.a
)
target_compile_definitions(
        hpfw
        INTERFACE

        -DUSE_FFTW
        -DEIGEN_USE_MKL_ALL
)

target_compile_options(
        hpfw
        INTERFACE

        -m64

        -ffast-math
        -Ofast
        -march=native

        -pedantic
        -Wall
        -Wextra
        -Wcast-align
        -Wcast-qual
        -Wctor-dtor-privacy
        -Wformat=2
        -Winit-self
        -Wmissing-declarations
        -Wmissing-include-dirs
        -Woverloaded-virtual
        -Wredundant-decls
        -Wsign-promo
        -Wstrict-overflow=5
        -Wswitch-default
        -Wundef
        -Wno-unused
)

add_subdirectory(examples/cpp)
add_subdirectory(modules/python)
