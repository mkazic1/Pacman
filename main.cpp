#include "mbed.h"
#include "stm32f413h_discovery_ts.h"
#include "stm32f413h_discovery_lcd.h"
#include "stdlib.h"

#define pacman_size 10
#define ghost_size 10

int pac_x = 17;
int pac_y = 187;
int bghost_x = 123;
int bghost_y = 66;

//positions of coins
int coins[82][3] = {
    {17,17,0},{40,17,0},{63,17,0},{86,17,0},{109,17,0},{123,17,0},{138,17,0},{161,17,0},{184,17,0},{200,17,0},{223,17,0},
    {17,40,0},{40,40,0},{63,40,0},{86,40,0},{109,40,0},{123,40,0},{138,40,0},{161,40,0},{184,40,0},{200,40,0},{223,40,0},
    {17,66,0},{40,66,0},{63,66,0},{86,66,0},{161,66,0},{184,66,0},{200,66,0},{223,66,0},
    {17,89,0},{40,89,0},{63,89,0},{86,89,0},{109,89,0},{123,89,0},{138,89,0},{161,89,0},{184,89,0},{200,89,0},{223,89,0},
    {17,115,0},{40,115,0},{63,115,0},{86,115,0},{109,115,0},{123,115,0},{138,115,0},{161,115,0},{184,115,0},{200,115,0},{223,115,0},
    {17,138,0},{40,138,0},{63,138,0},{86,138,0},{109,138,0},{138,138,0},{161,138,0},{184,138,0},{200,138,0},{223,138,0},
    {17,164,0},{40,164,0},{63,164,0},{86,164,0},{109,164,0},{123,164,0},{138,164,0},{161,164,0},{184,164,0},{200,164,0},{223,164,0},
    {40,187,0},{63,187,0},{86,187,0},{109,187,0},{138,187,0},{161,187,0},{184,187,0},{200,187,0},{223,187,0}
};

//values that represent how much place to shift ghost or pacman
int rightLeftShift[17] = {11,12,11,12,11,12,11,12,14,15,11,12,11,12,16,11,12};
int upDownShift[14] = {11,12,13,13,11,12,13,13,11,12,13,13,11,12};

//numbers of steps from lower left corner of maze
int shift_x_pacman = 0;
int shift_y_pacman = 0;
int shift_x_ghost = 9;
int shift_y_ghost = 10;

bool pac_go_right = true;
bool pac_go_left = false;
bool pac_go_up = false;
bool pac_go_down = false;

InterruptIn button_playgame(p5);
InterruptIn button_left(p6);
InterruptIn button_right(p7);
InterruptIn button_up(p8);
InterruptIn button_down(p9);

//ticker1 - using for showing strawberry on display
//ticker2 - using for removing strawbery from display after 15 seconds
//ticker3 - using if big coins is eaten to change color of ghost to blue after 15 seconds
//ticker4 - using to move ghost every 0.5 seconds
Ticker ticker1, ticker2, ticker3, ticker4;

int score = 0;
int numberoflives = 3;
int level = 1;
bool startedGame = true;
bool next_level = false;
bool win = false;
bool strawberry = false;// if strawberry is shown or no
bool bigCoinsEaten = false;



