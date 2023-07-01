/*****************************************************************************
 *                                                                           *
 *  gpifont.h                                                                *
 *                                                                           *
 *  Definitions for handling standard OS/2 GPI bitmap fonts.                 *
 *                                                                           *
 *  There are several OS/2-specific font formats, but the main one is the    *
 *  'standard' GPI format.  This is the format which is supported here,      *
 *  although some of the structures and routines may someday prove useful    *
 *  in implementing support for other types.                                 *
 *                                                                           *
 *  Those others include the extremely obscure Uni-font format (of which I   *
 *  have never found a single example), several related 'combined font'      *
 *  formats which are basically wrappers or aliases to other fonts, and the  *
 *  ultra-convoluted native DBCS fonts which are composed of standard GPI    *
 *  fonts in association with separately-loaded CJK glyph libraries (which   *
 *  themselves seem to support a few different formats but are mostly        *
 *  something from MS called WIFE).  But none of these concern us here.      *
 *                                                                           *
 *  The GPI documentation implies that the GPI font format may also contain  *
 *  outline data, but the format of such outlines is only alluded to.  In    *
 *  any event, I'm not aware of any such fonts actually existing (all        *
 *  outline fonts included with OS/2 are in TTF or Type 1 format).           *
 *                                                                           *
 *  (C) 2012 Alexander Taylor                                                *
 *                                                                           *
 *  This code is placed in the public domain.                                *
 *                                                                           *
 *****************************************************************************/

#ifndef __GPIFONT_H__
#define __GPIFONT_H__


// ----------------------------------------------------------------------------
// CONSTANTS

/* Text signatures for standard OS/2 bitmap fonts.
 */
#define OS2FNT_SIGNATURE    "OS/2 FONT"
#define OS2FNT2_SIGNATURE   "OS/2 FONT 2"

/* Binary signatures for various OS/2 font records.
 */
#define SIG_OS2FONTSTART    0xFFFFFFFE
#define SIG_OS2METRICS      0x1
#define SIG_OS2FONTDEF      0x2
#define SIG_OS2KERN         0x3
#define SIG_OS2ADDMETRICS   0x4
#define SIG_OS2FONTEND      0xFFFFFFFF

/* GPI categorizes fonts in 3 types according to character increment definition:
 * Type 1 : Fixed-width font
 * Type 2 : Proportional-width font with each character increment defined as
 *          one value in the character definition.
 * Type 3 : Proportional-width font with each character increment defined in
 *          terms of: a_space + b_space + c_space
 *             Where: a_space = leading space in front of the character
 *                    b_space = width of the character glyph
 *                    c_space = space following the character
 *
 * Each type is required to have specific values of flFontDef and flCharDef
 * in the font definition header.  These values are defined below.
 */
#define OS2FONTDEF_FONT1    0x47
#define OS2FONTDEF_FONT2    0x42
#define OS2FONTDEF_FONT3    0x42
#define OS2FONTDEF_CHAR1    0x81
#define OS2FONTDEF_CHAR2    0x81
#define OS2FONTDEF_CHAR3    0XB8

/* Font charset-support flags.
 */
#define FOCA_CHARSET_LATIN1         0x10
#define FOCA_CHARSET_PC             0x20
#define FOCA_CHARSET_LATINX         0x40
#define FOCA_CHARSET_CYRILLIC       0x80
#define FOCA_CHARSET_HEBREW         0x100
#define FOCA_CHARSET_GREEK          0x200
#define FOCA_CHARSET_ARABIC         0x400
#define FOCA_CHARSET_UGLEXT         0x800
#define FOCA_CHARSET_KANA           0x1000
#define FOCA_CHARSET_THAI           0x2000
#define FOCA_SELECTION_JAPAN        0x1000
#define FOCA_SELECTION_TAIWAN       0x2000
#define FOCA_SELECTION_CHINA        0x4000
#define FOCA_SELECTION_KOREA        0x8000

/* Error definitions.
 */
#define OS2FNT_ERR_BASE     0x0F00
#define ERR_FILE_OPEN       OS2FNT_ERR_BASE + 1
#define ERR_FILE_STAT       OS2FNT_ERR_BASE + 2
#define ERR_FILE_READ       OS2FNT_ERR_BASE + 3
#define ERR_FILE_FORMAT     OS2FNT_ERR_BASE + 4

#define ERR_NO_FONT         OS2FNT_ERR_BASE + 10

#define ERR_MEMORY          OS2FNT_ERR_BASE + 20


