/*****************************************************************************
 *                                                                           *
 *  unifont.h                                                                *
 *                                                                           *
 *  Definitions for handling OS/2 Uni-font bitmap fonts.                     *
 *                                                                           *
 *  The Uni-font format is conceptually very similar to the standard GPI     *
 *  font format, although the actual structure details are different.        *
 *  (However, some of the Uni-font structures are also used by the separate  *
 *  combined-font format.)                                                   *
 *                                                                           *
 *  Uni-font files seem to be very rare - so rare, in fact, that I've never  *
 *  actually encountered one.  IBM did go out of their way to write an IFI   *
 *  driver for the format and include it in recent versions of OS/2, so      *
 *  presumably there were some made at some point (probably in-house for     *
 *  some corporate customers).                                               *
 *                                                                           *
 *  (C) 2012 Alexander Taylor                                                *
 *                                                                           *
 *  This code is placed in the public domain.                                *
 *                                                                           *
 *****************************************************************************/

#ifndef __UNIFONT_H__
#define __UNIFONT_H__


#define FACESIZE        32
#define GLYPHNAMESIZE   16

/* IFIMETRICS32 flType flags */

#define IFIMETRICS32_FIXED       0x0001     /* font is fixed-width          */
#define IFIMETRICS32_LICENSED    0x0002     /* font is protected by license */
#define IFIMETRICS32_KERNING     0x0004     /* font has kerning data        */
#define IFIMETRICS32_DBCS        0x0010     /* font has DBCS glyphs         */
#define IFIMETRICS32_MBCS        0x0018     /* font has MBCS glyphs         */
#define IFIMETRICS32_ANTIALIASED 0x0020     /* reserved for future use      */
#define IFIMETRICS32_UNICODE     0x0040     /* font supports UNICODE        */
#define IFIMETRICS32_NO_CACHE    0x0080     /* unclear, possibly not used   */
#define IFIMETRICS32_FACETRUNC   0x1000     /* face name was truncated      */
#define IFIMETRICS32_FAMTRUNC    0x2000     /* family name was truncated    */

/* IFIMETRICS32 flDefn flags */

#define IFIMETRICS_OUTLINE       0x0001     /* font is outline (not bitmap) */
#define IFIMETRICS32_UDC_FONT    0x0010     /* DBCS user-defined characters */

/* IFIMETRICS32 flSelection flags */

#define IFIMETRICS32_ITALIC      0x8000     /* italic (outline/bitmap fonts)     */
#define IFIMETRICS32_UNDERSCORE  0x4000     /* underscore (outline/bitmap fonts) */
#define IFIMETRICS32_OVERSTRUCK  0x2000     /* strikeout (outline/bitmap fonts)  */
#define IFIMETRICS32_NEGATIVE    0x1000     /* negative image (bitmap fonts) */
#define IFIMETRICS32_HOLLOW      0x0800     /* outlined image (bitmap fonts) */


#pragma pack(1)

/* The IFIMETRICS32 structure is slightly different from the regular IFIMETRICS
 * used by the OS/2 installable font interface.
 */
typedef struct _IFIMETRICS32 {
    UCHAR   szFamilyname[FACESIZE];
    UCHAR   szFacename[FACESIZE];
    UCHAR   szGlyphlistName[GLYPHNAMESIZE];
    ULONG   idRegistry;
    LONG    lCapEmHeight;
    LONG    lXHeight;
    LONG    lMaxAscender;
    LONG    lMaxDescender;
    LONG    lLowerCaseAscent;
    LONG    lLowerCaseDescent;
    LONG    lInternalLeading;
    LONG    lExternalLeading;
    LONG    lAveCharWidth;
    LONG    lMaxCharInc;
    LONG    lEmInc;
    LONG    lMaxBaselineExt;
    FIXED   fxCharSlope;
    FIXED   fxInlineDir;
    FIXED   fxCharRot;
    ULONG   ulWeightClass;
    ULONG   ulWidthClass;
    LONG    lEmSquareSizeX;
    LONG    lEmSquareSizeY;
    GLYPH   giFirstChar;
    GLYPH   giLastChar;
    GLYPH   giDefaultChar;
    GLYPH   giBreakChar;
    ULONG   ulNominalPointSize;
    ULONG   ulMinimumPointSize;
    ULONG   ulMaximumPointSize;
    ULONG   flType;
    ULONG   flDefn;
    ULONG   flSelection;
    ULONG   flCapabilities;
    LONG    lSubscriptXSize;
    LONG    lSubscriptYSize;
    LONG    lSubscriptXOffset;
    LONG    lSubscriptYOffset;
    LONG    lSuperscriptXSize;
    LONG    lSuperscriptYSize;
    LONG    lSuperscriptXOffset;
    LONG    lSuperscriptYOffset;
    LONG    lUnderscoreSize;
    LONG    lUnderscorePosition;
    LONG    lStrikeoutSize;
    LONG    lStrikeoutPosition;
    ULONG   ulKerningPairs;
    ULONG   ulFontClass;
} IFIMETRICS32;
typedef IFIMETRICS32 *PIFIMETRICS32;