void showWindowHome(){
    BSP_LCD_Clear(LCD_COLOR_BLACK);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_DisplayStringAt(0,35,(uint8_t *)"P A   M A N",CENTER_MODE);
    //DRAWING PACMAN
    BSP_LCD_FillCircle(105,41,5);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_FillRect(105,39,6,4);

    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0,70,(uint8_t *)"!WELCOME!",CENTER_MODE);
    //DRAWING BLUE GHOST
    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    BSP_LCD_FillCircle(120,100,7);
    BSP_LCD_FillRect(113,100,15,7);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_FillCircle(117,100,1);
    BSP_LCD_FillCircle(123,100,1);

    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_DisplayStringAt(0,125,(uint8_t *)"Press first button",CENTER_MODE);
    BSP_LCD_DisplayStringAt(0,145,(uint8_t *)"and start playing",CENTER_MODE);
    //DRAWING BORDER
    BSP_LCD_FillCircle(8,8,3);
    BSP_LCD_FillCircle(232,8,3);
    BSP_LCD_FillCircle(232,232,3);
    BSP_LCD_FillCircle(8,232,3);
    BSP_LCD_DrawVLine(8,8,224);
    BSP_LCD_DrawVLine(232,8,224);
    BSP_LCD_DrawHLine(8,8,224);
    BSP_LCD_DrawHLine(8,232,224);
}
void showWindowLosedGame(){
    char printscore[30];
    snprintf(printscore,30,"Your score is: %d",score);

    BSP_LCD_Clear(LCD_COLOR_BLACK);

    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0,70,(uint8_t *)"!GAME OVER!",CENTER_MODE);
    BSP_LCD_DisplayStringAt(0,105,(uint8_t *)printscore,CENTER_MODE);
    BSP_LCD_DisplayStringAt(0,140,(uint8_t *)"PLAY AGAIN",CENTER_MODE);

    BSP_LCD_FillCircle(8,8,3);
    BSP_LCD_FillCircle(232,8,3);
    BSP_LCD_FillCircle(232,232,3);
    BSP_LCD_FillCircle(8,232,3);
    BSP_LCD_DrawVLine(8,8,224);
    BSP_LCD_DrawVLine(232,8,224);
    BSP_LCD_DrawHLine(8,8,224);
    BSP_LCD_DrawHLine(8,232,224);
}
void showWindowWinnedGame(){
    char printscore[30];
    snprintf(printscore,30,"Your score is: %d",score);

    BSP_LCD_Clear(LCD_COLOR_BLACK);

    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0,70,(uint8_t *)"!YOU ARE WINNER!",CENTER_MODE);
    BSP_LCD_DisplayStringAt(0,105,(uint8_t *)printscore,CENTER_MODE);
    BSP_LCD_DisplayStringAt(0,140,(uint8_t *)"PLAY AGAIN",CENTER_MODE);

    BSP_LCD_FillCircle(8,8,3);
    BSP_LCD_FillCircle(232,8,3);
    BSP_LCD_FillCircle(232,232,3);
    BSP_LCD_FillCircle(8,232,3);
    BSP_LCD_DrawVLine(8,8,224);
    BSP_LCD_DrawVLine(232,8,224);
    BSP_LCD_DrawHLine(8,8,224);
    BSP_LCD_DrawHLine(8,232,224);
}

