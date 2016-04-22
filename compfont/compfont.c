/*****************************************************************************
 * compfont.c                                                                *
 *                                                                           *
 *  OS/2-GPI Composite Font Editor/Inspector                                 *
 *  Copyright (C) 2016 Alexander Taylor                                      *
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


/*****************************************************************************
 ** IMPLEMENTATION                                                          **
 *****************************************************************************/

int main( int argc, char *argv[] )
{
    CFEGLOBAL global;
    HACCEL    hAccel;
    QMSG      qmsg;
    CHAR      szError[ SZERR_MAXZ ]  = {0};
    PSZ       pszOpen  = NULL;
    BOOL      bInitErr = FALSE;


    // Parse command-line
    if ( argc > 1 ) {
        pszOpen = argv[ 1 ];
    }

    // Initialize the PM stuff
    memset( &global, 0, sizeof( CFEGLOBAL ));
    global.cb  = sizeof( global );
    global.hab = WinInitialize( 0 );
    if ( global.hab == NULLHANDLE ) {
        sprintf( szError, "WinInitialize() failed.");
        bInitErr = TRUE;
    }
    if ( !bInitErr && ( global.hmq = WinCreateMsgQueue( global.hab, 0 )
                      ) == NULLHANDLE )
    {
        sprintf( szError, "Unable to create message queue: PM error code = 0x%X\n",
                 ERRORIDERROR( WinGetLastError( global.hab )));
        bInitErr = TRUE;
    }
    global.usType = FONT_TYPE_CMB;

    // Load the main window dialog
    if ( !bInitErr && ( global.hwndMain = WinLoadDlg( HWND_DESKTOP, HWND_DESKTOP,
                                                      (PFNWP) MainDialogProc,
                                                      NULLHANDLE, ID_MAIN, &global )
                      ) == NULLHANDLE )
    {
        sprintf( szError, "Failed to load main window dialog resource: PM error code = 0x%X\n",
                 ERRORIDERROR( WinGetLastError( global.hab )));
        bInitErr = TRUE;
    }

    // If an error occurred, show the predetermined message and exit
    if ( bInitErr )
    {
        WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, szError,
                       "Initialization Error", 0, MB_MOVEABLE | MB_OK | MB_ERROR );
    }
    else {
        // initialize keyboard shortcuts
        hAccel = WinLoadAccelTable( global.hab, NULLHANDLE, ID_MAIN );
        WinSetAccelTable( global.hab, hAccel, global.hwndMain );

        // save a pointer to the global data
        WinSetWindowPtr( global.hwndMain, 0, &global );

        if ( pszOpen && ReadFontFile( global.hwndMain, pszOpen, &global )) {
            switch ( global.usType ) {
                case FONT_TYPE_CMB:
                    PopulateValues_CMB( global.hwndMain, &global );
                    break;
            }
        }

        // main program loop
        while ( WinGetMsg( global.hab, &qmsg, 0, 0, 0 ))
            WinDispatchMsg( global.hab, &qmsg );
    }

    WinDestroyWindow( global.hwndMain );
    WinDestroyMsgQueue( global.hmq );
    WinTerminate( global.hab );

    return 0;
}


