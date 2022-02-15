#include <tice.h>
#include <stdint.h>
#include <graphx.h>
#include <math.h>
#include <keypadc.h>
#include <fileioc.h>

#include "gfx/vargfx.h"


#define SIMXS 110
#define SIMXE 210

#define SIMYS 70
#define SIMYE 170

static const int LOOKUP[9][2] = {
    {0, 0},  // If no neighbors
    {0, 0},  // If one neighbors
    {0, 1},  // If two neighbors... etc
    {1, 1},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0}
};

void convolve() {
    short i, j, s;

    uint8_t (*b2)[320] = (void*)(*(uintptr_t*)0xE30014);  // Hidden buffer
    /* Address hack to get the active buffer by MateoC */
    uint8_t (*b1)[320] = (void*)((*(uintptr_t*)0xE30014) ^ 0x12C00);  // Active buffer

    for (i=SIMYS+1; i<SIMYE-1; i++) {
        for (j=SIMXS+1; j<SIMXE-1; j++) {
            s = 0;
            /* Summing neighbors */
            s += (b1[i-1][j-1] + b1[i-1][j] + b1[i-1][j+1]);
            s += (b1[i][j-1]   + b1[i][j+1]);
            s += (b1[i+1][j-1] + b1[i+1][j] + b1[i+1][j+1]);

            b2[i][j] = LOOKUP[s][b1[i][j]];
        }
    }
}

void setup() {
    short i, j;

    gfx_SetColor(1);   
    gfx_SetTextFGColor(1);
    gfx_SetTextScale(1, 1);

    for (i=0; i<2; i++){
        gfx_FillScreen(0);
        gfx_Rectangle(SIMXS-2, SIMYS-2, (SIMXE-SIMXS)+4, (SIMYE-SIMYS)+4);
        gfx_PrintStringXY("John Conway's Game of Life", SIMXS-40, SIMYS-18);
        gfx_SwapDraw();
    }
    
    uint8_t (*b_buffer)[320] = (void*)(*(uintptr_t*)0xE30014);  // Hidden buffer
    
    for (i=SIMYS+1; i<SIMYE-1; i++) {
        for (j=SIMXS+1; j<SIMXE-1; j++) {
            b_buffer[i][j] = randInt(0, 1);  
        }
    }

    gfx_SwapDraw();
}

int main(void) {

    if (vargfx_init() == 0)
    {
        return 1;
    }

    int SEED = ti_A;
    srand(SEED);

    bool active = 1;

    gfx_Begin();
    gfx_SetPalette(global_palette, sizeof_global_palette, 0);
    gfx_SetDrawBuffer();

    setup();

    while (active) {

        /* Input polling */
        kb_Scan();
        if (kb_Data[6] == kb_Clear) { gfx_End(); return 0; }

        convolve();
        gfx_SwapDraw();
    }

    gfx_End();
    return 0;
}