#define max(a, b) (a > b ? a : b)

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#define DB_MINUS_INFINITY -100.0f
// 10f32.powf(MINUS_INFINITY_DB / 20)
#define GAIN_MINUS_INFINITY 0.00001f

float db_to_gain(float dbs);
float gain_to_db(float gain);
float lerp(float a, float b, float t);

