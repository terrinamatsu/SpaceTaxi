//build in clang with: $ clang spacetaxi.c -lSDL2 -lGLU -lGL -lm -o spacetaxi 
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//all code enclosed by '    /*////////////////////////////////////////////////////////*/
//comments is code from the example lunar lander game, by Eike Anderson

//gameplay constants::
//window dimensions
#define WINWIDTH     800;
#define WINHEIGHT    600;
//max safe velocity that won't crash the taxi
#define SAFEVELOCITY 100.0;
//force of gravity on taxi
#define GRAVITY      200.0;
//strength of vertical thrusters of taxi
#define VERTICALSTR  400.0;
//strength of horizontal thrusters of taxi
#define HORIZONTSTR  280.0;

typedef struct
{
    //line segment, if taxi collides it crashes and loses a life
    //made of two connected coordinates
    double x1;
    double y1;
    double x2;
    double y2;
}lseg;

typedef struct
{
    //information needed for each customer
    double fare; //how much money currently giving, decrements by 10 (£0.10) each second
    double timeWaiting; //how long the customer has been waiting, increments each second until at destination

    char name[4]; //5 letter name of customer

    int destPlatform; //int 1 to 10 to tell destination platform (10 for top exit)
    int fromPlatform; //int 1 to 10 to tell where the customer starts (10 from previous level)
}customer;

typedef struct
{
    //the landing pads, on which the taxi can land and customers spawn
    lseg l; //contains a line segment for collission detection and coordinates for rendering the line
    char customerWaiting; //flag to show if a customer is waiting on the landing pad 1 true, 0 false
    customer c; //a copy of any customer waiting on the pad,  HDSAHKJDAS//////////////////////////////////////might go unused.?
}lpad;

typedef struct
{
    //bounding box, consisting of two opposite coordinates of a box
    double x1;
    double y1;
    double x2;
    double y2;
}bbox;

typedef struct
{
    //taxi and general game information not specific to any level
    double x; // X coordinate of the taxi
	double y; // Y coordinate of the taxi
    bbox bb; // bounding box of taxi

	double vSpeed; // vertical velocity of taxi
	double hSpeed; // horizontal velocity of taxi
	double vThrust; // vertical thrust flag (-1/0/1)
	double hThrust; // horizontal thrust flag (0/1)  
	char direction; // flag - gives the direction the taxi is currently facing, l for left, r for right
    int velocity; // The total current velocity of the taxi

	char bintact;   // flag - is the lander whole/intact (1) or has it crashed (0)
	char inflight; // flag - is the lander flying (1) or has it landed/crashed (0)
    char wheelLandersEnabled; // if the landers are enabled (1), thus horizontal movement is disabled, else (0)
    
    double fuel;    // the amount of fuel left
    double maxFuel; // the size of the taxi's fuel tank, in case upgraded
    double money;   // how much total the player has earnt during the game
    customer custo_current; // the current customer, if in the taxi
    char custo_waiting; //checks if there is a customer riding the taxi
    
    char landingOnceCustoCheck; // makes sure that customer is checked only once when landed

    char justDied; // flag to check if the taxi just crashed, (1) just crashed, otherwise (0)
    char lives;    // number of lives
    int levelCurrent; // the number of the currently loaded level
    char nextLevel; // flag to say to load next level (1), or not (0)

    char bonusGiven; //flag to check if extra life has been awarded, (1) yes, (0) no
}taxi;

typedef struct
{
    //The level, including the custmer, timer, and the level terrain (landing pads and line segments)
    int rainChance; // chance of the level having rain, percentage (from 0 to 100)
    char isRaining; // after random chance is calculated on level load, if it is now raining (1) or not (0)

    int numbounds;   // number of boundary lines
    int numplatforms; // number of platforms
    lpad platforms[9]; // array of the platforms
    lseg bounds[100]; // array of the boundary lines 

    Uint32 currentTicks; //number of ticks since start of program, for game clock
    int changedTicks; //number of ticks between frames
    int currentTimeSecs;   //current number of seconds since program start

    char customerWaiting;  //flag to check if there is a customer waiting on a platform (1) or not (0)
    int levelCustomerNumber; //the number of customers that the given level has in total, the final being the exit customer
    int levelCustomerCurrentNumber; //the current customer number the player is on, out of the total above
    customer currentCustomer;   //the current customer
}level;

