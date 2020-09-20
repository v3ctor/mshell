#ifndef _V3CT0R_SIGNALS_H
#define _V3CT0R_SIGNALS_H

#include <signal.h>

sigset_t signals_empty_mask(void);
void signals_block(void);
void signals_init(void);
void signals_unblock(void);

#endif // _V3CT0R_SIGNALS_H
