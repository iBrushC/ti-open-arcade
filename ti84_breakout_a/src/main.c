#include <tice.h>
#include <graphx.h>
#include <keypadc.h>
#include <string.h>
#include <math.h>

#define PADDING 5
#define BOUNDX LCD_WIDTH-PADDING  // X screen bound
#define BOUNDY LCD_HEIGHT-PADDING // Y screen bound

#define PSX 60                    // Player size x
#define HALF_PSX 30               // Half player x size
#define PSY 5                     // Player size y
#define PSS 3                     // Player set speed
#define PH  BOUNDY-(10+PADDING)   // Player set height

#define BLOCKROWS 3               // Number of rows of blocks
#define BPR 10                    // Blocks per row

#define BLOCKSPACE 4
#define BSX ((LCD_WIDTH-(2*PADDING)) / BPR)
#define BSY 5
#define TWO_BSY BSY*2


int clamp(short x, short a, short b) ;


// ! when possible, replace short with uint8_t5
int main() {
    gfx_Begin();
    gfx_SetDrawBuffer();

    /* Game Variables */
    short px = LCD_WIDTH/2;     // player position
    short bx = LCD_WIDTH/2;     // ball x position
    short by = LCD_HEIGHT/2;    // ball y position
    short bvx = -2;             // ball velocity x
    short bvy = -2;             // ball velocity y
    bool blockarr[BLOCKROWS][BPR];

    bool active = 1;
    bool win = 0;
    

    short i, r, s;
    for (r=0; r<BLOCKROWS; r++) {
        for (i=0; i<BPR; i++) {
            blockarr[r][i] = 1;
        }
    }

    while (active) {
        gfx_SwapDraw(); // ! will have to be swapped with partial redraw later for speed purposes
        gfx_FillScreen(255);

        /* Input detection */
        kb_Scan();
        /* Arrows */
        switch (kb_Data[7]) {
            case (kb_Left):
                px -= PSS;
                break;

            case (kb_Right):
                px += PSS;
                break;
        }
        /* End Game */
        switch (kb_Data[6]) {
            case (kb_Clear):
                active = 0;
                break;
        }

        /* Collision detection */
        if (bx+bvx+PADDING >= BOUNDX || bx+bvx <= PADDING) { bvx = -bvx; }
        if (by+bvy <= PADDING) { bvy = -bvy; }
        if (by+bvy+PADDING >= BOUNDY) {
            bx = LCD_WIDTH/2;
            by = LCD_HEIGHT/2;
            bvx = 0;
            bvy = 2;
        }

        if (by+bvy >= PH && bx > px && bx < px+PSX) { 
            bvy = -bvy;
            bvx += (bx - (px+HALF_PSX))/9; // ! replace with lookup table for speed
        }

        for (r=0; r<BLOCKROWS; r++) {
            for (i=0; i<BPR; i++) {
                if (!blockarr[r][i]) { continue; }
                short blockx = (PADDING+i*BSX)+BLOCKSPACE;
                short blocky = PADDING+r*BSY+r*BLOCKSPACE;
                if (bx > blockx && bx < blockx+BSX && by+bvy > blocky && by+bvy < blocky+BSY) {
                    blockarr[r][i] = 0;
                    bvy = -bvy;
                }
                if (bx+bvx > blockx && bx+bvx < blockx+BSX && by > blocky && by < blocky+BSY) {
                    blockarr[r][i] = 0;
                    bvx = -bvx;
                }
            }
        }

        /* Ball updates */
        bx += bvx;
        by += bvy;

        /* Drawing */
        px = clamp(px, PADDING, LCD_WIDTH-PSX-PADDING);
        gfx_Circle_NoClip(bx, by, 2);
        gfx_Rectangle_NoClip(px, PH, PSX, PSY);

        for (r=0; r<BLOCKROWS; r++) {
            for (i=0; i<BPR; i++) {
                if (!blockarr[r][i]) { continue; }
                short blockx = (PADDING+i*BSX)+BLOCKSPACE;
                short blocky = PADDING+r*BSY+r*BLOCKSPACE;
                gfx_Rectangle_NoClip(blockx, blocky, BSX-BLOCKSPACE, BSY);
            }
        }

        /* Checking if the game is over */
        s = 0;
        for (r=0; r<BLOCKROWS; r++) {
            for (i=0; i<BPR; i++) {
                s += blockarr[r][i];
            }
        }
        if (s == 0) {
            active = 0;
            win = 1;
        }
    }
    if (win) {
        const char* winstr = "You Win!";
        
        gfx_PrintStringXY(winstr,
                      (LCD_WIDTH - gfx_GetStringWidth(winstr)) / 2,
                      (LCD_HEIGHT - 8) / 2);
        gfx_SwapDraw();
        
        while(!os_GetCSC());
    }

    gfx_End();
    return 0;
}

int clamp(short x, short a, short b) {
    /* https://stackoverflow.com/questions/427477 */
    const short t = x < a ? a : x;
    return t > b ? b : t;
}