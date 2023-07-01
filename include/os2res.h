/*****************************************************************************
 *                                                                           *
 *  os2res.h                                                                 *
 *                                                                           *
 *  Definitions used in extracting binary resources from OS/2 modules.       *
 *  There are two types of OS/2 module: 16-bit (or "NE") format, and 32-bit  *
 *  (or "LX") format.  Information for both types is included here.  Note    *
 *  that the structures defined here only provide the fields which are       *
 *  necessary for extracting resources.                                      *
 *                                                                           *
 *  (C) 2012 Alexander Taylor                                                *
 *                                                                           *
 *  This code is placed in the public domain.                                *
 *                                                                           *
 *****************************************************************************/

#ifndef __OS2RES_H__
#define __OS2RES_H__


// ----------------------------------------------------------------------------
// CONSTANTS

/* Magic numbers indicating executable headers.
 */
#define MAGIC_MZ        0x5A4D    /* "MZ": DOS/old-style executable header */
#define MAGIC_NE        0x454E    /* "NE": 16-bit OS/2 executable header   */
#define MAGIC_LX        0x584C    /* "LX": 32-bit OS/2 executable header   */

#define EH_OFFSET_ADDRESS   0x3C  /* Location of the 'real' exe header offset */

/* OS/2 resource types that we're interested in.
 */
#define OS2RES_FONTDIR      6
#define OS2RES_FONTFACE     7

/* Values of interest for flags field of LXOPMENTRY (LX object page map entry).
 */
#define OP32_VALID           0x0000             /* Normal unpacked data    */
#define OP32_ITERDATA        0x0001             /* Data in EXEPACK1 format */
#define OP32_ITERDATA2       0x0005             /* Data in EXEPACK2 format */


// ----------------------------------------------------------------------------
// TYPEDEFS

/* Structures used for parsing 16-bit OS/2 executables.  These must be
 * single-byte aligned.
 */

#pragma pack(1)


typedef struct _NE_rt_entry {           /* Resource table entry */
    USHORT etype;
    USHORT ename;
} NERTENTRY, *PNERTENTRY;

typedef struct _NE_header {             /* 16-bit EXE header    */
    USHORT  magic;                  /* 0x454E ("NE")                  */
    CHAR    unused1[26];            /* various unnecessary fields     */
    USHORT  cseg;                   /* number of file segments        */
    CHAR    unused2[4];             /* various unnecessary fields     */
    USHORT  seg_tbl;                /* offset to segment table        */
    USHORT  res_tbl;                /* offset to resource table       */
    USHORT  rnam_tbl;               /* offset to resident-names table */
    CHAR    unused3[10];            /* various unnecessary fields     */
    USHORT  segshift;               /* segment alignment shift        */
    USHORT  cres;                   /* number of resource entries     */
    /*  10 bytes of various unnecessary fields follow */
} NEHEADER, *PNEHEADER;


/* Structures used for parsing 32-bit OS/2 executables.
 */

typedef struct _LX_rt_entry {           /* Resource table entry  */
    USHORT type;
    USHORT name;
    ULONG  cb;
    USHORT obj;
    ULONG  offset;
} LXRTENTRY;

typedef struct _LX_ot_entry {           /* Object table entry    */
    ULONG size;
    ULONG base;
    ULONG flags;
    ULONG pagemap;
    ULONG mapsize;
    ULONG reserved;
} LXOTENTRY;

typedef struct LX_opm_entry {           /* Object page-map entry */
    ULONG  dataoffset;
    USHORT size;
    USHORT flags;
} LXOPMENTRY, *PLXOPMENTRY;

typedef struct _LX_header {             /* 32-bit EXE header     */
    USHORT  magic;                  /* 0x4C58 ("LX")                  */
    UCHAR   unused1[42];            /* various unnecessary fields     */
    ULONG   pageshift;              /* page alignment shift           */
    UCHAR   unused2[8];             /* various unnecessary fields     */
    ULONG   ldrsize;                /* loader section size            */
    ULONG   ldrsum;                 /* loader section checksum        */
    ULONG   obj_tbl;                /* offset to object table         */
    UCHAR   unused3[4];             /* various unnecessary fields     */
    ULONG   objmap;                 /* offset to object page map      */
    UCHAR   unused4[4];             /* various unnecessary fields     */
    ULONG   res_tbl;                /* offset to resource table       */
    ULONG   cres;                   /* number of resource entries     */
    ULONG   rnam_tbl;               /* offset to resident-names table */
    UCHAR   unused5[36];            /* various unnecessary fields     */
    ULONG   datapage;               /* offset to data pages           */
    /* 64 bytes of various unnecessary fields follow                          */
} LXHEADER;


#pragma pack()

#endif      /* #ifndef __OS2RES_H__ */

