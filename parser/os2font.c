
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "otypes.h"
#include "gpifont.h"


void show_glyph( ULONG ulOffset, POS2FONTRESOURCE pFont )
{
    GLYPHBITMAP glyph;
    USHORT      i, j, k;

    if ( ! ExtractOS2FontGlyph( ulOffset, pFont, &glyph )) {
        printf("Failed to extract glyph bitmap.\n");
        return;
    }

    // now draw the bitmap (inside a border indicating the cell size)
    for ( i = 0; i < glyph.horiBearingX; i++ ) printf(" ");
    printf("+");
    for ( i = 0; i < glyph.width; i++ ) printf("-");
    printf("+\n");
    for ( i = 0; i < glyph.rows; i++ ) {
        for ( j = 0; j < glyph.horiBearingX; j++ ) printf(" ");
        printf("|");
        for ( j = 0; j < glyph.pitch; j++ ) {
            for ( k = 0; k < 8; k++ ) {
                if ((( j * 8 ) + k ) >= glyph.width ) break;
                if (( glyph.buffer[ (i * glyph.pitch) + j ] << k ) & 0x80 )
                    printf("*");
                else
                    printf(( i+1 ) == pFont->pFontDef->pCellBaseOffset ? "." : " ");
            }
        }
        printf("|\n");
    }
    for ( i = 0; i < glyph.horiBearingX; i++ ) printf(" ");
    printf("+");
    for ( i = 0; i < glyph.width; i++ ) printf("-");
    printf("+\n");

    free( glyph.buffer );
}


int main( int argc, char *argv[] )
{
    PSZ             pszFile;
    OS2FONTRESOURCE font = {0};
    ULONG           index,
                    total,
                    offset,
                    cb,
                    error;
    PBYTE           pBuf;

    if ( argc < 2 ) {
        printf("Syntax: TEST <FNT filename> [Unicode index]\n");
        return 0;
    }
    pszFile = argv[1];
    if (( argc < 3 ) || ( !sscanf( argv[2], "%x", &index )))
        index = 0;


    error = ReadOS2FontResource( pszFile, 0, &total, &font );
    if ( error ) {
        switch ( error ) {
            case ERR_FILE_OPEN:
                printf("The file %s could not be opened.\n", pszFile );
                break;
            case ERR_FILE_STAT:
            case ERR_FILE_READ:
                printf("Failed to read file %s.\n", pszFile );
                break;
            case ERR_FILE_FORMAT:
                printf("The file %s does not contain a valid font.\n", pszFile );
                break;
            case ERR_NO_FONT:
                printf("The requested font number was not found in %s\n", pszFile );
                break;
            case ERR_MEMORY:
                printf("A memory allocation error occurred.\n");
                break;
            default:
                printf("An unknown error occurred.\n");
                break;
        }
        return error;
    }

    printf("File %s contains %u fonts.  Reading first font.\n", pszFile, total );
    printf(" - Font signature:    %s\n", font.pSignature->achSignature );
    printf(" - Family name:       %s\n - Face name:         %s\n",
           font.pMetrics->szFamilyname, font.pMetrics->szFacename );
    if ( font.pMetrics->usKerningPairs )
        printf(" - %d kerning pairs are defined\n", font.pMetrics->usKerningPairs );
    if ( font.pPanose )
        printf(" - PANOSE table:      (%u,%u,%u,%u,%u,%u,%u,%u,%u,%u)\n",
               font.pPanose->panose[0], font.pPanose->panose[1],
               font.pPanose->panose[2], font.pPanose->panose[3],
               font.pPanose->panose[4], font.pPanose->panose[5],
               font.pPanose->panose[6], font.pPanose->panose[7],
               font.pPanose->panose[8], font.pPanose->panose[8] );
    printf(" - Font type:         ");
    switch ( font.pFontDef->fsChardef ) {
        case OS2FONTDEF_CHAR3:
            printf("3 (ABC-space proportional-width)\n");
            break;
        default:
            if ( font.pFontDef->fsFontdef == OS2FONTDEF_FONT2 )
                printf("2 (single-increment proportional-width)\n");
            else
                printf("1 (fixed-width)\n");
            break;
    }

    printf(" - Point size:        %u (%ux%u dpi)\n",
           font.pMetrics->usNominalPointSize / 10,
           font.pMetrics->xDeviceRes, font.pMetrics->yDeviceRes );
    printf(" - First glyph index: %u\n", font.pMetrics->usFirstChar );
    printf(" - Last glyph index:  %u\n", font.pMetrics->usLastChar + font.pMetrics->usFirstChar );
    printf(" - Default character: %u\n", font.pMetrics->usDefaultChar + font.pMetrics->usFirstChar );
    printf(" - Break character:   %u\n", font.pMetrics->usBreakChar + font.pMetrics->usFirstChar );
    if ( index == 0 ) return 0;

    printf("\n");
    offset = OS2FontGlyphIndex( &font, index );
    if ( offset == 0 )
        printf("The character U+%04X is not supported by this font. Loading default glyph %u.\n",
                index, font.pMetrics->usFirstChar + font.pMetrics->usDefaultChar );
    else
        printf("The character U+%04X corresponds to font glyph index %u.\n", index, offset );
    show_glyph( offset, &font );
    return 0;
}