void drawBorders(){
    BSP_LCD_SetTextColor(LCD_COLOR_LIGHTRED);

    BSP_LCD_DrawRect(4,4,232,196);

    BSP_LCD_DrawHLine(9,9,39);
    BSP_LCD_DrawHLine(55,9,176);
    BSP_LCD_DrawHLine(9,195,108);
    BSP_LCD_DrawHLine(130,195,101);
    BSP_LCD_DrawVLine(9,9,137);
    BSP_LCD_DrawVLine(9,156,39);
    BSP_LCD_DrawVLine(231,9,39);
    BSP_LCD_DrawVLine(231,58,88);
    BSP_LCD_DrawVLine(231,156,39);

    BSP_LCD_DrawRect(25,25,7,56);
    BSP_LCD_DrawVLine(48,9,98);
    BSP_LCD_DrawHLine(48,107,7);
    BSP_LCD_DrawVLine(55,9,98);
    BSP_LCD_DrawRect(71,25,7,56);
    BSP_LCD_DrawHLine(192,48,39);
    BSP_LCD_DrawVLine(192,48,10);
    BSP_LCD_DrawHLine(192,58,39);
    BSP_LCD_DrawHLine(169,146,62);
    BSP_LCD_DrawVLine(169,146,10);
    BSP_LCD_DrawHLine(169,156,62);
    BSP_LCD_DrawVLine(117,172,23);
    BSP_LCD_DrawHLine(117,172,13);
    BSP_LCD_DrawVLine(130,172,23);
    BSP_LCD_DrawHLine(9,146,69);
    BSP_LCD_DrawVLine(9,78,10);
    BSP_LCD_DrawVLine(78,146,10);
    BSP_LCD_DrawHLine(9,156,69);
    BSP_LCD_DrawRect(25,172,53,7);
    BSP_LCD_DrawRect(94,172,7,7);
    BSP_LCD_DrawRect(146,172,7,7);
    BSP_LCD_DrawRect(169,172,46,7);
    BSP_LCD_DrawRect(117,123,13,33);
    BSP_LCD_DrawVLine(94,97,59);
    BSP_LCD_DrawVLine(101,107,49);
    BSP_LCD_DrawVLine(146,107,49);
    BSP_LCD_DrawVLine(153,97,59);
    BSP_LCD_DrawHLine(94,97,59);
    BSP_LCD_DrawHLine(101,107,45);
    BSP_LCD_DrawHLine(94,156,7);
    BSP_LCD_DrawHLine(146,156,7);
    BSP_LCD_DrawVLine(25,97,33);
    BSP_LCD_DrawVLine(32,97,26);
    BSP_LCD_DrawVLine(71,97,26);
    BSP_LCD_DrawVLine(78,97,33);
    BSP_LCD_DrawHLine(25,97,7);
    BSP_LCD_DrawHLine(71,97,7);
    BSP_LCD_DrawHLine(32,123,39);
    BSP_LCD_DrawHLine(25,130,53);
    BSP_LCD_DrawRect(94,25,59,7);
    BSP_LCD_DrawRect(169,97,23,10);
    BSP_LCD_DrawVLine(94,48,33);
    BSP_LCD_DrawVLine(101,58,16);
    BSP_LCD_DrawVLine(117,48,10);
    BSP_LCD_DrawVLine(130,48,10);
    BSP_LCD_DrawVLine(153,48,33);
    BSP_LCD_DrawVLine(146,58,16);
    BSP_LCD_DrawHLine(94,48,23);
    BSP_LCD_DrawHLine(101,58,16);
    BSP_LCD_DrawHLine(130,48,23);
    BSP_LCD_DrawHLine(130,58,16);
    BSP_LCD_DrawHLine(101,74,45);
    BSP_LCD_DrawHLine(94,81,59);
    BSP_LCD_DrawHLine(169,25,46);
    BSP_LCD_DrawHLine(176,32,39);
    BSP_LCD_DrawHLine(176,74,39);
    BSP_LCD_DrawHLine(169,81,39);
    BSP_LCD_DrawHLine(169,123,39);
    BSP_LCD_DrawHLine(169,130,46);
    BSP_LCD_DrawVLine(169,25,56);
    BSP_LCD_DrawVLine(176,32,42);
    BSP_LCD_DrawVLine(215,25,7);
    BSP_LCD_DrawVLine(215,74,56);
    BSP_LCD_DrawVLine(208,81,42);
    BSP_LCD_DrawVLine(169,123,7);
}
void drawCoinses(){
    BSP_LCD_SetTextColor(LCD_COLOR_ORANGE);
    BSP_LCD_FillCircle(17,17,4);
    BSP_LCD_FillCircle(17,164,4);
    BSP_LCD_FillCircle(184,66,4);
    BSP_LCD_FillCircle(223,187,4);

    int i;
    for (i = 0; i < 82; i++){
        BSP_LCD_FillCircle(coins[i][0],coins[i][1],2);
    }

}
void drawPacman(){
    if(!startedGame) return;
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_FillCircle(pac_x,pac_y,pacman_size/2);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    if(pac_go_right)
        BSP_LCD_FillRect(pac_x,pac_y - 2,6,4);
    else if(pac_go_left)
        BSP_LCD_FillRect(pac_x - pacman_size/2,pac_y - 2,6,4);
    else if(pac_go_up)
        BSP_LCD_FillRect(pac_x - 2, pac_y - pacman_size/2,5,6);
    else
        BSP_LCD_FillRect(pac_x - 2, pac_y,5,6);
}

