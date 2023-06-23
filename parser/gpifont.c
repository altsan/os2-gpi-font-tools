/*****************************************************************************
 *                                                                           *
 *  gpifont.c                                                                *
 *                                                                           *
 *  Implementation of support for standard OS/2 PM/GPI bitmap fonts.  The    *
 *  aim is to be as platform-independent as possible, so OS/2-specific APIs  *
 *  are avoided.                                                             *
 *                                                                           *
 *  (C) 2012 Alexander Taylor                                                *
 *                                                                           *
 *  This code is placed in the public domain.                                *
 *                                                                           *
 *****************************************************************************/

/* Uncomment to write the extracted font and fontdir to files:
 * #define DEBUG_DUMP_RESOURCE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "otypes.h"
#include "gpifont.h"
#include "pmugl.h"
#include "os2res.h"


/* File I/O routines.
 */
#define _FILE_OPEN( fspec )         fopen( fspec, "rb")
#define _FILE_STAT( f, pstat )      fstat( fileno(f), pstat )
#define _FILE_SEEK( f, ofs )        fseek( f, ofs, SEEK_SET )
#define _FILE_READ( f, pbuf, cb )   fread( (void *) pbuf, 1, cb, f )
#define _FILE_CLOSE( f )            fclose( f )


/* Useful byte-manipulation macros.
 */
#define HIBYTE( x )                     (( x & 0xFF00 ) >> 8 )
#define LOBYTE( x )                     ( x & 0xFF )
#define WORDFROMBYTES( b1, b2 )         ( b1 | (b2 << 8) )
#define LONGFROMBYTES( b1, b2, b3, b4 ) ( b1 | (b2 << 8) | (b3 << 16) | (b4 << 24) )


/* Internal function prototypes.
 */
void   CopyByteSeq( PUCHAR target, PUCHAR source, ULONG count );
BOOL   LXExtractResource( FILE *pf, LXHEADER lx_hd, LXRTENTRY lx_rte, ULONG ulBase, PBYTE *ppBuffer, PULONG pulSize );
USHORT LXUnpack1( PBYTE pBuf, USHORT cbPage );
USHORT LXUnpack2( PBYTE pBuf, USHORT cbPage );



/* ------------------------------------------------------------------------- *
 * CopyByteSeq                                                               *
 *                                                                           *
 * Perform a byte-over-byte iterative copy from one byte array into another, *
 * or from one point to another within the same array.  Used by LXUnpack2(). *
 * Note that memcpy() does not work for this purpose, because the source and *
 * target address spaces could overlap - that is, the end of the source      *
 * sequence could extend into the start of the target sequence, thus copying *
 * bytes that were previously written by the same call to this function.     *
 *                                                                           *
 * ------------------------------------------------------------------------- */
void CopyByteSeq( PUCHAR target, PUCHAR source, ULONG count )
{
    ULONG i;
    for ( i = 0; i < count; i++ ) target[ i ] = source[ i ];
}


/* ------------------------------------------------------------------------- *
 * ExtractOS2FontGlyph                                                       *
 *                                                                           *
 * Extracts the bitmap data for the OS/2 font glyph at the given index, and  *
 * converts it into standard bitmap format.  The bitmap will be written into *
 * the buffer field of the provided glyph information structure; the buffer  *
 * will be allocated by this function and should be freed by the caller when *
 * no longer needed.                                                         *
 *                                                                           *
 * The bitmap has to be converted manually by this function because the      *
 * standard OS/2 font format stores bitmap data with consecutive bytes       *
 * representing vertical columns, rather than rows as with standard bitmaps. *
 *                                                                           *
 * The written bitmap buffer contains the image bits only, no header         *
 * information is included.  The image format is 1 bit per pixel.            *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   ULONG            ulIndex: Glyph index (codepoint) within the font.  (I) *
 *   POS2FONTRESOURCE pFont  : Pointer to the font resource data.        (I) *
 *   PGLYPHBITMAP     pGlyph : Pointer to the glyph information.         (O) *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 * ------------------------------------------------------------------------- */
