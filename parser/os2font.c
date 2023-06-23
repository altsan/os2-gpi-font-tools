/*****************************************************************************
 * os2font.c                                                                 *
 *                                                                           *
 * Program to parse an OS/2 GPI-format bitmap font and do various things     *
 * with it.                                                                  *
 *                                                                           *
 *  (C) 2012,2023 Alexander Taylor                                           *
 *                                                                           *
 *  This code is placed in the public domain.                                *
 *                                                                           *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "otypes.h"
#include "gpifont.h"

/* Local function prototypes */
void show_glyph( ULONG ulOffset, POS2FONTRESOURCE pFont );
BOOL write_font( OS2FONTRESOURCE font, ULONG count, USHORT dpi, PSZ pszFileName );


/* ------------------------------------------------------------------------ */
int main( int argc, char *argv[] )
{
    OS2FONTRESOURCE font = {0};
    CHAR            achOutFile[ 251 ] = {0};
    BOOL            bOutput = FALSE,    /* write font to output file? */
                    bIndex = FALSE;     /* is glyph ID an absolute glyph index (instead of Unicode)? */
    PSZ             pszFile,            /* input filename */
                    pszArg;             /* argument pointer */
    ULONG           number = 0,         /* glyph ID (if bOutput FALSE) or number of glyphs (if bOutput TRUE) */
                    resource = 0,       /* font number within the input file to read */
                    total,              /* count of fonts found in the input file */
                    index,              /* absolute glyph index to read */
                    error;              /* error code */
    USHORT          a,                  /* arg loop counter */
                    dpi = 0;            /* target DPI of output font */


    /* parse command-line arguments */
    if ( argc < 2 ) {
        printf("Syntax:\nOS2FONT <filename> [/F:<font>] [/O:<filename>] [/D:<dpi>] [/I] [<number>]\n");
        return 0;
    }
    pszFile = argv[1];
    for ( a = 2; a < argc; a++ ) {
        pszArg = argv[a];
        if ( *pszArg == '/' || *pszArg == '-') {
            pszArg++;
            if ( !(*pszArg) ) continue;
            if (( tolower( *pszArg ) == 'o') &&
                ( sscanf( pszArg+1, ":%250s", achOutFile ) == 1 ))
            {
                bOutput = TRUE;
            }
            else if ( tolower( *pszArg ) == 'i') {
                bIndex = TRUE;
            }
            else if ( tolower( *pszArg ) == 'f') {
                if ( !sscanf( pszArg+1, ":%u", &resource ))
                    resource = 0;
            }
            else if ( tolower( *pszArg ) == 'd') {
                if ( !sscanf( pszArg+1, ":%u", &dpi ))
                    dpi = 0;
            }

        }
        else if ( !sscanf( pszArg, "%x", &number ) || !sscanf( pszArg, "%u", &number ))
            number = 0;
    }

    /* try to parse a font from the file */
    error = ReadOS2FontResource( pszFile, resource, &total, &font );
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

    /* show font information */

    printf("File %s contains %u fonts.  Reading first font.\n", pszFile, total );
    printf(" - Font size:         %u bytes\n", font.cbSize );
    printf(" - Font signature:    %s\n", font.pSignature->achSignature );
    printf(" - Family name:       %s\n - Face name:         %s\n",
           font.pMetrics->szFamilyname, font.pMetrics->szFacename );
    printf(" - Point size:        %u (%ux%u dpi)\n",
           font.pMetrics->usNominalPointSize / 10,
           font.pMetrics->xDeviceRes, font.pMetrics->yDeviceRes );
    printf(" - First glyph index: %u\n", font.pMetrics->usFirstChar );
    printf(" - Last glyph index:  %u\n", font.pMetrics->usLastChar + font.pMetrics->usFirstChar );
    printf(" - Default character: %u\n", font.pMetrics->usDefaultChar + font.pMetrics->usFirstChar );
    printf(" - Break character:   %u\n", font.pMetrics->usBreakChar + font.pMetrics->usFirstChar );
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
    printf(" - Cell height:       %u\n", font.pFontDef->yCellHeight );
    printf(" - Glyph data length: %u bytes\n", font.pFontDef->ulSize );

    if ( font.pMetrics->usKerningPairs )
        printf(" - %d kerning pairs are defined\n", font.pMetrics->usKerningPairs );
    if ( font.pPanose )
        printf(" - PANOSE table:      (%u,%u,%u,%u,%u,%u,%u,%u,%u,%u)\n",
               font.pPanose->panose[0], font.pPanose->panose[1],
               font.pPanose->panose[2], font.pPanose->panose[3],
               font.pPanose->panose[4], font.pPanose->panose[5],
               font.pPanose->panose[6], font.pPanose->panose[7],
               font.pPanose->panose[8], font.pPanose->panose[8] );
    if ( !bOutput && !number ) goto done;

    printf("\n");
    if ( bOutput && achOutFile[0] ) {
        /* write the output file */
        write_font( font, number, dpi, achOutFile );
    }
    else {
        /* show the requested glyph */
        if ( bIndex ) {
            index = number;
        }
        else {
            index = OS2FontGlyphIndex( &font, number );
            if ( index == 0 )
                printf("The character U+%04X is not supported by this font. Loading default glyph %u.\n",
                        number, font.pMetrics->usFirstChar + font.pMetrics->usDefaultChar );
            else
                printf("The character U+%04X corresponds to font glyph index %u.\n", number, index );
        }
        show_glyph( index, &font );
    }
done:
    free( font.pSignature );
    return 0;
}


