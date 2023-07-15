/*****************************************************************************
 * combined.c                                                                *
 *                                                                           *
 * Functions specific to the Combined Font format.                           *
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
#include "compfont.h"                       // program definitions


/* ------------------------------------------------------------------------- *
 * ParseFont_CMB                                                             *
 *                                                                           *
 * Opens and parses a combined font file.                                    *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PGENERICRECORD pStart: pointer to the start of the font                 *
 *   PCFEGLOBAL pGlobal: pointer to global program data.                     *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE on success, FALSE if an error occurred.                            *
 * ------------------------------------------------------------------------- */
BOOL ParseFont_CMB( PGENERICRECORD pStart, PCFEGLOBAL pGlobal )
{
    ULONG cbCompArray;      // Size of COMPFONTS array minus the first entry
    // These four pointers are offsets into the file data as read from disk
    PCOMBFONTSIGNATURE pFileSig;
    PCOMBFONTMETRICS   pFileMetrics;
    PCOMPFONTHEADER    pFileComponents;
    PCOMBFONTEND       pFileEnd;
    // This is our internal working copy of the font data (in pGlobal)
    PCOMBFONTFILE      pWorkingFont;


    // Get pointers to the various parts of the font data

    pFileSig        = (PCOMBFONTSIGNATURE) pStart;
    pFileMetrics    = (PCOMBFONTMETRICS)( (PBYTE) pStart + pFileSig->ulSize );
    pFileComponents = (PCOMPFONTHEADER)( (PBYTE) pFileMetrics + pFileMetrics->ulSize );
    cbCompArray     = pFileComponents->ulCmpFonts ?
                        (( pFileComponents->ulCmpFonts - 1 ) * sizeof( COMPFONT )):
                        0;
    pFileEnd        = (PCOMBFONTEND)( (PBYTE) pFileComponents + pFileComponents->ulSize + cbCompArray );

    pWorkingFont = &(pGlobal->font.combined);

    // Allocate separate new buffers for each portion of the file
    // so we can more easily add or remove parts later

#if 0
    pWorkingFont->pSignature = (PCOMBFONTSIGNATURE) malloc( pFileSig->ulSize );
    if ( !pWorkingFont->pSignature )
        return FALSE;
    pWorkingFont->pMetrics = (PCOMBFONTMETRICS) malloc( pFileMetrics->ulSize );
    if ( !pWorkingFont->pMetrics ) {
        wrap_free( (PPVOID) &(pWorkingFont->pSignature) );
        return FALSE;
    }
    ComponentListInit( pWorkingFont, pFileComponents );
    /*
    pWorkingFont->pComponents = (PCOMPFONTHEADER) malloc( pFileComponents->ulSize + cbCompArray );
    if ( !pWorkingFont->pComponents ) {
        wrap_free( (PPVOID) &(pWorkingFont->pSignature) );
        wrap_free( (PPVOID) &(pWorkingFont->pMetrics) );
        return FALSE;
    }
    */
    pWorkingFont->pEnd = (PCOMBFONTEND) malloc( pFileEnd->ulSize );
    if ( !pWorkingFont->pEnd ) {
        wrap_free( (PPVOID) &(pWorkingFont->pSignature) );
        wrap_free( (PPVOID) &(pWorkingFont->pMetrics) );
        wrap_free( (PPVOID) &(pWorkingFont->pComponents) );
        return FALSE;
    }
    memcpy( pWorkingFont->pSignature, pFileSig, pFileSig->ulSize );
    memcpy( pWorkingFont->pMetrics, pFileMetrics, pFileMetrics->ulSize );
    /*
    memcpy( pWorkingFont->pComponents, pFileComponents,
            pFileComponents->ulSize + cbCompArray );
    */
    memcpy( pWorkingFont->pEnd, pFileEnd, pFileEnd->ulSize );
#else
    if ( InitFontStructure_CMB( pGlobal, pFileSig->ulSize,
                                pFileMetrics->ulSize, pFileEnd->ulSize ) &&
         ComponentListInit( pWorkingFont, pFileComponents ))
    {
        // Create the font components linked list
        /*
        pWorkingFont->pComponents = (PCOMPFONTHEADER) malloc( pFileComponents->ulSize + cbCompArray );
        if ( !pWorkingFont->pComponents ) {
            wrap_free( (PPVOID) &(pWorkingFont->pSignature) );
            wrap_free( (PPVOID) &(pWorkingFont->pMetrics) );
            wrap_free( (PPVOID) &(pWorkingFont->pEnd) );
            return FALSE;
        }
        */
        memcpy( pWorkingFont->pSignature, pFileSig, pFileSig->ulSize );
        memcpy( pWorkingFont->pMetrics, pFileMetrics, pFileMetrics->ulSize );
        memcpy( pWorkingFont->pEnd, pFileEnd, pFileEnd->ulSize );
    }
    else {
        wrap_free( (PPVOID) &(pWorkingFont->pSignature) );
        wrap_free( (PPVOID) &(pWorkingFont->pMetrics) );
        wrap_free( (PPVOID) &(pWorkingFont->pEnd) );
        return FALSE;
    }

#endif

    pGlobal->usType = FONT_TYPE_CMB;

    return TRUE;
}


