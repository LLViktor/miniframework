#pragma once

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <map>
#endif /** __linux */

#ifdef _WIN32
#  define MOUSE_BUTTON_LEFT 0
#  define MOUSE_BUTTON_RIGHT 1
#elif __linux__
#  define MOUSE_BUTTON_LEFT 1
#  define MOUSE_BUTTON_RIGHT 3
#endif

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

class BaseWindow;

struct App
{
	App();
	int Run();
	void Exit();

	void SetMainWindow(BaseWindow* W) { MainWnd = W; }

#ifdef __linux__
	static std::map<Window, BaseWindow*> FWnd2Window;

	static Display *FDisplay;
	static int FScreen;

	bool FShouldExit;

	static void RegisterWindow(BaseWindow* W);
	static void UnregisterWindow(BaseWindow* W);
#endif
private:
	// reference to the main window (once it closes we exit the app)
	BaseWindow* MainWnd;
};

struct BaseWindow
{
	BaseWindow(int x, int y, int w, int h, const char* title);
	~BaseWindow();

	void Repaint();
	
	void SetTitle(const char* title);
	void SetPos(int x, int y);
	void SetSize(int w, int h);

	void Show(bool Visible);

	void SetDelta(float dt);
	float GetDelta() const { return DeltaTime; }

	virtual void OnPaint();
	virtual void OnTimer() {}
	/// Callback for user-defined rendering
	virtual void OnDraw() {}

	virtual void OnMouseDown(int btn, int x, int y) {}
	virtual void OnMouseUp(int btn, int x, int y) {}
	virtual void OnMouseMove(int x, int y) {}
	virtual void OnWheelUp() {}
	virtual void OnWheelDown() {}

	virtual void OnKeyDown(int key) {}
	virtual void OnKeyUp(int key) {}

	int Width, Height;
	unsigned char* FB;

#ifdef _WIN32
	HWND hWnd;

	void SendDestroy();

	/// Temporary memory device context
	HDC hMemDC;
	
	/// Temporary GDI bitmap object
	HBITMAP hTmpBmp;
	
	/// Packed bitmap information
	BITMAPINFO BitmapInfo;

	bool IsCtrlOn()  const { return (GetKeyState(VK_CONTROL) < 0); }
	bool IsShiftOn() const { return (GetKeyState(VK_SHIFT) < 0); }
	bool IsAltOn() const { return (GetKeyState(VK_MENU) < 0); }
#endif

#ifdef __linux__
	bool IsAltOn()   const { return AltPressed; }
	bool IsCtrlOn()  const { return CtrlPressed; }
	bool IsShiftOn() const { return ShiftPressed; }

	bool ShiftPressed;
	bool AltPressed;
	bool CtrlPressed;

	Window FWnd;
private:
	unsigned char* FBOut;
	int outBits;
	GC copyGC;
	XImage* img;
#endif

private:
	BaseWindow() {}
	float DeltaTime;
};
