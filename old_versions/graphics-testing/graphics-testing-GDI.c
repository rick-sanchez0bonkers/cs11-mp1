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
#include <wchar.h> // https://en.cppreference.com/c/header/wchar for wide strings since file paths r wchar_t :(
#include <signal.h> // for aborting

// #include <d2d1.h> // REQUIRES A MODERN C COMPILER. i had a x32 version (MinGW.org GCC-6.3.0-1) 6.3.0 i had to uninstgall then install x64. followed https://code.visualstudio.com/docs/cpp/config-mingw

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

typedef struct {
    BITMAP *bitmap; // contains size
    HBITMAP *hbitmap; // the actual bitmap
} Image;

// ImageArray
typedef struct {
    int layer; // draw order. used to sort the array. lower layer = drawn first
    int frames;
    int framerate;
    Image **image_pointer; // pointer to the array that contains the loaded images [one or more]. double pointer since itll be malloc'd

} ImageArray;

// global vars
LPCWSTR bitmapsdir = L"bmp";
LPCWSTR c_filedir = L"graphics-testing-GDI.c";

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

// credit:
// https://stackoverflow.com/questions/3828835/how-can-we-check-if-a-file-exists-or-not-using-win32-program

/// @brief checks if a given path is valid
/// @param path 
/// @return boolean int
int isPath(LPCWSTR path) {
    DWORD attributes = GetFileAttributes(path);
    return (attributes != INVALID_FILE_ATTRIBUTES); // actually exists
}

/// @brief checks if a given path is a FILE not a directory
/// @param path 
/// @return boolean int
int isFile(LPCWSTR path) {
    DWORD attributes = GetFileAttributes(path);
    return (attributes != INVALID_FILE_ATTRIBUTES && (0 == (attributes & FILE_ATTRIBUTE_DIRECTORY))); // actually exists and is a file
}

// TODO: make a function that returns full path name length.
size_t getFullPathLength(LPCWSTR dir) {

    return -1;
}

//https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfullpathnamew
/// @brief returns the full path of a reltive path.
/// @param dir LPCWSTR relative dir
/// @return allocated var. dont forget to free().
LPWSTR getFullPathSafe(LPCWSTR dir) {
    WCHAR *buffer = L"hi"; // cant be 0 because itll overcount ffs fuck this dumb ass function bruh
    DWORD buflen = 3;

    while (1) { // a shitty loop to make sure i allocate enough memory for the damn thing
        DWORD pathlen = GetFullPathName(dir, buflen, buffer, NULL);
        
        if (pathlen == 0) {
            wprintf(L"ERROR: FAILED TO RETRIEVE FULL PATH OF DIR '%s'\n", dir);
            free(buffer);
            error("Failed to get the path of dir above.");
        }
        if (buflen < pathlen) { // funny thing, pathlen FIRST overcounts and then finally counts correctly. wow!
            WCHAR *temp = realloc(buffer, pathlen * sizeof(WCHAR));
            if (temp != NULL) {
                buffer = temp;
            } else {
                free(buffer);
                error("REALLOC FAILURE.");
            }
            buflen = pathlen;
            continue;
        }
        // printf("Pathlen: %d\n", buflen);
        break;
    }
    return (LPWSTR) buffer;
}

