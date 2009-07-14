#ifndef DRIFTY_H
#define DRIFTY_H

/** SALSA 20 **/
#ifndef ECRYPT_SYNC
#include "salsa20/ecrypt-sync.h"
#endif

/** SKEIN **/
#ifndef _SKEIN_H
#include "skein/skein_port.h"
#include "skein/skein.h"
#endif


/** extra unsigned 32-bit integer */
typedef uint_32t u32b_t;


#endif
