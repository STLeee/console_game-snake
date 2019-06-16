#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <memory.h>

#define SCREEN_HIGHT 20
#define SCREEN_WIDTH 80

#define MAX_LENGTH 1000

#define X 0
#define Y 1

#define wallDiagram '#'
#define snakeHeadDiagram '@'
#define snakeDiagram 'o'
#define appleDiagram 'Q'

int SnakeState;
int SnakeLength;
int** SnakePosition;

int* applePosition;

int point;
int leval;
int gameOver;
int delayUTime;

//draw thread
void* drawThread (void* arg)
{
    int row, column, length;
    
    char diagram[SCREEN_HIGHT][SCREEN_WIDTH];
    
    while (!gameOver) {
        system("clear");
        
        //draw space
        for (row=0; row<SCREEN_HIGHT; row++) {
            for (column=0; column<SCREEN_WIDTH; column++) {
                if (!row || row == SCREEN_HIGHT-1 || !column || column == SCREEN_WIDTH-1) diagram[row][column] = wallDiagram;
                else diagram[row][column] = ' ';
            }
        }
        
        //draw snake
        diagram[SnakePosition[0][Y]][SnakePosition[0][X]] = snakeHeadDiagram;
        for (length=1; length<SnakeLength; length++) {
            if (SnakePosition[length][Y] && SnakePosition[length][X]) diagram[SnakePosition[length][Y]][SnakePosition[length][X]] = snakeDiagram;
        }
        
        //draw apple
        diagram[applePosition[Y]][applePosition[X]] = appleDiagram;
        
        //display
        for (row=0; row<SCREEN_HIGHT; row++) {
            for (column=0; column<SCREEN_WIDTH; column++) {
                printf("%c", diagram[row][column]);
            }
            printf("\n");
        }
        
        //display state
        printf("Point: %d\n", point);
        printf("Level: %d\n", leval);
        
        usleep(10000);
    }
    
    
    //game over text
    char** gameOverText = (char**)malloc(sizeof(char*) * 5);
    gameOverText[0] = "  ____    _    __  __ _____    _____     _______ ____  ";
    gameOverText[1] = " / ___|  / \\  |  \\/  | ____|  / _ \\ \\   / / ____|  _ \\ ";
    gameOverText[2] = "| |  _  / _ \\ | |\\/| |  _|   | | | \\ \\ / /|  _| | |_) |";
    gameOverText[3] = "| |_| |/ ___ \\| |  | | |___  | |_| |\\ V / | |___|  _ < ";
    gameOverText[4] = " \\____/_/   \\_\\_|  |_|_____|  \\___/  \\_/  |_____|_| \\_\\";
    
    for (row=0; row<5; row++) {
        for (column=0; column<55; column++) {
            diagram[SCREEN_HIGHT/2 - 3 + row][SCREEN_WIDTH/2 - 55/2 + column] = gameOverText[row][column];
        }
    }
    
    //display
    system("clear");
    for (row=0; row<SCREEN_HIGHT; row++) {
        for (column=0; column<SCREEN_WIDTH; column++) {
            printf("%c", diagram[row][column]);
        }
        printf("\n");
    }
    
    //display state
    printf("Point: %d\n", point);
    printf("Level: %d", leval);
    
    switch (gameOver) {
        case 1:
            printf("                HIT THE WALL!!!!\n");
            break;
            
        case 2:
            printf("                EAT YOUR SELF!!!!\n");
            break;
            
        default:
            break;
    }
    
    printf("press any key to continue...\n");
}

//keyboard thread
void* keyboardThread (void* arg)
{
    while (!gameOver) {
        char c = getchar();
        switch (c) {
            case 'h': //up
                if (SnakeState != 1) SnakeState = 0;
                break;
                
            case 'n': //down
                if (SnakeState != 0) SnakeState = 1;
                break;
                
            case 'b': //left
                if (SnakeState != 3) SnakeState = 2;
                break;
                
            case 'm': //right
                if (SnakeState != 2) SnakeState = 3;
                break;
                
            default:
                break;
        }
        usleep(delayUTime);
    }
}

