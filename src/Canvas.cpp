#include "Canvas.h"
#include "Bitmap.h"
#include <algorithm>

void Canvas3D::Frame3D(const vec3& base, const mtx4& mtx, float size, int Xcolor, int Ycolor, int Zcolor)
{
    vec3 xa, ya, za;

    vec3 pos;
    mtx4 m;
    decompose_camera_transform( mtx, pos, m );

    xa = vec3(MTX4_ELT(m, 0, 0), MTX4_ELT(m, 0, 1), MTX4_ELT(m, 0, 2));
    ya = vec3(MTX4_ELT(m, 1, 0), MTX4_ELT(m, 1, 1), MTX4_ELT(m, 1, 2));
    za = vec3(MTX4_ELT(m, 2, 0), MTX4_ELT(m, 2, 1), MTX4_ELT(m, 2, 2));

    Arrow3D(base + size * xa, base, 0.2f * size, Xcolor, Xcolor);
    Arrow3D(base + size * ya, base, 0.2f * size, Ycolor, Ycolor);
    Arrow3D(base + size * za, base, 0.2f * size, Zcolor, Zcolor);
}

void Canvas3D::Arrow3D(const vec3& Point1,
                        const vec3& Point2,
                        float TipSize,
                        int ArrowColor,
                        int TipColor )
{
    Line3D( Point1, Point2, ArrowColor );

    // build coordsys for tip
    vec3 Arrow = Point2 - Point1;
    vec3 Up, Left;

    BuildComplementaryBasis(Arrow, Up, Left);

    Arrow.Normalize();
    Up.Normalize();
    Left.Normalize();

    Arrow = TipSize * Arrow;
    Up    = 0.5f * TipSize * Up;
    Left  = 0.5f * TipSize * Left;

    vec3 Pt1 = Point1 + Arrow + Left;
    vec3 Pt2 = Point1 + Arrow + Up;
    vec3 Pt3 = Point1 + Arrow - Left;
    vec3 Pt4 = Point1 + Arrow - Up;

    Line3D( Pt1, Point1, TipColor );
    Line3D( Pt2, Point1, TipColor );
    Line3D( Pt3, Point1, TipColor );
    Line3D( Pt4, Point1, TipColor );

    Line3D( Pt1, Pt2, TipColor );
    Line3D( Pt2, Pt3, TipColor );
    Line3D( Pt3, Pt4, TipColor );
    Line3D( Pt4, Pt1, TipColor );
}

void Canvas3D::Plane(const vec3& p, const vec3& v1, const vec3& v2, float step1, float step2, int numx, int numy, int color)
{
    for(int i = 0 ; i <= numx ; i++)
        Line3D(p + ((float)(i - numx/2) * step1) * v1 + (numy/2 ) * step2 * v2, p + ((float)(i - numx/2) * step1) * v1 - (numy/2) * step2 * v2, color);

    for(int j = 0 ; j <= numy ; j++)
        Line3D(p + ((float)(j - numy/2) * step2) * v2 + (numx/2 ) * step1 * v1, p + ((float)(j - numy/2) * step2) * v2 - (numx/2) * step1 * v1, color);
}

void Canvas3D::Pt3D(const vec3& pt, float sz, int color)
{
    vec3 p1x = pt - vec3(sz, 0, 0);
    vec3 p2x = pt + vec3(sz, 0, 0);

    vec3 p1y = pt - vec3(0, sz, 0);
    vec3 p2y = pt + vec3(0, sz, 0);

    vec3 p1z = pt - vec3(0, 0, sz);
    vec3 p2z = pt + vec3(0, 0, sz);

    Line3D(p1x, p2x, color);
    Line3D(p1y, p2y, color);
    Line3D(p1z, p2z, color);
}

void canvas_ndc_to_fb(vec3& V, int _w2, int _h2)
{
    V.x = (float)((int)((V.x + 1)*_w2));
    V.y = (float)((int)((V.y + 1)*_h2));

    V.z = (V.z + 1.0f) / 2;
}

