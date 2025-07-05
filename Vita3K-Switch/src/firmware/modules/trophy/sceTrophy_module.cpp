#include "sceTrophy.h"
#include "../../../core/module/module_registry.h"

// Register the trophy module with the module registry
REGISTER_MODULE("SceTrophy", firmware::modules::trophy::sceTrophyInit, firmware::modules::trophy::sceTrophyTerm, {
    // Register functions
    REG_FUNC(SceTrophy, sceTrophyCreateContext);
    REG_FUNC(SceTrophy, sceTrophyDestroyContext);
    REG_FUNC(SceTrophy, sceTrophyCreateHandle);
    REG_FUNC(SceTrophy, sceTrophyDestroyHandle);
    REG_FUNC(SceTrophy, sceTrophyGetTrophyInfo);
    REG_FUNC(SceTrophy, sceTrophyGetTrophyGroupInfo);
    REG_FUNC(SceTrophy, sceTrophyGetTrophyCount);
    REG_FUNC(SceTrophy, sceTrophyGetTrophyUnlockState);
    REG_FUNC(SceTrophy, sceTrophyUnlockTrophy);
    REG_FUNC(SceTrophy, sceTrophyGetTrophyIcon);
    REG_FUNC(SceTrophy, sceTrophyGetTrophyUnlockState2);
});