/* The Uni-font resource signature string (null-terminated).
 */
#define UNIFNT_SIGNATURE    "UNI FONT"


/* Uni-font data structure signatures, in 4-byte integer format.  These are
 * stored in the actual structures as byte arrays; due to byte-order conventions
 * the values below appear reversed relative to the equivalent string values.
 * (i.e. "UNFD" == {0x55,0x4E,0x46,0x44} becomes 0x44464E55 when cast to ULONG.)
 * They are all 4-byte fixed width, NOT null-terminated.
 */
#define SIG_UNFD    0x44464E55      /* Font directory header               */
#define SIG_UNFS    0x53464E55      /* Font resource-start signature block */
#define SIG_UNFM    0x4D464E55      /* Metrics structure                   */
#define SIG_UNFH    0x48464E55      /* Font definition header              */
#define SIG_UNGH    0x48474E55      /* Character-group definition header   */
#define SIG_UNKT    0x544B4E55      /* Kerning-pair table header           */
#define SIG_UNFE    0x45464E55      /* Font resource-end signature block   */



/* A Uni-font file consists of a Uni-font directory (UNIFONTDIRECTORY structure)
 * which contains a variable-length array of Uni-font resource definitions
 * (UNIFONTRESOURCEENTRY structures) that define some flags and point to the
 * offset within the file of the actual font resources they describe.
 *
 * A Uni-font resource contains the following records, in order:
 *  - Font signature                         (UNIFONTSIGNATURE)
 *  - Font metrics                           (UNIFONTMETRICS)
 *  - Font definition header                 (UNIFONTDEFINITIONHEADER)
 *  - Character group definitions (1)        (UNICHARGROUPDEFINITION)
 *  - (OPTIONAL) Kerning table               (UNIKERNPAIRTABLE)
 *  - Character definitions (1)              (variable format)
 *  - The font end signature                 (UNIENDFONTRESOURCE)
 *
 * The font image data (glyph bitmaps) follow the font character definitions.
 *
 * (1) It is actually possible for a Uni-font to lack both the character group
 * and character definitions (indicated by UNIFONT_VIRTUAL_FONT in the flUniFont
 * field of the UNIFONTRESOURCEENTRY structure).  Such a font is called a
 * "virtual font", and contains no glyphs in and of itself.  (AFAIK this is
 * mainly used on DBCS systems where a special kind of combined-font file called
 * an "associated bitmaps rule" may be defined to associate separately-loaded
 * bitmaps with an existing font name and point size.  This allows multiple
 * fonts to share glyph bitmaps.  What's usually done is that the bitmaps are
 * associated with certain point sizes of one or more outline fonts to improve
 * their on-screen appearance - similar to TTF/OTF embedded bitmaps except that
 * the bitmaps are independent of the outline font - and then a virtual font is
 * also created to allow those bitmaps to be used as a standalone screen font as
 * well.)
 */


#define UNIFONT_VIRTUAL_FONT    0x1

typedef struct _UNIFONTRESOURCEENTRY {
    ULONG flUniFont;        /* Uni font flags:                              */
                            /*   UNIFONT_VIRTUAL_FONT (bit 0)               */
                            /*   - This is a virtual font, and contains     */
                            /*     only the font signature, metrics,        */
                            /*     definition header and end signature.     */
                            /*   Other bits must be 0.                      */

    LONG  offsetUniFont;    /* The offset from the beginning of the file of */
                            /* the Uni-font resource.                       */

    ULONG ulBaseUniFont;    /* The index in the FontResEntry array in       */
                            /* UNIFONTDIRECTORY which specifies the base    */
                            /* Uni font resource used by the virtual font.  */
                            /* This field will be specified only when the   */
                            /* font is defined as a virtual font.           */
} UNIFONTRESOURCEENTRY;



