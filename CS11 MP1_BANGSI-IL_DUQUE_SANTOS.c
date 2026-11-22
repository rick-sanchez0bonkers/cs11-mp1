/*   -   -   -   -   -   -   -   -   -   -   -   -   */
/*      BS COMPUTER SCIENCE PROJECT -- SNAKE GAME    */
/*      THIS IS A GRAPHICS IMPLEMENTATION OF THE     */
/*      SNAKE GAME USING <windows.h>.                */
/*      ONLY WINDOWS COMPATIBLE                      */
/*   -   -   -   -   -   -   -   -   -   -   -   -   */
/*-------------------------------------------------------
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
-------------------------------------------------------*/

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
#define WINDOWWIDTH 750          // window width
#define WINDOWHEIGHT 750         // window height

#define COLOR_KEY RGB(255,0,255) // this specific color is drawn as completely transparent.
#define BACKGROUND_COLOR RGB(27, 51, 47)    // background color of the screen

#define BMPPATH     L"bmp"
#define SOURCEPATH  L"CS11 MP1_BANGSI-IL_DUQUE_SANTOS.c"

// file paths for images used in the game
// no .bmp = animation folder
// has .bmp = image
// animation and .bmp can be freely swapped, nothing will
// break (as long as the file exists)
//        num. of paths to combine vv  file name vvvvvvvvvv
#define TILEFULLPATH    concatPaths(3, BMPPATH, L"snakegame", L"tilefull.bmp")
#define TILEHOLLOWPATH  concatPaths(3, BMPPATH, L"snakegame", L"tilehollow.bmp")
#define WALLPATH        concatPaths(3, BMPPATH, L"snakegame", L"wall.bmp")
#define BLOCKPATH        concatPaths(3, BMPPATH, L"snakegame", L"block.bmp")
#define FOODPATH        concatPaths(3, BMPPATH, L"snakegame", L"apple.bmp")
#define PORTALPATH      concatPaths(3, BMPPATH, L"snakegame", L"portal")
#define DIEDPATH        concatPaths(3, BMPPATH, L"snakegame", L"died")
#define SCOREPATH       concatPaths(3, BMPPATH, L"snakegame", L"score")
#define LOSERPATH       concatPaths(3, BMPPATH, L"snakegame", L"youdied")
#define WINNERPATH       concatPaths(3, BMPPATH, L"snakegame", L"youwin")

// each cardinal direction
#define SNAKEHEADPATH   concatPaths(3, BMPPATH, L"snakegame", L"snakehead.bmp")
#define SNAKEBODYPATH // unused rn
#define SNAKETAILPATH // unused rn

// numeral paths, so tedious...
#define NUMERALSPACING  3 // how far apart each number is in pixels
#define ZEROPATH         concatPaths(3, BMPPATH, L"numerals", L"zero")
#define ONEPATH         concatPaths(3, BMPPATH, L"numerals", L"one")
#define TWOPATH         concatPaths(3, BMPPATH, L"numerals", L"two")
#define THREEPATH         concatPaths(3, BMPPATH, L"numerals", L"three")
#define FOURPATH         concatPaths(3, BMPPATH, L"numerals", L"four")
#define FIVEPATH         concatPaths(3, BMPPATH, L"numerals", L"five")
#define SIXPATH         concatPaths(3, BMPPATH, L"numerals", L"six")
#define SEVENPATH         concatPaths(3, BMPPATH, L"numerals", L"seven")
#define EIGHTPATH         concatPaths(3, BMPPATH, L"numerals", L"eight")
#define NINEPATH         concatPaths(3, BMPPATH, L"numerals", L"nine")

// main menu paths
#define TITLEPATH       concatPaths(3, BMPPATH, L"mainmenu", L"title")
#define HTPPATH         concatPaths(3, BMPPATH, L"mainmenu", L"howtoplay")
#define PLAYBTNPATH     concatPaths(4, BMPPATH, L"mainmenu", L"buttons", L"play")
#define INSTRUCTIONSBTNPATH   concatPaths(4, BMPPATH, L"mainmenu", L"buttons", L"instructions")
#define EXITBTNPATH     concatPaths(4, BMPPATH, L"mainmenu", L"buttons", L"exit") // my dumbass thought it was "QUIT" instead of "EXIT"
#define CLOSEBTNPATH       concatPaths(4, BMPPATH, L"mainmenu", L"buttons", L"close")

// options paths
#define DIFFICULTYPROMPTPATH  concatPaths(3, BMPPATH, L"options", L"difficulty")
#define FOODPROMPTPATH    concatPaths(3, BMPPATH, L"options", L"food")
#define BLOCKSPROMPTPATH  concatPaths(3, BMPPATH, L"options", L"blocks")
#define EMPTYFIELDPATH    concatPaths(3, BMPPATH, L"options", L"emptyfield")
#define DIFFICULTBTNPATH  concatPaths(4, BMPPATH, L"options", L"buttons", L"difficult")
#define EASYBTNPATH  concatPaths(4, BMPPATH, L"options", L"buttons", L"easy")
#define CONFIRMBTNPATH  concatPaths(4, BMPPATH, L"options", L"buttons", L"confirm")
#define UPBTNPATH  concatPaths(4, BMPPATH, L"options", L"buttons", L"up")
#define DOWNBTNPATH  concatPaths(4, BMPPATH, L"options", L"buttons", L"down")


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
// these ansi escape codes dont really work in
// powershell since we gotta manually enable it
// but hey they look cool in the vs code terminal
#define debug(fmt, ...) if (DEBUG_MODE) printf("\033[34m[%d]\033[0m : " fmt "\n", __LINE__, ##__VA_ARGS__)      // prints blue, DEBUG_MODE only
#define wdebug(fmt, ...) if (DEBUG_MODE) wprintf(L"\033[34m[%d]\033[0m : " fmt "\n", __LINE__, ##__VA_ARGS__)   // WCHAR support
#define warn(fmt, ...) printf("\033[43m[%d]\033[0m : " fmt "\n", __LINE__, ##__VA_ARGS__)                       // prints yellow
#define wwarn(fmt, ...) wprintf(L"\033[43m[%d]\033[0m : " fmt "\n", __LINE__, ##__VA_ARGS__)                    // WCHAR support
#define error(fmt, ...) printf("\033[41m[%s:%d]\033[0m : " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); raise(SIGABRT)      // for raising exceptions
#define werror(fmt, ...) wprintf(L"\033[41m[%d]\033[0m : " fmt "\n", __LINE__, ##__VA_ARGS__); raise(SIGABRT)      // WCHAR support
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

// for objects that are only ever loaded once
#define varloadimage(var, path, layer) \
    if (var == NULL) { \
        var = loadimage(path, layer); \
    }