// ----------------------------------------------------------------------------
// TYPEDEFS

/* All structures must be byte-aligned.
 */
#pragma pack(1)


/* A generic font record structure, mostly used for typecasting.
 */
typedef struct _Generic_Font_Record {
    ULONG   Identity;                    /* structure identity code  */
    ULONG   ulSize;                      /* structure size in bytes  */
} GENERICRECORD, *PGENERICRECORD;


/* A reasonably generic structure used to store a glyph bitmap.  The first
 * four fields deliberately parallel those of the basic bitmap type
 * (FT_Bitmap) used by FreeType, in the hopes of making porting easier.  The
 * last two reflect horizontal glyph metrics (similar to those in the
 * FT_Glyph_Metrics type).
 *
 * Note that the horizontal advance (increment) is equivalent to
 *   a_space + b_space + c_space
 * for type 3 fonts.  (Type 1 & 2 fonts have no side-bearings, so in those
 * cases the advance will be the same as the bitmap width.)
 *
 * GPI fonts do not contain any information related to vertical text flow, so
 * the vertical side-bearing is derived from the font's external-leading value.
 */
typedef struct _Glyph_Bitmap {
    ULONG  rows;                    /* number of rows (== height in pels)   */
    ULONG  width;                   /* number of pels per row (== width)    */
    ULONG  pitch;                   /* number of bytes per row              */
    PUCHAR buffer;                  /* pointer to bitmap data               */
    ULONG  cbBuffer;                /* size of buffer in bytes              */
    SHORT  horiBearingX;            /* horizontal (left) side-bearing       */
    SHORT  horiAdvance;             /* horizontal advance (increment)       */
    SHORT  vertBearingY;            /* vertical (top) side-bearing          */
    SHORT  vertAdvance;             /* vertical advance (increment)         */
} GLYPHBITMAP, *PGLYPHBITMAP;


/* A font signature (header) block.
 */
typedef struct _OS2_Font_Header {
    ULONG   Identity;               /* 0xFFFFFFFE                    */
    ULONG   ulSize;                 /* size of this structure        */
    CHAR    achSignature[12];       /* string indicating font format */
} OS2FONTSTART, *POS2FONTSTART;


/* A font end signature block (identical to the generic record).  The
 * Identity field in this case should be 0xFFFFFFFF.
 */
typedef GENERICRECORD OS2FONTEND, *POS2FONTEND;


/* The font metrics structure ("IBM Font Object Content Architecture" metrics).
 *
 * The number of _reported_ glyphs in the raster font normally equals
 * usLastChar+1 (as this value is an index offset from usFirstChar).  Most
 * fonts actually have an additional glyph definition (and corresponding
 * bitmap) at the end, which isn't included in the count, for the so-called
 * .null character.  However (and the GPI documentation doesn't mention this),
 * some fonts have the .null character explicitly defined at index 0, in which
 * case there is no extra glyph at the end.
 */
