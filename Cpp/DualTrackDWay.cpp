#include "header/DualTrackDWay.h"
#include "header/MurmurHash3.h"
#include "header/utils.h"

#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

DualTrackDWay::DualTrackDWay(float memory_kb, uint32_t d_val)
    : DualTrackDWay(memory_kb, 32, 0.6f, 3, d_val) {}

DualTrackDWay::DualTrackDWay(float memory_kb, uint32_t d_val, uint32_t seed)
    : DualTrackDWay(memory_kb, 32, 0.6f, 3, d_val, seed) {}

DualTrackDWay::DualTrackDWay(float memory_kb, uint32_t k_val, float m_frac,
                             uint32_t method_code, uint32_t d_val)
    : DualTrackDWay(memory_kb, k_val, m_frac, method_code, d_val,
                    generateSeeds32(1)[0]) {}

DualTrackDWay::DualTrackDWay(float memory_kb, uint32_t k_val, float m_frac,
                             uint32_t method_code, uint32_t d_val, uint32_t seed)
    : k(k_val), d(d_val), m_ht_frac(m_frac), method(method_code), rand_seed(seed) {
    if (!std::isfinite(memory_kb) || memory_kb <= 0 ||
        !std::isfinite(m_frac) || m_frac <= 0 || m_frac >= 1 || k == 0 || d == 0) {
        throw std::invalid_argument("DualTrackDWay: invalid memory, HT fraction, k, or d");
    }

    // Use the same logical bit-budget model as DualTrack: 160 bits per HT
    // slot, and 64 + ceil(log2(slot_budget)) bits per QT cell. In particular,
    // increasing d does not multiply the HT capacity or enlarge QT.
    const float memory_ht = memory_kb * m_ht_frac;
    const float slot_count = std::round(memory_ht * 1024 * 8 / 160);
    if (slot_count < d || static_cast<double>(slot_count) >
                          std::numeric_limits<uint32_t>::max()) {
        throw std::invalid_argument("DualTrackDWay: HT cannot hold one full d-way set");
    }
    const uint32_t slot_budget = static_cast<uint32_t>(slot_count);
    set_count = slot_budget / d;
    m1 = set_count * d; // At most d - 1 budgeted slots remain unused.

    // Base pointer width on the original budget so QT capacity remains the
    // same for all d, even when slot_budget is not divisible by d.
    const uint32_t qt_cell_bits = 64 +
        static_cast<uint32_t>(std::ceil(std::log2(slot_budget)));
    const float memory_qt = memory_kb - memory_ht;
    const float cell_count = std::round(memory_qt * 1024 * 8 / qt_cell_bits);
    if (cell_count < k || static_cast<double>(cell_count) >
                         std::numeric_limits<uint32_t>::max()) {
        throw std::invalid_argument("DualTrackDWay: QT cannot hold k cells");
    }
    m2 = static_cast<uint32_t>(cell_count);
    heavy_table.resize(m1);
    quad_table.resize(m2);
}

uint32_t DualTrackDWay::hashFlow(uint32_t x) const {
    uint32_t hash_val = 0;
    // Preserve the baseline implementation's hashing, including its QT map.
    MurmurHash3_x86_32(&x, sizeof(x), rand_seed, &hash_val);
    return hash_val;
}

uint32_t DualTrackDWay::setLoss(uint32_t slot_index) const {
    const uint32_t start = (slot_index / d) * d;
    uint32_t loss = 0;
    for (uint32_t i = start; i < start + d; ++i) {
        loss += heavy_table[i].D;
    }
    return loss;
}

void DualTrackDWay::insertFlow(uint32_t slot_index, uint32_t x) {
    auto& slot = heavy_table[slot_index];
    slot.F = x;
    // A returning flow may occupy another way. Its historical losses must
    // therefore be taken from the entire set rather than this slot alone.
    slot.U = setLoss(slot_index);
    slot.C = 1;
    slot.V = 0;
}

void DualTrackDWay::clearFlowRecord(uint32_t slot_index) {
    auto& slot = heavy_table[slot_index];
    slot.D += slot.C + slot.V;
    slot.F = 0;
    slot.U = 0;
    slot.C = 0;
    slot.V = 0;
}