// this is really long and is repeated so im just
// making it a macro. this is basically just checking
// if a given button is being hovered over and adds
// a neat highlight effect with debounce
#define hoverlogic(BUTTON) \
    if (BUTTON !=NULL) { \
        if (BUTTON->enabled && \
            MOUSE.x > BUTTON->position.x && \
            MOUSE.y > BUTTON->position.y && \
            MOUSE.x < BUTTON->position.x + BUTTON->size.x && \
            MOUSE.y < BUTTON->position.y + BUTTON->size.y \
        ) { \
            if (!BUTTON##_HOVERED) { \
                BUTTON##_HOVERED = 1; \
                BUTTON->size = v2add( \
                    BUTTON->size, \
                    v2(10, 10) \
                ); \
                BUTTON->position = v2add( \
                    BUTTON->position, \
                    v2(-5, -5) \
                ); \
            } \
            BUTTON##_HOVERED = 1;\
        } else if (BUTTON##_HOVERED == 1) {\
            BUTTON##_HOVERED = 0;\
            BUTTON->size = v2add(\
                BUTTON->size,\
                v2(-10, -10)\
            );\
            BUTTON->position = v2add(\
                BUTTON->position,\
                v2(5, 5)\
            );\
        }\
    }
    

// macros r so useful. call the necessary
// button function if a button is pressed
#define buttonlogic(BUTTON, hwnd)\
    if (BUTTON##_HOVERED) {\
        BUTTON##_FUNC(hwnd);\
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
    LPWSTR name; // the file name. if its an animation, gets the name of the folder
    Image *image_pointer; // pointer to the array that contains the loaded images [one or more].
} ImageArray;

// -- CONSTANTS -- //
const Vector2 UP = {.x = 0, .y = -1};
const Vector2 DOWN = {.x = 0, .y = 1};
const Vector2 LEFT = {.x = -1, .y = 0};
const Vector2 RIGHT = {.x = 1, .y = 0};

// -- GLOBAL VARS -- //
// GRAPHICS
long long unsigned int FRAME_LIFETIME = 0; // counts the number of frames the game has been thru
ImageArray** MEM_IMAGES; // this stores ALL of the loaded image's pointers.
size_t IMAGES_LOADED = 0;
size_t MAX_IMAGES_LOADED = 0;

int THREAD_RUNNING = 1; // set to 0 to kill the thread
int THREAD_ENABLED = 1; // set to 0 to disable the thread without killing it
Vector2 MOUSE = {.x = 0, .y = 0};

// cool spiral pattern
int ANIMATE_BOARD_DISAPPEAR = 0;
Vector2 pos = {.x = 0, .y = 0};
Vector2 stepdir = {.x = 1, .y = 0};
int turns = 0;
int steps = SIZE-1;
int stepsTaken = 0;

// GAME SETTINGS 
int DIFFICULTY = EASY;
int NUM_OF_FOOD = MAX_FOOD;
int NUM_OF_BLOCK = MAX_BLOCKS;

// GAME STATES
int GAME_OVER = 1;      // snake game isnt running
int RETURN_TO_MENU = 1; // not implemented yet
int SCORE = 0;          // in game score
int CURR_FOOD = MAX_FOOD; // current amount of food
int WINNER = 0; // if snake won or not

// -- GAME OBJECTS -- //
// snake game 
ImageArray* SNAKEGAME_BACKGROUND[SIZE][SIZE]; // contains the tile images.
ImageArray* SNAKEGAME_FOREGROUND[SIZE][SIZE]; // contains anything thats drawn above the tiles.
ImageArray* DIEDINDICATOR = NULL;
ImageArray* WINNERCARD = NULL;
ImageArray* LOSERCARD = NULL;
ImageArray* SCORECARD = NULL;

// numerals
ImageArray* NUMERALS[100]; // i dont feel like reallocing
size_t NUMERALS_LOADED = 0;

// why the hell does C not support OOP this is such an ugly
// way to handle this bruh
// all textures are initialized by MAIN_MENU

// main menu
ImageArray* TITLECARD = NULL;
ImageArray* HOWTOPLAYCARD = NULL;
ImageArray* PLAYBUTTON = NULL;
int PLAYBUTTON_HOVERED = 0;
ImageArray* INSTRUCTIONSBUTTON = NULL;
int INSTRUCTIONSBUTTON_HOVERED = 0;
ImageArray* EXITBUTTON = NULL;
int EXITBUTTON_HOVERED = 0;
ImageArray* CLOSEINSTBUTTON = NULL;
int CLOSEINSTBUTTON_HOVERED = 0;

// options menu
ImageArray* DIFFICULTYPROMPT = NULL;
ImageArray* FOODPROMPT = NULL;
ImageArray* BLOCKSPROMPT = NULL;
ImageArray* EMPTYFIELD = NULL;
ImageArray* DIFFICULTBUTTON = NULL;
int DIFFICULTBUTTON_HOVERED = 0;
ImageArray* EASYBUTTON = NULL;
int EASYBUTTON_HOVERED = 0;
ImageArray* UPBUTTON = NULL;
int UPBUTTON_HOVERED = 0;
ImageArray* DOWNBUTTON = NULL;
int DOWNBUTTON_HOVERED = 0;
ImageArray* CONFIRMBUTTON = NULL;
int CONFIRMBUTTON_HOVERED = 0;



// -- ARRAYS -- //
// diff implementation from the consoleonly version
// since i have to keep track of objects properly
// or else we go to silent crashes purgatory

// the snake's body
Vector2 SNAKE_ARR[SIZE*SIZE];
int SNAKE_ARR_LEN = 0;

// playable area with their specific objects
char AREA[SIZE][SIZE]; 
// this is a front buffer, what is actually being rendered.
// only deletes and loads the images that actually changed
// in the AREA
char AREADISPLAYED[SIZE][SIZE];

// ------- VECTOR2 FUNCTIONS ------- //

// returns a vector2 with these specific properties
Vector2 v2(int x, int y) {
    return (Vector2) {.x = x, .y = y};
}

// prints an Vector2
void printv2(Vector2 v2) {
    print("(%d, %d)", v2.x, v2.y);
}

/// @brief Adds two vectors together
/// @param vec1 
/// @param vec2 
/// @return added v2
Vector2 v2add(Vector2 vec1, Vector2 vec2) {
    return v2(vec1.x + vec2.x, vec1.y + vec2.y);
}

/// @brief check if two vectors have the exact same components
/// @param v1 
/// @param v2 
/// @return boolean int
int v2equality(Vector2 v1, Vector2 v2) {
    return (v1.x == v2.x && v1.y == v2.y);
}

/// @brief generates a random coordinate with bounds
// (dont forget to initialize srand())
/// @param x_min 
/// @param x_max 
/// @param y_min 
/// @param y_max 
/// @return rand vec2
Vector2 v2rand(int x_min, int x_max, int y_min, int y_max) {
    return v2((rand() % (x_max-x_min + 1)) + x_min, (rand() % (y_max-y_min + 1)) + y_min);
}

// ------- GRAPHICS HELPER FUNCTIONS ------- //

