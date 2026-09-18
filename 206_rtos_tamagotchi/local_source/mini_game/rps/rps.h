#ifndef RPS_H
#define RPS_H

#include <stdint.h>
#include <float.h>
#include <stdbool.h>
#include "input.h"


void rps_initFns(void);
void rps_renderFn(void);
void rps_updateFn(void);
void rps_select_rock(void);
void rps_select_paper(void);
void rps_select_scissors(void);

#endif