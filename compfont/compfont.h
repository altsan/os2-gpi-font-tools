/*****************************************************************************
 * compfont.h                                                                *
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


/*****************************************************************************
 ** DEFINITIONS                                                             **
 *****************************************************************************/

// Handy message box for errors and debug messages
#define ErrorPopup( text ) \
    WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, text, "Error", 0, MB_OK | MB_ERROR )

// ----------------------------------------------------------------------------
// CONSTANTS

#define SZ_PROGRAM_TITLE    "Composite Font Editor"


/* Font family class and subclass definitions.
 */
#define FF_CLASS_0          "0 - No Classification"
#define FF_CLASS_1          "1 - Old Style Serifs"
#define FF_CLASS_2          "2 - Transitional Serifs"
#define FF_CLASS_3          "3 - Modern Serifs"
#define FF_CLASS_4          "4 - Clarendon Serifs"
#define FF_CLASS_5          "5 - Slab Serifs"
#define FF_CLASS_7          "7 - Free Form Serifs"
#define FF_CLASS_8          "8 - Sans Serif"
#define FF_CLASS_9          "9 - Ornamentals"
#define FF_CLASS_10         "10 - Scripts"
#define FF_CLASS_12         "12 - Symbolic"

#define FF_SUBCLASS_0       "0 - No Classification"
#define FF_SUBCLASS_15      "15 - Miscellaneous"

#define FF_SUBCLASS_1_1     "1 - IBM rounded legibility"
#define FF_SUBCLASS_1_2     "2 - Garalde"
#define FF_SUBCLASS_1_3     "3 - Venetian"
#define FF_SUBCLASS_1_4     "4 - Modified Venetian"
#define FF_SUBCLASS_1_5     "5 - Dutch modern"
#define FF_SUBCLASS_1_6     "6 - Dutch traditional"
#define FF_SUBCLASS_1_7     "7 - Contemporary"
#define FF_SUBCLASS_1_8     "8 - Calligraphic"

#define FF_SUBCLASS_2_1     "1 - Direct line"
#define FF_SUBCLASS_2_2     "2 - Scripts"

#define FF_SUBCLASS_3_1     "1 - Italian"
#define FF_SUBCLASS_3_2     "2 - Scripts"

#define FF_SUBCLASS_4_1     "1 - Clarendon"
#define FF_SUBCLASS_4_2     "2 - Modern"
#define FF_SUBCLASS_4_3     "3 - Traditional"
#define FF_SUBCLASS_4_4     "4 - Newspaper"
#define FF_SUBCLASS_4_5     "5 - Stub serif"
#define FF_SUBCLASS_4_6     "6 - Monotone"
#define FF_SUBCLASS_4_7     "7 - Typewriter"

#define FF_SUBCLASS_5_1     "1 - Monotone"
#define FF_SUBCLASS_5_2     "2 - Humanist"
#define FF_SUBCLASS_5_3     "3 - Geometric"
#define FF_SUBCLASS_5_4     "4 - Swiss"
#define FF_SUBCLASS_5_5     "5 - Typewriter"

#define FF_SUBCLASS_7_1     "1 - Modern"

#define FF_SUBCLASS_8_1     "1 - IBM neo-grotesque gothic"
#define FF_SUBCLASS_8_2     "2 - Humanist"
#define FF_SUBCLASS_8_3     "3 - Low-x round geometric"
#define FF_SUBCLASS_8_4     "4 - High-x round geometric"
#define FF_SUBCLASS_8_5     "5 - Neo-grotesque gothic"
#define FF_SUBCLASS_8_6     "6 - Modified neo-grotesque gothic"
#define FF_SUBCLASS_8_9     "9 - Typewriter"
#define FF_SUBCLASS_8_10    "10 - Matrix"

#define FF_SUBCLASS_9_1     "1 - Engraved"
#define FF_SUBCLASS_9_2     "2 - Black letter"
#define FF_SUBCLASS_9_3     "3 - Decorative"
#define FF_SUBCLASS_9_4     "4 - 3-D"

#define FF_SUBCLASS_10_1    "1 - Uncial"
#define FF_SUBCLASS_10_2    "2 - Brush joined"
#define FF_SUBCLASS_10_3    "3 - Formal joined"
#define FF_SUBCLASS_10_4    "4 - Monotone joined"
#define FF_SUBCLASS_10_5    "5 - Calligraphic"
#define FF_SUBCLASS_10_6    "6 - Brush unjoined"
#define FF_SUBCLASS_10_7    "7 - Formal unjoined"
#define FF_SUBCLASS_10_8    "8 - Monotone unjoined"

