#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "header/DualTrack.h"
#include "header/DualTrackDWay.h"
#include "header/DualTrackNoLoss.h"
#include "header/CocoSketch.h"
#include "header/HyperUSS.h"
#include "header/DUET_CU.h"
#include "header/GlobalHH.h"
#include "header/TwoDMisraGries.h"

/**
 * @brief Loads the CAIDA dataset from files.
 * @return A tuple containing three collections:
 * 1. std::vector<std::pair<uint32_t, uint32_t>>: The raw dataset of (source_ip, dest_ip) pairs.
 * 2. std::map<uint32_t, uint32_t>: The true count for each flow (source IP).
 * 3. std::map<uint32_t, std::map<uint32_t, uint32_t>>: The true count for each element (dest IP)
 * within each flow (source IP).
 */
std::tuple<std::vector<std::pair<uint32_t, uint32_t>>,
        std::map<uint32_t, uint32_t>,
        std::map<uint32_t, std::map<uint32_t, uint32_t>>> loadDataSetCAIDA() {

    // Please change the following paths to your dataset files.
    std::vector<std::string> file_paths = {
        "../../dataset/CAIDA_demo.txt",
    };

    std::vector<std::pair<uint32_t, uint32_t>> data;
    std::map<uint32_t, uint32_t> true_flow_size;
    std::map<uint32_t, std::map<uint32_t, uint32_t>> true_element_size;
    uint64_t total_data_num = 0;

    for (const auto &file_path: file_paths) {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << file_path << std::endl;
            continue;
        }

        std::string line;
        while (getline(file, line)) {
            std::istringstream iss(line);
            std::string source_ip, dest_ip;
            iss >> source_ip >> dest_ip;
            if (source_ip.empty() || dest_ip.empty()) continue;

            uint32_t src_ip_int = 0, dst_ip_int = 0;
            int part = 0;
            char dot;
            std::istringstream ss1(source_ip), ss2(dest_ip);
            bool valid = true;

            for (int i = 0; i < 4; i++) {
                if (!(ss1 >> part)) {
                    valid = false;
                    break;
                }
                if (part < 0 || part > 255) {
                    valid = false;
                    break;
                }
                src_ip_int = (src_ip_int << 8) | part;
                if (i < 3 && !(ss1 >> dot && dot == '.')) {
                    valid = false;
                    break;
                }
            }
            if (ss1 >> dot) valid = false;

            for (int i = 0; i < 4 && valid; i++) {
                if (!(ss2 >> part)) {
                    valid = false;
                    break;
                }
                if (part < 0 || part > 255) {
                    valid = false;
                    break;
                }
                dst_ip_int = (dst_ip_int << 8) | part;
                if (i < 3 && !(ss2 >> dot && dot == '.')) {
                    valid = false;
                    break;
                }
            }
            if (ss2 >> dot) valid = false;

            if (!valid) continue;

            if (src_ip_int == 0 || dst_ip_int == 0) continue;

            data.emplace_back(src_ip_int, dst_ip_int);
            true_flow_size[src_ip_int]++;
            true_element_size[src_ip_int][dst_ip_int]++;

            total_data_num++;
        }

        std::cout << file_path << " is loaded." << std::endl;
    }

    std::cout << file_paths.size() << " files loaded.\n"
              << "Totaling data items: " << total_data_num
              << ", Unique flows (src_ip as flow ID): " << true_flow_size.size() << std::endl;

    return std::make_tuple(data, true_flow_size, true_element_size);
}

std::tuple<std::vector<std::pair<uint32_t, uint32_t>>,
        std::map<uint32_t, uint32_t>,
        std::map<uint32_t, std::map<uint32_t, uint32_t>>> loadDatasetFreqItemMining() {

    // Please change the following paths to your dataset files.
    std::vector<std::string> file_paths = {
        "../../dataset/kosarak.dat",
    };

    std::vector<std::pair<uint32_t, uint32_t>> data;
    std::map<uint32_t, uint32_t> flow_true_count;
    std::map<uint32_t, std::map<uint32_t, uint32_t>> flow_element_true_count;

    uint64_t total_data_num = 0;

    for (const auto &file_path: file_paths) {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << file_path << std::endl;
            continue;
        }

        std::string line;
        uint32_t line_count = 0;

        while (getline(file, line)) {
            line_count++;
            if (line.empty() || line.find_first_not_of(" \t\n\r") == std::string::npos) {
                continue;
            }

            std::istringstream iss(line);
            std::vector<uint32_t> numbers;
            uint32_t number;

            while (iss >> number) {
                numbers.push_back(number);
            }

            // A pair requires two IDs; never fabricate one or read past the row.
            if (numbers.size() < 2) {
                continue;
            }

            try {
                uint32_t flow_id = numbers[0] + 1;
                uint32_t ele_id = numbers[1] + 1;

                if (flow_id == 0 || ele_id == 0) continue;

                data.emplace_back(flow_id, ele_id);
                flow_true_count[flow_id]++;
                flow_element_true_count[flow_id][ele_id]++;

                total_data_num++;
            } catch (const std::exception &e) {
                std::cerr << "Skipping invalid line " << line_count << " in " << file_path << ": " << line << " ("
                          << e.what() << ")" << std::endl;
                continue;
            }
        }
    }

    std::cout << "Frequent Itemset Mining Dataset loaded, totaling data: " << total_data_num << std::endl;
    std::cout << "Unique flows: " << flow_true_count.size() << std::endl;

    return std::make_tuple(data, flow_true_count, flow_element_true_count);
}

