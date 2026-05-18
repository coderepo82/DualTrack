
#include "header/CocoSketch.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <chrono>

CocoSketch::CocoSketch(float memory_kb){

    d = 2; // as the setting in CocoSketch original paper

    size_t totalBits = static_cast<size_t>(memory_kb) * 1024 * 8; // KB to bits

    size_t bucketSize = 64 + 32;

    l = totalBits / (d * bucketSize);

    table.resize(d, std::vector<Bucket>(l));

    rand_seeds = generateSeeds32(d); // random seed for each bucket array
}



CocoSketch::~CocoSketch() {
}


void CocoSketch::update(uint64_t key_, uint32_t weight) {
    int min_d = -1;
    uint32_t min_val = 0xFFFFFFFF;
    int tie_count = 0;

    int first_empty_d = -1;

    // the hashed bucket index for each array
    // e.g., indices[0] is the hashed index in the first array for "key_"
    std::vector<uint32_t> indices(d);

    for (int i = 0; i < d; ++i) {

        uint32_t hash_val = 0;
        MurmurHash3_x86_32(&key_, sizeof(key_), rand_seeds[i], &hash_val);
        uint32_t idx = hash_val % l; // bkt index

        indices[i] = idx;
        Bucket& cur_bucket = table[i][idx];

        if (cur_bucket.key == key_) {
            cur_bucket.value += weight;
            return;
        }

        if (first_empty_d == -1 && cur_bucket.key == 0) {
            first_empty_d = i;
        }

        if (first_empty_d == -1) {
            if (cur_bucket.value < min_val) {
                min_val = cur_bucket.value;
                min_d = i;
                tie_count = 1;
            } else if (cur_bucket.value == min_val) { // multiple minimum found
                tie_count++;
                std::uniform_int_distribution<int> tie_dis(0, tie_count - 1);
                if (tie_dis(gen) == 0) {
                    min_d = i;
                }
            }
        }
    }


    if (first_empty_d != -1) {
        Bucket& target = table[first_empty_d][indices[first_empty_d]];
        target.key = key_;
        target.value = weight;
        return;
    }


    Bucket& target = table[min_d][indices[min_d]];
    target.value += weight;

    double prob = static_cast<double>(weight) / target.value;

    if (dis(gen) < prob) {
        target.key = key_;
    }
}





/**
 * @brief retrieve heavy hitters and their heavy quadratic elements.
 * @return A pair of maps.
 * 1st map stores heavy hitters: flow ID (uint32_t) -> estimated frequency (uint32_t).
 * 2nd map stores heavy quadratic elements for each heavy hitter:
 * The key is the heavy hitter's flow ID (uint32_t), and the value is another map.
 * The inner map stores element ID (uint32_t) -> frequency (uint32_t).
 */
std::pair<std::map<uint32_t, uint32_t>, std::map<uint32_t, std::map<uint32_t, uint32_t>>> CocoSketch::getHHAndHotQuadEle(uint32_t heavy_hitter_th, float phi) {

    // flow ID x -> {element y1: count1, element y2: count2, ...}
    std::map<uint32_t, std::map<uint32_t, uint32_t>> flow_elements;
    // flow ID -> frequency
    std::map<uint32_t, uint32_t> flow_sizes;

    // get aggressive data from CocoSketch
    for (int i = 0; i < d; ++i) {
        for (int j = 0; j < l; ++j) {
            if (table[i][j].key > 0) {
                uint64_t combined_xy = table[i][j].key;
                uint32_t val = table[i][j].value;

                // split full key and get partial Keys
                auto [x, y] = split_xy(combined_xy);

                // aggregate the count value
                flow_elements[x][y] += val;
                flow_sizes[x] += val;
            }
        }
    }

    std::map<uint32_t, uint32_t> heavy_hitters;
    std::map<uint32_t, std::map<uint32_t, uint32_t>> hot_quad_elements;


    for (auto const& [x, total_size] : flow_sizes) {
        // check Heavy Hitter
        if (total_size >= heavy_hitter_th) {
            heavy_hitters[x] = total_size;

            // check each element whether is a Heavy Quadratic element
            const auto& elements = flow_elements[x];
            uint32_t quad_threshold = static_cast<uint32_t>(total_size * phi);

            for (auto const& [y, y_size] : elements) {
                if (y_size >= quad_threshold) {
                    hot_quad_elements[x][y] = y_size;
                }
            }
        }
    }

    return {heavy_hitters, hot_quad_elements};
}



/**
 * @brief Queries the Sketch for a single specified flow.
 * @param flow_id The ID of the flow to query (uint32_t).
 * @return A pair of values:
 * 1st: estimated frequency of the specified flow (uint32_t).
 * 2nd: map of elements for this flow (element id -> element frequency).
 */
std::pair<uint32_t, std::map<uint32_t, uint32_t>> CocoSketch::querySingleFlow(uint32_t flow_id) {

    uint32_t estimated_frequency = 0;
    std::map<uint32_t, uint32_t> quad_elements;

    // get aggressive data from CocoSketch
    for (int i = 0; i < d; ++i) {
        for (int j = 0; j < l; ++j) {
            if (table[i][j].key > 0) {
                uint64_t combined_xy = table[i][j].key;
                uint32_t val = table[i][j].value;
                // split full key and get partial Keys
                auto [x, y] = split_xy(combined_xy);
                if (x == flow_id){
                    estimated_frequency += val;
                    quad_elements[y] += val;
                }
            }
        }
    }

    return {estimated_frequency, quad_elements};
}