//key functions
void updateTicks(level *lvl) 
{
    //Updates the in game timer and ticks
    Uint32 lastTicks = lvl->currentTicks;
    lvl->currentTicks = SDL_GetTicks();
    lvl->changedTicks = lvl->currentTicks - lastTicks;
    //if number of ticks has incremented by 1000, one second is added to the level timer
    if(lvl->currentTicks/1000 != lvl->currentTimeSecs)
    {
        lvl->currentTimeSecs++;
        lvl->currentCustomer.fare -= 10;
    }
    if(lvl->changedTicks > 20)
    {
        lvl->changedTicks = 20;
    }
}
void printLetter(char ltr, double xPos, double yPos)
{
    //function to print a letter to screen, using GL_LINES, based on letter, and possition perameters
    switch(ltr)
    {
        case 1: case '1':
            glBegin(GL_LINES);
                //right top
                glVertex3d(xPos + 10, yPos - 1, 0.0f);
                glVertex3d(xPos + 10, yPos - 8, 0.0f);
                //right bottom
                glVertex3d(xPos + 10, yPos - 9, 0.0f);
                glVertex3d(xPos + 10, yPos - 16, 0.0f);
            glEnd();
        break;
        case 2: case '2':
            glBegin(GL_LINES);
                //right top
                glVertex3d(xPos + 10, yPos - 1, 0.0f);
                glVertex3d(xPos + 10, yPos - 8, 0.0f);
                //left bottom
                glVertex3d(xPos + 0, yPos - 9, 0.0f);
                glVertex3d(xPos + 0, yPos - 16, 0.0f);
                //top
                glVertex3d(xPos + 1, yPos - 0, 0.0f);
                glVertex3d(xPos + 9, yPos - 0, 0.0f);
                //middle
                glVertex3d(xPos + 1, yPos - 9, 0.0f);
                glVertex3d(xPos + 9, yPos - 9, 0.0f);
                //bottom
                glVertex3d(xPos + 1, yPos - 17, 0.0f);
                glVertex3d(xPos + 9, yPos - 17, 0.0f);
            glEnd();
        break;
        case 3: case '3':
            glBegin(GL_LINES);
                //right top
                glVertex3d(xPos + 10, yPos - 1, 0.0f);
                glVertex3d(xPos + 10, yPos - 8, 0.0f);
                //right bottom
                glVertex3d(xPos + 10, yPos - 9, 0.0f);
                glVertex3d(xPos + 10, yPos - 16, 0.0f);
                //top
                glVertex3d(xPos + 1, yPos - 0, 0.0f);
                glVertex3d(xPos + 9, yPos - 0, 0.0f);
                //middle
                glVertex3d(xPos + 1, yPos - 9, 0.0f);
                glVertex3d(xPos + 9, yPos - 9, 0.0f);
                //bottom
                glVertex3d(xPos + 1, yPos - 17, 0.0f);
                glVertex3d(xPos + 9, yPos - 17, 0.0f);
            glEnd();
        break;
        case 4: case '4':
            glBegin(GL_LINES);
                //right top
                glVertex3d(xPos + 10, yPos - 1, 0.0f);
                glVertex3d(xPos + 10, yPos - 8, 0.0f);
                //right bottom
                glVertex3d(xPos + 10, yPos - 9, 0.0f);
                glVertex3d(xPos + 10, yPos - 16, 0.0f);
                //left top
                glVertex3d(xPos + 0, yPos - 1, 0.0f);
                glVertex3d(xPos + 0, yPos - 8, 0.0f);
                //middle
                glVertex3d(xPos + 1, yPos - 9, 0.0f);
                glVertex3d(xPos + 9, yPos - 9, 0.0f);
            glEnd();
        break;
        case 5: case '5':
            glBegin(GL_LINES);
                //right bottom
                glVertex3d(xPos + 10, yPos - 9, 0.0f);
                glVertex3d(xPos + 10, yPos - 16, 0.0f);
                //left top
                glVertex3d(xPos + 0, yPos - 1, 0.0f);
                glVertex3d(xPos + 0, yPos - 8, 0.0f);
                //top
                glVertex3d(xPos + 1, yPos - 0, 0.0f);
                glVertex3d(xPos + 9, yPos - 0, 0.0f);
                //middle
                glVertex3d(xPos + 1, yPos - 9, 0.0f);
                glVertex3d(xPos + 9, yPos - 9, 0.0f);
                //bottom
                glVertex3d(xPos + 1, yPos - 17, 0.0f);
                glVertex3d(xPos + 9, yPos - 17, 0.0f);
            glEnd();
        break;
        case 6: case '6':
            glBegin(GL_LINES);
                //right bottom
                glVertex3d(xPos + 10, yPos - 9, 0.0f);
                glVertex3d(xPos + 10, yPos - 16, 0.0f);
                //left top
                glVertex3d(xPos + 0, yPos - 1, 0.0f);
                glVertex3d(xPos + 0, yPos - 8, 0.0f);
                //left bottom
                glVertex3d(xPos + 0, yPos - 9, 0.0f);
                glVertex3d(xPos + 0, yPos - 16, 0.0f);
                //top
                glVertex3d(xPos + 1, yPos - 0, 0.0f);
                glVertex3d(xPos + 9, yPos - 0, 0.0f);
                //middle
                glVertex3d(xPos + 1, yPos - 9, 0.0f);
                glVertex3d(xPos + 9, yPos - 9, 0.0f);
                //bottom
                glVertex3d(xPos + 1, yPos - 17, 0.0f);
                glVertex3d(xPos + 9, yPos - 17, 0.0f);
            glEnd();
        break;
        case 7: case '7':
            glBegin(GL_LINES);
                //right top
                glVertex3d(xPos + 10, yPos - 1, 0.0f);
                glVertex3d(xPos + 10, yPos - 8, 0.0f);
                //right bottom
                glVertex3d(xPos + 10, yPos - 9, 0.0f);
                glVertex3d(xPos + 10, yPos - 16, 0.0f);
                //top
                glVertex3d(xPos + 1, yPos - 0, 0.0f);
                glVertex3d(xPos + 9, yPos - 0, 0.0f);
            glEnd();
        break;
        case 8: case '8':
            glBegin(GL_LINES);
                //right top
                glVertex3d(xPos + 10, yPos - 1, 0.0f);
                glVertex3d(xPos + 10, yPos - 8, 0.0f);
                //right bottom
                glVertex3d(xPos + 10, yPos - 9, 0.0f);
                glVertex3d(xPos + 10, yPos - 16, 0.0f);
                //left top
                glVertex3d(xPos + 0, yPos - 1, 0.0f);
                glVertex3d(xPos + 0, yPos - 8, 0.0f);
                //left bottom
                glVertex3d(xPos + 0, yPos - 9, 0.0f);
                glVertex3d(xPos + 0, yPos - 16, 0.0f);
                //top
                glVertex3d(xPos + 1, yPos - 0, 0.0f);
                glVertex3d(xPos + 9, yPos - 0, 0.0f);
                //middle
                glVertex3d(xPos + 1, yPos - 9, 0.0f);
                glVertex3d(xPos + 9, yPos - 9, 0.0f);
                //bottom
                glVertex3d(xPos + 1, yPos - 17, 0.0f);
                glVertex3d(xPos + 9, yPos - 17, 0.0f);
            glEnd();
        break;
        case 9: case '9':
            glBegin(GL_LINES);
                //right top
                glVertex3d(xPos + 10, yPos - 1, 0.0f);
                glVertex3d(xPos + 10, yPos - 8, 0.0f);
                //right bottom
                glVertex3d(xPos + 10, yPos - 9, 0.0f);
                glVertex3d(xPos + 10, yPos - 16, 0.0f);
                //left top
                glVertex3d(xPos + 0, yPos - 1, 0.0f);
                glVertex3d(xPos + 0, yPos - 8, 0.0f);
                //top
                glVertex3d(xPos + 1, yPos - 0, 0.0f);
                glVertex3d(xPos + 9, yPos - 0, 0.0f);
                //middle
                glVertex3d(xPos + 1, yPos - 9, 0.0f);
                glVertex3d(xPos + 9, yPos - 9, 0.0f);
                //bottom
                glVertex3d(xPos + 1, yPos - 17, 0.0f);
                glVertex3d(xPos + 9, yPos - 17, 0.0f);
            glEnd();
        break;
        case 0: case '0':
            glBegin(GL_LINES);
                //right top
                glVertex3d(xPos + 10, yPos - 1, 0.0f);
                glVertex3d(xPos + 10, yPos - 8, 0.0f);
                //right bottom
                glVertex3d(xPos + 10, yPos - 9, 0.0f);
                glVertex3d(xPos + 10, yPos - 16, 0.0f);
                //left top
                glVertex3d(xPos + 0, yPos - 1, 0.0f);
                glVertex3d(xPos + 0, yPos - 8, 0.0f);
                //left bottom
                glVertex3d(xPos + 0, yPos - 9, 0.0f);
                glVertex3d(xPos + 0, yPos - 16, 0.0f);
                //top
                glVertex3d(xPos + 1, yPos - 0, 0.0f);
                glVertex3d(xPos + 9, yPos - 0, 0.0f);
                //bottom
                glVertex3d(xPos + 1, yPos - 17, 0.0f);
                glVertex3d(xPos + 9, yPos - 17, 0.0f);
            glEnd();
        break;
        case 'a': case 'A':
            glBegin(GL_LINES);
                //full right
                glVertex3d(xPos + 10, yPos - 0, 0.0f);
                glVertex3d(xPos + 10, yPos - 17, 0.0f);
                //full left
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 0, yPos - 17, 0.0f);
                //top
                glVertex3d(xPos + 1, yPos - 0, 0.0f);
                glVertex3d(xPos + 9, yPos - 0, 0.0f);
                //middle
                glVertex3d(xPos + 1, yPos - 9, 0.0f);
                glVertex3d(xPos + 9, yPos - 9, 0.0f);
            glEnd();
        break;
        case 'b': case 'B':
            glBegin(GL_LINES);
                //right top
                glVertex3d(xPos + 10, yPos - 1, 0.0f);
                glVertex3d(xPos + 10, yPos - 8, 0.0f);
                //right bottom
                glVertex3d(xPos + 10, yPos - 9, 0.0f);
                glVertex3d(xPos + 10, yPos - 16, 0.0f);
                //full left
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 0, yPos - 17, 0.0f);
                //top
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 9, yPos - 0, 0.0f);
                //middle
                glVertex3d(xPos + 1, yPos - 9, 0.0f);
                glVertex3d(xPos + 9, yPos - 9, 0.0f);
                //bottom
                glVertex3d(xPos + 1, yPos - 17, 0.0f);
                glVertex3d(xPos + 10, yPos - 17, 0.0f);
            glEnd();
        break;
        case 'c': case 'C':
            glBegin(GL_LINES);
                //full left
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 0, yPos - 16, 0.0f);
                //top
                glVertex3d(xPos + 1, yPos - 0, 0.0f);
                glVertex3d(xPos + 9, yPos - 0, 0.0f);
                //bottom
                glVertex3d(xPos + 1, yPos - 17, 0.0f);
                glVertex3d(xPos + 9, yPos - 17, 0.0f);
            glEnd();
        break;
        case 'd': case 'D':
            glBegin(GL_LINES);
                //full right
                glVertex3d(xPos + 10, yPos - 0, 0.0f);
                glVertex3d(xPos + 10, yPos - 16, 0.0f);
                //full left
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 0, yPos - 17, 0.0f);
                //top
                glVertex3d(xPos + 1, yPos - 0, 0.0f);
                glVertex3d(xPos + 9, yPos - 0, 0.0f);
                //bottom
                glVertex3d(xPos + 1, yPos - 17, 0.0f);
                glVertex3d(xPos + 9, yPos - 17, 0.0f);
            glEnd();
        break;
        case 'e': case 'E':
            glBegin(GL_LINES);
                //full left
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 0, yPos - 17, 0.0f);
                //top
                glVertex3d(xPos + 1, yPos - 0, 0.0f);
                glVertex3d(xPos + 9, yPos - 0, 0.0f);
                //middle
                glVertex3d(xPos + 1, yPos - 9, 0.0f);
                glVertex3d(xPos + 9, yPos - 9, 0.0f);
                //bottom
                glVertex3d(xPos + 1, yPos - 17, 0.0f);
                glVertex3d(xPos + 9, yPos - 17, 0.0f);
            glEnd();
        break;
        case 'f': case 'F':
            glBegin(GL_LINES);
                //full left
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 0, yPos - 17, 0.0f);
                //top
                glVertex3d(xPos + 1, yPos - 0, 0.0f);
                glVertex3d(xPos + 9, yPos - 0, 0.0f);
                //middle
                glVertex3d(xPos + 1, yPos - 9, 0.0f);
                glVertex3d(xPos + 9, yPos - 9, 0.0f);
            glEnd();
        break;
        case 'g': case 'G':
            glBegin(GL_LINES);
                //right bottom
                glVertex3d(xPos + 10, yPos - 9, 0.0f);
                glVertex3d(xPos + 10, yPos - 16, 0.0f);
                //full left
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 0, yPos - 16, 0.0f);
                //top
                glVertex3d(xPos + 1, yPos - 0, 0.0f);
                glVertex3d(xPos + 9, yPos - 0, 0.0f);
                //middle
                glVertex3d(xPos + 7, yPos - 9, 0.0f);
                glVertex3d(xPos + 9, yPos - 9, 0.0f);
                //bottom
                glVertex3d(xPos + 1, yPos - 17, 0.0f);
                glVertex3d(xPos + 9, yPos - 17, 0.0f);
            glEnd();
        break;
        case 'h': case 'H':
            glBegin(GL_LINES);
                //full right
                glVertex3d(xPos + 10, yPos - 0, 0.0f);
                glVertex3d(xPos + 10, yPos - 17, 0.0f);
                //full left
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 0, yPos - 17, 0.0f);
                //middle
                glVertex3d(xPos + 1, yPos - 9, 0.0f);
                glVertex3d(xPos + 9, yPos - 9, 0.0f);
            glEnd();
        break;
        case 'i': case 'I':
            glBegin(GL_LINES);
                //full centre
                glVertex3d(xPos + 5, yPos - 0, 0.0f);
                glVertex3d(xPos + 5, yPos - 17, 0.0f);
                //top
                glVertex3d(xPos + 1, yPos - 0, 0.0f);
                glVertex3d(xPos + 9, yPos - 0, 0.0f);
                //bottom
                glVertex3d(xPos + 1, yPos - 17, 0.0f);
                glVertex3d(xPos + 9, yPos - 17, 0.0f);
            glEnd();
        break;
        case 'j': case 'J':
            glBegin(GL_LINES);
                //right top
                glVertex3d(xPos + 10, yPos - 1, 0.0f);
                glVertex3d(xPos + 10, yPos - 8, 0.0f);
                //right bottom
                glVertex3d(xPos + 10, yPos - 9, 0.0f);
                glVertex3d(xPos + 10, yPos - 16, 0.0f);
                //full right
                glVertex3d(xPos + 10, yPos - 0, 0.0f);
                glVertex3d(xPos + 10, yPos - 17, 0.0f);
                //left top
                glVertex3d(xPos + 0, yPos - 1, 0.0f);
                glVertex3d(xPos + 0, yPos - 8, 0.0f);
                //left bottom
                glVertex3d(xPos + 0, yPos - 9, 0.0f);
                glVertex3d(xPos + 0, yPos - 16, 0.0f);
                //full left
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 0, yPos - 17, 0.0f);
                //top
                glVertex3d(xPos + 1, yPos - 0, 0.0f);
                glVertex3d(xPos + 9, yPos - 0, 0.0f);
                //middle
                glVertex3d(xPos + 1, yPos - 9, 0.0f);
                glVertex3d(xPos + 9, yPos - 9, 0.0f);
                //bottom
                glVertex3d(xPos + 1, yPos - 17, 0.0f);
                glVertex3d(xPos + 9, yPos - 17, 0.0f);
            glEnd();
        break;
        case 'k': case 'K':
            glBegin(GL_LINES);
                //right top
                glVertex3d(xPos + 1, yPos - 8, 0.0f);
                glVertex3d(xPos + 10, yPos - 1, 0.0f);
                //right bottom
                glVertex3d(xPos + 1, yPos - 9, 0.0f);
                glVertex3d(xPos + 10, yPos - 16, 0.0f);
                //full left
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 0, yPos - 17, 0.0f);
            glEnd();
        break;
        case 'l': case 'L':
            glBegin(GL_LINES);
                //full left
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 0, yPos - 17, 0.0f);
                //bottom
                glVertex3d(xPos + 1, yPos - 17, 0.0f);
                glVertex3d(xPos + 9, yPos - 17, 0.0f);
            glEnd();
        break;
        case 'm': case 'M':
            glBegin(GL_LINES);
                //bottom left to top middle diagonal
                glVertex3d(xPos + 0, yPos - 17, 0.0f);
                glVertex3d(xPos + 3, yPos - 0, 0.0f);

                glVertex3d(xPos + 3, yPos - 0, 0.0f);
                glVertex3d(xPos + 5, yPos - 17, 0.0f);
                glVertex3d(xPos + 5, yPos - 17, 0.0f);
                glVertex3d(xPos + 8, yPos - 0, 0.0f);

                //top middle to bottom right diagonal
                glVertex3d(xPos + 8, yPos - 0, 0.0f);
                glVertex3d(xPos + 10, yPos -17, 0.0f);
            glEnd();
        break;
        case 'n': case 'N':
            glBegin(GL_LINES);
                //full right
                glVertex3d(xPos + 10, yPos - 0, 0.0f);
                glVertex3d(xPos + 10, yPos - 17, 0.0f);
                //full left
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 0, yPos - 17, 0.0f);
                //top left to bottom right diagonal
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 10, yPos - 17, 0.0f);
            glEnd();
        break;
        case 'o': case 'O':
            glBegin(GL_LINES);
                //full right
                glVertex3d(xPos + 10, yPos - 0, 0.0f);
                glVertex3d(xPos + 10, yPos - 17, 0.0f);
                //full left
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 0, yPos - 17, 0.0f);
                //top
                glVertex3d(xPos + 1, yPos - 0, 0.0f);
                glVertex3d(xPos + 9, yPos - 0, 0.0f);
                //bottom
                glVertex3d(xPos + 1, yPos - 17, 0.0f);
                glVertex3d(xPos + 9, yPos - 17, 0.0f);
            glEnd();
        break;
        case 'p': case 'P':
            glBegin(GL_LINES);
                //right top
                glVertex3d(xPos + 10, yPos - 1, 0.0f);
                glVertex3d(xPos + 10, yPos - 8, 0.0f);     
                //full left
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 0, yPos - 17, 0.0f);
                //top
                glVertex3d(xPos + 1, yPos - 0, 0.0f);
                glVertex3d(xPos + 9, yPos - 0, 0.0f);
                //middle
                glVertex3d(xPos + 1, yPos - 9, 0.0f);
                glVertex3d(xPos + 9, yPos - 9, 0.0f);
            glEnd();
        break;
        case 'q': case 'Q':
            glBegin(GL_LINES);
                //full right
                glVertex3d(xPos + 10, yPos - 0, 0.0f);
                glVertex3d(xPos + 10, yPos - 17, 0.0f);
                //full left
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 0, yPos - 16, 0.0f);
                //top
                glVertex3d(xPos + 1, yPos - 0, 0.0f);
                glVertex3d(xPos + 9, yPos - 0, 0.0f);
                //bottom
                glVertex3d(xPos + 1, yPos - 17, 0.0f);
                glVertex3d(xPos + 9, yPos - 17, 0.0f);
                //right diagonal
                glVertex3d(xPos + 5, yPos - 9, 0.0f);
                glVertex3d(xPos + 11, yPos - 18, 0.0f);
            glEnd();
        break;
        case 'r': case 'R':
            glBegin(GL_LINES);
                //right top
                glVertex3d(xPos + 10, yPos - 1, 0.0f);
                glVertex3d(xPos + 10, yPos - 8, 0.0f);
                //right bottom
                glVertex3d(xPos + 10, yPos - 9, 0.0f);
                glVertex3d(xPos + 10, yPos - 17, 0.0f);
                //full left
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 0, yPos - 17, 0.0f);
                //top
                glVertex3d(xPos + 1, yPos - 0, 0.0f);
                glVertex3d(xPos + 9, yPos - 0, 0.0f);
                //middle
                glVertex3d(xPos + 1, yPos - 9, 0.0f);
                glVertex3d(xPos + 9, yPos - 9, 0.0f);
            glEnd();
        break;
        case 's': case 'S':
            glBegin(GL_LINES);
                //right bottom
                glVertex3d(xPos + 10, yPos - 9, 0.0f);
                glVertex3d(xPos + 10, yPos - 16, 0.0f);
                //left top
                glVertex3d(xPos + 0, yPos - 1, 0.0f);
                glVertex3d(xPos + 0, yPos - 8, 0.0f);
                //top
                glVertex3d(xPos + 1, yPos - 0, 0.0f);
                glVertex3d(xPos + 9, yPos - 0, 0.0f);
                //middle
                glVertex3d(xPos + 1, yPos - 9, 0.0f);
                glVertex3d(xPos + 9, yPos - 9, 0.0f);
                //bottom
                glVertex3d(xPos + 1, yPos - 17, 0.0f);
                glVertex3d(xPos + 9, yPos - 17, 0.0f);
            glEnd();
        break;
        case 't': case 'T':
            glBegin(GL_LINES);
                //full centre
                glVertex3d(xPos + 5, yPos - 0, 0.0f);
                glVertex3d(xPos + 5, yPos - 17, 0.0f);
                //top
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 10, yPos - 0, 0.0f);
            glEnd();
        break;
        case 'u': case 'U':
            glBegin(GL_LINES);
                //full right
                glVertex3d(xPos + 10, yPos - 0, 0.0f);
                glVertex3d(xPos + 10, yPos - 17, 0.0f);
                //full left
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 0, yPos - 16, 0.0f);
                //bottom
                glVertex3d(xPos + 1, yPos - 17, 0.0f);
                glVertex3d(xPos + 9, yPos - 17, 0.0f);
            glEnd();
        break;
        case 'v': case 'V':
            glBegin(GL_LINES);
                //top left to bottom middle diagonal
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 5, yPos - 17, 0.0f);
                //bottom middle to top right diagonal
                glVertex3d(xPos + 5, yPos - 17, 0.0f);
                glVertex3d(xPos + 10, yPos - 0, 0.0f);
            glEnd();
        break;
        case 'w': case 'W':
            glBegin(GL_LINES);
                //top left to bottom middle diagonal
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 3, yPos - 17, 0.0f);

                glVertex3d(xPos + 3, yPos - 17, 0.0f);
                glVertex3d(xPos + 5, yPos - 0, 0.0f);
                glVertex3d(xPos + 5, yPos - 0, 0.0f);
                glVertex3d(xPos + 8, yPos - 17, 0.0f);

                //bottom middle to top right diagonal
                glVertex3d(xPos + 8, yPos - 17, 0.0f);
                glVertex3d(xPos + 10, yPos - 0, 0.0f);
            glEnd();
        break;
        case 'x': case 'X':
            glBegin(GL_LINES);
                //top left to bottom right diagonal
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 10, yPos - 17, 0.0f);
                //bottom left to top right diagonal
                glVertex3d(xPos + 0, yPos - 17, 0.0f);
                glVertex3d(xPos + 10, yPos - 0, 0.0f);
            glEnd();
        break;
        case 'y': case 'Y':
            glBegin(GL_LINES);
                //top left to middle diagonal
                glVertex3d(xPos + 0, yPos - 0, 0.0f);
                glVertex3d(xPos + 5, yPos - 8, 0.0f);
                //middle to top right diagonal
                glVertex3d(xPos + 5, yPos - 8, 0.0f);
                glVertex3d(xPos + 10, yPos - 0, 0.0f);
                //left bottom
                glVertex3d(xPos + 5, yPos - 9, 0.0f);
                glVertex3d(xPos + 5, yPos - 16, 0.0f);
            glEnd();
        break;
        case 'z': case 'Z':
            glBegin(GL_LINES);
                //bottom left to top right diagonal
                glVertex3d(xPos + 0, yPos - 17, 0.0f);
                glVertex3d(xPos + 10, yPos - 0, 0.0f);
                //top
                glVertex3d(xPos + 1, yPos - 0, 0.0f);
                glVertex3d(xPos + 9, yPos - 0, 0.0f);
                //bottom
                glVertex3d(xPos + 1, yPos - 17, 0.0f);
                glVertex3d(xPos + 9, yPos - 17, 0.0f);
            glEnd();
        break;
        case '.':
            glBegin(GL_LINES);
                glVertex3d(xPos + 0, yPos - 15, 0.0f);
                glVertex3d(xPos + 0, yPos - 17, 0.0f);
            glEnd();
        break;
        case '!':
            glBegin(GL_LINES);
                //left top
                glVertex3d(xPos + 5, yPos - 1, 0.0f);
                glVertex3d(xPos + 5, yPos - 12, 0.0f);
                //dot
                glVertex3d(xPos + 5, yPos - 15, 0.0f);
                glVertex3d(xPos + 5, yPos - 17, 0.0f);
            glEnd();
        break;
    }  
}


