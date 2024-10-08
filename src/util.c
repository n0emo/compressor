#include "util.h"

#include <math.h>

float db_to_gain(float dbs) {
    if (dbs >= DB_MINUS_INFINITY) {
        return powf(10.0f, dbs * 0.05f);
    } else {
        return 0.0f;
    }
}

float gain_to_db(float gain) {
    return log10f(max(fabsf(gain), GAIN_MINUS_INFINITY)) * 20.0f;
}

float lerp(float a, float b, float t) {
    return a * (1 - t) + b * t;
}