void drawWhiteBlueGhost(){
    if(!bigCoinsEaten) return;
    if(!startedGame) return;
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_FillCircle(bghost_x,bghost_y,ghost_size/2);
    BSP_LCD_FillRect(bghost_x - ghost_size/2,bghost_y,ghost_size + 1,ghost_size/2);

    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    BSP_LCD_FillCircle(bghost_x - 2,bghost_y,1);
    BSP_LCD_FillCircle(bghost_x + 2,bghost_y,1);
}
void drawBlueWhiteGhost(){

    if(!startedGame) return;
    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    BSP_LCD_FillCircle(bghost_x,bghost_y,ghost_size/2);
    BSP_LCD_FillRect(bghost_x - ghost_size/2,bghost_y,ghost_size + 1,ghost_size/2);

    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_FillCircle(bghost_x - 2,bghost_y,1);
    BSP_LCD_FillCircle(bghost_x + 2,bghost_y,1);
}
void drawBlackGhost(){
    if(!startedGame) return;
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_FillCircle(bghost_x,bghost_y,ghost_size/2);
    BSP_LCD_FillRect(bghost_x - ghost_size/2,bghost_y,ghost_size + 1,ghost_size/2);

    BSP_LCD_FillCircle(bghost_x - 2,bghost_y,1);
    BSP_LCD_FillCircle(bghost_x + 2,bghost_y,1);
}
void drawStrawberryBLACK(){
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);

    BSP_LCD_FillCircle(17,187,5);
    BSP_LCD_FillRect(16,180,3,3);
}
void drawStrawberryRED(){
    if(!startedGame) return;
    strawberry = true;
    ticker2.attach(&drawStrawberryBLACK,15);

    BSP_LCD_SetTextColor(LCD_COLOR_RED);

    BSP_LCD_FillCircle(17,187,5);

    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_FillRect(12,187,11,5);

    BSP_LCD_SetTextColor(LCD_COLOR_RED);

    Point tacke[3];
    tacke[0].X = 12;
    tacke[0].Y = 187;
    tacke[1].X = 22;
    tacke[1].Y = 187;
    tacke[2].X = 17;
    tacke[2].Y = 192;

    BSP_LCD_FillPolygon(tacke, 3);

    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

    BSP_LCD_FillCircle(14,186,1);
    BSP_LCD_FillCircle(20,186,1);
    BSP_LCD_FillCircle(17,189,1);

    BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
    BSP_LCD_FillRect(16,180,3,3);
}

void moveGhost();

void playGame(){
    if(startedGame == false || next_level == true){
        startedGame = true;
        bigCoinsEaten = false;
        ticker1.attach(&drawStrawberryRED,30);
        ticker4.attach(&moveGhost,0.5);
        pac_go_right = true;
        pac_go_left = false;
        pac_go_up = false;
        pac_go_down = false;

        if(win){
            win = false;
        }

        if(!next_level){
            score = 0;
            numberoflives = 3;
            level = 1;
        }

        pac_x = 17;
        pac_y = 187;
        shift_x_pacman = 0;
        shift_y_pacman = 0;
        bghost_x = 123;
        bghost_y = 66;
        shift_x_ghost = 9;
        shift_y_ghost = 10;
        next_level = false;

        BSP_LCD_Clear(LCD_COLOR_BLACK);
        drawBorders();
        drawCoinses();
        drawPacman();
        drawBlueWhiteGhost();
    }else{
        startedGame = false;
        score = 0;
        numberoflives = 3;
        level = 1;
        showWindowHome();
        ticker1.detach();
        ticker2.detach();
        ticker4.detach();
    }
}

void replaceCoins(){
    int i;
    for(i = 0; i < 82; i++){
        if(coins[i][0] == pac_x && coins[i][1] == pac_y && coins[i][2] != 1 && next_level == false){
            if(level == 1){
                score += 10;
            }else if(level == 2){
                score += 20;
            }else {
                score += 40;
            }
            coins[i][2] = 1;
            break;
        }else if(next_level == true)
            coins[i][2] = 0;
    }

}

