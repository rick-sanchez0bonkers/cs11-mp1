/*      BS COMPUTER SCIENCE PROJECT -- SNAKE GAME    */
/*      THIS IS A GRAPHICS IMPLEMENTATION OF THE     */
/*      SNAKE GAME USING <windows.h>.                */
/*      ONLY WINDOWS COMPATIBLE                      */
/*
-------------------------------------------------------
Authors:
    *omitted on github*

Date Started:
    November 9, 2026

Date Finished:
    November 21, 2026

IMPORTANT INFORMATION:
    To compile on WINDOWS 
        gcc "CS11 MP1_BANGSI-IL_DUQUE_SANTOS.c" -lgdi32 -lmsimg32 -Wall -o "CS11 MP1_BANGSI-IL_DUQUE_SANTOS.exe"
    
    To compile and run immediately (powershell):
        gcc "CS11 MP1_BANGSI-IL_DUQUE_SANTOS.c" -lgdi32 -lmsimg32 -Wall -o "CS11 MP1_BANGSI-IL_DUQUE_SANTOS.exe"; ./"CS11 MP1_BANGSI-IL_DUQUE_SANTOS.exe"

    -ldgdi32 is windows Graphics Device Interface for rendering images (like drawing ellipses or most crucially BitBlt)
    -lmsimg32 is some windows library for manipulating HBITMAPS, adds TransparentBlt which allows keying out a specific color for transparency
        
    THE EXECUTABLE FILE MUST BE IN THE SAME DIRECTORY AS THE SOURCE FILE
    THE EXECUTABLE FILE MUST BE IN THE SAME DIRECTORY AS THE bmp FOLDER
    


Credits to very useful resources used throughout the project:
    ANSI CODES for console coloring:
    https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797

    configuring gcc for MinGW x64:
    https://code.visualstudio.com/docs/cpp/config-mingw

    windows.h basics:
    https://learn.microsoft.com/en-us/windows/win32/learnwin32/creating-a-window
    https://winprog.org/tutorial/bitmaps.html

    windows.h docs:
    https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-transparentblt
    https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfullpathnamew
    https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findfirstfilea
    https://learn.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-win32_find_dataa
    https://learn.microsoft.com/en-us/windows/win32/fileio/listing-the-files-in-a-directory

    wchar.h documentation:
    https://en.cppreference.com/c/header/wchar

    geeksforgeeks:
    https://www.geeksforgeeks.org/dsa/program-count-digits-integer-3-different-methods/

    stackoverflow things:
    https://stackoverflow.com/questions/3828835/how-can-we-check-if-a-file-exists-or-not-using-win32-program
    https://stackoverflow.com/questions/68215054/browse-a-whole-folder-in-c
    https://stackoverflow.com/questions/1541492/how-to-code-for-reducing-flicker-need-your-help-with-code-win32-vc-flicker
-------------------------------------------------------

*/

// file to mess around with making 
// graphics.
// since <graphics.h> is literally nonexistent for me
// im using <windows.h> instead and doing it raw :(
// following: https://learn.microsoft.com/en-us/windows/win32/learnwin32/creating-a-window
//          ^^ this is in C++ so it has different syntax but its close enough
//   this is pretty much boilerplate for <windows.h>
//   its so different from working in a regular thing

// this is literally just learning a whole new ass way to code
// graphics is done using Windows GDI

/*
Very imporant things:
-lgdi32
this flag is needed for the compiler to link
the graphics functionality (for drawing with
hbrushes and whatever)
-lmsimg32
this flag is needed for the compiler to link
with functionalities for some more
graphics functionalities. i have no idea why
theyre separated.
the entire rendering uses TransparentBlt()
which this library provides.

full command (in powershell)
gcc graphics-testing-GDI.c -lgdi32 -lmsimg32 -Wall -o graphics-testing-GDI.exe; ./graphics-testing-GDI.exe
                                                                    ^ runs the .exe immediately
-ldgdi32 is windows Graphics Device Interface for rendering images (like drawing ellipses or most crucially BitBlt)
-lmsimg32 is some windows library for manipulating HBITMAPS, adds TransparentBlt which allows keying out a specific color for transparency

*/

// adds support for unicode characters. Adds the L prefix for strings which can specify wide character type.
#ifndef UNICODE
#define UNICODE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h> // variadic functions support
#include <windows.h>
#include <process.h>
#include <math.h>
#include <wchar.h> // https://en.cppreference.com/c/header/wchar for wide strings since file paths are wchar_t/WCHAR
#include <signal.h> // for aborting