BOOL ExtractOS2FontGlyph( ULONG ulIndex, POS2FONTRESOURCE pFont, PGLYPHBITMAP pGlyph )
{
    USHORT       cx, cy,        // size of the character bitmap in pels
                 bearingL,      // left side-bearing (a_space) in pels
                 bearingR,      // right side-bearing (c_space) in pels
                 usWidth,       // number of bytes in each row
                 i, j, k;       // loop indices
    PBYTE        pBitmap,       // pointer to bitmap within the font data
                 pBuffer;       // new buffer to receive the glyph bitmap
    ULONG        ulPos;         // offset into pBuffer
    POS2CHARDEF1 pChar1;        // pointer to type 1/2 glyph definition
    POS2CHARDEF3 pChar3;        // pointer to type 3 glyph definition


    // Map index 0 to the default/substitution glyph
    if ( ulIndex == 0 )
        ulIndex = pFont->pMetrics->usFirstChar + pFont->pMetrics->usDefaultChar;

    // Make sure our index actually falls within the range contained in the font
    if (( ulIndex < pFont->pMetrics->usFirstChar ) ||
        ( ulIndex > ( pFont->pMetrics->usFirstChar +
                       pFont->pMetrics->usLastChar )))
        return FALSE;

    ulIndex -= pFont->pMetrics->usFirstChar;

    // Find the character data for the given offset
    if ( pFont->pFontDef->fsChardef == OS2FONTDEF_CHAR3 ) {
        pChar3 = (POS2CHARDEF3) ( (PBYTE) pFont->data.pABC +
                                  ( ulIndex * pFont->pFontDef->usCellSize ));
        if ( pChar3->ulOffset == 0 ) return FALSE;
        pBitmap = (PBYTE) pFont->pSignature + pChar3->ulOffset;
        cx = pChar3->bSpace;
        bearingL = pChar3->aSpace;
        bearingR = pChar3->cSpace;
    }
    else {
        pChar1 = (POS2CHARDEF1) ( (PBYTE) pFont->data.pABC +
                                  ( ulIndex * pFont->pFontDef->usCellSize ));
        pBitmap = (PBYTE) pFont->pSignature + pChar1->ulOffset;
        if ( pChar1->ulOffset == 0 ) return FALSE;
        cx = pChar1->ulWidth;
        bearingL = 0;
        bearingR = 0;
    }
    cy = pFont->pFontDef->yCellHeight;
    usWidth = cx / 8;
    if ( cx % 8 ) usWidth++;

    pBuffer = (PBYTE) calloc( cy, usWidth );
    if ( !pBuffer ) return FALSE;

    // Now convert the bitmap
    ulPos = 0;
    for ( i = 0; i < cy; i++ ) {
        for ( j = 0; j < usWidth; j++ ) {
            pBuffer[ ulPos++ ] = pBitmap[ i + (cy * j) ];
        }
    }
    pGlyph->rows         = cy;
    pGlyph->width        = cx;
    pGlyph->pitch        = usWidth;
    pGlyph->buffer       = pBuffer;
    pGlyph->cbBuffer     = cy * usWidth;
    pGlyph->horiBearingX = bearingL;
    pGlyph->horiAdvance  = bearingL + cx + bearingR;
    pGlyph->vertBearingY = pFont->pMetrics->yExternalLeading;
    pGlyph->vertAdvance  = cy + pFont->pMetrics->yExternalLeading;
    return TRUE;
}


/* ------------------------------------------------------------------------- *
 * LXExtractResource                                                         *
 *                                                                           *
 * Extracts a binary resource from an LX-format (32-bit OS/2) module.  The   *
 * function takes a pointer to a buffer which will receive the extracted     *
 * data of the object containing the resource.  It is up to the caller to    *
 * locate the actual resource data within this buffer (this should be        *
 * lx_rte.offset bytes from the start of the buffer).  The buffer is         *
 * allocated by this function on successful return, and must be freed once   *
 * no longer needed.                                                         *
 *                                                                           *
 * This routine is based on information made available by Martin Lafaix,     *
 * Veit Kannegieser and Max Alekseyev.                                       *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   FILE      *pf      : Pointer to the open file.                      (I) *
 *   LXHEADER   lx_hd   : LX-format executable header                    (I) *
 *   LXRTENTRY  lx_rte  : Resource-table entry of the requested resource (I) *
 *   ULONG      ulBase  : File offset of the LX-format header            (I) *
 *   PBYTE     *ppBuffer: Pointer to a buffer for the resource data      (O) *
 *   PULONG     pulSize : Pointer to the returned resource size          (O) *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   FALSE: Failed to extract object data; ppBuffer & pulSize are unchanged. *
 *   TRUE: Data extracted successfully, ppBuffer points to allocated buffer. *
 * ------------------------------------------------------------------------- */
