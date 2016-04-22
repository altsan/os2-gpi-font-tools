/*****************************************************************************
 *                                                                           *
 *  cmbfont.h                                                                *
 *                                                                           *
 *  Definitions for handling OS/2 Combined fonts.                            *
 *                                                                           *
 *  The Combined format is somewhat similar to the separate Uni-font format, *
 *  and shares some of the same structures.  It is described in the CMBFont  *
 *  document from IBM.                                                       *
 *                                                                           *
 *  Combined font files are mostly used to assemble working DBCS fonts out   *
 *  various component font files, and many of these are included in DBCS     *
 *  releases of OS/2.  The format can also be used to define font aliases,   *
 *  and non-DBCS OS/2 users are probably most familiar with them in this     *
 *  context (WSeB and MCP/ACP include one such alias file: TNRMT30.CMB).     *
 *                                                                           *
 *  (C) 2012 Alexander Taylor                                                *
 *                                                                           *
 *  This code is placed in the public domain.                                *
 *                                                                           *
 *****************************************************************************/

#ifndef __CMBFONT_H__
#define __CMBFONT_H__

typedef ULONG GLYPH;
typedef ULONG *PGLYPH;

#include "unifont.h"              /* Defines UNIFONTMETRICS and IFIMETRICS32 */


// ----------------------------------------------------------------------------
// CONSTANTS

/* Combined font data block signatures
 */

#define SIG_CBFS        0x53464243      // Combined font signature header
#define SIG_CBFM        0x4D464243      // Combined font metrics
#define SIG_CPFH        0x48465043      // Component-fonts array header
#define SIG_CPFT        0x54464243      // Component-font definition
#define SIG_FTAS        0x53415446      // Font association definition
#define SIG_UFMM        0x4D4D4655      // Uni-font metrics member
#define SIG_CBFE        0x45464243      // Combined-font end signature

#define SIG_ABRS        0x53524241      // Associated bitmaps rule file signature
#define SIG_ABRE        0x45524241      // Associated bitmaps rule end signature

#define SIG_PCRS        0x53524350      // Pre-combine rule file signature
#define SIG_TFAH        0x48414654      // Target-font association header
#define SIG_PCRE        0x45524350      // Pre-combine rule end signature


// ----------------------------------------------------------------------------
// TYPEDEFS


/* The structure of a combine font file is as follows:
 *  1. Combined font signature (COMBFONTSIGNATURE)
 *  2. Combined font metrics (COMBFONTMETRICS)
 *  3. Component font header and 1-st component font (COMPFONTHEADER)
 *  4. 2-nd to (N-1)-th component font (COMPFONT)
 *  5. N-th component font (COMPFONT)
 *  6. Combined font end signature (COMBFONTEND)
 */


typedef struct _COMBFONTSIGNATURE {
    ULONG Identity;                 /* Must be 0x53464243 ("CBFS").         */
    ULONG ulSize;                   /* The size of this structure in bytes  */
    ULONG flEndian;                 /* Endian flags; not used, must be 0    */
    ULONG ulReserved;               /* Must be 0                            */
    ULONG ulVersion;                /* Must be 0                            */
    UCHAR szSignature[32];          /* Must be "COMBINED FONT"              */
} COMBFONTSIGNATURE;
typedef COMBFONTSIGNATURE *PCOMBFONTSIGNATURE;



typedef struct _COMBFONTMETRICS {
    ULONG          Identity;        /* Must be 0x4D464243 ("CBFM").         */
    ULONG          ulSize;          /* The size of this structure in bytes  */
    UNIFONTMETRICS unifm;           /* Structure representing the combined  */
                                    /* font metrics. This structure is      */
                                    /* defined in 'unifont.h'.              */
} COMBFONTMETRICS;
typedef COMBFONTMETRICS *PCOMBFONTMETRICS;




#define ASSOC_EXACT_MATCH   0x00000001
#define ASSOC_DONT_CARE     0x00000002
#define ASSOC_USE_PARENT    0x00000004

