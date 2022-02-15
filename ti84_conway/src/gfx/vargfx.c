#include "vargfx.h"
#include <fileioc.h>

#define vargfx_HEADER_SIZE 0

unsigned char *vargfx_appvar[1] =
{
    (unsigned char*)0,
};

unsigned char vargfx_init(void)
{
    unsigned int data, i;
    ti_var_t appvar;

    appvar = ti_Open("vargfx", "r");
    if (appvar == 0)
    {
        return 0;
    }

    data = (unsigned int)ti_GetDataPtr(appvar) - (unsigned int)vargfx_appvar[0] + vargfx_HEADER_SIZE;
    for (i = 0; i < 1; i++)
    {
        vargfx_appvar[i] += data;
    }

    ti_Close(appvar);

    return 1;
}

