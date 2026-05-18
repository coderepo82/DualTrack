# DualSketch

**DualSketch** is an efficient sketch designed to jointly identify heavy hitters and their heavy quadratic elements in high-speed data streams. It addresses the challenge of capturing heavy hitters while uncovering their high-contributing elements, achieving high identification accuracy and processing efficiency with low memory overhead.

This repository provides a complete C++ implementation of DualSketch, together with baseline algorithms and demo datasets to facilitate evaluation, reproducibility, and further experimentation.

## Repository Structure

This repository contains two main directories:

```
DualSketch/
├── Cpp/
│   ├── source code implementation
│   └── reproduction instructions (README)
└── dataset/
    ├── demo files for public datasets
    └── instructions for obtaining the full datasets (README)
```

- The "Cpp/" folder provides the full C++17 implementation of DualSketch and related baseline algorithms, along with detailed build and execution instructions.

- The "dataset/" folder includes small demo files for quick testing, links to the official sources of public datasets used in the experiments.

---

This repository allows researchers and practitioners to reproduce the experimental results, test new configurations, and extend DualSketch to other data stream analysis tasks.