/* ------------------------------------------------------------------------- *
 * MainDialogProc()                                                          *
 *                                                                           *
 * Event handler for the main program window.                                *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY MainDialogProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    PCFEGLOBAL pGlobal;
    FILEDLG    fd;
    HWND       hwndMenu,
               hwndFD;
    MENUITEM   mi;
    HPOINTER   hIcon;
    PSWP       pswp;
    PCFRECORD  pRec;
    CFPROPS    cfprops;
    SHORT      sIdx;
    USHORT     i,
               usCount,
               usClass;
    ULONG      flStyle;
    PSZ        *ppszNames;
    PNOTIFYRECORDEMPHASIS pnemp;


    if ( msg != WM_INITDLG ) pGlobal = WinQueryWindowPtr( hwnd, 0 );

    switch( msg ) {

        case WM_INITDLG:
            pGlobal = (PCFEGLOBAL) mp2;
            hIcon = WinLoadPointer( HWND_DESKTOP, NULLHANDLE, ID_MAIN );
            WinSendMsg( hwnd, WM_SETICON, (MPARAM) hIcon, NULL );

            hwndMenu = WinLoadMenu( hwnd, NULLHANDLE, ID_MAIN );
            WinSendMsg( hwnd, WM_UPDATEFRAME, (MPARAM) FCF_MENU, NULL );
            if ( (BOOL) WinSendMsg( hwndMenu, MM_QUERYITEM,
                                    MPFROM2SHORT( IDM_NEW, TRUE ),
                                    MPFROMP( &mi )))
            {
                flStyle = WinQueryWindowULong( mi.hwndSubMenu, QWL_STYLE );
                WinSetWindowULong( mi.hwndSubMenu, QWL_STYLE,
                                   flStyle | MS_CONDITIONALCASCADE );
                WinSendMsg( mi.hwndSubMenu, MM_SETDEFAULTITEMID,
                            MPFROMLONG( ID_NEWCMB ), MPVOID );
            }

            WinSendDlgItemMsg( hwnd, IDD_FAMILYNAME, EM_SETTEXTLIMIT,
                               MPFROMSHORT( 255 ), MPVOID );
            WinSendDlgItemMsg( hwnd, IDD_FACENAME, EM_SETTEXTLIMIT,
                               MPFROMSHORT( 255 ), MPVOID );
            WinSendDlgItemMsg( hwnd, IDD_REGISTRY, SPBM_SETLIMITS,
                               MPFROMLONG( 0x7FFFFFFF ), 0 );

            for ( i = 0; i < 11; i++ ) {
                WinSendDlgItemMsg( hwnd, IDD_FONTCLASS, LM_INSERTITEM,
                                   MPFROMSHORT( i ),
                                   MPFROMP( g_FamilyClasses[ i ] ));
                sscanf( g_FamilyClasses[ i ], "%u -", &usClass );
                WinSendDlgItemMsg( hwnd, IDD_FONTCLASS, LM_SETITEMHANDLE,
                                   MPFROMSHORT( i ),
                                   MPFROMLONG( (ULONG) usClass ));
            }
            WinSendDlgItemMsg( hwnd, IDD_FONTCLASS, LM_SELECTITEM,
                               MPFROMSHORT( 0 ), MPFROMSHORT( TRUE ));

            SetupCnrCF( hwnd );
            EmboldenWindowText( WinWindowFromID( hwnd, IDD_STATUS ), NULLHANDLE );

            CentreWindow( hwnd, NULLHANDLE );
            WinShowWindow( hwnd, TRUE );

            return (MRESULT) FALSE;
        // WM_INITDLG


        case WM_CLOSE:
            // TODO verify quit if changes are outstanding
            WinPostMsg( hwnd, WM_QUIT, 0, 0 );
            return (MRESULT) 0;
        // WM_CLOSE


        case WM_COMMAND:
            switch( SHORT1FROMMP( mp1 )) {

                case ID_NEWCMB:                 // "New -> Combined font or alias file"
                    // TODO see if unsaved changes are pending
                    CloseFontFile( hwnd, pGlobal );
                    if ( pGlobal->usType != FONT_TYPE_CMB ) {
                        WinSendDlgItemMsg( hwnd, IDD_COMPONENTS,
                                           CM_REMOVEDETAILFIELDINFO, MPVOID,
                                           MPFROM2SHORT( 0, CMA_INVALIDATE | CMA_FREE ));
                        pGlobal->usType = FONT_TYPE_CMB;
                        WinShowWindow( WinWindowFromID( hwnd, IDD_FACEGROUP ), TRUE );
                        WinShowWindow( WinWindowFromID( hwnd, IDD_FACETEXT ),  TRUE );
                        WinShowWindow( WinWindowFromID( hwnd, IDD_FACENAME ),  TRUE );
                        WinShowWindow( WinWindowFromID( hwnd, ID_METRICS ),    TRUE );
                        SetupCnrCF( hwnd );
                    }
                    return (MRESULT) 0;

                case ID_NEWUNI:                 // "New -> Uni-font file"
                    // TODO see if unsaved changes are pending
                    CloseFontFile( hwnd, pGlobal );
                    if ( pGlobal->usType != FONT_TYPE_UNI ) {
                        WinSendDlgItemMsg( hwnd, IDD_COMPONENTS,
                                           CM_REMOVEDETAILFIELDINFO, MPVOID,
                                           MPFROM2SHORT( 0, CMA_INVALIDATE | CMA_FREE ));
                        pGlobal->usType = FONT_TYPE_UNI;
                        WinShowWindow( WinWindowFromID( hwnd, IDD_FACEGROUP ), FALSE );
                        WinShowWindow( WinWindowFromID( hwnd, IDD_FACETEXT ),  FALSE );
                        WinShowWindow( WinWindowFromID( hwnd, IDD_FACENAME ),  FALSE );
                        WinShowWindow( WinWindowFromID( hwnd, ID_METRICS ),    FALSE );
                        SetupCnrUF( hwnd );
                    }
                    return (MRESULT) 0;

                case ID_OPEN:                  // "Open"
                    memset( &fd, 0, sizeof( FILEDLG ));
                    fd.cbSize = sizeof( FILEDLG );
                    fd.fl = FDS_CENTER | FDS_OPEN_DIALOG;
                    fd.pszTitle = NULL;
                    sprintf( fd.szFullFile, "*.CMB");
                    hwndFD = WinFileDlg( HWND_DESKTOP, hwnd, &fd );
                    if ( hwndFD && fd.lReturn == DID_OK &&
                         ReadFontFile( hwnd, fd.szFullFile, pGlobal ))
                    {
                        switch ( pGlobal->usType ) {
                            case FONT_TYPE_CMB:
                                WinShowWindow( WinWindowFromID( hwnd, IDD_FACEGROUP ), TRUE );
                                WinShowWindow( WinWindowFromID( hwnd, IDD_FACETEXT ),  TRUE );
                                WinShowWindow( WinWindowFromID( hwnd, IDD_FACENAME ),  TRUE );
                                WinShowWindow( WinWindowFromID( hwnd, ID_METRICS ),    TRUE );
                                SetupCnrCF( hwnd );
                                PopulateValues_CMB( hwnd, pGlobal );
                                break;
                            case FONT_TYPE_UNI:
                                WinShowWindow( WinWindowFromID( hwnd, IDD_FACEGROUP ), FALSE );
                                WinShowWindow( WinWindowFromID( hwnd, IDD_FACETEXT ),  FALSE );
                                WinShowWindow( WinWindowFromID( hwnd, IDD_FACENAME ),  FALSE );
                                WinShowWindow( WinWindowFromID( hwnd, ID_METRICS ),    FALSE );
                                SetupCnrUF( hwnd );
                                break;
                        }
                    }
                    return (MRESULT) 0;

                case ID_EXIT:                   // "Exit"
                    WinPostMsg( hwnd, WM_CLOSE, 0, 0 );
                    return (MRESULT) 0;

                case ID_ADD:                 // "Add"
                    switch ( pGlobal->usType ) {
                        case FONT_TYPE_UNI:
                            AddUniFont( hwnd, pGlobal );
                            break;

                        case FONT_TYPE_CMB:
                            AddComponentFont( hwnd, pGlobal );
                            break;

                        default: ErrorPopup("Unsupported type");
                            break;
                    }
                    return (MRESULT) 0;

                case ID_CHANGE:                 // "Edit"
                    pRec = (PCFRECORD) WinSendDlgItemMsg( hwnd, IDD_COMPONENTS,
                                                          CM_QUERYRECORDEMPHASIS,
                                                          MPFROMP( (PRECORDCORE)CMA_FIRST ),
                                                          MPFROMSHORT( CRA_SELECTED ));
                    if ( pRec && pGlobal->font.combined.pComponents && pGlobal->font.combined.pComponents->ulCmpFonts ) {
#if 1
                        cfprops.cb = sizeof( CFPROPS );
                        cfprops.hab = pGlobal->hab;
                        cfprops.hmq = pGlobal->hmq;
                        cfprops.hwndMain = hwnd;
                        cfprops.fEditExisting = TRUE;
                        cfprops.pCFA = &(pGlobal->font.combined.pComponents->CompFont[ pRec->ulIndex ].CompFontAssoc);
                        WinDlgBox( HWND_DESKTOP, hwnd, (PFNWP) CompFontDlgProc,
                                   NULLHANDLE, IDD_COMPFONT, &cfprops );
#else
                        AddComponentFont( hwnd, pGlobal, pRec->ulIndex );
#endif
                    }
                    return (MRESULT) 0;

                case ID_HELP:                   // "General help"
                    return (MRESULT) 0;

                case ID_ABOUT:                  // "Product information"
                    WinDlgBox( HWND_DESKTOP, hwnd, (PFNWP) ProductInfoDlgProc,
                               NULLHANDLE, IDD_ABOUT, pGlobal );
                    return (MRESULT) 0;

                default: break;
            }
            return (MRESULT) 0;
        // WM_COMMAND


        case WM_CONTROL:
            switch( SHORT1FROMMP( mp1 )) {

                case IDD_COMPONENTS:
                    switch( SHORT2FROMMP( mp1 )) {

                        case CN_EMPHASIS:
                            pnemp = (PNOTIFYRECORDEMPHASIS) mp2;
                            if (( pnemp->fEmphasisMask & CRA_SELECTED ) &&
                                ( pnemp->pRecord->flRecordAttr & CRA_SELECTED ))
                            {
                                WinEnableControl( hwnd, ID_CHANGE, TRUE );
                                WinEnableControl( hwnd, ID_REMOVE, TRUE );
                            }
                            break;

                    }
                    break;

                case IDD_FONTCLASS:
                    switch( SHORT2FROMMP( mp1 )) {
                        case CBN_EFCHANGE:
                            sIdx = (SHORT) WinSendDlgItemMsg( hwnd, IDD_FONTCLASS,
                                                              LM_QUERYSELECTION,
                                                              MPFROMSHORT( LIT_CURSOR ), 0 );
                            usClass = (USHORT) WinSendDlgItemMsg( hwnd, IDD_FONTCLASS,
                                                                  LM_QUERYITEMHANDLE,
                                                                  MPFROMSHORT( sIdx ), 0 );
                            switch ( usClass ) {
                                default:
                                case 0:  ppszNames = g_FamilySubclasses0;  usCount = 1;  break;
                                case 1:  ppszNames = g_FamilySubclasses1;  usCount = 10; break;
                                case 2:  ppszNames = g_FamilySubclasses2;  usCount = 4;  break;
                                case 3:  ppszNames = g_FamilySubclasses3;  usCount = 4;  break;
                                case 4:  ppszNames = g_FamilySubclasses4;  usCount = 9;  break;
                                case 5:  ppszNames = g_FamilySubclasses5;  usCount = 7;  break;
                                case 7:  ppszNames = g_FamilySubclasses7;  usCount = 3;  break;
                                case 8:  ppszNames = g_FamilySubclasses8;  usCount = 10; break;
                                case 9:  ppszNames = g_FamilySubclasses9;  usCount = 6;  break;
                                case 10: ppszNames = g_FamilySubclasses10; usCount = 10; break;
                                case 12: ppszNames = g_FamilySubclasses12; usCount = 5;  break;
                            }
                            WinSendDlgItemMsg( hwnd, IDD_FONTSUBCLASS, LM_DELETEALL, 0, 0 );
                            for ( i = 0; i < usCount; i++ ) {
                                WinSendDlgItemMsg( hwnd, IDD_FONTSUBCLASS, LM_INSERTITEM,
                                                   MPFROMSHORT( i ),
                                                   MPFROMP( ppszNames[ i ] ));
                            }
                            WinSendDlgItemMsg( hwnd, IDD_FONTSUBCLASS, LM_SELECTITEM,
                                               MPFROMSHORT( 0 ), MPFROMSHORT( TRUE ));
                            break;
                    }
                    break;

                default: break;
            }
            return (MRESULT) 0;
        // WM_CONTROL


        case WM_MINMAXFRAME:
            pswp = (PSWP) mp1;
            if ( pswp->fl & SWP_MINIMIZE ) {
                WinShowWindow( WinWindowFromID( hwnd, IDD_GROUPBOX ),   FALSE );
                WinShowWindow( WinWindowFromID( hwnd, ID_ADD ),         FALSE );
            }
            else {
                WinShowWindow( WinWindowFromID( hwnd, IDD_GROUPBOX ),   TRUE );
                WinShowWindow( WinWindowFromID( hwnd, ID_ADD ),         TRUE );
            }
            return (MRESULT) FALSE;


        case WM_PRESPARAMCHANGED:
            if ( (ULONG) mp1 == PP_FONTNAMESIZE )
                EmboldenWindowText( WinWindowFromID( hwnd, IDD_STATUS ), hwnd );
            break;


        case WM_DESTROY:
            CloseFontFile( hwnd, pGlobal );
            WinSendDlgItemMsg( hwnd, IDD_COMPONENTS, CM_REMOVERECORD, NULL,
                               MPFROM2SHORT( 0, CMA_INVALIDATE | CMA_FREE ));
            WinSendDlgItemMsg( hwnd, IDD_COMPONENTS, CM_REMOVEDETAILFIELDINFO, NULL,
                               MPFROM2SHORT( 0, CMA_INVALIDATE | CMA_FREE ));
            break;
        // WM_DESTROY


        default: break;
    }

    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * AddComponentFont                                                          *
 *                                                                           *
 * Brings up the dialog to add a new component font to a combined-font file. *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd         : handle of the main program dialog.                  *
 *   PCFEGLOBAL pGlobal: pointer to global program data.                     *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void AddComponentFont( HWND hwnd, PCFEGLOBAL pGlobal )
{
    CFPROPS         cfprops;
    FONTASSOCIATION ftas = {0};

    cfprops.cb = sizeof( CFPROPS );
    cfprops.hab = pGlobal->hab;
    cfprops.hmq = pGlobal->hmq;
    cfprops.hwndMain = hwnd;
    cfprops.fEditExisting = FALSE;
    cfprops.pCFA = &ftas;
    WinDlgBox( HWND_DESKTOP, hwnd, (PFNWP) CompFontDlgProc,
               NULLHANDLE, IDD_COMPFONT, &cfprops );
    // ..etc
}


/* ------------------------------------------------------------------------- *
 * AddUniFont                                                                *
 *                                                                           *
 * Brings up the dialog to add a new component font to a combined-font file. *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd         : handle of the main program dialog.                  *
 *   PCFEGLOBAL pGlobal: pointer to global program data.                     *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void AddUniFont( HWND hwnd, PCFEGLOBAL pGlobal )
{
    UFPROPS       ufprops;
    UNIFONTHEADER font = {0};

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
 * CentreWindow                                                              *
 *                                                                           *
 * Centres the given window on the screen, or relative to another window.    *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd   : handle of the window to be centred.                       *
 *   HWND hwndRel: window that hwnd will be centred relative to.             *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void CentreWindow( HWND hwnd, HWND hwndRel )
{
    LONG scr_width,
         scr_height,
         x, y;
    SWP  wp, wp2;

    scr_width = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
    scr_height = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );

    if ( WinQueryWindowPos( hwnd, &wp )) {
        if ( hwndRel && WinQueryWindowPos( hwndRel, &wp2 )) {
            x = wp2.x + (( wp2.cx - wp.cx ) / 2 );
            y = wp2.y + (( wp2.cy - wp.cy ) / 2 );
        }
        else {
            x = ( scr_width - wp.cx ) / 2;
            y = ( scr_height - wp.cy ) / 2;
        }
        WinSetWindowPos( hwnd, HWND_TOP, x, y, wp.cx, wp.cy, SWP_MOVE | SWP_ACTIVATE );
    }

}


/* ------------------------------------------------------------------------- *
 * CloseFontFile                                                             *
 *                                                                           *
 * Closes the current font file and frees any associated data.               *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND       hwnd   : handle of the main program dialog.                  *
 *   PCFEGLOBAL pGlobal: pointer to global program data.                     *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void CloseFontFile( HWND hwnd, PCFEGLOBAL pGlobal )
{
    PCFRECORD pRec;

    // Clear the UI
    pRec = (PCFRECORD) WinSendDlgItemMsg( hwnd, IDD_COMPONENTS,
                                          CM_QUERYRECORD, NULL,
                                          MPFROM2SHORT( CMA_FIRST, CMA_ITEMORDER ));
    while ( pRec ) {
        if ( pRec->pszFace )      free( pRec->pszFace );
        if ( pRec->pszRanges )    free( pRec->pszRanges );
        if ( pRec->pszGlyphList ) free( pRec->pszGlyphList );
        if ( pRec->pszFlags )     free( pRec->pszFlags );
        pRec = (PCFRECORD) pRec->record.preccNextRecord;
    }
    WinSendDlgItemMsg( hwnd, IDD_COMPONENTS, CM_REMOVERECORD,
                       NULL, MPFROM2SHORT( 0, CMA_INVALIDATE | CMA_FREE ));
    WinEnableControl( hwnd, ID_CHANGE, FALSE );
    WinEnableControl( hwnd, ID_REMOVE, FALSE );
    WinSetDlgItemText( hwnd, IDD_FAMILYNAME, "");
    WinSetDlgItemText( hwnd, IDD_FACENAME, "");
    WinSendDlgItemMsg( hwnd, IDD_REGISTRY, SPBM_SETCURRENTVALUE,
                       MPFROMLONG( 0 ), MPVOID );
    WinCheckButton( hwnd, IDD_LICENCED, FALSE );
    WinSendDlgItemMsg( hwnd, IDD_FONTCLASS, LM_SELECTITEM,
                       MPFROMSHORT( 0 ), MPFROMSHORT( TRUE ));

    // Free the global data for the current file
    switch ( pGlobal->usType ) {
        case FONT_TYPE_CMB:
            if ( pGlobal->font.combined.pSignature )
                DosFreeMem( pGlobal->font.combined.pSignature );
            if ( pGlobal->font.combined.pMetrics )
                DosFreeMem( pGlobal->font.combined.pMetrics );
            if ( pGlobal->font.combined.pComponents )
                DosFreeMem( pGlobal->font.combined.pComponents );
            if ( pGlobal->font.combined.pEnd )
                DosFreeMem( pGlobal->font.combined.pEnd );
            pGlobal->font.combined.pSignature  = NULL;
            pGlobal->font.combined.pMetrics    = NULL;
            pGlobal->font.combined.pComponents = NULL;
            pGlobal->font.combined.pEnd        = NULL;
            break;

        case FONT_TYPE_UNI:
            if ( pGlobal->font.pUFontDir )
                free( pGlobal->font.pUFontDir );
            pGlobal->font.pUFontDir = NULL;
            break;

        case FONT_TYPE_PCR:
        case FONT_TYPE_ABR:
        case FONT_TYPE_UFF:
        default:
             break;
    }

    if ( pGlobal->hFile )
        DosClose( pGlobal->hFile );
    pGlobal->szCurrentFile[0] = '\0';
    pGlobal->cbFile = 0;
}


/* ------------------------------------------------------------------------- *
 * CompFontDlgProc                                                           *
 *                                                                           *
 * Dialog procedure for the component font properties dialog.                *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY CompFontDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    static PCFPROPS       pProps;
    HWND                  hwndCnr;
    HPS                   hps;
    CNRINFO               cnr;
    PFIELDINFO            pFld,
                          pFld1st;
    PNOTIFYRECORDEMPHASIS pnemp;
    FONTMETRICS           fm;
    FIELDINFOINSERT       fi;
    PCMRECORD             pRec;
    LONG                  lQuery;
    SHORT                 sIdx;
    ULONG                 i;
    CHAR                  szText[ SZRANGES_MAXZ ],
                          szFont[ FACESIZE ];


    switch ( msg ) {

        case WM_INITDLG:
            pProps = (PCFPROPS) mp2;

            // Set up the container
            hwndCnr = WinWindowFromID( hwnd, IDD_COMPMETRICS );
            memset( &cnr, 0, sizeof( CNRINFO ));
            cnr.flWindowAttr  = CV_DETAIL;
            cnr.pszCnrTitle   = "Metric Flags";
            cnr.cyLineSpacing = 1;
            WinSendMsg( hwndCnr, CM_SETCNRINFO, MPFROMP( &cnr ),
                        MPFROMLONG( CMA_FLWINDOWATTR | CMA_LINESPACING ));
            pFld = (PFIELDINFO) WinSendMsg( hwndCnr,
                                            CM_ALLOCDETAILFIELDINFO,
                                            MPFROMLONG( 2L ), 0 );
            pFld1st = pFld;
            // (first column: font metric item name)
            pFld->cb = sizeof( FIELDINFO );
            pFld->pTitleData = "Metric Name";
            pFld->flData     = CFA_STRING | CFA_FIREADONLY | CFA_VCENTER | CFA_HORZSEPARATOR | CFA_SEPARATOR;
            pFld->offStruct  = FIELDOFFSET( CMRECORD, pszMetric );
            pFld = pFld->pNextFieldInfo;
            // (second column: font information item value)
            pFld->cb = sizeof( FIELDINFO );
            pFld->pTitleData = "Match Setting";
            pFld->flData     = CFA_STRING | CFA_FIREADONLY | CFA_VCENTER | CFA_HORZSEPARATOR;
            pFld->offStruct  = FIELDOFFSET( CMRECORD, pszFlag );

            fi.cb                   = (ULONG) sizeof( FIELDINFOINSERT );
            fi.pFieldInfoOrder      = (PFIELDINFO) CMA_END;
            fi.fInvalidateFieldInfo = TRUE;
            fi.cFieldInfoInsert     = 2;
            WinSendMsg( hwndCnr, CM_INSERTDETAILFIELDINFO,
                        MPFROMP( pFld1st ), MPFROMP( &fi ));
            PopulateMetricFlags( hwndCnr, pProps );

            // Set up the scaling drop-down
            WinSendDlgItemMsg( hwnd, IDD_SCALING, LM_INSERTITEM,
                               MPFROMSHORT( LIT_END ), MPFROMP("Horizontal"));
            WinSendDlgItemMsg( hwnd, IDD_SCALING, LM_INSERTITEM,
                               MPFROMSHORT( LIT_END ), MPFROMP("Vertical"));
            WinSendDlgItemMsg( hwnd, IDD_SCALING, LM_INSERTITEM,
                               MPFROMSHORT( LIT_END ), MPFROMP("Horizontal & vertical"));
            WinSendDlgItemMsg( hwnd, IDD_SCALING, LM_INSERTITEM,
                               MPFROMSHORT( LIT_END ), MPFROMP("None"));
            WinSendDlgItemMsg( hwnd, IDD_SCALING, LM_INSERTITEM,
                               MPFROMSHORT( LIT_END ), MPFROMP("Undefined"));

            // Set up the metric-flags drop-down
            WinSendDlgItemMsg( hwnd, IDD_MMATCH, LM_INSERTITEM,
                               MPFROMSHORT( LIT_END ), MPFROMP("Don't care"));
            WinSendDlgItemMsg( hwnd, IDD_MMATCH, LM_INSERTITEM,
                               MPFROMSHORT( LIT_END ), MPFROMP("Must match"));
            WinSendDlgItemMsg( hwnd, IDD_MMATCH, LM_INSERTITEM,
                               MPFROMSHORT( LIT_END ), MPFROMP("Use parent information"));
            WinSendDlgItemMsg( hwnd, IDD_MMATCH, LM_INSERTITEM,
                               MPFROMSHORT( LIT_END ), MPFROMP("Undefined"));
            pRec = WinSendDlgItemMsg( hwnd, IDD_COMPMETRICS, CM_QUERYRECORDEMPHASIS,
                                      MPFROMP( CMA_FIRST ), MPFROMSHORT( CRA_SELECTED ));
            switch ( pRec->fb ) {
                case ASSOC_DONT_CARE:   sIdx = 0; break;
                case ASSOC_EXACT_MATCH: sIdx = 1; break;
                case ASSOC_USE_PARENT:  sIdx = 2; break;
                case 0:                 sIdx = 3; break;
                default:                sIdx = 4; break;
            }
            if ( sIdx <= 3 ) {
                WinSendDlgItemMsg( hwnd, IDD_MMATCH, LM_SELECTITEM,
                                   MPFROMSHORT( sIdx ), MPFROMSHORT( TRUE ));
            }
            else {
                WinSetDlgItemText( hwnd, IDD_MMATCH, pRec->pszFlag );
            }
            WinEnableControl( hwnd, IDD_MMAPPLY, FALSE );

            sIdx = pProps->pCFA->flFlags ? ((SHORT) pProps->pCFA->flFlags - 1) : 4;
            if ( sIdx <= 4 ) {
                WinSendDlgItemMsg( hwnd, IDD_SCALING, LM_SELECTITEM,
                                   MPFROMSHORT( sIdx ), MPFROMSHORT( TRUE ));
            }
            else {
                sprintf( szText, "0x%08x", pProps->pCFA->flFlags );
                WinSetDlgItemText( hwnd, IDD_SCALING, szText );
            }

            // Populate the current font's values
            if ( pProps->fEditExisting ) {
                for ( i = 0; i < pProps->pCFA->ulGlyphRanges; i++ ) {
                    sprintf( szText, SZ_GLYPHRANGE,
                             pProps->pCFA->GlyphRange[ i ].giStart,
                             pProps->pCFA->GlyphRange[ i ].giEnd,
                             pProps->pCFA->GlyphRange[ i ].giTarget );
                    WinSendDlgItemMsg( hwnd, IDD_RANGES, LM_INSERTITEM,
                                       MPFROMSHORT( LIT_END ),
                                       MPFROMP( szText ));
                }
                WinSetDlgItemText( hwnd, IDD_FONTNAME,
                                   ( pProps->pCFA->unifm.flOptions & UNIFONTMETRICS_FULLFACENAME_EXIST ) ?
                                     pProps->pCFA->unifm.szFullFacename :
                                     pProps->pCFA->unifm.ifiMetrics.szFacename );
            }
            CentreWindow( hwnd, pProps->hwndMain );
            WinShowWindow( hwnd, TRUE );
            break;
        // WM_INITDLG


        case WM_COMMAND:
            switch( SHORT1FROMMP( mp1 )) {

                case IDD_SELECTFONT:
                    if ( SelectInstalledFont( hwnd, szFont ))
                        WinSetDlgItemText( hwnd, IDD_FONTNAME, szFont );
                    return (MRESULT) 0;


                case IDD_ADDRANGE:
                    GlyphRangeDialog( hwnd, pProps, FALSE );
                    return (MRESULT) 0;


                case IDD_EDITRANGE:
                    GlyphRangeDialog( hwnd, pProps, TRUE );
                    return (MRESULT) 0;


                case IDD_DELRANGE:
                    sIdx = (SHORT) WinSendDlgItemMsg( hwnd, IDD_RANGES,
                                                      LM_QUERYSELECTION,
                                                      MPFROMSHORT( LIT_CURSOR ),
                                                      MPVOID );
                    if ( sIdx != LIT_NONE )
                        WinSendDlgItemMsg( hwnd, IDD_RANGES, LM_DELETEITEM,
                                           MPFROMSHORT( sIdx ), MPVOID );
                    return (MRESULT) 0;


                case IDD_MMAPPLY:
                    pRec = WinSendDlgItemMsg( hwnd, IDD_COMPMETRICS,
                                              CM_QUERYRECORDEMPHASIS,
                                              MPFROMP( CMA_FIRST ),
                                              MPFROMSHORT( CRA_SELECTED ));
                    sIdx = (SHORT) WinSendDlgItemMsg( hwnd, IDD_MMATCH,
                                                      LM_QUERYSELECTION,
                                                      MPFROMSHORT( LIT_CURSOR ),
                                                      MPVOID );
                    switch ( sIdx ) {
                        case 0 :
                            pRec->fb = ASSOC_DONT_CARE;
                            strcpy( pRec->pszFlag, "Don't care");
                            break;
                        case 1 :
                            pRec->fb = ASSOC_EXACT_MATCH;
                            strcpy( pRec->pszFlag, "Must match");
                            break;
                        case 2 :
                            pRec->fb = ASSOC_USE_PARENT;
                            strcpy( pRec->pszFlag, "Use parent information");
                            break;
                        default:
                            strcpy( pRec->pszFlag, "");
                            pRec->fb = 0;
                            break;
                    }
                    WinSendDlgItemMsg( hwnd, IDD_COMPMETRICS,
                                       CM_INVALIDATEDETAILFIELDINFO, MPVOID, MPVOID );
                    WinEnableControl( hwnd, IDD_MMAPPLY, FALSE );
                    return (MRESULT) 0;


                case DID_OK:
                    if ( ! WinQueryDlgItemText( hwnd, IDD_FONTNAME, FACESIZE, szFont ))
                        return (MRESULT) 0;
                    if ( !pProps->fEditExisting ) {
                        pProps->pCFA->Identity = SIG_FTAS;
                        pProps->pCFA->ulSize   = sizeof( FONTASSOCIATION ); // (?)
                    }
                    // Get the metrics of the indicated font face
                    hps = WinGetScreenPS( HWND_DESKTOP );
                    lQuery = 1;
                    GpiQueryFonts( hps, QF_PUBLIC, szFont, &lQuery, sizeof(fm), &fm );
                    WinReleasePS( hps );
                    DeriveUniFontMetrics( &(pProps->pCFA->unifm), &fm );
                    // get the metric flags (save to pProps->pCFA->unimbr)
                    // get the ranges
                    //      check for discrepencies with the metrics
                    //      set pProps->pCFA->ulGlyphRanges
                    //      if fEditExisting && old range count != new range count
                    //          reallocate pProps->pCFA->GlyphRange[]
                    //      else
                    //          allocate pProps->pCFA->GlyphRange[]
                    //      populate pProps->pCFA->GlyphRange[]
                    //
                    break;


                case DID_CANCEL:
                    break;
            }
            break;
        // WM_COMMAND


        case WM_CONTROL:
            switch( SHORT1FROMMP( mp1 )) {

                case IDD_COMPMETRICS:
                    switch( SHORT2FROMMP( mp1 )) {

                        case CN_EMPHASIS:
                            pnemp = (PNOTIFYRECORDEMPHASIS) mp2;
                            if (( pnemp->fEmphasisMask & CRA_SELECTED ) &&
                                ( pnemp->pRecord->flRecordAttr & CRA_SELECTED ))
                            {
                                pRec = (PCMRECORD) pnemp->pRecord;
                                switch ( pRec->fb ) {
                                    case ASSOC_DONT_CARE:   sIdx = 0; break;
                                    case ASSOC_EXACT_MATCH: sIdx = 1; break;
                                    case ASSOC_USE_PARENT:  sIdx = 2; break;
                                    case 0:                 sIdx = 3; break;
                                    default:                sIdx = 4; break;
                                }
                                if ( sIdx <= 3 ) {
                                    WinSendDlgItemMsg( hwnd, IDD_MMATCH, LM_SELECTITEM,
                                                       MPFROMSHORT( sIdx ), MPFROMSHORT( TRUE ));
                                }
                                else {
                                    WinSetDlgItemText( hwnd, IDD_MMATCH, pRec->pszFlag );
                                }
                                WinEnableControl( hwnd, IDD_MMAPPLY, FALSE );
                            }
                            break;

                    }
                    break;

                case IDD_MMATCH:
                    switch( SHORT2FROMMP( mp1 )) {
                        case CBN_EFCHANGE:
                            WinEnableControl( hwnd, IDD_MMAPPLY, TRUE );
                            break;
                    }
                    break;

                case IDD_RANGES:
                    switch( SHORT2FROMMP( mp1 )) {
                        case LN_SELECT:
                            WinEnableControl( hwnd, IDD_EDITRANGE, TRUE );
                            WinEnableControl( hwnd, IDD_DELRANGE,  TRUE );
                            break;
                    }
                    break;

            }
            return (MRESULT) 0;
        // WM_CONTROL


        case WM_DESTROY:
            // free the allocated container memory
            hwndCnr = WinWindowFromID( hwnd, IDD_COMPMETRICS );
            pRec = (PCMRECORD) WinSendMsg( hwndCnr, CM_QUERYRECORD, MPVOID,
                                           MPFROM2SHORT( CMA_FIRST, CMA_ITEMORDER ));
            while ( pRec ) {
                free( pRec->pszMetric );
                free( pRec->pszFlag );
                pRec = (PCMRECORD) pRec->record.preccNextRecord;
            }
            WinSendMsg( hwnd, CM_REMOVERECORD, MPVOID,
                        MPFROM2SHORT( 0, CMA_INVALIDATE | CMA_FREE ));
            WinSendMsg( hwnd, CM_REMOVEDETAILFIELDINFO, MPVOID,
                        MPFROM2SHORT( 0, CMA_INVALIDATE | CMA_FREE ));
            break;
        // WM_DESTROY


        default: break;
    }

    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * ComponentListDelete                                                       *
 *                                                                           *
 * Delete a font association from the linked list.                           *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PCFEGLOBAL pGlobal: pointer to global data.                             *
 *   ULONG      ulIndex: list index of the item to delete.                   *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void ComponentListDelete( PCFEGLOBAL pGlobal, ULONG ulIndex )
{
    PFONTASSOCLIST pNode,
                   pTemp;
    ULONG          i;

    pNode = pGlobal->font.combined.pFontList;
    if ( pNode == NULL ) return;

    for ( i = 0; pNode->pNext && ( i < ulIndex ); i++ ) pNode = pNode->pNext;
    if ( i < ulIndex ) return;

    pTemp = pNode->pNext;
    pNode->pNext = pTemp->pNext;
    DosFreeMem( pTemp );
}


/* ------------------------------------------------------------------------- *
 * ComponentListFree                                                         *
 *                                                                           *
 * Free the linked list of component font associations.                      *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PCFEGLOBAL pGlobal: pointer to global data.                             *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void ComponentListFree( PCFEGLOBAL pGlobal )
{
    PFONTASSOCLIST pNode;

    pNode = pGlobal->font.combined.pFontList;
    if ( !pNode ) return;

    while ( pNode ) {
        DosFreeMem( pNode );
        pNode = pNode->pNext;
    }
}


/* ------------------------------------------------------------------------- *
 * ComponentListInit                                                         *
 *                                                                           *
 * Copy the array of component font associations from a parsed combined font *
 * file into a linked list (stored in the global data) which is easier for   *
 * us to modify as needed.                                                   *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PCFEGLOBAL      pGlobal    : pointer to global data.                    *
 *   PCOMPFONTHEADER pComponents: original array of components being copied. *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void ComponentListInit( PCFEGLOBAL pGlobal, PCOMPFONTHEADER pComponents )
{
    PFONTASSOCIATION pFA;
    PFONTASSOCLIST   pNode,
                     pNew;
    ULONG            cbFA,
                     i;
    APIRET           rc;


    if ( !pGlobal->font.combined.pComponents->ulCmpFonts ) return;

    pFA  = &(pGlobal->font.combined.pComponents->CompFont[ 0 ].CompFontAssoc);
    cbFA = sizeof( FONTASSOCIATION ) +
           ( pFA->ulGlyphRanges * sizeof( FONTASSOCGLYPHRANGE )) -
           sizeof( FONTASSOCGLYPHRANGE );
    rc = DosAllocMem( (PPVOID) &pNode, cbFA, PAG_READ | PAG_WRITE | PAG_COMMIT );
    if ( rc != NO_ERROR ) {
        ErrorPopup("Failed to allocate memory for component font structure.");
        return;
    }
    memcpy( &(pNode->font), pFA, cbFA );
    pGlobal->font.combined.pFontList = pNode;

    for ( i = 1; i < pGlobal->font.combined.pComponents->ulCmpFonts; i++ ) {
        pFA  = &(pGlobal->font.combined.pComponents->CompFont[ i ].CompFontAssoc);
        cbFA = sizeof( FONTASSOCIATION ) +
               ( pFA->ulGlyphRanges * sizeof( FONTASSOCGLYPHRANGE )) -
               sizeof( FONTASSOCGLYPHRANGE );
        rc = DosAllocMem( (PPVOID) &pNew, cbFA, PAG_READ | PAG_WRITE | PAG_COMMIT );
        if ( rc != NO_ERROR ) {
            ErrorPopup("Failed to allocate memory for component font structure.");
            return;
        }
        pNode->pNext = pNew;
        pNew->pNext  = NULL;
        memcpy( &(pNew->font), pFA, cbFA );
        pNode = pNew;
    }
}


/* ------------------------------------------------------------------------- *
 * ComponentListInsert                                                       *
 *                                                                           *
 * Insert a new font association into the linked list.                       *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PCFEGLOBAL       pGlobal  : pointer to global data.                     *
 *   PFONTASSOCIATION pCompFont: the new font association to add.            *
 *   ULONG            ulIndex  : list index of the newly added item .        *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 * ------------------------------------------------------------------------- */
