#ifndef DUALTRACK_NO_UPPER_BOUND_H
#define DUALTRACK_NO_UPPER_BOUND_H

#include <cstdint>
#include <map>
#include <utility>
#include <vector>

// No Upper Bound (No Up) ablation: DualTrack's single-slot HT without U/D.
// Corresponds to DualTrackLowerBound / run_lower_bound_experiment in the development project.
// Each HT slot keeps F/C/V. U/D storage and updates are removed, and their
// memory is reclaimed for capacity. As in DualTrack, IDs must be nonzero.
class DualTrackNoUpperBound {
public:
    explicit DualTrackNoUpperBound(float memory_kb);
    // Default configuration with a shared seed for paired comparisons.
    DualTrackNoUpperBound(float memory_kb, uint32_t seed);
    DualTrackNoUpperBound(float memory_kb, uint32_t k_val, float m_frac);
    DualTrackNoUpperBound(float memory_kb, uint32_t k_val, float m_frac, uint32_t seed);

    uint32_t getHashSeed() const { return rand_seed; }
    uint32_t getHTSlotCount() const { return m1; }
    uint32_t getQTCellCount() const { return m2; }

    void update(uint32_t x, uint32_t y);

    std::pair<std::map<uint32_t, uint32_t>,
              std::map<uint32_t, std::map<uint32_t, uint32_t>>>
    query(uint32_t heavy_hitter_th);

    std::pair<uint32_t, std::map<uint32_t, uint32_t>>
    querySingleFlow(uint32_t flow_id);

    void evaluation(const std::vector<std::pair<uint32_t, uint32_t>>& dataset,
                    const std::map<uint32_t, uint32_t>& flows,
                    std::map<uint32_t, std::map<uint32_t, uint32_t>> quadratic_eles,
                    uint32_t heavy_hitter_th, float ele_th_phi);

private:
    struct HTSlot {
        uint32_t F = 0;
        uint32_t C = 0;
        uint32_t V = 0;
    };
    static_assert(sizeof(HTSlot) == 12, "No Upper Bound HT slots must contain only F, C and V");

    struct QTEntry {
        uint32_t E = 0;
        uint32_t R = 0;
        uint32_t P = 0; // Absolute HT slot index, as in DualTrack.
    };

    std::vector<HTSlot> heavy_table;
    std::vector<QTEntry> quad_table;
    uint32_t m1;
    uint32_t m2;
    uint32_t k;
    float m_ht_frac;
    uint32_t rand_seed;

    uint32_t hashFlow(uint32_t x) const;
    void replaceCell(uint32_t cell_index, uint32_t slot_index, uint32_t y);
    std::map<uint32_t, uint32_t> queryElements(uint32_t slot_index,
                                             uint32_t hash_val) const;
};

#endif // DUALTRACK_NO_UPPER_BOUND_H