#define UNIFONT_WRITABLE  0x2

typedef struct _UNIFONTDIRECTORY {
    UCHAR Identity[4];                     /* "UNFD"                        */

    ULONG ulSize;                          /* The size of the structure, in */
                                           /* bytes, including all entries  */
                                           /* of the UNIFONTRESOURCEENTRY   */
                                           /* array.  i.e.:                 */
                                           /*  sizeof(UNIFONTDIRECTORY) +   */
                                           /*  sizeof(UNIFONTRESOURCEENTRY) */
                                           /*  * (ulUniFontResources - 1)   */

    ULONG ulUniFontResources;              /* The number of Uni font        */
                                           /* resources in the font file.   */

    ULONG flEndian;                        /* Flags specifying the byte and */
                                           /* bit order of the font file.   */
                                           /* Currently all bits must be 0. */

    ULONG flFileMode;                      /* Flags to specify file mode:   */
                                           /*   UNIFONT_WRITABLE (bit 1)    */
                                           /*   - This flag is ON if the    */
                                           /*     font file is designed to  */
                                           /*     be updated, as with a     */
                                           /*     User Defined Font (UDC).  */

    UNIFONTRESOURCEENTRY FontResEntry[1];  /* Array of UNIFONTRESOURCEENTRY */
                                           /* structures.                   */

} UNIFONTDIRECTORY;
typedef UNIFONTDIRECTORY *PUNIFONTDIRECTORY;


#define UNIFONT_KERNINGPAIRS_EXIST  0x1
#define UNIFONT_PANOSE_EXIST        0x2

typedef struct _UNIFONTSIGNATURE {
    UCHAR Identity[4];          /* "UNFS"                                   */
    ULONG ulSize;               /* The size of the structure in bytes.      */
    UCHAR szSignature [24];     /* Must be "UNI FONT" (null-terminated).    */
    UCHAR szTechnology[64];     /* Reserved for future use; must be "".     */
    LONG  offsetCompressTable;  /* Reserved for future use; must be 0.      */
    ULONG flFontResource;       /* Flags:                                   */
                                /*   UNIFONT_KERNINGPAIRS_EXIST (bit 0)     */
                                /*   - Font includes a kerning pairs table  */
                                /*   UNIFONT_PANOSE_EXIST (bit 1)           */
                                /*   - Font includes a PANOSE table         */
                                /*   Other bits must be ZERO.               */
} UNIFONTSIGNATURE, *PUNIFONTSIGNATURE;


typedef struct _UNIENDFONTRESOURCE {
    UCHAR Identity[4];          /* Must be 0x554E4645 ('UNFE').             */
    ULONG ulSize;               /* The size of the structure, in bytes.     */
} UNIENDFONTRESOURCE, *PUNIENDFONTRESOURCE;



#define UNIFONTMETRICS_PANOSE_EXIST             0x1
#define UNIFONTMETRICS_FULLFAMILYNAME_EXIST     0x2
#define UNIFONTMETRICS_FULLFACENAME_EXIST       0x4

/* NOTE: According to the documented specification (from which the following
 * definition is derived), the szFull*name arrays should both be 256 bytes.
 * However, all existing CMB fonts (which provide the only examples I could
 * find of this structure in actual use) report 580 bytes in the ulSize field
 * (and the file format bears this number out).  Since the other fields appear
 * to be accurate, that leaves a total of 296 bytes available for both arrays.
 * The 256/40 bytes assigned to the arrays here is just a guess; unfortunately,
 * no CMB font that I'm aware of uses either field so the format cannot be
 * reverse-engineered any further.  Until a more definitive reference can be
 * found, use of these two fields is probably best avoided.  (It's also a good
 * idea to always use the contents of ulSize to determine the structure size.)
 */
