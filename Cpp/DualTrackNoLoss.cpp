#include "header/DualTrackNoLoss.h"
#include "header/MurmurHash3.h"
#include "header/utils.h"

#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

DualTrackNoLoss::DualTrackNoLoss(float memory_kb)
    : DualTrackNoLoss(memory_kb, 32, 0.6f) {}

DualTrackNoLoss::DualTrackNoLoss(float memory_kb, uint32_t seed)
    : DualTrackNoLoss(memory_kb, 32, 0.6f, seed) {}

DualTrackNoLoss::DualTrackNoLoss(float memory_kb, uint32_t k_val, float m_frac)
    : DualTrackNoLoss(memory_kb, k_val, m_frac, generateSeeds32(1)[0]) {}

DualTrackNoLoss::DualTrackNoLoss(float memory_kb, uint32_t k_val, float m_frac,
                                 uint32_t seed)
    : k(k_val), m_ht_frac(m_frac), rand_seed(seed) {
    if (!std::isfinite(memory_kb) || memory_kb <= 0 ||
        !std::isfinite(m_frac) || m_frac <= 0 || m_frac >= 1 || k == 0) {
        throw std::invalid_argument("DualTrackNoLoss: invalid memory, HT fraction, or k");
    }

    // Reuse DualTrack's logical bit-budget and rounding conventions. Removing
    // U/V/D changes HT records from 160 to 64 bits; the HT fraction is fixed.
    const float memory_ht = memory_kb * m_ht_frac;
    const float slot_count = std::round(memory_ht * 1024 * 8 / 64);
    if (slot_count < 1 || static_cast<double>(slot_count) >
                          std::numeric_limits<uint32_t>::max()) {
        throw std::invalid_argument("DualTrackNoLoss: HT capacity is out of range");
    }
    m1 = static_cast<uint32_t>(slot_count);

    // More HT slots can require a wider parent index. Recompute QT capacity
    // instead of silently exceeding the baseline's logical memory model.
    const uint32_t qt_cell_bits = 64 +
        static_cast<uint32_t>(std::ceil(std::log2(m1)));
    const float memory_qt = memory_kb - memory_ht;
    const float cell_count = std::round(memory_qt * 1024 * 8 / qt_cell_bits);
    if (cell_count < k || static_cast<double>(cell_count) >
                         std::numeric_limits<uint32_t>::max()) {
        throw std::invalid_argument("DualTrackNoLoss: QT cannot hold k cells");
    }
    m2 = static_cast<uint32_t>(cell_count);
    heavy_table.resize(m1);
    quad_table.resize(m2);
}

uint32_t DualTrackNoLoss::hashFlow(uint32_t x) const {
    uint32_t hash_val = 0;
    MurmurHash3_x86_32(&x, sizeof(x), rand_seed, &hash_val);
    return hash_val;
}

void DualTrackNoLoss::replaceCell(uint32_t cell_index, uint32_t slot_index, uint32_t y) {
    const uint32_t old_parent = quad_table[cell_index].P;
    quad_table[cell_index] = {y, 1, slot_index};
    if (old_parent == slot_index) {
        return;
    }

    const uint32_t start = hashFlow(heavy_table[old_parent].F) % (m2 - k + 1);
    for (uint32_t j = start; j < start + k; ++j) {
        // Empty cells with P=0 are not children of HT slot 0.
        if (quad_table[j].E != 0 && quad_table[j].P == old_parent) {
            return;
        }
    }
    heavy_table[old_parent] = {};
}

void DualTrackNoLoss::update(uint32_t x, uint32_t y) {
    const uint32_t hash_val = hashFlow(x);
    const uint32_t i = hash_val % m1;
    auto& slot = heavy_table[i];

    // DualTrack Case 3: decay the resident flow and drop this arrival, even
    // when decay evicts the resident. Only loss-field updates are removed.
    if (slot.F != 0 && slot.F != x) {
        --slot.C;
        if (slot.C == 0) {
            const uint32_t start = hashFlow(slot.F) % (m2 - k + 1);
            for (uint32_t j = start; j < start + k; ++j) {
                if (quad_table[j].E != 0 && quad_table[j].P == i) {
                    quad_table[j] = {};
                }
            }
            slot = {};
        }
        return;
    }

    const bool stored = slot.F != 0;
    if (stored) {
        ++slot.C;
    }
    const uint32_t start = hash_val % (m2 - k + 1);
    uint32_t empty_cell = m2;
    uint32_t min_cell = m2;
    uint32_t min_r = std::numeric_limits<uint32_t>::max();
    for (uint32_t j = start; j < start + k; ++j) {
        // DualTrack Case 2 searches for a matching element before filling
        // an empty cell, whereas Case 1 stops at the first empty cell.
        if (stored && quad_table[j].E == y && quad_table[j].P == i) {
            ++quad_table[j].R;
            return;
        }
        if (quad_table[j].E == 0) {
            if (!stored) {
                quad_table[j] = {y, 1, i};
                slot = {x, 1};
                return;
            }
            if (empty_cell == m2) {
                empty_cell = j;
            }
        } else if (quad_table[j].R < min_r) {
            min_r = quad_table[j].R;
            min_cell = j;
        }
    }

    if (empty_cell != m2) {
        quad_table[empty_cell] = {y, 1, i};
        return;
    }

    --quad_table[min_cell].R;
    if (quad_table[min_cell].R > 0) {
        return;
    }
    if (!stored) {
        slot = {x, 1};
    }
    replaceCell(min_cell, i, y);
}

