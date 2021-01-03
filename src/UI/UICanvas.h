#pragma once

#include "CoreMinimal.h"

namespace k3d {
class Font;
class UIDevice;
class UILayout;
class UITextLayout;

// Record UI Elements with LayerIds
// 2D Rasterizer
// Event-driven
// Event -> Layout (Multithread/SingleThread) -> Render
class UICanvas {
public:
    virtual ~UICanvas() { }

    /* Images, Texts, Boxes, Curves (Beizier, Spline), Lines, Circles, Triangles */

    virtual void clear() { }
    virtual void setAntialias(bool enable) { }

    virtual void drawTextRun(String16 const& inText, math::TVector<int, 2> const& position, Font const& font) { }
    virtual void drawImage(Image const& image, math::TVector<int, 2> const& position, float scale) { }
    virtual void drawRect(math::TVector<int, 4> const& positionAndSize) { }
    virtual void drawPoints() { }
    virtual void drawLine() { }

    virtual void save() { }
    virtual void translate(float x, float y) { }
    virtual void restore() { }

    virtual void flush() { }

    virtual const UIDevice* getDevice() const { return nullptr; }
    virtual bool supportGPU() const { return false; }
};

}