int main (int argc, char* argv) {
    int i, length;
    
    //Initialize Snake
    SnakeState = 0;
    SnakeLength = 4;
    SnakePosition = (int**)malloc(sizeof(int*) * MAX_LENGTH);
    SnakePosition[0] = (int*)malloc(sizeof(int) * 2);
    SnakePosition[0][X] = SCREEN_WIDTH / 2;
    SnakePosition[0][Y] = SCREEN_HIGHT / 2;
    for (length=1; length<MAX_LENGTH; length++) {
        SnakePosition[length] = (int*)malloc(sizeof(int) * 2);
        if (length < SnakeLength) {
            SnakePosition[length][X] = SnakePosition[length-1][X];
            SnakePosition[length][Y] = SnakePosition[length-1][Y] + 1;
        } else {
            SnakePosition[length][X] = 0;
            SnakePosition[length][Y] = 0;
        }
    }
    
    
    //Initialize apple
    applePosition = (int*)malloc(sizeof(int)*2);
    time_t t;
    srand((unsigned) time(&t));
    while (1) {
        applePosition[X] = (rand() % (SCREEN_WIDTH-2)) + 1;
        applePosition[Y] = (rand() % (SCREEN_HIGHT-2)) + 1;
        for (length=0; length<SnakeLength; length++) if (applePosition[Y] == SnakePosition[length][Y] && applePosition[X] == SnakePosition[length][X]) break;
        if (length == SnakeLength) break;
    }
    
    
    //Initialize game state
    point = 0;
    leval = 1;
    gameOver = 0;
    delayUTime = 100000;
    
    
    //Initialize new terminal i/o settings
    static struct termios old, new;
    int echo = 0;
    tcgetattr(0, &old); /* grab old terminal i/o settings */
    new = old; /* make new settings same as old settings */
    new.c_lflag &= ~ICANON; /* disable buffered i/o */
    new.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
    tcsetattr(0, TCSANOW, &new); /* use these new terminal i/o settings now */
    
    
    //Create thread
    pthread_t draw, keyboard;
    pthread_create(&draw, NULL, &drawThread, &delayUTime);
    pthread_create(&keyboard, NULL, &keyboardThread, &delayUTime);
    
    
    //main thread
    while (1) {
        //Snake next step
        for (length = SnakeLength-1; length>0; length--) {
            SnakePosition[length][X] = SnakePosition[length-1][X];
            SnakePosition[length][Y] = SnakePosition[length-1][Y];
        }
        
        switch (SnakeState) {
            case 0: //up
                SnakePosition[0][Y]--;
                break;
                
            case 1: //down
                SnakePosition[0][Y]++;
                break;
                
            case 2: //left
                SnakePosition[0][X]--;
                break;
                
            case 3: //rignt
                SnakePosition[0][X]++;
                break;
                
            default:
                break;
        }
        
        //check wall
        if (!SnakePosition[0][Y] || SnakePosition[0][Y] == SCREEN_HIGHT-1 || !SnakePosition[0][X] || SnakePosition[0][X] == SCREEN_WIDTH-1) {
            //Hit the wall
            gameOver = 1;
            break;
        }
        
        //check self
        for (length=4; length<SnakeLength; length++) if (SnakePosition[0][Y] == SnakePosition[length][Y] && SnakePosition[0][X] == SnakePosition[length][X]) break;
        if (length != SnakeLength) {
            //eat self
            gameOver = 2;
            break;
        }
        
        //check apple
        if (SnakePosition[0][Y] == applePosition[Y] && SnakePosition[0][X] == applePosition[X]) {
            //eat apple
            
            //get point
            point++;
            
            //add length
            SnakeLength++;
            
            //rand apple position
            while (1) {
                applePosition[X] = (rand() % (SCREEN_WIDTH-2)) + 1;
                applePosition[Y] = (rand() % (SCREEN_HIGHT-2)) + 1;
                for (length=0; length<SnakeLength; length++) if (applePosition[Y] == SnakePosition[length][Y] && applePosition[X] == SnakePosition[length][X]) break;
                if (length == SnakeLength) break;
            }
        }
        
        //check level
        leval = point / 5 + 1;
        delayUTime = 100000 / (1.0 + (leval-1)/5.0);
        
        usleep(delayUTime);
    }
    
    pthread_cancel(keyboard);
    getchar();
    
    system("clear");
    
    //Restore old terminal i/o settings
    tcsetattr(0, TCSANOW, &old);
    
    return 0;
}