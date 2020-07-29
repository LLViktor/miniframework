#pragma once

/// Simple 24-bit image with pixel and line rendering
struct Bitmap
{
    Bitmap(unsigned char* buffer, int W, int H): FB(buffer), Width(W), Height(H) {}
    
    void Clear(int color);

    void SetPixel(int x, int y, int color);
    int  GetPixel(int x, int y);

    void Line(int x1, int y1, int x2, int y2, int color);

    // Dimensions
    int  Width, Height;

    // The buffer;
    unsigned char* FB;
};