typedef struct _IFIMETRICS32MEMBER {
    BYTE fbFamilyname;
    BYTE fbFacename;
    BYTE fbGlyphlistname;
    BYTE fbRegistry;
    BYTE fbCapEmHeight;
    BYTE fbXHeight;
    BYTE fbMaxAscender;
    BYTE fbMaxDescender;
    BYTE fbLowerCaseAscent;
    BYTE fbLowerCaseDescent;
    BYTE fbInternalLeading;
    BYTE fbExternalLeading;
    BYTE fbAveCharWidth;
    BYTE fbMaxCharInc;
    BYTE fbEmInc;
    BYTE fbMaxBaselineExt;
    BYTE fbCharSlope;
    BYTE fbInlineDir;
    BYTE fbCharRot;
    BYTE fbWeightClass;
    BYTE fbWidthClass;
    BYTE fbEmSquareSizeX;
    BYTE fbEmSquareSizeY;
    BYTE fbFirstChar;
    BYTE fbLastChar;
    BYTE fbDefaultChar;
    BYTE fbBreakChar;
    BYTE fbNominalPointSize;
    BYTE fbMinimumPointSize;
    BYTE fbMaximumPointSize;
    BYTE fbType;
    BYTE fbDefn;
    BYTE fbSelection;
    BYTE fbCapabilities;
    BYTE fbSubscriptXSize;
    BYTE fbSubscriptYSize;
    BYTE fbSubscriptXOffset;
    BYTE fbSubscriptYOffset;
    BYTE fbSuperscriptXSize;
    BYTE fbSuperscriptYSize;
    BYTE fbSuperscriptXOffset;
    BYTE fbSuperscriptYOffset;
    BYTE fbUnderscoreSize;
    BYTE fbUnderscorePosition;
    BYTE fbStrikeoutSize;
    BYTE fbStrikeoutPosition;
    BYTE fbKerningPairs;
    BYTE fbFontClass;
} IFIMETRICS32MEMBER;



typedef struct _UNIFONTMETRICSMEMBER {
    ULONG              Identity;          /* Must be 0x4D4D4655 ("UFMM")    */
    ULONG              ulSize;            /* Size of the structure in bytes */
    IFIMETRICS32MEMBER ifi32mbr;          /* Flags for IFIMETRICS32 fields  */
    ULONG              ulReserved;
    BYTE               fbPanose;          /* Flag for PANOSE field          */
    BYTE               fbFullFamilyname;  /* Flag for family name field     */
    BYTE               fbFullFacename;    /* Flag for face name field       */
    UCHAR              ucReserved[5];
} UNIFONTMETRICSMEMBER;
typedef UNIFONTMETRICSMEMBER *PUNIFONTMETRICSMEMBER;



#define FONTASSOC_H_SCALING   0x00000001  /* Horizontal scaling             */
#define FONTASSOC_V_SCALING   0x00000002  /* Vertical scaling               */
#define FONTASSOC_NO_XLATION  0x00000004  /* No translation                 */

typedef struct _FONTASSOCGLYPHRANGE {
    GLYPH giStart;                  /* Start glyph index                    */
    GLYPH giEnd;                    /* End glyph index                      */
    GLYPH giTarget;                 /* Target glyph index mapped to         */
    ULONG ulReserved1;
    ULONG ulReserved2;
} FONTASSOCGLYPHRANGE;
typedef FONTASSOCGLYPHRANGE *PFONTASSOCGLYPHRANGE;

typedef struct _FONTASSOCIATION {
    ULONG                Identity;        /* Must be 0x53415446 ("FTAS").   */

    ULONG                ulSize;          /* The size of the structure, in  */
                                          /* bytes, which includes all of   */
                                          /* the FONTASSOCIATION structure. */

    UNIFONTMETRICSMEMBER unimbr;          /* The UNIFONTMETRICSMEMBER       */
                                          /* structure which has flags for  */
                                          /* all the UNIFONTMETRICS fields. */
                                          /* Each byte field represents     */
                                          /* whether the field is           */
                                          /* concerned for the association: */
                                          /*   ASSOC_EXACT_MATCH            */
                                          /*     This field must match.     */
                                          /*   ASSOC_DONT_CARE              */
                                          /*     This field is not          */
                                          /*     concerned.                 */
                                          /*   ASSOC_USE_PARENT             */
                                          /*     Use the information of     */
                                          /*     the parent association.    */

    UNIFONTMETRICS       unifm;           /* The UNIFONTMETRICS structure   */
                                          /* that represents the combined   */
                                          /* font metrics. Refer to the Uni */
                                          /* font format specification for  */
                                          /* the structure format.          */

    ULONG                ulGlyphRanges;   /* The number of glyph ranges to  */
                                          /* associate.                     */

    ULONG                flFlags;

    FONTASSOCGLYPHRANGE  GlyphRange[1];   /* The array of glyph ranges to   */
                                          /* associate. For combined fonts, */
                                          /* the range will decide what     */
                                          /* component font glyphs to be    */
                                          /* used as a part of the combined */
                                          /* font. For pre combine rule     */
                                          /* file, the range will decide    */
                                          /* what target font glyphs to be  */
                                          /* used to associate with the     */
                                          /* source font. The number of     */
                                          /* arrays depends on the value of */
                                          /* ulGlyphRanges.                 */
} FONTASSOCIATION;
typedef FONTASSOCIATION *PFONTASSOCIATION;



