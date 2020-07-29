#include "Bitmap.h"
#include <stdlib.h>

void Bitmap::Clear(int color)
{
    unsigned char R = (color >> 16) & 0xFF;
    unsigned char G = (color >>  8) & 0xFF;
    unsigned char B = (color      ) & 0xFF;

    int sz = Width * Height;
    unsigned char* pos = FB;

    for(int i = 0 ; i < sz ; i++)
    {
        *pos++ = R;
        *pos++ = G;
        *pos++ = B;
    }
}

void Bitmap::SetPixel(int x, int y, int color)
{
    if(x < 0 || y < 0 || x >= Width || y >= Height) { return; }

    unsigned char R = (color >> 16) & 0xFF;
    unsigned char G = (color >>  8) & 0xFF;
    unsigned char B = (color      ) & 0xFF;

    int ofs = (y * Width + x) * 3;

    FB[ofs + 0] = R;
    FB[ofs + 1] = G;
    FB[ofs + 2] = B;
}

int  Bitmap::GetPixel(int x, int y)
{
    if(x < 0 || y < 0 || x >= Width || y >= Height) { return 0; }

    int ofs = (y * Width + x) * 3;
    return (((int)(FB[ofs + 0])) << 16) + (((int)(FB[ofs + 1])) << 8) + ((int)(FB[ofs + 2]));
}

// Adapted from https://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C
void Bitmap::Line(int x0, int y0, int x1, int y1, int color)
{
    int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
    int err = (dx>dy ? dx : -dy)/2, e2;

    for(;;)
    {
        SetPixel(x0,y0, color);
        if (x0==x1 && y0==y1) break;
        e2 = err;
        if (e2 >-dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}
