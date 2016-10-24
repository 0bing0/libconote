#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SET_UINT64 0xffFFffFFffFFffFF


typedef struct _GUID {
	uint32_t Data1;
	uint16_t Data2;
	uint16_t Data3;
	char Data4[8];
} __attribute__((packed))	GUID,
	UUID,
	*PGUID;

typedef struct _ExtendedGUID {
	GUID guid;
	uint32_t n;
} __attribute__((packed))	ExtendedGUID;

typedef struct _CompactID {
	unsigned int n : 8;
	uint32_t guidIndex : 24;
} __attribute__((packed)) CompactID;

#ifdef __cplusplus
} // extern C
#endif

#endif // TYPES_H_