typedef struct _OS2_FOCA_Metrics {
    ULONG   Identity;               /* 0x00000001                           */
    ULONG   ulSize;                 /* size of this structure               */
    CHAR    szFamilyname[32];       /* font family name (null-terminated)   */
    CHAR    szFacename[32];         /* font face name (null-terminated)     */
    SHORT   usRegistryId;           /* the registered font ID number        */
    SHORT   usCodePage;             /* font encoding (850 indicates PMUGL)  */
    SHORT   yEmHeight;              /* height of the em square              */
    SHORT   yXHeight;               /* lowercase x height                   */
    SHORT   yMaxAscender;           /* total cell height above baseline     */
    SHORT   yMaxDescender;          /* total cell depth below baseline      */
    SHORT   yLowerCaseAscent;       /* height (+) of a lowercase ascender   */
    SHORT   yLowerCaseDescent;      /* height (+) of a lowercase descender  */
    SHORT   yInternalLeading;       /* space above glyph used for accents   */
    SHORT   yExternalLeading;       /* extra linespace padding              */
    SHORT   xAveCharWidth;          /* average character width              */
    SHORT   xMaxCharInc;            /* maximum character increment (width)  */
    SHORT   xEmInc;                 /* width of the em square               */
    SHORT   yMaxBaselineExt;        /* sum of max ascender and descender    */
    SHORT   sCharSlope;             /* nominal character slope in degrees   */
    SHORT   sInlineDir;             /* inline text direction in degrees     */
    SHORT   sCharRot;               /* nominal character rotation angle     */
    USHORT  usWeightClass;          /* font weight class (1000-9000)        */
    USHORT  usWidthClass;           /* font width class (1000-9000)         */
    SHORT   xDeviceRes;             /* target horiz. resolution (dpi)       */
    SHORT   yDeviceRes;             /* target vert. resolution (dpi)        */
    SHORT   usFirstChar;            /* codepoint of the first character     */
    SHORT   usLastChar;             /* codepoint offset of the last char    */
    SHORT   usDefaultChar;          /* codepoint offset of the default char */
    SHORT   usBreakChar;            /* codepoint offset of the space char   */
    SHORT   usNominalPointSize;     /* font's point size multiplied by 10   */
    SHORT   usMinimumPointSize;     /* (same as above for bitmap fonts)     */
    SHORT   usMaximumPointSize;     /* (same as above for bitmap fonts)     */
    SHORT   fsTypeFlags;            /* font type flags                      */
    SHORT   fsDefn;                 /* font definition flags                */
    SHORT   fsSelectionFlags;       /* font selection flags                 */
    SHORT   fsCapabilities;         /* font capability flags                */
    SHORT   ySubscriptXSize;        /* width of subscript characters        */
    SHORT   ySubscriptYSize;        /* height of subscript characters       */
    SHORT   ySubscriptXOffset;      /* x-offset of subscript characters     */
    SHORT   ySubscriptYOffset;      /* y-offset of subscript characters     */
    SHORT   ySuperscriptXSize;      /* width of superscript characters      */
    SHORT   ySuperscriptYSize;      /* height of superscript characters     */
    SHORT   ySuperscriptXOffset;    /* x-offset of superscript characters   */
    SHORT   ySuperscriptYOffset;    /* y-offset of superscript characters   */
    SHORT   yUnderscoreSize;        /* underscore stroke thickness          */
    SHORT   yUnderscorePosition;    /* underscore depth below baseline      */
    SHORT   yStrikeoutSize;         /* strikeout stroke thickness           */
    SHORT   yStrikeoutPosition;     /* strikeout height above baseline      */
    SHORT   usKerningPairs;         /* number of kerning pairs defined      */
    SHORT   sFamilyClass;           /* IBM font family class and subclass   */
    ULONG   reserved;               /* device name address (not used)       */
} OS2FOCAMETRICS, *POS2FOCAMETRICS;


/* The font definition header.  Not all of these fields may be used, depending
 * on the font type (indicated by the flags in fsFontdef).
 * According to the GPI documentation, the ulSize field is supposed to contain
 * the size of this data structure. In fact, it actually contains the size of
 * the data structure _plus_ all the glyph data that follows.
 */
typedef struct _OS2_Font_Definition {
    ULONG Identity;             /* 0x00000002                               */
    ULONG ulSize;               /* size of character data (see note above)  */
    SHORT fsFontdef;            /* flags indicating which fields are used   */
    SHORT fsChardef;            /* flags indicating format of char defs     */
    SHORT usCellSize;           /* size (in bytes) of each char definition  */
    SHORT xCellWidth;           /* char cell-width in pixels (type 1 only)  */
    SHORT yCellHeight;          /* char cell-height in pixels               */
    SHORT xCellIncrement;       /* char increment in pixels (type 1 only)   */
    SHORT xCellA;               /* character a_space (type 3 only)          */
    SHORT xCellB;               /* character b_space (type 3 only)          */
    SHORT xCellC;               /* character c_space (type 3 only)          */
    SHORT pCellBaseOffset;      /* distance between baseline & top of cell  */
} OS2FONTDEFHEADER, *POS2FONTDEFHEADER;


/* The font kerning-pairs table.  This and the following structure are a bit
 * of a mystery.  They are defined here as the specification in the OS/2 GPI
 * programming guide describes them.  However, the specification also says
 * that the ulSize field should be 10 bytes (not 9), and doesn't describe at
 * all what the cFirstpair field does.  The KERNINGPAIRS structure is even more
 * problematic, as the standard GPI toolkit headers define a KERNINGPAIRS
 * structure which is different from what the documentation describes.
 *
 * Unfortunately, I cannot find any examples of fonts which actually contain
 * kerning information, and the IBM Font Editor doesn't support kerning at
 * all... so there's no way to analyze how it really works.  (It's entirely
 * possible that kerning support was never actually implemented.)
 *
 * The good news is that the kerning information is right at the end of the
 * font file (except for the PANOSE structure and the font-end signature),
 * so even if our kern-table parsing is flawed it shouldn't be fatal.
 */