void createPlatform(level *lvl, int xStart, int xEnd, int yStart, int yEnd, int num)
{
    //function to create a platform and add it to the level struct, based on the platform start and end coordinates
    //automatically fills in the unserside line segments

    lseg t; //temp line segment

    /* first the landing pads */
	t.x1 = xStart;
	t.y1 = yStart;
	t.x2 = xEnd;
	t.y2 = yEnd;
	lvl->platforms[lvl->numplatforms].l=t; 
	
    /* then the undersides of the landing pads */
	t.x1 = xStart;
	t.y1 = yStart - 1.0;
	t.x2 = xStart + 30;
	t.y2 = yStart - 30;
	lvl->bounds[lvl->numbounds]=t;
	t.x1 = xStart + 30;
	t.y1 = yStart - 30;
	t.x2 = xEnd - 30;
	t.y2 = yEnd - 30;
	lvl->bounds[lvl->numbounds + 1]=t;
	t.x1 = xEnd - 30;
	t.y1 = yEnd - 30;
	t.x2 = xEnd;
	t.y2 = yEnd - 1;
	lvl->bounds[lvl->numbounds + 2]=t;

    //increment the numbers of boundary lines and landing platforms
    lvl->numbounds += 3;
    lvl->numplatforms += 1;
}


void waitForSecs(int secs, char stillRenderScene)
{
    //function to make the program wait for a given number of seconds
    SDL_Delay(secs * 1000);
}
void initialiseTaxi(taxi *t, double x, double y, char lives)
{
    //function to automatically initialise the given taxi struct
    t->x = x;
    t->y = y;
    t->bb.x1=-26.0;
	t->bb.y1=-9.0;
	t->bb.x2= 25.0;
	t->bb.y2= 12.0;

    t->vSpeed = 0;
    t->hSpeed = 0;
    t->vThrust = 0;
    t->hThrust = 0;
    t->direction = 'r';
    t->velocity = 0;

    t->bintact = '1';
    t->inflight = '1';
    t->wheelLandersEnabled = '1';

    t->maxFuel = 300.0000;
    t->fuel = t->maxFuel;
    t->money = 0.00;
    t->custo_waiting = '0';

    t->justDied = '0';
    t->lives = lives;
    t->levelCurrent = 1;

    t->bonusGiven = 0;
}
void initialiseCustomer(customer *custo, int destpad, int currentpad)
{
    //function to automatically initialise the given customer struct
    custo->fare = rand()%1000 + 1000;
    custo->timeWaiting = 0;
    custo->destPlatform = destpad;
    custo->fromPlatform = currentpad;
}
void initialiseBBox(bbox *b, int ax1, int ay1, int ax2, int ay2)
{
    //function to initialise the given bounding box with given coordinates
    b->x1 = ax1;
    b->y1 = ay1;
    b->x2 = ax2;
    b->y2 = ay2;
}
bbox getBBox(taxi *t)
{
    //function to create a bouding box based on the taxi's bounding box, and then return it
    bbox b;
    b.x1 = t->bb.x1+t->x;
    b.y1 = t->bb.y1+t->y;
    b.x2 = t->bb.x2+t->x;
    b.y2 = t->bb.y2+t->y;
    return b;
}
int intersectBBxBB(bbox A, bbox B)
{
    /* identify intersection between two bounding boxes, returning 1 for intersection, 0 otherwise */
    if((A.x1 >= B.x2 || A.x2 <= B.x1 || A.y1 >= B.y2 || A.y2 <= B.y1) || (B.x1 >= A.x2 || B.x2 <= A.x1 || B.y1 >= A.y2 || B.y2 <= A.y1))
    {
        return 0;
    }
    return 1;
}
int intersectLxL(lseg L1, lseg L2)
{
    /*////////////////////////////////////////////////////////*/
    /* basic line-line intersection maths */
	double D1 = ( (L2.x2-L2.x1)*(L1.y1-L2.y1) - (L2.y2-L2.y1)*(L1.x1-L2.x1) ) / ( (L2.y2-L2.y1)*(L1.x2-L1.x1) - (L2.x2-L2.x1)*(L1.y2-L1.y1) );
	double D2 = ( (L1.x2-L1.x1)*(L1.y1-L2.y1) - (L1.y2-L1.y1)*(L1.x1-L2.x1) ) / ( (L2.y2-L2.y1)*(L1.x2-L1.x1) - (L2.x2-L2.x1)*(L1.y2-L1.y1) );
	if (D1 >= 0 && D1 <= 1 && D2 >= 0 && D2 <= 1) return 1; /* intersection */
	return 0;
    /*////////////////////////////////////////////////////////*/
}
int intersectBBxL(bbox B, lseg Line)
{
    /*////////////////////////////////////////////////////////*/
    int top=0, bottom=0, left=0, right=0; /* intersection with a side of the box */
	lseg tLine; /* line segment for a side of the box */
	
	/* generate a side */
	tLine.x1 =B.x1; 
	tLine.y1 =B.y2;
	tLine.x2 =B.x2;
	tLine.y2 =B.y2;
	bottom = intersectLxL(tLine,Line); /* test if it intersects */
	
	/* generate a side */
	tLine.x1 =B.x1;
	tLine.y1 =B.y1;
	tLine.x2 =B.x2;
	tLine.y2 =B.y1;
	top    = intersectLxL(tLine,Line); /* test if it intersects */
	
	/* generate a side */
	tLine.x1 =B.x1;
	tLine.y1 =B.y1;
	tLine.x2 =B.x1;
	tLine.y2 =B.y2;
	left   = intersectLxL(tLine,Line); /* test if it intersects */
	
	/* generate a side */
	tLine.x1 =B.x2;
	tLine.y1 =B.y1;
	tLine.x2 =B.x2;
	tLine.y2 =B.y2;
	right  = intersectLxL(tLine,Line); /* test if it intersects */
	
	if(bottom || top || left || right) return 1; /* mark collision/intersection if one of the sides has an intersection */
	
    return 0;
    /*////////////////////////////////////////////////////////*/
}
void customerMakeNext(level *lvl, int notPlatform, char death)
{
    //function to initialise the next customer in the level struct
    int r = notPlatform;
    //sets the destination platform to 1 if there is only one platform, or to a random number if more than one
    if(lvl->numplatforms == 1)
    {
        r = 1;
    }
    else
    {
        //random number for destination platform is based on the number of platforms
        while(r == notPlatform)
        {
            r = (rand() % (lvl->numplatforms));
            printf("here %d! \n", r);  
        }
    }
    int c;

    //sets the destination platform to go up if the customer counter for the level has been reached
    //otherwise, the customer's staring platform is randomised

    if(death == 'f')
    {
        lvl->levelCustomerCurrentNumber ++;
    }

    if(lvl->levelCustomerCurrentNumber == lvl->levelCustomerNumber)
    {
        r = lvl->numplatforms + 1;
    }
    else
    {
        //might not be working, but meant to loop while the from and destination platforms are the same
        c = r;
        //while(c == r)
        {
            c = (rand() % (lvl->numplatforms - 1) + 1);
        }
    }

    //makes sure the customer is waiting
    lvl->customerWaiting = '1';

    //initialises the customer, and adds it to the level
    customer custo;
    initialiseCustomer(&custo, r, c);
    lvl->currentCustomer = custo;



    ///////// here!!!! make it a list of numbes, shuffled, and the first two numbers picked...
}