typedef struct _UNIFONTMETRICS {
    UCHAR        Identity[4];   /* "UNFM"                                   */
    ULONG        ulSize;        /* The size of this structure in bytes      */
    IFIMETRICS32 ifiMetrics;    /* Basic font metrics.                      */
    ULONG        flOptions;     /* Option flags:                            */
                                /*   UNIFONTMETRICS_PANOSE_EXIST (bit 0)    */
                                /*   - The panose field is valid.           */
                                /*   UNIFONTMETRICS_FULLFAMILYNAME_EXIST    */
                                /*   (bit 1)                                */
                                /*   - The szFullFamilyname field is valid. */
                                /*   UNIFONTMETRICS_FULLFACENAME_EXIST      */
                                /*   (bit 2)                                */
                                /*   - The szFullFacename field is valid.   */
    UCHAR        panose[ 12 ];          /* The 12-byte PANOSE structure.    */
    UCHAR        szFullFamilyname[256]; /* The untruncated font family name */
    UCHAR        szFullFacename[40];    /* The untruncated font face name   */

} UNIFONTMETRICS;
typedef UNIFONTMETRICS *PUNIFONTMETRICS;


/* The font definition header contains information about the format of the
 * character definitions, the character group definitions, data about each
 * group, each character including width data and the offset into the
 * definition section at which the character glyph definition begins. This
 * structure defines the format of the character group definition records and
 * the character definition records that follow:
 */

#define UNIFONTDEF_CHARWIDTH_DEFINED    0x01
#define UNIFONTDEF_CHARHEIGHT_DEFINED   0x02
#define UNIFONTDEF_CHARINC_DEFINED      0x04
#define UNIFONTDEF_ASPACE_DEFINED       0x08
#define UNIFONTDEF_BSPACE_DEFINED       0x10
#define UNIFONTDEF_CSPACE_DEFINED       0x20
#define UNIFONTDEF_BASELINE_DEFINED     0x40
#define UNIFONTDEF_CHAR_OFFSET_DEFINED  0x80


/* GPI categorizes fonts in 3 types according to character increment definition:
 * Type 1 : Fixed-width font
 * Type 2 : Proportional-width font with each character increment defined as
 *          one value in the character definition.
 * Type 3 : Proportional-width font with each character increment defined in
 *          terms of: a_space + b_space + c_space
 *
 * Each type is required to have specific values of flFontDef and flCharDef
 * in the font definition header.  These values are defined below.
 */
#define UNIFONTDEF_TYPE_1_FONTDEF       0x47
#define UNIFONTDEF_TYPE_2_FONTDEF       0x42
#define UNIFONTDEF_TYPE_3_FONTDEF       0x42
#define UNIFONTDEF_TYPE_1_GROUPDEF      0
#define UNIFONTDEF_TYPE_2_GROUPDEF      0
#define UNIFONTDEF_TYPE_3_GROUPDEF      0
#define UNIFONTDEF_TYPE_1_CHARDEF       0x81
#define UNIFONTDEF_TYPE_2_CHARDEF       0x81
#define UNIFONTDEF_TYPE_3_CHARDEF       0xB9
#define UNIFONTDEF_TYPE_1_CHARDEF_SIZE  6
#define UNIFONTDEF_TYPE_2_CHARDEF_SIZE  6
#define UNIFONTDEF_TYPE_3_CHARDEF_SIZE  10

