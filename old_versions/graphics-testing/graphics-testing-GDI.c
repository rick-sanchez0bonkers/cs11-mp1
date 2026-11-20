// THIS FILE IS FOR CREATING THE BUILDING BLOCKS
// OF MAKING A FULLY FLEDGED GRAPHICS-FRIENDY WINDOWS
// things added:
// windows app essentials (WM_CREATE, WM_DESTROY, & WM_PAINT)
// keyboard & mouse input support
// rendering support
// .bmp support

// time taken:
// 5 days (this shit complicated to learn from the ground up)


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

#ifndef UNICODE // adds support for unicode characters. Adds the L prefix for strings which can specify wide character type.
#define UNICODE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h> // variadic functions support
#include <windows.h>
#include <process.h>
#include <math.h>
#include <wchar.h> // https://en.cppreference.com/c/header/wchar for wide strings since file paths r wchar_t :(
#include <signal.h> // for aborting

// #include <d2d1.h> // REQUIRES A MODERN C COMPILER. i had a x32 version (MinGW.org GCC-6.3.0-1) 6.3.0 i had to uninstgall then install x64. followed https://code.visualstudio.com/docs/cpp/config-mingw

#define FRAME_RATE 60
#define DEBUG_MODE 1
#define CONFIGSIZE 1024*2
#define COLOR_KEY RGB(255,0,255) // this specific color is drawn as completely transparent.

#define wstringify(x) L#x

#define debug(fmt, ...) if (DEBUG_MODE) printf("\033[34m[%s:%d]\033[0m : " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define wdebug(fmt, ...) if (DEBUG_MODE) wprintf(L"\033[34m[%d]\033[0m : " fmt "\n", __LINE__, ##__VA_ARGS__) // omitting file name cuz no idea how to convert it to wchar_t*
#define wwarn(fmt, ...) wprintf(L"\033[43m[%d]\033[0m : " fmt "\n", __LINE__, ##__VA_ARGS__) // omitting file name cuz no idea how to convert it to wchar_t*
#define error(fmt, ...) printf("\033[41m[%s:%d]\033[0m : " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); raise(SIGABRT)
#define print(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#define wprint(fmt, ...) wprintf(fmt L"\n", ##__VA_ARGS__)

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
    // debug("Successful realloc. Pointer address: %p, number of bytes: %d", pointer, (int) (size));

// some types to make my life easier
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
    int loop; // if it loops or not. boolean int
    int enabled; // if it renders or not. boolean int
    double opacity; // 255 (opaque) to 0 (transparent)
    double rotation; // in degrees
    Vector2 position; // where on the screen to render
    Vector2 size; // what size to render the image?
    LPCWSTR name; // the file name. if its an animation, gets the name of the first file and removes the number at the end (e.g. sad0.bmp -> sad)
    Image *image_pointer; // pointer to the array that contains the loaded images [one or more].
} ImageArray;

// global vars
long long unsigned int FRAME_LIFETIME = 0; // counts the number of frames the game has been thru
ImageArray* MEM_IMAGES; // this stores ALL of the loaded images. NOT SORTED BY LAYER
// this is essentially an array of pointers pointing to things in MEM_IMAGES
// and actually orders the pointers by layer.
// loadimage() returns the address where an ImageArray is stored inside MEM_IMAGES
// so sorting MEM_IMAGES will completely jumble all variables that rely on loadimage()
// hence we need a new array to order them for the renderer to draw in proper order
ImageArray* ORDERED_IMAGES[];
size_t IMAGES_LOADED = 0;
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
ImageArray *animdata = NULL;
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
        if (animdata != NULL) {
            Vector2 newpos = {.x = 250, .y = 250};
            newpos.x += 50 * sin(FRAME_LIFETIME/3.1415926 * 0.5);
            newpos.y += 50 * cos(FRAME_LIFETIME/3.1415926 * 0.5);
            animdata->position = newpos;
        }

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
    // typedef struct {
    //     double layer; // draw order. used to sort the array. lower layer = drawn first
    //     int frames; // number of images
    //     int framecount; // how much frames have elapsed?
    //     int framerate; // how much frames to wait before the next frame gets swapped
    //     int currframe; // what image are we on
    //     int loop; // if it loops or not. boolean int
    //     int enabled; // if it renders or not. boolean int
    //     double opacity; // 255 (opaque) to 0 (transparent)
    //     double rotation; // in degrees
    //     Vector2 position; // where on the screen to render
    //     LPCWSTR name; // the file name. if its an animation, gets the name of the first file and removes the number at the end (e.g. sad0.bmp -> sad)
    //     Image *image_pointer; // pointer to the array that contains the loaded images [one or more].
    // } ImageArray;

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

