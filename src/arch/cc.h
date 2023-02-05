#ifndef ARCH_CC_H_
#define ARCH_CC_H_

#include <stdlib.h>
#include "debug.h"

#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif

// GCC style
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT __attribute__((__packed__))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x

#define LWIP_PLATFORM_ASSERT(x) printf("Assertion \"%s\" failed at line %d in %s\n", x, __LINE__, __FILE__);
#define LWIP_PLATFORM_DIAG(x) printf x;
#define LWIP_RAND() ((uint32_t)rand())

typedef uint32_t sys_prot_t;

#endif
