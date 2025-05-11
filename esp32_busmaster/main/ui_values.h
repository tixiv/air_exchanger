
#include <stdbool.h>

struct UI_values
{
    bool power;
    int heater;
    int fan_speeds[2];
};

extern struct UI_values ui_values;