int main() {

    std::cout << "DualTrack experiment starts ..." << std::endl;
    auto start_time = std::chrono::steady_clock::now();

    auto [dataset, flows, quadratic_eles] = loadDataSetCAIDA();
//    auto [dataset, flows, quadratic_eles] = loadDatasetFreqItemMining();

    if (dataset.empty()) {
        std::cerr << "Experiment aborted: no valid data was loaded. "
                     "Check dataset paths and file contents." << std::endl;
        return 1;
    }

    std::vector<uint32_t> memo_kb_values = {100}; // memory in KB
    std::vector<float> heavy_hitter_th_values = {0.0001}; // phi_1
    std::vector<float> quad_ele_th_values = {0.1}; // phi_2

    // Enable the supplementary experiments as needed.
    constexpr bool run_no_loss_experiment = false;
    constexpr bool run_dway_experiment = false;

    for (uint32_t memo_kb: memo_kb_values) {
        for (float hh_th_ratio: heavy_hitter_th_values) {
            for (float ele_th_phi: quad_ele_th_values) {
                uint32_t heavy_hitter_th = hh_th_ratio * dataset.size();

                std::cout << "\nHeavy hitter th = " << heavy_hitter_th
                          << " (N*phi), phi_1 = " << hh_th_ratio
                          << ", Quad element th (phi_2) = " << ele_th_phi
                          << ", memo_kb = " << memo_kb
                          << std::endl;

                auto *dualTrack = new DualTrack(memo_kb);
                const uint32_t seed = dualTrack->getHashSeed();
                dualTrack->evaluation(dataset, flows, quadratic_eles, heavy_hitter_th, ele_th_phi);
                delete dualTrack;

                auto* duet_cu = new DUET_CU(memo_kb);
                duet_cu->evaluation(dataset, flows, quadratic_eles, heavy_hitter_th, ele_th_phi);
                delete duet_cu;

                auto* global_hh = new GlobalHH(memo_kb);
                global_hh->evaluation(dataset, flows, quadratic_eles, heavy_hitter_th, ele_th_phi);
                delete global_hh;

                auto* twod_mg = new TwoDMisraGries(memo_kb);
                twod_mg->evaluation(dataset, flows, quadratic_eles, heavy_hitter_th, ele_th_phi);
                delete twod_mg;

                auto* coco = new CocoSketch(memo_kb);
                coco->evaluation(dataset, flows, quadratic_eles, heavy_hitter_th, ele_th_phi);
                delete coco;

                auto* hyperUSS = new HyperUSS(memo_kb);
                hyperUSS->evaluation(dataset, flows, quadratic_eles, heavy_hitter_th, ele_th_phi);
                delete hyperUSS;

                if (run_no_loss_experiment) {
                    DualTrackNoLoss no_loss(memo_kb, seed);
                    no_loss.evaluation(dataset, flows, quadratic_eles,
                                       heavy_hitter_th, ele_th_phi);
                }

                if (run_dway_experiment) {
                    for (uint32_t d : {2u, 3u, 4u, 5u}) {
                        DualTrackDWay dual_variant(memo_kb, d, seed);
                        dual_variant.evaluation(dataset, flows, quadratic_eles,
                                                heavy_hitter_th, ele_th_phi);
                    }
                }

            }
        }
    }

    auto end_time = std::chrono::steady_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
    std::cout << "\nExperiment ends. Elapsed time: "
              << elapsed_time / 3600 << "h " << (elapsed_time % 3600) / 60 << "m " << elapsed_time % 60 << "s"
              << std::endl;

    return 0;
}
