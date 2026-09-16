#pragma once

#include "ComponentVisualizer.h"

/** Viewport relationships for the selected reveal component or its owner. */
class FBertaBlackEyeRevealComponentVisualizer : public FComponentVisualizer
{
public:
    virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View,
        FPrimitiveDrawInterface* PDI) override;
    virtual void DrawVisualizationHUD(const UActorComponent* Component, const FViewport* Viewport,
        const FSceneView* View, FCanvas* Canvas) override;
};