/* ------------------------------------------------------------------------- *
 * InitFontStructure_CMB                                                     *
 *                                                                           *
 * Allocates the global font data structures for a Combined font, except for *
 * the component font linked list (which is left empty for now).             *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PGENERICRECORD pStart: pointer to the start of the font                 *
 *   PCFEGLOBAL pGlobal: pointer to global program data.                     *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE on success, FALSE if an error occurred.                            *
 * ------------------------------------------------------------------------- */
BOOL InitFontStructure_CMB( PCFEGLOBAL pGlobal, ULONG cbSig, ULONG cbMetrics, ULONG cbEnd )
{
    PCOMBFONTFILE pFontData;

    pFontData = &(pGlobal->font.combined);
    pFontData->pSignature = (PCOMBFONTSIGNATURE) calloc( cbSig, 1 );
    if ( !pFontData->pSignature )
        return FALSE;

    pFontData->pMetrics = (PCOMBFONTMETRICS) calloc( cbMetrics, 1 );
    if ( !pFontData->pMetrics ) {
        wrap_free( (PPVOID) &(pFontData->pSignature) );
        return FALSE;
    }

    pFontData->pEnd = (PCOMBFONTEND) calloc( cbEnd, 1 );
    if ( !pFontData->pEnd ) {
        wrap_free( (PPVOID) &(pFontData->pSignature) );
        wrap_free( (PPVOID) &(pFontData->pMetrics) );
        return FALSE;
    }

    return TRUE;
}


/* ------------------------------------------------------------------------- *
 * NewFont_CMB                                                               *
 *                                                                           *
 * Create a new, empty Combined font file.                                   *
 * ------------------------------------------------------------------------- */
BOOL NewFont_CMB( HWND hwnd, PCFEGLOBAL pGlobal )
{
    // Set up the UI for Combined font if it isn't already
    if ( pGlobal->usType != FONT_TYPE_CMB )
    {
        WinSendDlgItemMsg( hwnd, IDD_COMPONENTS,
                           CM_REMOVEDETAILFIELDINFO, MPVOID,
                           MPFROM2SHORT( 0, CMA_INVALIDATE | CMA_FREE ));
        pGlobal->usType = FONT_TYPE_CMB;
        SetupWindowCF( hwnd );
        SetupCnrCF( hwnd );
    }

    return InitFontStructure_CMB( pGlobal,
                                  sizeof(COMBFONTSIGNATURE),
                                  sizeof(COMBFONTMETRICS),
                                  sizeof(COMBFONTEND) );
}


/* ------------------------------------------------------------------------- *
 * SetupWindowCF                                                             *
 *                                                                           *
 * Set up the window UI controls for Combined font format.                   *
 * ------------------------------------------------------------------------- */
void SetupWindowCF( HWND hwnd )
{
    WinShowWindow( WinWindowFromID( hwnd, IDD_FACEGROUP ), TRUE );
    WinShowWindow( WinWindowFromID( hwnd, IDD_FACETEXT ),  TRUE );
    WinShowWindow( WinWindowFromID( hwnd, IDD_FACENAME ),  TRUE );
    WinShowWindow( WinWindowFromID( hwnd, ID_METRICS ),    TRUE );
}