void DualTrackDWay::replaceCell(uint32_t cell_index, uint32_t slot_index, uint32_t y) {
    const uint32_t old_parent = quad_table[cell_index].P;
    quad_table[cell_index] = {y, 1, slot_index};
    if (old_parent == slot_index) {
        return;
    }

    const uint32_t old_hash = hashFlow(heavy_table[old_parent].F);
    const uint32_t start = old_hash % (m2 - k + 1);
    for (uint32_t j = start; j < start + k; ++j) {
        // Empty cells have P = 0, which must not count as children of slot 0.
        if (quad_table[j].E != 0 && quad_table[j].P == old_parent) {
            return;
        }
    }
    clearFlowRecord(old_parent);
}

void DualTrackDWay::update(uint32_t x, uint32_t y) {
    const uint32_t hash_val = hashFlow(x);
    const uint32_t start = (hash_val % set_count) * d;
    uint32_t match = m1;
    uint32_t empty = m1;
    uint32_t victim = start;
    for (uint32_t i = start; i < start + d; ++i) {
        if (heavy_table[i].F == x) {
            match = i;
            break;
        }
        if (heavy_table[i].F == 0) {
            if (empty == m1) {
                empty = i;
            }
        } else if (heavy_table[i].C < heavy_table[victim].C) {
            victim = i;
        }
    }

    // Full-set miss: only the minimum-C slot competes with this arrival.
    // Retain Case 3's policy of dropping this arrival even after eviction.
    if (match == m1 && empty == m1) {
        auto& slot = heavy_table[victim];
        --slot.C;
        ++slot.V;
        if (slot.C == 0) {
            const uint32_t old_hash = hashFlow(slot.F);
            const uint32_t qt_start = old_hash % (m2 - k + 1);
            for (uint32_t j = qt_start; j < qt_start + k; ++j) {
                if (quad_table[j].E != 0 && quad_table[j].P == victim) {
                    quad_table[j] = {};
                }
            }
            clearFlowRecord(victim);
        }
        ++slot.D;
        return;
    }

    const bool stored = match != m1;
    const uint32_t slot_index = stored ? match : empty;
    if (stored) {
        ++heavy_table[slot_index].C;
    }

    const uint32_t qt_start = hash_val % (m2 - k + 1);
    uint32_t empty_cell = m2;
    uint32_t min_cell = qt_start;
    for (uint32_t j = qt_start; j < qt_start + k; ++j) {
        if (stored && quad_table[j].E == y && quad_table[j].P == slot_index) {
            ++quad_table[j].R;
            return;
        }
        if (quad_table[j].E == 0 && empty_cell == m2) {
            if (!stored) {
                // Baseline Case 1 inserts at the first empty QT cell.
                // Only Case 2 must keep scanning for an existing element.
                quad_table[j] = {y, 1, slot_index};
                insertFlow(slot_index, x);
                return;
            }
            empty_cell = j;
        }
        if (quad_table[j].E != 0 && quad_table[j].R < quad_table[min_cell].R) {
            min_cell = j;
        }
    }

    if (empty_cell != m2) {
        if (!stored) {
            insertFlow(slot_index, x);
        }
        quad_table[empty_cell] = {y, 1, slot_index};
        return;
    }

    --quad_table[min_cell].R;
    if (quad_table[min_cell].R > 0) {
        if (!stored) {
            ++heavy_table[slot_index].D;
        }
        return;
    }
    if (!stored) {
        insertFlow(slot_index, x);
    }
    replaceCell(min_cell, slot_index, y);
}

uint32_t DualTrackDWay::estimateFrequency(uint32_t slot_index) const {
    const auto& slot = heavy_table[slot_index];
    const uint32_t lower = slot.C + slot.V;
    const uint32_t upper = lower + slot.U;
    switch (method) {
        case 0:
            return lower;
        case 1:
            return upper;
        case 2:
            return (lower + upper) / 2;
        default:
            return lower + upper == 0 ? 0 :
                static_cast<uint32_t>((2ULL * lower * upper) / (lower + upper));
    }
}

