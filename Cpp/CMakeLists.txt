cmake_minimum_required(VERSION 3.26)
project(HH_QuadraticEle)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -funroll-loops -ffast-math -DNDEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")

add_executable(HH_QuadraticEle main.cpp
        MurmurHash3.cpp
        header/DualTrack.h
        DualTrack.cpp
        header/DualTrackDWay.h
        DualTrackDWay.cpp
        header/DualTrackNoLoss.h
        DualTrackNoLoss.cpp
        header/utils.h
        utils.cpp
        header/GlobalHH.h
        GlobalHH.cpp
        header/TwoDMisraGries.h
        TwoDMisraGries.cpp
        header/CocoSketch.h
        CocoSketch.cpp
        header/HyperUSS.h
        HyperUSS.cpp
        header/DUET_CU.h
        DUET_CU.cpp
        header/CUSketch.h
        CUSketch.cpp
        header/CountMin.h
        CountMin.cpp
)


