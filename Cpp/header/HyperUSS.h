

#ifndef HYPERUSS_H
#define HYPERUSS_H

#include <cmath>
#include <vector>
#include <cstdint>
#include <map>
#include <random>
#include "utils.h"
#include "MurmurHash3.h"


// An C++ implementation of Hyper-USS: Answering Subset ery Over Multi-Aribute Data Stream
// DOI: https://dl.acm.org/doi/10.1145/3580305.3599383

// HyperUSS is designed for multi-attribute measurement with high fidelity using Joint Variance Optimization.
// It can handle arbitrary partial key queries by storing full keys and managing multiple attributes.
// In this implementation, we simplify it to a single attribute (i.e., frequency) to match the context
// of detecting heavy hitters and heavy quadratic elements.


struct USS_Bucket {
    uint64_t key; // the full key,
    uint32_t value; // Here we consider frequency because the problem only involves this single attribute.

    USS_Bucket() : key(0), value(0) {}
};

class HyperUSS {
private:
    int d; // Number of arrays
    int w; // Number of buckets per array
    std::vector<std::vector<USS_Bucket>> table;

    // Random number generation
    std::mt19937 gen;
    std::uniform_real_distribution<double> dis;

    std::vector<uint32_t> rand_seeds; // random seeds for d arrays


public:

    HyperUSS(float memory_kb);

    // The problem only involves the frequency attribute, therefore
    // we set the original input (e, v1, v2, ..., vn) to (e, v1),
    // i.e., the full key and attribute v1 (frequency)
    void update(uint64_t key_, uint32_t v1 = 1);

    // Retrieve heavy hitters and quadratic elements by aggregating the sketch results.
    std::pair<std::map<uint32_t, uint32_t>, std::map<uint32_t, std::map<uint32_t, uint32_t>>>
    getHHAndHotQuadEle(uint32_t heavy_hitter_th, float phi);

    std::pair<uint32_t, std::map<uint32_t, uint32_t>> querySingleFlow(uint32_t flow_id);

    void evaluation(const std::vector<std::pair<uint32_t, uint32_t>>& dataset,
                    const std::map<uint32_t, uint32_t>& flows,
                    std::map<uint32_t, std::map<uint32_t, uint32_t>> quadratic_eles,
                    uint32_t heavy_hitter_th, float ele_th_phi);

};



#endif