std::map<uint32_t, uint32_t>
DualTrackNoLoss::queryElements(uint32_t slot_index, uint32_t hash_val) const {
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
DualTrackNoLoss::query(uint32_t heavy_hitter_th) {
    std::map<uint32_t, uint32_t> heavy_hitters;
    std::map<uint32_t, std::map<uint32_t, uint32_t>> quad_elements;
    for (uint32_t i = 0; i < m1; ++i) {
        const auto& slot = heavy_table[i];
        if (slot.F != 0 && slot.C >= heavy_hitter_th) {
            heavy_hitters[slot.F] = slot.C;
            // Retain the result-map copy used by DualTrack.
            std::map<uint32_t, uint32_t> current = queryElements(i, hashFlow(slot.F));
            quad_elements[slot.F] = current;
        }
    }
    return {heavy_hitters, quad_elements};
}

std::pair<uint32_t, std::map<uint32_t, uint32_t>>
DualTrackNoLoss::querySingleFlow(uint32_t flow_id) {
    uint32_t estimated_frequency = 0;
    std::map<uint32_t, uint32_t> quad_elements;
    if (flow_id == 0) {
        return {estimated_frequency, quad_elements};
    }
    const uint32_t hash_val = hashFlow(flow_id);
    const uint32_t i = hash_val % m1;
    if (heavy_table[i].F == flow_id) {
        estimated_frequency = heavy_table[i].C;
        quad_elements = queryElements(i, hash_val);
    }
    // Match DualTrack's local-map result construction for query timing.
    return {estimated_frequency, quad_elements};
}

void DualTrackNoLoss::evaluation(
    const std::vector<std::pair<uint32_t, uint32_t>>& dataset,
    const std::map<uint32_t, uint32_t>& flows,
    std::map<uint32_t, std::map<uint32_t, uint32_t>> quadratic_eles,
    uint32_t heavy_hitter_th, float ele_th_phi) {
    std::cout << "\nDualTrack (without loss tracking): (k = " << k
              << ", m_ht_frac = " << m_ht_frac << ")\n";
    std::cout << " - Frequency loss tracking: disabled (U, V, D removed)\n"
              << " - Frequency estimate: C (residual count)\n"
              << " - HT slots: " << m1 << ", QT cells: " << m2
              << ", hash seed: " << rand_seed << '\n'
              << " - Logical storage: 64 bits/HT slot, "
              << 64 + static_cast<uint32_t>(std::ceil(std::log2(m1)))
              << " bits/QT cell (same budget model as DualTrack)\n"
              << " - ARE population: true positives (same as DualTrack)" << std::endl;

    const auto start_update = std::chrono::high_resolution_clock::now();
    for (const auto& [x, y] : dataset) {
        update(x, y);
    }
    const auto end_update = std::chrono::high_resolution_clock::now();
    const double seconds = std::chrono::duration<double>(end_update - start_update).count();
    std::cout << " - Update Throughput: "
              << (seconds > 0 ? (dataset.size() / 1e6) / seconds : 0)
              << " Mdps" << std::endl;

    const auto start_query = std::chrono::high_resolution_clock::now();
    const auto [queried_heavy_hitters, queried_quad_elements] = query(heavy_hitter_th);
    const auto end_query = std::chrono::high_resolution_clock::now();
    std::cout << " - Full Query Time: "
              << std::chrono::duration<double, std::milli>(end_query - start_query).count()
              << " ms" << std::endl;

    // Match DualTrack: query all ground-truth flows, including absent records.
    std::vector<double> single_query_times;
    single_query_times.reserve(flows.size());
    for (const auto& [flow_id, flow_size] : flows) {
        const auto start = std::chrono::high_resolution_clock::now();
        auto [estimated_frequency, quad_map] = querySingleFlow(flow_id);
        const auto end = std::chrono::high_resolution_clock::now();
        single_query_times.push_back(std::chrono::duration<double, std::micro>(end - start).count());
    }
    double total_query_time = 0;
    for (double duration : single_query_times) {
        total_query_time += duration;
    }
    std::cout << " - Avg. Single-Flow Query Time: "
              << (flows.empty() ? 0 : total_query_time / flows.size())
              << " us (micro second)" << std::endl;

    std::map<uint32_t, uint32_t> true_heavy_hitters;
    std::map<uint32_t, std::map<uint32_t, uint32_t>> true_hot_elements;
    for (const auto& [flow_id, flow_size] : flows) {
        if (flow_size >= heavy_hitter_th) {
            true_heavy_hitters[flow_id] = flow_size;
            const auto elements = quadratic_eles.find(flow_id);
            if (elements != quadratic_eles.end()) {
                for (const auto& [ele_id, ele_size] : elements->second) {
                    if (ele_size >= ele_th_phi * flow_size) {
                        true_hot_elements[flow_id][ele_id] = ele_size;
                    }
                }
            }
        }
    }

    float hh_are_sum = 0;
    uint32_t hh_tp = 0;
    for (const auto& [id, true_size] : true_heavy_hitters) {
        const auto found = queried_heavy_hitters.find(id);
        if (found != queried_heavy_hitters.end()) {
            hh_are_sum += std::abs(static_cast<float>(true_size) - found->second) / true_size;
            ++hh_tp;
        }
    }
    const float hh_are = hh_tp > 0 ? hh_are_sum / hh_tp : 0;
    const float hh_pr = queried_heavy_hitters.empty() ? 0 :
        static_cast<float>(hh_tp) / queried_heavy_hitters.size();
    const float hh_cr = true_heavy_hitters.empty() ? 0 :
        static_cast<float>(hh_tp) / true_heavy_hitters.size();
    const float hh_f1 = hh_pr + hh_cr > 0 ? 2 * hh_pr * hh_cr / (hh_pr + hh_cr) : 0;
    std::cout << " - Heavy Hitter Metrics | ARE: " << hh_are << ", PR: " << hh_pr
              << ", CR: " << hh_cr << ", F1: " << hh_f1 << '\n';
    std::cout << " - Heavy Hitter Counts | TP: " << hh_tp
              << ", Reported: " << queried_heavy_hitters.size()
              << ", True: " << true_heavy_hitters.size() << '\n';

    std::map<uint32_t, std::map<uint32_t, uint32_t>> queried_hot_elements;
    uint32_t total_queried = 0;
    for (const auto& [flow_id, elements] : queried_quad_elements) {
        const uint32_t frequency = queried_heavy_hitters.at(flow_id);
        for (const auto& [ele_id, ele_size] : elements) {
            if (ele_size >= ele_th_phi * frequency) {
                queried_hot_elements[flow_id][ele_id] = ele_size;
                ++total_queried;
            }
        }
    }

    float ele_are_sum = 0;
    uint32_t ele_tp = 0;
    uint32_t total_true = 0;
    for (const auto& [flow_id, elements] : true_hot_elements) {
        total_true += elements.size();
        const auto flow = queried_hot_elements.find(flow_id);
        if (flow == queried_hot_elements.end()) {
            continue;
        }
        for (const auto& [ele_id, true_size] : elements) {
            const auto found = flow->second.find(ele_id);
            if (found != flow->second.end()) {
                ele_are_sum += std::abs(static_cast<float>(true_size) - found->second) / true_size;
                ++ele_tp;
            }
        }
    }
    const float ele_are = ele_tp > 0 ? ele_are_sum / ele_tp : 0;
    const float ele_pr = total_queried > 0 ? static_cast<float>(ele_tp) / total_queried : 0;
    const float ele_cr = total_true > 0 ? static_cast<float>(ele_tp) / total_true : 0;
    const float ele_f1 = ele_pr + ele_cr > 0 ? 2 * ele_pr * ele_cr / (ele_pr + ele_cr) : 0;
    std::cout << " - Quadratic Ele Metrics | ARE: " << ele_are << ", PR: " << ele_pr
              << ", CR: " << ele_cr << ", F1: " << ele_f1 << '\n';
    std::cout << " - Quadratic Ele Counts | TP: " << ele_tp
              << ", Reported: " << total_queried << ", True: " << total_true << '\n';
}
