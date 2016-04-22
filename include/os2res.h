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
    unsigned short etype;
    unsigned short ename;
} NERTENTRY, *PNERTENTRY;

typedef struct _NE_header {             /* 16-bit EXE header    */
    unsigned short  magic;                  /* 0x454E ("NE")                  */
    char            unused1[26];            /* various unnecessary fields     */
    unsigned short  cseg;                   /* number of file segments        */
    char            unused2[4];             /* various unnecessary fields     */
    unsigned short  seg_tbl;                /* offset to segment table        */
    unsigned short  res_tbl;                /* offset to resource table       */
    unsigned short  rnam_tbl;               /* offset to resident-names table */
    char            unused3[10];            /* various unnecessary fields     */
    unsigned short  segshift;               /* segment alignment shift        */
    unsigned short  cres;                   /* number of resource entries     */
    /*  10 bytes of various unnecessary fields follow */
} NEHEADER, *PNEHEADER;


/* Structures used for parsing 32-bit OS/2 executables.
 */

typedef struct _LX_rt_entry {           /* Resource table entry  */
    unsigned short type;
    unsigned short name;
    unsigned long  cb;
    unsigned short obj;
    unsigned long  offset;
} LXRTENTRY;

typedef struct _LX_ot_entry {           /* Object table entry    */
    unsigned long size;
    unsigned long base;
    unsigned long flags;
    unsigned long pagemap;
    unsigned long mapsize;
    unsigned long reserved;
} LXOTENTRY;

typedef struct LX_opm_entry {           /* Object page-map entry */
    unsigned long  dataoffset;
    unsigned short size;
    unsigned short flags;
} LXOPMENTRY, *PLXOPMENTRY;

typedef struct _LX_header {             /* 32-bit EXE header     */
    unsigned short  magic;                  /* 0x4C58 ("LX")                  */
    unsigned char   unused1[42];            /* various unnecessary fields     */
    unsigned long   pageshift;              /* page alignment shift           */
    unsigned char   unused2[8];             /* various unnecessary fields     */
    unsigned long   ldrsize;                /* loader section size            */
    unsigned long   ldrsum;                 /* loader section checksum        */
    unsigned long   obj_tbl;                /* offset to object table         */
    unsigned char   unused3[4];             /* various unnecessary fields     */
    unsigned long   objmap;                 /* offset to object page map      */
    unsigned char   unused4[4];             /* various unnecessary fields     */
    unsigned long   res_tbl;                /* offset to resource table       */
    unsigned long   cres;                   /* number of resource entries     */
    unsigned long   rnam_tbl;               /* offset to resident-names table */
    unsigned char   unused5[36];            /* various unnecessary fields     */
    unsigned long   datapage;               /* offset to data pages           */
    /* 64 bytes of various unnecessary fields follow                          */
} LXHEADER;


#pragma pack()

#endif      /* #ifndef __OS2RES_H__ */