/// @brief prints the images stored in memory
void printmemimages() {
    printf("Number of images: %llu\n", IMAGES_LOADED);
    for (int i = 0; i < IMAGES_LOADED; i++) {
        print("IMAGE %d", i);
        ImageArray data = MEM_IMAGES[i];
        wprint(L"\tNAME: %s", data.name);
        print("\tLAYER: %f", data.layer);
        print("\tFRAMES: %d", data.frames);
        print("\tFRAMECOUNT: %d", data.framecount);
        print("\tFRAMERATE: %d", data.framerate);
        print("\tCURRFRAME: %d", data.currframe);
        print("\tLOOP: %d", data.loop);
        print("\tENABLED: %d", data.enabled);
        print("\tOPACITY: %f", data.opacity);
        print("\tROTATION: %f", data.rotation);
        print("\tPOSITION: (%d, %d)", data.position.x, data.position.y);

        Image imagedata = *data.image_pointer;
        BITMAP bitmap = imagedata.bitmap;

        print("\tDIMENSIONS: (%ld, %ld)", bitmap.bmWidth, bitmap.bmHeight);
    }
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

LPCWSTR concatPaths(int count, ...) {
    // setting as null makes wcslen freak out 
    // so we premalloc it
    LPWSTR concatenated = NULL; 
    safemalloc(concatenated, 1 * sizeof(WCHAR));
    concatenated[0] = '\0';
    va_list args;
    va_start(args, count);
    debug("Got %d args", count);
    for (int i = 0; i < count; i++) {
        LPWSTR wstr = va_arg(args, LPWSTR);
        wdebug(L"Got string: %s",wstr);
        wdebug(L"concat len: %d",wcslen(concatenated));
        wdebug(L"str len: %d",wcslen(wstr));
        wdebug(L"full len: %d", wcslen(concatenated) + 1 + wcslen(wstr) + 1);
        saferealloc(concatenated, (wcslen(concatenated) + 1 + wcslen(wstr) + 1) * sizeof(WCHAR));
        wdebug(L"reallocated");
        // concated + \\ + wstr
        // if concatenated alr has a \\ at the end
        // or if wstr alr has a \\ at the start,
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

    // debug("Calculated space: %d", pathlen - indexofdot);
    LPWSTR dest = NULL;
    // properly calculated the string size (with \0 included)
    safemalloc(dest, (pathlen - indexofdot) * sizeof(WCHAR)); 

    for (int i = 0; i < pathlen-indexofdot; i++) {
        dest[i] = path[indexofdot + 1 + i];
        // wdebug(L"CHAR TO COPYING: %c\n", path[indexofdot + 1 + i]);
    }
    // does the actual comparison lol. getting the extension takes like 50 billion lines
    // but god bless wchar.h
    int comparison = wcscmp(dest, extension);
    free(dest); // free dat shit

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
            safemalloc(dest, (i) * sizeof(WCHAR));
            wcsncpy(dest, path, i);
            wdebug(L"Path coped: %s", dest);
            wdebug(L"char at index: %c", path[i]);
            // for some reason, passing in image_or_folder_name (the heap copy) completely fucks wcsncpy up.
            // even though path[i] correctly points to a '.'
            // but adding this does fix it
            // though i think the mallocated space is too much 
            dest[i] = L'\0';
            break;
        }
        if (path[i] == L'\\') { // if we went thru the entire file name
            safemalloc(dest, (pathlen + 1) * sizeof(WCHAR));
            wcscpy(dest, path);
            break;
        }
    }
    // debug("dest: %p", dest);
    *modifying = dest;
    // debug("modifying: %p", modifying);
    // wdebug(L"%s", dest);
}