/* ------------------------------------------------------------------------- *
 * CompFontDlgProc                                                           *
 *                                                                           *
 * Dialog procedure for the component font properties dialog.                *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY CompFontDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    static PCFPROPS       pProps;
    PFONTASSOCGLYPHRANGE  pRange;
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
            cnr.flWindowAttr  = CV_DETAIL | CA_DETAILSVIEWTITLES;
            cnr.pszCnrTitle   = "Component Metrics";
            cnr.cyLineSpacing = 1;
            WinSendMsg( hwndCnr, CM_SETCNRINFO, MPFROMP( &cnr ),
                        MPFROMLONG( CMA_FLWINDOWATTR | CMA_LINESPACING ));
            pFld = (PFIELDINFO) WinSendMsg( hwndCnr,
                                            CM_ALLOCDETAILFIELDINFO,
                                            MPFROMLONG( 3L ), 0 );
            pFld1st = pFld;
            // (first column: font metric item name)
            pFld->cb = sizeof( FIELDINFO );
            pFld->pTitleData = "Metric";
            pFld->flData     = CFA_STRING | CFA_FIREADONLY | CFA_VCENTER | CFA_HORZSEPARATOR | CFA_SEPARATOR;
            pFld->offStruct  = FIELDOFFSET( CMRECORD, pszMetric );
            pFld = pFld->pNextFieldInfo;
            // (second column: font metric flag value)
            pFld->cb = sizeof( FIELDINFO );
            pFld->pTitleData = "Criterion";
            pFld->flData     = CFA_STRING | CFA_FIREADONLY | CFA_VCENTER | CFA_HORZSEPARATOR | CFA_SEPARATOR;
            pFld->offStruct  = FIELDOFFSET( CMRECORD, pszFlag );
            pFld = pFld->pNextFieldInfo;
            // (third column: font metric item value)
            pFld->cb = sizeof( FIELDINFO );
            pFld->pTitleData = "Value";
            pFld->flData     = CFA_STRING | CFA_FIREADONLY | CFA_VCENTER | CFA_HORZSEPARATOR;
            pFld->offStruct  = FIELDOFFSET( CMRECORD, pszValue );

            fi.cb                   = (ULONG) sizeof( FIELDINFOINSERT );
            fi.pFieldInfoOrder      = (PFIELDINFO) CMA_END;
            fi.fInvalidateFieldInfo = TRUE;
            fi.cFieldInfoInsert     = 3;
            WinSendMsg( hwndCnr, CM_INSERTDETAILFIELDINFO,
                        MPFROMP( pFld1st ), MPFROMP( &fi ));
            PopulateMetricFlags( hwndCnr, &(pProps->pCFA->font) );

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

            sIdx = pProps->pCFA->font.flFlags ? ((SHORT) pProps->pCFA->font.flFlags - 1) : 4;
            if ( sIdx <= 4 ) {
                WinSendDlgItemMsg( hwnd, IDD_SCALING, LM_SELECTITEM,
                                   MPFROMSHORT( sIdx ), MPFROMSHORT( TRUE ));
            }
            else {
                sprintf( szText, "0x%08x", pProps->pCFA->font.flFlags );
                WinSetDlgItemText( hwnd, IDD_SCALING, szText );
            }

            // Populate the current font's values
            if ( pProps->fEditExisting ) {
                for ( i = 0; i < pProps->pCFA->font.ulGlyphRanges; i++ ) {
                    pRange = (PFONTASSOCGLYPHRANGE) gl_list_at( pProps->pCFA->pRangeList, i );
                    if ( pRange ) {
                        sprintf( szText, SZ_GLYPHRANGE, pRange->giStart, pRange->giEnd, pRange->giTarget );
                        WinSendDlgItemMsg( hwnd, IDD_RANGES, LM_INSERTITEM,
                                           MPFROMSHORT( LIT_END ), MPFROMP( szText ));
                    }
                }
                WinSetDlgItemText( hwnd, IDD_FONTNAME,
                                   ( pProps->pCFA->font.unifm.flOptions & UNIFONTMETRICS_FULLFACENAME_EXIST ) ?
                                     pProps->pCFA->font.unifm.szFullFacename :
                                     pProps->pCFA->font.unifm.ifiMetrics.szFacename );
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
                    /*
                    if ( !pProps->fEditExisting ) {
                        pProps->pCFA->font.Identity = SIG_FTAS;
                        pProps->pCFA->font.ulSize   = sizeof( FONTASSOCIATION ); // (?)
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
                    */
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
                free( pRec->pszValue );
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
    ASSOCIATIONDATA ftas = {0};

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
/*
    PFONTASSOCLIST   pNode;
    PFONTASSOCIATION pAssocCopy,    // a working copy of the current association
                     pAssocOrig;    // pointer to the original font association
    ULONG            cbFA;
    ULONG            i;

    pNode = pGlobal->font.combined.pFontList;
    for ( i = 0; pNode->pNext && (i < ulAssoc); i++ )
        pNode = pNode->pNext;
    if ( i < ulAssoc ) return;  // error, bad index
    pAssocOrig = &(pNode->font);
    cbFA = sizeof( FONTASSOCIATION1 ) +
           ( pAssocOrig->ulGlyphRanges * sizeof( FONTASSOCGLYPHRANGE )) -
           sizeof( FONTASSOCGLYPHRANGE );

    if (( pAssocCopy = (PFONTASSOCIATION) malloc( cbFA )) == NULL ) {
        ErrorPopup("Memory allocation error.");
        return;
    }
    memcpy( pAssocCopy, pAssocOrig, cbFA );
    cfprops.pCFA = pAssocCopy;
*/

    CFPROPS          cfprops = {0};
    PASSOCIATIONDATA pAssociation;  // pointer to the font association data

    pAssociation = gl_list_at( pGlobal->font.combined.pFontList, ulAssoc );
    if ( pAssociation == NULL ) {
        ErrorPopup("Invalid font association.");
        return;
    }

    cfprops.cb = sizeof( CFPROPS );
    cfprops.hab = pGlobal->hab;
    cfprops.hmq = pGlobal->hmq;
    cfprops.hwndMain = hwnd;
    cfprops.fEditExisting = TRUE;
    cfprops.pCFA = pAssociation;
    WinDlgBox( HWND_DESKTOP, hwnd, (PFNWP) CompFontDlgProc,
               NULLHANDLE, IDD_COMPFONT, &cfprops );
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
    PUNIFONTMETRICS pUFM;           // pointer to font metrics data
