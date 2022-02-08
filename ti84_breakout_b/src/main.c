#include <tice.h>
#include <graphx.h>
#include <keypadc.h>
#include <string.h>
#include <math.h>

/*
LCD Height: 240
LCD Width:  320
*/

/* Constants (probably an unecessary amount) */

#define HALF_LCD_W 160
#define HALF_LCD_H 120
#define SIZE 230
#define HALF_SIZE 115

#define UPPER_BOUND_X HALF_LCD_W-HALF_SIZE
#define LOWER_BOUND_X HALF_LCD_W+HALF_SIZE

#define UPPER_BOUND_Y HALF_LCD_H-HALF_SIZE
#define LOWER_BOUND_Y HALF_LCD_H+HALF_SIZE

#define BLOCKS_X 10
#define BLOCKS_Y 6
#define TOP_SPACE 20
#define BLOCK_SPACE 60

#define BLOCKSIZE_X SIZE/BLOCKS_X
#define BLOCKSIZE_Y BLOCK_SPACE/BLOCKS_Y

#define PADDLE_SPEED 5
#define PADDLE_WIDTH 5
#define PADDLE_LENGTH 50
#define PADDLE_HEIGHT 10

#define BALL_SIZE 4

/* https://ce-programming.github.io/toolchain/_images/graphx_palette.png */
/* red, orange, dark yellow, light yellow, green, blue */
const static uint8_t color_bands[BLOCKS_Y] = {169, 194, 163, 229, 69, 52};

/* Structs */

typedef struct vec2 {
    short int x;
    short int y;
} v2;


/* Functions */

int clamp(short x, short a, short b) ;
void vertical_print(const char* text, int x, int y, int scalex, int scaley) ;
int get_digit(int value, int digit) ;
void int_to_str(char* buffer, int value) ;
void update_int_text(char* buffer, int value, int x, int y) ;


/* Main */

