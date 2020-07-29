#include "CommonFramework.h"

#ifdef _WIN32
#  include <windowsx.h>
#endif

#ifdef __linux__

#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

Display* App::FDisplay = NULL;
int App::FScreen;

std::map<Window, BaseWindow*> App::FWnd2Window;

App::App()
{
	FShouldExit = false;
	FDisplay = XOpenDisplay (NULL);
	FScreen  = DefaultScreen (FDisplay);

	MainWnd = NULL;
}

void App::Exit()
{
	if(!MainWnd)
		FShouldExit = true;
}

void App::RegisterWindow(BaseWindow* W)
{
	FWnd2Window[W->FWnd] = W;
}

void App::UnregisterWindow(BaseWindow* W)
{
	FWnd2Window[W->FWnd] = NULL;
}

int App::Run()
{
	while (!FShouldExit)
	{
		XEvent event;
		if ( XPending ( this->FDisplay ) )
		{
			XNextEvent( this->FDisplay, &event );
		} else
		{
			// wait for 10 milliseconds
			usleep(10000);

			for(std::map<Window, BaseWindow*>::iterator i = App::FWnd2Window.begin(); i != App::FWnd2Window.end() ; i++)
			{
				if(i->second != NULL)
					i->second->OnTimer();
			}
			continue;
		}

		if(FWnd2Window.count(event.xany.window) < 1)
			continue;

		BaseWindow* wnd = FWnd2Window[event.xany.window];

		switch  (event.type)
		{
			/* We could have handled the ConfigureNotify for window resize */
			case Expose:
				wnd->OnPaint();
				break;            

			case KeyRelease:
			{
				KeySym sym = XLookupKeysym (&event.xkey, 0);
				/* handle modifiers */

				if ( sym == XK_Control_L || sym == XK_Control_R )
				{
					wnd->CtrlPressed = true;
				} else
				if ( sym == XK_Shift_L || sym == XK_Shift_R )
				{
					wnd->ShiftPressed = true;
				} else
				if ( sym == XK_Alt_L || sym == XK_Alt_R )
				{
					wnd->AltPressed = true;
				} else
				{
					wnd->OnKeyUp( XLookupKeysym (&event.xkey, 0) );
				}
			}
			break;

			case KeyPress:
			{
				KeySym sym = XLookupKeysym (&event.xkey, 0);
				/* handle modifiers */

				if ( sym == XK_Control_L || sym == XK_Control_R )
				{
					wnd->CtrlPressed = false;
				} else
				if ( sym == XK_Shift_L || sym == XK_Shift_R )
				{
					wnd->ShiftPressed = false;
				} else
				if ( sym == XK_Alt_L || sym == XK_Alt_R )
				{
					wnd->AltPressed = false;
				} else
				{
					wnd->OnKeyDown( XLookupKeysym (&event.xkey, 0) );
				}

				break;
			}

			case ButtonPress:
				if(event.xbutton.button < 4)
					wnd->OnMouseDown(event.xbutton.button, event.xbutton.x, event.xbutton.y);
				break;

			case ButtonRelease:
			{
				if(event.xbutton.button < 4)
				{
					wnd->OnMouseUp(event.xbutton.button, event.xbutton.x, event.xbutton.y);
				} else
				if (event.xbutton.button == 4)
				{
					wnd->OnWheelDown();
				} else
				if (event.xbutton.button == 5)
				{
					wnd->OnWheelUp();
				}
				break;
			}
  
			case MotionNotify:
			{
				wnd->OnMouseMove(event.xbutton.x, event.xbutton.y);
				break;
			}

			default:
				break;
		}			
	}

	return 0;
}

BaseWindow::BaseWindow(int x, int y, int w, int h, const char* title): Width(w), Height(h)
{
	FBOut = new unsigned char[w * h * 4];

	FB = new unsigned char[w * h * 3];
	memset(FB, 0xFF, w * h * 3);

	Display* dis = App::FDisplay;

	FWnd = XCreateSimpleWindow(dis, RootWindow(dis, 0), x, y, w, h, 0, BlackPixel (dis, 0), BlackPixel(dis, 0));

	XSelectInput(dis, FWnd, StructureNotifyMask | ExposureMask | PointerMotionMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask );

	copyGC = XCreateGC (dis, FWnd, 0, NULL);

	App::RegisterWindow(this);

	int depth   = DefaultDepth (App::FDisplay, App::FScreen);

	/** some devices have 16-bit only FB. another problem is the compatibility with Win32/24-bit */
	outBits = (depth == 16) ? 16 : 32;

	img = XCreateImage (App::FDisplay, CopyFromParent, depth, ZPixmap, 0, (char *)FBOut, Width, Height, outBits, 0);
	if(img == NULL)
		return;

	XInitImage (img);

	img->byte_order = LSBFirst;
	/// The bitmap_bit_order doesn't matter with ZPixmap images.
	img->bitmap_bit_order = MSBFirst;

	XMapWindow(App::FDisplay, FWnd);
	SetPos(x, y);
	SetSize(w, h);
	SetTitle(title);

	CtrlPressed  = false;
	ShiftPressed = false;
	AltPressed   = false;
}

