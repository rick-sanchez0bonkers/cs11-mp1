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
#define DEBUG_MODE 1

#define debug(fmt, ...) if (DEBUG_MODE) printf("\033[34m[%s:%d]\033[0m : " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define wdebug(fmt, ...) if (DEBUG_MODE) wprintf(L"\033[34m[%d]\033[0m : " fmt "\n", __LINE__, ##__VA_ARGS__) // omitting file name cuz no idea how to convert it to wchar_t*
#define error(fmt, ...) printf("\033[41m[%s:%d]\033[0m : " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); raise(SIGABRT)
// adds error checking
#define safemalloc(pointer, size) \
    pointer = malloc(size); \
    if (pointer == NULL) { \
        error("Malloc failed to allocate memory"); \
    }
#define saferealloc(pointer, size) \
    pointer = realloc(pointer, size); \
    if (pointer == NULL) { \
        error("Realloc failed to allocate memory"); \
    } \
    debug("Successful realloc. Pointer address: %p, number of bytes: %d", pointer, (int) (size));

// free the the htings that getAllChildren()
// rely on
#define freearraypointer(array) 

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
    int frames; // number of images
    int framerate; // how much frames to wait before the next frame gets swapped
    int currframe; // what image are we on
    int loop; // if it loops or not
    LPCWSTR name; // the file name. if its an animation, gets the name of the first file and removes the number at the end (e.g. sad0.bmp -> sad)
    Image *image_pointer; // pointer to the array that contains the loaded images [one or more]. double pointer since itll be malloc'd

} ImageArray;

// global vars
ImageArray* MEM_IMAGES;
LPCWSTR bitmapsdir = L"bmp";
LPCWSTR c_filedir = L"graphics-testing-GDI.c";

// some global vars that keep track of important things
int THREAD_RUNNING = 1;
int THREAD_ENABLED = 1; // set to 0 to disable the thread without killing it
Vector2 MOUSE = {.x = 0, .y = 0};



// a seperate thread that acts as the
// frame rate by repeatedly calling WM_PAINT
// in a consistent interval
// not the best way to implement this because
// the update lags behind, but whatever. good enough
unsigned __stdcall thread(void *args) {
    HWND hwnd = *(HWND*) args; // retrieve the hwnd parm
    debug("Thread running\n");
    while (THREAD_RUNNING) {
        if (THREAD_ENABLED) 
            PostMessage(hwnd, WM_PAINT, 0, 0);
        Sleep(max(1000/FRAME_RATE, 1));
    }
    debug("Thread ended\n");

    return 0;
}

// ------- FUNCTIONS FOR PATHS ------- //
// credit:
// https://stackoverflow.com/questions/3828835/how-can-we-check-if-a-file-exists-or-not-using-win32-program

/// @brief checks if a given path is a directory
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

/// @brief remove the extension name. (e.g. bmp\\sad.bmp -> sad)
/// @param path directory
/// @param modifying address of LPWSTR place to output
void trimExtension(LPCWSTR path, LPWSTR *modifying) {
    size_t pathlen = wcslen(path);
    LPWSTR dest;
    for (int i=pathlen; i>0; i--) {
        // wprintf(L"%c", path[i]);
        if (path[i] == L'.') { // dot found
            // copy everything before the dot.
            safemalloc(dest, (i) * sizeof(WCHAR));
            wcsncpy(dest, path, i);
            break;
        }
        if (path[i] == L'\\') { // if we went thru the entire file name
            safemalloc(dest, (pathlen + 1) * sizeof(WCHAR));
            wcscpy(dest, path);
            break;
        }
    }
    debug("dest: %p", dest);
    *modifying = dest;
    debug("modifying: %p", modifying);
    wdebug("%s", dest);
}

/// @brief only get the file/folder name. (e.g. graphics-testing\\bmp\\sadanim -> sadanim)
/// @param path directory
/// @param modifying address of string to place output
void trimPath(LPCWSTR path, LPWSTR* modifying) {

}