/// @brief frees the filearray u get from getAllChildren
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
        .loop = 1,
        .enabled = 1,
        .opacity = 255.0,
        .rotation = 0.0,
        .position = v2(0,0), // dont got a function for this yet
        .size = v2(0,0), // dont got a function for this yet
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
    // wdebug(L"Got pointer %p [%s]", ptrImageDat, ptrImageDat->name);
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
    // wdebug(L"Inserted %s to %d.", ptrImageDat->name, indextoinsert);
}

/// @brief only prints the pointer and the filename. not any imagearray info
void printdraworder() {
    debug("Full list:");
    for (int i = 0; i<IMAGES_LOADED; i++) {
        ImageArray* dat = MEM_IMAGES[i];
        if (dat == NULL) {warn("index %d was deleted yet hasnt been popped.", i); continue;}
        wdebug(L"PTR %p [%s : %.2lf]", dat, dat->name, dat->layer);
    }
    debug("Loaded images: %d", IMAGES_LOADED);
}

/// @brief finds the first match of ptrImageDat in MEM_IMAGES
/// @param ptrImageDat 
/// @return index, -1 if none found.
int getIndexInDraw(ImageArray* ptrImageDat) {
    for (int i = 0; i<IMAGES_LOADED; i++) {
        ImageArray* dat = MEM_IMAGES[i];
        if (dat == ptrImageDat) return i;
    }
    return -1;
}

/// @brief pops an element at index in MEM_IMAGES.
// shifts all elements, maintains order.
/// @param index index to pop
void popInDraw(int index) {
    IMAGES_LOADED--;
    for (int i = index; i < IMAGES_LOADED; i++) {
        MEM_IMAGES[i] = *(MEM_IMAGES + i+1); // moves x
    }
    MEM_IMAGES[IMAGES_LOADED] = NULL; // just in case
}


/// @brief completely frees an ImageArray from memory
/// @param ptrImageArray 
void freeImageArray(ImageArray* ptrImageArray) {
    int index = getIndexInDraw(ptrImageArray);
    if (index < 0) {
        return;
    }
    ImageArray* fromIndex = MEM_IMAGES[index];
    if (fromIndex != ptrImageArray) {
        werror(L"Mismatching pointer given to free [%p] and actually found [%p]", ptrImageArray, fromIndex);
    }
    wdebug(L"Freeing image %p [%s]", ptrImageArray, ptrImageArray->name);
    MEM_IMAGES[index] = NULL; // mark as being deleted, so WM_PAINT doesnt accidentally try to draw garbage data
    for (int j = 0; j<ptrImageArray->frames; j++) { // for each image loaded, delete HBITMAP
        Image image = *(ptrImageArray->image_pointer + j);
        DeleteObject(image.hbitmap);
    }
    // free up mallocated things
    free(ptrImageArray->name);
    free(ptrImageArray->image_pointer);
    free(ptrImageArray);

    popInDraw(index);

}

/// @brief wipes MEM_IMAGES
void clearImages() {
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
    IMAGES_LOADED = 0;
}

// ------- SNAKE GAME HELPER FUNCTIONS ------- //

/// @brief gets the point in the area
/// @param position pos
/// @return shallow copy i think
char getareapoint(Vector2 position) {
    if (position.x > -1 && position.y > -1 &&
        position.x < SIZE && position.y < SIZE
    ) {
        return AREA[position.y][position.x];
    }
    else return (char) -1;
}

/// @brief places an obj to in AREA. only places if its within bounds.
/// @param position 
/// @param obj 
void placeareapoint(Vector2 position, char obj) {
    if (position.x > -1 && position.y > -1 &&
        position.x < SIZE && position.y < SIZE
    ) {
        AREA[position.y][position.x] = obj;
    }
}

/// @brief checks if a specific point is in an vector2 array
/// @param array 
/// @param arr_len 
/// @param point 
/// @return index (if found), else -1.
int ispointinarray(Vector2 array[], int arr_len, Vector2 point) {
    // debug("arrlen: %d", arr_len);
    for (int i=0; i<arr_len; i++) {
        Vector2 value = *(array + i); // advance pointer by i. pointers are hella weird
        // printv2(value); printv2(point), printf(" %d <", v2equality(value, point)); printf("\n");
        if (v2equality(value, point)) {
            return i;
        }
    }  
    return -1;
}

/// @brief pops an element at index.
// shifts all elements, maintains order.
/// @param array 
/// @param arr_len modified, pass it as an address
/// @param index index to pop
void popInVector2(Vector2 array[], int *arr_len, int index) {
    *arr_len -= 1; // subtract the spot the element was occupying
    for (int i = index; i < *arr_len; i++) {
        array[i] = *(array + i+1); // moves x
    }
}

/// @brief gone
void removeSnakeFromArea() {
    // for every snake part in area currently,
    // delete them. i dont feel like doing this
    // the smart way
    for (int i = 0; i<SNAKE_ARR_LEN; i++) {
        Vector2 pos = SNAKE_ARR[i];
        placeareapoint(pos, EMPTY);
    }
}

/// @brief back
void returnSnakeFromArea() {
    // put them back lol
    for (int i = 0; i<SNAKE_ARR_LEN; i++) {
        Vector2 pos = SNAKE_ARR[i];
        if (getareapoint(pos) == PORTAL) continue; // dont overwrite a portal
        placeareapoint(pos, SNAKE);
        // AREA[pos.y][pos.x] = SNAKE;
    }
}

/// @brief adds a new snake part at pos. takes care of putting it in the area.
/// @param position 
void newSnakePart(Vector2 position) {
    // removeSnakeFromArea();
    SNAKE_ARR[SNAKE_ARR_LEN] = position;
    SNAKE_ARR_LEN++;
    // returnSnakeFromArea();
}

/// @brief get direction from wasd input.
/// @param input 
/// @return vector2 direction, defaults to (0,0).
Vector2 wasddirection(long long input) {
    // translates the wasd to vector direction
    // https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    switch (input) {
        case 'W': return UP;
        case 'A': return LEFT;
        case 'S': return DOWN;
        case 'D': return RIGHT;
    }

    return v2(0,0);
}

/// @brief prints the area in the console.
/// for debugging only.
void printarea() {
    if (!DEBUG_MODE) return;
    for (int y=0; y<SIZE; y++) {
        for (int x=0; x<SIZE; x++) {
            // differentiating the snake's body from its head
            if ((AREA[y][x] == SNAKE) && (SNAKE_ARR_LEN > 0)) { // its a snake, has a head
                if (v2equality(SNAKE_ARR[0], v2(x,y))) { // if is the head
                    printf("%c ", SNAKE);
                } else printf("%c ", SNAKEBODY); // if it isnt
                continue;
            }
            printf("%c ", AREA[y][x]);
        }
        printf("\n");
    }
}