BOOL LXExtractResource( FILE *pf, LXHEADER lx_hd, LXRTENTRY lx_rte, ULONG ulBase, PBYTE *ppBuffer, PULONG pulSize )
{
    LXOTENTRY   lx_obj;      // object table entry
    PLXOPMENTRY plxpages;    // array of individual object page information
    USHORT      cb_obj,      // size of an object table entry
                cb_pme;      // size of an object page map entry
    ULONG       cbData,      // total size of the object data
                cbPageAddr,  // address of an individual object page
                i;
    PBYTE       pBuf,
                pBufOff;
    BOOL        fOK = FALSE;

//printf("Extracting resource %u (%u bytes from %u) from object %u\n", lx_rte.name, lx_rte.cb, lx_rte.offset, lx_rte.obj );

    cb_obj = sizeof( LXOTENTRY );
    cb_pme = sizeof( LXOPMENTRY );

    // Locate & read the object table entry for this resource
    if (( _FILE_SEEK( pf, ulBase + lx_hd.obj_tbl +
                      ( cb_obj * (lx_rte.obj-1) ))) ||
        ( ! _FILE_READ( pf, &lx_obj, cb_obj ))        )
        return FALSE;

    // Locate & read the object page table entries for this object
    plxpages = (PLXOPMENTRY) calloc( lx_obj.mapsize, cb_pme );
    if ( !plxpages ) return FALSE;
    cbData = 0;
    // - go to the first indicated entry in the pagemap
    if ( _FILE_SEEK( pf, ulBase + lx_hd.objmap +
                         ( cb_pme * ( lx_obj.pagemap-1 )))) goto finish;
    // - read the indicated number of pages from this point
    for ( i = 0; i < lx_obj.mapsize; i++ ) {
        if ( ! _FILE_READ( pf, (PVOID)(plxpages + i), cb_pme ))
            goto finish;
        cbData += (( plxpages[ i ].flags == OP32_ITERDATA ) ||
                   ( plxpages[ i ].flags == OP32_ITERDATA2 )) ?
                  4096 : plxpages[ i ].size;
    }

    if ( cbData < ( lx_rte.offset + lx_rte.cb - 1 )) goto finish;

    // Now read each page from its indicated location into our buffer
    pBuf = (PBYTE) calloc( cbData, 1 );
    if ( !pBuf ) goto finish;
    pBufOff = pBuf;
    cbData  = 0;
    for ( i = 0; i < lx_obj.mapsize; i++ ) {
        cbPageAddr = lx_hd.datapage +
                     ( plxpages[ i ].dataoffset << lx_hd.pageshift );
        if (( _FILE_SEEK( pf, cbPageAddr )) ||
            ( ! _FILE_READ( pf, pBufOff, plxpages[ i ].size )))
            break;
//printf(" - page %u [flags 0x%x] size is %u\n", i, plxpages[ i ].flags, plxpages[ i ].size );
        if ( plxpages[ i ].flags == OP32_ITERDATA )
            cbData += LXUnpack1( pBufOff, plxpages[ i ].size );
        else if ( plxpages[ i ].flags == OP32_ITERDATA2 )
            cbData += LXUnpack2( pBufOff, plxpages[ i ].size );
        else if ( plxpages[ i ].flags == OP32_VALID )
            cbData += plxpages[ i ].size;
        else continue;
//printf(" - resource buffer size: %u\n", cbData );
        pBufOff = (PBYTE)(pBuf + cbData);
        if (( i + 1 ) == lx_obj.mapsize ) fOK = TRUE;
    }
    if ( fOK ) {
        *ppBuffer = pBuf;
        *pulSize  = cbData;
    }
    else free( pBuf );

finish:
    free( plxpages );
    return ( fOK );
}


/* ------------------------------------------------------------------------- *
 * LXUnpack1                                                                 *
 *                                                                           *
 * Unpacks a (max 4096-byte) page which has been compressed using the OS/2   *
 * /EXEPACK1 method (a simple form of run-length encoding).  The unpacked    *
 * data (max 4096 bytes) is written back into the input buffer.  The input   *
 * buffer must therefore provide at least 4096 bytes.                        *
 *                                                                           *
 * This algorithm was derived from public-domain Pascal code by Veit         *
 * Kannegieser (based on previous work by Max Alekseyev).                    *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PBYTE  pBuf  : Buffer containing the page data.                    (IO) *
 *   USHORT cbPage: The size of the input page data.                     (I) *
 *                                                                           *
 * RETURNS: USHORT                                                           *
 *   The number of output bytes written back to the buffer.                  *
 * ------------------------------------------------------------------------- */
USHORT LXUnpack1( PBYTE pBuf, USHORT cbPage )
{
    BYTE   abOut[ 4096 ] = {0};     // temporary output buffer
    USHORT ofIn,                    // current input buffer offset
           ofOut,                   // current output buffer offset
           usReps,                  // number of repetitions of current sequence
           usLen;                   // length of current sequence

    if ( cbPage > 4096 ) return cbPage;
    ofIn  = 0;
    ofOut = 0;
    do {
        usReps = (USHORT)( pBuf[ofIn] | (pBuf[ofIn+1] << 8 ));
        if ( !usReps ) break;
        ofIn += 2;
        usLen = (USHORT)( pBuf[ofIn] | (pBuf[ofIn+1] << 8 ));
        ofIn += 2;
        if (( ofOut + ( usReps * usLen )) > 4096 ) break;
        while ( usReps ) {
            memcpy( abOut + ofOut, pBuf + ofIn, usLen );
            usReps--;
            ofOut += usLen;
        }
        ofIn += usLen;
    } while ( ofIn <= cbPage );

    memcpy( pBuf, abOut, ofOut );
    return ofOut;
}


/* ------------------------------------------------------------------------- *
 * LXUnpack2                                                                 *
 *                                                                           *
 * Unpacks a (max 4096-byte) page which has been compressed using the OS/2   *
 * /EXEPACK2 method (which is apparently a modified Lempel-Ziv algorithm).   *
 * The unpacked data (max 4096 bytes) is written back into the input buffer. *
 * The input buffer must therefore provide at least 4096 bytes.              *
 *                                                                           *
 * This algorithm was derived from public-domain Pascal-and-x86-assembly     *
 * code by Veit Kannegieser (based on previous work by Max Alekseyev).       *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PBYTE  pBuf  : Buffer containing the page data.                    (IO) *
 *   USHORT cbPage: The size of the input page data.                     (I) *
 *                                                                           *
 * RETURNS: USHORT                                                           *
 *   The number of output bytes written back to the buffer.                  *
 * ------------------------------------------------------------------------- */
