/*****************************************************************************
 * unifont.c                                                                 *
 *                                                                           *
 * Functions specific to the Uni-font format.                                *
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
#include "gpifont.h"                        // for GENERICRECORD
#include "gllist.h"                         // generic linked list
#include "compfont.h"



/* ------------------------------------------------------------------------- *
 * ParseFont_UNI                                                             *
 *                                                                           *
 * Opens and parses a Uni-font file.                                         *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PGENERICRECORD pStart: pointer to the start of the font                 *
 *   PCFEGLOBAL pGlobal: pointer to global program data                      *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE on success, FALSE if an error occurred.                            *
 * ------------------------------------------------------------------------- */
BOOL ParseFont_UNI( PGENERICRECORD pStart, PCFEGLOBAL pGlobal )
{
    PUNIFONTDIRECTORY pFileUniFD;

    pFileUniFD = (PUNIFONTDIRECTORY) pStart;

    // For now we just copy the font file contents directly.
    // If and when we support modifying it, we may change this.
    pGlobal->font.pUFontDir = (PUNIFONTDIRECTORY) malloc( pFileUniFD->ulSize );
    if ( !(pGlobal->font.pUFontDir) )
        return FALSE;

    pGlobal->usType = FONT_TYPE_UNI;
    return TRUE;
}


/* ------------------------------------------------------------------------- *
 * PopulateValues_UNI                                                        *
 *                                                                           *
 * Populate the main window UI with the data read from a newly-opened font   *
 * file (Uni-font version).                                                  *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND       hwnd   : handle of the main program dialog.                  *
 *   PCFEGLOBAL pGlobal: pointer to global program data.                     *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void PopulateValues_UNI( HWND hwnd, PCFEGLOBAL pGlobal )
{
    PUNIFONTRESOURCEENTRY pDirEntry;  // A resource entry in the font directory
    PUNIFONTRESOURCE      pFont;      // Start of the actual font resource

    HWND         hwndCnr;
    PCFRECORD    pRec,
                 pFirst;
    RECORDINSERT ri;
    PSZ          pszFace;
    ULONG        i,
                 ulCB;
/*
    CHAR         szFamilyName[ 256 ];
    USHORT       usRegistry;
    BOOL         fLicensed;
    PSZ          pszFamily;
*/


    if ( !pGlobal->font.pUFontDir->ulUniFontResources )
        return;