#define FF_SUBCLASS_12_3    "3 - Mixed serif"
#define FF_SUBCLASS_12_6    "6 - Old-style serif"
#define FF_SUBCLASS_12_7    "7 - Neo-grotesque sans serif"


/* Font weight and width class definitions
 */
#define FONT_WEIGHT_1       "1 - Ultra-light"
#define FONT_WEIGHT_2       "2 - Extra-light"
#define FONT_WEIGHT_3       "3 - Light"
#define FONT_WEIGHT_4       "4 - Semi-light"
#define FONT_WEIGHT_5       "5 - Medium (normal)"
#define FONT_WEIGHT_6       "6 - Semi-bold"
#define FONT_WEIGHT_7       "7 - Bold"
#define FONT_WEIGHT_8       "8 - Extra-bold"
#define FONT_WEIGHT_9       "9 - Ultra-bold"

#define FONT_WIDTH_1        "1 - Ultra-condensed"
#define FONT_WIDTH_2        "2 - Extra-condensed"
#define FONT_WIDTH_3        "3 - Condensed"
#define FONT_WIDTH_4        "4 - Semi-condensed"
#define FONT_WIDTH_5        "5 - Medium (normal)"
#define FONT_WIDTH_6        "6 - Semi-expanded"
#define FONT_WIDTH_7        "7 - Expanded"
#define FONT_WIDTH_8        "8 - Extra-expanded"
#define FONT_WIDTH_9        "9 - Ultra-expanded"


/* Format of an entry in the glyph ranges list.
 */
#define SZ_GLYPHRANGE       "%u - %u   (mapped to offset %u)"

/* Various string buffer lengths.
 */
#define SZERR_MAXZ                  255     // length of an error string
#define SZRANGES_MAXZ               100     // length of a glyph-ranges string
#define SZFLAGS_MAXZ                50      // length of scaling-flags string
#define SZMETRIC_MAX                50      // length of a metric name
#define SZMETRICVAL_MAX             100     // length of a metric value

/* Encoding type for bEncoding field in GRDATA structure
 */
#define ENC_PM                      0
#define ENC_UNICODE                 1
#define ENC_SYMBOL                  9

/* Values for usType field in CFEGLOBAL structure
 */
#define FONT_TYPE_CMB               1       // Combined font
#define FONT_TYPE_PCR               2       // (TBI) Pre-combined rule file
#define FONT_TYPE_ABR               3       // Associated bitmaps rule file
#define FONT_TYPE_UNI               4       // Uni-font (including font directory)
#define FONT_TYPE_UFF               5       // (TBI) Uni-font face (standalone resource)

/* Private window messages
 */
#define WM_READFILE                 (WM_USER+1)


// ----------------------------------------------------------------------------
// TYPEDEFS

#pragma pack(1)

// FONTASSOCIATION structure without the GlyphRange array.  It is otherwise
// identical to the FONTASSOCIATION structure.
typedef struct _FONTASSOCIATION1 {
    ULONG                Identity;        /* Must be 0x53415446 ("FTAS").   */
    ULONG                ulSize;
    UNIFONTMETRICSMEMBER unimbr;
    UNIFONTMETRICS       unifm;
    ULONG                ulGlyphRanges;
    ULONG                flFlags;
} FONTASSOCIATION1;
typedef FONTASSOCIATION1 *PFONTASSOCIATION1;

typedef struct _font_association_data {
    FONTASSOCIATION1 font;          // the current component font association
    PGLINKEDLIST     pRangeList;    // linked list of glyph ranges
} ASSOCIATIONDATA, *PASSOCIATIONDATA;

// Contains pointers to all the components of a combined font
// (Note: this does not quite reflect the actual file structure on disk, as we
// use a linked list of font associations instead of a single contiguous array)
typedef struct _cmb_font_data {
    PCOMBFONTSIGNATURE pSignature;      // pointer to the font signature block
    PCOMBFONTMETRICS   pMetrics;        // pointer to the font metrics block
    ULONG              ulCmpFonts;      // number of component fonts
    PGLINKEDLIST       pFontList;       // linked list of font component definitions
    PCOMBFONTEND       pEnd;            // pointer to the font end signature
} COMBFONTFILE, *PCOMBFONTFILE;


// Contains pointers to all the components of a pre-combine rule
// (same note as for the combined font structure, above)
typedef struct _pcr_file_data {
    PPRECOMBRULESIGNATURE  pSignature;    // pointer to the start of the font
    PFONTASSOCIATION       pSourceAssoc;  // pointer to the source font association structure
    PTARGETFONTASSOCHEADER pTargetHeader; // pointer to the target font association header
    PPRECOMBRULEEND        pEnd;          // pointer to the font end signature
    PGLINKEDLIST           pFontList;     // linked list of target font definitions
} PCRFILE, *PPCRFILE;