/// @brief only get the file/folder name. (e.g. graphics-testing\\bmp\\sadanim -> sadanim)
/// @param path directory
/// @param modifying address of string to place output
void trimPath(LPCWSTR path, LPWSTR* modifying) {
    // debug("trim path");
    size_t pathlen = wcslen(path);
    int indexofslashslash = 0;
    for (int i=pathlen; i>0; i--) {
        if (path[i] == L'\\') { // if we went thru the entire file name
            indexofslashslash = i;
            break;
        }
    }

    LPWSTR dest = NULL;
    // properly calculated the string size (with \0 included)
    safemalloc(dest, (pathlen - indexofslashslash) * sizeof(WCHAR)); 

    // manually setting the chars because
    // idk if theres a wchar function
    // to only copy a specific section of 
    // a string.
    // i miss string.sub() in lua...
    // or even slicing in python...
    // i hate how tedious strings are in C god damn it
    for (int i = 0; i < pathlen-indexofslashslash; i++) {
        dest[i] = path[indexofslashslash + 1 + i];
        // wdebug(L"CHAR TO COPYING: %c\n", path[indexofslashslash + 1 + i]);
    }

    // debug("dest: %p", dest);
    *modifying = dest;
    // debug("modifying: %p", modifying);
    // wdebug(L"%s", dest);
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
    // wdebug(L"Given dir: %s", dir);
    DWORD pathlen = GetFullPathName(dir, 0, NULL, NULL);
    // wdebug("Got pathlen %lu", pathlen);

    LPWSTR buffer = NULL;
    safemalloc(buffer, (pathlen + 1) * sizeof(WCHAR));
    DWORD pathlen2 = GetFullPathName(dir, pathlen, buffer, NULL); // patheln2 DOESNT include the null terminator
    // paranoia kicking in
    if (pathlen2 >= pathlen) {
        error("Pathlen discrepancy. Pathlen1: %d, Pathlen2: %d", (int) pathlen, (int) pathlen2);
    } 
    // wdebug(L"Buffer: %s", buffer);
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
    wdebug(L"Get all children in dir %s", fulldir);
    free(fulldir);

    // // WCHAR buff[wcslen(dir) + 1] = NULL; // have to initalize it bc malloc sux
    
    LPWSTR buff = NULL;
    safemalloc(buff, (wcslen(dir) + 1 + 2) * sizeof(WCHAR)); // have to initalize it bc malloc sux
    wcscpy(buff, dir); // copy the dir
    wcscat(buff, L"\\*"); // append this thing, no idea what it does or means.

    // wprintf(L"Buffer: %s\n", buff);

    // necessary vars for FindFirstFile and FindNextFile
    WIN32_FIND_DATA fd;
    HANDLE currfilehandle = FindFirstFile(buff, &fd);
    int cond = currfilehandle != INVALID_HANDLE_VALUE;

    if (!cond) { // big whoopsie
        error("Failed to read first file.");
    }

    // shit we want
    LPCWSTR *array = NULL;
    DWORD number_of_files = 0;

    LPWSTR filebuff = NULL;

    while (cond) {
        if (!isPath(fd.cFileName)) { // passing in things like "..\\folder" counts .. as a "file" so this check makes sure all the things r either folders or files.
            wdebug(L"Found file: %s", fd.cFileName);
            // if we dont add the folder before the file, getfullpathsafe will assume its in the same directory
            // as the executazble
            size_t filenamelen = (wcslen(fd.cFileName) + 1);
            saferealloc(filebuff, (wcslen(dir) + filenamelen + 5) * sizeof(WCHAR)); // reuse filebuff
            wcscpy(filebuff, dir);
            wcscat(filebuff, L"\\");
            wcscat(filebuff, fd.cFileName);

            LPWSTR fulldirname = getFullPathSafe(filebuff); 
            wdebug(L"Full dir: %s", fulldirname);
            
            // add the pointer to our array
            saferealloc(array, (number_of_files+1) * sizeof(LPWSTR*));
            array[number_of_files] = fulldirname;
            number_of_files++;
        }
        cond = FindNextFile(currfilehandle, &fd); // returns 0 if failure or just done
    }
    FindClose(currfilehandle);

    debug("Buffer:");
    for (int i=0; i<number_of_files; i++) {
        LPCWSTR file = array[i];
        wdebug(L"%s", file);
    }

    *arraybuffer = array; // make the arraybuffer point to array.

    debug("Total number of files: %d", (int) number_of_files);
    free(buff);
    free(filebuff);
    return number_of_files;
    
}

