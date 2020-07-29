#pragma once

#include "vecmath.h"
#include "Bitmap.h"

struct iCanvas2D
{
    iCanvas2D() {}
    virtual ~iCanvas2D() {}

    virtual void SetPixel(int x, int y, int color) = 0;
    
    virtual void Line(int x1, int y1, int x2, int y2, int color) = 0;

    virtual void Clear(int color) = 0;

    virtual int GetWidth()  const = 0;
    virtual int GetHeight() const = 0;

    virtual void LineW(float x1, float y1, float x2, float y2, int color)
    {
        this->Line(XToScreen(x1), YToScreen(y1), XToScreen(x2), YToScreen(y2), color);
    }

    int XToScreen(float x) const { return (int)(GetWidth () / 2 + x * XScale + XOfs); }
    int YToScreen(float y) const { return (int)(GetHeight() / 2 - y * YScale + YOfs); }

    float ScreenToX(int x) const { return  ((float)(x - GetWidth () / 2) - XOfs) / XScale; }
    float ScreenToY(int y) const { return -((float)(y - GetHeight() / 2) - YOfs) / YScale; }

    // scaling
    float XScale, YScale;
    float XOfs, YOfs;
};

struct Canvas3D
{
    Canvas3D(iCanvas2D* C): FCanvas(C) {}

    virtual void Arrow3D(const vec3& p1, const vec3& p2, float size, int lineColor, int tipColor);
    virtual void Frame3D(const vec3& base, const mtx4& mtx, float size, int Xcolor, int Ycolor, int Zcolor);
    virtual void Plane(const vec3& p, const vec3& v1, const vec3& v2, float step1, float step2, int numx, int numy, int color);

    void Pt3D(const vec3& pt, float sz, int color);

    virtual void SetMatrices(const mtx4& Proj, const mtx4& View)
    {
        FView = View;
        FProj = Proj;
    }

    virtual void Line3D(const vec3& p1, const vec3& p2, int color);

    iCanvas2D* FCanvas;

    mtx4 FProj, FView;
};

/// Adapter of the Bitmap class for the Canvas2D interface (used in offscreen rendering). Redirects calls to Bitmap methods. By default the XScale/YScale are 1.0
struct Canvas2D_Bitmap: public iCanvas2D
{
    Canvas2D_Bitmap(Bitmap* bmp): FDest(bmp) {}
    virtual ~Canvas2D_Bitmap() { delete FDest; }

    virtual void SetPixel(int x, int y, int color);

    virtual void Line(int x1, int y1, int x2, int y2, int color);

    virtual void Clear(int color);

    virtual int GetWidth()  const;
    virtual int GetHeight() const;

    // target for this canvas
    Bitmap* FDest;
};

/// Simple camera positioner for 3D rendering
struct PanOrbitPositioner
{
    PanOrbitPositioner() {
        MiddleButton = false;
        AltKey       = false;
        FOrbiting = false;
        FPanning  = false;

        FZoomOut = 0;
        FZoomIn  = 0;

        FMouseDelta = vec3(0,0,0);
        FPanDelta = vec3(0,0,0);
        FOrbitDelta = vec3(0,0,0);

        FLastMouse = vec3(0,0,0);
        FMouse = vec3(0,0,0);

        FMinDistance = 0.1f;

        FVelocityMultiplier = 4.0f;

        FOrbitingVelocity = 2.0f * 5.25f;
        FPanningVelocity = 0.08f;
        FZoomVelocity = 5.25f;

        FMinPanningVelocity = 0.08f;
        FMinZoomingVelocity = 0.50f;

        FUpVector.x = 0;
        FUpVector.y = 0;
        FUpVector.z = 1;
    }

    /// Read mouse, keyboard, joysticks and call MakeStep to update the state
    virtual void Update( float dt );

    /// Calculate initial transform for a given Up/Target/ViewPos triple
    virtual void Reset();

    void SetMouse(float x, float y);

    mtx4 FCurrentTransform;

    bool MiddleButton;
    bool AltKey;

#pragma region Instant state (set externally by some actual input controller)

    int FWheelTicks;

    /// Are we orbiting/panning
    bool   FOrbiting, FPanning;

    /// Amount of zoom-in/zoom-out
    float  FZoomOut, FZoomIn;

    /// Last mouse change
    vec3   FMouseDelta;

    /// Last panning/orbiting value (might be taken from joystick axis)
    vec3   FPanDelta, FOrbitDelta;

#pragma endregion

#pragma region Movement properties

    // One global multiplier for tweaking
    float  FVelocityMultiplier;

    float  FOrbitingVelocity; // 0.25f;
    float  FPanningVelocity;  // 0.08f;
    float  FZoomVelocity;     // 0.25f;
    float  FMinPanningVelocity; // 0.08f;
    float  FMinZoomingVelocity; // 0.50f;

#pragma endregion

#pragma region Viewing parameters

    /// Viewer position
    vec3  FViewerPosition;

    /// The Up vector
    vec3  FUpVector;

    /// The target we are looking at
    vec3  FTarget;

    /// Minimum possible distance to the object
    float FMinDistance;

#pragma endregion

public:
#pragma region Calculated parameters

    /// Spherical coordinates of the viewer in target-centered frame
    vec3  FSphericalCoords;

    /// (Target - ViewerPos)
    vec3  FView;

    /// Length of FView
    float FViewDistance;
#pragma endregion

    /// Internal update
    void MakeStep( float dt );

    /// Current/Previous mouse positions
    vec3 FLastMouse, FMouse;
};