void generatefood(int amount) {
    int attempts = 0;
    int amountactuallyplaced = 0;
    while (amount > 0) {
        if (attempts > 50) {
            break;
        }
        Vector2 randpos = v2rand(1, 13, 1, 13);
        if (getareapoint(randpos) == EMPTY) {
            placeareapoint(randpos, FOOD);
            // AREA[randpos.x][randpos.y] = FOOD;
            // placeobject(FOOD, randpos);
            // updatearea();
            amount--; 
            amountactuallyplaced++;
            attempts = 0;
        } else attempts++;
    }
    CURR_FOOD = amountactuallyplaced;
    debug("food: %d", CURR_FOOD);
}


// tries to generate n number of blocks on the map
// checks if the blocks will be placed on a valid position
// same code as above but i just couldn't figure out how to make it a macro...
void generateblocks(int amount) {
    int attempts = 0;
    while (amount > 0) {
        if (attempts > 50) {
            break;
        }
        Vector2 randpos = v2rand(1, 13, 1, 13);
        if (getareapoint(randpos) == EMPTY) {
            placeareapoint(randpos, BLOCK);
            // placeobject(BLOCK, randpos);
            amount--; 
            attempts = 0;
        } else attempts++;
    }
}

// ------- PATH FUNCTIONS ------- //
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

    // wdebug(L"given thing to load: %s", image_or_folder_name);

    if (!isPath(image_or_folder_name)) {
        werror(L"Malformed/Nonexistent path given: %s", image_or_folder_name);
    }

    int isBMP = compareExtension(image_or_folder_name, L"bmp");
    LPWSTR trimmedExtension;
    LPWSTR trimmed;
    trimExtension(image_or_folder_name, &trimmedExtension);
    trimPath(trimmedExtension, &trimmed);
    free(trimmedExtension);

    // wdebug(L"path trimmed: %s", trimmed);

    // differentiating between images and animations
    ImageArray* imagedat = newHeapImageArray();
    imagedat->name = trimmed;
    imagedat->layer = layer;

    Image *images = NULL;
    LPCWSTR *files = NULL;
    DWORD nfiles;
    DWORD nimages;
    if (isBMP) { // is a BMP
        // debug("loading as image");
        nfiles = 0;
        nimages = 1;
    } else if (!isBMP && !isFile(image_or_folder_name)) { // is an anim folder
        // debug("loading as animation");
        
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
    // debug("IMAGES TO LOAD:");
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
        // wdebug(L"loading %s", name);

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
        // wdebug(L"successfully loaded %s", name);
    }

    imagedat->image_pointer = images;
    imagedat->size = v2(images->bitmap.bmWidth, images->bitmap.bmHeight); // inherit image or 1st frame size

    // sorts the images by layer
    insertToDraw(imagedat);
    if (files != NULL) freeFiles(files, nfiles);

    // wdebug(L"successfully loaded %s", imagedat->name);
    return imagedat;
}

// ------- SNAKE GAME FUNCTIONS ------- //

/// @brief displays numerals at a point with specified height.
/// only displays positive bc im lazy.
/// size of number is 2:1.
/// this function only took me 2 tries to make it work omg im getting better
/// @param number number to display
/// @param center 
/// @param height 
void displayNumerals(int number, Vector2 center, int height) {
    // get numbers to display
    // debug("to display: %d", number);

    int reverseorder[10];
    int digits = 0;

    if (number <= 0) { //edge case
        reverseorder[0] = 0;
        digits = 1;
    }
    while (number > 0) {
        int digit = number%10;
        // debug("display %d", digit);
        if (digits > 9) {error("Cant display more than 10 digits lol");}
        reverseorder[digits] = digit;
        digits++;
        number /= 10;
    }

    int width = height/2;
    int totalspaceneeded = width*digits + NUMERALSPACING*(digits-1);
    Vector2 origin = v2(
        center.x - totalspaceneeded/2,
        center.y
    );

    // load the images
    // debug("digits: %d", digits);
    for (int i = digits; i>0; i--) {
        int digit = reverseorder[i-1];
        // debug("DISPLAYING DIGIT %d", digit);
        Vector2 topleft = v2(
            // origin.x + (i - digits)*width + NUMERALSPACING*(i - digits),
            origin.x - (width + NUMERALSPACING)*(i-digits),
            origin.y - height/2
        );
        LPCWSTR pathname = NULL;
        switch (digit) {
            case 0: pathname = ZEROPATH; break;
            case 1: pathname = ONEPATH; break;
            case 2: pathname = TWOPATH; break;
            case 3: pathname = THREEPATH; break;
            case 4: pathname = FOURPATH; break;
            case 5: pathname = FIVEPATH; break;
            case 6: pathname = SIXPATH; break;
            case 7: pathname = SEVENPATH; break;
            case 8: pathname = EIGHTPATH; break;
            case 9: pathname = NINEPATH; break;
            default: error("UNACCOUNTED DIGIT: %d", digit);
        }
        NUMERALS[NUMERALS_LOADED] = loadimage(pathname, 1000);
        ImageArray* numeral = NUMERALS[NUMERALS_LOADED];
        numeral->position = topleft;
        numeral->size = v2(
            width, height
        );

        NUMERALS_LOADED++;
        // im not dealing with realloc again
        // you cant make me.
        if (NUMERALS_LOADED>99) {
            error("Cant display more than 100 total numbers");
        }
    }
}

/// @brief clears all numeral objs
void clearNumerals() {
    for (int i = 0; i < NUMERALS_LOADED; i++) {
        freeImageArray(NUMERALS[i]);
        NUMERALS[i] = NULL;
    }
    NUMERALS_LOADED = 0;
}

/// @brief fills some arrays with their default values
void initarrays() {
    for (int y=0; y<SIZE; y++) {
        for (int x=0; x<SIZE; x++) {
            AREA[y][x] = EMPTY;             // default: EMPTY
            AREADISPLAYED[y][x] = -1;             // default: -1, different so it actually updates the first time
            SNAKEGAME_BACKGROUND[y][x] = NULL;   // default: NULL
            SNAKEGAME_FOREGROUND[y][x] = NULL;   // default: NULL
        }
    }
}

/// @brief clears the bg and fg objects
void clearBGFG() {
    debug("clearing everything");
    // wipes all the objects
    for (int y = 0; y<SIZE; y++) {
        for (int x = 0; x<SIZE; x++) {
            ImageArray* tilebg = SNAKEGAME_BACKGROUND[y][x];
            ImageArray* tilefg = SNAKEGAME_FOREGROUND[y][x];
            if (tilebg != NULL) {      
                freeImageArray(tilebg);
                SNAKEGAME_BACKGROUND[y][x] = NULL;
            }
            if (tilefg != NULL) {      
                freeImageArray(tilefg);
                SNAKEGAME_FOREGROUND[y][x] = NULL;
            }
            AREADISPLAYED[y][x] = -1;
        }
    }
}