typedef struct _OS2_Kerning_Table {
    ULONG   Identity;               /* 0x00000003       */
    ULONG   ulSize;                 /* must be 10 (??)  */
    CHAR    cFirstpair;             /* undocumented     */
} OS2KERNPAIRTABLE, *POS2KERNPAIRTABLE;

typedef struct _OS2_Kerning_Pair {
    SHORT   sFirstChar;             /* first character of pair  */
    SHORT   sSecondChar;            /* second character of pair */
    SHORT   sKerningAmount;         /* kerning amount           */
} OS2KERNINGPAIRS, *POS2KERNINGPAIRS;


/* The "additional metrics" structure contains the PANOSE table.
 */
typedef struct _OS2_Extra_Metrics {
    ULONG   Identity;               /* 0x00000004                       */
    ULONG   ulSize;                 /* structure size (20 bytes)        */
    UCHAR   panose[12];             /* PANOSE table padded to 12 bytes  */
} OS2ADDMETRICS, *POS2ADDMETRICS;


/* An individual character definition for a type 1 or 2 font.
 */
typedef struct _OS2_Character_1 {
    ULONG   ulOffset;           /* offset of glyph bitmap within the font */
    USHORT  ulWidth;            /* width of the glyph bitmap (in pixels)  */
} OS2CHARDEF1, *POS2CHARDEF1;

/* An individual character definition for a type 3 font.
 */
typedef struct _OS2_Character_3 {
    ULONG   ulOffset;           /* offset of glyph bitmap within the font */
    SHORT   aSpace;             /* character a_space (in pixels)          */
    SHORT   bSpace;             /* character b_space (in pixels)          */
    SHORT   cSpace;             /* character c_space (in pixels)          */
} OS2CHARDEF3, *POS2CHARDEF3;


/* A font directory (used in compiled fonts).
 */
typedef struct _OS2_FontDir_Entry {
    USHORT          usIndex;             /* The resource ID of the font   */
    OS2FOCAMETRICS  metrics;             /* The font's metrics            */
    UCHAR           panose[12];          /* The font's PANOSE data        */
} OS2FONTDIRENTRY, *POS2FONTDIRENTRY;

typedef struct _OS2_Font_Directory {
    USHORT          usHeaderSize;        /* The size of this header       */
    USHORT          usnFonts;            /* The number of fonts           */
    USHORT          usiMetrics;          /* The size of all the metrics   */
    OS2FONTDIRENTRY fntEntry[ 1 ];       /* Array of individual font info */
} OS2FONTDIRECTORY, *POS2FONTDIRECTORY;


/* Structure used to refer to the various parts of a font resource.  This is
 * used by most of the various functions to reference the font as a whole.
 */
typedef struct _OS2_Font_Resource {
    POS2FONTSTART       pSignature;    /* Pointer to the signature block    */
    POS2FOCAMETRICS     pMetrics;      /* Pointer to the font metrics       */
    POS2FONTDEFHEADER   pFontDef;      /* Pointer to the font definition    */
    union {
        POS2CHARDEF1    pChars;        /* Character data for type 1/2 fonts */
        POS2CHARDEF3    pABC;          /* Character data for type 3 fonts   */
    } data;
    POS2KERNPAIRTABLE   pKerning;      /* Pointer to kerning table          */
    POS2ADDMETRICS      pPanose;       /* Pointer to PANOSE table           */
    POS2FONTEND         pEnd;          /* Pointer to end-signature block    */
    ULONG               cbSize;        /* Total size of the font resource   */
} OS2FONTRESOURCE, *POS2FONTRESOURCE;


#pragma pack()


// ----------------------------------------------------------------------------
// FUNCTION PROTOTYPES

BOOL  ExtractOS2FontGlyph( ULONG ulOffset, POS2FONTRESOURCE pFont, PGLYPHBITMAP pGlyph );
ULONG OS2FontGlyphIndex( POS2FONTRESOURCE pFont, ULONG index );
ULONG ParseOS2FontResource( PVOID pBuffer, ULONG cbBuffer, POS2FONTRESOURCE pFont );
ULONG ReadOS2FNTFile( FILE *pf, PBYTE *ppBuffer, PULONG pulSize );
ULONG ReadOS2FontResource( PSZ pszFile, ULONG ulFace, PULONG pulCount, POS2FONTRESOURCE pFont );

#endif      // #ifndef __GPIFONT_H__