//    PFONTASSOCLIST  pNode;     // current node in font association linked list
    PASSOCIATIONDATA pAssociation;  // pointer to a component font association
    PFONTASSOCGLYPHRANGE pRange;
//    PULONG          pulID;          // pointer to unifm ID field
    PSZ             pszFamily,
                    pszFace;
    ULONG           ulCB,
                    i;
    HWND            hwndCnr;
    PCFRECORD       pRec,
                    pFirst;
    RECORDINSERT    ri;
    SHORT           sIdx;
    CHAR            szNum[ 5 ];


    pUFM   = &(pGlobal->font.combined.pMetrics->unifm);
    if (( pGlobal->font.combined.pMetrics->Identity == SIG_CBFM ) &&
        ( pUFM->Identity == SIG_UNFM ))
    {

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
                                       MPFROMLONG( pGlobal->font.combined.ulCmpFonts ));
        pFirst = pRec;
        ulCB = sizeof( MINIRECORDCORE );

//        for ( i = 0, pNode = pGlobal->font.combined.pFontList; pNode; i++ ) {
        for ( i = 0; i < pGlobal->font.combined.ulCmpFonts; i++ ) {
            pAssociation = (PASSOCIATIONDATA) gl_list_at( pGlobal->font.combined.pFontList, i );
            if ( !pAssociation ) continue;

            pRec->pszFace      = (PSZ) calloc( FACESIZE, 1 );
            pRec->pszRanges    = (PSZ) calloc( SZRANGES_MAXZ, 1 );
            pRec->pszGlyphList = (PSZ) calloc( GLYPHNAMESIZE, 1 );
            pRec->pszFlags     = (PSZ) calloc( SZFLAGS_MAXZ, 1 );
            strcpy( pRec->pszFace, pAssociation->font.unifm.ifiMetrics.szFacename );
            if ( pAssociation->font.ulGlyphRanges > 1 )
                sprintf( pRec->pszRanges, "(%u ranges)", pAssociation->font.ulGlyphRanges );
            else if ( pAssociation->font.ulGlyphRanges == 1 ) {
                pRange = (PFONTASSOCGLYPHRANGE) gl_list_at( pAssociation->pRangeList, 0 );
                if ( pRange )
                    sprintf( pRec->pszRanges, "%u - %u", pRange->giStart, pRange->giEnd );
                else
                    strcpy( pRec->pszRanges, "(error)");
            }
            else
                strcpy( pRec->pszRanges, "*");
            strcpy( pRec->pszGlyphList, pAssociation->font.unifm.ifiMetrics.szGlyphlistName );
            sprintf( pRec->pszFlags, "0x%X", pAssociation->font.flFlags );
            pRec->record.cb = ulCB;
            pRec->record.pszIcon = pRec->pszFace;
            pRec->ulIndex = i;
            pRec = (PCFRECORD) pRec->record.preccNextRecord;
            //pNode = pNode->pNext;
        }
        ri.cb                = sizeof( RECORDINSERT );
        ri.pRecordOrder      = (PRECORDCORE) CMA_END;
        ri.pRecordParent     = NULL;
        ri.zOrder            = (ULONG) CMA_TOP;
        ri.fInvalidateRecord = TRUE;
        ri.cRecordsInsert    = pGlobal->font.combined.ulCmpFonts;
        WinSendMsg( hwndCnr, CM_INSERTRECORD, MPFROMP( pFirst ), MPFROMP( &ri ));
    }
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
    pFld->pTitleData = "Glyphs Included";
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
 * ComponentListFree                                                         *
 *                                                                           *
 * Free the linked list of component font associations.  This also frees     *
 * each association's linked list of glyph ranges, if any.                   *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PCOMBFONTFILE pCombFont: pointer to combined font data.                 *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void ComponentListFree( PCOMBFONTFILE pCombFont )
{
    PASSOCIATIONDATA pAssociation;

    if ( pCombFont->pFontList != NULL ) {
        do {
            pAssociation = gl_list_pop( pCombFont->pFontList );
            if ( pAssociation ) {
                if ( pAssociation->pRangeList ) {
                    GlyphRangeListFree( pAssociation );
                    pAssociation->pRangeList = NULL;
                }
                free( pAssociation );
            }
        } while ( pAssociation );
        gl_list_free( pCombFont->pFontList );
    }
    pCombFont->pFontList = NULL;
    pCombFont->ulCmpFonts = 0;
}