/*
    pFont = (PUNIFONTRESOURCE)((PBYTE) pGlobal->font.pUFontDir +
                              pGlobal->font.pUFontDir->FontResEntry[ 0 ].offsetUniFont );

    pszFamily = (( pFont->metrics.flOptions & UNIFONTMETRICS_FULLFAMILYNAME_EXIST ) &&
                 (*(pFont->metrics.szFullFamilyname))) ?
                    pFont->metrics.szFullFamilyname    :
                    pFont->metrics.ifiMetrics.szFamilyname;
    strncpy( szFamilyName, pszFamily, FACESIZE-1 );
    usRegistry = pFont->metrics.ifiMetrics.idRegistry;
    fLicensed = ( pFont->metrics.ifiMetrics.flType & IFIMETRICS32_LICENSED ) ? TRUE : FALSE;
*/

    // Populate the container with the list of fonts included in the file
    hwndCnr = WinWindowFromID( hwnd, IDD_COMPONENTS );
    ulCB = sizeof( CFRECORD ) - sizeof( MINIRECORDCORE );
    pRec = (PCFRECORD) WinSendMsg( hwndCnr, CM_ALLOCRECORD,
                                   MPFROMLONG( ulCB ),
                                   MPFROMLONG( pGlobal->font.pUFontDir->ulUniFontResources ));
    pFirst = pRec;
    ulCB = sizeof( MINIRECORDCORE );

    for ( i = 0; i < pGlobal->font.pUFontDir->ulUniFontResources; i++ ) {
        pDirEntry = (PUNIFONTRESOURCEENTRY) &(pGlobal->font.pUFontDir->FontResEntry[ i ]);
        if ( !pDirEntry->offsetUniFont ) continue;
        pFont = (PUNIFONTRESOURCE)((PBYTE) pGlobal->font.pUFontDir
                                         + pDirEntry->offsetUniFont );
        if (( (ULONG) pFont->unifSignature.Identity != SIG_UNFS ) ||
            ( strcmp( pFont->unifSignature.szSignature, UNIFNT_SIGNATURE ) != 0 ) ||
            ( (ULONG) pFont->unifDefHeader.Identity != SIG_UNFH ))
            continue;
/*
        pszFamily = (( pFont->unifMetrics.flOptions & UNIFONTMETRICS_FULLFAMILYNAME_EXIST ) &&
                     (*(pFont->unifMetrics.szFullFamilyname))) ?
                        pFont->unifMetrics.szFullFamilyname    :
                        pFont->unifMetrics.ifiMetrics.szFamilyname;
*/
        pszFace = (( pFont->unifMetrics.flOptions & UNIFONTMETRICS_FULLFACENAME_EXIST ) &&
                     (*(pFont->unifMetrics.szFullFacename))) ?
                        pFont->unifMetrics.szFullFacename    :
                        pFont->unifMetrics.ifiMetrics.szFacename;
        //if ( strcmp( szFamilyName, pszFamily ) != 0 ) continue;

        pRec->pszFace = strdup( pszFace );
        pRec->pszRanges = (PSZ) calloc( SZRANGES_MAXZ, 1 );     // use Ranges field for font type
        pRec->pszFlags = (PSZ) calloc( SZFLAGS_MAXZ, 1 );
        switch ( pFont->unifDefHeader.flFontDef ) {
            case UNIFONTDEF_TYPE_1_FONTDEF:
                strncpy( pRec->pszRanges, "Fixed-width", SZRANGES_MAXZ-1 );
            case UNIFONTDEF_TYPE_2_FONTDEF:     // == UNIFONTDEF_TYPE_3_FONTDEF
                strncpy( pRec->pszRanges, "Proportional-width", SZRANGES_MAXZ-1 );
            default:
                strncpy( pRec->pszRanges, "Unknown", SZRANGES_MAXZ-1 );
        }
        sprintf( pRec->pszFlags, "0x%X", pDirEntry->flUniFont );

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
    ri.cRecordsInsert    = pGlobal->font.pUFontDir->ulUniFontResources;
    WinSendMsg( hwndCnr, CM_INSERTRECORD, MPFROMP( pFirst ), MPFROMP( &ri ));

}


/* ------------------------------------------------------------------------- *
 * NewFont_UNI                                                               *
 *                                                                           *
 * Create a new, empty Uni-font file.                                        *
 * ------------------------------------------------------------------------- */
BOOL NewFont_UNI( HWND hwnd, PCFEGLOBAL pGlobal )
{
    // Set up the UI for Uni-font if it isn't already
    if ( pGlobal->usType != FONT_TYPE_UNI ) {
        WinSendDlgItemMsg( hwnd, IDD_COMPONENTS,
                           CM_REMOVEDETAILFIELDINFO, MPVOID,
                           MPFROM2SHORT( 0, CMA_INVALIDATE | CMA_FREE ));
        pGlobal->usType = FONT_TYPE_UNI;
        SetupWindowUF( hwnd );
        SetupCnrUF( hwnd );
    }
    return TRUE;
}


/* ------------------------------------------------------------------------- *
 * SetupWindowUF                                                             *
 *                                                                           *
 * Set up the window UI controls for Uni-font format.                        *
 * ------------------------------------------------------------------------- */
void SetupWindowUF( HWND hwnd )
{
    WinShowWindow( WinWindowFromID( hwnd, IDD_FACEGROUP ), FALSE );
    WinShowWindow( WinWindowFromID( hwnd, IDD_FACETEXT ),  FALSE );
    WinShowWindow( WinWindowFromID( hwnd, IDD_FACENAME ),  FALSE );
    WinShowWindow( WinWindowFromID( hwnd, ID_METRICS ),    FALSE );
}


