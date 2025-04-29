
#include <stdint.h>

extern uint16_t adc_filtered[2];
extern int16_t temperatures[2];

void update_adc(void);
void init_adc(void);
