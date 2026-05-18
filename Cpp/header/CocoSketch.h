#ifndef COCOSKETCH_H
#define COCOSKETCH_H

#include <vector>
#include <cstdint>
#include <map>
#include <random>
#include "utils.h"
#include "MurmurHash3.h"

struct Bucket {
    uint64_t key;
    uint32_t value;

    Bucket() : key(0), value(0) {}
};


// An C++ implementation of CocoSketch: High Performance Sketch-based Measurement over Arbitrary Partial Key Query
// DOI: https://doi.org/10.1145/3452296.3472892

// CocoSketch is proposed for the problem of querying arbitrary partial key subsets,
// and hence can also be used to detect heavy hitters and heavy quadratic elements.
// e.g., the (flow ID, element ID) as the full key, while flow ID as the partial key
// Here, we implement it for this problem.

class CocoSketch {
private:
    int d;              // num of bucket array
    int l;              // length for each bucket array

    std::vector<std::vector<Bucket>> table;

    std::vector<uint32_t> rand_seeds; // random seeds

    // C++11 Random members
    std::mt19937 gen;
    std::uniform_real_distribution<double> dis;


public:

    CocoSketch(float memory_kb);

    ~CocoSketch();

    void update(uint64_t key, uint32_t weight = 1);

    // Retrieve heavy hitters and quadratic elements by aggregating the sketch results.
    std::pair<std::map<uint32_t, uint32_t>, std::map<uint32_t, std::map<uint32_t, uint32_t>>> getHHAndHotQuadEle(uint32_t heavy_hitter_th, float phi);

    std::pair<uint32_t, std::map<uint32_t, uint32_t>> querySingleFlow(uint32_t flow_id);

    void evaluation(const std::vector<std::pair<uint32_t, uint32_t>>& dataset,
                    const std::map<uint32_t, uint32_t>& flows,
                    std::map<uint32_t, std::map<uint32_t, uint32_t>> quadratic_eles,
                    uint32_t heavy_hitter_th, float ele_th_phi);
};

#endif