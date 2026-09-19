#ifndef RPS_H
#define RPS_H

#include <stdint.h>
#include <float.h>
#include <stdbool.h>


void rps_initFn(void);
void rps_renderFn(void);
void rps_updateFn(bool* exit_flag);
void rps_select_rock(void);
void rps_select_paper(void);
void rps_select_scissors(void);

#endif