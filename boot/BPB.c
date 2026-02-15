#include "BPB.h"

__attribute__((section(".oem_id"), used, aligned(1)))
const char OEM_ID[8] = "DAVIDAKK";

__attribute__((section(".bpb"), used, aligned(1)))
const _BPB BPB;

__attribute__((section(".ebpb"), used, aligned(1)))
const _EBPB EBPB;