BOOL ComponentListInsert( PCFEGLOBAL pGlobal, PFONTASSOCIATION pCompFont, ULONG ulIndex )
{
    PFONTASSOCLIST  pNode,
                    pNew;
    ULONG           cb,
                    i;
    APIRET          rc;

    cb = sizeof( FONTASSOCIATION ) +
         ( pCompFont->ulGlyphRanges * sizeof( FONTASSOCGLYPHRANGE )) -
         sizeof( FONTASSOCGLYPHRANGE );
    rc = DosAllocMem( (PPVOID) &pNew, cb, PAG_READ | PAG_WRITE | PAG_COMMIT );
    if ( rc != NO_ERROR ) {
        ErrorPopup("Failed to allocate memory for component font structure.");
        return FALSE;
    }
    memcpy( &(pNew->font), pCompFont, cb );
    if ( pGlobal->font.combined.pFontList == NULL ) {
        pNew->pNext = NULL;
        pGlobal->font.combined.pFontList = pNew;
        return TRUE;
    }

    pNode = pGlobal->font.combined.pFontList;
    for ( i = 0; pNode->pNext && ( i < ulIndex ); i++ ) pNode = pNode->pNext;
    pNew->pNext = pNode->pNext;
    pNode->pNext = pNew;

    return TRUE;
}


