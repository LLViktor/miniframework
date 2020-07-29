#include "CameraView.h"

struct DemoWindow: public Window3D
{
    DemoWindow(int x, int y, int w, int h, const char* title): Window3D(x,y,w,h,title)
    {
        Camera.FTarget = vec3(0, -5, 0);
        Camera.FViewerPosition = vec3(-70, 0, -65);
        Camera.FUpVector = vec3(0,0,1);
        Camera.Reset();
    }

    virtual void Render3D()
    {
        FCanvas2D->Clear(0xAAAAAA);
        FCanvas3D->Plane(vec3(0,0,0), vec3(1, 0, 0), vec3(0, 1, 0), 2.0, 2.0, 10, 10, 0x00AA00);
    }
};

int main()
{
    App a;
    DemoWindow w(10, 10, 640, 360, "Demo");
    w.SetDelta(0.02f);
    w.Show(true);
    return a.Run();
}
