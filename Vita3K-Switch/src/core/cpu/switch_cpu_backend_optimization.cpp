#include "switch_cpu_backend.h"
#include <cstdio>

// ARM-to-ARM optimization control methods

bool SwitchCPUBackend::enableOptimization(bool enable) {
    if (enable && !translator) {
        printf("[SwitchCPUBackend] Error: Cannot enable optimization without a translator\n");
        return false;
    }
    
    optimization_enabled = enable;
    
    if (enable) {
        printf("[SwitchCPUBackend] ARM-to-ARM optimization enabled\n");
        
        // Reset translation statistics
        if (translator) {
            translator->flushCache();
        }
    } else {
        printf("[SwitchCPUBackend] ARM-to-ARM optimization disabled\n");
    }
    
    return true;
}

bool SwitchCPUBackend::isOptimizationEnabled() const {
    return optimization_enabled;
}

void SwitchCPUBackend::printTranslationStats() const {
    if (!translator) {
        printf("[SwitchCPUBackend] No translator available\n");
        return;
    }
    
    printf("[SwitchCPUBackend] ARM-to-ARM Translation Statistics:\n");
    printf("  Optimization enabled: %s\n", optimization_enabled ? "Yes" : "No");
    printf("  Total translated blocks: %llu\n", translator->getTotalTranslatedBlocks());
    printf("  Total executed blocks: %llu\n", translator->getTotalExecutedBlocks());
    printf("  Total translation time: %llu µs\n", translator->getTotalTranslationTime());
    printf("  Total execution time: %llu µs\n", translator->getTotalExecutionTime());
    
    if (translator->getTotalExecutedBlocks() > 0) {
        double avg_time = static_cast<double>(translator->getTotalExecutionTime()) / 
                          static_cast<double>(translator->getTotalExecutedBlocks());
        printf("  Average execution time per block: %.2f µs\n", avg_time);
    }
}