// credit: https://stackoverflow.com/questions/68215054/browse-a-whole-folder-in-c
// https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findfirstfilea
// https://learn.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-win32_find_dataa
// https://learn.microsoft.com/en-us/windows/win32/fileio/listing-the-files-in-a-directory
/// @brief gets all files and subfolders in a folder. why this shit so damn complicated
/// @param dir 
/// @return an array that contains the full path names. dont forget to free().
void getAllChildren(LPCWSTR dir) {
    // LPWSTR fulldir = getFullPathSafe(dir);
    // wprintf(L"Full dir: %s\n", fulldir);

    // WCHAR buff[wcslen(dir) + 1] = NULL; // have to initalize it bc malloc sux
    
    LPWSTR buff = malloc((wcslen(dir) + 5) * sizeof(WCHAR)); // have to initalize it bc malloc sux
    wcscpy(buff, dir); // copy the dir
    wcscat(buff, L"\\*"); // append this thing, no idea what it does or means.

    wprintf(L"Buffer: %s\n", buff);

    WIN32_FIND_DATA fd;
    HANDLE currfilehandle = FindFirstFile(buff, &fd);
    size_t pathslen = 0;
    int cond = currfilehandle != INVALID_HANDLE_VALUE;

    LPWSTR* temp_array;

    while (cond) {
        wprintf(L"Found file: %s\n", fd.cFileName);
        if (!isPath(fd.cFileName)) {
            printf("Not a path.\n");

            // format this shit properly...
            size_t filenamelen = (wcslen(fd.cFileName) + 1);
            LPWSTR filebuff = malloc((wcslen(dir) + filenamelen + 5) * sizeof(WCHAR) + filenamelen);
            wcscpy(filebuff, dir);
            wcscat(filebuff, L"\\");
            wcscat(filebuff, fd.cFileName);

            // pathslen += filenamelen;
            // wprintf(L"File buff: %s\n", filebuff);
            LPWSTR fulldir = getFullPathSafe(filebuff); // this is what we actually care about
            wprintf(L"Full dir: %s\n", fulldir);
            free(fulldir);
            LPWSTR temp[3] = L"hi";
            DWORD fullffname = GetFullPathName(filebuff, 1, temp, NULL); // setting buffer length to 0 causes GetFullPathName to overcount
            wprintf(L"Full dir len: %d\n", (int) fullffname);
            free(filebuff);
            free(temp);
        }
        cond = FindNextFile(currfilehandle, &fd); // returns 0 if failure or just done
        printf("cond: %d\n", cond);
    }
    FindClose(currfilehandle);
    free(buff);

    printf("Total length: %d\n", pathslen);
    // return (LPCWSTR *) NULL;
    
}

/// @brief loads an image to memory.
/// @param image_or_folder_name the thing to load. can be a folder (if animation) or .bmp (if standalone)
/// @param layer the draw order. 0 = highest priority
/// @param framerate if its an animation, the framerate. (tied to FRAME_RATE)
void loadimage(LPCWSTR image_or_folder_name, int layer, int framerate) {

}

void OnCreate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    printf("CREATE\n");
    
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
}

// this is where ALL events are sent to
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: OnCreate(hwnd, uMsg, wParam, lParam); break; // when the window is messaged to be made.
            


        // quit the window. not having this means the window never really closes even if you click X.
        // this is callled if the window is QUITTING from any reason (X button, alt F4, task manager, etc.)
        // DestroyWindow() sends the WM_DESTROY message.
        case WM_DESTROY: 
            printf("STOP\n");
            THREAD_RUNNING = 0;
            PostQuitMessage(0); // this POSTS a msg of WM_QUIT, causing the message loop to end 
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

            printf("key down, %c %lld\n", (char) KEYCODE_DOWN, KEYCODE_DOWN); 
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
    // just check if the .c file is in the same path as the executable
    LPWSTR fullpath = getFullPathSafe(c_filedir);
    wprintf(L"%s\n", fullpath);
    free(fullpath);
    if (!isFile(c_filedir)) {
        error("EXECUTABLE FILE NOT IN THE SAME DIRECTORY AS THE .C FILE. \nTHE EXECUTABLE FILE HAS TO BE IN THE CS11 MP1 FOLDER TO PROPERLY FUNCTION.");
    }
    if (!isPath(bitmapsdir)) {
        error("EXECUTABLE FILE NOT IN THE SAME DIRECTORY AS bmp FOLDER. \nTHE EXECUTABLE FILE HAS TO BE IN THE CS11 MP1 FOLDER TO PROPERLY FUNCTION.");
    }

    LPCWSTR fundir = L"..\\.."; 
    getAllChildren(fundir);

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