// Contains pointers to all the components of an ABR file
typedef struct _abr_file_data {
    PABRFILESIGNATURE  pSignature;      // pointer to the start of the font
    PFONTASSOCIATION   pAssociations;   // pointer to the array of font associations
    PABRFILEEND        pEnd;            // pointer to the font end signature
} ABRFILE, *PABRFILE;


// Combined structure for Uni-font character definition and bitmap data
typedef struct _unifont_char_data {
    union {
        UNICHARDEF1 type1;                  // type 1/2 character definition
        UNICHARDEF3 type3;                  // type 3 character definition
    } definition;
    ULONG cbBitmap;                         // size of the character bitmap
    PBYTE pBitmap;                          // pointer to bitmap data
} UNIFONTCHARACTER, *PUNIFONTCHARACTER;


// Data about a Uni-font resource
typedef struct _uni_font_data {
    PUNIFONTRESOURCE         pHeader;       // pointer to amalgamated header
    PUNIENDFONTRESOURCE      pEnd;          // pointer to font end signature
    ULONG                    ulKernPairs;   // number of pairs in the kerning table
    ULONG                    ulGroups;      // number of character groups in the font
    PGLINKEDLIST             pKerning;      // linked list of kerning pairs
    PGLINKEDLIST             pGroups;       // linked list of character group definitions
} UNIFONTFACE, *PUNIFONTFACE;


// Contains pointers to the contents of a Uni-font file, plus the internal
// data maintained when creating or modifying it.
typedef struct _uni_font_file_data {
    PUNIFONTDIRECTORY   pFontDir;       // pointer to the original font file
    PGLINKEDLIST        pFontList;      // linked list of font face resources
} UNIFONTFILE, *PUNIFONTFILE;


// Record structure for the Uni-font glyphs container
typedef struct _uni_font_ranges {
    MINIRECORDCORE record;                  // standard data (short version)
    PSZ            pszRanges,               // source glyph ranges field
                   pszTarget,               // target glyph offset field
                   pszDesc;                 // description field
    ULONG          ulIndex;                 // array index of this component
} UFRECORD, *PUFRECORD;


// Record structure for the component metrics container
typedef struct _metric_flags_record {
    MINIRECORDCORE record;                  // standard data (short version)
    PSZ            pszMetric,               // font face name field
                   pszFlag,                 // flags field (human-readable)
                   pszValue;                // metric value field
    BYTE           fb;                      // numeric flag value (not displayed)
} CMRECORD, *PCMRECORD;


// Record structure for the component-font container
typedef struct _font_info_record {
    MINIRECORDCORE record;                  // standard data (short version)
    PSZ            pszFace,                 // font face name field
                   pszRanges,               // associated glyph ranges field
                   pszGlyphList,            // glyph-list (encoding) field
                   pszFlags;                // flags field
    ULONG          ulIndex;                 // array index of this component
} CFRECORD, *PCFRECORD;


// Window data for component-font glyph range dialog
typedef struct _Glyph_Range_Data {
    USHORT              cb;                 // size of this structure
    HAB                 hab;                // anchor-block handle
    HMQ                 hmq;                // msg-queue handle
    HWND                hwndMain;           // handle of main program window
    BOOL                fEditExisting;      // we are editing/viewing an existing range
    USHORT              usFirstChar,        // first glyph in component font
                        usLastChar;         // last glyph in component font
    BYTE                bEncoding;          // glyphlist of component font
    FONTASSOCGLYPHRANGE range;              // range association structure
} GRDATA, *PGRDATA;


// Window data for component-font properties dialog
typedef struct _Comp_Font_Data {
    USHORT           cb;                    // size of this structure
    HAB              hab;                   // anchor-block handle
    HMQ              hmq;                   // msg-queue handle
    HWND             hwndMain;              // handle of main program window
    BOOL             fEditExisting;         // we are editing/viewing an existing component
    PASSOCIATIONDATA pCFA;                  // pointer to the current component-font association
} CFPROPS, *PCFPROPS;


// Window data for uni-font properties dialog
typedef struct _Uni_Font_Data {
    USHORT           cb;                    // size of this structure
    HAB              hab;                   // anchor-block handle
    HMQ              hmq;                   // msg-queue handle
    HWND             hwndMain;              // handle of main program window
    BOOL             fEditExisting;         // we are editing/viewing an existing component
    PUNIFONTRESOURCE pFontHeader;           // pointer to the current Unifont resource header
    PVOID            pFontData;             // pointer to the variable-format font data
} UFPROPS, *PUFPROPS;