// ------- END OF PATH FUNCTIONS ------- //

// ------- GRAPHICS RELATED FUNCTIONS ------- //

/// @brief read thru a config file and assign the necessary vars to imagearray
/// @param configfilepath 
/// @param imagedatmodifying address of the image(s) image array
void readConfig(LPCWSTR configfilepath, ImageArray *imagedatmodifying) {
    FILE *filepointer = _wfopen(configfilepath, L"rb");

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

                    debug("Got variable: %s", var);
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
                    // debug("vlen: %d", valuelength);;
                    // debug("i: %d", i);
                    // debug("j: %d", j);
                    // debug("i-1: %d", i-1);
                    // debug("j+1: %d", j+1);
                    safemalloc(strvalue, valuelength * sizeof(char));
                    memcpy(strvalue, buffer + j + 1, valuelength);
                    strvalue[valuelength] = '\0';

                    value = atoi(strvalue);
                    debug("Got value: %s [%d]", strvalue, value);

                    // switches cant have char* so just a big hardcoded if else if
                    // strcmp my beloved
                    if (0 == strcmp(var, "framerate")) {
                        debug("set framerate to %d", value);
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
    // .config -> contains framerate and other stuff (if im gonna add any other stuff)

    // create a copy of image_or_folder_name
    LPWSTR copy = NULL;
    safemalloc(copy, (wcslen(image_or_folder_name) + 1) * sizeof(WCHAR));
    wcscpy(copy, image_or_folder_name);
    image_or_folder_name = copy;

    wdebug(L"given thing to load: %s", image_or_folder_name);
    wdebug(L"is file? %d", isFile(image_or_folder_name));
    wdebug(L"is path? %d", isPath(image_or_folder_name));

    if (!isPath(image_or_folder_name)) {
        error("Malformed path given.");
    }

    int isBMP = compareExtension(image_or_folder_name, L"bmp");
    LPWSTR trimmedExtension; // cant just use 1 var since causes a leak
    LPWSTR trimmed;
    trimExtension(image_or_folder_name, &trimmedExtension);
    trimPath(trimmedExtension, &trimmed);
    free(trimmedExtension);

    wdebug(L"path trimmed: %s", trimmed);

    // differentiating between images and animations
    ImageArray imagedat = newImageArray();
    imagedat.name = trimmed;
    imagedat.layer = layer;

    Image *images = NULL;
    LPCWSTR *files;
    DWORD nfiles;
    DWORD nimages;
    if (isBMP) { // is a BMP
        debug("loading as image");
        safemalloc(files, sizeof(LPCWSTR))
        files[0] = image_or_folder_name;
        nfiles = 1;
        nimages = 1;
    } else if (!isBMP && !isFile(image_or_folder_name)) { // not a bmp and is a folder
        debug("loading as animation");
        // looad images
        imagedat.framecount = 0;
        nfiles = getAllChildren(image_or_folder_name, &files);
        
        // make sure everything is valid
        for (int i = 0; i < nfiles; i++) {
            LPCWSTR file = files[i];
            if (!compareExtension(file, L"bmp") && !compareExtension(file, L"config")) { // neither .bmp or .config
                wwarn(L"In animation folder '%s', file '%s' is neither .bmp nor .config.", image_or_folder_name, file);
                error("See warning above.");
            }
        }
        nimages = nfiles-1; // assuming everything is dandy
        int indexofconfig = getFirstOccurenceInFileArray(L".config", files, nfiles);

        if (indexofconfig < 0) {
            wwarn(L"No '.config' file found for animation folder:\n\t%s", image_or_folder_name);
            error("See warning above.");
        }

        debug("Index of .config: %d", indexofconfig);
        readConfig(files[indexofconfig], &imagedat);
        imagedat.frames = nimages;
    } else {
        // cant pass in the file name bc the file names r wchar_t* while
        // error only supports char*
        error("A file within the folder bitmaps folder is neither a .bmp nor a folder (animation).\nOffending file name cant be displayed (due to poor programming lol), find it yourself.");
    }

    // give space
    saferealloc(images, nimages * sizeof(Image));
    debug("IMAGES TO LOAD:");
    for (int i = 0; i<nimages; i++) {
        LPCWSTR name = isBMP ? image_or_folder_name : NULL;
        if (name == NULL) { // its an animation
            // gets the index. converts index to WCHAR
            LPWSTR indexWCHAR;
            int iws = i/10 + 2;
            safemalloc(indexWCHAR, iws * sizeof(WCHAR));
            swprintf(indexWCHAR, iws, L"%d", i);
            debug("iws: %d", iws);

            // combine everything
            LPWSTR withProperOrder;
            int orderlen = wcslen(trimmed) + iws + 4; // iws accounts for \0 already
            safemalloc(withProperOrder, orderlen * sizeof(WCHAR));
            debug("ORDERLEN: %d", orderlen);
            // trimmed + i's digits + .bmp + \0
            wcscpy(withProperOrder, trimmed);
            wcscat(withProperOrder, indexWCHAR);
            wcscat(withProperOrder, L".bmp");
            // withProperOrder[orderlen] = '\0';
            wdebug(L"%s", withProperOrder);

            // get index of first occurance
            // all of this bullshit because we cant assume that the files r in perfect order
            // at least this makes sure that we are in perfect order
            int index = getFirstOccurenceInFileArray(withProperOrder, files, nfiles);
            if (index < 0) {
                wwarn(L"Missing animation frame. Expecting to find %s but couldnt find it.", withProperOrder);
                error("Read the warning");
            }
            debug("index %d", index);
            name = files[index];

            free(withProperOrder);
            free(indexWCHAR);
        }
        wdebug(L"%s", name);


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
        // printf("Size X: %ld\n", bitmap.bmWidth);
        // printf("Size Y: %ld\n", bitmap.bmHeight);

        images[i].hbitmap = hBitmap;
        images[i].bitmap = bitmap;
    }

    imagedat.image_pointer = images;
    imagedat.size = (Vector2) {.x = images->bitmap.bmWidth, .y = images->bitmap.bmHeight};

    // p.s. this doesnt sort the array by layer yet :(
    saferealloc(MEM_IMAGES, (IMAGES_LOADED + 1) * sizeof(ImageArray));
    MEM_IMAGES[IMAGES_LOADED] = imagedat;
    // IMAGES_LOADED++;

    freeFiles(files, nfiles);
    return &MEM_IMAGES[IMAGES_LOADED++];
}

/// @brief things that happen when the window is messaged to open
void OnCreate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // debug("CREATE");
    // looad images
    // LPCWSTR *files;
    // DWORD nfiles = getAllChildren(bitmapsdir, &files);
    // for (int i = 0; i < (int) nfiles; i++) {
    //     loadimage(files[i], 0);
    // }
    // freeFiles(files, nfiles); // clean up (safe since we make a heap copy of image_or_folder_name)

    LPCWSTR anim = concatPaths(2, bitmapsdir, L"chomp");
    wdebug(L"concatenated: %s", anim);
    animdata = loadimage(anim, 0);
    // animdata->position = (Vector2) {.x = 250, .y = 250};
    animdata->size = (Vector2) {.x = 300, .y = 300};
    printmemimages();

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
    // printf("painting, %d, %d\n", MOUSE.x, MOUSE.y);
    InvalidateRect(hwnd, NULL, 0); // forces windows to include the entire window as the rcPaint param instead of only the area needing updating.

    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    // // All painting occurs here, between BeginPaint and EndPaint.
    HBRUSH black = CreateSolidBrush(RGB(0,0,0)); 
    // HBRUSH blue = CreateSolidBrush(RGB(0,0,255)); 

    // // FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW-1));
    FillRect(hdc, &ps.rcPaint, black);

    // SelectObject(hdc, blue);
    // // problem with this is, im pretty sure this uses the CPU to draw
    // // and also no antialiasing
    // Ellipse(hdc, MOUSE.x, MOUSE.y, MOUSE.x + 100, MOUSE.y + 100);

    DeleteObject(black);
    // DeleteObject(blue);

    // https://winprog.org/tutorial/bitmaps.html
    if (IMAGES_LOADED > 0) {
        HDC mem = CreateCompatibleDC(hdc);
        HBITMAP old = NULL;

        for (int i = 0; i<IMAGES_LOADED; i++) {
            ImageArray *imagedat = &MEM_IMAGES[i]; // not getting as a pointer only gives us a shallow copy :/
            if (!imagedat->enabled) continue; // skip all disabled frames
            
            // update currframe
            if (imagedat->framecount >= imagedat->framerate) {
                imagedat->framecount = 0;


                imagedat->currframe++;
                
                // if ((imagedat->currframe == imagedat->frames - 1) && !imagedat->loop) { // if it dont loop
                //     imagedat->framecount = -1;
                // } else {
                //     imagedat->currframe = imagedat->currframe % imagedat->frames;
                // }
                imagedat->currframe = imagedat->currframe % imagedat->frames;
            }
            
            // increment frame counter for animations
            if (imagedat->framecount > -1) {
                imagedat->framecount++;
            }
            
            // debug("Framecount: %d, frames: %d, currframe: %d", imagedat->framecount, imagedat->frames, imagedat->currframe);
            
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

                //https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-transparentblt
                // this requires the flag -lmsimg32
                // BitBlt(//
                TransparentBlt(
                    hdc,            // Destination DC
                    imagedat->position.x, imagedat->position.y,           // Destination X, Y coordinates
                    imagedat->size.x,     // Width to draw
                    imagedat->size.y,    // Height to draw
                    mem,         // Source DC
                    0, 0,           // Source X, Y coordinates
                    bitmap.bmWidth, bitmap.bmHeight, // source rect
                    COLOR_KEY // the color to key out
                );

            // wprintf(L"NAME: %s\n", sadarr.name);
            // Image image = *imagedat.image_pointer;
            // HBITMAP sadhbit = sadimage.hbitmap;
            // BITMAP sadbit = sadimage.bitmap;

            // old = SelectObject(mem, sadhbit);

            // if (old == NULL || old == HGDI_ERROR) {
            //     error("select obj failed");
            // }

            // BitBlt(
            //     hdc,            // Destination DC
            //     MOUSE.x, MOUSE.y,           // Destination X, Y coordinates
            //     sadbit.bmWidth,     // Width to draw
            //     sadbit.bmHeight,    // Height to draw
            //     mem,         // Source DC
            //     0, 0,           // Source X, Y coordinates
            //     SRCCOPY         // Raster operation (direct copy)
            // );
        }


        SelectObject(mem, old);
        DeleteDC(mem);
    }

    EndPaint(hwnd, &ps);
}

void OnDestroy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    debug("STOP");
    THREAD_RUNNING = 0;
    // clean up the entire MEM_IMAGES array
    for (int i = 0; i<IMAGES_LOADED; i++) {
        ImageArray imagedat = MEM_IMAGES[i];
        wdebug(L"Freeing image %s", imagedat.name);

        for (int j = 0; j<imagedat.frames; j++) { // for each image loaded, delete HBITMAP
            Image image = *(imagedat.image_pointer + j);
            DeleteObject(image.hbitmap);
        }

        // free up mallocated things
        free(imagedat.name);
        free(imagedat.image_pointer);
    }


    PostQuitMessage(0); // this POSTS a msg of WM_QUIT, causing the message loop to end 
}

// this is where ALL events are sent to
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: OnCreate(hwnd, uMsg, wParam, lParam); break; // when the window is messaged to be made.
            


        // quit the window. not having this means the window never really closes even if you click X.
        // this is callled if the window is QUITTING from any reason (X button, alt F4, task manager, etc.)
        // DestroyWindow() sends the WM_DESTROY message.
        case WM_DESTROY: 
            OnDestroy(hwnd, uMsg, wParam, lParam);
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
            OnPaint(hwnd, uMsg, wParam, lParam);
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