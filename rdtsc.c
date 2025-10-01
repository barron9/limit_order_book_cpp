//measure TSC cycles on a single core - CPU PINNING & TSC Cycles.

#include <stdint.h>
#include <stdio.h>
#include <intrin.h>
#include <windows.h>

uint64_t read_tsc() {
    return __rdtsc();
}
double cycles_to_us(uint64_t cycles, double cpu_ghz) {
    return (double)cycles / (cpu_ghz * 1e3);
}
int main() {
    double cpu_ghz = 2.8; 

    // Pin thread to core 0
    DWORD_PTR mask = 1;  // CPU 0
    HANDLE thread = GetCurrentThread();
    if (!SetThreadAffinityMask(thread, mask)) {
        printf("Failed to set thread affinity\n");
        return 1;
    }

    uint64_t start = read_tsc();
    for (volatile int i = 0; i < 1000000; ++i);
    uint64_t end = read_tsc();
    printf("TSC cycles: %llu\n", end - start);
        uint64_t elapsed_cycles = end - start;

    double elapsed_us = cycles_to_us(elapsed_cycles, cpu_ghz);

    printf("Elapsed cycles: %llu\n", elapsed_cycles);
    printf("Elapsed time: %.3f microseconds\n", elapsed_us);
    return 0;
}