// GRAPHICS CONSTANTS
#define FRAME_RATE 60            // number of frames per second
#define DEBUG_MODE 1             // if the console will print debug calls
#define CONFIGSIZE 1024*2        // max len for .config files in animation folders.
#define COLOR_KEY RGB(255,0,255) // this specific color is drawn as completely transparent.
#define WINDOWWIDTH 750          // window width
#define WINDOWHEIGHT 750         // window height
#define BMPPATH     L"bmp"
#define SOURCEPATH  L"CS11 MP1_BANGSI-IL_DUQUE_SANTOS.c"
//

// SNAKE GAME CONSTANTS
#define SIZE        15
#define MAX_FOOD    10
#define MIN_FOOD    1
#define MAX_BLOCKS  15
#define MIN_BLOCKS  5
#define EASY        0xFFFFFF // arbitrary values
#define DIFFICULT   0x0FFFFF

// numerical definition for each object
// this will be their values in the array
#define EMPTY       '.'
#define SNAKE       '@' // the snake is internally all SNAKE.
#define SNAKEBODY   'o' // this is just for printing. NOT ACTUAL INTERNAL REPRESENTATION OF SNAKE.
#define FOOD        'F'
#define PORTAL      '$'
#define BLOCK       'B' // this is the obstacles inside
#define WALL        '#' // this makes up the border.
//

// macros for printing
#define debug(fmt, ...) if (DEBUG_MODE) printf("\033[34m[%d]\033[0m : " fmt "\n", __LINE__, ##__VA_ARGS__)      // prints blue, DEBUG_MODE only
#define wdebug(fmt, ...) if (DEBUG_MODE) wprintf(L"\033[34m[%d]\033[0m : " fmt "\n", __LINE__, ##__VA_ARGS__)   // WCHAR support
#define warn(fmt, ...) printf("\033[43m[%d]\033[0m : " fmt "\n", __LINE__, ##__VA_ARGS__)                       // prints yellow
#define wwarn(fmt, ...) wprintf(L"\033[43m[%d]\033[0m : " fmt "\n", __LINE__, ##__VA_ARGS__)                    // WCHAR support
#define error(fmt, ...) printf("\033[41m[%s:%d]\033[0m : " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); raise(SIGABRT)      // for raising exceptions. no wchar support
#define print(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)                                                         // included \\n
#define wprint(fmt, ...) wprintf(fmt L"\n", ##__VA_ARGS__)                                                      // WCHAR support

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
    }

// -- TYPES -- //

// 2D Coordinates (integers)
typedef struct {
    int x;
    int y;
} Vector2;

// double version of Vector2
typedef struct {
    double x;
    double y;
} dVector2;

// Image
typedef struct {
    BITMAP bitmap; // contains size
    HBITMAP hbitmap; // the actual rgb information
} Image;

// ImageArray
typedef struct {
    double layer; // draw order. used to sort the array. lower layer = drawn first
    int frames; // number of images
    int framecount; // how much frames have elapsed?
    int framerate; // how much frames to wait before the next frame gets swapped
    int currframe; // what image are we on. starts at 0
    int loop; // if it loops or not. boolean int        UNSUPPORTED
    int enabled; // if it renders or not. boolean int
    double opacity; // 255 (opaque) to 0 (transparent). UNSUPPORTED
    double rotation; // in degrees                      UNSUPPORTED
    Vector2 position; // where on the screen to render
    Vector2 size; // what size to render the image?
    LPCWSTR name; // the file name. if its an animation, gets the name of the folder
    Image *image_pointer; // pointer to the array that contains the loaded images [one or more].
} ImageArray;

// global vars
long long unsigned int FRAME_LIFETIME = 0; // counts the number of frames the game has been thru
ImageArray** MEM_IMAGES; // this stores ALL of the loaded image's pointers.
size_t IMAGES_LOADED = 0;
size_t MAX_IMAGES_LOADED = 0;

int THREAD_RUNNING = 1; // set to 0 to kill the thread
int THREAD_ENABLED = 1; // set to 0 to disable the thread without killing it
Vector2 MOUSE = {.x = 0, .y = 0};

// -- GAME OBJECTS -- // 
// 15x15
ImageArray* SNAKEGAME_TILES[SIZE*SIZE]; // contains the tiles


// a seperate thread that acts as the
// frame rate by repeatedly calling WM_PAINT
// in a consistent interval
// not the best way to implement this because
// the update lags behind, but whatever. good enough
unsigned __stdcall thread(void *args) {
    HWND hwnd = (HWND) args; // retrieve the hwnd parm
    debug("Thread running");

    // make sure we didnt fuck anything up
    // verify if hwnd is actually linked to a window
    if (!IsWindow(hwnd)) {
        error("THREAD DID NOT GET HWND HANDLE SAFELY. FRAME LOOP IS NOT WORKING PROPERLY.");
    }

    while (THREAD_RUNNING) {
        FRAME_LIFETIME++;
        if (THREAD_ENABLED) 
            PostMessage(hwnd, WM_PAINT, 0, 0);
        Sleep(max(1000/FRAME_RATE, 1));
    }
    debug("Thread ended");

    return 0;
}