// returns full path length
// already includes null terminator
DWORD getFullPathLength(LPCWSTR dir) {
    return GetFullPathName(dir, 0, NULL, NULL);;
}

//https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfullpathnamew
/// @brief returns the full path of a reltive path.
/// @param dir LPCWSTR relative dir
/// @return allocated var. dont forget to free().
LPWSTR getFullPathSafe(LPCWSTR dir) {
    // wdebug(L"Given dir: %s", dir);
    DWORD pathlen = GetFullPathName(dir, 0, NULL, NULL);
    // wdebug("Got pathlen %lu", pathlen);

    LPWSTR buffer;
    safemalloc(buffer, (pathlen + 1) * sizeof(WCHAR));
    DWORD pathlen2 = GetFullPathName(dir, pathlen, buffer, NULL); // patheln2 DOESNT include the null terminator
    // paranoia kicking in
    if (pathlen2 >= pathlen) {
        error("Pathlen discrepancy. Pathlen1: %d, Pathlen2: %d", (int) pathlen, (int) pathlen2);
    } 
    // wdebug(L"Buffer: %s", buffer);
    return buffer;

    // this shit below is completely unnecessary
    // because passing in NULL as the buffer WORKS WITH ZERO FUCKKKK
    
    // WCHAR *buffer = L"hi"; // cant be 0 because itll overcount ffs fuck this dumb ass function bruh
    // DWORD buflen = 3;
    // while (1) { // a shitty loop to make sure i allocate enough memory for the damn thing
    //     DWORD pathlen = GetFullPathName(dir, buflen, buffer, NULL);
    //     printf("Got pathlen %lu\n", pathlen);
    //     if (pathlen == 0) {
    //         wprintf(L"ERROR: FAILED TO RETRIEVE FULL PATH OF DIR '%s'\n", dir);
    //         free(buffer);
    //         error("Failed to get the path of dir above.");
    //     }
    //     if (buflen < pathlen) { // funny thing, pathlen FIRST overcounts and then finally counts correctly. wow!
    //         WCHAR *temp = realloc(buffer, pathlen * sizeof(WCHAR));
    //         if (temp != NULL) {
    //             buffer = temp;
    //         } else {
    //             free(buffer);
    //             error("REALLOC FAILURE.");
    //         }
    //         buflen = pathlen;
    //         continue;
    //     }
    //     printf("Pathlen: %lu\n", buflen);
    //     break;
    // }
    // return (LPWSTR) buffer;
}

