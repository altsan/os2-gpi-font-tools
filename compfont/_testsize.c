#define INCL_FONTFILEFORMAT
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>

#include "cmbfont.h"

int main( int argc, char *argv[] )
{
    printf("Size of FOCAMETRICS: %d (0x%X)\n", sizeof( FOCAMETRICS ), sizeof( FOCAMETRICS ));
    printf("Size of IFIMETRICS32: %d (0x%X)\n", sizeof( IFIMETRICS32 ), sizeof( IFIMETRICS32 ));
    printf("Size of UNIFONTMETRICS: %d (0x%X)\n", sizeof( UNIFONTMETRICS ), sizeof( UNIFONTMETRICS ));
    return 0;

}