// ------- HELPER FUNCTIONS ------- //

// frees the filearray u get from getAllChildren
void freeFiles(LPCWSTR* files, DWORD nfiles) {
    for (int i = 0; i<nfiles; i++) {
        free((void *) files[i]);
        files[i] = NULL;
    }
    free(files);
}

/// @brief makes a fresh image array with default params
/// @return an image array
ImageArray newImageArray() {
    return (ImageArray) {
        .layer = 0.0,
        .frames = 1,
        .framecount = -1, // ignored if only 1 frame
        .framerate = -1, // ignored if only 1 frame
        .currframe = 0, // starts at 0 up until frames-1
        .loop = 0,
        .enabled = 1,
        .opacity = 255.0,
        .rotation = 0.0,
        .position = (Vector2) {.x = 0, .y = 0}, // dont got a function for this yet
        .size = (Vector2) {.x = 0, .y = 0}, // dont got a function for this yet
        // .name                omitted
        // .image_pointer       omitted
    };
}

/// @brief makes a heap image array with default params
/// @return a mallocated image array
ImageArray* newHeapImageArray() {
    ImageArray* heap = NULL;
    safemalloc(heap, sizeof(ImageArray));
    *heap = newImageArray(); // copies 
    return heap;
}


/// @brief prints the images stored in memory
void printmemimages() {
    debug("Number of images: %llu", IMAGES_LOADED);
    for (int i = 0; i < IMAGES_LOADED; i++) {
        debug("IMAGE %d", i);
        ImageArray data = *MEM_IMAGES[i];
        wdebug(L"\tNAME: %s", data.name);
        debug("\tLAYER: %f", data.layer);
        debug("\tFRAMES: %d", data.frames);
        debug("\tFRAMECOUNT: %d", data.framecount);
        debug("\tFRAMERATE: %d", data.framerate);
        debug("\tCURRFRAME: %d", data.currframe);
        debug("\tLOOP: %d", data.loop);
        debug("\tENABLED: %d", data.enabled);
        debug("\tOPACITY: %f", data.opacity);
        debug("\tROTATION: %f", data.rotation);
        debug("\tPOSITION: (%d, %d)", data.position.x, data.position.y);

        Image imagedata = *data.image_pointer;
        BITMAP bitmap = imagedata.bitmap;

        debug("\tDIMENSIONS: (%ld, %ld)", bitmap.bmWidth, bitmap.bmHeight);
    }
}

/// @brief inserts the pointer of an imagearray to the ordered_images arr based on the layer
/// @param ptrImageDat ptr to the imagearray
void insertToDraw(ImageArray* ptrImageDat) {
    wdebug(L"Got pointer %p [%s]", ptrImageDat, ptrImageDat->name);
    double layer = ptrImageDat->layer;
    int indextoinsert = IMAGES_LOADED;

    // find a spot
    for (int i = 0; i<IMAGES_LOADED; i++) {
        ImageArray *imageatindex = MEM_IMAGES[i];
        if (layer < imageatindex->layer) {
            indextoinsert = i;
            break;
        }
    }

    // realloc if needed
    IMAGES_LOADED++;
    if (IMAGES_LOADED > MAX_IMAGES_LOADED) {
        MAX_IMAGES_LOADED = IMAGES_LOADED;
        saferealloc(MEM_IMAGES, MAX_IMAGES_LOADED * sizeof(ImageArray*));
    }
    // move everything forward
    // not putting this '- 1' cost me AN HOUR OF DEBUGGING bc of silent crashes yayyy
    for (int i = (IMAGES_LOADED - 1); i>indextoinsert; i--) {
        MEM_IMAGES[i] = MEM_IMAGES[i - 1];
    }
    MEM_IMAGES[indextoinsert] = ptrImageDat;
    wdebug(L"Inserted %s to %d.", ptrImageDat->name, indextoinsert);
}

void printdraworder() {
    debug("Full list:");
    for (int i = 0; i<IMAGES_LOADED; i++) {
        ImageArray* dat = MEM_IMAGES[i];
        wdebug(L"PTR %p [%s : %.2lf]", dat, dat->name, dat->layer);
    }
}

int getIndexInDraw(ImageArray* ptrImageDat) {
    return 0;
}