/* ------------------------------------------------------------------------- *
 * DeriveUniFontMetrics                                                      *
 *                                                                           *
 * Populates a Uni-font metrics structure with default values derived from   *
 * the actual metrics of a font.                                             *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PUNIFONTMETRICS: Pointer to the UNIFONTMETRICS structure            (O) *
 *   PFONTMETRICS   : Pointer to the reference font metrics              (I) *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void DeriveUniFontMetrics( PUNIFONTMETRICS pUFM, PFONTMETRICS pFM )
{
    PULONG pID;
    if ( !pUFM || !pFM ) return;

    memset( pUFM, 0, sizeof( UNIFONTMETRICS ));
    pID = (PULONG) pUFM->Identity;
    *pID = SIG_UNFM;
    pUFM->ulSize = sizeof( UNIFONTMETRICS );

    strncpy( pUFM->ifiMetrics.szFamilyname, pFM->szFamilyname, FACESIZE-1 );
    strncpy( pUFM->ifiMetrics.szFacename,   pFM->szFacename, FACESIZE-1 );

    /* Unfortunately GPI doesn't seem to provide a way to query the glyphlist,
     * so we have to make an educated guess based on what we know...
     */
    if (( pFM->fsType & FM_TYPE_UNICODE ) || ( pFM->sLastChar == (SHORT) 0xFFFD ))
        sprintf( pUFM->ifiMetrics.szGlyphlistName, "UNICODE");
    else if ( pFM->usCodePage == 65400 )
        sprintf( pUFM->ifiMetrics.szGlyphlistName, "SYMBOL");
    else if (( pFM->usCodePage == 932 ) || ( pFM->usCodePage == 942 ) ||
             ( pFM->fsSelection & FM_SEL_DBCSMASK ) == FM_SEL_JAPAN )
        sprintf( pUFM->ifiMetrics.szGlyphlistName, "PMJPN");
    else if (( pFM->usCodePage == 949 ) ||
             ( pFM->fsSelection & FM_SEL_DBCSMASK ) == FM_SEL_KOREA )
        sprintf( pUFM->ifiMetrics.szGlyphlistName, "PMKOR");
    else if (( pFM->usCodePage == 950 ) ||
             ( pFM->fsSelection & FM_SEL_DBCSMASK ) == FM_SEL_TAIWAN )
        sprintf( pUFM->ifiMetrics.szGlyphlistName, "PMCHT");
    else if (( pFM->usCodePage == 1381 ) || ( pFM->usCodePage == 1386 ) ||
             ( pFM->fsSelection & FM_SEL_DBCSMASK ) == FM_SEL_CHINA )
        sprintf( pUFM->ifiMetrics.szGlyphlistName, "PMPRC");
    else
        sprintf( pUFM->ifiMetrics.szGlyphlistName, "PM383");

    pUFM->ifiMetrics.idRegistry        = (ULONG) pFM->idRegistry;
    pUFM->ifiMetrics.lCapEmHeight      = pFM->lEmHeight;
    pUFM->ifiMetrics.lXHeight          = pFM->lXHeight;
    pUFM->ifiMetrics.lMaxAscender      = pFM->lMaxAscender;
    pUFM->ifiMetrics.lMaxDescender     = pFM->lMaxDescender;
    pUFM->ifiMetrics.lLowerCaseAscent  = pFM->lLowerCaseAscent;
    pUFM->ifiMetrics.lLowerCaseDescent = pFM->lLowerCaseDescent;
    pUFM->ifiMetrics.lInternalLeading  = pFM->lInternalLeading;
    pUFM->ifiMetrics.lExternalLeading  = pFM->lExternalLeading;
    pUFM->ifiMetrics.lAveCharWidth     = pFM->lAveCharWidth;
    pUFM->ifiMetrics.lMaxCharInc       = pFM->lMaxCharInc;
    pUFM->ifiMetrics.lEmInc            = pFM->lEmInc;
    pUFM->ifiMetrics.lMaxBaselineExt   = pFM->lMaxBaselineExt;

    pUFM->ifiMetrics.fxCharSlope = MAKEFIXED( (pFM->sCharSlope & 0xFF00) >> 8,
                                              pFM->sCharSlope & 0xFF );
    pUFM->ifiMetrics.fxInlineDir = MAKEFIXED( (pFM->sInlineDir & 0xFF00) >> 8,
                                              pFM->sInlineDir & 0xFF );
    pUFM->ifiMetrics.fxCharRot   = MAKEFIXED( (pFM->sCharRot & 0xFF00) >> 8,
                                              pFM->sCharRot & 0xFF );

    pUFM->ifiMetrics.ulWeightClass      = (ULONG) pFM->usWeightClass;
    pUFM->ifiMetrics.ulWidthClass       = (ULONG) pFM->usWidthClass;
    pUFM->ifiMetrics.lEmSquareSizeX     = pFM->lEmInc;
    pUFM->ifiMetrics.lEmSquareSizeY     = pFM->lEmHeight;
    pUFM->ifiMetrics.giFirstChar        = (GLYPH) pFM->sFirstChar;
    pUFM->ifiMetrics.giLastChar         = (GLYPH) pFM->sLastChar;
    pUFM->ifiMetrics.giDefaultChar      = (GLYPH) pFM->sDefaultChar;
    pUFM->ifiMetrics.giBreakChar        = (GLYPH) pFM->sBreakChar;
    pUFM->ifiMetrics.ulNominalPointSize = (ULONG) pFM->sNominalPointSize;
    pUFM->ifiMetrics.ulMinimumPointSize = (ULONG) pFM->sMinimumPointSize;
    pUFM->ifiMetrics.ulMaximumPointSize = (ULONG) pFM->sMaximumPointSize;

    pUFM->ifiMetrics.flType = (ULONG) pFM->fsType;
    if ( pFM->fsDefn & FM_DEFN_OUTLINE )
        pUFM->ifiMetrics.flDefn |= IFIMETRICS_OUTLINE;
    if ( pFM->fsSelection & FM_SEL_ITALIC )
        pUFM->ifiMetrics.flSelection |= IFIMETRICS32_ITALIC;
    if ( pFM->fsSelection & FM_SEL_UNDERSCORE )
        pUFM->ifiMetrics.flSelection |= IFIMETRICS32_UNDERSCORE;
    if ( pFM->fsSelection & FM_SEL_NEGATIVE )
        pUFM->ifiMetrics.flSelection |= IFIMETRICS32_NEGATIVE;
    if ( pFM->fsSelection & FM_SEL_OUTLINE )
        pUFM->ifiMetrics.flSelection |= IFIMETRICS32_HOLLOW;
    if ( pFM->fsSelection & FM_SEL_STRIKEOUT )
        pUFM->ifiMetrics.flSelection |= IFIMETRICS32_OVERSTRUCK;

    pUFM->ifiMetrics.flCapabilities      = (ULONG) pFM->fsCapabilities;
    pUFM->ifiMetrics.lSubscriptXSize     = pFM->lSubscriptXSize;
    pUFM->ifiMetrics.lSubscriptYSize     = pFM->lSubscriptYSize;
    pUFM->ifiMetrics.lSubscriptXOffset   = pFM->lSubscriptXOffset;
    pUFM->ifiMetrics.lSubscriptYOffset   = pFM->lSubscriptYOffset;
    pUFM->ifiMetrics.lSuperscriptXSize   = pFM->lSuperscriptXSize;
    pUFM->ifiMetrics.lSuperscriptYSize   = pFM->lSuperscriptYSize;
    pUFM->ifiMetrics.lSuperscriptXOffset = pFM->lSuperscriptXOffset;
    pUFM->ifiMetrics.lSuperscriptYOffset = pFM->lSuperscriptYOffset;
    pUFM->ifiMetrics.lUnderscoreSize     = pFM->lUnderscoreSize;
    pUFM->ifiMetrics.lUnderscorePosition = pFM->lUnderscorePosition;
    pUFM->ifiMetrics.lStrikeoutSize      = pFM->lStrikeoutSize;
    pUFM->ifiMetrics.lStrikeoutPosition  = pFM->lStrikeoutPosition;
    pUFM->ifiMetrics.ulKerningPairs      = (ULONG) pFM->sKerningPairs;
    pUFM->ifiMetrics.ulFontClass = (( pFM->sFamilyClass & 0xFF00 ) << 16 ) |
                                    ( pFM->sFamilyClass & 0xFF );
}