void newCustomerMakeNext(level *lvl)
{
    int i, j[99], k, l, y = lvl->numplatforms;

    for(i = 0; i < y; i++)
    {
        j[i] = i;
    }

    for(i = 0; i < 100; i++)
    {
        k = (rand() %  y);
        l = j[i];
        j[i] = j[k];
        j[k] = l;
    }
}

void customerReDestination(customer c)
{

}

void doEvents(taxi *t, double frames, level *lvl)
{
    //this is the place where all the main calculations like movement take place
    
    updateTicks(lvl);
    int windowHeight = WINHEIGHT;
    double h = HORIZONTSTR;double v = VERTICALSTR;double g = GRAVITY;
    bbox b = getBBox(t);

    lpad *pad = lvl->platforms;
    
    //checks that the taxi hasn't crashed before calculations
    if(t->bintact == '1')
    {
        //movement calculations
        t->vSpeed-=frames*g;
        t->vSpeed+=frames*t->vThrust*v;
        t->hSpeed+=frames*t->hThrust*h;
        t->velocity = sqrt((t->vSpeed * t->vSpeed) + (t->hSpeed * t->hSpeed));

        if(t->inflight == '1')
        {
            //only calculates movement if not on a platform
            t->y+=frames*t->vSpeed;
            t->x+=frames*t->hSpeed;
        }
        
        //hit detection
        int i;
        int safeVel = SAFEVELOCITY;
        //landing pads
        for(i=0;i<lvl->numplatforms;i++)
        {
            if(intersectBBxL(b, pad[i].l) == 1)
            {
                if(t->wheelLandersEnabled == '1' && t->velocity <= safeVel)
                {
                    t->inflight = '0';
                    t->vSpeed = 10.0;
                    t->hThrust = 0;
                    t->hSpeed = 0;
                    int y;
                    y = pad[i].l.y1 + (pad[i].l.y2 - pad[i].l.y1)*(t->x-pad[i].l.x1/pad[i].l.x2 - pad[i].l.x1) + 10; 
                    t->y=y;

                    // customer waiting on platform check
                    if(i == lvl->currentCustomer.fromPlatform && t->landingOnceCustoCheck == '0')
                    {
                        t->custo_current = lvl->currentCustomer;
                        t->custo_waiting = '1';
                        lvl->customerWaiting = '0';
                        printf("\r peach has got it %d",lvl->currentCustomer.destPlatform);
                        fflush(stdout);
                        t->landingOnceCustoCheck = '1';
                    }
                    
                    if(t->custo_waiting == '1') // check if there is a customer in the taxi
                    {
                        if(lvl->currentCustomer.destPlatform == i)
                        {
                            t->custo_waiting = '0';
                            t->money += t->custo_current.fare;
                            customerMakeNext(lvl, t->custo_current.destPlatform, 'f');
                            t->landingOnceCustoCheck = '0';
                        }
                    }
                }
                else
                {
                    t->justDied = '1';
                    t->inflight = '0';
                    t->bintact = '0';
                }
                
            }
        } 
        //boundary lines
        for(i=0;i<lvl->numbounds;i++)
        {
            if(intersectBBxL(b, lvl->bounds[i]) == 1)
            {
                t->justDied = '1';
                t->inflight = '0';
                t->bintact = '0';
            }
        }
        //exit level above
        if(b.y1 > 600)
        {
            t->nextLevel = '1';
        }
    }

}
void drawTaxi(taxi *t, level *lvl)
{
    //this function draws the taxi to screen

    /*////////////////////////////////////////////////////////*/
    GLint matrixmode=0;
	glGetIntegerv(GL_MATRIX_MODE,&matrixmode); /* get current matrix mode */
	
	glMatrixMode(GL_MODELVIEW); /* set the modelview matrix */
	glPushMatrix(); /* store current modelview matrix */
	glTranslated(t->x,t->y,0.0); /* move the lander to its correct position */
    /*////////////////////////////////////////////////////////*/


    //sets the direction of the taxi based on horizontal thruster direction
    if(t->hThrust < 0)
	{
		t->direction = 'l';
	}
	else if(t->hThrust > 0)
	{
		t->direction = 'r';
	}


    //sets colour of taxi
    if(t->bintact == '0')
    {
        glColor3d(1.0,0.0,0.0); //Taxi is red if destroyed.
    }
    else if(lvl->customerWaiting == '1')
    {
        glColor3d(1.0,1.0,0.0); /* Taxi is yellow if noone is riding*/
    }
    else
    {
        glColor3d(0.0,1.0,0.0); /* Taxi is green if someone is riding*/
    }
	if(t->direction == 'l')
	{
           if(t->wheelLandersEnabled == '0')
           {
               glBegin(GL_TRIANGLE_FAN); /* draw Taxi */
	    		glVertex3d(13.0,12.0,0.0);
	    		glVertex3d(16.0,3.0,0.0);
	    		glVertex3d(25.0,3.0,0.0);
			    glVertex3d(25.0,-6.0,0.0); //values here have been - 1 to center
	    		glVertex3d(-26.0,-6.0,0.0);
    			glVertex3d(-26.0,0.0,00);
   				glVertex3d(-14.0,3.0,0.0);
			    glVertex3d(-8.0,12.0,0.0);
		    glEnd();
           }
           else
           {
               glBegin(GL_TRIANGLE_FAN); /* draw Taxi with landing wheels */
	    		glVertex3d(13.0,12.0,0.0);
	    		glVertex3d(16.0,3.0,0.0);
	    		glVertex3d(25.0,3.0,0.0);
			    glVertex3d(25.0,-6.0,0.0); //values here have been - 1 to center
		    	glVertex3d(22.0,-6.0,0.0);
	    		glVertex3d(17.0,-9.0,0.0);
		    	glVertex3d(13.0,-9.0,0.0);
	    		glVertex3d(10.0,-6.0,0.0);
		    	glVertex3d(-5.0,-6.0,0.0);
	    		glVertex3d(-8.0,-9.0,0.0);
			    glVertex3d(-14.0,-9.0,0.0);
		    	glVertex3d(-17.0,-6.0,0.0);
	    		glVertex3d(-26.0,-6.0,0.0);
    			glVertex3d(-26.0,0.0,00);
   				glVertex3d(-14.0,3.0,0.0);
			    glVertex3d(-8.0,12.0,0.0);
		    glEnd();
           }
	}
	else
	{
           if(t->wheelLandersEnabled == '0')
           {
	    	glBegin(GL_TRIANGLE_FAN); /* draw Taxi */
    			glVertex3d(-13.0,12.0,0.0);
		    	glVertex3d(-16.0,3.0,0.0);
		    	glVertex3d(-25.0,3.0,0.0);
	    		glVertex3d(-25.0,-6.0,0.0);  //values here have been + 1 to center
		    	glVertex3d(26.0,-6.0,0.0);
    			glVertex3d(26.0,0.0,00);
    			glVertex3d(14.0,3.0,0.0);
			    glVertex3d(8.0,12.0,0.0);
		    glEnd();            
           }
           else
           {
	    	glBegin(GL_TRIANGLE_FAN); /* draw Taxi with landing wheels */
    			glVertex3d(-13.0,12.0,0.0);
		    	glVertex3d(-16.0,3.0,0.0);
		    	glVertex3d(-25.0,3.0,0.0);
	    		glVertex3d(-25.0,-6.0,0.0);
    			glVertex3d(-22.0,-6.0,0.0);
    			glVertex3d(-19.0,-9.0,0.0);
    			glVertex3d(-13.0,-9.0,0.0); //values here have been + 1 to center
	    		glVertex3d(-10.0,-6.0,0.0);
	    		glVertex3d(5.0,-6.0,0.0);
    			glVertex3d(8.0,-9.0,0.0);
   				glVertex3d(14.0,-9.0,0.0);
	    		glVertex3d(17.0,-6.0,0.0);
		    	glVertex3d(26.0,-6.0,0.0);
    			glVertex3d(26.0,0.0,00);
    			glVertex3d(14.0,3.0,0.0);
			    glVertex3d(8.0,12.0,0.0);
		    glEnd();                
           }

	}

}
void render(taxi *t, level *lvl)
{
    //this function draws all the level information to the screen
    int i;

   	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //adds rain, if the level has it on
    if(lvl->isRaining == '1')
    {
        //adds randomised blue lines to the screen each frame
        glColor3d(0.0,1.0,1.0); //the rain is blue
        for(i=0;i<40;i++)
        {
            int k = rand()%800;
            int e = rand()%500 + 100;
            glBegin(GL_LINES);
                glVertex3d(k,e,0.0);
                glVertex3d(k+5,e+5,0.0);
            glEnd();
        }
    }

    // draw lines
    // first the boundary lines...
    glColor3d(0.5,0.5,0.5);
    for(i=0;i<lvl->numbounds;i++)
    {
        glBegin(GL_LINES);
            glVertex3d(lvl->bounds[i].x1,lvl->bounds[i].y1,0.0f);
            glVertex3d(lvl->bounds[i].x2,lvl->bounds[i].y2,0.0f);
        glEnd();
    }
    // ...then the landing platforms
    glColor3d(0.0,1.0,0.0);
    for(i=0;i<lvl->numplatforms;i++)
    {
        //sets the colour of the landing platform
        if(lvl->customerWaiting == '0' && lvl->currentCustomer.destPlatform == i)
        {
            glColor3d(0.0,1.0,0.0); //bright green if it is the destination platform
        }
        else
        {
            glColor3d(0.0,1.0,0.5); //duller green if not
        }
        //print the platform number
        printLetter(i + 1, lvl->platforms[i].l.x1 + 30, lvl->platforms[i].l.y1 - 9);
        //draw the platform line
        glBegin(GL_LINES);
            glVertex3d(lvl->platforms[i].l.x1,lvl->platforms[i].l.y1,0.0f);
            glVertex3d(lvl->platforms[i].l.x2,lvl->platforms[i].l.y1,0.0f);
        glEnd();
        //checks if there is a customer on the platform
        if(lvl->currentCustomer.fromPlatform == i && lvl->customerWaiting == '1')
        {
            //if there is a customer, draws a blue line to represent a person on the platform
            glColor3d(0.0,0.0,1.0);
            glBegin(GL_LINES);
                glVertex3d(lvl->platforms[i].l.x1 + 3,lvl->platforms[i].l.y1 + 3,0.0f);
                glVertex3d(lvl->platforms[i].l.x1 + 3,lvl->platforms[i].l.y1 + 8,0.0f);
            glEnd();
            glColor3d(0.0,1.0,0.0);
        }
    }

    //draw player's lives
    char vv[7] = "Lives ";
    vv[6] = t->lives;
    for(i=6; i >= 0; i--)
    {
        printLetter(vv[i], 14 * i + 600, 80);
    }

    //draw player's current money total
    glColor3d(0.5,0.5,1.0);
    int a = t->money;
    for(i=9; i > 0; i--)
    {
        printLetter(a%10, 14 * i + 5, 50);
        a /= 10;
    }

    //draw timer
    int c = lvl->currentCustomer.fare;
    for(i=4; i > 0; i--)
    {
        printLetter(c%10, 14 * i + 620, 50);
        c /= 10;
    }

    //draw customer text
    char b[21] = "Hey taxi! Platform  !";
    char v[21] = "Hey taxi! Up!        ";
    if(lvl->levelCustomerCurrentNumber == lvl->levelCustomerNumber)
    {
        for(i=20; i >= 0; i--)
        {
            printLetter(v[i], 14 * i + 20, 80);
        }
    }
    else
    {
        b[19] = lvl->currentCustomer.destPlatform + 1;
        for(i=20; i >= 0; i--)
        {
            printLetter(b[i], 14 * i + 20, 80);
        }
    }

    //draw current velocity
    char bv[21] = "Velocity: ";
    int tempVel = abs(t->velocity) / 10;

    if(tempVel > 10)
    {
        glColor3d(1.0,0.0,0.0);
    }
    else if(tempVel > 5)
    {
        glColor3d(1.0,1.0,0.0);
    }
    else
    {
        glColor3d(0.0,1.0,0.0);
    }
    
    bv[12] = abs(tempVel % 10);
    tempVel /= 10;
    bv[11] = abs(tempVel % 10);
    tempVel /= 10;
    bv[10] = abs(tempVel % 10);
    for(i=0; i < 13; i++)
    {
        printLetter(bv[i], 14 * i + 300, 30);
    }


    //draw taxi
    drawTaxi(t, lvl);
    //print it to screen
    glFlush();
}