void popInDraw(ImageArray* ptrImageDat) {

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

/// @brief concatenate count number of LPCSTRs
/// @param count number of paths
/// @param ... LPWSTR paths
/// @return concatenated LPCWSTR
LPCWSTR concatPaths(int count, ...) {
    // setting as null makes wcslen freak out 
    // so we premalloc it
    LPWSTR concatenated = NULL; 
    safemalloc(concatenated, 1 * sizeof(WCHAR));
    concatenated[0] = '\0';
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        LPWSTR wstr = va_arg(args, LPWSTR);
        saferealloc(concatenated, (wcslen(concatenated) + 1 + wcslen(wstr) + 1) * sizeof(WCHAR));
        // concated + \\ + wstr + \0
        // if concatenated alr has a \ at the end
        // or if wstr alr has a \ at the start,
        // then dont add a backslash
        if (!(concatenated[wcslen(concatenated) - 1] == L'\\' || 
            wstr[0] == L'\\') &&
            wcslen(concatenated) > 0) // and str isnt empty
            wcscat(concatenated, L"\\");
        wcscat(concatenated, wstr);
    }

    va_end(args);
    return (LPCWSTR) concatenated; 
}

/// @brief compare if the extension of a path matches the extension given
/// @param path path to check
/// @param extension extension (e.g. L"bmp")
/// @return boolean int
int compareExtension(LPCWSTR path, LPWSTR extension) {
    // most of thios code is from trimPath
    size_t pathlen = wcslen(path);
    int indexofdot = -1;
    for (int i=pathlen; i>0; i--) {
        if (path[i] == L'.') { // if we went thru the entire file name
            indexofdot = i;
            break;
        }
        if (path[i] == L'\\') {
            break;
        }
    }

    if (indexofdot < 0) { // no dot
        return 0;
    }

    LPWSTR dest = NULL;
    safemalloc(dest, (pathlen - indexofdot) * sizeof(WCHAR)); 

    for (int i = 0; i < pathlen-indexofdot; i++) {
        dest[i] = path[indexofdot + 1 + i];
    }
    
    int comparison = wcscmp(dest, extension);
    free(dest); // free dat

    // why the hell does wcscmp
    // return 0 if its equal
    // ^ its for sorting and stuff
    return 0 == comparison;
}

/// @brief remove the extension name. (e.g. bmp\\sad.bmp -> sad)
/// @param path directory
/// @param modifying address of LPWSTR place to output
void trimExtension(LPCWSTR path, LPWSTR *modifying) {
    size_t pathlen = wcslen(path);
    LPWSTR dest = NULL;
    for (int i=pathlen; i>0; i--) {
        // wprintf(L"%c", path[i]);
        if (path[i] == L'.') { // dot found
            // copy everything before the dot.
            safemalloc(dest,  (i + 1)* sizeof(WCHAR));
            wcsncpy(dest, path, i);
            dest[i] = L'\0'; // wnsncpy doesnt add this
            break;
        }
        if (path[i] == L'\\') { // if we went thru the entire file name
            safemalloc(dest, (pathlen + 1) * sizeof(WCHAR));
            wcscpy(dest, path);
            break;
        }
    }
    *modifying = dest;
}

/// @brief only get the file/folder name. (e.g. graphics-testing\\bmp\\sadanim -> sadanim)
/// @param path directory
/// @param modifying address of string to place output
void trimPath(LPCWSTR path, LPWSTR* modifying) {
    size_t pathlen = wcslen(path);
    int indexofslashslash = 0;
    for (int i=pathlen; i>0; i--) {
        if (path[i] == L'\\') { // if we went thru the entire file name
            indexofslashslash = i;
            break;
        }
    }
    LPWSTR dest = NULL;
    safemalloc(dest, (pathlen - indexofslashslash) * sizeof(WCHAR)); 
    for (int i = 0; i < pathlen-indexofslashslash; i++) {
        dest[i] = path[indexofslashslash + 1 + i];
    }
    *modifying = dest;
}


/// @brief finds the index of the first occuring file with given filename
/// @param filename filename only, not path. e.g. L"image.bmp"
/// @param filearray file array from getAllChildren()
/// @param number_of_files file count from getAllChildren()
/// @return index of occurance. -1 if none found.
int getFirstOccurenceInFileArray(LPCWSTR filename, LPCWSTR* filearray, DWORD number_of_files) {
    for (int i = 0; i < (int) number_of_files; i++) {
        LPCWSTR file = filearray[i];
        LPWSTR trimmed;
        trimPath(file, &trimmed);
        if (wcscmp(trimmed, filename) == 0) {
            return i;
        }
    }
    return -1;
}

/// @brief returns full path length
/// @param dir 
/// @return full path length. already includes null terminator.
DWORD getFullPathLength(LPCWSTR dir) {
    return GetFullPathName(dir, 0, NULL, NULL);
}

