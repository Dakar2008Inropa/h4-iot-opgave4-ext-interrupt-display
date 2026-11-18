#ifndef EXT_INTERRUPT_H_
#define EXT_INTERRUPT_H_

#include "ProjectDefines.h"

void ext_interrupt_init(void);
bool ext_interrupt_take_event(uint16_t *count_out);

#endif
