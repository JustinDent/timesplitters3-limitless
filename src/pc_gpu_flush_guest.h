/* The host renderer already owns submitted work. The runtime's NV2A ack
 * worker therefore clears PFB's flush bit without waiting for GPU completion
 * (xbox_memory_layout.c, NV2A_ACK). Do the same acknowledgement at the US
 * D3D poll, avoiding a busy wait for that worker's next scheduled pass.
 * This is not a fence/DMA acknowledgement or a replacement for submission. */
#ifndef TSFP_PC_GPU_FLUSH_GUEST_H
#define TSFP_PC_GPU_FLUSH_GUEST_H
#include <stdint.h>
#ifdef _MSC_VER
#include <intrin.h>
#endif
uint32_t pc_gpu_flush_poll(uint32_t address, uint32_t observed)
{
    const uint32_t busy = 0x10000u;
    if (address != 0xfd100410u || !(observed & busy)) return observed;
    uint32_t *reg = (uint32_t *)((uintptr_t)address + g_xbox_mem_offset);
    /* Preserve unrelated register bits, including changes since the caller's
     * read. Both compiler adapters implement the same atomic AND operation. */
#ifdef _MSC_VER
    uint32_t before = (uint32_t)_InterlockedAnd((volatile long *)reg, (long)~busy);
#else
    uint32_t before = __atomic_fetch_and(reg, ~busy, __ATOMIC_SEQ_CST);
#endif
    return before & ~busy;
}
#endif
