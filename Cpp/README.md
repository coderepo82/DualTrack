# Code Reproduction

## Project Structure

This project implements several algorithms for joint heavy hitter and heavy quadratic element identification, including DualTrack, DUET[1], GlobalHH[2], 2D-MG[3], CocoSketch[4], and Hyper-USS[5].

The directory structure is as follows:

```
Cpp/
├── CMakeLists.txt
├── main.cpp
├── CocoSketch.cpp
├── CountMin.cpp
├── CUSketch.cpp
├── DualTrack.cpp
├── DualTrackDWay.cpp
├── DualTrackNoLoss.cpp
├── DUET_CU.cpp
├── GlobalHH.cpp
├── HyperUSS.cpp
├── MurmurHash3.cpp
├── TwoDMisraGries.cpp
├── utils.cpp
└── header/
    ├── CocoSketch.h
    ├── CountMin.h
    ├── CUSketch.h
    ├── DualTrack.h
    ├── DualTrackDWay.h
    ├── DualTrackNoLoss.h
    ├── DUET_CU.h
    ├── GlobalHH.h
    ├── HyperUSS.h
    ├── MurmurHash3.h
    ├── TwoDMisraGries.h
    └── utils.h
```

## Implementation and Environment

The project is written in **C++17** and built using **CMake**.  We perform our experiments on a machine equipped with a 13th Gen Intel(R) Core(TM) i7-13700 2.10 GHz processor. You can follow the steps below to build and run the project, and test with your desired parameters.

## Build Instructions

```bash
cd Cpp
mkdir build && cd build
cmake ..
make
```

After compilation, an executable file named `HH_QuadraticEle` will be generated in the `build` directory.  You can run the program with the following command:

```bash
./HH_QuadraticEle
```

> **Note**: Before building and running, please update the dataset file paths in both `loadDataSet` functions in `main.cpp` to your local paths, or write your own data loading function to test with your own dataset.

In `main.cpp`, set `run_no_loss_experiment` or `run_dway_experiment` to `true` to enable the corresponding variant experiment (both default to `false`). 

## References

> [1] Jiaqian Liu, Haipeng Dai, Rui Xia, Meng Li, Ran Ben Basat, Rui Li, and Guihai Chen. Duet: A generic framework for finding special quadratic elements in data streams. In Proceedings of the ACM Web Conference 2022, pages 2989–2997, 2022.
> 
> [2] Katsiaryna Mirylenka, Themis Palpanas, Graham Cormode, and Divesh Srivastava. Finding interesting correlations with conditional heavy hitters. In 2013 IEEE 29th International Conference on Data Engineering (ICDE), pages 1069–1080. IEEE, 2013.
> 
> [3] Bibudh Lahiri, Arko Provo Mukherjee, and Srikanta Tirthapura. Identifying correlated heavy-hitters in a two-dimensional data stream. Data mining and knowledge discovery, 30(4):797–818, 2016.
> 
> [4] Ruijie Miao, Yinda Zhang, Zihao Zheng, Ruixin Wang, Ruwen Zhang, Tong Yang, Zaoxing Liu, and Junchen Jiang. CocoSketch: High-performance sketch-based measurement over arbitrary partial key query[J]. IEEE/ACM Transactions on Networking, 2023, 31(6): 2653-2668.
> 
> [5] Ce Zheng, Zhouran Shi, Ruijie Miao, Wenpu Liu, Tong Yang,  Bin Cui,  and Steve Uhlig. Answering Subset Query Over Multi-Attribute Data Streams Using Hyper-USS[J]. IEEE Transactions on Knowledge and Data Engineering, 2025.