//https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfullpathnamew
/// @brief returns the full path of a reltive path.
/// @param dir LPCWSTR relative dir
/// @return allocated var. dont forget to free().
LPWSTR getFullPathSafe(LPCWSTR dir) {
    DWORD pathlen = GetFullPathName(dir, 0, NULL, NULL);
    LPWSTR buffer = NULL;
    safemalloc(buffer, (pathlen + 1) * sizeof(WCHAR));
    DWORD pathlen2 = GetFullPathName(dir, pathlen, buffer, NULL); // patheln2 DOESNT include the null terminator
    // paranoia kicking in
    if (pathlen2 >= pathlen) {
        error("Pathlen discrepancy. Pathlen1: %d, Pathlen2: %d", (int) pathlen, (int) pathlen2);
    } 
    return buffer;
}

// credit: https://stackoverflow.com/questions/68215054/browse-a-whole-folder-in-c
// https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findfirstfilea
// https://learn.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-win32_find_dataa
// https://learn.microsoft.com/en-us/windows/win32/fileio/listing-the-files-in-a-directory
/// @brief gets all files and subfolders in a folder. why this shit so damn complicated
/// @param dir 
/// @param arraybuffer modifies the given buffer. pass in &arraybuffer.
/// @return the number of files in the arraybuffer
DWORD getAllChildren(LPCWSTR dir, LPCWSTR **arraybuffer) {
    LPWSTR fulldir = getFullPathSafe(dir);
    free(fulldir);
    
    LPWSTR buff = NULL;
    safemalloc(buff, (wcslen(dir) + 1 + 2) * sizeof(WCHAR)); // have to initalize it bc malloc sux
    wcscpy(buff, dir); // copy the dir
    wcscat(buff, L"\\*"); // append the wildcard to specify all files

    // necessary vars for FindFirstFile and FindNextFile
    WIN32_FIND_DATA fd;
    HANDLE currfilehandle = FindFirstFile(buff, &fd);
    int cond = currfilehandle != INVALID_HANDLE_VALUE;

    if (!cond) { // big whoopsie
        error("Failed to read first file.");
    }

    LPCWSTR *array = NULL;
    DWORD number_of_files = 0;
    LPWSTR filebuff = NULL;
    while (cond) {
        if (!isPath(fd.cFileName)) { // passing in things like "..\\folder" counts .. as a "file" so this check makes sure all the things r either folders or files.
            // if we dont add the folder before the file, getfullpathsafe will assume its in the same directory
            // as the executazble
            size_t filenamelen = (wcslen(fd.cFileName) + 1);
            saferealloc(filebuff, (wcslen(dir) + filenamelen + 5) * sizeof(WCHAR)); // reuse filebuff
            wcscpy(filebuff, dir);
            wcscat(filebuff, L"\\");
            wcscat(filebuff, fd.cFileName);

            LPWSTR fulldirname = getFullPathSafe(filebuff);             
            // add the pointer to our array
            saferealloc(array, (number_of_files+1) * sizeof(LPWSTR*));
            array[number_of_files] = fulldirname;
            number_of_files++;
        }
        cond = FindNextFile(currfilehandle, &fd); // returns 0 if failure or just done
    }
    FindClose(currfilehandle);
    *arraybuffer = array; // make the arraybuffer point to array.

    free(buff);
    free(filebuff);
    return number_of_files;
    
}

// ------- GRAPHICS RELATED FUNCTIONS ------- //

/// @brief read thru a config file and assign the necessary vars to imagearray
/// @param configfilepath 
/// @param imagedatmodifying address of the image(s) image array
void readConfig(LPCWSTR configfilepath, ImageArray *imagedatmodifying) {
    FILE *filepointer = _wfopen(configfilepath, L"rb"); // fopen() but supports WCHAR filename

    char buffer[CONFIGSIZE];
    size_t read = fread(buffer, sizeof(char), CONFIGSIZE-1, filepointer);
    buffer[read] = '\0';

    int start = 0;
    for (int i = 0; i<read; i++) {
        // just in case
        if (buffer[i] == ' ') {wwarn(L"In config file %s, a SPACE was found. Do not put spaces at all", configfilepath); error("Read the warning");}

        // this is just a parser. very annoying to think about indexes in ur head
        if ((buffer[i] == '\n') || i == (read - 1)) { // its an assignment
          for (int j = start; j<i; j++) {
                if (buffer[j] == ' ') {wwarn(L"In config file %s, a SPACE was found. Do not put spaces at all", configfilepath); error("Read the warning");}
                if (buffer[j] == '=') { // equals sign
                    // name of var
                    // start to j-1
                    char *var;
                    int varlength = j - start;
                    safemalloc(var, varlength * sizeof(char));
                    memcpy(var, buffer + start, varlength);
                    var[varlength] = '\0';

                    // value
                    // j+1 to i-1
                    int value;
                    char *strvalue;
                    int valuelength;
                    if (i == (read - 1)) { // weird why it dont work for this case...
                        valuelength = i - j;
                    } else {
                        valuelength = (i - 1) - (j + 1);
                    }
                    safemalloc(strvalue, valuelength * sizeof(char));
                    memcpy(strvalue, buffer + j + 1, valuelength);
                    strvalue[valuelength] = '\0';

                    value = atoi(strvalue);

                    // these r the config settings that r actually supportewd
                    if (0 == strcmp(var, "framerate")) {
                        imagedatmodifying->framerate = value;
                    } else if (0 == strcmp(var, "loop")) {
                        imagedatmodifying->loop = value;
                    }
                    free(var);
                    free(strvalue);
                    break;
                }
            }
            start = i+1;
        }
    }

    fclose(filepointer);
}