typedef struct _UNIFONTDEFINITIONHEADER {
    UCHAR  Identity[4];         /* Must be equal to 0x554E4648 ('UNFH').    */

    ULONG  ulSize;              /* Size of this structure in bytes.         */

    ULONG  flFontDef;           /* Flags indicating which fields are present in the font definition header.                                             */
                                /*   Bit 0  1 = character width is defined in the font definition header (fixed pitch)                                  */
                                /*          0 = character width is not defined in the font definition header                                            */
                                /*   Bit 1  1 = character height is defined in the font definition header (same for all characters)                     */
                                /*          0 = character height is not defined in the font definition header                                           */
                                /*   Bit 2  1 = character increment is defined in the font definition header (same for all characters)                  */
                                /*          0 = character increment is not defined in the font definition header                                        */
                                /*   Bit 3  1 = a_space is defined in the font definition header (same for all characters)                              */
                                /*          0 = a_space is not defined in the font definition header (non abc_space font or defined for each character) */
                                /*   Bit 4  1 = b_space is defined in the font definition header (same for all characters)                              */
                                /*          0 = b_space is not defined in the font definition header (non abc_space font or defined for each character) */
                                /*   Bit 5  1 = c_space is defined in the font definition header (same for all characters)                              */
                                /*          0 = c_space is not defined in the font definition header (non abc_space font or defined for each character) */
                                /*   Bit 6  1 = baseline offset is defined in the font definition header (same  for all characters)                         */
                                /*          0 = baseline offset is not defined in the font definition header                                                */

    ULONG  flGroupDef;          /* Flags indicating which fields are present on a per character group basis. Each flag is valid only when the           */
                                /* corresponding flFontDef flag bit is 1.                                                                               */
                                /*   Bit 0  1 = character width is defined in the character group definitions (defined for each character group)        */
                                /*          0 = character width is not defined in the character group definition (same for all characters)              */
                                /*   Bit 1  1 = character height is defined in the character group definitions (defined for each character group)       */
                                /*          0 = character height is not defined in the character group definitions (same for all characters)            */
                                /*   Bit 2  1 = character increment is defined in the the character group definitions (defined for each character group)*/
                                /*          0 = character increment is not defined in the character group definitions (same for all characters)         */
                                /*   Bit 3  1 = a_space is defined in the character group definitions (defined for each character group)                */
                                /*          0 = a_space is not defined in the character group definitions (same for all characters)                     */
                                /*   Bit 4  1 = b_space is defined in the character group definitions (defined for each character group)                */
                                /*          0 = b_space is not defined in the character group definitions (same for all characters)                     */
                                /*   Bit 5  1 = c_space is defined in the character group definitions (defined for each character group)                */
                                /*          0 = c_space is not defined in the character group definitions (same for all characters)                     */
                                /*   Bit 6  1 = baseline offset is defined in the character group definitions (defined for each character group)            */
                                /*          0 = baseline offset is not defined in the character group definitions (same for all characters)                 */

    ULONG  flCharDef;           /* Flags indicating which fields are present on a per character basis.                                                  */
                                /*   Bit 0  1 = character width is defined in the character definitions (defined for each character definition)         */
                                /*          0 = character width is not defined in the character definitions                                             */
                                /*   Bit 1  1 = character height is defined in the character definitions (defined for each character definition)        */
                                /*          0 = character height is not defined in the character definitions                                            */
                                /*   Bit 2  1 = character increment is defined in the the character definitions (defined for each character definition) */
                                /*          0 = character increment is not defined in the character definitions                                         */
                                /*   Bit 3  1 = a_space is defined in the character definitions (defined for each character definition)                 */
                                /*          0 = a_space is not defined in the character definitions                                                     */
                                /*   Bit 4  1 = b_space is defined in the character definitions (defined for each character definition)                 */
                                /*          0 = b_space is not defined in the character definitions                                                     */
                                /*   Bit 5  1 = c_space is defined in the character definitions (defined for each character definition)                 */
                                /*          0 = c_space is not defined in the character definitions                                                     */
                                /*   Bit 6  1 = baseline offset is defined in the character definitions (defined for each character definition)             */
                                /*          0 = baseline offset is not defined in the character definitions                                                 */
                                /*   Bit 7  1 = offset to glyph is defined                                                                              */
                                /*          0 = offset to glyph is not defined                                                                          */
                                /*   Bit 7 must be ON for OS/2 4.0.                                                                                     */

    ULONG  ulCharDefSize;       /* Indicates the length in bytes of each    */
                                /* character definition record (the per     */
                                /* character data). This does not include   */
                                /* the offset to glyph field which starts   */
                                /* every character definition.              */

    SHORT  xCellWidth;          /* The width of the characters, in pels for */
                                /* image fonts.                             */

    SHORT  yCellHeight;         /* The height of the characters, in pels    */
                                /* for image fonts.                         */

    SHORT  xCellIncrement;      /* The distance, in pels for image fonts,   */
                                /* along with the character baseline        */
                                /* required to step from one character to   */
                                /* the next (when forming a character       */
                                /* string).                                 */

    SHORT  xCellA;              /* The width of the space before a          */
                                /* character in the inline direction (the   */
                                /* a_space), in pels for image fonts.       */

    SHORT  xCellB;              /* The width of a character (inline         */
                                /* direction), in pels for image fonts (the */
                                /* b_space).                                */

    SHORT  xCellC;              /* The width of the space after a character */
                                /* in the inline direction, in pels for     */
                                /* image fonts (the c_space).               */

    SHORT  yCellBaseOffset;     /* The position of the top of a character   */
                                /* definition relative to the baseline in   */
                                /* the direction perpendicular to the       */
                                /* baseline, in pels for image fonts.       */

    USHORT usReserved;          /* Must be ZERO.                            */

    GLYPH  giFirstChar;         /* The index of the first character glyph   */
                                /* in the font file.                        */

    GLYPH  giLastChar;          /* The index of the last character glyph in */
                                /* the font file.                           */

    ULONG  ulCharDefNum;        /* The number of the character definitions  */
                                /* in the font file. If all of the          */
                                /* characters between the first and the     */
                                /* last character are defined, the value in */
                                /* this field will be giLastChar -          */
                                /* giFirstChar + 1.                         */
} UNIFONTDEFINITIONHEADER;
typedef UNIFONTDEFINITIONHEADER *PUNIFONTDEFINITIONHEADER;


