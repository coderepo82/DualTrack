#ifndef DUALTRACK_H
#define DUALTRACK_H

#include <vector>
#include <cstdint>
#include <map>
#include "utils.h"
#include "MurmurHash3.h"

// DualTrack: An Efficient Method for Joint Heavy Hitter and Heavy Quadratic Element Identification

// Bucket in HeavyTable
struct HTBucket {
    uint32_t F;
    uint32_t U;
    uint32_t C;
    uint32_t V;
    uint32_t D;

    HTBucket() : F(0), U(0), C(0), V(0), D(0) {}
};

// Cell in QuadTable
struct QTCell {
    uint32_t E; // element ID
    uint32_t R; // residual frequency count
    // For implementation convenience, P does NOT store the explicit parent flow ID.
    // Instead, it stores the bucket index of its parent flow in HT.
    // The parent ID is recovered from the referenced live HT slot.
    uint32_t P;
    QTCell() : E(0), R(0), P(0) {}
};

class DualTrack {
private:
    std::vector<HTBucket> heavy_table;
    std::vector<QTCell> quad_table;

    uint32_t m1;
    uint32_t m2;
    uint32_t k;
    float m_ht_frac;
    uint32_t method;

    uint32_t rand_seed;

public:

    DualTrack(float memory_kb);

    // Parameters for the sensitivity and ablation experiments.
    DualTrack(float memory_kb, uint32_t k_val, float m_frac, uint32_t method_code);

    ~DualTrack();

    // Reuse the baseline's actual hash seed in controlled variant comparisons.
    uint32_t getHashSeed() const { return rand_seed; }

    // x is flow label, y is element label, (x, y) equals (f, e)
    void update(uint32_t x, uint32_t y);

    std::pair<std::map<uint32_t, uint32_t>,
    std::map<uint32_t, std::map<uint32_t, uint32_t>>> query(uint32_t heavy_hitter_th);

    std::pair<uint32_t, std::map<uint32_t, uint32_t>>querySingleFlow(uint32_t flow_id);

    void evaluation(const std::vector<std::pair<uint32_t, uint32_t>>& dataset,
                    const std::map<uint32_t, uint32_t>& flows,
                    std::map<uint32_t, std::map<uint32_t, uint32_t>> quadratic_eles,
                    uint32_t heavy_hitter_th, float phi);

};

#endif // DUALTRACK_H