/// @brief loads an image to memory.
/// @param image_or_folder_name the thing to load. can be a folder (if animation) or .bmp (if standalone)
/// @param layer the draw order. 0 = highest priority
/// @param framerate if its an animation, the framerate. (tied to FRAME_RATE)
/// @return address to the ImageArray in MEM_IMAGES. just realized doing this really fucks up the layering thing. oops!
ImageArray* loadimage(LPCWSTR image_or_folder_name, double layer) {
    // if image:
    // image.bmp file

    // if folder:
    // image0.bmp
    // image1.bmp
    // ...
    // imageN.bmp
    // .config -> contains framerate and other stuff

    wdebug(L"given thing to load: %s", image_or_folder_name);

    if (!isPath(image_or_folder_name)) {
        error("Malformed/Nonexistent path given.");
    }

    int isBMP = compareExtension(image_or_folder_name, L"bmp");
    LPWSTR trimmedExtension;
    LPWSTR trimmed;
    trimExtension(image_or_folder_name, &trimmedExtension);
    trimPath(trimmedExtension, &trimmed);
    free(trimmedExtension);

    wdebug(L"path trimmed: %s", trimmed);

    // differentiating between images and animations
    ImageArray* imagedat = newHeapImageArray();
    imagedat->name = trimmed;
    imagedat->layer = layer;

    Image *images = NULL;
    LPCWSTR *files = NULL;
    DWORD nfiles;
    DWORD nimages;
    if (isBMP) { // is a BMP
        debug("loading as image");
        nfiles = 0;
        nimages = 1;
    } else if (!isBMP && !isFile(image_or_folder_name)) { // is an anim folder
        debug("loading as animation");
        
        imagedat->framecount = 0; // differentiates from bmp
        nfiles = getAllChildren(image_or_folder_name, &files);
        
        // make sure everything is valid
        for (int i = 0; i < nfiles; i++) {
            LPCWSTR file = files[i];
            if (!compareExtension(file, L"bmp") && !compareExtension(file, L"config")) {
                wwarn(L"In animation folder '%s', file '%s' is neither .bmp nor .config.", image_or_folder_name, file);
                error("See warning above.");
            }
        }
        int indexofconfig = getFirstOccurenceInFileArray(L".config", files, nfiles);
        if (indexofconfig < 0) {
            wwarn(L"No '.config' file found for animation folder:\n\t%s", image_or_folder_name);
            error("See warning above.");
        }

        nimages = nfiles-1; // everything is dandy
        readConfig(files[indexofconfig], imagedat);
        imagedat->frames = nimages;
    } else {
        wwarn(L"%s is neither a .bmp nor a folder.", image_or_folder_name);
        error("^^^");
    }

    // load in the images
    saferealloc(images, nimages * sizeof(Image));
    debug("IMAGES TO LOAD:");
    for (int i = 0; i<nimages; i++) {
        LPCWSTR name = isBMP ? image_or_folder_name : NULL;
        if (name == NULL) { // its an animation
            // gets the index. converts index to WCHAR
            LPWSTR indexWCHAR;
            int iws = i == 0 ? 2 : floor(log10(i)) + 1 + 1; // https://www.geeksforgeeks.org/dsa/program-count-digits-integer-3-different-methods/
            safemalloc(indexWCHAR, iws * sizeof(WCHAR));
            swprintf(indexWCHAR, iws, L"%d", i);

            // combine everything
            LPWSTR withProperOrder;
            int orderlen = wcslen(trimmed) + iws + 4; // iws accounts for \0 already
            safemalloc(withProperOrder, orderlen * sizeof(WCHAR));
            // trimmed + i's digits + .bmp + \0
            wcscpy(withProperOrder, trimmed);
            wcscat(withProperOrder, indexWCHAR);
            wcscat(withProperOrder, L".bmp");

            // get index of first occurance
            // we cant assume that the files r in perfect order
            int index = getFirstOccurenceInFileArray(withProperOrder, files, nfiles);
            if (index < 0) {
                wwarn(L"Missing animation frame. Expecting to find %s but couldnt find it.", withProperOrder);
                error("Read the warning");
            }
            name = files[index];

            free(withProperOrder);
            free(indexWCHAR);
        }
        wdebug(L"loading %s", name);

        // load the actual image to memory
        HBITMAP hBitmap = (HBITMAP)LoadImage(
            NULL,                   // NULL when loading from a file
            name,                   // File path
            IMAGE_BITMAP,           // Type of image to load
            0, 0,                   // Width and height (0 = use actual size)
            LR_LOADFROMFILE         // Load from disk instead of a resource
        );
        
        BITMAP bitmap; // fetch image information
        GetObject(hBitmap, sizeof(bitmap), &bitmap);

        images[i].hbitmap = hBitmap;
        images[i].bitmap = bitmap;
        wdebug(L"successfully loaded %s", name);
    }

    imagedat->image_pointer = images;
    imagedat->size = (Vector2) {.x = images->bitmap.bmWidth, .y = images->bitmap.bmHeight};

    // sorts the images by layer
    insertToDraw(imagedat);
    if (files != NULL) freeFiles(files, nfiles);
    return imagedat;
}

