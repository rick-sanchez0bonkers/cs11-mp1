// file to mess around with making 
// graphics.
// since <graphics.h> is literally nonexistent for me
// im using <windows.h> instead and doing it raw :(
// following: https://learn.microsoft.com/en-us/windows/win32/learnwin32/creating-a-window
//          ^^ this is in C++ so it has different syntax fuck
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // widnow class
    // L prefix makes it a wide char (uses more than 1 byte per char)
    // wide char is for unicode support
    // wchar_t CLASSNAME[] = L"peepoo";
    // WNDCLASS windowclass = {
    //     .lpfnWndProc = Wind
    //     .hInstance = inst;

    // };

    MessageBoxW(NULL, L"Hi", L"Hello", MB_OK | MB_ICONINFORMATION);
    return 0;
}