#pragma once
#include <pch.h>
#include "FeatureRegistrar.h"
class Memory final : public FeatureRegistrar<Memory> {
public:
    Memory();

    auto render() -> void override;
    auto update() -> void override;
};