// ------- uMsg HANDLER FUNCTIONS ------- //

/// @brief things that happen when the window is messaged to open
void OnCreate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    debug("CREATE");
    
    // create the tiles
    LPCWSTR tilefullpath = concatPaths(2, BMPPATH, L"tilefull");
    LPCWSTR tilehollowpath = concatPaths(2, BMPPATH, L"tilehollow");
    for (int y = 0; y<SIZE; y++) {
        for (int x = 0; x<SIZE; x++) {
            ImageArray* tileimg = loadimage(((x+y)%2 == 0 ? tilefullpath : tilehollowpath), 100);
            SNAKEGAME_TILES[SIZE*y + x] = tileimg;
            // these values r from trial and error lol
            // im not gonna go do math just to perfectly
            // align everything
            tileimg->size = (Vector2) {.x = 30, .y = 30};
            tileimg->position = (Vector2) {.x = 150 + x*30, .y = 150 + y*30};
        }
    }

    printdraworder();

    // start the frame loop thread
    HANDLE hThread = (HANDLE) _beginthreadex(
        NULL,
        0,
        &thread,
        hwnd, // just pass in as is
        0,
        NULL
    );

    if (hThread == NULL) {
        error("Unable to start frame loop thread.");
    }

    CloseHandle(hThread); // dereference the thread, itll keep running in the backgrund
}

/// @brief painting on the window
/// credit: http://parallel.vub.ac.be/education/modula2/technology/Win32_tutorial/bitmaps.html
void OnPaint(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    InvalidateRect(hwnd, NULL, 0); // forces windows to include the entire window as the rcPaint param instead of only the area needing updating.

    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    // https://stackoverflow.com/questions/1541492/how-to-code-for-reducing-flicker-need-your-help-with-code-win32-vc-flicker
    HDC hdcBack = CreateCompatibleDC(hdc); // another hdc to render things onto before rendering on the screen (to remove flickering)
    
    RECT rc;
    GetClientRect(hwnd, &rc);
    int winW = rc.right - rc.left;
    int winH = rc.bottom - rc.top;
    HBITMAP hbitBack = CreateCompatibleBitmap(hdc, winW, winH);
    HBITMAP oldhbitBack = SelectObject(hdcBack, hbitBack);

    // All painting occurs here, between BeginPaint and EndPaint.
    // https://winprog.org/tutorial/bitmaps.html
    FillRect(hdcBack, &ps.rcPaint, (HBRUSH) GetStockObject(BLACK_BRUSH));
    if (IMAGES_LOADED > 0) {
        HDC mem = CreateCompatibleDC(hdcBack); // surface to draw on hdcBack
        HBITMAP old = NULL;

        for (int i = 0; i<IMAGES_LOADED; i++) {
            ImageArray *imagedat = MEM_IMAGES[i];
            if (!imagedat->enabled) continue; // skip all disabled frames
            
            // update currframe
            if (imagedat->framecount >= imagedat->framerate) {
                imagedat->framecount = 0;
                imagedat->currframe++;
                imagedat->currframe = imagedat->currframe % imagedat->frames;
            }
            
            // increment frame counter for animations
            if (imagedat->framecount > -1) {
                imagedat->framecount++;
            }
            
            Image imageframe = *(imagedat->image_pointer + imagedat->currframe);
            HBITMAP hbitmap = imageframe.hbitmap;
            BITMAP bitmap = imageframe.bitmap;
            HBITMAP status;

            if (old == NULL) {
                old = SelectObject(mem, hbitmap);
                status = old;
            } else {
                status = SelectObject(mem, hbitmap);
            }
            if (status == NULL || status == HGDI_ERROR) {
                error("select obj failed");
            }

            // https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-transparentblt
            // draw on hdcBack
            // this requires the flag -lmsimg32
            TransparentBlt(
                hdcBack,
                imagedat->position.x, imagedat->position.y,
                imagedat->size.x,
                imagedat->size.y,
                mem,
                0, 0,
                bitmap.bmWidth, bitmap.bmHeight,
                COLOR_KEY
            );
        }

        SelectObject(mem, old);
        DeleteDC(mem);
    }
    // draw hdcBack on the screen
    BitBlt(
        hdc,
        0, 0,
        winW,
        winH,
        hdcBack,
        0, 0,
        SRCCOPY // copy 
    );

    SelectObject(hdcBack, oldhbitBack);
    DeleteObject(hbitBack);
    DeleteDC(hdcBack);
    EndPaint(hwnd, &ps);
}