/// @brief actually updates the images from what changed in AREA instead of completely deleting
/// and rebuilding bruh
void updateDisplayedArea() {
    Vector2 wherethehelleverythingis = v2(150, 150);

    for (int y = 0; y<SIZE; y++) {
        for (int x = 0; x<SIZE; x++) {
            char areaChar = AREA[y][x];
            char displayedChar = AREADISPLAYED[y][x];
            if (areaChar == displayedChar) {  // do nothing if its the same
                // debug("SKIP (%d, %d)", x,y); 
                continue;
            }
            // debug("%c -> %c [%d]", displayedChar, areaChar, displayedChar == areaChar);
            int isForeground = 0; // also used as layer lol
            int isSnake = 0;
            LPWSTR filename; // i have to spam (LPWSTR) to make the compiler SHUT UP ABOUT THE CONST QUALIFIER
            LPWSTR emptyfilename;
            switch (areaChar) {
                case WALL:
                    isForeground = 0;
                    filename = (LPWSTR) WALLPATH;
                    break;
                case BLOCK:
                    isForeground = 0;
                    filename = (LPWSTR) BLOCKPATH;
                    break;
                case FOOD:
                    isForeground = 1;
                    filename =(LPWSTR)  FOODPATH;
                    break;
                case SNAKE: // special snake rendering
                    isForeground = 1;
                    isSnake = 1;
                    filename =(LPWSTR)  SNAKEHEADPATH;
                    break;
                case PORTAL:
                    isForeground = 1;
                    filename = (LPWSTR) PORTALPATH;
                    break;
                default:
                    isForeground = 0;
                    filename = (LPWSTR) ((x+y)%2 == 0 ? TILEFULLPATH : TILEHOLLOWPATH);
            }

            // clean up the current tile there
            // debug("FG: %p", SNAKEGAME_FOREGROUND[y][x]);
            // debug("BG: %p", SNAKEGAME_BACKGROUND[y][x]);
            if (SNAKEGAME_FOREGROUND[y][x] != NULL) {
                // debug("CLEAN UP FG");    
                freeImageArray(SNAKEGAME_FOREGROUND[y][x]);
                SNAKEGAME_FOREGROUND[y][x] = NULL;
            }
            if (SNAKEGAME_BACKGROUND[y][x] != NULL) {  
                // debug("CLEAN UP BG");        
                freeImageArray(SNAKEGAME_BACKGROUND[y][x]);
                SNAKEGAME_BACKGROUND[y][x] = NULL;
            }

            ImageArray* tileimg = loadimage(filename, isForeground);
            // if its a foreground, also draw the empty square
            if (isForeground) {
                emptyfilename = (LPWSTR)  ((x+y)%2 == 0 ? TILEFULLPATH : TILEHOLLOWPATH);
                SNAKEGAME_BACKGROUND[y][x] = loadimage(emptyfilename, 0);
                SNAKEGAME_FOREGROUND[y][x] = tileimg;
                SNAKEGAME_BACKGROUND[y][x]->size = v2(30, 30);
                SNAKEGAME_BACKGROUND[y][x]->position = v2(
                    wherethehelleverythingis.x + x*30, 
                    wherethehelleverythingis.y + y*30
                );
                SNAKEGAME_BACKGROUND[y][x]->framecount = FRAME_LIFETIME % SNAKEGAME_BACKGROUND[y][x]->frames;
                
                // set fg
                SNAKEGAME_FOREGROUND[y][x] = tileimg;
                free(emptyfilename); // SHUT UP!!! I DONT CARE ABOUT THE CONST QUALIFIER!! DAMN WARNINGS
            } else {
                // set bg
                SNAKEGAME_BACKGROUND[y][x] = tileimg;
            }
            if (tileimg == NULL) continue;
            // stanard pos and sizes
            // these r got from trial and error lolz
            tileimg->size = v2(30, 30);
            tileimg->position = v2(
                wherethehelleverythingis.x + x*30, 
                wherethehelleverythingis.y + y*30
            );
            free(filename);
            AREADISPLAYED[y][x] = AREA[y][x];
        }
    }
}

/// @brief handles a lot. movement, eating, dying, winning, and spawning the portal
/// @param direction 
void movesnake(Vector2 direction) {
    if (v2equality(direction, v2(0,0))) return;
    Vector2 lastpos;
    Vector2 futurepos = v2add(direction, SNAKE_ARR[0]);
    
    
    // handle dying
    char objInWay = getareapoint(futurepos);
    if (objInWay != EMPTY && objInWay != FOOD && objInWay != PORTAL && objInWay != (char) -1) {
        // gross way to do this lol
        // kind of buggy
        Vector2 head = futurepos; //SNAKE_ARR[0];
        ImageArray* headImg = SNAKEGAME_BACKGROUND[head.y][head.x];
        debug("DIED, %p", headImg);
        printv2(SNAKE_ARR[0]);
        printv2(futurepos);

        DIEDINDICATOR->position = v2(
            headImg->position.x - DIEDINDICATOR->size.x/2 + headImg->size.x/2,
            headImg->position.y - DIEDINDICATOR->size.y/2 + headImg->size.y/2
        );
        DIEDINDICATOR->enabled = 1;
        GAME_OVER = 1;
        ANIMATE_BOARD_DISAPPEAR = 1;
        WINNER = 0;
        return;
    }

    removeSnakeFromArea();
    
    // advance the snake
    for (int i=0; i<SNAKE_ARR_LEN; i++) {
        Vector2 snakebody = SNAKE_ARR[i]; // pos
        // is it the head?
        if (i==0) {
            lastpos = snakebody;
            SNAKE_ARR[i] = futurepos;
            debug("New SNAKE POS: "); printv2(SNAKE_ARR[i]); printf("\n");
            continue;
        }
        // if its not the head, then advance
        // the body to where the previous one was
        SNAKE_ARR[i] = lastpos;
        lastpos = snakebody;
    }

    // gotta handle this here just so i dont have to do
    // any math
    if (objInWay == FOOD) {
        newSnakePart(lastpos);
        CURR_FOOD--;
        SCORE++;
        clearNumerals();
        displayNumerals(SCORE, 
            v2(
                WINDOWWIDTH/2,
                SCORECARD->size.y + 25
            ),
            50
        );
    }

    // spawn in a portal
    if (CURR_FOOD == 0 && getareapoint(v2(14,7)) != PORTAL) {
        placeareapoint(v2(14, 7), PORTAL);
    }

    returnSnakeFromArea();

    printarea();

    updateDisplayedArea();
    // updateareabackground();
    // updateareaforeground();
}