/* ------------------------------------------------------------------------ */
void show_glyph( ULONG ulOffset, POS2FONTRESOURCE pFont )
{
    GLYPHBITMAP glyph;
    USHORT      i, j, k;

    if ( ! ExtractOS2FontGlyph( ulOffset, pFont, &glyph )) {
        printf("Failed to extract glyph bitmap.\n");
        return;
    }

    /* now draw the bitmap (inside a border indicating the cell size) */
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


/* ------------------------------------------------------------------------ */
BOOL write_font( OS2FONTRESOURCE font, ULONG count, USHORT dpi, PSZ pszFileName )
{
    /* font records */
    OS2FONTSTART     recFontSignature = {0};
    OS2FOCAMETRICS   recFontMetrics   = {0};
    OS2FONTDEFHEADER recFontDefHeader = {0};
    OS2ADDMETRICS    recFontPanose    = {0};
    OS2FONTEND       recFontEnd       = {0};

    PBYTE  pGlyphData = NULL,
           pOffset    = NULL,
           pBitmap    = NULL;
    BYTE   bWidth,
           bHeight;
    USHORT i,
           usPts;
    FILE  *newFontFile;
    ULONG  cbBaseOffset,
           cbOffset,
           cbCharDefs,
           cbFont,
           cbBitmap;


    /* default to all glyphs (note that usLastChar is an offset from
     * usFirstChar, so we add 1 to get the total glyph count)
     */
    if ( !count || (count > font.pMetrics->usLastChar ))
        count = font.pMetrics->usLastChar + 1;

    printf("Saving %u glyphs to output file %s...\n", count, pszFileName );

    /* copy the required header records */
    memcpy( &recFontSignature, font.pSignature, sizeof( recFontSignature ));
    memcpy( &recFontMetrics, font.pMetrics, sizeof( recFontMetrics ));
    memcpy( &recFontDefHeader, font.pFontDef, sizeof( recFontDefHeader ));
    recFontSignature.ulSize = sizeof( recFontSignature );
    recFontMetrics.ulSize = sizeof( recFontMetrics );
    recFontDefHeader.ulSize = sizeof( recFontDefHeader );

    /* we don't support the kerning table feature */
    recFontMetrics.usKerningPairs = 0;

    /* copy the glyph data */
    cbFont = 0xFFFF;            /* allocate 64 KB (maximum size of a font resource) */
    pGlyphData = (PBYTE) malloc( cbFont );
    if ( !pGlyphData ) {
        fprintf( stderr, "Failed to allocate memory for glyph data.  Cannot continue.\n");
        return FALSE;
    }
    memset( pGlyphData, 0, cbFont );
    cbBaseOffset = sizeof( recFontSignature )
                 + sizeof( recFontMetrics )
                 + sizeof( recFontDefHeader );

    /* all glyphs have the same cell height */
    bHeight = recFontDefHeader.yCellHeight;

    if ( font.pFontDef->fsChardef == OS2FONTDEF_CHAR3 ) {       /* Type 3 glyph data */
        POS2CHARDEF3 pCharDefs,
                     pCharSrc, pCharDest;

        pCharDefs = (POS2CHARDEF3) pGlyphData;

        /* copy the requested number of character definition records
         * (adding space for the .null character if needed)
         */
        cbCharDefs = (count) * font.pFontDef->usCellSize;
        if ( font.pMetrics->usFirstChar )
            cbCharDefs += font.pFontDef->usCellSize;
        memcpy( pCharDefs, font.data.pABC, cbCharDefs );

        /* keep track of the current offset */
        pOffset = pGlyphData + cbCharDefs;
        cbOffset = cbBaseOffset + cbCharDefs;

        /* now copy each corresponding glyph bitmap */
        for ( i = 0; i < count; i++ ) {
            pCharSrc = (POS2CHARDEF3)( (PBYTE)font.data.pABC + (i * font.pFontDef->usCellSize) );
            if ( !pCharSrc->ulOffset ) {
                fprintf( stderr, "The character definition at index %u is not valid.\n", i );
                continue;
            }
            pBitmap = (PBYTE)font.pSignature + pCharSrc->ulOffset;

            /* get the size of the glyph bitmap */
            bWidth = ceil( pCharSrc->bSpace / 8.0 );
            cbBitmap = bWidth * bHeight;

            /* make sure it has the correct offset in the new file */
            pCharDest = (POS2CHARDEF3)( (PBYTE)pCharDefs + (i * recFontDefHeader.usCellSize) );
            pCharDest->ulOffset = cbOffset;
            if (( cbOffset + cbBitmap ) > ( cbFont - 28 )) {
                printf("Maximum font size reached.\n");
                break;
            }
#ifdef DEBUG
            printf("Copying ABC-type glyph %u (%ux%u), %u bytes at offset %u\n", i, bWidth, bHeight, cbBitmap, pCharDest->ulOffset );
#endif
            memcpy( pOffset, pBitmap, cbBitmap );
            pOffset += cbBitmap;
            cbOffset += cbBitmap;
        }

        /* add the .null character, unless it's already present as glyph 0 */
        if ( font.pMetrics->usFirstChar ) {
            pCharDest = (POS2CHARDEF3)( (PBYTE)pCharDefs + (i * recFontDefHeader.usCellSize) );
            cbBitmap = bHeight;                     /* width is always 1 byte */
            pCharDest->ulOffset = cbOffset;
            if (( cbOffset + cbBitmap ) > ( cbFont - 28 )) {
                printf("Maximum font size reached.\n");
            }
            else {
#ifdef DEBUG
                printf("Adding .null glyph of %u bytes at offset %u\n", cbBitmap, pCharDest->ulOffset );
#endif
                memset( pOffset, 0, cbBitmap );     /* blank glyph bitmap */
                pOffset += cbBitmap;
                cbOffset += cbBitmap;
            }
        }
    }
    else {                                  /* Type 1 or 2 glyph data */
        POS2CHARDEF1 pCharDefs,
                     pCharSrc, pCharDest;

        pCharDefs = (POS2CHARDEF1) pGlyphData;

        /* copy the requested number of character definition records
         * (adding space for the .null character if needed)
         */
        cbCharDefs = (count) * font.pFontDef->usCellSize;
        if ( font.pMetrics->usFirstChar )
            cbCharDefs += font.pFontDef->usCellSize;
        memcpy( pCharDefs, font.data.pChars, cbCharDefs );

        /* keep track of the current offset */
        pOffset = pGlyphData + cbCharDefs;
        cbOffset = cbBaseOffset + cbCharDefs;

        /* now copy each corresponding glyph bitmap */
        for ( i = 0; i < count; i++ ) {
            pCharSrc = (POS2CHARDEF1)( (PBYTE)font.data.pChars + (i * font.pFontDef->usCellSize) );
            if ( !pCharSrc->ulOffset ) {
                fprintf( stderr, "The character definition at index %u is not valid.\n", i );
                continue;
            }
            pBitmap = (PBYTE)font.pSignature + pCharSrc->ulOffset;

            /* get the size of the glyph bitmap */
            bWidth = ceil( pCharSrc->ulWidth / 8.0 );
            cbBitmap = bWidth * bHeight;

            /* make sure it has the correct offset in the new file */
            pCharDest = (POS2CHARDEF1)( (PBYTE)pCharDefs + (i * recFontDefHeader.usCellSize) );
            pCharDest->ulOffset = cbOffset;
            if (( cbOffset + cbBitmap ) > ( cbFont - 28 )) {
                printf("Maximum font size reached.\n");
                break;
            }
#ifdef DEBUG
            printf("Copying single-increment glyph %u (%ux%u), %u bytes from %u at offset %u\n", i, bWidth, bHeight, cbBitmap, pCharSrc->ulOffset, pCharDest->ulOffset );
#endif
            memcpy( pOffset, pBitmap, cbBitmap );
            pOffset += cbBitmap;
            cbOffset += cbBitmap;
        }

        /* add the .null character, unless it's already present as glyph 0 */
        if ( font.pMetrics->usFirstChar ) {
            pCharDest = (POS2CHARDEF1)( (PBYTE)pCharDefs + (i * recFontDefHeader.usCellSize) );
            cbBitmap = bHeight;                     /* width is always 1 byte */
            pCharDest->ulOffset = cbOffset;
            if (( cbOffset + cbBitmap ) > ( cbFont - 28 )) {
                printf("Maximum font size reached.\n");
            }
            else {
#ifdef DEBUG
                printf("Adding .null glyph of %u bytes at offset %u\n", cbBitmap, pCharDest->ulOffset );
#endif
                memset( pOffset, 0, cbBitmap );     /* blank glyph bitmap */
                pOffset += cbBitmap;
                cbOffset += cbBitmap;
            }
        }
    }

    cbFont = cbOffset;
    cbOffset -= cbBaseOffset;
    recFontDefHeader.ulSize = sizeof( recFontDefHeader ) + cbOffset;

    if ( font.pPanose && ( font.pPanose->Identity == SIG_OS2ADDMETRICS )) {
        memcpy( &recFontPanose, font.pPanose, sizeof( recFontPanose ));
        recFontPanose.ulSize = sizeof( recFontPanose );
        cbFont += recFontPanose.ulSize;
    }

    recFontEnd.Identity = SIG_OS2FONTEND;
    recFontEnd.ulSize = sizeof( recFontEnd );
    cbFont += recFontEnd.ulSize;

    /* update the character count metrics */
    recFontMetrics.usLastChar = count - 1;
    if ( recFontMetrics.usDefaultChar > recFontMetrics.usLastChar )
        recFontMetrics.usDefaultChar = recFontMetrics.usFirstChar;

    if (( dpi == 96 ) || ( dpi == 120 )) {
        usPts = 10 * round( recFontMetrics.yEmHeight / (dpi / 72.0) );
        recFontMetrics.xDeviceRes = dpi;
        recFontMetrics.yDeviceRes = dpi;
        recFontMetrics.usNominalPointSize = usPts;
        recFontMetrics.usMinimumPointSize = usPts;
        recFontMetrics.usMaximumPointSize = usPts;
    }

    /* now save the font*/

    printf("Saving new font %s (%u bytes)...\n", pszFileName, cbFont );
    newFontFile = fopen( pszFileName, "wb");

    /* font file signature */
    printf(" - Font signature:    %s\n", recFontSignature.achSignature );
    fwrite( &recFontSignature, recFontSignature.ulSize, 1, newFontFile );

    /* font metrics */
    printf(" - Family name:       %s\n - Face name:         %s\n",
           recFontMetrics.szFamilyname, recFontMetrics.szFacename );
    printf(" - Point size:        %u (%ux%u dpi)\n",
           recFontMetrics.usNominalPointSize / 10,
           recFontMetrics.xDeviceRes, recFontMetrics.yDeviceRes );
    printf(" - First glyph index: %u\n", recFontMetrics.usFirstChar );
    printf(" - Last glyph index:  %u\n", recFontMetrics.usLastChar + recFontMetrics.usFirstChar );
    printf(" - Default character: %u\n", recFontMetrics.usDefaultChar + recFontMetrics.usFirstChar );
    printf(" - Break character:   %u\n", recFontMetrics.usBreakChar + recFontMetrics.usFirstChar );
    fwrite( &recFontMetrics, recFontMetrics.ulSize, 1, newFontFile );

    /* character definition header */
    printf(" - Font type:         ");
    switch ( recFontDefHeader.fsChardef ) {
        case OS2FONTDEF_CHAR3:
            printf("3 (ABC-space proportional-width)\n");
            break;
        default:
            if ( recFontDefHeader.fsFontdef == OS2FONTDEF_FONT2 )
                printf("2 (single-increment proportional-width)\n");
            else
                printf("1 (fixed-width)\n");
            break;
    }
    printf(" - Cell height:       %u\n", recFontDefHeader.yCellHeight );
    printf(" - Definition length: %u\n", recFontDefHeader.ulSize );
    fwrite( &recFontDefHeader, sizeof( recFontDefHeader ), 1, newFontFile );

    /* character data */
    printf(" - Glyph data length: %u\n", cbOffset );
    fwrite( pGlyphData, cbOffset, 1, newFontFile );
    free( pGlyphData );

    /* panose table, if present */
    if ( recFontPanose.Identity == SIG_OS2ADDMETRICS ) {
        printf(" - PANOSE table:      (%u,%u,%u,%u,%u,%u,%u,%u,%u,%u)\n",
               recFontPanose.panose[0], recFontPanose.panose[1],
               recFontPanose.panose[2], recFontPanose.panose[3],
               recFontPanose.panose[4], recFontPanose.panose[5],
               recFontPanose.panose[6], recFontPanose.panose[7],
               recFontPanose.panose[8], recFontPanose.panose[8] );
        fwrite( &recFontPanose, recFontPanose.ulSize, 1, newFontFile );
    }

    /* font end signature */
    printf("\n");
    fwrite( &recFontEnd, recFontEnd.ulSize, 1, newFontFile );

    fclose( newFontFile );
    printf("File saved.\n");

    return TRUE;
}