/* ------------------------------------------------------------------------- *
 * ComponentListInit                                                         *
 *                                                                           *
 * Copy the array of component font associations as parsed from a combined   *
 * font file into a linked list which is easier for us to manage internally. *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PCOMBFONTFILE   pCombFont  : our internal combined font representation. *
 *   PCOMPFONTHEADER pComponents: original array of components being copied. *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 * ------------------------------------------------------------------------- */
BOOL ComponentListInit( PCOMBFONTFILE pCombFont, PCOMPFONTHEADER pComponents )
{
    PFONTASSOCIATION pFA;           // a component font item as parsed from file
    PASSOCIATIONDATA pAssociation;  // our internal font association data
    BOOL             bOK = FALSE;
    ULONG            i;

    if ( pCombFont->pFontList != NULL )
         ComponentListFree( pCombFont );
    pCombFont->pFontList = gl_list_new();
    if ( !pCombFont->pFontList ) {
        ErrorPopup("Failed to allocate memory for component font list.");
        return FALSE;
    }

    for ( i = 0; i < pComponents->ulCmpFonts; i++ ) {
        pFA = &(pComponents->CompFont[ i ].CompFontAssoc);
        pAssociation = (PASSOCIATIONDATA) calloc( sizeof(ASSOCIATIONDATA), 1 );
        if ( !pAssociation ) {
            ErrorPopup("Failed to allocate memory for component font structure.");
            break;
        }
        // Using sizeof(FONTASSOCIATION1) lets us skip the range array...
        memcpy( &(pAssociation->font), pFA, sizeof(FONTASSOCIATION1) );

        // ...which we handle separately here
        if ( pAssociation->font.ulGlyphRanges ) {
            GlyphRangeListInit( pAssociation, pFA );
        }
        if ( gl_list_append( pCombFont->pFontList, pAssociation )) {
            pCombFont->ulCmpFonts++;
            bOK = TRUE;     // at least one item was successfully saved
        }
    }

    return bOK;
}