USHORT LXUnpack2( PBYTE pBuf, USHORT cbPage )
{
    BYTE   abOut[ 4096 ] = {0};     // temporary output buffer
    ULONG  ofIn,                    // current input buffer offset
           ofOut,                   // current output buffer offset
           ulControl,               // control word(s)
           ulLen;                   // length of current sequence


    if ( cbPage > 4096 ) return cbPage;
    ofIn  = 0;
    ofOut = 0;
    do {
        ulControl = WORDFROMBYTES( *(pBuf+ofIn), *(pBuf+ofIn+1) );

        /* Bits 1 & 0 hold the case flag (0-3); the interpretation of the
         * remaining bits depend on the flag value.
         */
        switch ( ulControl & 0x3 ) {
            case 0:
                /* bits 15..8  = length2
                 * bits  7..2  = length1
                 */
                if ( LOBYTE( ulControl ) == 0 ) {
                    /* When length1 == 0, fill (length2) bytes with the byte
                     * value following ulControl; if length2 is 0 we're done.
                     */
                    ulLen = HIBYTE( ulControl );
                    if ( !ulLen ) goto done;
                    memset( abOut + ofOut, *(pBuf + ofIn + 2), ulLen );
                    ofIn  += 3;
                    ofOut += ulLen;
                }
                else {
                    // block copy (length1) bytes from after ulControl
                    ulLen = ( LOBYTE( ulControl ) >> 2 );
                    memcpy( abOut + ofOut, pBuf + ofIn + 1, ulLen );
                    ofIn  += (ulLen + 1);
                    ofOut += ulLen;
                }
                break;

            case 1:
                /* bits 15..7     = backwards reference
                 * bits  6..4  +3 = length2
                 * bits  3..2     = length1
                 */
                // copy length1 bytes following ulControl
                ulLen = ( ulControl >> 2 ) & 0x3;
                memcpy( abOut + ofOut, pBuf + ofIn + 2, ulLen );
                ofIn += ulLen + 2;
                ofOut += ulLen;
                // get length2 from what's been unpacked already
                ulLen = (( ulControl >> 4 ) & 0x7 ) + 3;
                CopyByteSeq( abOut + ofOut, abOut + ( ofOut - ((ulControl >> 7 ) & 0x1FF )), ulLen );
                ofOut += ulLen;
                break;

            case 2:
                /* bits 15.. 4     = backwards reference
                 * bits  3.. 2  +3 = length
                 */
                ulLen = (( ulControl >> 2 ) & 0x3 ) + 3;
                CopyByteSeq( abOut + ofOut, abOut + ( ofOut - ((ulControl >> 4 ) & 0xFFF )), ulLen );
                ofIn  += 2;
                ofOut += ulLen;
                break;

            case 3:
                ulControl = LONGFROMBYTES( *(pBuf+ofIn), *(pBuf+ofIn+1), *(pBuf+ofIn+2), *(pBuf+ofIn+3) );
                /* bits 23..21  = ?
                 * bits 20..12  = backwards reference
                 * bits 11.. 6  = length2
                 * bits  5.. 2  = length1
                 */
                // block copy (length1) bytes
                ulLen = ( ulControl >> 2 ) & 0xF;
                memcpy( abOut + ofOut, pBuf + ofIn + 3, ulLen );
                ofIn  += ulLen + 3;
                ofOut += ulLen;
                // copy (length2) bytes from previously-unpacked data
                ulLen = ( ulControl >> 6 ) & 0x3F;
                CopyByteSeq( abOut + ofOut, abOut + ( ofOut - ((ulControl >> 12) & 0xFFF )), ulLen );
                ofOut += ulLen;
                break;

        }

    } while ( ofIn < cbPage );

done:
    /* It seems that the unpacked data will always be 4096 bytes, except for
     * the final page (which will be taken care of when the caller uses the
     * total object length to read the concatenated buffer).
     */
    memcpy( pBuf, abOut, 4096 );
    return 4096;
}


/* ------------------------------------------------------------------------- *
 * OS2FontGlyphIndex                                                         *
 *                                                                           *
 * Given a Unicode (UCS) codepoint, convert it into a glyph index/codepoint  *
 * within an OS/2 GPI bitmap font.  This currently supports only standard    *
 * UGL encoding; everything else will be treated as a symbol font (that is   *
 * to say, with passthrough encoding).  Fonts from non-English versions of   *
 * OS/2 prior to 4.5x therefore may not work properly, but such fonts are    *
 * likely vanishingly rare nowadays anyway.  (If the utilising program cares *
 * about this, it should check the pFont->pMetrics->usCodepage field and     *
 * reject any font which does not report 850 or 65400.)  Since OS/2 4.5x     *
 * (circa 1998) bitmap fonts from all languages use the common UGL encoding. *
 *                                                                           *
 * Note that while there is a direct correspondence between the glyph index  *
 * and the physical glyph offset, the two may not be exactly the same if the *
 * first actual character in the font is not character index 0.  The glyph   *
 * offset will be equal to the character index minus the index of the first  *
 * actual character in the font; the conversion is done in the               *
 * ExtractOS2FontGlyph() function.  (The caller need not be aware of the     *
 * distinction, but it is mentioned here just to be thorough.)               *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   POS2FONTRESOURCE pFont: Pointer to the font data structure.         (I) *
 *   ULONG            index: UCS-2 codepoint requested.                  (I) *
 *                                                                           *
 * RETURNS: ULONG                                                            *
 *   The corresponding index within the font's own encoding (either UGL or   *
 *   passthrough/verbatim), or 0 if the requested character does not exist.  *
 *   (Technically index 0 corresponds to the .null character, but there      *
 *   should be no need to maintain a distinction as .null is by definition   *
 *   not a displayable glyph in any case.)                                   *
 * ------------------------------------------------------------------------- */