/* ------------------------------------------------------------------------- *
 * EditComponentFont                                                         *
 *                                                                           *
 * Brings up the dialog to edit an existing component in a combined font.    *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND       hwnd   : handle of the main program dialog.                  *
 *   PCFEGLOBAL pGlobal: pointer to global program data.                     *
 *   ULONG      ulAssoc: index of the association being modified.            *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void EditComponentFont( HWND hwnd, PCFEGLOBAL pGlobal, ULONG ulAssoc )
{
    CFPROPS          cfprops;
    PFONTASSOCIATION pAssocCopy,    // a working copy of the current association
                     pAssocOrig;    // pointer to the current font association
    ULONG            cbFA;
    APIRET           rc;

    pAssocOrig = &(pGlobal->font.combined.pComponents->CompFont[ ulAssoc ].CompFontAssoc);

    cbFA = sizeof( FONTASSOCIATION ) +
           ( pAssocOrig->ulGlyphRanges * sizeof( FONTASSOCGLYPHRANGE )) -
           sizeof( FONTASSOCGLYPHRANGE );

    rc = DosAllocMem( (PPVOID) &pAssocCopy, cbFA, PAG_READ | PAG_WRITE | PAG_COMMIT );
    if ( rc != NO_ERROR ) {
        ErrorPopup("Memory allocation error.");
        return;
    }
    memcpy( pAssocCopy, pAssocOrig, cbFA );

    cfprops.cb = sizeof( CFPROPS );
    cfprops.hab = pGlobal->hab;
    cfprops.hmq = pGlobal->hmq;
    cfprops.hwndMain = hwnd;
    cfprops.fEditExisting = TRUE;
    cfprops.pCFA = pAssocCopy;
    WinDlgBox( HWND_DESKTOP, hwnd, (PFNWP) CompFontDlgProc,
               NULLHANDLE, IDD_COMPFONT, &cfprops );

    // ..etc
    DosFreeMem( pAssocCopy );
}


/* ------------------------------------------------------------------------- *
 * EmboldenWindowText                                                        *
 *                                                                           *
 * Sets the font presentation parameter of the specified window to the bold  *
 * equivalent of the current font.                                           *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd   : Handle of the window to update.                           *
 *   HWND hwndUse: Handle of the window whose font is to be used as a        *
 *                 reference. If NULL, the current font of hwndCtl is used.  *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if the font was updated; FALSE otherwise.                          *
 * ------------------------------------------------------------------------- */
BOOL EmboldenWindowText( HWND hwnd, HWND hwndUse )
{
    HPS         hps;
    HWND        hwndCtl;
    FONTMETRICS fm,
                *pfm;                  // array of FONTMETRICS objects
    LONG        i,                     // loop index
                cFonts = 0,            // number of fonts found
                cCount = 0;            // number of fonts to return
    BOOL        fOK    = FALSE,        // was the font updated successfully?
                fFound = FALSE;        // did we find a suitable font?
    CHAR        szFontPP[ FACESIZE+5 ] = {0};


    hwndCtl = hwndUse ? hwndUse : hwnd;

    hps = WinGetPS( hwndCtl );
    if ( ! GpiQueryFontMetrics( hps, sizeof( FONTMETRICS ), &fm ))
        goto finish;

    // If current font is already bold, no need to do anything
    if ( fm.usWeightClass > 5 ) {
        fFound = TRUE;
        goto finish;
    }

    // Query the list of installed fonts
    cFonts = GpiQueryFonts( hps, QF_PUBLIC, NULL,
                            &cCount, sizeof(FONTMETRICS), NULL );
    if ( cFonts < 1 )
        goto finish;
    if (( pfm = (PFONTMETRICS) calloc( cFonts, sizeof( FONTMETRICS ))) == NULL )
        goto finish;
    GpiQueryFonts( hps, QF_PUBLIC, NULL, &cFonts, sizeof( FONTMETRICS ), pfm );

    for ( i = 0; i < cFonts; i++ ) {
        if (( strcmp( pfm[i].szFamilyname, fm.szFamilyname ) == 0 ) &&
            ( pfm[i].usWeightClass > 5 ) && !( pfm[i].fsSelection & FM_SEL_ITALIC ))
        {
            /* We have a match on the name and weight.  But for non-outline
             * (bitmap) fonts, we have to check the explicit point size as well.
             */
            if (( pfm[i].fsDefn & FM_DEFN_OUTLINE ) ||
                ( pfm[i].sNominalPointSize == fm.sNominalPointSize ))
            {
                fFound = TRUE;
                break;
            }
        }
    }

    if ( fFound ) {
        sprintf( szFontPP, "%u.%s", ( pfm[i].sNominalPointSize / 10 ),
                                    pfm[i].szFacename, FACESIZE+4 );
        fOK = WinSetPresParam( hwnd, PP_FONTNAMESIZE,
                               strlen( szFontPP )+1, szFontPP );
    }
    else if ( hwndUse ) {
        /* No matching bold font was found, so fall back to the
         * original reference font (only necessary if it's been
         * taken from a different control).  Note that we return
         * FALSE in this case.
         */
        sprintf( szFontPP, "%u.%s", ( fm.sNominalPointSize / 10 ),
                                    fm.szFacename, FACESIZE+4 );
        WinSetPresParam( hwnd, PP_FONTNAMESIZE,
                         strlen( szFontPP )+1, szFontPP );
    }

    free( pfm );

finish:
    WinReleasePS( hps );
    return ( fOK );
}


/* ------------------------------------------------------------------------- *
 * GetCurrentDPI                                                             *
 *                                                                           *
 * Queries the current vertical font resolution (a.k.a. DPI).                *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd: Window handle of control.                                    *
 *                                                                           *
 * RETURNS: LONG                                                             *
 *   Current screen DPI setting.                                             *
 * ------------------------------------------------------------------------- */