/* ------------------------------------------------------------------------- *
 * ComponentListInsert                                                       *
 *                                                                           *
 * Insert a new font association into the linked list.                       *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PCOMBFONTFILE    pCombFont   :    current combined font data.           *
 *   PASSOCIATIONDATA pAssociation: the new font association to add.         *
 *   ULONG            ulIndex     : list index of the newly added item.      *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 * ------------------------------------------------------------------------- */
BOOL ComponentListInsert( PCOMBFONTFILE pCombFont, PFONTASSOCIATION pAssociation, ULONG ulIndex )
{
    BOOL bOK = FALSE;

    if ( pCombFont->pFontList &&
         gl_list_insert( pCombFont->pFontList, pAssociation, ulIndex ))
    {
        pCombFont->ulCmpFonts++;
        bOK = TRUE;
    }
    return bOK;
}


/* ------------------------------------------------------------------------- *
 * ComponentListDelete                                                       *
 *                                                                           *
 * Delete a font association from a combined font's linked list.             *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PCOMBFONTFILE pCombFont: pointer to combined font data.                 *
 *   ULONG         ulIndex  : list index of the item to delete.              *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void ComponentListDelete( PCOMBFONTFILE pCombFont, ULONG ulIndex )
{
    if ( pCombFont->pFontList &&
         gl_list_delete( pCombFont->pFontList, ulIndex ))
    {
        pCombFont->ulCmpFonts--;
    }
}


/* ------------------------------------------------------------------------- *
 * GlyphRangeListInit                                                        *
 *                                                                           *
 * Copy the array of glyph range structures as parsed from a combined font   *
 * association structure into an internally-managed linked list.             *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PASSOCIATIONDATA pAssociation: internal font association data           *
 *   PFONTASSOCIATION pFA         : parsed font association                  *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 * ------------------------------------------------------------------------- */
BOOL GlyphRangeListInit( PASSOCIATIONDATA pAssociation, PFONTASSOCIATION pFA )
{
    PFONTASSOCGLYPHRANGE pRangeInternal;    // copy of the above for our internal data
    BOOL                 bOK = FALSE;
    ULONG                i;

    if ( pAssociation->pRangeList != NULL )
        GlyphRangeListFree( pAssociation );
    pAssociation->pRangeList = gl_list_new();
    if ( !pAssociation->pRangeList ) {
        ErrorPopup("Failed to allocate memory for glyph range list.");
        return FALSE;
    }

    pAssociation->font.ulGlyphRanges = 0;
    for ( i = 0; i < pFA->ulGlyphRanges; i++ ) {
        pRangeInternal = (PFONTASSOCGLYPHRANGE) malloc( sizeof(FONTASSOCGLYPHRANGE) );
        if ( !pRangeInternal ) {
            ErrorPopup("Failed to allocate memory for glyph range structure.");
            break;
        }
        memcpy( pRangeInternal, &(pFA->GlyphRange[i]), sizeof(FONTASSOCGLYPHRANGE) );
        if ( gl_list_append( pAssociation->pRangeList, pRangeInternal )) {
            pAssociation->font.ulGlyphRanges++;
            bOK = TRUE;     // at least one item was successfully saved
        }
    }
    return bOK;
}


/* ------------------------------------------------------------------------- *
 * GlyphRangeListFree                                                        *
 *                                                                           *
 * Free the linked list of glyph ranges associated with a font association.  *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PASSOCIATIONDATA pAssociation: pointer to font association data.        *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void GlyphRangeListFree( PASSOCIATIONDATA pAssociation )
{
    PFONTASSOCGLYPHRANGE pRange;

    if ( pAssociation->pRangeList != NULL ) {
        do {
            pRange = (PFONTASSOCGLYPHRANGE) gl_list_pop( pAssociation->pRangeList );
            wrap_free( (PPVOID) &pRange );
        } while ( pRange );
        gl_list_free( pAssociation->pRangeList );
    }
    pAssociation->font.ulGlyphRanges = 0;
}