ULONG OS2FontGlyphIndex( POS2FONTRESOURCE pFont, ULONG index )
{
    USHORT i;

    // If this is not a UGL font (reported as 0 or 850), assume no translation
    if ( pFont->pMetrics->usCodePage && ( pFont->pMetrics->usCodePage != 850 ))
        return index;

    // Basic ASCII needs no translation and is always supported
    if ( index >= 32 && index <= 126 ) return index;


    /* Certain character blocks have direct mappings...
     */

    // Parts of Cyrillic
    if ( index >= 0x401 && index <= 0x40C )
        i = index - 0x281;
    else if (( index >= 0x40E && index <= 0x45C && index != 0x450 ))
        i = index - 0x282;

    // Parts of Hebrew
    else if ( index >= 0x5D0 && index <= 0x5EA )
        i = index - 0x3B8;
    else if ( index >= 0x5B0 && index <= 0x5B9 )
        i = index - 0x37D;
    else if ( index >= 0x5BB && index <= 0x5C3 )
        i = index - 0x37E;

    // Halfwidth katakana
    else if ( index >= 0xFF61 && index <= 0xFF9F )
        i = index - 0xFC5F;

    // Hangul
    else if ( index >= 0x3131 && index <= 0x3163 )
        i = index - 0x2DB1;

    // Thai
    else if ( index >= 0xE01 && index <= 0xE3A )
        i = index - 0xA4A;
    else if ( index >= 0xE40 && index <= 0xE4F )
        i = index - 0xA4F;
    else if ( index >= 0xE50 && index <= 0xE59 )
        i = index - 0xA4D;


    /* Otherwise do a brute-force search for the Unicode to UGL mapping
     * (this is not terribly efficient...)
     */
    else
        for ( i = 0; ( i < OS2UGL_MAX_GLYPH ) && ( UGL2Uni[i] != index ); i++ );


    /* We have our UGL index, now see if the font actually supports it
     */

    // Character not supported by UGL encoding at all, return 0.
    if ( i >= OS2UGL_MAX_GLYPH ) return 0;

    // Character falls outside the font's own range
    if (( i < pFont->pMetrics->usFirstChar ) ||
        ( i > ( pFont->pMetrics->usFirstChar + pFont->pMetrics->usLastChar )))
        return 0;

    // Character belongs to an unsupported character group
    if ( CHAR_IS_LATIN1( i )   && !( pFont->pMetrics->fsDefn & FOCA_CHARSET_LATIN1 ))
        return 0;
    if ( CHAR_IS_PCEXTRA( i )  && !( pFont->pMetrics->fsDefn & FOCA_CHARSET_PC ))
        return 0;
    if ( CHAR_IS_LATINEXT( i ) && !( pFont->pMetrics->fsDefn & FOCA_CHARSET_LATINX ))
        return 0;
    if ( CHAR_IS_CYRILLIC( i ) && !( pFont->pMetrics->fsDefn & FOCA_CHARSET_CYRILLIC ))
        return 0;
    if ( CHAR_IS_HEBREW( i )   && !( pFont->pMetrics->fsDefn & FOCA_CHARSET_HEBREW ))
        return 0;
    if ( CHAR_IS_GREEK( i )    && !( pFont->pMetrics->fsDefn & FOCA_CHARSET_GREEK ))
        return 0;
    if ( CHAR_IS_ARABIC( i )   && !( pFont->pMetrics->fsDefn & FOCA_CHARSET_ARABIC ))
        return 0;
    if ( CHAR_IS_UGLEXT( i )   && !( pFont->pMetrics->fsDefn & FOCA_CHARSET_UGLEXT ))
        return 0;
    if ( CHAR_IS_KANA( i )     && !( pFont->pMetrics->fsDefn & FOCA_CHARSET_KANA ))
        return 0;
    if ( CHAR_IS_THAI( i )     && !( pFont->pMetrics->fsDefn & FOCA_CHARSET_THAI ))
        return 0;

    return ( i );
}