LONG GetCurrentDPI( HWND hwnd )
{
    HDC  hdc;           // device-context handle
    LONG lCap,          // value from DevQueryCaps
         lDPI;          // returned DPI value

    hdc = WinOpenWindowDC( hwnd );
    if ( DevQueryCaps( hdc, CAPS_VERTICAL_FONT_RES, 1, &lCap ))
        lDPI = lCap;
    if ( !lDPI )
        lDPI = 96;

    return lDPI;
}


/* ------------------------------------------------------------------------- *
 * GlyphRangeDialog                                                          *
 *                                                                           *
 * Brings up the dialog to add a new glyph range to a component font.        *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND    hwnd     : handle of the main program dialog.                   *
 *   PCFDATA pCompFont: pointer to component-font dialog data.               *
 *   BOOL    fEdit    : is an existing range being edited?                   *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void GlyphRangeDialog( HWND hwnd, PCFPROPS pCompFont, BOOL fEdit )
{
    FONTMETRICS fm     = {0};
    LONG        lQuery = 1;
    SHORT       sIdx;
    GRDATA      grdata;
    CHAR        szFont[ FACESIZE ],
                szRange[ SZRANGES_MAXZ ];
    HPS         hps;

    if ( ! WinQueryDlgItemText( hwnd, IDD_FONTNAME, FACESIZE, szFont )) {
        ErrorPopup("No font is selected.");
        return;
    }

    // Get the metrics of the selected font face
    hps = WinGetScreenPS( HWND_DESKTOP );
    GpiQueryFonts( hps, QF_PUBLIC, szFont, &lQuery, sizeof(fm), &fm );
    WinReleasePS( hps );

    grdata.cb            = sizeof( GRDATA );
    grdata.hab           = pCompFont->hab;
    grdata.hmq           = pCompFont->hmq;
    grdata.hwndMain      = pCompFont->hwndMain;
    grdata.fEditExisting = fEdit;
    grdata.usFirstChar   = (USHORT) fm.sFirstChar;
    grdata.usLastChar    = (USHORT)( fm.sFirstChar + fm.sLastChar );
    if ( fm.fsType & FM_TYPE_UNICODE )
        grdata.bEncoding = ENC_UNICODE;
    else if ( fm.usCodePage == 65400 )
        grdata.bEncoding = ENC_SYMBOL;
    else grdata.bEncoding = ENC_PM;

    if ( fEdit ) {
        sIdx = (SHORT) WinSendDlgItemMsg( hwnd, IDD_RANGES, LM_QUERYSELECTION,
                                          MPFROMSHORT( LIT_CURSOR ), MPVOID );
        if ( sIdx == LIT_NONE ) return;
        if ( ! WinSendDlgItemMsg( hwnd, IDD_RANGES, LM_QUERYITEMTEXT,
                                  MPFROM2SHORT( sIdx, SZRANGES_MAXZ-1 ),
                                  MPFROMP( szRange ))) return;
        if ( sscanf( szRange, SZ_GLYPHRANGE,
                     &grdata.range.giStart, &grdata.range.giEnd,
                     &grdata.range.giTarget ) != 3 ) return;
        grdata.range.ulReserved1 = 0;
        grdata.range.ulReserved2 = 0;
    }
    else memset( &(grdata.range), 0, sizeof( FONTASSOCGLYPHRANGE ));

    if ( WinDlgBox( HWND_DESKTOP, hwnd, (PFNWP) RangeDlgProc,
                    NULLHANDLE, IDD_GLYPHRANGE, &grdata ) == DID_OK )
    {
        sprintf( szRange, SZ_GLYPHRANGE, grdata.range.giStart,
                                         grdata.range.giEnd,
                                         grdata.range.giTarget );
        if ( fEdit )
            WinSendDlgItemMsg( hwnd, IDD_RANGES, LM_SETITEMTEXT,
                               MPFROMSHORT( sIdx ), MPFROMP( szRange ));
        else
            WinSendDlgItemMsg( hwnd, IDD_RANGES, LM_INSERTITEM,
                               MPFROMSHORT( LIT_SORTASCENDING ),
                               MPFROMP( szRange ));
    }
}


/* ------------------------------------------------------------------------- *
 * ImportDlgProc                                                             *
 *                                                                           *
 * Dialog procedure for the import-glyphs dialog.                            *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY ImportDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    PCFEGLOBAL pGlobal;
    HWND       hwndFD;
    FILEDLG    fd;


    switch ( msg ) {

        case WM_INITDLG:
            pGlobal = (PCFEGLOBAL) mp2;
            WinSendDlgItemMsg( hwnd, IDD_IMPFILE, EM_SETTEXTLIMIT,
                               MPFROMSHORT( CCHMAXPATH ), MPVOID );
            WinSendDlgItemMsg( hwnd, IDD_IMPTARGET, SPBM_SETLIMITS,
                               MPFROMLONG( 65535 ), MPFROMLONG( 0 ));
            CentreWindow( hwnd, pGlobal->hwndMain );
            break;
        // WM_INITDLG


        case WM_COMMAND:
            switch( SHORT1FROMMP( mp1 )) {

                case IDD_IMPOPEN:
                    memset( &fd, 0, sizeof( FILEDLG ));
                    fd.cbSize = sizeof( FILEDLG );
                    fd.fl = FDS_CENTER | FDS_OPEN_DIALOG;
                    fd.pszTitle = NULL;
                    sprintf( fd.szFullFile, "*.FNT");
                    hwndFD = WinFileDlg( HWND_DESKTOP, hwnd, &fd );
                    if ( hwndFD && fd.lReturn == DID_OK ) {
                        WinSetDlgItemText( hwnd, IDD_IMPFILE, fd.szFullFile );
                        // TODO get the actual number of glyphs instead of 1105
                        WinSendDlgItemMsg( hwnd, IDD_IMPSTART, SPBM_SETLIMITS,
                                           MPFROMLONG( 1105 ), MPFROMLONG( 0 ));
                        WinSendDlgItemMsg( hwnd, IDD_IMPEND, SPBM_SETLIMITS,
                                           MPFROMLONG( 1105 ), MPFROMLONG( 0 ));
                    }
                    return (MRESULT) 0;

            }
            break;
        // WM_COMMAND


        default: break;
    }

    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * PopulateMetricFlags                                                       *
 *                                                                           *
 * Populate the metric flags container on the font properties dialog.        *
 * ------------------------------------------------------------------------- */
void PopulateMetricFlags( HWND hwndCnr, PCFPROPS pProps )
{
    ULONG        ulCB;
    USHORT       i;
    PCMRECORD    pRec, pFirst;
    RECORDINSERT ri;
    PBYTE        fbMetricFields;

    // Allocate the records (there are 51 metric items to show)
    ulCB = sizeof( CMRECORD ) - sizeof( MINIRECORDCORE );
    pRec = (PCMRECORD) WinSendMsg( hwndCnr, CM_ALLOCRECORD,
                                   MPFROMLONG( ulCB ),
                                   MPFROMLONG( 51L ));
    pFirst = pRec;
    ulCB = sizeof( MINIRECORDCORE );

    /* The first 48 values are sequential byte fields in the ifi32mbr
     * structure, so we can just cast the structure to an array and loop
     * through.  The corresponding description strings are defined in the
     * global array g_MetricItems[] to facilitate this.
     */
    fbMetricFields = (PBYTE)(&(pProps->pCFA->unimbr.ifi32mbr));
    for ( i = 0; i < 48; i++ ) {
        pRec->pszMetric = (PSZ) malloc( SZMETRIC_MAX );
        pRec->pszFlag   = (PSZ) malloc( SZFLAGS_MAXZ );
        pRec->fb        = fbMetricFields[ i ];
        strcpy( pRec->pszMetric, g_MetricItems[ i ] );
        switch ( fbMetricFields[ i ] ) {
            case 0:
                strcpy( pRec->pszFlag, ""); break;
            case ASSOC_EXACT_MATCH:
                strcpy( pRec->pszFlag, "Must match"); break;
            case ASSOC_DONT_CARE:
                strcpy( pRec->pszFlag, "Don't care"); break;
            case ASSOC_USE_PARENT:
                strcpy( pRec->pszFlag, "Use parent information"); break;
            default:
                sprintf( pRec->pszFlag, "0x%08X", fbMetricFields[ i ] ); break;
        }
        pRec->record.cb = ulCB;
        pRec->record.pszIcon = pRec->pszMetric;
        pRec = (PCMRECORD) pRec->record.preccNextRecord;
    }

    // The last three values are directly in the unimbr structure...
    pRec->pszMetric = (PSZ) malloc( SZMETRIC_MAX );
    pRec->pszFlag   = (PSZ) malloc( SZFLAGS_MAXZ );
    pRec->fb        = pProps->pCFA->unimbr.fbPanose;
    strcpy( pRec->pszMetric, "Panose table");
    switch ( pProps->pCFA->unimbr.fbPanose ) {
        case 0:                 strcpy( pRec->pszFlag, "");                       break;
        case ASSOC_EXACT_MATCH: strcpy( pRec->pszFlag, "Must match");             break;
        case ASSOC_DONT_CARE:   strcpy( pRec->pszFlag, "Don't care");             break;
        case ASSOC_USE_PARENT:  strcpy( pRec->pszFlag, "Use parent information"); break;
        default:                sprintf( pRec->pszFlag, "0x%08X",
                                         pProps->pCFA->unimbr.fbPanose );
                                break;
    }
    pRec->record.cb = ulCB;
    pRec->record.pszIcon = pRec->pszMetric;
    pRec = (PCMRECORD) pRec->record.preccNextRecord;

    pRec->pszMetric = (PSZ) malloc( SZMETRIC_MAX );
    pRec->pszFlag   = (PSZ) malloc( SZFLAGS_MAXZ );
    pRec->fb        = pProps->pCFA->unimbr.fbFullFamilyname;
    strcpy( pRec->pszMetric, "Full family name");
    switch ( pProps->pCFA->unimbr.fbFullFamilyname ) {
        case 0:                 strcpy( pRec->pszFlag, "");                       break;
        case ASSOC_EXACT_MATCH: strcpy( pRec->pszFlag, "Must match");             break;
        case ASSOC_DONT_CARE:   strcpy( pRec->pszFlag, "Don't care");             break;
        case ASSOC_USE_PARENT:  strcpy( pRec->pszFlag, "Use parent information"); break;
        default:                sprintf( pRec->pszFlag, "0x%08X",
                                         pProps->pCFA->unimbr.fbFullFamilyname );
                                break;
    }
    pRec->record.cb = ulCB;
    pRec->record.pszIcon = pRec->pszMetric;
    pRec = (PCMRECORD) pRec->record.preccNextRecord;

    pRec->pszMetric = (PSZ) malloc( SZMETRIC_MAX );
    pRec->pszFlag   = (PSZ) malloc( SZFLAGS_MAXZ );
    pRec->fb        = pProps->pCFA->unimbr.fbFullFacename;
    strcpy( pRec->pszMetric, "Full face name");
    switch ( pProps->pCFA->unimbr.fbFullFacename ) {
        case 0:                 strcpy( pRec->pszFlag, "");                       break;
        case ASSOC_EXACT_MATCH: strcpy( pRec->pszFlag, "Must match");             break;
        case ASSOC_DONT_CARE:   strcpy( pRec->pszFlag, "Don't care");             break;
        case ASSOC_USE_PARENT:  strcpy( pRec->pszFlag, "Use parent information"); break;
        default:                sprintf( pRec->pszFlag, "0x%08X",
                                         pProps->pCFA->unimbr.fbFullFacename );
                                break;
    }
    pRec->record.cb = ulCB;
    pRec->record.pszIcon = pRec->pszMetric;
    pRec = (PCMRECORD) pRec->record.preccNextRecord;

    // Now we can insert all 51 records
    ri.cb                = sizeof( RECORDINSERT );
    ri.pRecordOrder      = (PRECORDCORE) CMA_END;
    ri.pRecordParent     = NULL;
    ri.zOrder            = (ULONG) CMA_TOP;
    ri.fInvalidateRecord = TRUE;
    ri.cRecordsInsert    = 51;
    WinSendMsg( hwndCnr, CM_INSERTRECORD, MPFROMP( pFirst ), MPFROMP( &ri ));
}


