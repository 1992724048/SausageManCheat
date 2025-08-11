#pragma once
#include <pch.h>

#include "FeatureRegistrar.h"

class Performance final : public FeatureRegistrar<Performance> {
public:
    Performance();

    auto render() -> void override;
    auto update() -> void override;
};
