#include "./murmurhash.h"
#include <stdlib.h>

#define M1 1024
#define M2 4096
#define K 32

#define seed1 12312
#define seed2 65123

/** metadata of HT **/
uint32_t FIDs[M1] = {0};
int Us[M1] = {0};
int Cs[M1] = {0};
int Vs[M1] = {0};
int Ds[M1] = {0};

/** metadata of QT **/
uint32_t EIDs[M2] = {0};
int Rs[M2] = {0};
int Ps[M2] = {0};

/** replace QT cells and reset HT bucket**/
void replaceCell(int idx, uint32_t x, uint32_t y) {
    uint32_t hash_value, hash_index;
    uint32_t replaced_x = Ps[idx];
    EIDs[idx] = y; Rs[idx] = 1; Ps[idx] = x;
    if (replaced_x != x) {
        MurmurHash3_x86_32(&replaced_x, seed2, &hash_value);
        hash_index = hash_value % (M2 - K + 1);
        int exists_cell = 0;
        for (int i = hash_index; i < hash_index + K; ++i) {
            if (Ps[i] == replaced_x) {
                exists_cell += 1;
            }
        }
        if (exists_cell == 0) {
            MurmurHash3_x86_32(&replaced_x, seed1, &hash_value);
            hash_index = hash_value % M1;
            FIDs[hash_index] = 0;
            Us[hash_index] = 0;
            Ds[hash_index] = Ds[hash_index] + Cs[hash_index] + Vs[hash_index];
            Cs[hash_index] = 0;
            Vs[hash_index] = 0;
        }
    }
}

/** update HT bucket and QT cells **/
void update(uint32_t x, uint32_t y) {
    uint32_t hash_value1, hash_index1;
    uint32_t hash_value2, hash_index2;
    MurmurHash3_x86_32(&x, seed1, &hash_value1);
    hash_index1 = hash_value1 % M1;
    MurmurHash3_x86_32(&x, seed2, &hash_value2);
    hash_index2 = hash_value2 % (M2 - K + 1);
    if (FIDs[hash_index1] == 0) {
        int empty_idx = -1, min_idx = -1, min_val = 0x7fffffff;
        for (int i = hash_index2; i < hash_index2 + K; ++i) {
            if (EIDs[i] == 0) {
                empty_idx = i;
            } else {
                if (Rs[i] < min_val) {
                    min_idx = i;
                    min_val = Rs[i];
                }
            }
        }
        if (empty_idx != -1) {
            EIDs[empty_idx] = y; Rs[empty_idx] = 1; Ps[empty_idx] = x;
            FIDs[hash_index1] = x;
            Us[hash_index1] = Ds[hash_index1];
            Cs[hash_index1] = 1; Vs[hash_index1] = 0;
            return;
        } else {
            Rs[min_idx] -= 1;
            if (Rs[min_idx] > 0) {
                Ds[hash_index1] += 1;
            } else {
                FIDs[hash_index1] = x;
                Us[hash_index1] = Ds[hash_index1];
                Cs[hash_index1] = 1; Vs[hash_index1] = 0;
                replaceCell(min_idx, x, y);
            }
        }
    } else if (FIDs[hash_index1] == x) {
        Cs[hash_index1] += 1;
        int empty_idx = -1, min_idx = -1, min_val = 0x7fffffff;
        for (int i = hash_index2; i < hash_index2 + K; ++i) {
            if (EIDs[i] == 0) {
                empty_idx = i;
            } else if (EIDs[i] == y) {
                if (Ps[i] == x) {
                    Rs[i] += 1;
                    return;
                } else {
                    if (Rs[i] < min_val) {
                        min_idx = i;
                        min_val = Rs[i];
                    }
                }
            } else {
                if (Rs[i] < min_val) {
                    min_idx = i;
                    min_val = Rs[i];
                }
            }
        }
        if (empty_idx != -1) {
            EIDs[empty_idx] = y; Rs[empty_idx] = 1; Ps[empty_idx] = x;
        } else {
            Rs[min_idx] -= 1;
            if (Rs[min_idx] == 0) {
                replaceCell(min_idx, x, y);
            }
        }
    } else {
        Cs[hash_index1] -= 1;
        Vs[hash_index1] += 1;
        if (Cs[hash_index1] > 0) {
            Ds[hash_index1] += 1;
            return;
        }
        uint32_t replaced_x = FIDs[hash_index1];
        FIDs[hash_index1] = 0;
        Us[hash_index1] = 0; Cs[hash_index1] = 0;
        Ds[hash_index1] = Ds[hash_index1] + Vs[hash_index1];
        Vs[hash_index1] = 0;
        uint32_t new_hash_index, new_hash_value;
        MurmurHash3_x86_32(&replaced_x, seed2, &new_hash_value);
        new_hash_index = new_hash_value % (M2 - K + 1);
        for (int i = new_hash_index; i < new_hash_index + K; ++i) {
            if (Ps[i] == replaced_x) {
                EIDs[i] = 0; Rs[i] = 0; Ps[i] = 0;
            }
        }
        Ds[hash_index1] += 1;
    }
}