// ------- SNAKE GAME HANDLER ------- //
void SNAKE_GAME() {
    GAME_OVER = 0; // important marker
    SCORE = 0;

    // some other objects used
    varloadimage(DIEDINDICATOR, DIEDPATH, 10000); // anim for death
    DIEDINDICATOR->enabled = 0;
    DIEDINDICATOR->size = v2(75, 75);
    varloadimage(SCORECARD, SCOREPATH, 100);
    SCORECARD->size = v2(
        WINDOWWIDTH*0.3,
        WINDOWHEIGHT*0.1
    );
    SCORECARD->position = v2(
        WINDOWWIDTH/2 - SCORECARD->size.x/2,
        WINDOWHEIGHT*0
    );
    displayNumerals(SCORE, 
        v2(
            WINDOWWIDTH/2,
            SCORECARD->size.y + 25
        ),
        50
    );
    varloadimage(LOSERCARD, LOSERPATH, 100);
    LOSERCARD->size = v2(
        DIFFICULTYPROMPT->size.x,
        DIFFICULTYPROMPT->size.y
    );
    LOSERCARD->position = v2(
        DIFFICULTYPROMPT->position.x,
        DIFFICULTYPROMPT->position.y
    );
    varloadimage(WINNERCARD, WINNERPATH, 100);
    WINNERCARD->size = v2(
        DIFFICULTYPROMPT->size.x,
        DIFFICULTYPROMPT->size.y
    );
    WINNERCARD->position = v2(
        DIFFICULTYPROMPT->position.x,
        DIFFICULTYPROMPT->position.y
    );
    
    LOSERCARD->enabled = 0;
    WINNERCARD->enabled = 0;

    initarrays();

    // Places the blocks surrounding the playing field
    for (int y=0; y<SIZE; y++) {
        for (int x=0; x<SIZE; x++) {
            // if its at the top, bottom, or side
            if (y == 0 || y == (SIZE-1) || x == 0 || x == (SIZE - 1)) {
                // placeobject(WALL, pos);
                placeareapoint(v2(x,y), WALL);
                // AREA[y][x] = WALL;
                continue;
            }
        }   
    }
    newSnakePart(v2(1,1));
    returnSnakeFromArea(); // blits the snake basically. no idea why i chose to do it this way
    generatefood(NUM_OF_FOOD);
    generateblocks(NUM_OF_BLOCK);

    updateDisplayedArea();
    // updateareabackground();
    // updateareaforeground();
    

    printdraworder();
}

// ------- MENU HANDLERS ------- //

// button functions

/// @brief show options menu
/// @param hwnd 
void PLAYBUTTON_FUNC(HWND hwnd) {
    PLAYBUTTON->enabled = 0;
    INSTRUCTIONSBUTTON->enabled = 0;
    EXITBUTTON->enabled = 0;

    DIFFICULTYPROMPT->enabled = 1;
    EASYBUTTON->enabled = 1;
    DIFFICULTBUTTON->enabled = 1;
}

/// @brief show instructions and disable the other buttons
/// @param hwnd 
void INSTRUCTIONSBUTTON_FUNC(HWND hwnd) {
    HOWTOPLAYCARD->enabled = 1;
    CLOSEINSTBUTTON->enabled = 1;
    PLAYBUTTON->enabled = 0;
    INSTRUCTIONSBUTTON->enabled = 0;
    EXITBUTTON->enabled = 0;
}

/// @brief closes the game
/// @param hwnd 
void EXITBUTTON_FUNC(HWND hwnd) {
    PostMessage(hwnd, WM_DESTROY, 0, 0);
}

/// @brief close instructions and enable the other buttons
/// @param hwnd 
void CLOSEINSTBUTTON_FUNC(HWND hwnd) {
    HOWTOPLAYCARD->enabled = 0;
    CLOSEINSTBUTTON->enabled = 0;
    PLAYBUTTON->enabled = 1;
    INSTRUCTIONSBUTTON->enabled = 1;
    EXITBUTTON->enabled = 1;
}

/// @brief updates the counter
void updateNumeralForOptions(int numberToDisplay) {
    clearNumerals();
    displayNumerals(
        numberToDisplay,
        v2(
            EMPTYFIELD->position.x + EMPTYFIELD->size.x/2,
            EMPTYFIELD->position.y + EMPTYFIELD->size.y/2
        ),
        50
    );
}


/// @brief just to avoid having to copy and paste repeatedly
void chosediff() {
    DIFFICULTYPROMPT->enabled = 0;
    EASYBUTTON->enabled = 0;
    DIFFICULTBUTTON->enabled = 0;

    FOODPROMPT->enabled = 1;
    EMPTYFIELD->enabled = 1;
    UPBUTTON->enabled = 1;
    DOWNBUTTON->enabled = 1;
    CONFIRMBUTTON->enabled = 1;

    updateNumeralForOptions(NUM_OF_FOOD);
}

/// @brief sets difficulty to easy
/// @param hwnd 
void EASYBUTTON_FUNC(HWND hwnd) {
    DIFFICULTY = EASY;
    NUM_OF_FOOD = MAX_FOOD;
    NUM_OF_BLOCK = 0;

    chosediff();
}

/// @brief sets difficulty to difficult
/// @param hwnd 
void DIFFICULTBUTTON_FUNC(HWND hwnd) {
    DIFFICULTY = DIFFICULT;
    NUM_OF_FOOD = MAX_FOOD;
    NUM_OF_BLOCK = MAX_BLOCKS;
    chosediff();
}

void CONFIRMBUTTON_FUNC(HWND hwnd) {
    if (FOODPROMPT->enabled && DIFFICULTY == DIFFICULT) { // if did food prompt and difficulty=difficult, prompt block
        FOODPROMPT->enabled = 0;
        BLOCKSPROMPT->enabled = 1;
        updateNumeralForOptions(NUM_OF_BLOCK);
    } else if (BLOCKSPROMPT->enabled || DIFFICULTY == EASY) { // if blocks prompt is done or difficulty=easy and did food prompt, actually start the game
        BLOCKSPROMPT->enabled = 0;
        FOODPROMPT->enabled = 0;
        UPBUTTON->enabled = 0;
        DOWNBUTTON->enabled = 0;
        EMPTYFIELD->enabled = 0;
        TITLECARD->enabled = 0;
        CONFIRMBUTTON->enabled = 0;

        // free all the htings that main menu loaded
        clearNumerals();
        // dont need to do this cuz varloadimage handles it fine
        // clearImages();
        debug("STAR THE GAME DIFF");
        debug("FOOD: %d, BLOCKS: %d", NUM_OF_FOOD, NUM_OF_BLOCK);
        SNAKE_GAME();
    }
}

/// @brief increment the food/blocks number
/// @param hwnd 
void UPBUTTON_FUNC(HWND hwnd) {
    if (FOODPROMPT->enabled) {
        NUM_OF_FOOD = max(MIN_FOOD, min(NUM_OF_FOOD+1, MAX_FOOD));
        updateNumeralForOptions(NUM_OF_FOOD);
    }
    if (BLOCKSPROMPT->enabled) {
        NUM_OF_BLOCK = max(MIN_BLOCKS, min(NUM_OF_BLOCK+1, MAX_BLOCKS));
        updateNumeralForOptions(NUM_OF_BLOCK);
    }
}

