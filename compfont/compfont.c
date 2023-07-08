/*****************************************************************************
 * compfont.c                                                                *
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


// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

// Font metric container field description strings
PSZ g_MetricItems[] = {         // corresponding field in IFIMETRICS32
    "Family name",              // 0  (UCHAR[FACESIZE])
    "Face name",                // 1  (UCHAR[FACESIZE])
    "Glyphlist",                // 2  (UCHAR[GLYPHNAMESIZE])
    "IBM registered ID",        // 3  (ULONG)
    "Em height",                // 4  (LONG)
    "X height",                 // 5  (LONG)
    "Max ascender",             // 6  (LONG)
    "Max descender",            // 7  (LONG)
    "Lowercase ascent",         // 8  (LONG)
    "Lowercase descent",        // 9  (LONG)
    "Internal leading",         // 10 (LONG)
    "External leading",         // 11 (LONG)
    "Avg char width",           // 12 (LONG)
    "Max char increment",       // 13 (LONG)
    "Em increment",             // 14 (LONG)
    "Max baseline extent",      // 15 (LONG)
    "Character slope",          // 16 (FIXED)
    "Inline direction",         // 17 (FIXED)
    "Character rotation",       // 18 (FIXED)
    "Weight class",             // 19 (ULONG)
    "Width class",              // 20 (ULONG)
    "X resolution",             // 21 (LONG)
    "Y resolution",             // 22 (LONG)
    "First character",          // 23 (GLYPH)
    "Last character",           // 24 (GLYPH)
    "Default character",        // 25 (GLYPH)
    "Break character",          // 26 (GLYPH)
    "Nominal point size",       // 27 (ULONG)
    "Minimum point size",       // 28 (ULONG)
    "Maximum point size",       // 29 (ULONG)
    "Type flags",               // 30 (ULONG)
    "Definition flags",         // 31 (ULONG)
    "Selection flags",          // 32 (ULONG)
    "Font capabilities",        // 33 (ULONG)
    "Subscript size X",         // 34 (LONG)
    "Subscript size Y",         // 35 (LONG)
    "Subscript offset X",       // 36 (LONG)
    "Subscript offset Y",       // 37 (LONG)
    "Superscript size X",       // 38 (LONG)
    "Superscript size Y",       // 39 (LONG)
    "Superscript offset X",     // 40 (LONG)
    "Superscript offset Y",     // 41 (LONG)
    "Underscore size",          // 42 (LONG)
    "Underscore position",      // 43 (LONG)
    "Strikeout size",           // 44 (LONG)
    "Strikeout position",       // 45 (LONG)
    "Kerning pairs",            // 46 (ULONG)
    "Font class"                // 47 (ULONG)
};

// Font family class names
PSZ g_FamilyClasses[] = {
    FF_CLASS_0, FF_CLASS_1, FF_CLASS_2, FF_CLASS_3, FF_CLASS_4,
    FF_CLASS_5, FF_CLASS_7, FF_CLASS_8, FF_CLASS_9, FF_CLASS_10, FF_CLASS_12
};

// Font family 0 subclass names
PSZ g_FamilySubclasses0[] = {
    FF_SUBCLASS_0
};

// Font family 1 subclass names
PSZ g_FamilySubclasses1[] = {
    FF_SUBCLASS_0,   FF_SUBCLASS_1_1, FF_SUBCLASS_1_2, FF_SUBCLASS_1_3,
    FF_SUBCLASS_1_4, FF_SUBCLASS_1_5, FF_SUBCLASS_1_6, FF_SUBCLASS_1_7,
    FF_SUBCLASS_1_8, FF_SUBCLASS_15
};

// Font family 2 subclass names
PSZ g_FamilySubclasses2[] = {
    FF_SUBCLASS_0, FF_SUBCLASS_2_1, FF_SUBCLASS_2_2, FF_SUBCLASS_15
};

// Font family 3 subclass names
PSZ g_FamilySubclasses3[] = {
    FF_SUBCLASS_0, FF_SUBCLASS_3_1, FF_SUBCLASS_3_2, FF_SUBCLASS_15
};

// Font family 4 subclass names
PSZ g_FamilySubclasses4[] = {
    FF_SUBCLASS_0,   FF_SUBCLASS_4_1, FF_SUBCLASS_4_2, FF_SUBCLASS_4_3,
    FF_SUBCLASS_4_4, FF_SUBCLASS_4_5, FF_SUBCLASS_4_6, FF_SUBCLASS_4_7,
    FF_SUBCLASS_15
};

// Font family 5 subclass names
PSZ g_FamilySubclasses5[] = {
    FF_SUBCLASS_0, FF_SUBCLASS_5_1, FF_SUBCLASS_5_2, FF_SUBCLASS_5_3,
    FF_SUBCLASS_5_4, FF_SUBCLASS_5_5, FF_SUBCLASS_15
};

// Font family 7 subclass names
PSZ g_FamilySubclasses7[] = {
    FF_SUBCLASS_0, FF_SUBCLASS_7_1, FF_SUBCLASS_15
};

// Font family 8 subclass names
PSZ g_FamilySubclasses8[] = {
    FF_SUBCLASS_0,    FF_SUBCLASS_8_1, FF_SUBCLASS_8_2, FF_SUBCLASS_8_3,
    FF_SUBCLASS_8_4,  FF_SUBCLASS_8_5, FF_SUBCLASS_8_6, FF_SUBCLASS_8_9,
    FF_SUBCLASS_8_10, FF_SUBCLASS_15
};

// Font family 9 subclass names
PSZ g_FamilySubclasses9[] = {
    FF_SUBCLASS_0,   FF_SUBCLASS_9_1, FF_SUBCLASS_9_2, FF_SUBCLASS_9_3,
    FF_SUBCLASS_9_4, FF_SUBCLASS_15
};

// Font family 10 subclass names
PSZ g_FamilySubclasses10[] = {
    FF_SUBCLASS_0,    FF_SUBCLASS_10_1, FF_SUBCLASS_10_2, FF_SUBCLASS_10_3,
    FF_SUBCLASS_10_4, FF_SUBCLASS_10_5, FF_SUBCLASS_10_6, FF_SUBCLASS_10_7,
    FF_SUBCLASS_10_8, FF_SUBCLASS_15
};

// Font family 12 subclass names
PSZ g_FamilySubclasses12[] = {
    FF_SUBCLASS_0, FF_SUBCLASS_12_3, FF_SUBCLASS_12_6, FF_SUBCLASS_12_7,
    FF_SUBCLASS_15
};


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
//    global.usType = FONT_TYPE_CMB;

    // pass filename specified on command line, if any
    if ( pszOpen )
        strncpy( global.szCurrentFile, pszOpen, CCHMAXPATH );

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

        if ( pszOpen )
            WinPostMsg( global.hwndMain, WM_READFILE, MPFROMP( pszOpen ), MPVOID );
        else
            NewFontFile( global.hwndMain, &global, FONT_TYPE_CMB );

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
//    CFPROPS    cfprops;
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

            EmboldenWindowText( WinWindowFromID( hwnd, IDD_STATUS ), NULLHANDLE );

            CentreWindow( hwnd, NULLHANDLE );
            WinShowWindow( hwnd, TRUE );

            return (MRESULT) FALSE;
        // WM_INITDLG end


        // WM_READFILE: Custom message used to open and parse a font file
        //
        case WM_READFILE:                   // mp1 = filename
            if ( ReadFontFile( hwnd, (PSZ) mp1, pGlobal )) {
                switch ( pGlobal->usType ) {
                    case FONT_TYPE_CMB:
                        SetupWindowCF( hwnd );
                        SetupCnrCF( hwnd );
                        PopulateValues_CMB( hwnd, pGlobal );
                        break;
                    case FONT_TYPE_ABR:
                        SetupWindowUF( hwnd );  // same UI as Uni-fonts
                        SetupCnrAB( hwnd );
                        PopulateValues_ABR( hwnd, pGlobal );
                        break;
                    case FONT_TYPE_UNI:
                        SetupWindowUF( hwnd );
                        SetupCnrUF( hwnd );
                        break;
                }
            }
            return (MRESULT) 0;
        // WM_READFILE end


        case WM_CLOSE:
            // TODO verify quit if changes are outstanding
            WinPostMsg( hwnd, WM_QUIT, 0, 0 );
            return (MRESULT) 0;
        // WM_CLOSE end


        case WM_COMMAND:
            switch( SHORT1FROMMP( mp1 )) {

                case ID_NEWCMB:                 // "New -> Combined font or alias file"
                    NewFontFile( hwnd, pGlobal, FONT_TYPE_CMB );
                    return (MRESULT) 0;

                case ID_NEWUNI:                 // "New -> Uni-font file"
                    NewFontFile( hwnd, pGlobal, FONT_TYPE_UNI );
                    return (MRESULT) 0;

                case ID_OPEN:                  // "Open"
                    // TODO see if unsaved changes are pending
                    memset( &fd, 0, sizeof( FILEDLG ));
                    fd.cbSize = sizeof( FILEDLG );
                    fd.fl = FDS_CENTER | FDS_OPEN_DIALOG;
                    fd.pszTitle = NULL;
                    sprintf( fd.szFullFile, "*.CMB");
                    hwndFD = WinFileDlg( HWND_DESKTOP, hwnd, &fd );
                    if ( hwndFD && fd.lReturn == DID_OK )
                        WinSendMsg( hwnd, WM_READFILE, MPFROMP( fd.szFullFile ), MPVOID );
                    return (MRESULT) 0;

                case ID_EXIT:                   // "Exit"
                    WinPostMsg( hwnd, WM_CLOSE, 0, 0 );
                    return (MRESULT) 0;

                case ID_ADD:                    // "Add"
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
                    if ( pRec && ( pGlobal->usType == FONT_TYPE_CMB )
                              && pGlobal->font.combined.pFontList
                              && pGlobal->font.combined.ulCmpFonts ) {
#if 0
                        cfprops.cb = sizeof( CFPROPS );
                        cfprops.hab = pGlobal->hab;
                        cfprops.hmq = pGlobal->hmq;
                        cfprops.hwndMain = hwnd;
                        cfprops.fEditExisting = TRUE;
                        cfprops.pCFA = &(pGlobal->font.combined.pComponents->CompFont[ pRec->ulIndex ].CompFontAssoc);
                        WinDlgBox( HWND_DESKTOP, hwnd, (PFNWP) CompFontDlgProc,
                                   NULLHANDLE, IDD_COMPFONT, &cfprops );
#else
                        EditComponentFont( hwnd, pGlobal, pRec->ulIndex );
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
        // WM_COMMAND end


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
        // WM_CONTROL end


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
        // WM_MINMAXFRAME end


        case WM_PRESPARAMCHANGED:
            if ( (ULONG) mp1 == PP_FONTNAMESIZE )
                EmboldenWindowText( WinWindowFromID( hwnd, IDD_STATUS ), hwnd );
            break;
        // WM_PRESPARAMCHANGED end


        case WM_DESTROY:
            CloseFontFile( hwnd, pGlobal );
            WinSendDlgItemMsg( hwnd, IDD_COMPONENTS, CM_REMOVERECORD, NULL,
                               MPFROM2SHORT( 0, CMA_INVALIDATE | CMA_FREE ));
            WinSendDlgItemMsg( hwnd, IDD_COMPONENTS, CM_REMOVEDETAILFIELDINFO, NULL,
                               MPFROM2SHORT( 0, CMA_INVALIDATE | CMA_FREE ));
            break;
        // WM_DESTROY end


        default: break;
    }

    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * NewFontFile                                                               *
 *                                                                           *
 * Create a new, empty font file of the specified type.                      *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd         : handle of the main program dialog.                  *
 *   PCFEGLOBAL pGlobal: pointer to global program data.                     *
 *   USHORT usType     : one of the FONT_TYPE_xxx constants.                 *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void NewFontFile( HWND hwnd, PCFEGLOBAL pGlobal, USHORT usType )
{
    BOOL bCreated;
    // TODO see if unsaved changes are pending

    // Close the current file
    if ( pGlobal->hFile )
        CloseFontFile( hwnd, pGlobal );
    pGlobal->bModified = FALSE;

    switch( usType ) {
        default:
        case FONT_TYPE_CMB:
            bCreated = NewFont_CMB( hwnd, pGlobal );
            break;
        case FONT_TYPE_UNI:
            bCreated = NewFont_UNI( hwnd, pGlobal );
            break;
        // We don't support creating other types yet
    }

    if ( bCreated )
        ShowFileName( pGlobal );
/*
    else
    // TODO if an error occured here, we should prompt for retry or else exit
    // the program, because there's probably not much else we can safely do.
*/

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
            wrap_free( (PPVOID) &(pGlobal->font.combined.pSignature) );
            wrap_free( (PPVOID) &(pGlobal->font.combined.pMetrics) );
            wrap_free( (PPVOID) &(pGlobal->font.combined.pEnd) );
            ComponentListFree( &(pGlobal->font.combined) );
            break;

        case FONT_TYPE_UNI:
            wrap_free( (PPVOID) &(pGlobal->font.pUFontDir) );
            break;

        case FONT_TYPE_ABR:
            wrap_free( (PPVOID) &(pGlobal->font.abr.pSignature) );
            wrap_free( (PPVOID) &(pGlobal->font.abr.pAssociations) );
            wrap_free( (PPVOID) &(pGlobal->font.abr.pEnd) );
            break;

        case FONT_TYPE_PCR:
        case FONT_TYPE_UFF:
        default:
             break;
    }

    if ( pGlobal->hFile )
        DosClose( pGlobal->hFile );
    pGlobal->szCurrentFile[0] = '\0';
    pGlobal->cbFile = 0;
    pGlobal->usType = 0;
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
void PopulateMetricFlags( HWND hwndCnr, PFONTASSOCIATION pFA )
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

    /* The first 48 flag values are sequential byte fields in the ifi32mbr
     * structure, so we can cast the structure to an array and loop through.
     * The corresponding description strings are defined in the global array
     * g_MetricItems[] to facilitate this.
     */
    fbMetricFields = (PBYTE)(&(pFA->unimbr.ifi32mbr));
    for ( i = 0; i < 48; i++ ) {
        pRec->pszMetric = (PSZ) malloc( SZMETRIC_MAX );
        pRec->pszFlag   = (PSZ) malloc( SZFLAGS_MAXZ );
        pRec->pszValue  = (PSZ) malloc( SZMETRICVAL_MAX );
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
        // Now copy the actual value from the corresponding IFIMETRICS32
        switch ( i ) {
            case  0: strncpy( pRec->pszValue, pFA->unifm.ifiMetrics.szFamilyname,    SZMETRICVAL_MAX ); break;
            case  1: strncpy( pRec->pszValue, pFA->unifm.ifiMetrics.szFacename,      SZMETRICVAL_MAX ); break;
            case  2: strncpy( pRec->pszValue, pFA->unifm.ifiMetrics.szGlyphlistName, SZMETRICVAL_MAX ); break;
            case  3: sprintf( pRec->pszValue, "%u", pFA->unifm.ifiMetrics.idRegistry ); break;
            case  4: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lCapEmHeight ); break;
            case  5: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lXHeight ); break;
            case  6: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lMaxAscender ); break;
            case  7: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lMaxDescender ); break;
            case  8: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lLowerCaseAscent ); break;
            case  9: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lLowerCaseDescent ); break;
            case 10: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lInternalLeading ); break;
            case 11: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lExternalLeading ); break;
            case 12: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lAveCharWidth ); break;
            case 13: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lMaxCharInc ); break;
            case 14: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lEmInc ); break;
            case 15: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lMaxBaselineExt ); break;
            case 16: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.fxCharSlope ); break;
            case 17: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.fxInlineDir ); break;
            case 18: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.fxCharRot ); break;
            case 19: sprintf( pRec->pszValue, "%u", pFA->unifm.ifiMetrics.ulWeightClass ); break;
            case 20: sprintf( pRec->pszValue, "%u", pFA->unifm.ifiMetrics.ulWidthClass ); break;
            case 21: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lEmSquareSizeX ); break;
            case 22: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lEmSquareSizeY ); break;
            case 23: sprintf( pRec->pszValue, "%u", pFA->unifm.ifiMetrics.giFirstChar ); break;
            case 24: sprintf( pRec->pszValue, "%u", pFA->unifm.ifiMetrics.giLastChar ); break;
            case 25: sprintf( pRec->pszValue, "%u", pFA->unifm.ifiMetrics.giDefaultChar ); break;
            case 26: sprintf( pRec->pszValue, "%u", pFA->unifm.ifiMetrics.giBreakChar ); break;
            case 27: sprintf( pRec->pszValue, "%u", pFA->unifm.ifiMetrics.ulNominalPointSize ); break;
            case 28: sprintf( pRec->pszValue, "%u", pFA->unifm.ifiMetrics.ulMinimumPointSize ); break;
            case 29: sprintf( pRec->pszValue, "%u", pFA->unifm.ifiMetrics.ulMaximumPointSize ); break;
            case 30: sprintf( pRec->pszValue, "0x%08X", pFA->unifm.ifiMetrics.flType ); break;
            case 31: sprintf( pRec->pszValue, "0x%08X", pFA->unifm.ifiMetrics.flDefn ); break;
            case 32: sprintf( pRec->pszValue, "0x%08X", pFA->unifm.ifiMetrics.flSelection ); break;
            case 33: sprintf( pRec->pszValue, "%X", pFA->unifm.ifiMetrics.flCapabilities ); break;
            case 34: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lSubscriptXSize ); break;
            case 35: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lSubscriptYSize ); break;
            case 36: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lSubscriptXOffset ); break;
            case 37: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lSubscriptYOffset ); break;
            case 38: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lSuperscriptXSize ); break;
            case 39: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lSuperscriptYSize ); break;
            case 40: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lSuperscriptXOffset ); break;
            case 41: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lSuperscriptYOffset ); break;
            case 42: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lUnderscoreSize ); break;
            case 43: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lUnderscorePosition ); break;
            case 44: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lStrikeoutSize ); break;
            case 45: sprintf( pRec->pszValue, "%d", pFA->unifm.ifiMetrics.lStrikeoutPosition ); break;
            case 46: sprintf( pRec->pszValue, "%u", pFA->unifm.ifiMetrics.ulKerningPairs ); break;
            case 47: sprintf( pRec->pszValue, "%u", pFA->unifm.ifiMetrics.ulFontClass ); break;
            default: sprintf( pRec->pszValue, ""); break;
        }
        pRec->record.cb = ulCB;
        pRec->record.pszIcon = pRec->pszMetric;
        pRec = (PCMRECORD) pRec->record.preccNextRecord;
    }

    // The last three values are directly in the unimbr structure...
    pRec->pszMetric = (PSZ) malloc( SZMETRIC_MAX );
    pRec->pszFlag   = (PSZ) malloc( SZFLAGS_MAXZ );
    pRec->pszValue  = (PSZ) malloc( SZMETRICVAL_MAX );
    pRec->fb        = pFA->unimbr.fbPanose;
    strcpy( pRec->pszMetric, "Panose table");
    switch ( pFA->unimbr.fbPanose ) {
        case 0:                 strcpy( pRec->pszFlag, "");                       break;
        case ASSOC_EXACT_MATCH: strcpy( pRec->pszFlag, "Must match");             break;
        case ASSOC_DONT_CARE:   strcpy( pRec->pszFlag, "Don't care");             break;
        case ASSOC_USE_PARENT:  strcpy( pRec->pszFlag, "Use parent information"); break;
        default:                sprintf( pRec->pszFlag, "0x%08X",
                                         pFA->unimbr.fbPanose );
                                break;
    }
    sprintf( pRec->pszValue, "{%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,0x%X,0x%X}",
                                pFA->unifm.panose[0],
                                pFA->unifm.panose[1],
                                pFA->unifm.panose[2],
                                pFA->unifm.panose[3],
                                pFA->unifm.panose[4],
                                pFA->unifm.panose[5],
                                pFA->unifm.panose[6],
                                pFA->unifm.panose[7],
                                pFA->unifm.panose[8],
                                pFA->unifm.panose[9],
                                pFA->unifm.panose[10],
                                pFA->unifm.panose[11] );
    pRec->record.cb = ulCB;
    pRec->record.pszIcon = pRec->pszMetric;
    pRec = (PCMRECORD) pRec->record.preccNextRecord;

    pRec->pszMetric = (PSZ) malloc( SZMETRIC_MAX );
    pRec->pszFlag   = (PSZ) malloc( SZFLAGS_MAXZ );
    pRec->pszValue  = (PSZ) malloc( SZMETRICVAL_MAX );
    pRec->fb        = pFA->unimbr.fbFullFamilyname;
    strcpy( pRec->pszMetric, "Full family name");
    switch ( pFA->unimbr.fbFullFamilyname ) {
        case 0:                 strcpy( pRec->pszFlag, "");                       break;
        case ASSOC_EXACT_MATCH: strcpy( pRec->pszFlag, "Must match");             break;
        case ASSOC_DONT_CARE:   strcpy( pRec->pszFlag, "Don't care");             break;
        case ASSOC_USE_PARENT:  strcpy( pRec->pszFlag, "Use parent information"); break;
        default:                sprintf( pRec->pszFlag, "0x%08X",
                                         pFA->unimbr.fbFullFamilyname );
                                break;
    }
    strncpy( pRec->pszValue, pFA->unifm.szFullFamilyname, SZMETRICVAL_MAX-1 );
    pRec->record.cb = ulCB;
    pRec->record.pszIcon = pRec->pszMetric;
    pRec = (PCMRECORD) pRec->record.preccNextRecord;

    pRec->pszMetric = (PSZ) malloc( SZMETRIC_MAX );
    pRec->pszFlag   = (PSZ) malloc( SZFLAGS_MAXZ );
    pRec->pszValue  = (PSZ) malloc( SZMETRICVAL_MAX );
    pRec->fb        = pFA->unimbr.fbFullFacename;
    strcpy( pRec->pszMetric, "Full face name");
    switch ( pFA->unimbr.fbFullFacename ) {
        case 0:                 strcpy( pRec->pszFlag, "");                       break;
        case ASSOC_EXACT_MATCH: strcpy( pRec->pszFlag, "Must match");             break;
        case ASSOC_DONT_CARE:   strcpy( pRec->pszFlag, "Don't care");             break;
        case ASSOC_USE_PARENT:  strcpy( pRec->pszFlag, "Use parent information"); break;
        default:                sprintf( pRec->pszFlag, "0x%08X",
                                         pFA->unimbr.fbFullFacename );
                                break;
    }
    strncpy( pRec->pszValue, pFA->unifm.szFullFacename, SZMETRICVAL_MAX-1 );
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
            EmboldenWindowText( WinWindowFromID( hwnd, IDD_ABOUTNAME ), hwnd );
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
                   cbRead;      // number of bytes read by DosRead
    PBYTE          pBuffer;     // raw buffer containing file contents
    APIRET         rc;          // return code from Dos**


    // Close any currently-open file and free its contents
    if ( pGlobal->hFile )
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

    // Verify the file format and parse accordingly
    pSig = (PGENERICRECORD) pBuffer;
    if (( pSig->Identity == SIG_CBFS ) &&
        ( pSig->ulSize == sizeof( COMBFONTSIGNATURE )))
    {
        // Combined font
        if ( ! ParseFont_CMB( pSig, pGlobal )) {
            sprintf( szError, "Failed to allocate memory for reading file (\"%s\").",
                     ((PCOMBFONTSIGNATURE)pSig)->szSignature );
            WinMessageBox( HWND_DESKTOP, hwnd, szError, "File Read Error",
                           0, MB_MOVEABLE | MB_OK | MB_ERROR );
            goto finish;
        }
        pGlobal->cbFile = fs3.cbFile;
        strncpy( pGlobal->szCurrentFile, pszFile, CCHMAXPATH );
    }
    else if (( pSig->Identity == SIG_ABRS ) &&
        ( pSig->ulSize == sizeof( ABRFILESIGNATURE )))
    {
        // Associated bitmap rules file
        if ( ! ParseFont_ABR( pSig, pGlobal )) {
            sprintf( szError, "Failed to allocate memory for reading file (\"%s\").",
                     ((PABRFILESIGNATURE)pSig)->szSignature );
            WinMessageBox( HWND_DESKTOP, hwnd, szError, "File Read Error",
                           0, MB_MOVEABLE | MB_OK | MB_ERROR );
            goto finish;
        }
        pGlobal->cbFile = fs3.cbFile;
        strncpy( pGlobal->szCurrentFile, pszFile, CCHMAXPATH );
    }
    else if (( pSig->Identity == SIG_UNFD ) &&
        ( pSig->ulSize == sizeof( UNIFONTDIRECTORY )))
    {
        // Uni-font
        if ( ! ParseFont_UNI( pSig, pGlobal )) {
            sprintf( szError, "Failed to allocate memory for reading file (\"%s\").",
                     ((PUNIFONTSIGNATURE)pSig)->szSignature );
            WinMessageBox( HWND_DESKTOP, hwnd, szError, "File Read Error",
                           0, MB_MOVEABLE | MB_OK | MB_ERROR );
            goto finish;
        }
        pGlobal->cbFile = fs3.cbFile;
        strncpy( pGlobal->szCurrentFile, pszFile, CCHMAXPATH );
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
    ShowFileName( pGlobal );
    if ( pBuffer ) DosFreeMem( pBuffer );
    return pGlobal->usType;
}


