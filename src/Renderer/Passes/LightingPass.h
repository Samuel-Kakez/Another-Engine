#pragma once
#include "Renderer/Passes/IRenderPass.h"

class LightingPass : public IRenderPass
{
public:
    LightingPass() = default;
    ~LightingPass() override = default;

    void Execute(RenderData &data) override;

    // La passe d'éclairage est toujours active
    bool isEnabled(const RenderSettings &) const override
    {
        return true;
    }
};