void moveGhostOverCOINS() {
    BSP_LCD_SetTextColor(LCD_COLOR_ORANGE);
    int pos_x1 = 17, pos_x2 = 17, pos_x3 = 184, pos_x4 = 223;
    int pos_y1 = 17, pos_y2 = 164, pos_y3 = 66, pos_y4 = 187;
    for(int i=0; i<82; i++)
    {
        if(bghost_x == coins[i][0] && bghost_y == coins[i][1] && coins[i][2] == 0)
        {
            if(coins[i][0] == pos_x1 && coins[i][1] == pos_y1)
                BSP_LCD_FillCircle(coins[i][0],coins[i][1],4);
            else if(coins[i][0] == pos_x2 && coins[i][1] == pos_y2)
                BSP_LCD_FillCircle(coins[i][0],coins[i][1],4);
            else if(coins[i][0] == pos_x3 && coins[i][1] == pos_y3)
                BSP_LCD_FillCircle(coins[i][0],coins[i][1],4);
            else if(coins[i][0] == pos_x4 && coins[i][1] == pos_y4)
                BSP_LCD_FillCircle(coins[i][0],coins[i][1],4);
            else
                BSP_LCD_FillCircle(coins[i][0],coins[i][1],2);
        }
    }
}
void moveGhostOverStrawberry(){
    if(bghost_x == 17 && bghost_y == 187 && strawberry == true)
        drawStrawberryRED();
}

void eatStrawberry(){
    if(strawberry == true && pac_x == 17 && pac_y == 187){
        strawberry = false;
        if(level == 1) score += 100;
        else if(level == 2) score += 200;
        else score += 400;
        drawStrawberryBLACK();
        ticker2.detach();
        drawPacman();
    }
}

void setBigCoinsEatenFalse(){
    bigCoinsEaten = false;
    drawBlueWhiteGhost();
    ticker3.detach();
}

void attachTicker3(){
    bigCoinsEaten = true;
    drawWhiteBlueGhost();
    ticker3.attach(&setBigCoinsEatenFalse,15);
}

//checking if the current level is finished or player won this game
//if every coins is eated and level is 3 then player won this game
//if every coins is eated and level is not 3 then level up
void check(){
    int i;
    bool foundnoteatedcoins = false;
    for(i = 0; i < 82; i++){
        if(coins[i][2] == 0)
            foundnoteatedcoins = true;
    }

    if(!foundnoteatedcoins && level == 3){
        next_level = false;
        win = true;
        startedGame = false;
        showWindowWinnedGame();
    }
    else if(!foundnoteatedcoins && level != 3) {
        next_level = true;
        level += 1;
        replaceCoins();
        playGame();
    }
}

void checkPacmanPositionsWithGhostPositions(){
    if(pac_x == bghost_x && pac_y == bghost_y){
        drawBlackGhost();
        if(bigCoinsEaten == true)
            {
                bghost_x = 123;
                bghost_y = 66;
                shift_x_ghost = 9;
                shift_y_ghost = 10;

                if(level == 1) score += 200;
                else if(level == 2) score += 400;
                else score += 800;

                bigCoinsEaten = false;
                drawBlueWhiteGhost();
                ticker3.detach();
            }
        else{
            numberoflives -= 1;

            if(numberoflives == 0){
                startedGame = false;
                showWindowLosedGame();
            }else{

                pac_x = 17;
                pac_y = 187;
                shift_x_pacman = 0;
                shift_y_pacman = 0;
                bghost_x = 123;
                bghost_y = 66;
                shift_x_ghost = 9;
                shift_y_ghost = 10;

                drawBlueWhiteGhost();

                pac_go_right = true;
                pac_go_left = false;
                pac_go_up = false;
                pac_go_down = false;
            }
        }
    }
}
void checkIfBigCoinsEaten(){
    int i;
    int pos_x1 = 17, pos_x2 = 17, pos_x3 = 184, pos_x4 = 223;
    int pos_y1 = 17, pos_y2 = 164, pos_y3 = 66, pos_y4 = 187;

    for(i = 0; i < 82; i++){
        if(coins[i][2] == 0){
            if(coins[i][0] == pos_x1 && coins[i][1] == pos_y1 && pac_x == pos_x1 && pac_y == pos_y1) attachTicker3();
            else if(coins[i][0] == pos_x2 && coins[i][1] == pos_y2 && pac_x == pos_x2 && pac_y == pos_y2) attachTicker3();
            else if(coins[i][0] == pos_x3 && coins[i][1] == pos_y3 && pac_x == pos_x3 && pac_y == pos_y3) attachTicker3();
            else if(coins[i][0] == pos_x4 && coins[i][1] == pos_y4 && pac_x == pos_x4 && pac_y == pos_y4) attachTicker3();
        }
    }
}

