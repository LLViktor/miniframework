Simple wireframe rendering framework for Linux and Windows.

Compilation of the sample:

On Linux

    gcc -o demo -Isrc example/demo.cpp src/CommonFramework.cpp src/Canvas.cpp src/Bitmap.cpp -lstdc++ -lm -lX11

For Windows (using MinGW or MSys2)

    gcc -o demo -Isrc example/demo.cpp src/CommonFramework.cpp src/Canvas.cpp src/Bitmap.cpp -lstdc++ -lgdi32 -luser32