/* ------------------------------------------------------------------------- *
 * ShowFileName                                                              *
 *                                                                           *
 * Adds the current working filename to the window title.                    *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PCFEGLOBAL pGlobal: pointer to global program data.                     *
 *                                                                           *
 * RETURNS: N/AL                                                             *
 * ------------------------------------------------------------------------- */
void ShowFileName( PCFEGLOBAL pGlobal )
{
    CHAR achTitle[ _MAX_PATH + 40 ];
    CHAR achName[ _MAX_FNAME ];
    CHAR achExt[ _MAX_EXT ];

    if ( pGlobal->usType && pGlobal->szCurrentFile[0] ) {
        _splitpath( pGlobal->szCurrentFile, NULL, NULL, achName, achExt );
        sprintf( achTitle, "%.36s - %s%s", SZ_PROGRAM_TITLE, achName, achExt );
    }
    else {
        sprintf( achTitle, "%.248s - New", SZ_PROGRAM_TITLE );
    }

    WinSetWindowText( pGlobal->hwndMain, achTitle );

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
 * wrap_free                                                                 *
 *                                                                           *
 * Wrapper to free() which checks the pointer is valid and sets it to NULL   *
 * after freeing.                                                            *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PVOID address: Pointer to storage being freed.                      (I) *
 *                                                                           *
 * RETURNS: N/AL                                                             *
 * ------------------------------------------------------------------------- */
void wrap_free( void **address )
{
    if ( *address != NULL ) {
        free( *address );
        *address = NULL;
    }
}

