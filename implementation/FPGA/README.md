# FPGA Implementation

This directory contains the FPGA implementation of DualTrack.

## Platform

The implementation targets an XA Artix-7 XA7A100T device in the FGG484 package. Synthesis results are obtained using HLS 2022.1.

## Description

The design consists of three functional modules: hash index calculation, flow lookup and update in the HeavyTable (HT), and element lookup and update in the QuadTable (QT). A top-level module coordinates their operation and manages data input and output.

The C++ source can be synthesized into Verilog using the Xilinx HLS toolchain. We also provide our synthesized Verilog code in the "verilog" directory.  

## Synthesis Results

For a configuration with 1024 HT buckets and 4096 QT cells, HLS synthesis reports:

| Resource   | Usage | Utilization |
| ---------- | -----:| -----------:|
| BRAM_18K   | 32    | 11%         |
| Flip-flops | 3602  | 2%          |
| LUTs       | 5248  | 8%          |

The estimated maximum clock frequency is 141.58 MHz.
