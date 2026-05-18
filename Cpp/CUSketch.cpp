
#include <algorithm>
#include "header/CUSketch.h"

CUSketch::CUSketch(float memory_kb) {

    this->counter_bits = 32;
    this->depth = 4;

    uint32_t total_counters = static_cast<uint32_t>(std::round(memory_kb * 1024 * 8 / this->counter_bits));

    width = static_cast<int>(std::round(total_counters / depth));

    counters = std::vector<std::vector<uint32_t>>(depth, std::vector<uint32_t>(width, 0));
}




void CUSketch::update(const uint32_t flow_label, uint32_t weight) {

    std::pair<int, uint32_t> min_indices[depth];
    int min_count = 0;
    uint32_t min_val = UINT32_MAX;

    for (int i = 0; i < depth; i++) {
        uint32_t hash_value = 0;
        MurmurHash3_x86_32(&flow_label, 4, i, &hash_value);
        uint32_t j = hash_value % width;
        uint32_t current_val = counters[i][j];

        if (current_val < min_val) {
            min_val = current_val;
            min_indices[0] = {i, j};
            min_count = 1;
        } else if (current_val == min_val) {
            min_indices[min_count] = {i, j};
            min_count++;
        }
    }

    for (int k = 0; k < min_count; k++) {
        int r = min_indices[k].first;
        uint32_t c = min_indices[k].second;
        counters[r][c] = std::clamp(counters[r][c] + weight, 0u, UINT32_MAX);
    }
}




uint32_t CUSketch::query(const uint32_t flow_label) {

    uint32_t min_value = 0xFFFFFFFF;
    uint32_t hash_value = 0;
    for (int i = 0; i < depth; i++) {
        MurmurHash3_x86_32(&flow_label, 4, i, &hash_value);
        uint32_t j = hash_value % width;
        uint32_t val = counters[i][j];
        if (val < min_value) {
            min_value = val;
        }
    }
    return min_value;
}
