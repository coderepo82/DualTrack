# DualTrack

**DualTrack** is an efficient method designed to jointly identify heavy hitters and their heavy quadratic elements in high-speed data streams. It addresses the challenge of capturing heavy hitters while uncovering their high-contributing elements, achieving high identification accuracy and processing efficiency with low memory overhead.

This repository provides a complete C++ implementation of DualTrack, together with baseline algorithms and demo datasets to facilitate evaluation, reproducibility, and further experimentation. It also includes P4/BMv2 and FPGA implementations in the `implementation/` directory.

## Repository Structure

This repository contains three main directories:

```
DualTrack/
├── Cpp/
│   ├── source code implementation
│   └── reproduction instructions (README)
├── dataset/
│   ├── demo files for public datasets
│   └── instructions for obtaining the full datasets (README)
└── implementation/
    ├── P4/     # P4/BMv2 implementation and README
    └── FPGA/   # HLS C++ source, generated Verilog, and README
```

- The "Cpp/" folder provides the full C++17 implementation of DualTrack and related baseline algorithms, along with detailed build and execution instructions.

- The "dataset/" folder includes small demo files for quick testing, links to the official sources of public datasets used in the experiments.

- The "implementation/" folder contains the P4/BMv2 and FPGA implementations. 

---

This repository allows researchers and practitioners to reproduce the experimental results, test new configurations, and extend DualTrack to other data stream analysis tasks.