BaseWindow::~BaseWindow()
{
	App::UnregisterWindow(this);
	XDestroyImage(img);
	delete[] FB;
	FB = NULL;
}

void BaseWindow::SetTitle(const char* title) { XStoreName(App::FDisplay, FWnd, title); }

void BaseWindow::SetPos(int x, int y) { XMoveWindow(App::FDisplay, FWnd, x, y); }
void BaseWindow::SetSize(int w, int h) { XResizeWindow(App::FDisplay, FWnd, w, h); Width = w; Height = h; }

void BaseWindow::Show(bool Visible)
{
	Visible ? XMapWindow(App::FDisplay, FWnd) : XUnmapWindow(App::FDisplay, FWnd);

	XFlush(App::FDisplay);
}

void BaseWindow::SetDelta(float dt)
{
	DeltaTime = dt;
}

void BaseWindow::Repaint()
{
	XClearArea(App::FDisplay, FWnd, 0, 0, 1, 1, true);
}

void BaseWindow::OnPaint()
{
	OnDraw();

	// copy FB to FBOut (invert image rows and RGB(24bit) to BGRA(32bit) conversion)
	// for 16-bit output buffers we also should perform the conversion

	if(outBits == 32)
	{
		for(int j = 0 ; j < Height ; j++)
		{
			unsigned char *fb    = FB    + j * Width * 3;
			unsigned char *fbOut = FBOut + j * Width * 4;

			for(int i = 0 ; i < Width ; i++)
			{
				unsigned char r = *fb++;
				unsigned char g = *fb++;
				unsigned char b = *fb++;

				*fbOut++ = b;
				*fbOut++ = g;
				*fbOut++ = r;
				*fbOut++ = 0xFF;
			}
		}

	} else
	{
		for(int j = 0 ; j < Height ; j++)
		{
			unsigned char  *fb    = FB    + j * Width * 3;
			unsigned short *fbOut = (unsigned short *)FBOut + j * Width;

			for(int i = 0 ; i < Width ; i++)
			{
				unsigned char r = *fb++;
				unsigned char g = *fb++;
				unsigned char b = *fb++;

				unsigned short v = ((b >> 3) << 11) | ((g >> 2) << 5) | (r >> 3);

				*fbOut++ = v;
			}
		}

	}
	XPutImage (App::FDisplay, FWnd, copyGC, img, 0, 0, 0, 0, Width, Height);
	XFlush (App::FDisplay);
}

#endif

#ifdef _WIN32
// win32-specfic window class name
const char* AppWindowClassName = "OurApplicationWindow";

static LRESULT CALLBACK MyWindowFunction(HWND, UINT, WPARAM, LPARAM);

App::App()
{
	MainWnd = NULL;

	WNDCLASSA wcl;
	memset(&wcl, 0, sizeof(WNDCLASSA));
	wcl.lpszClassName = AppWindowClassName;
	wcl.lpfnWndProc = MyWindowFunction;
	wcl.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; // for GL compatibility
	wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);

	RegisterClassA(&wcl);
}

void App::Exit()
{
	if(!MainWnd) { return; }
	MainWnd->SendDestroy();
}

int App::Run()
{
	MSG msg;
	
	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	return msg.wParam;
}