/* ------------------------------------------------------------------------- *
 * PopulateValues_CMB                                                        *
 *                                                                           *
 * Populate the main window UI with the data read from a newly-opened font   *
 * (combined font-file version).                                             *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd : handle of the main program dialog.                          *
 *   PCFEGLOBAL pGlobal: pointer to global program data.                     *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void PopulateValues_CMB( HWND hwnd, PCFEGLOBAL pGlobal )
{
    PUNIFONTMETRICS pUFM;       // pointer to Uni-font metrics data
    PULONG          pulSig;     // pointer to current structure signature
    PSZ             pszFamily,
                    pszFace;
    ULONG           ulCB,
                    i;
    HWND            hwndCnr;
    PCFRECORD       pRec,
                    pFirst;
    RECORDINSERT    ri;
    PCOMPFONT       pComp;
    SHORT           sIdx;
    CHAR            szNum[ 5 ];


    pUFM   = &(pGlobal->font.combined.pMetrics->unifm);
    pulSig = (PULONG) pUFM->Identity;
    if (( pGlobal->font.combined.pMetrics->Identity == SIG_CBFM ) && ( *pulSig == SIG_UNFM )) {

        // Family and face name
        pszFamily = (( pUFM->flOptions & UNIFONTMETRICS_FULLFAMILYNAME_EXIST ) &&
                     (*(pUFM->szFullFamilyname))) ?
                        pUFM->szFullFamilyname: pUFM->ifiMetrics.szFamilyname;
        pszFace   = (( pUFM->flOptions & UNIFONTMETRICS_FULLFACENAME_EXIST ) &&
                     (*(pUFM->szFullFacename))) ?
                        pUFM->szFullFacename  : pUFM->ifiMetrics.szFacename;
        WinSetDlgItemText( hwnd, IDD_FAMILYNAME, pszFamily );
        WinSetDlgItemText( hwnd, IDD_FACENAME,   pszFace );

        // Font family class and subclass
        sprintf( szNum, "%u -", (( pUFM->ifiMetrics.ulFontClass & 0xFF00 ) >> 8 ));
        sIdx = (SHORT) WinSendDlgItemMsg( hwnd, IDD_FONTCLASS, LM_SEARCHSTRING,
                                          MPFROM2SHORT( LSS_PREFIX, LIT_FIRST ),
                                          MPFROMP( szNum ));
        WinSendDlgItemMsg( hwnd, IDD_FONTCLASS, LM_SELECTITEM,
                           MPFROMP( sIdx ), MPFROMSHORT( TRUE ));
        sprintf( szNum, "%u -", ( pUFM->ifiMetrics.ulFontClass & 0xFF ));
        sIdx = (SHORT) WinSendDlgItemMsg( hwnd, IDD_FONTSUBCLASS, LM_SEARCHSTRING,
                                          MPFROM2SHORT( LSS_PREFIX, LIT_FIRST ),
                                          MPFROMP( szNum ));
        WinSendDlgItemMsg( hwnd, IDD_FONTSUBCLASS, LM_SELECTITEM,
                           MPFROMP( sIdx ), MPFROMSHORT( TRUE ));

        // Registry and license settings
        WinSendDlgItemMsg( hwnd, IDD_REGISTRY, SPBM_SETCURRENTVALUE,
                           MPFROMLONG( pUFM->ifiMetrics.idRegistry ), MPVOID );
        if ( pUFM->ifiMetrics.flType & FM_TYPE_LICENSED )
            WinCheckButton( hwnd, IDD_LICENCED, TRUE );

        // now populate the container with the component-font information
        hwndCnr = WinWindowFromID( hwnd, IDD_COMPONENTS );
        ulCB = sizeof( CFRECORD ) - sizeof( MINIRECORDCORE );
        pRec = (PCFRECORD) WinSendMsg( hwndCnr, CM_ALLOCRECORD,
                                       MPFROMLONG( ulCB ),
                                       MPFROMLONG( pGlobal->font.combined.pComponents->ulCmpFonts ));
        pFirst = pRec;
        ulCB = sizeof( MINIRECORDCORE );

        for ( i = 0; i < pGlobal->font.combined.pComponents->ulCmpFonts; i++ ) {
            pComp = pGlobal->font.combined.pComponents->CompFont + i;
            pRec->pszFace      = (PSZ) calloc( FACESIZE, 1 );
            pRec->pszRanges    = (PSZ) calloc( SZRANGES_MAXZ, 1 );
            pRec->pszGlyphList = (PSZ) calloc( GLYPHNAMESIZE, 1 );
            pRec->pszFlags     = (PSZ) calloc( SZFLAGS_MAXZ, 1 );
            strcpy( pRec->pszFace, pComp->CompFontAssoc.unifm.ifiMetrics.szFacename );
            if ( pComp->CompFontAssoc.ulGlyphRanges > 1 )
                sprintf( pRec->pszRanges, "(%u ranges)", pComp->CompFontAssoc.ulGlyphRanges );
            else if ( pComp->CompFontAssoc.ulGlyphRanges == 1 ) {
                sprintf( pRec->pszRanges, "%u - %u",
                         pComp->CompFontAssoc.GlyphRange[0].giStart,
                         pComp->CompFontAssoc.GlyphRange[0].giEnd );
            }
            else
                strcpy( pRec->pszRanges, "None");
            strcpy( pRec->pszGlyphList, pComp->CompFontAssoc.unifm.ifiMetrics.szGlyphlistName );
            sprintf( pRec->pszFlags, "0x%X", pComp->CompFontAssoc.flFlags );
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
        ri.cRecordsInsert    = pGlobal->font.combined.pComponents->ulCmpFonts;
        WinSendMsg( hwndCnr, CM_INSERTRECORD, MPFROMP( pFirst ), MPFROMP( &ri ));
    }
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
 * ProductInfoDlgProc                                                        *
 *                                                                           *
 * Dialog procedure for the product information dialog.                      *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY ProductInfoDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    PCFEGLOBAL pGlobal;

    switch ( msg ) {

        case WM_INITDLG:
            pGlobal = (PCFEGLOBAL) mp2;
            CentreWindow( hwnd, pGlobal->hwndMain );
            break;

        default: break;
    }

    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * RangeDlgProc                                                              *
 *                                                                           *
 * Dialog procedure for the glyph range dialog.                              *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY RangeDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    static PGRDATA pData;

    switch ( msg ) {

        case WM_INITDLG:
            pData = (PGRDATA) mp2;
            WinSendDlgItemMsg( hwnd, IDD_GLYPHSTART, SPBM_SETLIMITS,
                               MPFROMLONG( (ULONG) pData->usLastChar ), MPFROMLONG( (ULONG) pData->usFirstChar ));
            WinSendDlgItemMsg( hwnd, IDD_GLYPHEND, SPBM_SETLIMITS,
                               MPFROMLONG( (ULONG) pData->usLastChar ), MPFROMLONG( (ULONG) pData->usFirstChar ));
            WinSendDlgItemMsg( hwnd, IDD_GLYPHTARGET, SPBM_SETLIMITS,
                               MPFROMLONG( 65535 ), MPFROMLONG( 0 ));
            if ( pData->fEditExisting ) {
                WinSendDlgItemMsg( hwnd, IDD_GLYPHSTART, SPBM_SETCURRENTVALUE,
                                   MPFROMLONG( pData->range.giStart ), MPVOID );
                WinSendDlgItemMsg( hwnd, IDD_GLYPHEND, SPBM_SETCURRENTVALUE,
                                   MPFROMLONG( pData->range.giEnd ), MPVOID );
                WinSendDlgItemMsg( hwnd, IDD_GLYPHTARGET, SPBM_SETCURRENTVALUE,
                                   MPFROMLONG( pData->range.giTarget ), MPVOID );
            }
            CentreWindow( hwnd, pData->hwndMain );
            break;
        // WM_INITDLG


        case WM_COMMAND:
            switch( SHORT1FROMMP( mp1 )) {

                case DID_OK:
                    WinSendDlgItemMsg( hwnd, IDD_GLYPHSTART, SPBM_QUERYVALUE,
                                       MPFROMP( &(pData->range.giStart) ),
                                       MPFROM2SHORT( 0, SPBQ_ALWAYSUPDATE ));
                    WinSendDlgItemMsg( hwnd, IDD_GLYPHEND, SPBM_QUERYVALUE,
                                       MPFROMP( &(pData->range.giEnd) ),
                                       MPFROM2SHORT( 0, SPBQ_ALWAYSUPDATE ));
                    WinSendDlgItemMsg( hwnd, IDD_GLYPHTARGET, SPBM_QUERYVALUE,
                                       MPFROMP( &(pData->range.giTarget) ),
                                       MPFROM2SHORT( 0, SPBQ_ALWAYSUPDATE ));
                    break;

                case DID_CANCEL:
                    break;

            }
            break;
        // WM_COMMAND


        default: break;
    }

    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * ReadFontFile                                                              *
 *                                                                           *
 * Opens, reads, and parses a combined or Uni font file.                     *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND       hwnd   : handle of the main program dialog.                  *
 *   PSZ        pszFile: fully-qualified name of the font file to open.      *
 *   PCFEGLOBAL pGlobal: pointer to global program data.                     *
 *                                                                           *
 * RETURNS: USHORT                                                           *
 *   One of the FONT_TYPE_xxx flags depending on the type of font file       *
 *   opened, or 0 if the file type is unrecognized/not supported.            *
 * ------------------------------------------------------------------------- */