void makeLevel1(level *lvl)
{
    /*
        The first level is very simple, only one platform in the centre.
        8 boundary lines, 1 pad
    */
    /* first create the level boundaries */
    lseg t; //temp line segment
	t.x1 = 1.0;
	t.y1 = 101.0;
	t.x2 = 1.0;
	t.y2 = 599.0;
	lvl->bounds[0]=t;
	t.x1 = 1.0;
	t.y1 = 101.0;
	t.x2 = 799.0;
	t.y2 = 101.0;
    lvl->bounds[1]=t;
	t.x1 = 799.0;
	t.y1 = 101.0;
	t.x2 = 799.0;
	t.y2 = 599.0;
	lvl->bounds[2]=t;
	t.x1 = 1.0;
	t.y1 = 599.0;
	t.x2 = 300.0;
	t.y2 = 599.0;
	lvl->bounds[3]=t; 
    t.x1 = 500.0;
	t.y1 = 599.0;
	t.x2 = 799.0;
	t.y2 = 599.0;
	lvl->bounds[4]=t; 
	
    lvl->numbounds = 5;
    lvl->numplatforms = 0;

    createPlatform(lvl, 180, 620, 250, 250, 1);
    //createPlatform(lvl, 400, 550, 450, 450, 2);

    // check for rain
    lvl->rainChance = 5;
    if(rand()%100 > lvl->rainChance)
    {
        lvl->isRaining = '1';
    }
    else
    {
        lvl->isRaining = '0';
    }

    //set level cutomer numbers
    lvl->levelCustomerNumber = 1;
    lvl->levelCustomerCurrentNumber = 0;
}
void makeLevel2(level *lvl)
{
    /*
        The second level is a bit more complicated, with two landin pads
        11 boundary lines, 2 pads
    */
    
    /* first create the level boundaries */
    lseg t; //temp line segment
	t.x1 = 1.0;
	t.y1 = 101.0;
	t.x2 = 1.0;
	t.y2 = 599.0;
	lvl->bounds[0]=t;
	t.x1 = 1.0;
	t.y1 = 101.0;
	t.x2 = 799.0;
	t.y2 = 101.0;
	lvl->bounds[1]=t;
	t.x1 = 799.0;
	t.y1 = 101.0;
	t.x2 = 799.0;
	t.y2 = 599.0;
	lvl->bounds[2]=t;
	t.x1 = 1.0;
	t.y1 = 599.0;
	t.x2 = 300.0;
	t.y2 = 599.0;
	lvl->bounds[3]=t; 
    t.x1 = 500.0;
	t.y1 = 599.0;
	t.x2 = 799.0;
	t.y2 = 599.0;
	lvl->bounds[4]=t; 

    lvl->numbounds = 5;
    lvl->numplatforms = 0;

    createPlatform(lvl, 50, 250, 250, 250, 1);
    createPlatform(lvl, 400, 550, 250, 250, 2);

    // check for rain
    lvl->rainChance = 40;
    if(rand()%100 > lvl->rainChance)
    {
        lvl->isRaining = '1';
    }
    else
    {
        lvl->isRaining = '0';
    }

    //set level cutomer numbers
    lvl->levelCustomerNumber = 3;
    lvl->levelCustomerCurrentNumber = 1;
}
void makeLevel3(level *lvl)
{
    /*
        The second level is a bit more complicated, with two landin pads
        32 boundary lines, 9 pads
    */

    /* first create the level boundaries */
    lseg t; //temp line segment
	t.x1 = 1.0;
	t.y1 = 101.0;
	t.x2 = 1.0;
	t.y2 = 599.0;
	lvl->bounds[0]=t;
	t.x1 = 1.0;
	t.y1 = 101.0;
	t.x2 = 799.0;
	t.y2 = 101.0;
	lvl->bounds[1]=t;
	t.x1 = 799.0;
	t.y1 = 101.0;
	t.x2 = 799.0;
	t.y2 = 599.0;
	lvl->bounds[2]=t;
	t.x1 = 1.0;
	t.y1 = 599.0;
	t.x2 = 300.0;
	t.y2 = 599.0;
	lvl->bounds[3]=t; 
    t.x1 = 500.0;
	t.y1 = 599.0;
	t.x2 = 799.0;
	t.y2 = 599.0;
	lvl->bounds[4]=t; 

    lvl->numbounds = 5;
    lvl->numplatforms = 0;
    
    createPlatform(lvl, 20, 100, 250, 250, 1);
    createPlatform(lvl, 120, 200, 250, 250, 2);
    createPlatform(lvl, 220, 300, 250, 250, 3);
    createPlatform(lvl, 320, 400, 250, 250, 4);
    createPlatform(lvl, 420, 500, 250, 250, 5);
    createPlatform(lvl, 520, 600, 250, 250, 6);
    createPlatform(lvl, 620, 700, 250, 250, 7);
    createPlatform(lvl, 500, 600, 400, 400, 8);
    createPlatform(lvl, 180, 620, 500, 500, 9);

    // check for rain
    lvl->rainChance = 90;
    if(rand()%100 > lvl->rainChance)
    {
        lvl->isRaining = '1';
    }
    else
    {
        lvl->isRaining = '0';
    }

    //set level cutomer numbers
    lvl->levelCustomerNumber = 9;
    lvl->levelCustomerCurrentNumber = 1;
}