/// @brief decrement the food/blocks number
/// @param hwnd 
void DOWNBUTTON_FUNC(HWND hwnd) {
    if (FOODPROMPT->enabled) {
        NUM_OF_FOOD = max(MIN_FOOD, min(NUM_OF_FOOD-1, MAX_FOOD));
        updateNumeralForOptions(NUM_OF_FOOD);
    }
    if (BLOCKSPROMPT->enabled) {
        NUM_OF_BLOCK = max(MIN_BLOCKS, min(NUM_OF_BLOCK-1, MAX_BLOCKS));
        updateNumeralForOptions(NUM_OF_BLOCK);
    }
}

// menu and other stuff
void MAIN_MENU() {
    // load the images safely without repetition;

    // main menu
    varloadimage(TITLECARD, TITLEPATH, 100);
    varloadimage(HOWTOPLAYCARD, HTPPATH, 101);
    varloadimage(PLAYBUTTON, PLAYBTNPATH, 100)
    varloadimage(INSTRUCTIONSBUTTON, INSTRUCTIONSBTNPATH, 100);
    varloadimage(EXITBUTTON, EXITBTNPATH, 100);
    varloadimage(CLOSEINSTBUTTON, CLOSEBTNPATH, 102);

    // options
    varloadimage(DIFFICULTYPROMPT, DIFFICULTYPROMPTPATH, 100);
    varloadimage(FOODPROMPT, FOODPROMPTPATH, 100);
    varloadimage(BLOCKSPROMPT, BLOCKSPROMPTPATH, 100);
    varloadimage(EMPTYFIELD, EMPTYFIELDPATH, 100);

    varloadimage(EASYBUTTON, EASYBTNPATH, 100);
    varloadimage(DIFFICULTBUTTON, DIFFICULTBTNPATH, 100);
    varloadimage(UPBUTTON, UPBTNPATH, 100);
    varloadimage(DOWNBUTTON, DOWNBTNPATH, 100);
    varloadimage(CONFIRMBUTTON, CONFIRMBTNPATH, 100);

    // making it look fire
    // these sizes and positions r got from trial
    // and error lol. 

    // option
    DIFFICULTYPROMPT->size = v2(
        WINDOWWIDTH*0.6,
        WINDOWHEIGHT/5
    );

    DIFFICULTYPROMPT->position = v2(
        WINDOWWIDTH/2 - DIFFICULTYPROMPT->size.x/2,
        WINDOWHEIGHT/2 - DIFFICULTYPROMPT->size.y*1
    );

    FOODPROMPT->size = v2(
        DIFFICULTYPROMPT->size.x,
        DIFFICULTYPROMPT->size.y
    );

    FOODPROMPT->position = v2(
        DIFFICULTYPROMPT->position.x,
        DIFFICULTYPROMPT->position.y
    );

    BLOCKSPROMPT->size = v2(
        DIFFICULTYPROMPT->size.x,
        DIFFICULTYPROMPT->size.y
    );

    BLOCKSPROMPT->position = v2(
        DIFFICULTYPROMPT->position.x,
        DIFFICULTYPROMPT->position.y
    );

    EASYBUTTON->size = v2(
        WINDOWWIDTH*0.3,
        WINDOWHEIGHT*0.1
    );

    EASYBUTTON->position = v2(
        WINDOWWIDTH/2 - EASYBUTTON->size.x/2,
        WINDOWHEIGHT/2 - EASYBUTTON->size.y/2 + WINDOWHEIGHT*0.1
    );

    DIFFICULTBUTTON->size = v2(
        WINDOWWIDTH*0.5,
        WINDOWHEIGHT*0.1
    );

    DIFFICULTBUTTON->position = v2(
        WINDOWWIDTH/2 - DIFFICULTBUTTON->size.x/2,
        WINDOWHEIGHT/2 - DIFFICULTBUTTON->size.y/2 + WINDOWHEIGHT*0.245
    );

    EMPTYFIELD->size = v2(
        EASYBUTTON->size.x,
        EASYBUTTON->size.y
    );

    EMPTYFIELD->position = v2(
        EASYBUTTON->position.x,
        EASYBUTTON->position.y
    );

    UPBUTTON->size = v2(
        70, 70
    );

    UPBUTTON->position = v2(
        WINDOWWIDTH/2 - UPBUTTON->size.x/2,
        WINDOWHEIGHT/2 - UPBUTTON->size.y/2 - 1
    );

    DOWNBUTTON->size = v2(
        UPBUTTON->size.x,
        UPBUTTON->size.y
    );

    DOWNBUTTON->position = v2(
        WINDOWWIDTH/2 - DOWNBUTTON->size.x/2,
        WINDOWHEIGHT/2 - DOWNBUTTON->size.y/2 + UPBUTTON->size.y + EMPTYFIELD->size.y + 8
    );

    CONFIRMBUTTON->size = v2(
        DIFFICULTBUTTON->size.x,
        DIFFICULTBUTTON->size.y
    );

    CONFIRMBUTTON->position = v2(
        WINDOWWIDTH/2 - CONFIRMBUTTON->size.x/2,
        WINDOWHEIGHT/2 - CONFIRMBUTTON->size.y/2 + WINDOWHEIGHT*0.32
    );

    DIFFICULTYPROMPT->enabled = 0;
    FOODPROMPT->enabled = 0;
    BLOCKSPROMPT->enabled = 0;
    EMPTYFIELD->enabled = 0;
    
    EASYBUTTON->enabled = 0;
    DIFFICULTBUTTON->enabled = 0;
    UPBUTTON->enabled = 0;
    DOWNBUTTON->enabled = 0;
    CONFIRMBUTTON->enabled = 0;
    
    // instructions
    HOWTOPLAYCARD->size = v2(
        WINDOWWIDTH*0.9,
        WINDOWHEIGHT*0.9
    );

    HOWTOPLAYCARD->position = v2(
        WINDOWWIDTH/2 - HOWTOPLAYCARD->size.x/2,
        WINDOWHEIGHT/2 - HOWTOPLAYCARD->size.y/2
    );

    CLOSEINSTBUTTON->size = v2(
        WINDOWWIDTH*0.1,
        WINDOWHEIGHT*0.1
    );

    CLOSEINSTBUTTON->position = v2(
        WINDOWWIDTH*0.9 - CLOSEINSTBUTTON->size.x/2,
        WINDOWHEIGHT*0.07
    );

    HOWTOPLAYCARD->enabled = 0;
    CLOSEINSTBUTTON->enabled = 0;

    // main menu things
    TITLECARD->size = v2(
        WINDOWWIDTH - WINDOWWIDTH*0.2,
        WINDOWHEIGHT/3.5
    );

    TITLECARD->position = v2(
        WINDOWWIDTH/2 - TITLECARD->size.x/2,
        WINDOWHEIGHT/2 - TITLECARD->size.y*1.6
    );

    PLAYBUTTON->size = v2(
        WINDOWWIDTH*0.3,
        WINDOWHEIGHT*0.1
    );

    PLAYBUTTON->position = v2(
        WINDOWWIDTH/2 - PLAYBUTTON->size.x/2,
        WINDOWHEIGHT/2 - PLAYBUTTON->size.y
    );

    INSTRUCTIONSBUTTON->size = v2(
        WINDOWWIDTH*0.5,
        WINDOWHEIGHT*0.1
    );

    INSTRUCTIONSBUTTON->position = v2(
        WINDOWWIDTH/2 - INSTRUCTIONSBUTTON->size.x/2,
        WINDOWHEIGHT/2 - INSTRUCTIONSBUTTON->size.y/2 + WINDOWHEIGHT*0.1
    );

    EXITBUTTON->size = v2(
        WINDOWWIDTH*0.25,
        WINDOWHEIGHT*0.1
    );

    EXITBUTTON->position = v2(
        WINDOWWIDTH/2 - EXITBUTTON->size.x/2,
        WINDOWHEIGHT/2 - EXITBUTTON->size.y/2 + WINDOWHEIGHT*0.245
    );

}

