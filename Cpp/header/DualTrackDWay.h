#ifndef DUALTRACK_DWAY_H
#define DUALTRACK_DWAY_H

#include <cstdint>
#include <map>
#include <utility>
#include <vector>

// Independent HT set-associative variant. Like DualTrack, ID 0 is reserved
// for empty entries. QT and the frequency estimation methods are unchanged.
class DualTrackDWay {
public:
    // Change this value to test d = 1, 2, 4, ... without changing main.cpp.
    static constexpr uint32_t DEFAULT_D = 4;

    explicit DualTrackDWay(float memory_kb, uint32_t d_val = DEFAULT_D);
    // Default configuration with a shared seed for paired comparisons.
    DualTrackDWay(float memory_kb, uint32_t d_val, uint32_t seed);
    DualTrackDWay(float memory_kb, uint32_t k_val, float m_frac,
                  uint32_t method_code, uint32_t d_val = DEFAULT_D);
    // Explicit seed for reproducible comparisons of different d values.
    DualTrackDWay(float memory_kb, uint32_t k_val, float m_frac,
                  uint32_t method_code, uint32_t d_val, uint32_t seed);

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
        uint32_t U = 0;
        uint32_t C = 0;
        uint32_t V = 0;
        // Remains attached to the physical slot after its flow is cleared.
        // Sum D over the set to obtain the set's cumulative frequency loss.
        uint32_t D = 0;
    };

    struct QTEntry {
        uint32_t E = 0;
        uint32_t R = 0;
        uint32_t P = 0; // Absolute HT slot index, not just the set index.
    };

    std::vector<HTSlot> heavy_table;
    std::vector<QTEntry> quad_table;
    uint32_t m1;
    uint32_t m2;
    uint32_t k;
    uint32_t d;
    uint32_t set_count;
    float m_ht_frac;
    uint32_t method;
    uint32_t rand_seed;

    uint32_t hashFlow(uint32_t x) const;
    uint32_t setLoss(uint32_t slot_index) const;
    uint32_t estimateFrequency(uint32_t slot_index) const;
    std::map<uint32_t, uint32_t> queryElements(uint32_t slot_index, uint32_t hash_val) const;
    void insertFlow(uint32_t slot_index, uint32_t x);
    void clearFlowRecord(uint32_t slot_index);
    void replaceCell(uint32_t cell_index, uint32_t slot_index, uint32_t y);
};

#endif // DUALTRACK_DWAY_H