std::map<uint32_t, uint32_t>
DualTrackDWay::queryElements(uint32_t slot_index, uint32_t hash_val) const {
    std::map<uint32_t, uint32_t> elements;
    const uint32_t start = hash_val % (m2 - k + 1);
    for (uint32_t j = start; j < start + k; ++j) {
        if (quad_table[j].E != 0 && quad_table[j].P == slot_index) {
            elements[quad_table[j].E] = quad_table[j].R;
        }
    }
    return elements;
}

std::pair<std::map<uint32_t, uint32_t>,
          std::map<uint32_t, std::map<uint32_t, uint32_t>>>
DualTrackDWay::query(uint32_t heavy_hitter_th) {
    std::map<uint32_t, uint32_t> heavy_hitters;
    std::map<uint32_t, std::map<uint32_t, uint32_t>> quad_elements;
    for (uint32_t i = 0; i < m1; ++i) {
        if (heavy_table[i].F == 0) {
            continue;
        }
        const uint32_t frequency = estimateFrequency(i);
        if (frequency >= heavy_hitter_th) {
            const uint32_t x = heavy_table[i].F;
            heavy_hitters[x] = frequency;
            // Match the baseline's result-map construction for scan timing.
            std::map<uint32_t, uint32_t> current_quad_elements = queryElements(i, hashFlow(x));
            quad_elements[x] = current_quad_elements;
        }
    }
    return {heavy_hitters, quad_elements};
}

std::pair<uint32_t, std::map<uint32_t, uint32_t>>
DualTrackDWay::querySingleFlow(uint32_t flow_id) {
    uint32_t estimated_frequency = 0;
    std::map<uint32_t, uint32_t> quad_elements;
    if (flow_id == 0) {
        return {estimated_frequency, quad_elements};
    }
    const uint32_t hash_val = hashFlow(flow_id);
    const uint32_t start = (hash_val % set_count) * d;
    for (uint32_t i = start; i < start + d; ++i) {
        if (heavy_table[i].F == flow_id) {
            estimated_frequency = estimateFrequency(i);
            quad_elements = queryElements(i, hash_val);
            break;
        }
    }
    return {estimated_frequency, quad_elements};
}