// ------- functions that i need to put at the bottom lol ------- //

/// @brief completely clear the game and
/// show the win/lose prompt
void clearSnakeGame() {
    SNAKE_ARR_LEN = 0; // we dont rlly need to clear it
    clearBGFG();
    freeImageArray(DIEDINDICATOR);
    DIEDINDICATOR = NULL;
    
    LOSERCARD->enabled = !WINNER;
    WINNERCARD->enabled = WINNER;
}


// ------- FRAME LOOP FUNCTION ------- //

/// @brief im separating it so that i can call it twice since its too slow
/// if its just one at a time. this also contains the logic for when the snake dies.
/// very good code organization wow
void spiralStep() {
    if (!ANIMATE_BOARD_DISAPPEAR) return;
    if (stepsTaken >= steps) { // turn
        if (v2equality(stepdir, RIGHT)) {
            stepdir = DOWN;
        } else if (v2equality(stepdir, DOWN)) {
            stepdir = LEFT;
        } else if (v2equality(stepdir, LEFT)) {
            stepdir = UP;
        } else if (v2equality(stepdir, UP)) {
            stepdir = RIGHT;
        }
        turns++;
        // debug("%d", turns);
        // wacky pattern... took a while to figure out
        if (turns%2 > 0 && turns > 1) {
            // debug("reduce");
            steps--;
        }
        stepsTaken = 0;
    }
    stepsTaken++;
    // disable the things
    SNAKEGAME_BACKGROUND[pos.y][pos.x] ->enabled = 0;
    if (SNAKEGAME_FOREGROUND[pos.y][pos.x] != NULL) {
        SNAKEGAME_FOREGROUND[pos.y][pos.x]->enabled = 0;
    }
    pos = v2add(pos, stepdir);
    if (steps < 1) {
        ANIMATE_BOARD_DISAPPEAR = 0;
        DIEDINDICATOR->enabled = 0;
        debug("finished spriral");
        // weird ass place to need to handle this in lol
        clearSnakeGame();
    }
}

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

        // checks if the main menu buttons are being hovered over
        if (GAME_OVER) {
            // menu
            hoverlogic(PLAYBUTTON);
            hoverlogic(INSTRUCTIONSBUTTON);
            hoverlogic(EXITBUTTON);
            hoverlogic(CLOSEINSTBUTTON);
            
            // options
            hoverlogic(EASYBUTTON);
            hoverlogic(DIFFICULTBUTTON);
            hoverlogic(CONFIRMBUTTON);
            hoverlogic(UPBUTTON);
            hoverlogic(DOWNBUTTON);
        }

        // the spiral effect
        // assumes the values r alreayd reset
        // this was so confusing
        if (ANIMATE_BOARD_DISAPPEAR) {
            spiralStep();
            spiralStep();
            spiralStep();
        }


        // sends WM_PAINT message
        if (THREAD_ENABLED) 
            PostMessage(hwnd, WM_PAINT, 0, 0);
        Sleep(max(1000/FRAME_RATE, 1));
    }
    debug("Thread ended");

    return 0;
}

// ------- uMsg HANDLER FUNCTIONS ------- //

/// @brief things that happen when the window is messaged to open
void OnCreate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    debug("CREATE");
    

    MAIN_MENU();

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
    HBRUSH bgBrush = CreateSolidBrush(BACKGROUND_COLOR);
    // FillRect(hdcBack, &ps.rcPaint, (HBRUSH) GetStockObject(BLACK_BRUSH));
    FillRect(hdcBack, &ps.rcPaint, bgBrush);
    DeleteObject(bgBrush);
    if (IMAGES_LOADED > 0) {
        HDC mem = CreateCompatibleDC(hdcBack); // surface to draw on hdcBack
        HBITMAP old = NULL;

        for (int i = 0; i<IMAGES_LOADED; i++) {
            ImageArray *imagedat = MEM_IMAGES[i];
            if (imagedat == NULL) continue; // if its in the process of being deleted
            if (!imagedat->enabled) continue; // skip all disabled frames
            
            // update currframe
            if (imagedat->framecount >= imagedat->framerate) {
                imagedat->framecount = 0;
                imagedat->currframe++;

                // handle looping
                if (imagedat->currframe >= imagedat->frames) {
                    if (imagedat->loop) {
                        imagedat->currframe = imagedat->currframe % imagedat->frames;
                    }else {
                        imagedat->currframe = imagedat->frames-1; // stay on last frame indefinitely
                        imagedat->framecount = -1;
                    }
                } 
                
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
    clearImages();

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
            // clearBGFG();
            if (GAME_OVER) {
                // main menu
                buttonlogic(PLAYBUTTON, hwnd);
                buttonlogic(INSTRUCTIONSBUTTON, hwnd);
                buttonlogic(EXITBUTTON, hwnd);
                buttonlogic(CLOSEINSTBUTTON, hwnd);
                // optionsd
                buttonlogic(EASYBUTTON, hwnd);
                buttonlogic(DIFFICULTBUTTON, hwnd);
                buttonlogic(UPBUTTON, hwnd);
                buttonlogic(DOWNBUTTON, hwnd);
                buttonlogic(CONFIRMBUTTON, hwnd);
            }
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
            // printv2(movedir);
            // if the game is running then move the snakee
            if (!GAME_OVER) {
            // Vector2 movedir = wasddirection(KEYCODE_UP);
                movesnake(wasddirection(KEYCODE_UP));
            }
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

// this whole project is such a patchwork bruh
// doing this blind and just searching up docs
// was not the optimal decision...
// a whole weeks worth of work just for something
// so damn simple (in theory).. UGHHHH
// this was a larger undertaking than i initially
// thought. no wonder so much things exist just to
// make rendering graphics a lot easier
// bring me back to pygame.....