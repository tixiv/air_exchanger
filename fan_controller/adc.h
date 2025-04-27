
#include <stdint.h>

extern uint16_t adc_filtered[4];
extern int16_t temperatures[4];

void update_adc(void);
void init_adc(void);