int main(void) {

    /* Variable Setup */

    bool block_array[BLOCKS_Y][BLOCKS_X];

    short int score = 0;
    char scorebuf[10];
    int_to_str(scorebuf, score);
    short int lives = 5;
    char livesbuf[10];
    int_to_str(livesbuf, lives);

    short int paddle_pos = HALF_LCD_W;
    short int p_paddle_pos = paddle_pos;

    v2 ball_pos = {HALF_LCD_W, HALF_LCD_H};
    v2 p_ball_pos = ball_pos;

    v2 ball_vel = {-1, 1};

    bool active = 1;
    bool endstate[2] = { false, false };

    gfx_Begin();


    /* One time draw routine */

    gfx_FillScreen(42);

    /* Background box */
    gfx_SetColor(0);
    gfx_FillRectangle_NoClip(UPPER_BOUND_X-4, UPPER_BOUND_Y-4, SIZE+8, SIZE+8);
    gfx_SetColor(42);
    gfx_FillRectangle_NoClip(UPPER_BOUND_X-2, UPPER_BOUND_Y-2, SIZE+4, SIZE+4);
    gfx_SetColor(0);
    gfx_FillRectangle_NoClip(UPPER_BOUND_X, UPPER_BOUND_Y, SIZE, SIZE);

    /* Text Labels */
    gfx_SetTextTransparentColor(23);
    gfx_SetTextBGColor(23);
    gfx_SetTextFGColor(255);
    vertical_print("Lives", 12, 20, 2, 2);
    vertical_print("Score", LOWER_BOUND_X+18, 20, 2, 2);
    vertical_print("Score", LOWER_BOUND_X+18, 20, 2, 2);

    update_int_text(livesbuf, lives, 12, 130);
    update_int_text(scorebuf, score, LOWER_BOUND_X+18, 130);
    
    /* Blocks */
    short int i, j, x, y, s;
    for (i=0; i<BLOCKS_Y; i++) {
        gfx_SetColor(color_bands[i]);
        for (j=0; j<BLOCKS_X; j++) {
            x = UPPER_BOUND_X+j*BLOCKSIZE_X;
            y = TOP_SPACE + UPPER_BOUND_Y+i*BLOCKSIZE_Y;
            gfx_FillRectangle_NoClip(x, y, BLOCKSIZE_X, BLOCKSIZE_Y);

            block_array[i][j] = 1;
        }
    }
    
    while (active) {
        /* Score Check */
        if (lives <= 0) {
            active = 0;
            endstate[0] = true;
            break;
        }
        /* Checking if all Blocks Have Been Hit */
        s = 0;
        for (i=0; i<BLOCKS_Y; i++) {
            for (j=0; j<BLOCKS_X; j++) {
                s += block_array[i][j];
            }
        }
        if (s == 0) {
            active = 0;
            endstate[0] = true;
            endstate[1] = true;
            break;
        }

        
        kb_Scan();
        /* Arrows */
        switch (kb_Data[7]) {
            case (kb_Left):  
                paddle_pos -= PADDLE_SPEED;
                break;
            case (kb_Right): 
                paddle_pos += PADDLE_SPEED;
                break;
        }
        /* End Game */
        if (kb_Data[6] == kb_Clear) {
            active = 0;
            break;
        }

        /* Movement control routine*/

        paddle_pos = clamp(paddle_pos, UPPER_BOUND_X, LOWER_BOUND_X-PADDLE_LENGTH);
        
        if (ball_pos.x+ball_vel.x-BALL_SIZE <= UPPER_BOUND_X || ball_pos.x+ball_vel.x+BALL_SIZE >= LOWER_BOUND_X) { ball_vel.x = -ball_vel.x; }
        if (ball_pos.y+ball_vel.y-BALL_SIZE <= UPPER_BOUND_Y) { ball_vel.y = -ball_vel.y; }
        if (ball_pos.y+ball_vel.y+BALL_SIZE >= LOWER_BOUND_Y) {
            lives--;
            ball_pos.x = HALF_LCD_W;
            ball_pos.y = HALF_LCD_H;
            ball_vel.x = -1;
            ball_vel.y = -1;

            update_int_text(livesbuf, lives, 12, 130);
        }

        /* Ugly conditional */
        if ((ball_pos.y+ball_vel.y >= LOWER_BOUND_Y-PADDLE_HEIGHT) && 
            (ball_pos.x+ball_vel.x >= paddle_pos) && 
            (ball_pos.x+ball_vel.x <= paddle_pos+PADDLE_LENGTH)) 
        {
            ball_vel.y = -ball_vel.y;
            ball_vel.x += (ball_pos.x - (paddle_pos+ PADDLE_LENGTH/2)) / 6;
        }

        ball_vel.x = clamp(ball_vel.x, -3, 3);

        /* Extremely inefficient for mildly better collisions */
        for (i=0; i<BLOCKS_Y; i++) {
            for (j=0; j<BLOCKS_X; j++) {
                if (!block_array[i][j]) { continue; }

                x = UPPER_BOUND_X+j*BLOCKSIZE_X;
                y = TOP_SPACE + UPPER_BOUND_Y+i*BLOCKSIZE_Y;

                /* Could technically be done with clamped indexing of the block array, but that would take away the directional bouncing */
                if ((ball_pos.x+BALL_SIZE>=x && ball_pos.x-BALL_SIZE<=x+BLOCKSIZE_X) && 
                    (ball_pos.y+ball_vel.y+BALL_SIZE>=y && ball_pos.y+ball_vel.y-BALL_SIZE<=y+BLOCKSIZE_Y)) {
                    ball_vel.y = -ball_vel.y;
                    block_array[i][j] = false;
                    gfx_SetColor(0);
                    gfx_FillRectangle_NoClip(x, y, BLOCKSIZE_X, BLOCKSIZE_Y);

                    score += 10;
                    update_int_text(scorebuf, score, LOWER_BOUND_X+18, 130);
                }
                if ((ball_pos.y+BALL_SIZE>=y && ball_pos.y-BALL_SIZE<=y+BLOCKSIZE_Y) && 
                    (ball_pos.x+ball_vel.x+BALL_SIZE>=x && ball_pos.x+ball_vel.x-BALL_SIZE<=x+BLOCKSIZE_X)) {
                    ball_vel.x = -ball_vel.x;
                    block_array[i][j] = false;
                    gfx_SetColor(0);
                    gfx_FillRectangle_NoClip(x, y, BLOCKSIZE_X, BLOCKSIZE_Y);

                    score += 10;
                    update_int_text(scorebuf, score, LOWER_BOUND_X+18, 130);
                }
            }
        }

        ball_pos.x += ball_vel.x;
        ball_pos.y += ball_vel.y;


        /* Draw routine */

        gfx_SetColor(0);
        gfx_FillRectangle_NoClip(p_paddle_pos, LOWER_BOUND_Y-PADDLE_HEIGHT, PADDLE_LENGTH, PADDLE_WIDTH); /* Paddle Undraw */
        gfx_SetColor(color_bands[0]);
        gfx_FillRectangle_NoClip(paddle_pos, LOWER_BOUND_Y-PADDLE_HEIGHT, PADDLE_LENGTH, PADDLE_WIDTH); /* Paddle Draw */

        gfx_SetColor(0);
        gfx_FillRectangle_NoClip(p_ball_pos.x, p_ball_pos.y, BALL_SIZE, BALL_SIZE); /* Ball Undraw */
        gfx_SetColor(255);
        gfx_FillRectangle_NoClip(ball_pos.x, ball_pos.y, BALL_SIZE, BALL_SIZE); /* Ball Draw */
        

        p_ball_pos = ball_pos;
        p_paddle_pos = paddle_pos;

        msleep(7);
    }

    if (endstate[0] && endstate[1]) {
        /* Win */
        const char winstr[] = "You Win!";
        gfx_SetColor(color_bands[4]);
        gfx_FillRectangle((LCD_WIDTH/2)-80, (LCD_HEIGHT/2)-20, 160, 40);
        gfx_SetTextScale(2, 2);
        gfx_PrintStringXY(winstr, (LCD_WIDTH/2)-(gfx_GetStringWidth(winstr)/2), (LCD_HEIGHT/2)-8);

        msleep(50);

        while(!os_GetCSC()) ;

    } else if (endstate[0] && !endstate[1]) {
        /* Win */
        const char winstr[] = "You Lose!";
        gfx_SetColor(color_bands[0]);
        gfx_FillRectangle((LCD_WIDTH/2)-90, (LCD_HEIGHT/2)-20, 180, 40);
        gfx_SetTextScale(2, 2);
        gfx_PrintStringXY(winstr, (LCD_WIDTH/2)-(gfx_GetStringWidth(winstr)/2), (LCD_HEIGHT/2)-8);

        msleep(50);

        while(!os_GetCSC()) ;
    }

    gfx_End();
    return 0;
}