// credit: https://stackoverflow.com/questions/68215054/browse-a-whole-folder-in-c
// https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findfirstfilea
// https://learn.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-win32_find_dataa
// https://learn.microsoft.com/en-us/windows/win32/fileio/listing-the-files-in-a-directory
/// @brief gets all files and subfolders in a folder. why this shit so damn complicated
/// @param dir 
/// @param arraybuffer modifies the given buffer. pass in &arraybuffer.
/// @return the number of files in the arraybuffer
DWORD getAllChildren(LPCWSTR dir, LPWSTR **arraybuffer) {
    LPWSTR fulldir = getFullPathSafe(dir);
    wdebug(L"Get all children in dir %s\n", fulldir);
    free(fulldir);

    // // WCHAR buff[wcslen(dir) + 1] = NULL; // have to initalize it bc malloc sux
    
    LPWSTR buff = safemalloc(buff, (wcslen(dir) + 1) * sizeof(WCHAR)); // have to initalize it bc malloc sux
    wcscpy(buff, dir); // copy the dir
    wcscat(buff, L"\\*"); // append this thing, no idea what it does or means.

    // wprintf(L"Buffer: %s\n", buff);

    // necessary vars for FindFirstFile and FindNextFile
    WIN32_FIND_DATA fd;
    HANDLE currfilehandle = FindFirstFile(buff, &fd);
    int cond = currfilehandle != INVALID_HANDLE_VALUE;

    // shit we want
    LPWSTR *array = NULL;
    DWORD number_of_files = 0;

    while (cond) {
        if (!isPath(fd.cFileName)) { // passing in things like "..\\folder" counts .. as a "file" so this check makes sure all the things r either folders or files.
            wdebug(L"Found file: %s", fd.cFileName);
            // if we dont add the folder before the file, getfullpathsafe will assume its in the same directory
            // as the executazble
            size_t filenamelen = (wcslen(fd.cFileName) + 1);
            LPWSTR filebuff;
            safemalloc(filebuff, (wcslen(dir) + filenamelen + 5) * sizeof(WCHAR));
            wcscpy(filebuff, dir);
            wcscat(filebuff, L"\\");
            wcscat(filebuff, fd.cFileName);

            DWORD fulldirlen = getFullPathLength(filebuff);// this is what we actually care about
            LPWSTR fulldir = getFullPathSafe(filebuff); 
            wdebug(L"Full dir: %s, length: %d", fulldir, (int) fulldirlen);
            
            // add the pointer to our array
            saferealloc(array, (number_of_files+1 + 1) * sizeof(WCHAR*));
            array[number_of_files] = fulldir;
            number_of_files++;
        }
        cond = FindNextFile(currfilehandle, &fd); // returns 0 if failure or just done
    }
    FindClose(currfilehandle);
    free(buff);

    debug("Buffer:");
    for (int i=0; i<number_of_files; i++) {
        LPWSTR file = array[i];
        wdebug("%s", file);
    }

    *arraybuffer = array; // make the arraybuffer point to array.

    debug("Total number of files: %d\n", (int) number_of_files);
    debug("array %p", array);
    return number_of_files;
    
}

/// @brief loads an image to memory.
/// @param image_or_folder_name the thing to load. can be a folder (if animation) or .bmp (if standalone)
/// @param layer the draw order. 0 = highest priority
/// @param framerate if its an animation, the framerate. (tied to FRAME_RATE)
void loadimage(LPCWSTR image_or_folder_name) {
    // if image:
    // image.bmp file

    // if folder:
    // image0.bmp
    // image1.bmp
    // ...
    // imageN.bmp
    // .config -> contains framerate and other stuff (if im gonna add any other stuff)

    wdebug(L"given thing to load: %s", image_or_folder_name);
    wdebug(L"is file? %d", isFile(image_or_folder_name));
    wdebug(L"is path? %d", isPath(image_or_folder_name));

    LPWSTR trimmed;
    trimExtension(image_or_folder_name, &trimmed);
    debug("trimmed: %p", trimmed);

    wdebug(L"%s", trimmed);

}

void OnCreate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    debug("CREATE\n");
    
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

    // looad images
    LPWSTR *files;
    DWORD nfiles = getAllChildren(bitmapsdir, &files);
    for (int i = 0; i < (int) nfiles; i++) {
        loadimage(files[i]);
    }
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
    // LPWSTR fullpath = getFullPathSafe(c_filedir);
    // wprintf(L"%s\n", fullpath);
    // free(fullpath);
    if (!isFile(c_filedir)) {
        error("EXECUTABLE FILE NOT IN THE SAME DIRECTORY AS THE .C FILE. \nTHE EXECUTABLE FILE HAS TO BE IN THE CS11 MP1 FOLDER TO PROPERLY FUNCTION.");
    }
    if (!isPath(bitmapsdir)) {
        error("EXECUTABLE FILE NOT IN THE SAME DIRECTORY AS bmp FOLDER. \nTHE EXECUTABLE FILE HAS TO BE IN THE CS11 MP1 FOLDER TO PROPERLY FUNCTION.");
    }
    debug("Verified required files\n");

    // testing getAllChildren
    // LPCWSTR fundir = L"..\\.."; 
    // LPWSTR *files = NULL;
    // DWORD number_of_files = getAllChildren(fundir, &files);
    // debug("files %p", files);
    // for (int i=0; i<number_of_files; i++) {
    //     LPWSTR file = files[i];
    //     wdebug("%s", file);
    // }

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