void OnDestroy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    debug("STOP");
    THREAD_RUNNING = 0;
    // clean up the entire MEM_IMAGES array
    for (int i = 0; i<IMAGES_LOADED; i++) {
        ImageArray *imagedat = MEM_IMAGES[i];
        wdebug(L"Freeing image %s", imagedat->name);

        for (int j = 0; j<imagedat->frames; j++) { // for each image loaded, delete HBITMAP
            Image image = *(imagedat->image_pointer + j);
            DeleteObject(image.hbitmap);
        }

        // free up mallocated things
        free(imagedat->name);
        free(imagedat->image_pointer);
        free(imagedat);
    }

    PostQuitMessage(0); // this POSTS a msg of WM_QUIT, causing the message loop to end 
}

// this is where ALL events are sent to
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        // when the window is messaged to be made.
        case WM_CREATE: 
            OnCreate(hwnd, uMsg, wParam, lParam);
            break;
            
        // handle closing the window
        case WM_DESTROY: 
            OnDestroy(hwnd, uMsg, wParam, lParam);
            break;
        
        // paint the window
        // needs -lgdi32 flag to function
        case WM_PAINT:
            OnPaint(hwnd, uMsg, wParam, lParam);
            break;

        // mouse movenent
        case WM_MOUSEMOVE:; // extra ; bc cant have a declaration at the first statement.
            MOUSE.y = (int) HIWORD(lParam);
            MOUSE.x = (int) LOWORD(lParam);
            break;
        // left mosue down
        case WM_LBUTTONDOWN:
            debug("m1 down");
            // SendMessage(hwnd, WM_PAINT, wParam, lParam);
            break;
        // left mouse button up
        case WM_LBUTTONUP: 
            debug("m1 up");
            break;
        // keyborad down
        case WM_KEYDOWN:;
            long long KEYCODE_DOWN = (long long)LOWORD(wParam);

            debug("key down, %c %lld", (char) KEYCODE_DOWN, KEYCODE_DOWN); 
            break;
        // keyborad up
        case WM_KEYUP:;
            long long KEYCODE_UP = (long long)LOWORD(wParam);  // 64 bits

            debug("key up, %c %d", (char) KEYCODE_UP, (int) KEYCODE_UP); 
            if (KEYCODE_UP == VK_ESCAPE) { // escape key
                PostMessage(hwnd, WM_DESTROY, 0, 0);
            }
            break;
        default:
            // default behavior for unhandled uMsg
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

// this is windows' int main()
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // just check if the .c file is in the same path as the executable
    // and if the bmp exists
    if (!isFile(SOURCEPATH)) {
        error("EXECUTABLE FILE NOT IN THE SAME DIRECTORY AS THE SOURCE FILE. \nTHE EXECUTABLE FILE HAS TO BE IN THE CS11 MP1 FOLDER TO PROPERLY FUNCTION.");
    }
    if (!isPath(BMPPATH)) {
        error("EXECUTABLE FILE NOT IN THE SAME DIRECTORY AS bmp FOLDER. \nTHE EXECUTABLE FILE HAS TO BE IN THE CS11 MP1 FOLDER TO PROPERLY FUNCTION.");
    }
    debug("Verified required files\n");

    // initialize the prerequisites for creating a window process
    LPCWSTR CLASSNAME = L"peepoo"; // this is what appears in task manager
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
        L"Snake Game", // window name
        // disable resizing and minimizing/maximizing
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,

        // pos (x, y) and size (x, y) in pixels
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOWWIDTH, WINDOWHEIGHT,

        // some other stuff
        NULL, // parent
        NULL, // menu
        hInstance, // the hinstance hting
        NULL    // additional app data
    );

    // show it
    ShowWindow(hwnd, nCmdShow);


    // message loop. this is the feedback loop
    // for messages. the messages are handled by the
    // WindowsProc function. 
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg); // calls WindowsProc
    }

    return 0;
}