/* ------------------------------------------------------------------------- *
 * ParseOS2FontResource                                                      *
 *                                                                           *
 * Parses a standard GPI-format OS/2 font resource.  Takes as input an       *
 * already-populated memory buffer containing the raw font resource data.    *
 * On successful return, the fields within the OS2FONTRESOURCE structure     *
 * will point to the appropriate structures within the font file data.       *
 *                                                                           *
 * NOTE: Even if this function returns success, the pPanose, pKerning and    *
 * pEnd fields of the pFont structure may be NULL.  The application must     *
 * check for this if it intends to use these fields.                         *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PVOID            pBuffer : Buffer containing the font resource data.(I) *
 *   ULONG            cbBuffer: Size of pBuffer in bytes.                (I) *
 *   POS2FONTRESOURCE pFont   : Pointer to an OS2FONTRESOURCE structure      *
 *                              which will receive the parsed font data. (O) *
 *                                                                           *
 * RETURNS: ULONG                                                            *
 *   0 on success or ERR_* on error                                          *
 * ------------------------------------------------------------------------- */
ULONG ParseOS2FontResource( PVOID pBuffer, ULONG cbBuffer, POS2FONTRESOURCE pFont )
{
    PGENERICRECORD pRecord;

    // Verify the file format
    pRecord = (PGENERICRECORD) pBuffer;
    if ( ( pRecord->Identity != SIG_OS2FONTSTART ) ||
         ( pRecord->ulSize != sizeof( OS2FONTSTART )))
        return ERR_FILE_FORMAT;

    // Now set the pointers in our font type to the correct offsets
    pFont->pSignature = (POS2FONTSTART) pBuffer;
    pFont->pMetrics   = (POS2FOCAMETRICS)( (PBYTE) pBuffer + sizeof( OS2FONTSTART ));
    pFont->pKerning   = NULL;
    pFont->pPanose    = NULL;
    pRecord           = (PGENERICRECORD)( (PBYTE)pFont->pMetrics + pFont->pMetrics->ulSize );
    if ( pRecord->Identity != SIG_OS2FONTDEF ) {
        return ERR_FILE_FORMAT;
    }
    pFont->pFontDef  = (POS2FONTDEFHEADER) pRecord;
    if ( pFont->pFontDef->fsChardef == OS2FONTDEF_CHAR3 ) {
        pFont->data.pABC = (POS2CHARDEF3)( (PBYTE) pRecord + sizeof( OS2FONTDEFHEADER ));
    }
    else {
        pFont->data.pChars = (POS2CHARDEF1)( (PBYTE) pRecord + sizeof( OS2FONTDEFHEADER ));
    }
    pRecord = (PGENERICRECORD)( (PBYTE) pRecord + pFont->pFontDef->ulSize );

    if ( pFont->pMetrics->usKerningPairs  && ( pRecord->Identity == SIG_OS2KERN )) {
        pFont->pKerning = (POS2KERNPAIRTABLE) pRecord;
        /* Advance to the next record (whether OS2ADDMETRICS or OS2FONTEND).
         * This is a guess; since the actual format, and thus size, of the
         * kerning information is unclear (see remarks in gpifont.h), there is
         * no guarantee this will work.  Fortunately, we've already parsed the
         * important stuff.
         */
        pRecord = (PGENERICRECORD)
                     ( (PBYTE)pRecord + sizeof( OS2KERNPAIRTABLE ) +
                       ( pFont->pMetrics->usKerningPairs * sizeof ( OS2KERNINGPAIRS )));
    }
    if ( pRecord->Identity == SIG_OS2ADDMETRICS ) {
        pFont->pPanose = (POS2ADDMETRICS) pRecord;
        pRecord = (PGENERICRECORD)( (PBYTE)pRecord + pRecord->ulSize );
    }

    /* We set the pointer to the end signature, but there's really no need to
     * use it for anything.  We check the Identity field to make sure it's
     * valid (if we did miscalculate the kern table size above, then it could
     * well be wrong) before setting the pointer.
     */
    if ( pRecord->Identity == SIG_OS2FONTEND )
        pFont->pEnd = (POS2FONTEND) pRecord;

    pFont->cbSize = cbBuffer;
    return 0;
}


/* ------------------------------------------------------------------------- *
 * ReadOS2FNTFile                                                            *
 *                                                                           *
 * Opens, reads, and parses a standard GPI-format OS/2 font from a FNT file  *
 * (as produced by the Font Editor).  It does NOT read compiled fonts        *
 * (FON/DLL or EXE resources), which are handled by ReadOS2FontResource().   *
 *                                                                           *
 * On successful return, the font contents will be read into the unformatted *
 * buffer pointed to by ppBuffer, which is allocated by this function.  This *
 * buffer should then be passed to ParseOS2FontResource(), and will be       *
 * retained in the resulting OS2FONTRESOURCE structure.                      *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   FILE  *pf      : Handle to the already-opened file.                 (I) *
 *   PBYTE *ppBuffer: Pointer to the buffer which will be created.       (O) *
 *   PULONG pulSize : Pointer to the created buffer size.                (O) *
 *                                                                           *
 * RETURNS: ULONG                                                            *
 *   0 on success, in which case ppBuffer will point to the read file        *
 *   contents.  ERR_* on failure (in which case ppBuffer will be unchanged). *
 * ------------------------------------------------------------------------- */