USHORT ReadFontFile( HWND hwnd, PSZ pszFile, PCFEGLOBAL pGlobal )
{
    PGENERICRECORD pSig;
    FILESTATUS3    fs3;
    CHAR           szError[ SZERR_MAXZ ] = {0};
    ULONG          ulAction,    // action reported by DosOpen
                   flOpFn,      // function flags for DosOpen
                   flOpMd,      // mode flags for DosOpen
                   cbBuffer,    // size of file (and thus of our read buffer)
                   cbRead,      // number of bytes read by DosRead
                   ulArray;     // size of COMPFONTS array minus the first entry
    USHORT         usType = 0;  // return value
    PBYTE          pBuffer;     // raw buffer containing file contents
    APIRET         rc;          // return code from Dos**


    // Close any currently-open file and free its contents
    CloseFontFile( hwnd, pGlobal );

    // Open the file
    flOpFn = OPEN_ACTION_OPEN_IF_EXISTS | OPEN_ACTION_FAIL_IF_NEW;
    flOpMd = OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_SEQUENTIAL |
             OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE;
    rc = DosOpen( pszFile, &(pGlobal->hFile), &ulAction, 0L, 0L, flOpFn, flOpMd, 0L );
    if ( rc != NO_ERROR || !pGlobal->hFile ) {
        sprintf( szError, "Failed to open file %s.\n\nThe return code was %u.\n",
                 pszFile, rc );
        WinMessageBox( HWND_DESKTOP, hwnd, szError, "File Open Error",
                       0, MB_MOVEABLE | MB_OK | MB_ERROR );
        return 0;
    }

    // Get the file size
    rc = DosQueryFileInfo( pGlobal->hFile, FIL_STANDARD, &fs3, sizeof( fs3 ));
    if ( rc != NO_ERROR ) {
        sprintf( szError, "Failed to get file information for %s.\n\nThe return code was %u.\n",
                 pszFile, rc );
        WinMessageBox( HWND_DESKTOP, hwnd, szError, "File Read Error",
                       0, MB_MOVEABLE | MB_OK | MB_ERROR );
        return 0;
    }
    if ( fs3.cbFile < sizeof( GENERICRECORD )) {
        // If the file is too small to contain any font info, return an error
        strcpy( szError, "The file format is not recognized.");
        WinMessageBox( HWND_DESKTOP, hwnd, szError, "Unsupported Format",
                       0, MB_MOVEABLE | MB_OK | MB_ERROR );
        return 0;
    }

    // Allocate our read/write buffer using the file size
    cbBuffer = fs3.cbFile;
    rc = DosAllocMem( (PPVOID) &pBuffer, cbBuffer, PAG_READ | PAG_WRITE | PAG_COMMIT );
    if ( rc != NO_ERROR ) {
        sprintf( szError, "Failed to allocate memory.\n\nThe return code was %u.\n",
                 rc );
        WinMessageBox( HWND_DESKTOP, hwnd, szError, "Out of Memory",
                       0, MB_MOVEABLE | MB_OK | MB_ERROR );
        return 0;
    }

    /* Read the whole file contents into memory (this is really not that elegant
     * but the kinds of font files we're dealing with should not be very large).
     */
    rc = DosRead( pGlobal->hFile, pBuffer, cbBuffer, &cbRead );
    if ( rc || !cbRead ) {
        sprintf( szError, "Failed to read file %s.\n\nThe return code was %u.\n",
                 pszFile, rc );
        WinMessageBox( HWND_DESKTOP, hwnd, szError, "File Read Error",
                       0, MB_MOVEABLE | MB_OK | MB_ERROR );
        goto finish;
    }

    // Verify the file format
    pSig = (PGENERICRECORD) pBuffer;
    if (( pSig->Identity == SIG_CBFS ) &&
        ( pSig->ulSize == sizeof( COMBFONTSIGNATURE )))
    {
        // Combined font
        PCOMBFONTSIGNATURE pSignature;
        PCOMBFONTMETRICS   pMetrics;
        PCOMPFONTHEADER    pComponents;
        PCOMBFONTEND       pEnd;

        pSignature  = (PCOMBFONTSIGNATURE) pSig;
        pMetrics    = (PCOMBFONTMETRICS)( pBuffer + pSig->ulSize );
        pComponents = (PCOMPFONTHEADER)( (PBYTE) pMetrics + pMetrics->ulSize );
        ulArray = pComponents->ulCmpFonts ?
                  (( pComponents->ulCmpFonts - 1 ) * sizeof( COMPFONT )): 0;
        pEnd = (PCOMBFONTEND)( (PBYTE) pComponents + pComponents->ulSize + ulArray );

        rc = DosAllocMem( (PPVOID) &(pGlobal->font.combined.pSignature),
                          pSignature->ulSize, PAG_READ | PAG_WRITE | PAG_COMMIT );
        if ( rc != NO_ERROR ) goto finish;
        rc = DosAllocMem( (PPVOID) &(pGlobal->font.combined.pMetrics),
                          pMetrics->ulSize, PAG_READ | PAG_WRITE | PAG_COMMIT );
        if ( rc != NO_ERROR ) goto finish;
        rc = DosAllocMem( (PPVOID) &(pGlobal->font.combined.pComponents),
                          pComponents->ulSize + ulArray,
                          PAG_READ | PAG_WRITE | PAG_COMMIT );
        if ( rc != NO_ERROR ) goto finish;
        rc = DosAllocMem( (PPVOID) &(pGlobal->font.combined.pEnd),
                          pEnd->ulSize, PAG_READ | PAG_WRITE | PAG_COMMIT );
        if ( rc != NO_ERROR ) goto finish;

        memcpy( pGlobal->font.combined.pSignature, pSignature, pSignature->ulSize );
        memcpy( pGlobal->font.combined.pMetrics, pMetrics, pMetrics->ulSize );
        memcpy( pGlobal->font.combined.pComponents, pComponents,
                pComponents->ulSize + ulArray );
        memcpy( pGlobal->font.combined.pEnd, pEnd, pEnd->ulSize );

        pGlobal->cbFile = fs3.cbFile;
        strncpy( pGlobal->szCurrentFile, pszFile, CCHMAXPATH );
        pGlobal->usType = FONT_TYPE_CMB;
        usType = pGlobal->usType;
    }
#if 0
    else if (( pSig->Identity == SIG_ABRS ) &&
        ( pSig->ulSize == sizeof( ABRFILESIGNATURE )))
    {
        // Associated bitmap rules file
        pGlobal->font.abr.pSignature    = (PABRFILESIGNATURE) pSig;
        pGlobal->font.abr.pAssociations = (PFONTASSOCIATION)( pBuffer + pSig->ulSize );
        ulArray = ( pGlobal->font.abr.pSignature->ulCount + 1 ) * sizeof( FONTASSOCIATION );
        pGlobal->font.abr.pEnd = (PABRFILEEND)
                                    ( (PBYTE)(pGlobal->font.abr.pAssociations) + ulArray );
        pGlobal->cbFile = fs3.cbFile;
        strncpy( pGlobal->szCurrentFile, pszFile, CCHMAXPATH );
        pGlobal->usType = FONT_TYPE_ABR;
        usType = pGlobal->usType;
    }
#endif
    else if (( pSig->Identity == SIG_UNFD ) &&
        ( pSig->ulSize == sizeof( UNIFONTDIRECTORY )))
    {
        // Uni-font
        pGlobal->font.pUFontDir = (PUNIFONTDIRECTORY) pSig;
        pGlobal->cbFile = fs3.cbFile;
        strncpy( pGlobal->szCurrentFile, pszFile, CCHMAXPATH );
        pGlobal->usType = FONT_TYPE_UNI;
        usType = pGlobal->usType;
    }
    else {
        if ( pSig->ulSize == sizeof( COMBFONTSIGNATURE ))
            sprintf( szError, "The font file format (\"%s\") is not supported.",
                     ((PCOMBFONTSIGNATURE)pSig)->szSignature );
        else
            strcpy( szError, "The file format is not recognized.");
        WinMessageBox( HWND_DESKTOP, hwnd, szError, "Unsupported Format",
                       0, MB_MOVEABLE | MB_OK | MB_ERROR );
    }

finish:
    if ( pBuffer ) DosFreeMem( pBuffer );
    return usType;
}


/* ------------------------------------------------------------------------- *
 * SelectInstalledFont                                                       *
 *                                                                           *
 * Allows the user to select an installed font face using the font dialog.   *
 * The selected font's face name will be returned in the passed buffer.      *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd      : Handle of the current window.                      (I) *
 *   PSZ  szFacename: Pointer to face name buffer (FACESIZE bytes)       (O) *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   FALSE on failure or if the user cancelled without a selection.          *
 * ------------------------------------------------------------------------- */
BOOL SelectInstalledFont( HWND hwnd, PSZ pszFacename )
{
    FONTDLG fontdlg = {0};
    CHAR    szName[ FACESIZE ] = {0};
    HWND    hwndFD;
    HPS     hps;
    BOOL    fRC = FALSE;

    if ( !pszFacename ) return FALSE;

    hps = WinGetScreenPS( HWND_DESKTOP );
    fontdlg.cbSize         = sizeof( FONTDLG );
    fontdlg.hpsScreen      = hps;
    fontdlg.pszFamilyname  = szName;
    fontdlg.usFamilyBufLen = FACESIZE;
    fontdlg.usWeight       = 5;
    fontdlg.fxPointSize    = MAKEFIXED( 10, 0 );
    fontdlg.clrFore        = SYSCLR_WINDOWTEXT;
    fontdlg.clrBack        = SYSCLR_WINDOW;
    fontdlg.fl             = FNTS_CENTER;
    hwndFD = WinFontDlg( HWND_DESKTOP, hwnd, &fontdlg );
    if (( hwndFD ) && ( fontdlg.lReturn == DID_OK )) {
        strncpy( pszFacename, fontdlg.fAttrs.szFacename, FACESIZE-1 );
        fRC = TRUE;
    }
    WinReleasePS( hps );

    return fRC;
}


/* ------------------------------------------------------------------------- *
 * SetupCnrCF                                                                *
 *                                                                           *
 * Sets up the component-font container.                                     *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd : handle of the main program dialog.                          *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void SetupCnrCF( HWND hwnd )
{
    HWND            hwndCnr;
    CNRINFO         cnr = {0};
    PFIELDINFO      pFld,
                    pFld1st;
    FIELDINFOINSERT finsert;
    POINTL          aptl[ 2 ];

    WinSetDlgItemText( hwnd, IDD_STATUS, "Combined Font File");

    // update the container & groupbox layout
    WinSetDlgItemText( hwnd, IDD_GROUPBOX, "Component fonts");
    aptl[0].x = 285;
    aptl[0].y = 78;
    aptl[1].x = 275;
    aptl[1].y = 50;
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
    // (second column: associated glyph ranges)
    pFld->cb = sizeof( FIELDINFO );
    pFld->pTitleData = "Associated Glyphs";
    pFld->flData     = CFA_STRING | CFA_FIREADONLY | CFA_VCENTER | CFA_HORZSEPARATOR | CFA_SEPARATOR;
    pFld->offStruct  = FIELDOFFSET( CFRECORD, pszRanges );
    pFld = pFld->pNextFieldInfo;
    // (third column: glyph-list, i.e. the font encoding exported by GPI)
    pFld->cb = sizeof( FIELDINFO );
    pFld->pTitleData = "GPI Encoding";
    pFld->flData     = CFA_STRING | CFA_FIREADONLY | CFA_VCENTER | CFA_HORZSEPARATOR;
    pFld->offStruct  = FIELDOFFSET( CFRECORD, pszGlyphList );

    finsert.cb                   = (ULONG) sizeof( FIELDINFOINSERT );
    finsert.pFieldInfoOrder      = (PFIELDINFO) CMA_END;
    finsert.fInvalidateFieldInfo = TRUE;
    finsert.cFieldInfoInsert     = 3;
    WinSendMsg( hwndCnr, CM_INSERTDETAILFIELDINFO,
                MPFROMP( pFld1st ), MPFROMP( &finsert ));

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
 * Dialog procedure for the product information dialog.                      *
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