typedef struct _COMPFONT {
    ULONG           Identity;       /* Must be 0x54464243 ("CPFT").         */
    ULONG           ulSize;         /* The size of this structure in bytes  */
    FONTASSOCIATION CompFontAssoc;  /* The FONTASSOCIATION structure        */
} COMPFONT;
typedef COMPFONT *PCOMPFONT;



typedef struct _COMPFONTHEADER {
    ULONG    Identity;              /* Must be 0x48465043 ("CPFH")          */
    ULONG    ulSize;                /* The size of the structure, in bytes  */
    ULONG    ulCmpFonts;            /* The number of component fonts.       */
    COMPFONT CompFont[1];           /* The first entry of the component     */
                                    /* font array.                          */
} COMPFONTHEADER;
typedef COMPFONTHEADER *PCOMPFONTHEADER;



typedef struct _COMBFONTEND {
    ULONG Identity;                 /* Must be 0x45464243 ("CBFE")          */
    ULONG ulSize;                   /* The size of this structure in bytes  */
} COMBFONTEND;
typedef COMBFONTEND *PCOMBFONTEND;





/* The structure of an associated-bitmaps rule file is not publically
 * documented, but appears to be as follows:
 *  1. ABR file signature (ABRFILESIGNATURE)
 *  2. 1st font association entry (FONTASSOCIATION)
 *  3. 2nd to (ulCount)-th font association entry (FONTASSOCIATION)
 *  4. (ulCount + 1)-th font association entry (FONTASSOCIATION)
 *  5. ABR file end signature (ABRFILEEND)
 */

typedef struct _ABRFILESIGNATURE {
    ULONG Identity;                 /* Must be 0x53524241 ("ABRS")            */
    ULONG ulSize;                   /* The size of this structure in bytes    */
    ULONG flEndian;                 /* Must be 0                              */
    ULONG ulCount;                  /* Number of associations AFTER the 1st   */
    ULONG ulVersion;                /* Must be 0                              */
    UCHAR szSignature[32];          /* Must be "Associated Bitmap-fonts Rule" */
} ABRFILESIGNATURE;
typedef ABRFILESIGNATURE *PABRFILESIGNATURE;



typedef struct _ABRFILEEND {
    ULONG Identity;                 /* Must be 0x45524241 ("ABRE")          */
    ULONG ulSize;                   /* The size of this structure in bytes  */
} ABRFILEEND;
typedef ABRFILEEND *PABRFILEEND;



/* The structure of a pre-combine rule file is as follows:
 *  1. Pre combine rule signature structure (PRECOMBRULESIGNATURE)
 *  2. Source font association structure (FONTASSOCIATION)
 *  3. Target font association structure header (TARGETFONTASSOCHEADER)
 *  4. 1-st target font association structure (FONTASSOCIATION)
 *  5. 2-nd to (N-1)-th target font association structure (FONTASSOCIATION)
 *  6. N-th target font association structure (FONTASSOCIATION)
 *  7. Pre-combine rule file end signature structure (PRECOMBRULEEND)
 */

typedef struct _PRECOMBRULESIGNATURE {
    ULONG Identity;                 /* Must be 0x53524350 ("PCRS")          */
    ULONG ulSize;                   /* The size of this structure in bytes  */
    ULONG flEndian;                 /* Endian flags; not used, must be 0    */
    ULONG ulReserved;               /* Must be 0                            */
    ULONG ulVersion;                /* Must be 0                            */
    UCHAR szSignature[32];          /* Must be "PRE COMBINE RULE"           */
} PRECOMBRULESIGNATURE;
typedef PRECOMBRULESIGNATURE *PPRECOMBRULESIGNATURE;


typedef struct _TARGETFONTASSOCHEADER {
    ULONG Identity;                 /* Must be 0x48414654 ("TFAH")          */
    ULONG ulSize;                   /* The size of this structure in bytes  */
    ULONG ulTargetAssoc;            /* The number of FONTASSOCIATION items  */
    ULONG ulReserved;               /* Must be 0                            */
} TARGETFONTASSOCHEADER;
typedef TARGETFONTASSOCHEADER *PTARGETFONTASSOCHEADER;


typedef struct _PRECOMBRULEEND {
    ULONG Identity;                 /* Must be 0x45524350 ("PCRE")          */
    ULONG ulSize;                   /* The size of this structure in bytes  */
} PRECOMBRULEEND;
typedef PRECOMBRULEEND *PPRECOMBRULEEND;

#endif      // #ifndef __CMBFONT_H__