/*
 * The character group definition record is defined to support the font indices
 * of a large character set of which indices may consist of sparse ranges.
 */

#define UNIFONT_FREQUENT_GROUP  0x1

typedef struct _UNICHARGROUPENTRY {
    ULONG  flCharGroupEntry;    /* If the font file is created for a single */
                                /* repository,                              */
                                /*   UNIFONT_FREQUENT_GROUP (Bit 0)         */
                                /* indicates that this character group may  */
                                /* be frequently used. It is recommended    */
                                /* that the font subsystem cache the        */
                                /* character definitions of this group in   */
                                /* memory for the performance when this bit */
                                /* is ON.                                   */

    GLYPH  giFirstChar;         /* Indicates the start font index of the    */
                                /* character group.                         */

    GLYPH  giLastChar;          /* Indicates the end font index of the      */
                                /* character group.                         */

    LONG   offsetCharDef;       /* The offset from the beginning of the Uni */
                                /* font resource (the beginning of the font */
                                /* signature structure) at which the        */
                                /* character definition record of this      */
                                /* character group begins.                  */

    /* The following fields are defined in the UNICHARGROUPENTRY structure,
     * however, the values may or may not be present in them according to the
     * flGroupDef field in UNIFONTDEFINITIONHEADER. If the values are present,
     * they are applied for only the character group.
     */
    SHORT  xCellWidth;          /* The width of the characters in the       */
                                /* character group definition, in pels.     */

    SHORT  yCellHeight;         /* The height of the characters in the      */
                                /* character group definition in pels.      */

    SHORT  xCellIncrement;      /* The length along the character baseline  */
                                /* required to step from this character to  */
                                /* the next in pels (when forming a         */
                                /* character string).                       */

    SHORT  xCellA;              /* The width of the space before the        */
                                /* character in the inline direction, in    */
                                /* pels (a_space).                          */

    SHORT  xCellB;              /* The width of the character shape in the  */
                                /* inline direction, in pels (b_space).     */

    SHORT  xCellC;              /* The width of the space after the         */
                                /* character in the inline direction, in    */
                                /* pels (c_space).                          */

    SHORT  yCellBaseOffset;     /* The position of the top of the           */
                                /* characters in the character group        */
                                /* definition relative to the baseline, in  */
                                /* the direction perpendicular to the       */
                                /* baseline, in pels.                       */

    USHORT usReserved;          /* Must be ZERO.                            */

} UNICHARGROUPENTRY;



typedef struct _UNICHARGROUPDEFINITION {
    UCHAR             Identity[4];        /* "UNGH"                         */

    ULONG             ulSize;             /* Size of this structure in      */
                                          /* bytes, including all entries   */
                                          /* of UNICHARGROUPENTRY.  i.e.    */
                                          /* sizeof(UNICHARGROUPDEFINITION) */
                                          /*   + sizeof(UNICHARGROUPENTRY)  */
                                          /*   * (ulCharGroups - 1)         */

    ULONG             ulCharGroups;       /* The number of character groups */
                                          /* in the array.                  */

    UNICHARGROUPENTRY CharGroupEntry[1];  /* Array of character groups.     */

} UNICHARGROUPDEFINITION;
typedef UNICHARGROUPDEFINITION *PUNICHARGROUPDEFINITION;