void Canvas3D::Line3D(const vec3& v1, const vec3& v2, int color)
{
    mtx4 m = FView * FProj;
    vec3 p1, p2;

    mult_mtx_vec(p1, m, v1);
    mult_mtx_vec(p2, m, v2);

    int w = FCanvas->GetWidth();
    int h = FCanvas->GetHeight();

    canvas_ndc_to_fb(p1, (w-1)/2, (h-1)/2);
    canvas_ndc_to_fb(p2, (w-1)/2, (h-1)/2);

    FCanvas->Line((int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, color);
}

void Canvas2D_Bitmap::SetPixel(int x, int y, int color)
{
    FDest->SetPixel(x, y, color);
}

void Canvas2D_Bitmap::Line(int x1, int y1, int x2, int y2, int color)
{
    FDest->Line(x1, y1, x2, y2, color);
}

void Canvas2D_Bitmap::Clear(int color)
{
    FDest->Clear(color);
}

int Canvas2D_Bitmap::GetWidth() const { return FDest->Width; }
int Canvas2D_Bitmap::GetHeight() const { return FDest->Height; }

//// Camera positioning

void PanOrbitPositioner::SetMouse(float x, float y)
{
    FLastMouse = FMouse;

    FMouse.x = x;
    FMouse.y = y;
    FMouse.z = 0;

    FMouseDelta = FMouseDelta + FMouse - FLastMouse;
}

void PanOrbitPositioner::Update( float dt )
{
    FZoomOut = FZoomIn = 0.0f;

    FOrbiting = AltKey && MiddleButton;
    FPanning  = !AltKey && MiddleButton;

    if ( FWheelTicks != 0 )
    {
        FZoomIn  = std::max( ( ( float )FWheelTicks ) * 0.1f, 0.0f );
        FZoomOut = std::min( ( ( float )FWheelTicks ) * 0.1f, 0.0f );
        FWheelTicks = 0;
    }

    MakeStep( dt );
}

void PanOrbitPositioner::Reset()
{
    MiddleButton = false;
    AltKey = false;

    FWheelTicks = 0;

    FView = FTarget - FViewerPosition;
    FViewDistance = FView.Length();

    FSphericalCoords = cartesian_to_spherical( FView );

    diag(FCurrentTransform, 1);

    MakeStep( 0.0f );
}

void PanOrbitPositioner::MakeStep( float dt )
{
    /// zoom/pan faster when we are farther away
    float PanVelocity  = FVelocityMultiplier * std::max( FMinPanningVelocity, FPanningVelocity * FViewDistance );
    float ZoomVelocity = FVelocityMultiplier * std::max( FMinZoomingVelocity, FZoomVelocity    * FViewDistance );

    /// use supplied input values
    float PanHorz  = -FPanDelta.x;
    float PanVert  = -FPanDelta.y;
    float LookHorz = -FOrbitDelta.x;
    float LookVert = -FOrbitDelta.y;

    float Zoom = FZoomIn + FZoomOut;

    if ( FOrbiting )
    {
        LookHorz += -FMouseDelta.x;
        LookVert += -FMouseDelta.y;
    }

    if ( FPanning )
    {
        PanHorz += FMouseDelta.x;
        PanVert += -FMouseDelta.y;
    }

    float MoveFactorH = PanHorz * PanVelocity * dt;
    float MoveFactorV = PanVert * PanVelocity * dt;

    // TODO : use Up/ViewDir here
    vec3 HorizDir (FCurrentTransform.x[0 + 0], FCurrentTransform.x[1 + 0], FCurrentTransform.x[2 + 0]);
    vec3 VertDir  (FCurrentTransform.x[0 + 4], FCurrentTransform.x[1 + 4], FCurrentTransform.x[2 + 4]);

    vec3 HorzMove = HorizDir;
    vec3 VertMove = VertDir;
    HorzMove *= MoveFactorH;
    VertMove *= MoveFactorV;

    FTarget = FTarget + HorzMove + VertMove;

    FViewDistance += Zoom * ZoomVelocity * dt;

    /// handle orbiting
    FSphericalCoords.z -= LookVert * FVelocityMultiplier * FOrbitingVelocity * dt;
    FSphericalCoords.y += LookHorz * FVelocityMultiplier * FOrbitingVelocity * dt;

    /// avoid that the camera slips past the center of interest
    if ( FViewDistance < FMinDistance )
    {
        FViewDistance = FMinDistance;
        vec3 delta (FCurrentTransform.x[0 + 8], FCurrentTransform.x[1 + 8], FCurrentTransform.x[2 + 8] );
         delta *= FViewDistance;
        FTarget = FTarget - delta;
    }

    /// get polar vector in cartesian space
    mtx4 ViewTrans = translate(0.0f, 0.0f, -FViewDistance);

    float Angle2 = deg2rad( FSphericalCoords.z );
    float Angle1 = deg2rad( FSphericalCoords.y - 90.0f );

    mtx4 RotY, RotZ;
    vec3 MinusX ( 1, 0, 0 );

    rotate_matrix_axis( RotY, Angle2, MinusX );
    rotate_matrix_axis( RotZ, Angle1, FUpVector );

    mtx4 m = RotZ * RotY * ViewTrans;

    FCurrentTransform = translate(FTarget.x, FTarget.y, FTarget.z) * m;

    decompose_camera_transform( FCurrentTransform, FViewerPosition, m );

    FMouseDelta.x = FMouseDelta.y = 0;
}