/* ------------------------------------------------------------------------- *
 * SetupCnrUF                                                                *
 *                                                                           *
 * Sets up the Uni-font component container.                                 *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd : handle of the main program dialog.                          *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void SetupCnrUF( HWND hwnd )
{
    HWND            hwndCnr;
    CNRINFO         cnr = {0};
    PFIELDINFO      pFld,
                    pFld1st;
    FIELDINFOINSERT finsert;
    POINTL          aptl[ 2 ];

    WinSetDlgItemText( hwnd, IDD_STATUS, "Uni-Font File");

    // update the container & groupbox layout
    WinSetDlgItemText( hwnd, IDD_GROUPBOX, "Included font resources");
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
    cnr.pszCnrTitle   = "Included Fonts";
    cnr.cyLineSpacing = 2;
    WinSendMsg( hwndCnr, CM_SETCNRINFO, MPFROMP( &cnr ),
                MPFROMLONG( CMA_FLWINDOWATTR | CMA_LINESPACING ));
    pFld = (PFIELDINFO) WinSendMsg( hwndCnr, CM_ALLOCDETAILFIELDINFO,
                                    MPFROMLONG( 3L ), MPVOID );
    pFld1st = pFld;
    // (first column: name)
    pFld->cb = sizeof( FIELDINFO );
    pFld->pTitleData = "Face Name";
    pFld->flData     = CFA_STRING | CFA_FIREADONLY | CFA_VCENTER | CFA_HORZSEPARATOR | CFA_SEPARATOR;
    pFld->offStruct  = FIELDOFFSET( CFRECORD, pszFace );
    pFld = pFld->pNextFieldInfo;
    // (second column: font type)
    pFld->cb = sizeof( FIELDINFO );
    pFld->pTitleData = "Type";
    pFld->flData     = CFA_STRING | CFA_FIREADONLY | CFA_VCENTER | CFA_HORZSEPARATOR;
    pFld->offStruct  = FIELDOFFSET( CFRECORD, pszRanges );
    pFld = pFld->pNextFieldInfo;
    // (third column: flags)
    pFld->cb = sizeof( FIELDINFO );
    pFld->pTitleData = "Flags";
    pFld->flData     = CFA_STRING | CFA_FIREADONLY | CFA_VCENTER | CFA_HORZSEPARATOR | CFA_SEPARATOR;
    pFld->offStruct  = FIELDOFFSET( CFRECORD, pszFlags );

    finsert.cb                   = (ULONG) sizeof( FIELDINFOINSERT );
    finsert.pFieldInfoOrder      = (PFIELDINFO) CMA_END;
    finsert.fInvalidateFieldInfo = TRUE;
    finsert.cFieldInfoInsert     = 3;
    WinSendMsg( hwndCnr, CM_INSERTDETAILFIELDINFO,
                MPFROMP( pFld1st ), MPFROMP( &finsert ));

}


/* ------------------------------------------------------------------------- *
 * AddUniFont                                                                *
 *                                                                           *
 * Brings up the dialog to add a new font to a Uni-font file.                *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd         : handle of the main program dialog.                  *
 *   PCFEGLOBAL pGlobal: pointer to global program data.                     *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void AddUniFont( HWND hwnd, PCFEGLOBAL pGlobal )
{
    UFPROPS         ufprops;
    UNIFONTRESOURCE font = {0};

    ufprops.cb = sizeof( UFPROPS );
    ufprops.hab = pGlobal->hab;
    ufprops.hmq = pGlobal->hmq;
    ufprops.hwndMain = hwnd;
    ufprops.fEditExisting = FALSE;
    ufprops.pFontHeader = &font;
    ufprops.pFontData = NULL;
    WinDlgBox( HWND_DESKTOP, hwnd, (PFNWP) UniFontDlgProc,
               NULLHANDLE, IDD_UNIFONT, &ufprops );
}


/* ------------------------------------------------------------------------- *
 * UniFontDlgProc                                                            *
 *                                                                           *
 * Dialog procedure for the Uni-font properties dialog.                      *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY UniFontDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    PUFPROPS        pProps;
    HWND            hwndCnr;
    PFIELDINFO      pFld,
                    pFld1st;
    CNRINFO         cnr;
    FIELDINFOINSERT fi;
    PUFRECORD       pRec;


    switch ( msg ) {

        case WM_INITDLG:
            pProps = (PUFPROPS) mp2;

            // Set up the container
            hwndCnr = WinWindowFromID( hwnd, IDD_UNIGLYPHS );
            memset( &cnr, 0, sizeof( CNRINFO ));
            cnr.flWindowAttr  = CV_DETAIL | CA_DETAILSVIEWTITLES;
            cnr.cyLineSpacing = 1;
            WinSendMsg( hwndCnr, CM_SETCNRINFO, MPFROMP( &cnr ),
                        MPFROMLONG( CMA_FLWINDOWATTR | CMA_LINESPACING ));
            pFld = (PFIELDINFO) WinSendMsg( hwndCnr,
                                            CM_ALLOCDETAILFIELDINFO,
                                            MPFROMLONG( 3L ), 0 );
            pFld1st = pFld;
            // (first column: source glyph range)
            pFld->cb = sizeof( FIELDINFO );
            pFld->pTitleData = "Range";
            pFld->flData     = CFA_STRING | CFA_FIREADONLY | CFA_VCENTER | CFA_HORZSEPARATOR | CFA_SEPARATOR;
            pFld->offStruct  = FIELDOFFSET( UFRECORD, pszRanges );
            pFld = pFld->pNextFieldInfo;
            // (second column: font information item value)
            pFld->cb = sizeof( FIELDINFO );
            pFld->pTitleData = "Offset";
            pFld->flData     = CFA_STRING | CFA_FIREADONLY | CFA_VCENTER | CFA_HORZSEPARATOR | CFA_SEPARATOR;
            pFld->offStruct  = FIELDOFFSET( UFRECORD, pszTarget );
            pFld = pFld->pNextFieldInfo;
            // (third column: font information item value)
            pFld->cb = sizeof( FIELDINFO );
            pFld->pTitleData = "Notes";
            pFld->flData     = CFA_STRING | CFA_FIREADONLY | CFA_VCENTER | CFA_HORZSEPARATOR;
            pFld->offStruct  = FIELDOFFSET( UFRECORD, pszDesc );

            fi.cb                   = (ULONG) sizeof( FIELDINFOINSERT );
            fi.pFieldInfoOrder      = (PFIELDINFO) CMA_END;
            fi.fInvalidateFieldInfo = TRUE;
            fi.cFieldInfoInsert     = 3;
            WinSendMsg( hwndCnr, CM_INSERTDETAILFIELDINFO,
                        MPFROMP( pFld1st ), MPFROMP( &fi ));

            // Populate the width & weight drop-downs
            WinSendDlgItemMsg( hwnd, IDD_WEIGHT, LM_INSERTITEM,
                               MPFROMSHORT( 0 ), MPFROMP( FONT_WEIGHT_1 ));
            WinSendDlgItemMsg( hwnd, IDD_WEIGHT, LM_INSERTITEM,
                               MPFROMSHORT( 1 ), MPFROMP( FONT_WEIGHT_2 ));
            WinSendDlgItemMsg( hwnd, IDD_WEIGHT, LM_INSERTITEM,
                               MPFROMSHORT( 2 ), MPFROMP( FONT_WEIGHT_3 ));
            WinSendDlgItemMsg( hwnd, IDD_WEIGHT, LM_INSERTITEM,
                               MPFROMSHORT( 3 ), MPFROMP( FONT_WEIGHT_4 ));
            WinSendDlgItemMsg( hwnd, IDD_WEIGHT, LM_INSERTITEM,
                               MPFROMSHORT( 4 ), MPFROMP( FONT_WEIGHT_5 ));
            WinSendDlgItemMsg( hwnd, IDD_WEIGHT, LM_INSERTITEM,
                               MPFROMSHORT( 5 ), MPFROMP( FONT_WEIGHT_6 ));
            WinSendDlgItemMsg( hwnd, IDD_WEIGHT, LM_INSERTITEM,
                               MPFROMSHORT( 6 ), MPFROMP( FONT_WEIGHT_7 ));
            WinSendDlgItemMsg( hwnd, IDD_WEIGHT, LM_INSERTITEM,
                               MPFROMSHORT( 7 ), MPFROMP( FONT_WEIGHT_8 ));
            WinSendDlgItemMsg( hwnd, IDD_WEIGHT, LM_INSERTITEM,
                               MPFROMSHORT( 8 ), MPFROMP( FONT_WEIGHT_9 ));
            WinSendDlgItemMsg( hwnd, IDD_WEIGHT, LM_SELECTITEM,
                               MPFROMSHORT( 4 ), MPFROMSHORT( TRUE ));

            WinSendDlgItemMsg( hwnd, IDD_WIDTH, LM_INSERTITEM,
                               MPFROMSHORT( 0 ), MPFROMP( FONT_WIDTH_1 ));
            WinSendDlgItemMsg( hwnd, IDD_WIDTH, LM_INSERTITEM,
                               MPFROMSHORT( 1 ), MPFROMP( FONT_WIDTH_2 ));
            WinSendDlgItemMsg( hwnd, IDD_WIDTH, LM_INSERTITEM,
                               MPFROMSHORT( 2 ), MPFROMP( FONT_WIDTH_3 ));
            WinSendDlgItemMsg( hwnd, IDD_WIDTH, LM_INSERTITEM,
                               MPFROMSHORT( 3 ), MPFROMP( FONT_WIDTH_4 ));
            WinSendDlgItemMsg( hwnd, IDD_WIDTH, LM_INSERTITEM,
                               MPFROMSHORT( 4 ), MPFROMP( FONT_WIDTH_5 ));
            WinSendDlgItemMsg( hwnd, IDD_WIDTH, LM_INSERTITEM,
                               MPFROMSHORT( 5 ), MPFROMP( FONT_WIDTH_6 ));
            WinSendDlgItemMsg( hwnd, IDD_WIDTH, LM_INSERTITEM,
                               MPFROMSHORT( 6 ), MPFROMP( FONT_WIDTH_7 ));
            WinSendDlgItemMsg( hwnd, IDD_WIDTH, LM_INSERTITEM,
                               MPFROMSHORT( 7 ), MPFROMP( FONT_WIDTH_8 ));
            WinSendDlgItemMsg( hwnd, IDD_WIDTH, LM_INSERTITEM,
                               MPFROMSHORT( 8 ), MPFROMP( FONT_WIDTH_9 ));
            WinSendDlgItemMsg( hwnd, IDD_WIDTH, LM_SELECTITEM,
                               MPFROMSHORT( 4 ), MPFROMSHORT( TRUE ));

            CentreWindow( hwnd, pProps->hwndMain );
            break;
        // WM_INITDLG


        case WM_COMMAND:
            switch( SHORT1FROMMP( mp1 )) {

                case IDD_UNIIMPORT:
                    return (MRESULT) 0;

                case IDD_UNIDELETE:
                    return (MRESULT) 0;

            }
            break;
        // WM_COMMAND


        case WM_DESTROY:
            // free the allocated container memory
            hwndCnr = WinWindowFromID( hwnd, IDD_UNIGLYPHS );
            pRec = (PUFRECORD) WinSendMsg( hwndCnr, CM_QUERYRECORD, MPVOID,
                                           MPFROM2SHORT( CMA_FIRST, CMA_ITEMORDER ));
            while ( pRec ) {
                free( pRec->pszRanges );
                free( pRec->pszTarget );
                if ( pRec->pszDesc ) free( pRec->pszDesc );
                pRec = (PUFRECORD) pRec->record.preccNextRecord;
            }
            WinSendMsg( hwndCnr, CM_REMOVERECORD, MPVOID,
                        MPFROM2SHORT( 0, CMA_INVALIDATE | CMA_FREE ));
            WinSendMsg( hwndCnr, CM_REMOVEDETAILFIELDINFO, MPVOID,
                        MPFROM2SHORT( 0, CMA_INVALIDATE | CMA_FREE ));
            break;
        // WM_DESTROY



        default: break;
    }

    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}



