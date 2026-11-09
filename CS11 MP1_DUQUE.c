/* BS COMPUTER SCIENCE PROJECT -- SNAKE GAME */
/* !! DISCLAIMER !! THIS IS WRITTEN SPECIFICALLY FOR A WINDOWS ENVIRONMENT
-------------------------------------------------
Authors:
    Cyrus Nathan M. Duque

Date Started:
    November 7, 2026

Date Finished:
    November 8, 2026 (day before an exam lol)
Made for CSMC 11 QR, UP BAGUIO A.Y. 2026-2027
ONLY CONSOLE DISPLAYING. 


Credits to very useful resources used throughout the project:
    ANSI CODES for console coloring:
    https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
-------------------------------------------------

This is the main script.
Contains game logic

'*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <signal.h> // for aborting
#include <stdbool.h> // for true or false because 1 or 0 SUCKS
#include <conio.h> // MOVE AWAY FROM CONIO. NEEDS TO BE SUPPORTED ON BOTH WINDOWS AND MAC
// #include <windows.h>
// #include <graphics.h>
// #include "logs.h"
// #include "vector2.h"

// hardcoded settings
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

// ---- MACROS ----- //

// DEBUGGING
// The wacky thingies are ANSI codes.
#define DEBUG_MODE 1 // change to 0 to disable debug printss
#define error(fmt, ...) printf("\033[41m[%s:%d]\033[0m : " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); raise(SIGABRT)
#define warning(fmt, ...) printf("\033[43m[%s:%d]\033[0m : " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define debug(fmt, ...) if (DEBUG_MODE) printf("\033[34m[%s:%d]\033[0m : " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define print(fmt, ...) printf(fmt "\n", ##__VA_ARGS__) // printf wrapper

// OTHER STUFF
// using this just bc its a bunch of repeated code
// dont attempt to render a snake thats outside the area (from the protal) 
#define updateareahelper(name) for (int i=0; i< name##_ARR_LEN;i++) { \
        Vector2 pos = name##_ARR[i]; \
        if (pos.x > (SIZE-1) || pos.y > (SIZE-1)) { \
            continue; \
        } \
        AREA[pos.y][pos.x] = name; \
    }

// places an object in its array with a position
// NOT SAFE. doesnt check if the object already exists or if it will
// overlap with another object.
#define placeobject(object_name, v2_position) \
    object_name##_ARR[object_name##_ARR_LEN] = v2_position; \
    object_name##_ARR_LEN += 1

// places an object in its array
// if ANY OTHER OBJECT is conflicting, then remove that first
// then place it. doesnt need updatearea() first since it
// checks the actual object arrays
#define hardplaceobject(object_name, v2_position) \
    int foodindex = ispointinarray(FOOD_ARR, FOOD_ARR_LEN, v2_position); \
    if (foodindex > -1) { debug("POP FOOD at INDEX %d", foodindex); pop(FOOD_ARR, &FOOD_ARR_LEN, foodindex); } \
    int SNAKEindex = ispointinarray(SNAKE_ARR, SNAKE_ARR_LEN, v2_position); \
    if (SNAKEindex > -1) pop(SNAKE_ARR, &SNAKE_ARR_LEN,SNAKEindex); \
    int WALLindex = ispointinarray(WALL_ARR, WALL_ARR_LEN, v2_position); \
    if (WALLindex > -1) { debug("POP WALL at INDEX %d", WALLindex); pop(WALL_ARR, &WALL_ARR_LEN, WALLindex); } \
    int BLOCKindex = ispointinarray(BLOCK_ARR, BLOCK_ARR_LEN, v2_position); \
    if (BLOCKindex > -1) { debug("POP BLOCK at INDEX %d", BLOCKindex); pop(BLOCK_ARR, &BLOCK_ARR_LEN, BLOCKindex); } \
    int PORTALindex = ispointinarray(PORTAL_ARR, PORTAL_ARR_LEN, v2_position); \
    if (PORTALindex > -1) pop(PORTAL_ARR, &PORTAL_ARR_LEN, PORTALindex); \
    object_name##_ARR[object_name##_ARR_LEN] = v2_position; \
    object_name##_ARR_LEN += 1

    

// 'clears' an object array
// just sets its internal len to 0. thats all we care about anyway.
#define cleararray(object_name) \
    object_name##_ARR_LEN = 0;

// -- TYPES -- //
// 2D Coordinates (integers)
typedef struct {
    int x;
    int y;
} Vector2;


// -- CONSTANTS -- //
const Vector2 UP = {.x = 0, .y = -1};
const Vector2 DOWN = {.x = 0, .y = 1};
const Vector2 LEFT = {.x = -1, .y = 0};
const Vector2 RIGHT = {.x = 1, .y = 0};

// for whatever keys are actually supported
const int SUPPORTED_INPUTS[] = {
    // MAIN MENU
    '1','2','3',
    // WASD
    'w','a','s','d',
    
    // exit key
    'q'
};
const int NUM_SUPPORTED_INPUTS = 3+7+1;

// -- GAME SETTINGS -- //
// these r eventually modified by the user.
// do NOT treat as constants
int DIFFICULTY = EASY;
int NUM_OF_FOOD = MAX_FOOD;
int NUM_OF_BLOCK = MAX_BLOCKS;

// -- GAME STATES -- //
// states of the game
int GAME_OVER = 1;
int SCORE = 0;

// -- ARRAYS -- //
// the snake's body
Vector2 SNAKE_ARR[SIZE*SIZE];
int SNAKE_ARR_LEN = 0;

// locations for food
Vector2 FOOD_ARR[SIZE*SIZE];
int FOOD_ARR_LEN = 0;

// locs for walls
Vector2 WALL_ARR[SIZE*SIZE];
int WALL_ARR_LEN = 0;

// locations for blocks
Vector2 BLOCK_ARR[SIZE*SIZE];
int BLOCK_ARR_LEN = 0;

// locations for portals
Vector2 PORTAL_ARR[SIZE*SIZE];
int PORTAL_ARR_LEN = 0;

// the playable area
// a 2D array with possible values of 0 to 255
// AREA[y][x]
char AREA[SIZE][SIZE];

// ------- VECTOR2 FUNCTIONS --------- //

// returns a vector2 with these specific properties
Vector2 v2(int x, int y) {
    return (Vector2) {.x = x, .y = y};
}

// prints an Vector2
void printv2(Vector2 v2) {
    printf("(%d, %d)", v2.x, v2.y);
}

// Arithmetic functions
// Adds two vectors (v1 and v2) together
// vmodifying is a Vector2 pointer (will be modified)
Vector2 v2add(Vector2 v1, Vector2 v2) {
    Vector2 vmodifying;
    vmodifying.x = v1.x + v2.x;
    vmodifying.y = v1.y + v2.y;
    return vmodifying;
}

// Returns the distance of the vector
double v2magnitude(Vector2 v) {
    return sqrt(v.x*v.x + v.y*v.y);
}

// check if two vectors have the exact same components
bool v2equality(Vector2 v1, Vector2 v2) {
    return (v1.x == v2.x && v1.y == v2.y);
}

// generates a random coordinate with bounds
// pass in v2modifying as an address since itll be modified
// (dont forget to initialize srand())
Vector2 v2rand(int x_min, int x_max, int y_min, int y_max) {
    Vector2 v2modifying;
    v2modifying.x = (rand() % (x_max-x_min + 1)) + x_min;
    v2modifying.y = (rand() % (y_max-y_min + 1)) + y_min;
    return v2modifying;
}


// ------- EVEN MORE HELPER FUNCTIONS ------- //

// gets the point in the area
char getareapoint(Vector2 position) {
    return AREA[position.y][position.x];
}

// checks if a given point is within an array
// and if it is, returns the index within the array.
// -1 if it isnt.
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

// pops an element at index.
// shifts all elements left (if at the right of
// where the popped index was)
// arr_len is modified, pass it as an address
void pop(Vector2 array[], int *arr_len, int index) {
    *arr_len -= 1; // subtract the spot the element was occupying
    for (int i = index; i < *arr_len; i++) {
        array[i] = *(array + i+1); // moves x
    }
}

Vector2 wasddirection(int input) {
    if (input < 0) { // invalid
        return v2(0,0);
    }
    // translates the wasd to vector direction
    switch (input) {
        case 'w': return v2(0, -1);
        case 'a': return v2(-1,0);
        case 's': return v2(0, 1);
        case 'd': return v2(1, 0);
    }

    return v2(0,0);
}

// ------- FUNCTIONS ------- //

// updates the area for any changes in
// the array locations.
void updatearea() {
    // first clear everything
    for (int y=0; y<SIZE; y++) {
        for (int x=0; x<SIZE; x++) {
            AREA[y][x] = EMPTY;
        }
    }
    // now place the things
    updateareahelper(WALL);
    updateareahelper(SNAKE);
    updateareahelper(BLOCK);
    updateareahelper(FOOD);
    updateareahelper(PORTAL);
}

// prints the area in the console
// mostly for debugging
void printarea() {
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

// grabs a user input.
// does all the error handling
// returns: int representation of char.
int grabinput() {
    int ch = getch();
    debug("got input: %c [%d]", (char) ch, ch);

    if (ch == 3) { // ctrl+c, keyboardescape. this damn thing makes me unable to forcequit
        error("KeyboardEscape");
    }

    for (int i = 0; i < NUM_SUPPORTED_INPUTS; i++) {
        if (SUPPORTED_INPUTS[i] == ch) {
            return ch;
        }
    }
    // not supported
    return -1;
}

// ------- GAMEPLAY FUNCTIONS ------- //

void resetgamestate() {
    GAME_OVER = true;
    SCORE = 0;
    cleararray(FOOD);
    cleararray(SNAKE);
    cleararray(BLOCK);
    cleararray(PORTAL);
    cleararray(WALL);
}

void gameover() {
    print(".\t\t\t.");
    print("|\tGAME OVER\t|");
    print("|\tSCORE: %03d\t|", SCORE);
    print(".\t\t\t.");
    resetgamestate();
}

void winnerrrr() {
    print(".\t\t\t.");
    print("|\t! YOU WIN !\t|");
    print("|\tSCORE: %03d\t|", SCORE);
    print(".\t\t\t.");
    resetgamestate();
}

// tries to generate n number of food on the map
// checks if the food will be placed on a valid position
void generatefood(int amount) {
    int attempts = 0;
    while (amount > 0) {
        if (attempts > 50) {
            break;
        }
        Vector2 randpos = v2rand(1, 13, 1, 13);
        if (getareapoint(randpos) == EMPTY) {
            placeobject(FOOD, randpos);
            updatearea();
            amount--; \
            attempts = 0;
        } else attempts++;
    }
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
            placeobject(BLOCK, randpos);
            amount--; \
            attempts = 0;
        } else attempts++;
    }
}

void movesnake(Vector2 direction) {
    Vector2 lastpos;// last spot where the previous snakebody was.
    // printv2(direction);

    // check if it ate itself
    // check must be done before movement
    // debug("SNAKE");
    // stupid ass wrong variable 'SNAKE_ARR_LEN' -> 'FOOD_ARR_LEN' cost me like an hour bruh
    int eatensnakeindex = ispointinarray(SNAKE_ARR, SNAKE_ARR_LEN, v2add(SNAKE_ARR[0], direction));

    // Vector2 f = v2add(SNAKE_ARR[0], direction);
    // printf("FUTURE POS: "); printv2(f); printf("\n");
    // for (int i=0; i < SNAKE_ARR_LEN; i++) {
    //     print("SNAKE [%d] ", i); printv2(SNAKE_ARR[i]), printv2(f), printf("%d", (f.x == SNAKE_ARR[i].x && f.y == SNAKE_ARR[i].y)), printf("%d", v2equality(SNAKE_ARR[i], f)); printf("\n");
    // }
    

    for (int i=0; i<SNAKE_ARR_LEN; i++) {
        Vector2 snakebody = SNAKE_ARR[i]; // pos
        // is it the head?
        if (i==0) {
            lastpos = snakebody;
            SNAKE_ARR[i] = v2add(snakebody, direction);
            // debug("New SNAKE POS: "); printv2(SNAKE_ARR[i]); printf("\n");
            continue;
        }
        // if its not the head, then advance
        // the body to where the previous one was
        SNAKE_ARR[i] = lastpos;
        lastpos = snakebody;
    }
    // check if theres food on where the snake is now at
    int eatenfoodindex = ispointinarray(FOOD_ARR, FOOD_ARR_LEN, SNAKE_ARR[0]);
    // check if it ate a block
    int eatenblockindex = ispointinarray(BLOCK_ARR, BLOCK_ARR_LEN, SNAKE_ARR[0]);
    // check if it ate a wall
    int eatenwallindex = ispointinarray(WALL_ARR, WALL_ARR_LEN, SNAKE_ARR[0]);

    // if it successfully ate a food
    if (eatenfoodindex > -1) {
        pop(FOOD_ARR, &FOOD_ARR_LEN, eatenfoodindex);
        debug("POPPED FOOD AT INDEX %d", eatenfoodindex);
        placeobject(SNAKE, lastpos);
        SCORE++;
    }
    // if it ate itself or a block or a wall
    // debug("ESI: %d, EBI: %d", eatensnakeindex, eatenblockindex);
    if ((eatensnakeindex > -1) || (eatenblockindex > -1) || (eatenwallindex > -1)) {
        // game over :(
        gameover();
    }
    

}

// ------- The SUPER DUPER important functions ------- //

// this prompts for game settings
void PROMPT_SETTINGS() {
    print("Choose a gamemode:");
    print("(1) Easy");
    print("(2) Difficult");
    int input;
    returnagain:
    
    input = grabinput();
    switch (input) {
        case '1':
            DIFFICULTY = EASY;
            break;
        case '2':
            DIFFICULTY = DIFFICULT;
            break;
        default:
            // warning("Unsupported input");
            // PROMPT_SETTINGS();
            goto returnagain;
    }
    print("Set gamemode to %s.", DIFFICULTY == EASY ? "EASY" : "DIFFICULT");

    if (DIFFICULTY == DIFFICULT) {
        int blocks = -1;
        print("Number of blocks to generate (between %d and %d):", MIN_BLOCKS, MAX_BLOCKS);
        
        // i tried using while() here but it kept ignoring the fact that scanf should pause the script
        // from future me: weird bug. scanf just stopped working. restarting vscode fixed it. while() would prolly work now but i dont care enough to change it
    difficultyloop:   
        scanf("%d", &blocks);
        if (blocks < MIN_BLOCKS || blocks > MAX_BLOCKS) {
            print("Invalid number of blocks [%d].", blocks);
            goto difficultyloop;

        }
        NUM_OF_BLOCK = blocks;
        print("Number of blocks to be generated: %d", NUM_OF_BLOCK);
    } else {
        NUM_OF_BLOCK = 0;
    }

    {
        int food = -1;
        print("Number of food to generate (between %d and %d):", MIN_FOOD, MAX_FOOD);
        foodloop:   
        scanf("%d", &food);
        if (food < MIN_FOOD || food > MAX_FOOD) {
            print("Invalid number of food [%d].", food);
            goto foodloop;
        }
        NUM_OF_FOOD = food;
        print("Number of food to be generated: %d", NUM_OF_FOOD);
    };

    GAME_LOOP();
}

// this promps for the main menu and settings and stuff 
void MAIN_MENU() {
    // scanf("%d");

    print("Our Snake Game!");
    print("Made by:");
    print("\tme");
    print("Choose an option:");
    print("(1) Start Game");
    print("(2) Instructions");
    print("(3) Exit");

    int input;
    returnhere:
    input = grabinput();
    if (input == -1) {
        warning("Unsupported input");
        MAIN_MENU();
        return;
    }
    switch (input) {
        case '1':
            PROMPT_SETTINGS();
            return;
            // break;
        case '2':
            warning("not yet implemented");
            MAIN_MENU();
            return;
            // break;
        case '3':
            error("Exiting");
            break;
        default:
            goto returnhere; // error("This should never happen. If it did, I suck.");
    }
}

// the actual SNAKE GAME
void GAME_LOOP() {
    // ----- things to do before the game actually starts ----- //
    // initialize super duper important things
    GAME_OVER = false;
    srand(time(NULL)); // random seed
    updatearea();
    
    // Places the blocks surrounding the playing field
    for (int y=0; y<SIZE; y++) {
        for (int x=0; x<SIZE; x++) {
            Vector2 pos = v2(x,y);
            // if its at the top, bottom, or side
            if (y == 0 || y == (SIZE-1) || x == 0 || x == (SIZE - 1)) {
                placeobject(WALL, pos);
                continue;
            }
        }   
    }
    // Places where the snake head is
    placeobject(SNAKE, v2(1, 1));
    updatearea(); // thsi gotta be called so they get placed inside the actual area

    generatefood(NUM_OF_FOOD);
    generateblocks(NUM_OF_BLOCK);
    
    while (!GAME_OVER) {
        updatearea();
        printarea();
        int input = grabinput();

        // quit the game
        if (input == 'q') {
            gameover();
            break;
        }

        Vector2 movedir = wasddirection(input);
        
        if (v2equality(movedir, v2(0,0))) continue; // just skip if it dont move
        // moving the snek
        movesnake(movedir);
        if (GAME_OVER) break; // plr can die but loop wont end yet so do it now

        // if a portal exists, start checking
        // if the entire snake is out of bounds.
        // if it is, win!!!
        if (PORTAL_ARR_LEN > 0) {
            int outside = 0;
            for (int i=0; i<SNAKE_ARR_LEN; i++) {
                Vector2 body = SNAKE_ARR[i];
                if (
                    (body.x > -1 && body.x < (SIZE)) && // didnt add a -1 so that the tail at the end wont peek thru
                    (body.y > -1 && body.y < (SIZE))
                ) continue; // if any inside, continue.
                outside++;
            }
            if (outside >= SNAKE_ARR_LEN) {
                debug("WIn");
                // GAME_OVER = true;
                winnerrrr();
                break;
            }
        }

        // if no more food and no portal yet
        // make one
        if (FOOD_ARR_LEN < 1 && PORTAL_ARR_LEN < 1) {
            debug("HARDPLACE PORTAL");
            // bruh i was wondering why is wasnt at middle right.
            // its because (7, 14) is MIDDLE BOTTOM NOT MIDDLE RIGHT.
            // ma'am put the wrong coordinates T-T
            // hardplaceobject(PORTAL, v2(7,14)); // hardplace a portal
            hardplaceobject(PORTAL, v2(14,7)); // hardplace a portal, just realized this is completely unnecessary since u also gotta clear the blocks.
            cleararray(BLOCK);
            // for (int i=0; i < BLOCK_ARR_LEN; i++) {
            //     print("BLOCK [%d] ", i); printv2(BLOCK_ARR[i]); printf("\n");
            // } < PORTAL_ARR_LEN; i++) {
            //     print("PORTAL [%d] ", i); printv2(PORTAL_ARR[i]); printf("\n");
            // }
        }
    }

    print("Play again?");
    print("(1) Yes");
    print("(2) Exit");
    int input;
nuh_uh: // the lazy method lol
    input = grabinput();

    switch (input) {
        case '1':
            MAIN_MENU();
            break;
        case '2':
            error("Exitting");
        default: 
            goto nuh_uh;
    }
    
    
}

int main() {
    // scanf("%d");
    MAIN_MENU();
    // GAME_LOOP();

    return 0;
}
