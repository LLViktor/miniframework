#pragma once

#include "CommonFramework.h"
#include "Canvas.h"

struct Window3D: public BaseWindow
{
    mtx4 FProj;
    PanOrbitPositioner Camera;

    virtual void OnDraw()
    {
        this->FCanvas3D->SetMatrices(FProj, Camera.FCurrentTransform);
        this->Render3D();
    }

    virtual void OnTimer()
    {
        BaseWindow::OnTimer();
        Camera.Update(GetDelta());
        this->Repaint();
    }

    virtual void OnWheelUp()   { Camera.FWheelTicks++; }
    virtual void OnWheelDown() { Camera.FWheelTicks--; }

    virtual void OnMouseUp(int Button, int mx, int my)
    {
        if(Button == MOUSE_BUTTON_LEFT)
        {
            pressed = false;

            Camera.FMouse.x = mx;
            Camera.FMouse.y = my;
            Camera.SetMouse(Camera.FMouse.x, Camera.FMouse.y);
            Camera.MiddleButton = false;
        }
        if(Button == MOUSE_BUTTON_RIGHT)
        {
            Camera.AltKey = false;
        }
    }

    virtual void OnMouseMove(int mx, int my)
    {
        if(pressed)
        {
            mousex = mx;
            mousey = my;
        }

        Camera.SetMouse(mx, my);
    }

    virtual void OnMouseDown(int Button, int mx, int my)
    {
        if(Button == MOUSE_BUTTON_LEFT)
        {
            Camera.MiddleButton = true;

            mousex = oldmousex = mx;
            mousey = oldmousey = my;
            pressed = true;
            return;
        }

        if(Button == MOUSE_BUTTON_RIGHT)
            Camera.AltKey = true;
    }

    void FixSize(int w, int h)
    {
        float aa = ((float)w / (float)h);
        frustum(FProj,10.0,150.0,-1.0 * aa,1.0 * aa,-1.0,+1.0);
    }

    Window3D(int x, int y, int w, int h, const char* title): BaseWindow(x, y, w, h, title)
    {
        Camera.FViewerPosition = vec3(0, 6, -20);
        Camera.FTarget         = vec3(0, -1,  0);

        Camera.FVelocityMultiplier = 4.0f / 20.0f;

        Camera.FOrbitingVelocity = 2.0f * 5.25f * 12;
        Camera.FPanningVelocity = 0.08f * 4;
        Camera.FZoomVelocity = 5.25f * 8 * 20;

        Camera.FMinPanningVelocity = 0.08f;
        Camera.FMinZoomingVelocity = 0.50f;

        Camera.Reset();

        // wrap this window's framebuffer
        FCanvasBitmap = new Bitmap(FB, w, h);
        FCanvas2D = new Canvas2D_Bitmap(FCanvasBitmap);
        FCanvas3D = new Canvas3D(FCanvas2D);

        FixSize(w, h);
    }

    Bitmap          *FCanvasBitmap;
    Canvas2D_Bitmap *FCanvas2D;
    Canvas3D        *FCanvas3D;

    virtual void Render3D() {}

protected:
    bool pressed;
    int mousex, mousey, oldmousex, oldmousey;
};