//main
int main(int argc, char **argv)
{
    int winposx = 100;
    int winposy = 100;
    int windowWidth = WINWIDTH;
    int windowHeight = WINHEIGHT;
    Uint32 timer;

    SDL_Window *window = SDL_CreateWindow("Taxi D'espace", winposx, winposy, windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	SDL_GLContext context = SDL_GL_CreateContext(window);

    /*//////////////////////////////////////////////////*/
    /* Set up the parts of the scene that will stay the same for every frame. */
    glFrontFace(GL_CCW);     /* Enforce counter clockwise face ordering (to determine front and back side) */
    glEnable(GL_NORMALIZE);
	glShadeModel(GL_FLAT); /* enable flat shading - Gouraud shading would be GL_SMOOTH */


    glEnable(GL_DEPTH_TEST);

    /* Set the clear (background) colour */
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    /* Set up the camera/viewing volume (projection matrix) and the timer */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, (GLdouble)(windowWidth-1),0.0,(GLdouble)(windowHeight-1));
    glViewport(0,0,windowWidth,windowHeight);
    /*//////////////////////////////////////////////////*/

    //map setup
    level lvl;
    makeLevel1(&lvl);
    customerMakeNext(&lvl, 999, 'f');

    //play objects setup
    taxi t;
    initialiseTaxi(&t, 400, 400, 3);

    //set game flags, first for while playing, second for pause function
    int go = 1;
    int go1 = 0;
    while(go)
    {
        SDL_Event incomingevent;

        while(SDL_PollEvent(&incomingevent))
        {
            switch(incomingevent.type)
            {
                // events
                case SDL_QUIT:
                go = 0;
                break;
                case SDL_KEYDOWN:
                // key events
                switch(incomingevent.key.keysym.sym)
                {
                    //pause function, press 1 to pause or unpause game
                    case SDLK_RETURN:
                    if(go1 == 1)
                    {
                        //game paused
                        go1 = 0;
                    }
                    else
                    {
                        //game unpaused
                        go1 = 1;
                    }
                    break;
                    //direction keys
                    //a;left
                    case SDLK_LEFT:
                    case SDLK_a:
                    if(t.wheelLandersEnabled == '0')
                    {
                        t.hThrust = -1;
                    }
                    break;
                    //d;right
                    case SDLK_RIGHT:
                    case SDLK_d:
                    if(t.wheelLandersEnabled == '0')
                    {
                        t.hThrust = 1;
                    }
                    break;
                    //w;up
                    case SDLK_UP:
                    case SDLK_w:
                    //up thruster enabled
                    t.vThrust = 1;
                    //is in air flag
                    t.inflight = '1';
                    //is no longer landed
                    t.landingOnceCustoCheck = '0';
                    break;
                    //d;down
                    case SDLK_DOWN:
                    case SDLK_s:
                    //checks if landers are enables, and reverses it
                    if(t.wheelLandersEnabled == '1')
                    {
                        t.wheelLandersEnabled = '0';
                        //puts the lander back in flight in case it was landed
                        t.inflight = '1';
                    }
                    else
                    {
                        t.wheelLandersEnabled = '1';
                    }
                    
                    break;
                }
                break;
                case SDL_KEYUP:
                switch(incomingevent.key.keysym.sym)
                {
                    //removes thrusts once key is lifted
                    case SDLK_LEFT:
                    case SDLK_a:
                    t.hThrust = 0;
                    break;
                    case SDLK_RIGHT:
                    case SDLK_d:
                    t.hThrust = 0;
                    break;
                    case SDLK_UP:
                    case SDLK_w:
                    t.vThrust = 0;
                    break;
                }
            }
        }
        if(go1 != 0 || t.nextLevel == '1')
        {
            if(t.money > 10000 && t.bonusGiven == 0)
            {
                t.lives++;
                t.bonusGiven = 1;
            }
            doEvents(&t,lvl.changedTicks * 0.0005,&lvl);
            if(t.justDied == '1')
            {
                if(t.lives > 1)
                {
                    //has died: reset taxi flags
                    //reset position
                    t.x = 400.0;
                    t.y = 400.0;
                    //decrement lives
                    t.lives--;
                    //now in flight and intact
                    t.inflight = '1';
                    t.bintact = '1';
                    //reset speed and thrusts
                    t.vSpeed = 0.0;
                    t.hSpeed = 0.0;
                    t.vThrust = 0.0;
                    t.hThrust = 0.0;
                    //customer no longer in taxi
                    t.custo_waiting = '0';
                    customerMakeNext(&lvl, 999, 't');
                    lvl.customerWaiting = '0';
                    //lvl.
                    //pause game, for gamefeel
                    waitForSecs(2, '1');
                    //resets just died flag, no longer dieing
                    t.justDied = '0';
                }
                else
                {
                    go = 0;
                }
            }
            //check if going to next level
            else if(t.nextLevel == '1')
            {
                //if true: increment level, then check which level to load
                t.levelCurrent++;
                if(t.levelCurrent == 1)
                {
                    //loading level 1 + new customer
                    makeLevel1(&lvl);
                    lvl.currentCustomer.destPlatform = (rand() % lvl.numplatforms);
                }
                else if(t.levelCurrent == 2)
                {
                    //loading level 2 + new customer
                    makeLevel2(&lvl);
                    lvl.currentCustomer.destPlatform = (rand() % lvl.numplatforms);
                }
                else if(t.levelCurrent == 3)
                {
                    //loading level 3 + new customer
                    makeLevel3(&lvl);
                    lvl.currentCustomer.destPlatform = (rand() % lvl.numplatforms);
                }
                else
                {
                    //end of the game you won what, add stuff in here like and end screen or sumethin' plez future me, k thx bi!
                    go = 0;
                    lvl.numbounds = 0;
                    lvl.numplatforms = 0;
                    lvl.isRaining = '0';
                }
                //resets for next level:
                //reset taxi position
                t.x = 400.0;
                t.y = 400.0;
                //reset thrusts & speed
                t.vSpeed = 0.0;
                t.hSpeed = 0.0;
                t.vThrust = 0.0;
                t.hThrust = 0.0;
                printf("\n%d\n", lvl.currentCustomer.destPlatform);
                //pause game, for gamefeel
                waitForSecs(2,'1');
                //resets next level flag, no longer going to next level
                t.nextLevel = '0';
            }
            //render the screen
            render(&t,&lvl);
        }
        SDL_GL_SwapWindow(window);
    }
    waitForSecs(4, '0');
    //exit cleanup
    SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}