ULONG ReadOS2FNTFile( FILE *pf, PBYTE *ppBuffer, PULONG pulSize )
{
    struct stat   fs = {0};    // file information structure
    size_t        cbRead;
    ULONG         ulRC;           // return code variable
    PBYTE         pBuf;           // raw buffer containing file contents
    GENERICRECORD startrec;


    // Get the file size
    if ( _FILE_STAT( pf, &fs ))
        return ERR_FILE_STAT;
    if (( fs.st_size < sizeof( GENERICRECORD )) ||
        ( ! _FILE_READ( pf, &startrec, sizeof( GENERICRECORD ))))
        return ERR_FILE_FORMAT;
    if ( _FILE_SEEK( pf, 0 ))
        return ERR_FILE_READ;

    if ( startrec.Identity != SIG_OS2FONTSTART )
        return ERR_FILE_FORMAT;

    // Allocate our read/write buffer using the file size
    pBuf = (char *) calloc( fs.st_size, 1 );
    if ( !pBuf )
        return ERR_MEMORY;

    /* Read the file contents into memory (this shouldn't be too expensive, as
     * an average FNT file is only around 15-30 KB, and even the largest font
     * shouldn't be more than about twice that).
     */
    cbRead = _FILE_READ( pf, pBuf, fs.st_size );
    if ( !cbRead ) {
        free( pBuf );
        return ERR_FILE_READ;
    }
    *ppBuffer = pBuf;
    *pulSize  = fs.st_size;
    return 0;
}


/* ------------------------------------------------------------------------- *
 * ReadOS2FontResource                                                       *
 *                                                                           *
 * Extracts and parses a font face from the specified file.  This requires   *
 * reading the binary resources from an OS/2 executable (program or DLL).    *
 *                                                                           *
 * On successful return, the fields within the OS2FONTRESOURCE structure     *
 * will point to the appropriate structures within the font file data.  The  *
 * entire font is read into an allocated memory buffer which must be freed   *
 * when no longer needed.  (The pSignature pointer within OS2FONTRESOURCE    *
 * corresponds to the start of the buffer, and can be used as a reference to *
 * the allocated buffer as a whole.)                                         *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PSZ              pszFile : Fully-qualified name of the font file.   (I) *
 *   ULONG            ulFace  : Font (face) number within the file to        *
 *                              retrieve (where 0 is the first font).    (I) *
 *   PULONG           pulCount: Total number of faces found in file.     (O) *
 *   POS2FONTRESOURCE pFont   : Pointer to an OS2FONTRESOURCE structure      *
 *                              which will receive the parsed font data. (O) *
 *                                                                           *
 * RETURNS: ULONG                                                            *
 *   0 if the font was successfully read and parsed, ERR_* otherwise.        *
 * ------------------------------------------------------------------------- */
ULONG ReadOS2FontResource( PSZ pszFile, ULONG ulFace, PULONG pulCount, POS2FONTRESOURCE pFont )
{
    ULONG       ulAddr,         // address of the new-style EXE header
                ulFaceCount,    // number of faces found
                ulResID,        // target resource ID (when fontdir is used)
                cbFont = 0,     // size of found font resource
                cbInc,          // offset increment of current resource entry
                i,
                ulRC;
    USHORT      usMagic,        // 2-byte magic number
                cb_rte;         // size of a resource entry
    BOOL        fFound;         // has the requested font been found?
    PBYTE       pBuf;
    FILE        *pf;

#ifdef DEBUG_DUMP_RESOURCE
    FILE *tf;
#endif


    ulRC        = ERR_NO_FONT;
    fFound      = FALSE;
    ulFaceCount = 0;
    ulResID     = 0;
    pBuf        = NULL;

    // Open the file
    if (( pf = _FILE_OPEN( pszFile )) == NULL )
        return ERR_FILE_OPEN;

    // See if it's an executable (EXE or DLL)
    if ( ! _FILE_READ( pf, &usMagic, 2 )) goto read_fail;

    if ( usMagic == MAGIC_MZ ) {
        // Locate the new-type executable header
        if ( _FILE_SEEK( pf, EH_OFFSET_ADDRESS )) goto read_fail;
        if ( ! _FILE_READ( pf, &ulAddr, 4 ))      goto read_fail;
        // Read the 2-byte magic number from this address
        if ( _FILE_SEEK( pf, ulAddr ))            goto read_fail;
        if ( ! _FILE_READ( pf, &usMagic, 2 ))     goto read_fail;
        // Now back up to the start of the new-type header again
        if ( _FILE_SEEK( pf, ulAddr ))            goto read_fail;
    }
    else if (( usMagic == MAGIC_LX ) || ( usMagic == MAGIC_NE )) {
        // No stub header, just start at the beginning of the file
        ulAddr = 0;
        if ( _FILE_SEEK( pf, 0 )) goto read_fail;
    }
    else {
        // Not a compiled (exe) font module
        // (non-compiled fonts cannot contain more than one face)
        if ( ulFace ) return ERR_NO_FONT;

        // Try to read it as a raw font file and then return
        if ( _FILE_SEEK( pf, 0 )) goto read_fail;
        ulRC = ReadOS2FNTFile( pf, &pBuf, &cbFont );
        if ( ulRC != 0 ) goto read_fail;
        ulRC = ParseOS2FontResource( pBuf, cbFont, pFont );
        if ( ulRC != 0 ) {
            free( pBuf );
            goto read_fail;
        }
        fFound = TRUE;
        ulFaceCount = 1;
        goto done;
    }

    // Identify the executable type and parse the resource data accordingly
    if ( usMagic == MAGIC_LX )
    {
        LXHEADER  lx_hd;   // executable header
        LXRTENTRY lx_rte;  // resource table entry

        if ( ! _FILE_READ( pf, &lx_hd, sizeof( LXHEADER ))) goto read_fail;

        // Make sure the file actually contains resources...
        if ( !lx_hd.cres ) {
            ulRC = ERR_FILE_FORMAT;
            goto done;
        }
        // ...and at least as many as the number of the font requested
        if ( ulFace >= lx_hd.cres )  {
            ulRC = ERR_NO_FONT;
            goto done;
        }

        // Now look for font resources
        cb_rte = sizeof( LXRTENTRY );
        for ( i = 0; i < lx_hd.cres; i++ ) {
            cbInc = cb_rte * i;
            if ( _FILE_SEEK( pf, ulAddr + lx_hd.res_tbl + cbInc )) goto read_fail;
            if ( ! _FILE_READ( pf, &lx_rte, cb_rte ))              goto read_fail;

            /* If ulResID is non-0 then we've already found & parsed a font
             * directory resource (see below), so we look for the resource with
             * that as its ID.  Otherwise, look for the ulFace'th font resource
             * found.
             */
            if ( lx_rte.type == OS2RES_FONTFACE ) {
                if ( !ulResID ) ulFaceCount++;
                // If we've already got our font then just count the remaining ones
                if ( fFound ) continue;
                if ((( ulFaceCount - 1 ) != ulFace ) &&
                    !( ulResID && ( lx_rte.name == ulResID )))
                    continue;
            }
            if (( lx_rte.type != OS2RES_FONTDIR ) &&
                !( ulResID && ( lx_rte.name == ulResID )))
                continue;

            /* This is either our target font, or else a font directory
             * resource.  Either way, extract the resource data.
             */
            pBuf = NULL;
            if ( !LXExtractResource( pf, lx_hd, lx_rte, ulAddr, &pBuf, &cbFont ) || !pBuf )
                goto read_fail;

#ifdef DEBUG_DUMP_RESOURCE
            tf = fopen( tmpnam(NULL), "wb");
            fwrite( pBuf+lx_rte.offset, 1, lx_rte.cb, tf );
            fclose( tf );
#endif

            if ( lx_rte.type == OS2RES_FONTDIR ) {
                /* If a font directory exists we use that to find the face's
                 * resource ID, as in this case it is not guaranteed to have
                 * a type of OS2RES_FONTFACE (7).
                 */
                 POS2FONTDIRECTORY pFD = (POS2FONTDIRECTORY)(pBuf + lx_rte.offset);

                 ulFaceCount = pFD->usnFonts;
                 if ( pFD->usnFonts < ( ulFace + 1 )) {
                    ulRC = ERR_NO_FONT;
                    free( pBuf );
                    goto done;
                 }
                 /* Set ulResID to the ID of the requested font number, then
                  * continue scanning the resource table.
                  */
                 ulResID = pFD->fntEntry[ ulFace ].usIndex;
                 free( pBuf );
            }
            else {
                /* pBuf contains our font, so parse it.
                 */
                ulRC = ParseOS2FontResource( pBuf + lx_rte.offset, lx_rte.cb, pFont );
                if ( ulRC != 0 ) goto read_fail;
                fFound = TRUE;
                /* If we successfully read a font directory resource, we already
                 * have the total number of fonts - we don't need to count the
                 * remaining fonts iteratively, so we can just finish up now.
                 */
                if ( ulResID ) goto done;
            }

        }
        if ( !fFound ) {
            ulRC = ERR_NO_FONT;
        }
    }
#if 0
    else if ( strcmp( cchSig, "NE") == 0 )
    {
        NEHEADER    ne_hd;
        NERTENTRY   ne_rte;

        if ( DosRead( hFile, &ne_hd, sizeof( NEHEADER ), &cb )) goto read_fail;
        if ( !ne_hd.cres ) {
            ulRC = ERR_FILE_FORMAT;
            goto done;
        }
        cb_rte = sizeof( NERTENTRY );
        for ( i = 0; i < ne_hd.cres; i++ ) {
            cbInc = cb_rte * i;
            if ( DosSetFilePtr( hFile, ne_hd.res_tbl + cbInc, FILE_BEGIN, &cb )) goto read_fail;
            if ( DosRead( hFile, &ne_rte, cb_rte, &cb )) goto read_fail;
            if ( ne_rte.type == 7 ) ulFaceCount++;
            else continue;
            if ( ulFaceCount != ulFace ) continue;

        }

    }
#endif
    else {
        ulRC = ERR_FILE_FORMAT;
    }

    goto done;

read_fail:
    ulRC = ERR_FILE_READ;
done:
    _FILE_CLOSE( pf );
    *pulCount = ulFaceCount;
    return ulRC;
}