LRESULT CALLBACK MyWindowFunction(HWND this_hwnd,UINT message, WPARAM wParam, LPARAM lParam)
{
	BaseWindow* W = ( BaseWindow* )GetWindowLongPtr( this_hwnd, GWLP_USERDATA );

	switch(message)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_KEYUP:
			if(W) { W->OnKeyUp(wParam); }

			break;

		case WM_KEYDOWN:
			if(W) { W->OnKeyDown(wParam); }

			break;

		case WM_TIMER:
			if(W) { W->OnTimer(); }
			break;

		case WM_LBUTTONDOWN:
			SetCapture(this_hwnd);
			if(W) { W->OnMouseDown(0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); }
			break;

		case WM_MOUSEMOVE:
			if(W) { W->OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); }
			break;

		case WM_LBUTTONUP:
			if(W) { W->OnMouseUp(0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); }
			ReleaseCapture();
			break;

		case WM_RBUTTONDOWN:
			SetCapture(this_hwnd);
			if(W) { W->OnMouseDown(1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); }
			break;

		case WM_RBUTTONUP:
			if(W) { W->OnMouseUp(1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); }
			ReleaseCapture();
			break;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(this_hwnd, &ps);

			if(W) { W->OnPaint(); }

			EndPaint(this_hwnd, &ps);
			break;
		}

		case WM_MOUSEWHEEL:
		{
			if (W)
			{
				( static_cast<int>( wParam ) > 0 ) ? W->OnWheelUp() : W->OnWheelDown();
			}

			break;
		}
	}

	return DefWindowProc(this_hwnd, message, wParam, lParam);
}

BaseWindow::BaseWindow(int x, int y, int w, int h, const char* title)
{
	hMemDC = NULL;

	FB = new unsigned char[w * h * 4];

	hWnd = CreateWindowA(AppWindowClassName, "", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP, NULL, NULL, NULL);
	SetWindowLongPtrA( hWnd, GWLP_USERDATA, (LONG_PTR)this );
	
	SetPos(x, y);
	SetSize(w, h);
	SetTitle(title);

	/// create bitmap DC
	HDC hdc = ::GetDC(hWnd);

	/// Internal representation of the GDI bitmap
	hMemDC = CreateCompatibleDC(hdc);
	hTmpBmp = CreateCompatibleBitmap(hdc, Width, Height);
	memset(&BitmapInfo.bmiHeader, 0, sizeof(BITMAPINFOHEADER));
	
	BitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	BitmapInfo.bmiHeader.biWidth = Width;
	BitmapInfo.bmiHeader.biHeight = Height;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 24;
	BitmapInfo.bmiHeader.biSizeImage = Width * Height * 24;

	ReleaseDC(hWnd, hdc);
}

BaseWindow::~BaseWindow()
{
	if(FB) { delete[] FB; }

	DeleteDC(hMemDC);
	DeleteObject(hTmpBmp);
}

void BaseWindow::SendDestroy()
{
	SendMessage(hWnd, WM_DESTROY, 0, 0);
}

void BaseWindow::SetTitle(const char* title)
{
	SetWindowTextA(hWnd, title);
}

void BaseWindow::SetPos(int x, int y)
{
	SetWindowPos(hWnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

void BaseWindow::SetSize(int w, int h)
{
	Width = w;
	Height = h;
	SetWindowPos(hWnd, NULL, 0, 0, w, h, SWP_NOZORDER | SWP_NOMOVE);
}

void BaseWindow::Show(bool Visible)
{
	ShowWindow(hWnd, Visible ? SW_SHOW : SW_HIDE);
}

void BaseWindow::SetDelta(float dt)
{
	DeltaTime = dt;
	
	SetTimer(hWnd, 111, (int)(1000.0f * dt), NULL);
}

void BaseWindow::Repaint()
{
	InvalidateRect(hWnd, NULL, 0);
}

void BaseWindow::OnPaint()
{
	OnDraw();

	int Stride = Width * 3;

	unsigned char Tmp[16384 * 3];

	for(int y = 0 ; y < Height / 2; y++)
	{
		unsigned char* Src = this->FB + y * Stride;
		unsigned char* Dst = this->FB + (Height - y - 1) * Stride;

		memcpy(Tmp, Src, Stride);
		memcpy(Src, Dst, Stride);
		memcpy(Dst, Tmp, Stride);
	}

	// Copy image bits to GDI bitmap
	SetDIBits(hMemDC, hTmpBmp, 0, Height, (BYTE*)FB, &BitmapInfo, DIB_RGB_COLORS);

	HDC h = ::GetDC(hWnd);

	SelectObject(hMemDC, hTmpBmp);
	BitBlt(h, 0, 0, Width, Height, hMemDC, 0, 0, SRCCOPY);

	ReleaseDC(hWnd, h);
}

#endif