/* This record collects the first four consecutive structures in each font
 * resource.  It will be followed by either the UNIKERNINGPAIR structure,
 * or by the character definition data.
 */
typedef struct _UNIFONTRESOURCE {
    UNIFONTSIGNATURE        unifSignature;
    UNIFONTMETRICS          unifMetrics;
    UNIFONTDEFINITIONHEADER unifDefHeader;
    UNICHARGROUPDEFINITION  unifCharGroup;
} UNIFONTRESOURCE, *PUNIFONTRESOURCE;



typedef struct _UNIKERNINGPAIR {
    ULONG ulFirstChar;          /* First character of the kerning pair.     */
                                /* This value must be zero-extended when    */
                                /* converted to the GLYPH type.             */
    ULONG ulSecondChar;         /* Second character of the kerning pair.    */
                                /* This value must be zero-extended when    */
                                /* converted to the GLYPH type.             */
    LONG  lKerningAmount;       /* Kerning value in pels.                   */
} UNIKERNINGPAIR;


/* This record will exist in a Uni font-file format if the
 * UNIFONT_KERNINGPAIRS_EXIST flag in the flFontSignature field in the
 * UNIFONTSIGNATURE structure is on. The kerning pair table record is not
 * present if the cKerningPairs field in the UNIFONTMETRICS structure is zero.
 * This table should be sorted by the first character (usFirstChar) and the
 * second character (usSecondChar) order to allow binary searches.
 */

typedef struct _UNIKERNPAIRTABLE {
    UCHAR          Identity[4];   /* "UNKT"                                 */
    ULONG          ulSize;        /* Size of this stucture in bytes,        */
                                  /* including all entries of the KernPairs */
                                  /* array.  i.e.:                          */
                                  /*   sizeof(UNIKERNINGPAIRTABLE)          */
                                  /*   + sizeof(UNIKERNINGPAIR)             */
                                  /*     * (ulKernPairs - 1)                */
    ULONG          ulKernPairs;   /* The number of kerning pairs            */
    UNIKERNINGPAIR KernPairs[1];  /* Variable-length array of kerning pairs */
} UNIKERNPAIRTABLE;
typedef UNIKERNPAIRTABLE *PUNIKERNPAIRTABLE;


/* The font character definitions follow the kern pairs table if it exists, or
 * the character group definitions otherwise.  Each character definition
 * consists of some or all of these fields (depending on the flags indicated
 * in the flCharDef field in UNIFONTDEFINITIONHEADER):
 *
 * LONG offsetImageData     Character Glyph Definition Offset
 *                          The offset from the beginning of the Uni-font
 *                          resource at which the character glyph definition
 *                          begins. This field should be set to zero if the
 *                          character glyph is not defined in the font file.
 *                          If the font subsystem detects a zero value in
 *                          this field, it should return the character glyph
 *                          image from the giDefaultChar index which is
 *                          specified in UNIFONTMETRICS.
 *                          This field must be present.
 *
 * SHORT xCellWidth         Character Cell Width
 *                          The width of the character definition in pels.
 *
 * SHORT yCellHeight        Character Cell Height
 *                          The height of the character definition in pels.
 *
 * SHORT xCellIncrement     Character Increment
 *                          The length along the character baseline required
 *                          to step from this character to the next in pels
 *                          (when forming a character string).
 *
 * SHORT xCellA             Character a-space
 *                          The width of the space before the character in
 *                          the inline direction, in pels.
 *
 * SHORT xCellB             Character b-space
 *                          The width of the character shape in the inline
 *                          direction, in pels.
 *
 * SHORT xCellC             Character c-space
 *                          The width of the space after the character in the
 *                          inline direction, in pels.
 *
 * SHORT yCellBaseOffset    Character Baseline Offset
 *                          The position of the top of a character definition
 *                          relative to the baseline in the direction
 *                          perpendicular to the baseline, in pels.
 *
 * The actual character image data which follows is equivalent in format to an
 * OS/2 Bitmap image of one bit per pixel (1bpp).
 */

#pragma pack()
#endif      // #ifndef __UNIFONT_H__