// Data structure containing global data
typedef struct _Global_Data {
    USHORT  cb;                             // size of this structure
    HAB     hab;                            // anchor-block handle
    HMQ     hmq;                            // msg-queue handle
    HWND    hwndMain;                       // handle of main dialog window
    CHAR    szCurrentFile[ CCHMAXPATH+1 ];  // the file currently open
    BOOL    bModified;                      // has the current file been modified?
    HFILE   hFile;                          // handle of the current file
    ULONG   cbFile;                         // total size of the file in bytes
    USHORT  usType;                         // current font type
    union {
        COMBFONTFILE      combined;         // when FONT_TYPE_CMB
        PCRFILE           pcr;              // when FONT_TYPE_PCR
        ABRFILE           abr;              // when FONT_TYPE_ABR
        PUNIFONTDIRECTORY pUFontDir;        // when FONT_TYPE_UNI
        UNIFONTFACE       unifont;          // when FONT_TYPE_UFF
    } font;
} CFEGLOBAL, *PCFEGLOBAL;

#pragma pack()


// ----------------------------------------------------------------------------
// FUNCTION PROTOTYPES

// compfont.c
void             CentreWindow( HWND hwnd, HWND hwndRel );
void             CloseFontFile( HWND hwnd, PCFEGLOBAL pGlobal );
void             DeriveUniFontMetrics( PUNIFONTMETRICS pUFM, PFONTMETRICS pFM );
BOOL             EmboldenWindowText( HWND hwndCtl, HWND hwndUse );
LONG             GetCurrentDPI( HWND hwnd );
void             GlyphRangeDialog( HWND hwnd, PCFPROPS pCompFont, BOOL fEdit );
MRESULT EXPENTRY ImportDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
MRESULT EXPENTRY MainDialogProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
void             NewFontFile( HWND hwnd, PCFEGLOBAL pGlobal, USHORT usType );
MRESULT EXPENTRY ProductInfoDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
void             PopulateMetricFlags( HWND hwndCnr, PFONTASSOCIATION1 pFA );
MRESULT EXPENTRY RangeDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
USHORT           ReadFontFile( HWND hwnd, PSZ pszfile, PCFEGLOBAL pGlobal  );
BOOL             SelectInstalledFont( HWND hwnd, PSZ pszFacename );
void             ShowFileName( PCFEGLOBAL pGlobal );
void             wrap_free( void **address );


// combined.c
void             AddComponentFont( HWND hwnd, PCFEGLOBAL pGlobal );
MRESULT EXPENTRY CompFontDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
void             ComponentListDelete( PCOMBFONTFILE pCombFont, ULONG ulIndex );
void             ComponentListFree( PCOMBFONTFILE pCombFont );
BOOL             ComponentListInit( PCOMBFONTFILE pCombFont, PCOMPFONTHEADER pComponents );
BOOL             ComponentListInsert( PCOMBFONTFILE pCombFont, PFONTASSOCIATION pAssociation, ULONG ulIndex );
void             EditComponentFont( HWND hwnd, PCFEGLOBAL pGlobal, ULONG ulAssoc );
void             GlyphRangeListFree( PASSOCIATIONDATA pAssociation );
BOOL             GlyphRangeListInit( PASSOCIATIONDATA pAssociation, PFONTASSOCIATION pFA );
BOOL             InitFontStructure_CMB( PCFEGLOBAL pGlobal, ULONG cbSig, ULONG cbMetrics, ULONG cbEnd );
BOOL             NewFont_CMB( HWND hwnd, PCFEGLOBAL pGlobal );
BOOL             ParseFont_CMB( PGENERICRECORD pStart, PCFEGLOBAL pGlobal );
void             PopulateValues_CMB( HWND hwnd, PCFEGLOBAL pGlobal );
void             SetupCnrCF( HWND hwnd );
void             SetupWindowCF( HWND hwnd );


// abr.c
BOOL             ParseFont_ABR( PGENERICRECORD pStart, PCFEGLOBAL pGlobal );
void             PopulateValues_ABR( HWND hwnd, PCFEGLOBAL pGlobal );
void             SetupCnrAB( HWND hwnd );


// unifont.c
void             AddUniFont( HWND hwnd, PCFEGLOBAL pGlobal );
void             PopulateValues_UNI( HWND hwnd, PCFEGLOBAL pGlobal );
BOOL             NewFont_UNI( HWND hwnd, PCFEGLOBAL pGlobal );
BOOL             ParseFont_UNI( PGENERICRECORD pStart, PCFEGLOBAL pGlobal );
void             SetupCnrUF( HWND hwnd );
void             SetupWindowUF( HWND hwnd );
MRESULT EXPENTRY UniFontDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );

