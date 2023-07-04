/*****************************************************************************
 * abr.c                                                                     *
 *                                                                           *
 * Functions specific to Associated Bitmap Rule format.                      *
 *                                                                           *
 *  OS/2-GPI Composite Font Editor/Inspector                                 *
 *  Copyright (C) 2016-2023 Alexander Taylor                                 *
 *                                                                           *
 *  This program is free software: you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation, either version 3 of the License, or        *
 *  (at your option) any later version.                                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License        *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *                                                                           *
 *****************************************************************************/

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_PM
#define INCL_WIN
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ids.h"
#include "cmbfont.h"                        // includes unifont.h
#include "gpifont.h"
#include "compfont.h"



/* ------------------------------------------------------------------------- *
 * ParseFont_ABR                                                             *
 *                                                                           *
 * Opens and parses an associated bitmap rule file.                          *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PGENERICRECORD pStart: pointer to the start of the font                 *
 *   PCFEGLOBAL pGlobal: pointer to global program data.                     *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE on success, FALSE if an error occurred.                            *
 * ------------------------------------------------------------------------- */
BOOL ParseFont_ABR( PGENERICRECORD pStart, PCFEGLOBAL pGlobal )
{
    ULONG cbAssocArray;      // Size of font association array
    // These three pointers are offsets into the file data as read from disk
    PABRFILESIGNATURE pFileSig;
    PFONTASSOCIATION  pFileAssocs;
    PABRFILEEND       pFileEnd;
    // This is our internal working copy of the font data (in pGlobal)
    PABRFILE          pWorkingFont;


    // Get pointers to the various parts of the font data
    pFileSig = (PABRFILESIGNATURE) pStart;
    pFileAssocs = (PFONTASSOCIATION)( (PBYTE) pStart + pFileSig->ulSize );
    cbAssocArray = ( pFileSig->ulCount + 1 ) * pFileAssocs->ulSize;
    pFileEnd = (PABRFILEEND)( (PBYTE) pFileAssocs + cbAssocArray );

    // Allocate separate new buffers for each portion of the file
    // so we can more easily add or remove parts later

    pWorkingFont = &(pGlobal->font.abr);

    pWorkingFont->pSignature = (PABRFILESIGNATURE) malloc( pFileSig->ulSize );
    if ( !pWorkingFont->pSignature )
        return FALSE;
    pWorkingFont->pAssociations = (PFONTASSOCIATION) malloc( cbAssocArray );
    if ( !pWorkingFont->pAssociations ) {
        wrap_free( (PPVOID) &(pWorkingFont->pSignature) );
        return FALSE;
    }
    pWorkingFont->pEnd = (PABRFILEEND) malloc( pFileEnd->ulSize );
    if ( !pWorkingFont->pEnd) {
        wrap_free( (PPVOID) &(pWorkingFont->pSignature) );
        wrap_free( (PPVOID) &(pWorkingFont->pEnd) );
        return FALSE;
    }
    memcpy( pWorkingFont->pSignature, pFileSig, pFileSig->ulSize );
    memcpy( pWorkingFont->pAssociations, pFileAssocs, cbAssocArray );
    memcpy( pWorkingFont->pEnd, pFileEnd, pFileEnd->ulSize );

    return TRUE;
}


/* ------------------------------------------------------------------------- *
 * PopulateValues_ABR                                                        *
 *                                                                           *
 * Populate the main window UI with the data read from a newly-opened font   *
 * bitmap rules (ABR) file.                                                  *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd : handle of the main program dialog.                          *
 *   PCFEGLOBAL pGlobal: pointer to global program data.                     *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void PopulateValues_ABR( HWND hwnd, PCFEGLOBAL pGlobal )
{
    PABRFILE         pABR;
    PFONTASSOCIATION pAssociation;
    ULONG            ulCB,
                     ulAssoc,
                     i;
    HWND             hwndCnr;
    PCFRECORD        pRec,
                     pFirst;
    RECORDINSERT     ri;
    CHAR             szError[ 256 ];

    pABR = &(pGlobal->font.abr);
    if ( !strcmp( pABR->pSignature->szSignature, "Associated Bitmap-fonts Rule")) {

        // now populate the container with the component-font information
        hwndCnr = WinWindowFromID( hwnd, IDD_COMPONENTS );
        ulCB = sizeof( CFRECORD ) - sizeof( MINIRECORDCORE );
        pRec = (PCFRECORD) WinSendMsg( hwndCnr, CM_ALLOCRECORD,
                                       MPFROMLONG( ulCB ),
                                       MPFROMLONG( pABR->pSignature->ulCount + 1 ));
        pFirst = pRec;
        ulCB = sizeof( MINIRECORDCORE );
        ulAssoc = pABR->pAssociations->ulSize;

        for ( i = 0; i <= pABR->pSignature->ulCount; i++ ) {
            pAssociation = (PFONTASSOCIATION)( (PBYTE) pABR->pAssociations + ( i * ulAssoc ));
            if ( pAssociation->Identity != SIG_FTAS ) continue;
            ulAssoc = pAssociation->ulSize;

            pRec->pszFace      = (PSZ) calloc( FACESIZE, 1 );
            pRec->pszRanges    = (PSZ) calloc( SZRANGES_MAXZ, 1 );
            pRec->pszGlyphList = (PSZ) calloc( GLYPHNAMESIZE, 1 );
            pRec->pszFlags     = (PSZ) calloc( SZFLAGS_MAXZ, 1 );

            strcpy( pRec->pszFace, pAssociation->unifm.ifiMetrics.szFacename );
            if ( pAssociation->ulGlyphRanges > 1 )
                sprintf( pRec->pszRanges, "(%u ranges)", pAssociation->ulGlyphRanges );
            else if ( pAssociation->ulGlyphRanges == 1 ) {
                sprintf( pRec->pszRanges, "%u - %u",
                         pAssociation->GlyphRange[0].giStart,
                         pAssociation->GlyphRange[0].giEnd );
            }
            else
                strcpy( pRec->pszRanges, "*");
            sprintf( pRec->pszGlyphList, "%ld", pAssociation->unifm.ifiMetrics.lCapEmHeight );
            sprintf( pRec->pszFlags, "0x%X", pAssociation->flFlags );
            pRec->record.cb = ulCB;
            pRec->record.pszIcon = pRec->pszFace;
            pRec->ulIndex = i;
            pRec = (PCFRECORD) pRec->record.preccNextRecord;
        }
        ri.cb                = sizeof( RECORDINSERT );
        ri.pRecordOrder      = (PRECORDCORE) CMA_END;
        ri.pRecordParent     = NULL;
        ri.zOrder            = (ULONG) CMA_TOP;
        ri.fInvalidateRecord = TRUE;
        ri.cRecordsInsert    = pABR->pSignature->ulCount + 1;
        WinSendMsg( hwndCnr, CM_INSERTRECORD, MPFROMP( pFirst ), MPFROMP( &ri ));
    }
    else {
        sprintf( szError, "Unrecognized file signature: %200s", pABR->pSignature->szSignature );
        WinMessageBox( HWND_DESKTOP, hwnd, szError, "Unsupported Format",
                       0, MB_MOVEABLE | MB_OK | MB_ERROR );
    }
}


/* ------------------------------------------------------------------------- *
 * SetupCnrAB                                                                *
 *                                                                           *
 * Sets up the associated bitmap rule container.                             *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd : handle of the main program dialog.                          *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void SetupCnrAB( HWND hwnd )
{
    HWND            hwndCnr;
    CNRINFO         cnr = {0};
    PFIELDINFO      pFld,
                    pFld1st;
    FIELDINFOINSERT finsert;
    POINTL          aptl[ 2 ];

    WinSetDlgItemText( hwnd, IDD_STATUS, "Associated Bitmap Rules");

    // update the container & groupbox layout
    WinSetDlgItemText( hwnd, IDD_GROUPBOX, "Associated fonts");
    aptl[0].x = 285;
    aptl[0].y = 120;
    aptl[1].x = 275;
    aptl[1].y = 92;
    WinMapDlgPoints( hwnd, aptl, 2, TRUE );
    WinSetWindowPos( WinWindowFromID( hwnd, IDD_COMPONENTS ), HWND_TOP,
                     0, 0, aptl[1].x, aptl[1].y, SWP_SIZE | SWP_ZORDER );
    WinSetWindowPos( WinWindowFromID( hwnd, IDD_GROUPBOX ), NULLHANDLE,
                     0, 0, aptl[0].x, aptl[0].y, SWP_SIZE );

    // set up the font information container
    hwndCnr = WinWindowFromID( hwnd, IDD_COMPONENTS );
    WinSendMsg( hwndCnr, CM_REMOVEDETAILFIELDINFO, MPVOID,
                MPFROM2SHORT( 0, CMA_INVALIDATE | CMA_FREE ));
    cnr.flWindowAttr  = CV_DETAIL | CA_DETAILSVIEWTITLES;
    cnr.pszCnrTitle   = "Component Fonts";
    cnr.cyLineSpacing = 2;
    WinSendMsg( hwndCnr, CM_SETCNRINFO, MPFROMP( &cnr ),
                MPFROMLONG( CMA_FLWINDOWATTR | CMA_LINESPACING ));
    pFld = (PFIELDINFO) WinSendMsg( hwndCnr, CM_ALLOCDETAILFIELDINFO,
                                    MPFROMLONG( 3L ), MPVOID );
    pFld1st = pFld;
    // (first column: font face name)
    pFld->cb = sizeof( FIELDINFO );
    pFld->pTitleData = "Face Name";
    pFld->flData     = CFA_STRING | CFA_FIREADONLY | CFA_VCENTER | CFA_HORZSEPARATOR | CFA_SEPARATOR;
    pFld->offStruct  = FIELDOFFSET( CFRECORD, pszFace );
    pFld = pFld->pNextFieldInfo;
    // (second column: character cell size, we use the glyphlist field for this)
    pFld->cb = sizeof( FIELDINFO );
    pFld->pTitleData = "Character Cell Size";
    pFld->flData     = CFA_STRING | CFA_FIREADONLY | CFA_VCENTER | CFA_HORZSEPARATOR | CFA_SEPARATOR;
    pFld->offStruct  = FIELDOFFSET( CFRECORD, pszGlyphList );
    pFld = pFld->pNextFieldInfo;
    // (third column: associated glyph ranges)
    pFld->cb = sizeof( FIELDINFO );
    pFld->pTitleData = "Flags";
    pFld->flData     = CFA_STRING | CFA_FIREADONLY | CFA_VCENTER | CFA_HORZSEPARATOR;
    pFld->offStruct  = FIELDOFFSET( CFRECORD, pszFlags );
    pFld = pFld->pNextFieldInfo;

    finsert.cb                   = (ULONG) sizeof( FIELDINFOINSERT );
    finsert.pFieldInfoOrder      = (PFIELDINFO) CMA_END;
    finsert.fInvalidateFieldInfo = TRUE;
    finsert.cFieldInfoInsert     = 3;
    WinSendMsg( hwndCnr, CM_INSERTDETAILFIELDINFO,
                MPFROMP( pFld1st ), MPFROMP( &finsert ));

}