int clamp(short x, short a, short b) {
    /* https://stackoverflow.com/questions/427477 */
    const short t = x < a ? a : x;
    return t > b ? b : t;
}

void vertical_print(const char* text, int x, int y, int scalex, int scaley) {
    unsigned int i;
    gfx_SetTextScale(scalex, scaley);
    for (i=0; i<strlen(text); i++) {
        gfx_SetTextXY(x, y+(8*i*scaley));
        gfx_PrintChar(text[i]);
    }
}

int get_digit(int value, int digit) {
    int p1, p2;
    float shift;
    shift = pow(10, digit-1);
    p1 = floor(value/shift);
    p2 = floor(value/(shift*10)) * 10;
    return p1-p2;
}

void int_to_str(char* buffer, int value) {
    /* Getting digits in the value */
    short d, i;
    d = 0;
    int v = value;
    if (v != 0) {
        while (v > 0) {
        d++;
        v = value/pow(10, d);
        }
    } else {
        d = 1;
    }
    
    for (i=d; i>0; i--) {
        buffer[d-i] = get_digit(value, i)+'0';
    }
    buffer[d] = '\0';
}

void update_int_text(char* buffer, int value, int x, int y) {
    gfx_SetTextFGColor(42);
    vertical_print(buffer, x, y, 2, 2);

    int_to_str(buffer, value);
    gfx_SetTextFGColor(255);
    vertical_print(buffer, x, y, 2, 2);
}