bool checkIfCanMoveRightPacman(){
    if(BSP_LCD_ReadPixel(4,4) != BSP_LCD_ReadPixel(pac_x + 8, pac_y))
        return true;
    return false;
}
bool checkIfCanMoveLeftPacman(){
    if(BSP_LCD_ReadPixel(4,4) != BSP_LCD_ReadPixel(pac_x - 8, pac_y))
        return true;
    return false;
}
bool checkIfCanMoveUpPacman(){
     if(BSP_LCD_ReadPixel(4,4) != BSP_LCD_ReadPixel(pac_x, pac_y - 8))
        return true;
    return false;
}
bool checkIfCanMoveDownPacman(){
    if(BSP_LCD_ReadPixel(4,4) != BSP_LCD_ReadPixel(pac_x, pac_y + 8) && !(pac_x == 123 && pac_y == 40))
        return true;
    return false;
}

void pacmanMoveRight(){
    if(checkIfCanMoveRightPacman()){
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_FillCircle(pac_x,pac_y,5);

        shift_x_pacman += 1;
        pac_x += rightLeftShift[shift_x_pacman-1];

        pac_go_right = true;
        pac_go_left = false;
        pac_go_up = false;
        pac_go_down = false;

        eatStrawberry();
        checkIfBigCoinsEaten();
        replaceCoins();
        checkPacmanPositionsWithGhostPositions();
        check();
        drawPacman();
    }
}
void pacmanMoveLeft(){
    if(checkIfCanMoveLeftPacman()){
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_FillCircle(pac_x,pac_y,5);

        shift_x_pacman -= 1;
        pac_x -= rightLeftShift[shift_x_pacman];
        pac_go_left = true;
        pac_go_right = false;
        pac_go_up = false;
        pac_go_down = false;
        eatStrawberry();
        checkIfBigCoinsEaten();
        replaceCoins();
        checkPacmanPositionsWithGhostPositions();
        check();
        drawPacman();
    }
}
void pacmanMoveUp(){
    if(checkIfCanMoveUpPacman()){
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_FillCircle(pac_x,pac_y,5);

        shift_y_pacman += 1;
        pac_y -= upDownShift[shift_y_pacman - 1];
        pac_go_left = false;
        pac_go_right = false;
        pac_go_up = true;
        pac_go_down = false;
        eatStrawberry();
        checkIfBigCoinsEaten();
        replaceCoins();
        checkPacmanPositionsWithGhostPositions();
        check();
        drawPacman();
    }
}
void pacmanMoveDown(){
    if(checkIfCanMoveDownPacman()){
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_FillCircle(pac_x,pac_y,5);

        shift_y_pacman -= 1;
        pac_y += upDownShift[shift_y_pacman];
        pac_go_left = false;
        pac_go_right = false;
        pac_go_up = false;
        pac_go_down = true;
        eatStrawberry();
        checkIfBigCoinsEaten();
        replaceCoins();
        checkPacmanPositionsWithGhostPositions();
        check();
        drawPacman();
    }
}

bool checkIfCanMoveRightGhost(){
    if(BSP_LCD_ReadPixel(4,4) != BSP_LCD_ReadPixel(bghost_x + 8, bghost_y))
        return true;
    return false;
}
bool checkIfCanMoveLeftGhost(){
    if(BSP_LCD_ReadPixel(4,4) != BSP_LCD_ReadPixel(bghost_x - 8, bghost_y))
        return true;
    return false;
}
bool checkIfCanMoveUpGhost(){
    if(BSP_LCD_ReadPixel(4,4) != BSP_LCD_ReadPixel(bghost_x, bghost_y - 8))
        return true;
    return false;
}
bool checkIfCanMoveDownGhost(){
    if(BSP_LCD_ReadPixel(4,4) != BSP_LCD_ReadPixel(bghost_x, bghost_y + 8) && !(bghost_x == 123 && bghost_y == 40))
        return true;
    return false;
}

