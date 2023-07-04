/*****************************************************************************
 * combined.c                                                                *
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
#include "gpifont.h"
#include "compfont.h"


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
    PUNIFONTHEADER pFont;
    CHAR           szFamilyName[ 256 ];
    USHORT         usRegistry;
    BOOL           fLicensed;
    PSZ            pszFamily, pszFace;
    ULONG          i;


    if ( !pGlobal->font.pUFontDir->ulUniFontResources )
        return;

    pFont = (PUNIFONTHEADER)((PBYTE) pGlobal->font.pUFontDir +
                              pGlobal->font.pUFontDir->FontResEntry[0].offsetUniFont );

    pszFamily = (( pFont->metrics.flOptions & UNIFONTMETRICS_FULLFAMILYNAME_EXIST ) &&
                 (*(pFont->metrics.szFullFamilyname))) ?
                    pFont->metrics.szFullFamilyname    :
                    pFont->metrics.ifiMetrics.szFamilyname;
    strncpy( szFamilyName, pszFamily, FACESIZE-1 );
    usRegistry = pFont->metrics.ifiMetrics.idRegistry;
    fLicensed = ( pFont->metrics.ifiMetrics.flType & IFIMETRICS32_LICENSED ) ? TRUE : FALSE;

    for ( i = 0; i < pGlobal->font.pUFontDir->ulUniFontResources; i++ ) {
        pFont = (PUNIFONTHEADER)((PBYTE) pGlobal->font.pUFontDir +
                                  pGlobal->font.pUFontDir->FontResEntry[i].offsetUniFont );
        pszFamily = (( pFont->metrics.flOptions & UNIFONTMETRICS_FULLFAMILYNAME_EXIST ) &&
                     (*(pFont->metrics.szFullFamilyname))) ?
                        pFont->metrics.szFullFamilyname    :
                        pFont->metrics.ifiMetrics.szFamilyname;
        pszFace = (( pFont->metrics.flOptions & UNIFONTMETRICS_FULLFACENAME_EXIST ) &&
                     (*(pFont->metrics.szFullFacename))) ?
                        pFont->metrics.szFullFacename    :
                        pFont->metrics.ifiMetrics.szFacename;
        if ( strcmp( szFamilyName, pszFamily ) != 0 ) continue;

        // Add this face to the container
    }

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
    WinSetDlgItemText( hwnd, IDD_GROUPBOX, "Included font faces");
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
    // (second column: resolution)
    pFld->cb = sizeof( FIELDINFO );
    pFld->pTitleData = "Resolution";
    pFld->flData     = CFA_STRING | CFA_FIREADONLY | CFA_VCENTER | CFA_HORZSEPARATOR | CFA_SEPARATOR;
    pFld->offStruct  = FIELDOFFSET( CFRECORD, pszFlags );
    pFld = pFld->pNextFieldInfo;
    // (third column: included glyphs)
    pFld->cb = sizeof( FIELDINFO );
    pFld->pTitleData = "Number of Glyphs";
    pFld->flData     = CFA_STRING | CFA_FIREADONLY | CFA_VCENTER | CFA_HORZSEPARATOR;
    pFld->offStruct  = FIELDOFFSET( CFRECORD, pszRanges );

    finsert.cb                   = (ULONG) sizeof( FIELDINFOINSERT );
    finsert.pFieldInfoOrder      = (PFIELDINFO) CMA_END;
    finsert.fInvalidateFieldInfo = TRUE;
    finsert.cFieldInfoInsert     = 3;
    WinSendMsg( hwndCnr, CM_INSERTDETAILFIELDINFO,
                MPFROMP( pFld1st ), MPFROMP( &finsert ));

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