void CocoSketch::evaluation(const std::vector<std::pair<uint32_t, uint32_t>> &dataset,
                            const std::map<uint32_t, uint32_t> &flows,
                            std::map<uint32_t, std::map<uint32_t, uint32_t>> quadratic_eles,
                            uint32_t heavy_hitter_th, float ele_th_phi) {

    std::cout << "\n" << "CocoSketch: " << std::endl;

    // Process the entire dataset
    auto start_update = std::chrono::high_resolution_clock::now();
    for (const auto &[x, y]: dataset) {
        uint64_t combined_xy = combine_xy(x, y);
        update(combined_xy, 1);
    }
    auto end_update = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> update_duration = end_update - start_update;
    double update_throughput_Mdps = (dataset.size() / 1e6) / update_duration.count();
    std::cout << " - Update Throughput: " << update_throughput_Mdps << " Mdps" << std::endl;


    auto start_query = std::chrono::high_resolution_clock::now();
    auto [queried_heavy_hitters, queried_quad_elements] = getHHAndHotQuadEle(heavy_hitter_th, ele_th_phi);
    auto end_query = std::chrono::high_resolution_clock::now();
    auto query_duration = end_query - start_query;
    std::cout << " - Full Query Time: " << std::chrono::duration<double, std::milli>(query_duration).count() << " ms" << std::endl;


    std::vector<double> single_query_times;
    single_query_times.reserve(flows.size());
    for (const auto &[flow_id, flow_size]: flows) {
        auto single_start = std::chrono::high_resolution_clock::now();
        auto [estimated_freq, quad_map] = querySingleFlow(flow_id);
        auto single_end = std::chrono::high_resolution_clock::now();
        auto duration_us = std::chrono::duration<double, std::micro>(single_end - single_start).count();
        single_query_times.push_back(duration_us);
    }
    double avg_query_time = 0.0;
    if (!single_query_times.empty()) {
        double total_time = std::accumulate(single_query_times.begin(),
                                             single_query_times.end(), 0.0);
        avg_query_time = total_time / single_query_times.size();
    }
    std::cout << " - Avg. Single-Flow Query Time: " << avg_query_time << " us (micro second)" << std::endl;


    std::map<uint32_t, uint32_t> true_heavy_hitters;
    std::map<uint32_t, std::map<uint32_t, uint32_t>> true_hot_quad_elements;
    for (const auto &[flow_id, flow_size]: flows) {
        if (flow_size >= heavy_hitter_th) {
            true_heavy_hitters[flow_id] = flow_size;
            if (quadratic_eles.count(flow_id)) {
                for (const auto &[ele_id, ele_size]: quadratic_eles.at(flow_id)) {
                    if (ele_size >= ele_th_phi * flow_size)
                        true_hot_quad_elements[flow_id][ele_id] = ele_size;
                }
            }
        }
    }

    // --- Heavy Hitter Evaluation ---

    float hh_are_sum = 0.0f;
    uint32_t hh_true_positives = 0;
    for (const auto &[id, true_size]: true_heavy_hitters) {
        if (queried_heavy_hitters.count(id)) {
            uint32_t queried_size = queried_heavy_hitters.at(id);
            hh_are_sum += std::abs(static_cast<float>(true_size) - queried_size) / true_size;
            hh_true_positives++;
        }
    }

    float hh_are = (hh_true_positives > 0) ? hh_are_sum / hh_true_positives : 0.0f;
    float hh_precision = (queried_heavy_hitters.size() > 0) ? static_cast<float>(hh_true_positives) /
                                                              queried_heavy_hitters.size() : 0.0f;
    float hh_recall = (true_heavy_hitters.size() > 0) ? static_cast<float>(hh_true_positives) /
                                                        true_heavy_hitters.size() : 0.0f;
    float hh_f1 = (hh_precision + hh_recall > 0) ? 2 * (hh_precision * hh_recall) / (hh_precision + hh_recall) : 0.0f;

    std::cout << " - Heavy Hitter Metrics | ";
    std::cout << "ARE: " << hh_are << ", ";
    std::cout << "F1: " << hh_f1 << "\n";


    // --- Quadratic Element Evaluation ---
    float ele_are_sum = 0.0f;
    uint32_t ele_true_positives = 0;
    uint32_t total_queried_hot_ele_count = 0;
    uint32_t total_true_hot_ele_count = 0;
    for (const auto &[flow_id, queried_elements]: queried_quad_elements) {
        total_queried_hot_ele_count += queried_elements.size();
    }

    for (const auto &[flow_id, true_hot_elements]: true_hot_quad_elements) {
        if (queried_quad_elements.count(flow_id)) {
            const auto &queried_elements = queried_quad_elements.at(flow_id);
            for (const auto &[ele_id, true_size]: true_hot_elements) {
                if (queried_elements.count(ele_id)) {
                    uint32_t queried_size = queried_elements.at(ele_id);
                    ele_are_sum += std::abs(static_cast<float>(true_size) - queried_size) / true_size;
                    ele_true_positives++;
                }
            }
        }
        total_true_hot_ele_count += true_hot_elements.size();
    }

    float ele_are = (ele_true_positives > 0) ? ele_are_sum / ele_true_positives : 0.0f;
    float ele_precision = (total_queried_hot_ele_count > 0) ? static_cast<float>(ele_true_positives) /
                                                              total_queried_hot_ele_count : 0.0f;
    float ele_recall = (total_true_hot_ele_count > 0) ? static_cast<float>(ele_true_positives) / total_true_hot_ele_count
                                                      : 0.0f;
    float ele_f1 = (ele_precision + ele_recall > 0) ? 2 * (ele_precision * ele_recall) / (ele_precision + ele_recall)
                                                    : 0.0f;

    std::cout << " - Heavy Quadratic Element Metrics | ";
    std::cout << "ARE: " << ele_are << ", ";
    std::cout << "F1: " << ele_f1 << "\n";

}
