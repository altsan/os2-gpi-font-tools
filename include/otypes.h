#include <stdint.h>

/* Standard types normally defined in the OS/2 headers...
 * redefined as fixed-width types here for portability.
 */
typedef uint8_t  UCHAR,  *PUCHAR;
typedef uint16_t USHORT, *PUSHORT;
typedef uint32_t ULONG,  *PULONG;
typedef uint8_t  BYTE,   *PBYTE;      /* (changed to unsigned) */
typedef int8_t   CHAR,   *PCHAR;
typedef int32_t  LONG,   *PLONG;
typedef int16_t  SHORT,  *PSHORT;
typedef uint32_t BOOL,   *PBOOL;

typedef char             *PSZ;
typedef void             *PVOID;

#ifndef FALSE
   #define FALSE   0
#endif
#ifndef TRUE
   #define TRUE    1
#endif

