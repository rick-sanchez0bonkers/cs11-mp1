// file to mess around with making 
// graphics.
// since <graphics.h> is literally nonexistent for me
// im using <windows.h> instead and doing it raw :(
// following: https://learn.microsoft.com/en-us/windows/win32/learnwin32/creating-a-window
//          ^^ this is in C++ so it has different syntax but its close enough
//   this is pretty much boilerplate for <windows.h>
//   its so different from working in a regular thing

// this is literally just learning a whole new ass way to code
// graphics is done using Direct2D

/*
Very imporant things:
-lgdi32
this flag is needed for the compiler to link
the graphics functionality (for drawing with
hbrushes and whatever)


full command (in powershell)
gcc graphics-testing.c -lgdi32 -o output/graphics-testing.exe -Wall; ./output/graphics-testing.exe
                                                                    ^ runs the .exe immediately

*/

#ifndef UNICODE // adds support for unicode characters. Adds the L prefix for strings which can specify wide character type.
#define UNICODE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <process.h>
#include <signal.h> // for aborting

#include <d2d1.h> // REQUIRES A MODERN C COMPILER. i had a x32 version (MinGW.org GCC-6.3.0-1) 6.3.0 i had to uninstgall then install x64. followed https://code.visualstudio.com/docs/cpp/config-mingw

#define FRAME_RATE 60

// #define max(a, b) a > b ? a : b // apparently windef.h already has this
#define error(fmt, ...) printf("\033[41m[%s:%d]\033[0m : " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); raise(SIGABRT)

// some types to make my life easier
// -- TYPES -- //
// 2D Coordinates (integers)
typedef struct {
    int x;
    int y;
} Vector2;

// some global vars that keep track of important things
int THREAD_RUNNING = 1;
Vector2 MOUSE = {.x = 0, .y = 0};

// a seperate thread that acts as the
// frame rate by repeatedly calling WM_PAINT
// in a consistent interval
// not the best way to implement this because
// the update lags behind, but whatever. good enough
unsigned __stdcall thread(void *args) {
    HWND hwnd = *(HWND*) args; // retrieve the hwnd parm
    printf("Thread ran\n");

    while (THREAD_RUNNING) {
        PostMessage(hwnd, WM_PAINT, 0, 0);
        Sleep(max(1000/FRAME_RATE, 1));
    }

    printf("Thread ended\n");

    return 0;
}

// this is where ALL events are sent to
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        // quit the window. not having this means the window never really closes even if you click X.
        // this is callled if the window is QUITTING from any reason (X button, alt F4, task manager, etc.)
        // DestroyWindow() sends the WM_DESTROY message.
        case WM_DESTROY: 
            PostQuitMessage(0); // this POSTS a msg of WM_QUIT, causing the message loop to end 
            THREAD_RUNNING = 0;
            break;
            // WM_CLOSE is another thing where you can
            // override the closing action (like pressing X)
            // by deciding whether u want it to proceed to
            // WM_DESTROY or not.

            // WM_SIZE indicates resizing
            // case WM_SIZE:
            //     break;
        
        // needs -lgdi32 flag to function
        case WM_PAINT:; // p.s. you CANNOT immediately declare after a case statement. adding an empty statement (just a semicolon) fixes it
            // printf("painting, %d, %d\n", MOUSE.x, MOUSE.y);
            InvalidateRect(hwnd, NULL, 0); // forces windows to include the entire window as the rcPaint param instead of only the area needing updating.

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // All painting occurs here, between BeginPaint and EndPaint.
            HBRUSH black = CreateSolidBrush(RGB(0,0,0)); 
            HBRUSH blue = CreateSolidBrush(RGB(0,0,255)); 

            // FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW-1));
            FillRect(hdc, &ps.rcPaint, black);

            SelectObject(hdc, blue);
            // problem with this is, im pretty sure this uses the CPU to draw
            // and also no antialiasing
            Ellipse(hdc, MOUSE.x, MOUSE.y, MOUSE.x + 100, MOUSE.y + 100);

            DeleteObject(black);
            DeleteObject(blue);

            EndPaint(hwnd, &ps);
            break;
        case WM_MOUSEMOVE:; // mousemovenent
            MOUSE.y = (int) HIWORD(lParam); // u just gotta read docs to figure these out bruh...
            MOUSE.x = (int) LOWORD(lParam);

            // printf("movement: %d, %d\n", MOUSE.x, MOUSE.y); 
            break;
        case WM_LBUTTONDOWN: // left mosue down
            printf("m1 down\n");
            // SendMessage(hwnd, WM_PAINT, wParam, lParam);
            break;
        case WM_LBUTTONUP: // left mouse button up
            printf("m1 up\n");
            break;
        case WM_KEYDOWN:; // keyborad down
            long long KEYCODE_DOWN = (long long)LOWORD(wParam);

            printf("key down, %c %d\n", KEYCODE_DOWN, KEYCODE_DOWN); 
            break;
        case WM_KEYUP:; // keyborad up
            long long KEYCODE_UP = (long long)LOWORD(wParam);  // 64 bits

            printf("key up, %c %d\n", (char) KEYCODE_UP, (int) KEYCODE_UP); 
            if (KEYCODE_UP == VK_ESCAPE) { // escape key
                PostMessage(hwnd, WM_DESTROY, 0, 0);
            }
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam); // required, if the message isnt handled, just give it back and do default behavior
    }

    return 0;
}

// this is equivalent to int main()
// adding int main() to the program completely makes this function not be called.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // widnow class
    // L prefix makes it a wide char (uses more than 1 byte per char)
    // wide char is for unicode support
    wchar_t CLASSNAME[] = L"peepoo";
    WNDCLASS windowclass = {
        .lpfnWndProc = WindowProc,
        .hInstance = hInstance,
        .lpszClassName = CLASSNAME
    };

    // register the window clas
    RegisterClass(&windowclass);

    // create the legit window
    HWND hwnd = CreateWindowEx(
        0,
        CLASSNAME,
        L"gello",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,// the extra things disable resizing and minimizing/maximizing

        // pos (x, y) and size (x, y) in pixels
        CW_USEDEFAULT, CW_USEDEFAULT, 750, 750,

        // some other stuff
        NULL, // parent
        NULL, // menu
        hInstance, // the hinstance hting
        NULL    // additional app data
    );

    // start the frame loop thread
    HANDLE hThread = (HANDLE) _beginthreadex(
        NULL,
        0,
        &thread,
        &hwnd,
        0,
        NULL
    );

    if (hThread == NULL) {
        error("Unable to start frame loop thread.");
    }

    CloseHandle(hThread); // dereference the thread, itll keep running in the backgrund

    ShowWindow(hwnd, nCmdShow);

    // message loop. this is the feedback loop
    // for messages. the messages are handled by the
    // WindowsProc function. the while condition
    // is also important since its the closing condition
    // of the window. 
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) { // if the windows wants to quit, this will be 0
        TranslateMessage(&msg);
        DispatchMessage(&msg); // calls WindowsProc
    }

    return 0;
}