void DualTrackDWay::evaluation(
    const std::vector<std::pair<uint32_t, uint32_t>>& dataset,
    const std::map<uint32_t, uint32_t>& flows,
    std::map<uint32_t, std::map<uint32_t, uint32_t>> quadratic_eles,
    uint32_t heavy_hitter_th, float ele_th_phi) {
    std::cout << "\nDualTrack (d-way HT): (d = " << d << ", k = " << k
              << ", m_ht_frac = " << m_ht_frac << ", method = " << method << ")\n";
    std::cout << " - HT sets: " << set_count << ", HT slots: " << m1
              << ", QT cells: " << m2 << ", hash seed: " << rand_seed << '\n';
    std::cout << "method: {0: lower bound, 1: upper bound, 2: arithmetic mean, 3: harmonic mean}"
              << std::endl;
    std::cout << " - ARE population: true positives (same as DualTrack)"
              << std::endl;

    const auto start_update = std::chrono::high_resolution_clock::now();
    for (const auto& [x, y] : dataset) {
        update(x, y);
    }
    const auto end_update = std::chrono::high_resolution_clock::now();
    const double update_seconds = std::chrono::duration<double>(end_update - start_update).count();
    const double throughput = update_seconds > 0 ? (dataset.size() / 1e6) / update_seconds : 0;
    std::cout << " - Update Throughput: " << throughput << " Mdps" << std::endl;

    const auto start_query = std::chrono::high_resolution_clock::now();
    const auto [queried_heavy_hitters, queried_quad_elements] = query(heavy_hitter_th);
    const auto end_query = std::chrono::high_resolution_clock::now();
    std::cout << " - Full Query Time: "
              << std::chrono::duration<double, std::milli>(end_query - start_query).count()
              << " ms" << std::endl;

    double single_query_total = 0;
    for (const auto& [flow_id, flow_size] : flows) {
        const auto single_start = std::chrono::high_resolution_clock::now();
        const auto result = querySingleFlow(flow_id);
        const auto single_end = std::chrono::high_resolution_clock::now();
        single_query_total +=
            std::chrono::duration<double, std::micro>(single_end - single_start).count();
    }
    std::cout << " - Avg. Single-Flow Query Time: "
              << (flows.empty() ? 0 : single_query_total / flows.size())
              << " us (micro second)" << std::endl;

    std::map<uint32_t, uint32_t> true_heavy_hitters;
    std::map<uint32_t, std::map<uint32_t, uint32_t>> true_hot_quad_elements;
    for (const auto& [flow_id, flow_size] : flows) {
        if (flow_size >= heavy_hitter_th) {
            true_heavy_hitters[flow_id] = flow_size;
            const auto elements = quadratic_eles.find(flow_id);
            if (elements != quadratic_eles.end()) {
                for (const auto& [ele_id, ele_size] : elements->second) {
                    if (ele_size >= ele_th_phi * flow_size) {
                        true_hot_quad_elements[flow_id][ele_id] = ele_size;
                    }
                }
            }
        }
    }

    // Match DualTrack: estimate error is averaged only over true positives.
    float hh_are_sum = 0;
    uint32_t hh_true_positives = 0;
    for (const auto& [id, true_size] : true_heavy_hitters) {
        const auto queried = queried_heavy_hitters.find(id);
        if (queried != queried_heavy_hitters.end()) {
            hh_are_sum += std::abs(static_cast<float>(true_size) - queried->second) / true_size;
            ++hh_true_positives;
        }
    }
    const float hh_are = hh_true_positives > 0 ? hh_are_sum / hh_true_positives : 0;
    const float hh_precision = queried_heavy_hitters.empty() ? 0 :
        static_cast<float>(hh_true_positives) / queried_heavy_hitters.size();
    const float hh_recall = true_heavy_hitters.empty() ? 0 :
        static_cast<float>(hh_true_positives) / true_heavy_hitters.size();
    const float hh_f1 = hh_precision + hh_recall > 0 ?
        2 * hh_precision * hh_recall / (hh_precision + hh_recall) : 0;
    std::cout << " - Heavy Hitter Metrics | ARE: " << hh_are << ", PR: " << hh_precision
              << ", CR: " << hh_recall << ", F1: " << hh_f1 << '\n';

    std::map<uint32_t, std::map<uint32_t, uint32_t>> queried_hot_quad_elements;
    uint32_t total_queried = 0;
    for (const auto& [flow_id, elements] : queried_quad_elements) {
        const uint32_t frequency = queried_heavy_hitters.at(flow_id);
        for (const auto& [ele_id, ele_size] : elements) {
            if (ele_size >= ele_th_phi * frequency) {
                queried_hot_quad_elements[flow_id][ele_id] = ele_size;
                ++total_queried;
            }
        }
    }

    float ele_are_sum = 0;
    uint32_t ele_true_positives = 0;
    uint32_t total_true = 0;
    for (const auto& [flow_id, elements] : true_hot_quad_elements) {
        total_true += elements.size();
        const auto queried_flow = queried_hot_quad_elements.find(flow_id);
        if (queried_flow == queried_hot_quad_elements.end()) {
            continue;
        }
        for (const auto& [ele_id, true_size] : elements) {
            const auto queried = queried_flow->second.find(ele_id);
            if (queried != queried_flow->second.end()) {
                ele_are_sum += std::abs(static_cast<float>(true_size) - queried->second) / true_size;
                ++ele_true_positives;
            }
        }
    }
    const float ele_are = ele_true_positives > 0 ? ele_are_sum / ele_true_positives : 0;
    const float ele_precision = total_queried > 0 ?
        static_cast<float>(ele_true_positives) / total_queried : 0;
    const float ele_recall = total_true > 0 ?
        static_cast<float>(ele_true_positives) / total_true : 0;
    const float ele_f1 = ele_precision + ele_recall > 0 ?
        2 * ele_precision * ele_recall / (ele_precision + ele_recall) : 0;
    std::cout << " - Quadratic Ele Metrics | ARE: " << ele_are << ", PR: " << ele_precision
              << ", CR: " << ele_recall << ", F1: " << ele_f1 << '\n';
}