void ghostMoveRight(){
    if(checkIfCanMoveRightGhost()){
        drawBlackGhost();
        moveGhostOverCOINS();
        moveGhostOverStrawberry();
        shift_x_ghost += 1;
        bghost_x += rightLeftShift[shift_x_ghost-1];
        checkPacmanPositionsWithGhostPositions();
        drawPacman();
        if(bigCoinsEaten)
		    drawWhiteBlueGhost();
	    else
        	drawBlueWhiteGhost();
    }
}
void ghostMoveLeft(){
    if(checkIfCanMoveLeftGhost()){
        drawBlackGhost();
        moveGhostOverCOINS();
        moveGhostOverStrawberry();
        shift_x_ghost -= 1;
        bghost_x -= rightLeftShift[shift_x_ghost];
        checkPacmanPositionsWithGhostPositions();
        drawPacman();
        if(bigCoinsEaten)
		    drawWhiteBlueGhost();
	    else
        	drawBlueWhiteGhost();
    }
}
void ghostMoveUp(){
    if(checkIfCanMoveUpGhost()){
        drawBlackGhost();
        moveGhostOverCOINS();
        moveGhostOverStrawberry();
        shift_y_ghost += 1;
        bghost_y -= upDownShift[shift_y_ghost - 1];

        checkPacmanPositionsWithGhostPositions();

        drawPacman();
        if(bigCoinsEaten)
		    drawWhiteBlueGhost();
	    else
        	drawBlueWhiteGhost();
    }
}
void ghostMoveDown(){
    if(checkIfCanMoveDownGhost()){
        drawBlackGhost();
        moveGhostOverCOINS();
        moveGhostOverStrawberry();
        shift_y_ghost -= 1;
        bghost_y += upDownShift[shift_y_ghost];

        checkPacmanPositionsWithGhostPositions();

        drawPacman();
        if(bigCoinsEaten)
		    drawWhiteBlueGhost();
	    else
        	drawBlueWhiteGhost();
    }
}

void pickMoveOfGhost(){
    int dec = rand()%4;
    if(dec == 0)
    {
        ghostMoveRight();
    }
    else if(dec == 1)
    {
        ghostMoveLeft();
    }
    else if(dec == 2)
    {
        ghostMoveUp();
    }
    else
    {
        ghostMoveDown();
    }
}

//if ghost and pacman are on the same or x or y positions then move ghost to appropriate direction
//or pick another direction using rand() function in pickMove function
void moveGhost() {
    if(startedGame == false) return;

    if(bghost_x == 123 && bghost_y == 66) {
        ghostMoveUp();
        ghostMoveUp();
    }
    else if(pac_x==bghost_x && bghost_y<pac_y && !win)
    {
        if(checkIfCanMoveDownGhost())
            ghostMoveDown();
        else
            pickMoveOfGhost();
    }
    else if(pac_x==bghost_x && bghost_y>pac_y && !win)
    {
        if(checkIfCanMoveUpGhost())
            ghostMoveUp();
        else
            pickMoveOfGhost();
    }
    else if(pac_y==bghost_y && bghost_x<pac_x && !win)
    {
        if(checkIfCanMoveRightGhost())
            ghostMoveRight();
        else
            pickMoveOfGhost();
    }
    else if(pac_y==bghost_y && bghost_x>pac_x && !win)
    {
        if(checkIfCanMoveLeftGhost())
            ghostMoveLeft();
        else
            pickMoveOfGhost();
    }
    else{
        pickMoveOfGhost();
    }
}

void printScoreAndNumberOfLives(){
    if(!startedGame) return;
    char printscore[20];
    char printlives[8];
    char printlevel[8];
    snprintf(printscore,20,"SCORE:%d",score);
    snprintf(printlives,8,"LIVES:%d",numberoflives);
    snprintf(printlevel,8,"LEVEL:%d",level);

    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0,208,(uint8_t *)printscore,LEFT_MODE);
    BSP_LCD_DisplayStringAt(0,224,(uint8_t *)printlives,LEFT_MODE);
    BSP_LCD_DisplayStringAt(0,224,(uint8_t *)printlevel,RIGHT_MODE);
}

int main() {
    startedGame = false;
    showWindowHome();

    button_playgame.rise(&playGame);
    button_right.rise(&pacmanMoveRight);
    button_left.rise(&pacmanMoveLeft);
    button_up.rise(&pacmanMoveUp);
    button_down.rise(&pacmanMoveDown);
    while (1) {
        eatStrawberry();

        printScoreAndNumberOfLives();

        wait_ms(100);
    }
}
