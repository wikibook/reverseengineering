#ifndef FIND_PATTERN_H_
#define FIND_PATTERN_H_

#include <windows.h>

typedef struct _SEARCH_CONTEXT {
  PUSHORT pusXPattern;
  PINT    piSuffixes;
  PINT    piGS;
  INT     nLength;
  INT     iBC[0x100];
} SEARCH_CONTEXT, far *LPSEARCH_CONTEXT, near *PSEARCH_CONTEXT, SRCH_CTX, far *LPSRCH_CTX, near *PSRCH_CTX;

#define NB_ERR  0xFF
#define NB_WC   0xCC
#define NB_WC_1 0x100
#define NB_WC_2 0x200

LPVOID FindPatternA(__in_bcount(nSize) LPCVOID lpcv, __in INT nSize, __in LPCSTR lpcszPattern);
LPVOID FindPatternW(__in_bcount(nSize) LPCVOID lpcv, __in INT nSize, __in LPCWSTR lpcwszPattern);

#endif // FIND_PATTERN_H_
