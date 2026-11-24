/* -*- P4_16 -*- */
#include <core.p4>
#include <v1model.p4>

#include "include/headers.p4"
#include "include/parsers.p4"

/* CONSTANTS */
#define CELL_SIZE 32
#define M1 1024
#define M2 1024
#define MAX_HASH 1024
#define PKT_INSTANCE_TYPE_RESUBMIT 6
/*************************************************************************
************   C H E C K S U M    V E R I F I C A T I O N   *************
*************************************************************************/

control MyVerifyChecksum(inout headers hdr, inout metadata meta) {
    apply {  }
}

/*************************************************************************
**************  I N G R E S S   P R O C E S S I N G   *******************
*************************************************************************/

control MyIngress(inout headers hdr,
                  inout metadata meta,
                  inout standard_metadata_t standard_metadata) {

    /** Data Structure of DualTrack **/
    // HeavyTable
    register<bit<CELL_SIZE>>(M1) HTF;
    register<bit<CELL_SIZE>>(M1) HTU;
    register<bit<CELL_SIZE>>(M1) HTC;
    register<bit<CELL_SIZE>>(M1) HTV;
    register<bit<CELL_SIZE>>(M1) HTD;

    // QuardTable (k=2)
    register<bit<CELL_SIZE>>(M2) QTE1;
    register<bit<CELL_SIZE>>(M2) QTR1;
    register<bit<CELL_SIZE>>(M2) QTP1;

    register<bit<CELL_SIZE>>(M2) QTE2;
    register<bit<CELL_SIZE>>(M2) QTR2;
    register<bit<CELL_SIZE>>(M2) QTP2;

    /** Hash calculation in DualTrack **/
    action ac_cal_hash1() {
        hash(meta.hash_index1, HashAlgorithm.crc32_custom, (bit<16>) 0, {hdr.ipv4.srcAddr}, (bit<32>) MAX_HASH);
    }

    table tb_cal_hash1 {
        actions = {
            ac_cal_hash1;
        }
        size = 1;
        const default_action = ac_cal_hash1;
    }

    action ac_cal_hash2() {
        hash(meta.hash_index2, HashAlgorithm.crc32_custom, (bit<16>) 0, {hdr.ipv4.srcAddr, hdr.ipv4.dstAddr}, (bit<32>) MAX_HASH);
    }

    table tb_cal_hash2 {
        actions = {
            ac_cal_hash2;
        }
        size = 1;
        const default_action = ac_cal_hash2;
    }

    /**Procedure for update**/

    action ac_read_HTF() {
        HTF.read(meta.mapped_ID, meta.hash_index1);
    }

    table tb_read_HTF {
        actions = {
            ac_read_HTF;
        }
        size = 1;
        const default_action = ac_read_HTF;
    }

    action ac_set_ID_HTF() {
        HTF.write(meta.hash_index1, hdr.ipv4.srcAddr);
    }

    table tb_set_ID_HTF {
        actions = {
            ac_set_ID_HTF;
        }
        size = 1;
        const default_action = ac_set_ID_HTF;
    }

    action ac_clean_HTF() {
        HTF.write(meta.hash_index1, (bit<32>)0);
    }

    table tb_clean_HTF {
        actions = {
            ac_clean_HTF;
        }
        size = 1;
        const default_action = ac_clean_HTF;
    }

    action ac_read_HTD() {
        HTD.read(meta.mapped_HT_D, meta.hash_index1);
    }

    table tb_read_HTD {
        actions = {
            ac_read_HTD;
        }
        size = 1;
        const default_action = ac_read_HTD;
    }

    action ac_set_D_HTU() {
        HTU.write(meta.hash_index1, meta.mapped_HT_D);
    }

    table tb_set_D_HTU {
        actions = {
            ac_set_D_HTU;
        }
        size = 1;
        const default_action = ac_set_D_HTU;
    }

    action ac_clean_HTU() {
        HTU.write(meta.hash_index1, (bit<32>) 0);
    }

    table tb_clean_HTU {
        actions = {
            ac_clean_HTU;
        }
        size = 1;
        const default_action = ac_clean_HTU;
    }

    action ac_add_HTD() {
        meta.mapped_HT_D = meta.mapped_HT_D |+| 1;
        HTD.write(meta.hash_index1, meta.mapped_HT_D);
    }

    table tb_add_HTD {
        actions = {
            ac_add_HTD;
        }
        size = 1;
        const default_action = ac_add_HTD;
    }

    action ac_clean_HTD() {
        meta.mapped_HT_D = meta.mapped_HT_D |+| meta.mapped_HT_V;
        HTD.write(meta.hash_index1, meta.mapped_HT_D);
    }

    table tb_clean_HTD {
        actions = {
            ac_clean_HTD;
        }
        size = 1;
        const default_action = ac_clean_HTD;
    }

    action ac_read_HTC() {
        HTC.read(meta.mapped_HT_C, meta.hash_index1);
    }

    table tb_read_HTC {
        actions = {
            ac_read_HTC;
        }
        size = 1;
        const default_action = ac_read_HTC;
    }

    action ac_set_one_HTC() {
        HTC.write(meta.hash_index1, (bit<32>)1);
    }

    table tb_set_one_HTC {
        actions = {
            ac_set_one_HTC;
        }
        size = 1;
        const default_action = ac_set_one_HTC;
    }

    action ac_add_HTC() {
        meta.mapped_HT_C = meta.mapped_HT_C |+| 1;
        HTC.write(meta.hash_index1, meta.mapped_HT_C);
    }

    table tb_add_HTC {
        actions = {
            ac_add_HTC;
        }
        size = 1;
        const default_action = ac_add_HTC;
    }

    action ac_sub_HTC() {
        meta.mapped_HT_C = meta.mapped_HT_C |-| 1;
        HTC.write(meta.hash_index1, meta.mapped_HT_C);
    }

    table tb_sub_HTC {
        actions = {
            ac_sub_HTC;
        }
        size = 1;
        const default_action = ac_sub_HTC;
    }

    action ac_clean_HTC() {
        HTC.write(meta.hash_index1, (bit<32>)0);
    }

    table tb_clean_HTC {
        actions = {
            ac_clean_HTC;
        }
        size = 1;
        const default_action = ac_clean_HTC;
    }

    action ac_read_HTV() {
        HTV.read(meta.mapped_HT_V, meta.hash_index1);
    }

    table tb_read_HTV {
        actions = {
            ac_read_HTV;
        }
        size = 1;
        const default_action = ac_read_HTV;
    }

    action ac_add_HTV() {
        meta.mapped_HT_V = meta.mapped_HT_V |+| 1;
        HTV.write(meta.hash_index1, meta.mapped_HT_V);
    }

    table tb_add_HTV {
        actions = {
            ac_add_HTV;
        }
        size = 1;
        const default_action = ac_add_HTV;
    }

    action ac_clean_HTV() {
        HTV.write(meta.hash_index1, (bit<32>)0);
    }

    table tb_clean_HTV {
        actions = {
            ac_clean_HTV;
        }
        size = 1;
        const default_action = ac_clean_HTV;
    }

    action ac_read_QTE1() {
        QTE1.read(meta.mapped_E1, meta.hash_index2);
    }

    table tb_read_QTE1 {
        actions = {
            ac_read_QTE1;
        }
        size = 1;
        const default_action = ac_read_QTE1;
    }

    action ac_set_ID_QTE1() {
        QTE1.write(meta.hash_index2, hdr.ipv4.dstAddr);
    }

    table tb_set_ID_QTE1 {
        actions = {
            ac_set_ID_QTE1;
        }
        size = 1;
        const default_action = ac_set_ID_QTE1;
    }

    action ac_clean_QTE1() {
        QTE1.write(meta.replace_index, (bit<32>)0);
    }

    table tb_clean_QTE1 {
        actions = {
            ac_clean_QTE1;
        }
        size = 1;
        const default_action = ac_clean_QTE1;
    }

    action ac_read_QTR1() {
        QTR1.read(meta.rcnt1, meta.hash_index2);
    }

    table tb_read_QTR1 {
        actions = {
            ac_read_QTR1;
        }
        size = 1;
        const default_action = ac_read_QTR1;
    }

    action ac_set_one_QTR1() {
        QTR1.write(meta.hash_index2, (bit<32>) 1);
    }

    table tb_set_one_QTR1 {
        actions = {
            ac_set_one_QTR1;
        }
        size = 1;
        const default_action = ac_set_one_QTR1;
    }

    action ac_sub_QTR1() {
        meta.rcnt1 = meta.rcnt1 |-| 1;
        QTR1.write(meta.hash_index2, meta.rcnt1);
    }

    table tb_sub_QTR1 {
        actions = {
            ac_sub_QTR1;
        }
        size = 1;
        const default_action = ac_sub_QTR1;
    }

    action ac_add_QTR1() {
        meta.rcnt1 = meta.rcnt1 |+| 1;
        QTR1.write(meta.hash_index2, meta.rcnt1);
    }

    table tb_add_QTR1 {
        actions = {
            ac_add_QTR1;
        }
        size = 1;
        const default_action = ac_add_QTR1;
    }

    action ac_clean_QTR1() {
        QTR1.write(meta.replace_index, (bit<32>) 0);
    }

    table tb_clean_QTR1 {
        actions = {
            ac_clean_QTR1;
        }
        size = 1;
        const default_action = ac_clean_QTR1;
    }

    action ac_set_ID_QTP1() {
        QTP1.write(meta.hash_index2, hdr.ipv4.srcAddr);
    }

    table tb_set_ID_QTP1 {
        actions = {
            ac_set_ID_QTP1;
        }
        size = 1;
        const default_action = ac_set_ID_QTP1;
    }

    action ac_clean_QTP1() {
        QTP1.write(meta.replace_index, (bit<32>)0);
    }

    table tb_clean_QTP1 {
        actions = {
            ac_clean_QTP1;
        }
        size = 1;
        const default_action = ac_clean_QTP1;
    }

    action ac_read_QTE2() {
        QTE2.read(meta.mapped_E2, meta.hash_index2);
    }

    table tb_read_QTE2 {
        actions = {
            ac_read_QTE2;
        }
        size = 1;
        const default_action = ac_read_QTE2;
    }

    action ac_set_ID_QTE2() {
        QTE2.write(meta.hash_index2, hdr.ipv4.dstAddr);
    }

    table tb_set_ID_QTE2 {
        actions = {
            ac_set_ID_QTE2;
        }
        size = 1;
        const default_action = ac_set_ID_QTE2;
    }

    action ac_clean_QTE2() {
        QTE2.write(meta.replace_index, (bit<32>)0);
    }

    table tb_clean_QTE2 {
        actions = {
            ac_clean_QTE2;
        }
        size = 1;
        const default_action = ac_clean_QTE2;
    }

    action ac_read_QTR2() {
        QTR2.read(meta.rcnt2, meta.hash_index2);
    }

    table tb_read_QTR2 {
        actions = {
            ac_read_QTR2;
        }
        size = 1;
        const default_action = ac_read_QTR2;
    }

    action ac_set_one_QTR2() {
        QTR2.write(meta.hash_index2, (bit<32>) 1);
    }

    table tb_set_one_QTR2 {
        actions = {
            ac_set_one_QTR2;
        }
        size = 1;
        const default_action = ac_set_one_QTR2;
    }

    action ac_sub_QTR2() {
        meta.rcnt2 = meta.rcnt2 |-| 1;
        QTR2.write(meta.hash_index2, meta.rcnt2);
    }

    table tb_sub_QTR2 {
        actions = {
            ac_sub_QTR2;
        }
        size = 1;
        const default_action = ac_sub_QTR2;
    }

    action ac_add_QTR2() {
        meta.rcnt2 = meta.rcnt2 |+| 1;
        QTR2.write(meta.hash_index2, meta.rcnt2);
    }

    table tb_add_QTR2 {
        actions = {
            ac_add_QTR2;
        }
        size = 1;
        const default_action = ac_add_QTR2;
    }

    action ac_clean_QTR2() {
        QTR2.write(meta.replace_index, (bit<32>)0);
    }

    table tb_clean_QTR2 {
        actions = {
            ac_clean_QTR2;
        }
        size = 1;
        const default_action = ac_clean_QTR2;
    }

    action ac_set_ID_QTP2() {
        QTP2.write(meta.hash_index2, hdr.ipv4.srcAddr);
    }

    table tb_set_ID_QTP2 {
        actions = {
            ac_set_ID_QTP2;
        }
        size = 1;
        const default_action = ac_set_ID_QTP2;
    }

    action ac_clean_QTP2() {
        QTP2.write(meta.replace_index, (bit<32>)0);
    }

    table tb_clean_QTP2 {
        actions = {
            ac_clean_QTP2;
        }
        size = 1;
        const default_action = ac_clean_QTP2;
    }

    action drop() {
        mark_to_drop(standard_metadata);
    }

    action set_egress_port(bit<9> egress_port){
        standard_metadata.egress_spec = egress_port;
    }

    table forwarding {
        key = {
            standard_metadata.ingress_port: exact;
        }
        actions = {
            set_egress_port;
            drop;
            NoAction;
        }
        size = 64;
        default_action = drop;
    }

    apply {
        if (hdr.ipv4.isValid()){
            if (standard_metadata.instance_type == PKT_INSTANCE_TYPE_RESUBMIT) {
                if (hdr.myheader.min_row == 1) {
                    QTP1.read(hdr.myheader.replace_ID, hdr.myheader.min_index);
                    QTE1.write(hdr.myheader.min_index, hdr.ipv4.dstAddr);
                    QTR1.write(hdr.myheader.min_index, (bit<32>)1);
                    QTP1.write(hdr.myheader.min_index, hdr.ipv4.srcAddr);
                } else {
                    QTP2.read(hdr.myheader.replace_ID, hdr.myheader.min_index);
                    QTE2.write(hdr.myheader.min_index, hdr.ipv4.dstAddr);
                    QTR2.write(hdr.myheader.min_index, (bit<32>)1);
                    QTP2.write(hdr.myheader.min_index, hdr.ipv4.srcAddr);
                }
                if (hdr.myheader.replace_ID != hdr.ipv4.srcAddr) {
                    hash(meta.replace_index, HashAlgorithm.crc32_custom, (bit<16>) 0, {hdr.myheader.replace_ID}, (bit<32>) MAX_HASH);
                    QTP1.read(hdr.myheader.E1P, meta.replace_index);
                    QTP2.read(hdr.myheader.E2P, meta.replace_index);
                    if (hdr.myheader.E1P != hdr.myheader.replace_ID) {
                        if (hdr.myheader.E2P != hdr.myheader.replace_ID) {
                            hash(hdr.myheader.recover_index, HashAlgorithm.crc32_custom, (bit<16>) 0, {hdr.myheader.replace_ID}, (bit<32>) MAX_HASH);
                            HTF.write(hdr.myheader.recover_index, (bit<32>)0);
                            HTU.write(hdr.myheader.recover_index, (bit<32>)0);
                            HTC.read(hdr.myheader.recover_C, hdr.myheader.recover_index);
                            HTV.read(hdr.myheader.recover_V, hdr.myheader.recover_index);
                            HTD.read(hdr.myheader.recover_D, hdr.myheader.recover_index);
                            hdr.myheader.recover_C = hdr.myheader.recover_C |+| hdr.myheader.recover_V;
                            hdr.myheader.recover_D = hdr.myheader.recover_C |+| hdr.myheader.recover_D;
                            HTD.write(hdr.myheader.recover_index, hdr.myheader.recover_D);
                            HTC.write(hdr.myheader.recover_index, (bit<32>)0);
                            HTV.write(hdr.myheader.recover_index, (bit<32>)0);
                        }
                    }
                }
            } else {
                tb_cal_hash1.apply();
                tb_cal_hash2.apply();
                tb_read_HTF.apply();
                tb_read_QTE1.apply();
                tb_read_QTE2.apply();
                tb_read_QTR1.apply();
                tb_read_QTR2.apply();
                tb_read_HTD.apply();
                tb_read_HTC.apply();
                tb_read_HTV.apply();
                if (meta.mapped_ID == 0) {
                    // case 1
                    if (meta.mapped_E1 == 0 || meta.mapped_E2 == 0) {
                        tb_set_ID_HTF.apply();
                        tb_set_D_HTU.apply();
                        tb_set_one_HTC.apply();
                        if (meta.mapped_E1 == 0) {
                            tb_set_ID_QTE1.apply();
                            tb_set_one_QTR1.apply();
                            tb_set_ID_QTP1.apply();
                        } else {
                            tb_set_ID_QTE2.apply();
                            tb_set_one_QTR2.apply();
                            tb_set_ID_QTP2.apply();
                        }
                    } else {
                        bit<8> update_HT;
                        if (meta.rcnt1 < meta.rcnt2) {
                            hdr.myheader.min_row = 1;
                            hdr.myheader.min_index = meta.hash_index2;
                            tb_sub_QTR1.apply();
                            if (meta.rcnt1 > 0) {
                                update_HT = (bit<8>) 1;
                            }
                        } else {
                            hdr.myheader.min_row = 2;
                            hdr.myheader.min_index = meta.hash_index2;
                            tb_sub_QTR2.apply();
                            if (meta.rcnt2 > 0) {
                                update_HT = (bit<8>) 1;
                            }
                        }
                        if (update_HT == 1) {
                            tb_add_HTD.apply();
                        } else {
                            HTF.write(meta.hash_index1, hdr.ipv4.srcAddr);
                            HTU.write(meta.hash_index1, meta.mapped_HT_D);
                            HTC.write(meta.hash_index1, (bit<32>)1);
                            HTV.write(meta.hash_index1, (bit<32>)0);
                            resubmit({hdr.myheader.min_index, hdr.myheader.min_row});
                        }
                    }
                } else {
                    if (meta.mapped_ID == hdr.ipv4.srcAddr) {
                        // case 2
                        bit<8> update_flag_case2;
                        tb_add_HTC.apply();
                        if (meta.mapped_E1 == hdr.ipv4.dstAddr) {
                            tb_add_QTR1.apply();
                            update_flag_case2 = (bit<8>)1;
                        }
                        if (meta.mapped_E2 == hdr.ipv4.dstAddr) {
                            tb_add_QTR2.apply();
                            update_flag_case2 = (bit<8>)1;
                        }
                        if (update_flag_case2 == 0) {
                            /*if (meta.mapped_E1 == 0) {
                                tb_set_ID_QTE1.apply();
                                tb_set_one_QTR1.apply();
                                tb_set_ID_QTP1.apply();
                                update_flag_case2 = (bit<8>)1;
                            } else {
                                tb_set_ID_QTE2.apply();
                                tb_set_one_QTR2.apply();
                                tb_set_ID_QTP2.apply();
                                update_flag_case2 = (bit<8>)1;
                            }*/
                            if (meta.mapped_E1 == 0) {
                                QTE1.write(meta.hash_index2, hdr.ipv4.dstAddr);
                                QTR1.write(meta.hash_index2, (bit<32>) 1);
                                QTP1.write(meta.hash_index2, hdr.ipv4.srcAddr);
                                update_flag_case2 = (bit<8>)1;
                            } else {
                                if (meta.mapped_E2 == 0) {
                                    QTE2.write(meta.hash_index2, hdr.ipv4.dstAddr);
                                    QTR2.write(meta.hash_index2, (bit<32>) 1);
                                    QTP2.write(meta.hash_index2, hdr.ipv4.srcAddr);
                                    update_flag_case2 = (bit<8>)1;
                                }
                            }
                        }
                        if (update_flag_case2 == 0) {
                            if (meta.rcnt1 < meta.rcnt2) {
                                //tb_sub_QTR1.apply();
                                meta.rcnt1 = meta.rcnt1 |-| 1;
                                QTR1.write(meta.hash_index2, meta.rcnt1);
                                if (meta.rcnt1 > 0) {
                                    update_flag_case2 = (bit<8>) 1;
                                }
                            } else {
                                //tb_sub_QTR2.apply();
                                meta.rcnt2 = meta.rcnt2 |-| 1;
                                QTR2.write(meta.hash_index2, meta.rcnt2);
                                if (meta.rcnt2 > 0) {
                                    update_flag_case2 = (bit<8>) 1;
                                }
                            }
                        }
                        if (update_flag_case2 == 0) {
                            // resubmit
                            resubmit({hdr.myheader.min_index, hdr.myheader.min_row});
                        }
                    } else {
                        // case 3
                        tb_sub_HTC.apply();
                        tb_add_HTV.apply();
                        if (meta.mapped_HT_C > 0) {
                            meta.mapped_HT_D = meta.mapped_HT_D |+| 1;
                            HTD.write(meta.hash_index1, meta.mapped_HT_D);
                        } else {
                            tb_clean_HTF.apply();
                            tb_clean_HTU.apply();
                            tb_clean_HTC.apply();
                            tb_clean_HTD.apply();
                            tb_clean_HTV.apply();
                            hash(meta.replace_index, HashAlgorithm.crc32_custom, (bit<16>) 0, {meta.mapped_ID}, (bit<32>) MAX_HASH);
                            QTP1.read(meta.replace_E1_P, meta.replace_index);
                            QTP2.read(meta.replace_E2_P, meta.replace_index);
                            if (meta.replace_E1_P == meta.mapped_ID) {
                                tb_clean_QTE1.apply();
                                tb_clean_QTP1.apply();
                                tb_clean_QTR1.apply();
                            }

                            if (meta.replace_E2_P == meta.mapped_ID) {
                                tb_clean_QTE2.apply();
                                tb_clean_QTP2.apply();
                                tb_clean_QTR2.apply();
                            }

                        }
                    }
                }
            }
            forwarding.apply();
        }
    }
}

/*************************************************************************
****************  E G R E S S   P R O C E S S I N G   *******************
*************************************************************************/

control MyEgress(inout headers hdr,
                 inout metadata meta,
                 inout standard_metadata_t standard_metadata) {
    apply {

    }
}

/*************************************************************************
*************   C H E C K S U M    C O M P U T A T I O N   **************
*************************************************************************/

control MyComputeChecksum(inout headers hdr, inout metadata meta) {
     apply {
    }
}

/*************************************************************************
***********************  S W I T C H  *******************************
*************************************************************************/

//switch architecture
V1Switch(
MyParser(),
MyVerifyChecksum(),
MyIngress(),
MyEgress(),
MyComputeChecksum(),
MyDeparser()
) main;
