#include <stdint.h>
#include <stddef.h>
#include <math.h>

// LTE Modulation

typedef struct
{
    float re;
    float im;
} T32fc;

typedef enum
{
    Bpsk,
    Qpsk,
    Qam16,
    Qam64,
    Qam256
} Modulation_t;

static const float C_PSK    = 1.f / sqrt(2.f);    // 1.4142
static const float C_16QAM  = 1.f / sqrt(10.f);   // 3.1623
static const float C_64QAM  = 1.f / sqrt(42.f);   // 6.4807
static const float C_256QAM = 1.f / sqrt(170.f);  // 13.0384

/* //##########################################################################
For integer case: replace T32fc with T16sc:
typedef struct
{
    int16_t re;
    int16_t im;
} T16sc;

static const int16_t C_PSK    = (int16_t)(16384.f / sqrt(2.f));
static const int16_t C_16QAM  = (int16_t)(16384.f / sqrt(10.f));
static const int16_t C_64QAM  = (int16_t)(16384.f / sqrt(42.f));
static const int16_t C_256QAM = (int16_t)(16384.f / sqrt(170.f));

Optimization note: separate odd & even bits from src (input byte) - 
this will significantly simplify/shorten the substitution table.

Result is expected in Q15 format (int16_t in format [1-bit sign][decimal dot][15-bit mantissa]).
"length" is multiple of 12, max length = 1200
*/ //##########################################################################

static const T32fc modBpsk[] =
{
    {C_PSK, C_PSK}, {-C_PSK, -C_PSK}
};

static const T32fc modQpsk[] =
{
    {C_PSK, C_PSK}, {C_PSK, -C_PSK}, {-C_PSK, C_PSK}, {-C_PSK, -C_PSK}
};

static const T32fc mod16QAM[] =
{
    { C_16QAM,  C_16QAM}, { C_16QAM,  3 * C_16QAM}, { 3 * C_16QAM,  C_16QAM}, { 3 * C_16QAM,  3 * C_16QAM},
    { C_16QAM, -C_16QAM}, { C_16QAM, -3 * C_16QAM}, { 3 * C_16QAM, -C_16QAM}, { 3 * C_16QAM, -3 * C_16QAM},
    {-C_16QAM,  C_16QAM}, {-C_16QAM,  3 * C_16QAM}, {-3 * C_16QAM,  C_16QAM}, {-3 * C_16QAM,  3 * C_16QAM},
    {-C_16QAM, -C_16QAM}, {-C_16QAM, -3 * C_16QAM}, {-3 * C_16QAM, -C_16QAM}, {-3 * C_16QAM, -3 * C_16QAM}
};

static const T32fc mod64QAM[] =
{
    { 3 * C_64QAM,  3 * C_64QAM}, { 3 * C_64QAM,      C_64QAM}, {     C_64QAM,  3 * C_64QAM}, {     C_64QAM,      C_64QAM},
    { 3 * C_64QAM,  5 * C_64QAM}, { 3 * C_64QAM,  7 * C_64QAM}, {     C_64QAM,  5 * C_64QAM}, {     C_64QAM,  7 * C_64QAM},
    { 5 * C_64QAM,  3 * C_64QAM}, { 5 * C_64QAM,      C_64QAM}, { 7 * C_64QAM,  3 * C_64QAM}, { 7 * C_64QAM,      C_64QAM},
    { 5 * C_64QAM,  5 * C_64QAM}, { 5 * C_64QAM,  7 * C_64QAM}, { 7 * C_64QAM,  5 * C_64QAM}, { 7 * C_64QAM,  7 * C_64QAM},
    { 3 * C_64QAM, -3 * C_64QAM}, { 3 * C_64QAM,     -C_64QAM}, {     C_64QAM, -3 * C_64QAM}, {     C_64QAM,     -C_64QAM},
    { 3 * C_64QAM, -5 * C_64QAM}, { 3 * C_64QAM, -7 * C_64QAM}, {     C_64QAM, -5 * C_64QAM}, {     C_64QAM, -7 * C_64QAM},
    { 5 * C_64QAM, -3 * C_64QAM}, { 5 * C_64QAM,     -C_64QAM}, { 7 * C_64QAM, -3 * C_64QAM}, { 7 * C_64QAM,     -C_64QAM},
    { 5 * C_64QAM, -5 * C_64QAM}, { 5 * C_64QAM, -7 * C_64QAM}, { 7 * C_64QAM, -5 * C_64QAM}, { 7 * C_64QAM, -7 * C_64QAM},
    {-3 * C_64QAM,  3 * C_64QAM}, {-3 * C_64QAM,      C_64QAM}, {    -C_64QAM,  3 * C_64QAM}, {    -C_64QAM,      C_64QAM},
    {-3 * C_64QAM,  5 * C_64QAM}, {-3 * C_64QAM,  7 * C_64QAM}, {    -C_64QAM,  5 * C_64QAM}, {    -C_64QAM,  7 * C_64QAM},
    {-5 * C_64QAM,  3 * C_64QAM}, {-5 * C_64QAM,      C_64QAM}, {-7 * C_64QAM,  3 * C_64QAM}, {-7 * C_64QAM,      C_64QAM},
    {-5 * C_64QAM,  5 * C_64QAM}, {-5 * C_64QAM,  7 * C_64QAM}, {-7 * C_64QAM,  5 * C_64QAM}, {-7 * C_64QAM,  7 * C_64QAM},
    {-3 * C_64QAM, -3 * C_64QAM}, {-3 * C_64QAM,     -C_64QAM}, {    -C_64QAM, -3 * C_64QAM}, {    -C_64QAM,     -C_64QAM},
    {-3 * C_64QAM, -5 * C_64QAM}, {-3 * C_64QAM, -7 * C_64QAM}, {    -C_64QAM, -5 * C_64QAM}, {    -C_64QAM, -7 * C_64QAM},
    {-5 * C_64QAM, -3 * C_64QAM}, {-5 * C_64QAM,     -C_64QAM}, {-7 * C_64QAM, -3 * C_64QAM}, {-7 * C_64QAM,     -C_64QAM},
    {-5 * C_64QAM, -5 * C_64QAM}, {-5 * C_64QAM, -7 * C_64QAM}, {-7 * C_64QAM, -5 * C_64QAM}, {-7 * C_64QAM, -7 * C_64QAM}
};

static const T32fc mod256QAM[] =
{
    {  5 * C_256QAM,  5 * C_256QAM}, {  5 * C_256QAM,  7 * C_256QAM}, {  7 * C_256QAM,  5 * C_256QAM}, {  7 * C_256QAM,  7 * C_256QAM},
    {  5 * C_256QAM,  3 * C_256QAM}, {  5 * C_256QAM,  1 * C_256QAM}, {  7 * C_256QAM,  3 * C_256QAM}, {  7 * C_256QAM,  1 * C_256QAM},
    {  3 * C_256QAM,  5 * C_256QAM}, {  3 * C_256QAM,  7 * C_256QAM}, {  1 * C_256QAM,  5 * C_256QAM}, {  1 * C_256QAM,  7 * C_256QAM},
    {  3 * C_256QAM,  3 * C_256QAM}, {  3 * C_256QAM,  1 * C_256QAM}, {  1 * C_256QAM,  3 * C_256QAM}, {  1 * C_256QAM,  1 * C_256QAM},
    {  5 * C_256QAM, 11 * C_256QAM}, {  5 * C_256QAM,  9 * C_256QAM}, {  7 * C_256QAM, 11 * C_256QAM}, {  7 * C_256QAM,  9 * C_256QAM},
    {  5 * C_256QAM, 13 * C_256QAM}, {  5 * C_256QAM, 15 * C_256QAM}, {  7 * C_256QAM, 13 * C_256QAM}, {  7 * C_256QAM, 15 * C_256QAM},
    {  3 * C_256QAM, 11 * C_256QAM}, {  3 * C_256QAM,  9 * C_256QAM}, {  1 * C_256QAM, 11 * C_256QAM}, {  1 * C_256QAM,  9 * C_256QAM},
    {  3 * C_256QAM, 13 * C_256QAM}, {  3 * C_256QAM, 15 * C_256QAM}, {  1 * C_256QAM, 13 * C_256QAM}, {  1 * C_256QAM, 15 * C_256QAM},
    { 11 * C_256QAM,  5 * C_256QAM}, { 11 * C_256QAM,  7 * C_256QAM}, {  9 * C_256QAM,  5 * C_256QAM}, {  9 * C_256QAM,  7 * C_256QAM},
    { 11 * C_256QAM,  3 * C_256QAM}, { 11 * C_256QAM,  1 * C_256QAM}, {  9 * C_256QAM,  3 * C_256QAM}, {  9 * C_256QAM,  1 * C_256QAM},
    { 13 * C_256QAM,  5 * C_256QAM}, { 13 * C_256QAM,  7 * C_256QAM}, { 15 * C_256QAM,  5 * C_256QAM}, { 15 * C_256QAM,  7 * C_256QAM},
    { 13 * C_256QAM,  3 * C_256QAM}, { 13 * C_256QAM,  1 * C_256QAM}, { 15 * C_256QAM,  3 * C_256QAM}, { 15 * C_256QAM,  1 * C_256QAM},
    { 11 * C_256QAM, 11 * C_256QAM}, { 11 * C_256QAM,  9 * C_256QAM}, {  9 * C_256QAM, 11 * C_256QAM}, {  9 * C_256QAM,  9 * C_256QAM},
    { 11 * C_256QAM, 13 * C_256QAM}, { 11 * C_256QAM, 15 * C_256QAM}, {  9 * C_256QAM, 13 * C_256QAM}, {  9 * C_256QAM, 15 * C_256QAM},
    { 13 * C_256QAM, 11 * C_256QAM}, { 13 * C_256QAM,  9 * C_256QAM}, { 15 * C_256QAM, 11 * C_256QAM}, { 15 * C_256QAM,  9 * C_256QAM},
    { 13 * C_256QAM, 13 * C_256QAM}, { 13 * C_256QAM, 15 * C_256QAM}, { 15 * C_256QAM, 13 * C_256QAM}, { 15 * C_256QAM, 15 * C_256QAM},

    {  5 * C_256QAM, -5 * C_256QAM}, {  5 * C_256QAM, -7 * C_256QAM}, {  7 * C_256QAM, -5 * C_256QAM}, {  7 * C_256QAM, -7 * C_256QAM},
    {  5 * C_256QAM, -3 * C_256QAM}, {  5 * C_256QAM, -1 * C_256QAM}, {  7 * C_256QAM, -3 * C_256QAM}, {  7 * C_256QAM, -1 * C_256QAM},
    {  3 * C_256QAM, -5 * C_256QAM}, {  3 * C_256QAM, -7 * C_256QAM}, {  1 * C_256QAM, -5 * C_256QAM}, {  1 * C_256QAM, -7 * C_256QAM},
    {  3 * C_256QAM, -3 * C_256QAM}, {  3 * C_256QAM, -1 * C_256QAM}, {  1 * C_256QAM, -3 * C_256QAM}, {  1 * C_256QAM, -1 * C_256QAM},
    {  5 * C_256QAM,-11 * C_256QAM}, {  5 * C_256QAM, -9 * C_256QAM}, {  7 * C_256QAM,-11 * C_256QAM}, {  7 * C_256QAM, -9 * C_256QAM},
    {  5 * C_256QAM,-13 * C_256QAM}, {  5 * C_256QAM,-15 * C_256QAM}, {  7 * C_256QAM,-13 * C_256QAM}, {  7 * C_256QAM,-15 * C_256QAM},
    {  3 * C_256QAM,-11 * C_256QAM}, {  3 * C_256QAM, -9 * C_256QAM}, {  1 * C_256QAM,-11 * C_256QAM}, {  1 * C_256QAM, -9 * C_256QAM},
    {  3 * C_256QAM,-13 * C_256QAM}, {  3 * C_256QAM,-15 * C_256QAM}, {  1 * C_256QAM,-13 * C_256QAM}, {  1 * C_256QAM,-15 * C_256QAM},
    { 11 * C_256QAM, -5 * C_256QAM}, { 11 * C_256QAM, -7 * C_256QAM}, {  9 * C_256QAM, -5 * C_256QAM}, {  9 * C_256QAM, -7 * C_256QAM},
    { 11 * C_256QAM, -3 * C_256QAM}, { 11 * C_256QAM, -1 * C_256QAM}, {  9 * C_256QAM, -3 * C_256QAM}, {  9 * C_256QAM, -1 * C_256QAM},
    { 13 * C_256QAM, -5 * C_256QAM}, { 13 * C_256QAM, -7 * C_256QAM}, { 15 * C_256QAM, -5 * C_256QAM}, { 15 * C_256QAM, -7 * C_256QAM},
    { 13 * C_256QAM, -3 * C_256QAM}, { 13 * C_256QAM, -1 * C_256QAM}, { 15 * C_256QAM, -3 * C_256QAM}, { 15 * C_256QAM, -1 * C_256QAM},
    { 11 * C_256QAM,-11 * C_256QAM}, { 11 * C_256QAM, -9 * C_256QAM}, {  9 * C_256QAM,-11 * C_256QAM}, {  9 * C_256QAM, -9 * C_256QAM},
    { 11 * C_256QAM,-13 * C_256QAM}, { 11 * C_256QAM,-15 * C_256QAM}, {  9 * C_256QAM,-13 * C_256QAM}, {  9 * C_256QAM,-15 * C_256QAM},
    { 13 * C_256QAM,-11 * C_256QAM}, { 13 * C_256QAM, -9 * C_256QAM}, { 15 * C_256QAM,-11 * C_256QAM}, { 15 * C_256QAM, -9 * C_256QAM},
    { 13 * C_256QAM,-13 * C_256QAM}, { 13 * C_256QAM,-15 * C_256QAM}, { 15 * C_256QAM,-13 * C_256QAM}, { 15 * C_256QAM,-15 * C_256QAM},

    { -5 * C_256QAM,  5 * C_256QAM}, { -5 * C_256QAM,  7 * C_256QAM}, { -7 * C_256QAM,  5 * C_256QAM}, { -7 * C_256QAM,  7 * C_256QAM},
    { -5 * C_256QAM,  3 * C_256QAM}, { -5 * C_256QAM,  1 * C_256QAM}, { -7 * C_256QAM,  3 * C_256QAM}, { -7 * C_256QAM,  1 * C_256QAM},
    { -3 * C_256QAM,  5 * C_256QAM}, { -3 * C_256QAM,  7 * C_256QAM}, { -1 * C_256QAM,  5 * C_256QAM}, { -1 * C_256QAM,  7 * C_256QAM},
    { -3 * C_256QAM,  3 * C_256QAM}, { -3 * C_256QAM,  1 * C_256QAM}, { -1 * C_256QAM,  3 * C_256QAM}, { -1 * C_256QAM,  1 * C_256QAM},
    { -5 * C_256QAM, 11 * C_256QAM}, { -5 * C_256QAM,  9 * C_256QAM}, { -7 * C_256QAM, 11 * C_256QAM}, { -7 * C_256QAM,  9 * C_256QAM},
    { -5 * C_256QAM, 13 * C_256QAM}, { -5 * C_256QAM, 15 * C_256QAM}, { -7 * C_256QAM, 13 * C_256QAM}, { -7 * C_256QAM, 15 * C_256QAM},
    { -3 * C_256QAM, 11 * C_256QAM}, { -3 * C_256QAM,  9 * C_256QAM}, { -1 * C_256QAM, 11 * C_256QAM}, { -1 * C_256QAM,  9 * C_256QAM},
    { -3 * C_256QAM, 13 * C_256QAM}, { -3 * C_256QAM, 15 * C_256QAM}, { -1 * C_256QAM, 13 * C_256QAM}, { -1 * C_256QAM, 15 * C_256QAM},
    {-11 * C_256QAM,  5 * C_256QAM}, {-11 * C_256QAM,  7 * C_256QAM}, { -9 * C_256QAM,  5 * C_256QAM}, { -9 * C_256QAM,  7 * C_256QAM},
    {-11 * C_256QAM,  3 * C_256QAM}, {-11 * C_256QAM,  1 * C_256QAM}, { -9 * C_256QAM,  3 * C_256QAM}, { -9 * C_256QAM,  1 * C_256QAM},
    {-13 * C_256QAM,  5 * C_256QAM}, {-13 * C_256QAM,  7 * C_256QAM}, {-15 * C_256QAM,  5 * C_256QAM}, {-15 * C_256QAM,  7 * C_256QAM},
    {-13 * C_256QAM,  3 * C_256QAM}, {-13 * C_256QAM,  1 * C_256QAM}, {-15 * C_256QAM,  3 * C_256QAM}, {-15 * C_256QAM,  1 * C_256QAM},
    {-11 * C_256QAM, 11 * C_256QAM}, {-11 * C_256QAM,  9 * C_256QAM}, { -9 * C_256QAM, 11 * C_256QAM}, { -9 * C_256QAM,  9 * C_256QAM},
    {-11 * C_256QAM, 13 * C_256QAM}, {-11 * C_256QAM, 15 * C_256QAM}, { -9 * C_256QAM, 13 * C_256QAM}, { -9 * C_256QAM, 15 * C_256QAM},
    {-13 * C_256QAM, 11 * C_256QAM}, {-13 * C_256QAM,  9 * C_256QAM}, {-15 * C_256QAM, 11 * C_256QAM}, {-15 * C_256QAM,  9 * C_256QAM},
    {-13 * C_256QAM, 13 * C_256QAM}, {-13 * C_256QAM, 15 * C_256QAM}, {-15 * C_256QAM, 13 * C_256QAM}, {-15 * C_256QAM, 15 * C_256QAM},

    { -5 * C_256QAM, -5 * C_256QAM}, { -5 * C_256QAM, -7 * C_256QAM}, { -7 * C_256QAM, -5 * C_256QAM}, { -7 * C_256QAM, -7 * C_256QAM},
    { -5 * C_256QAM, -3 * C_256QAM}, { -5 * C_256QAM, -1 * C_256QAM}, { -7 * C_256QAM, -3 * C_256QAM}, { -7 * C_256QAM, -1 * C_256QAM},
    { -3 * C_256QAM, -5 * C_256QAM}, { -3 * C_256QAM, -7 * C_256QAM}, { -1 * C_256QAM, -5 * C_256QAM}, { -1 * C_256QAM, -7 * C_256QAM},
    { -3 * C_256QAM, -3 * C_256QAM}, { -3 * C_256QAM, -1 * C_256QAM}, { -1 * C_256QAM, -3 * C_256QAM}, { -1 * C_256QAM, -1 * C_256QAM},
    { -5 * C_256QAM,-11 * C_256QAM}, { -5 * C_256QAM, -9 * C_256QAM}, { -7 * C_256QAM,-11 * C_256QAM}, { -7 * C_256QAM, -9 * C_256QAM},
    { -5 * C_256QAM,-13 * C_256QAM}, { -5 * C_256QAM,-15 * C_256QAM}, { -7 * C_256QAM,-13 * C_256QAM}, { -7 * C_256QAM,-15 * C_256QAM},
    { -3 * C_256QAM,-11 * C_256QAM}, { -3 * C_256QAM, -9 * C_256QAM}, { -1 * C_256QAM,-11 * C_256QAM}, { -1 * C_256QAM, -9 * C_256QAM},
    { -3 * C_256QAM,-13 * C_256QAM}, { -3 * C_256QAM,-15 * C_256QAM}, { -1 * C_256QAM,-13 * C_256QAM}, { -1 * C_256QAM,-15 * C_256QAM},
    {-11 * C_256QAM, -5 * C_256QAM}, {-11 * C_256QAM, -7 * C_256QAM}, { -9 * C_256QAM, -5 * C_256QAM}, { -9 * C_256QAM, -7 * C_256QAM},
    {-11 * C_256QAM, -3 * C_256QAM}, {-11 * C_256QAM, -1 * C_256QAM}, { -9 * C_256QAM, -3 * C_256QAM}, { -9 * C_256QAM, -1 * C_256QAM},
    {-13 * C_256QAM, -5 * C_256QAM}, {-13 * C_256QAM, -7 * C_256QAM}, {-15 * C_256QAM, -5 * C_256QAM}, {-15 * C_256QAM, -7 * C_256QAM},
    {-13 * C_256QAM, -3 * C_256QAM}, {-13 * C_256QAM, -1 * C_256QAM}, {-15 * C_256QAM, -3 * C_256QAM}, {-15 * C_256QAM, -1 * C_256QAM},
    {-11 * C_256QAM,-11 * C_256QAM}, {-11 * C_256QAM, -9 * C_256QAM}, { -9 * C_256QAM,-11 * C_256QAM}, { -9 * C_256QAM, -9 * C_256QAM},
    {-11 * C_256QAM,-13 * C_256QAM}, {-11 * C_256QAM,-15 * C_256QAM}, { -9 * C_256QAM,-13 * C_256QAM}, { -9 * C_256QAM,-15 * C_256QAM},
    {-13 * C_256QAM,-11 * C_256QAM}, {-13 * C_256QAM, -9 * C_256QAM}, {-15 * C_256QAM,-11 * C_256QAM}, {-15 * C_256QAM, -9 * C_256QAM},
    {-13 * C_256QAM,-13 * C_256QAM}, {-13 * C_256QAM,-15 * C_256QAM}, {-15 * C_256QAM,-13 * C_256QAM}, {-15 * C_256QAM,-15 * C_256QAM}
};

void modulation(const uint8_t *pSrc, T32fc *pDst, uint32_t length, Modulation_t modType)
{
    const T32fc *modTable = NULL;
    switch (modType)
    {
        case Bpsk:
            modTable = modBpsk;
            break;
        case Qpsk:
            modTable = modQpsk;
            break;
        case Qam16:
            modTable = mod16QAM;
            break;
        case Qam64:
            modTable = mod64QAM;
            break;
        case Qam256:
            modTable = mod256QAM;
            break;
        default:
            return;
    }

    for (uint32_t i = 0; i < length; i++)
    {
        pDst[i] = modTable[pSrc[i]];
    }
}



#include <riscv_vector.h>

static const float mod16QAM_IMG[]  = {-3 * C_16QAM, C_16QAM, C_16QAM, 3* C_16QAM};
static const float mod16QAM_REAL[] = {-3 * C_16QAM, C_16QAM, C_16QAM, 3* C_16QAM};

#if __riscv_xlen == 32

void static unzip(const uint8_t *origin, unsigned int *dest, int n)
{
    for (int i = 0; i < n; ++i)
    {
        dest[i] = __builtin_riscv_unzip(origin[i]);
    }
}

#else

void static unzip(const uint8_t *origin, unsigned int *dest, int n)
{
    for (int i = 0; i < n; ++i) {
        uint8_t tmp = origin[i];
        uint8_t odd  = (tmp & 0xA) >> 1;
        uint8_t even = tmp & 0x5;

        odd  = (odd | odd >> 1) & 3;
        even = (even | even >> 1) & 3;

        dest[i] = (odd << 2) | even;
    }
}

#endif

void modulation_opt(const uint8_t *pSrc, T32fc *pDst, uint32_t length, Modulation_t modType)
{
    for (int i = 0; i < length; ++i) {
        unsigned int tmp[1];
        unzip(pSrc, tmp, 1);

        pDst[i].re = mod16QAM_REAL[*tmp & 0xf];
        pDst[i].im = mod16QAM_IMG[(*tmp & 0xf0) >> 2];
    }
}

void modulation_full_opt(const uint8_t *pSrc, float *pDstReal, float *pDstImg, uint32_t length, Modulation_t modType)
{
    vfloat32m1_t tableImg  = __riscv_vle32_v_f32m1(mod16QAM_IMG, 4);
    vfloat32m1_t tableReal = __riscv_vle32_v_f32m1(mod16QAM_REAL, 4);

    const size_t VL = 4; // __riscv_vsetvlmax_e32m1();

    for (int i = 0; i < length; i += VL) {

        unsigned int tmp[VL];

        unzip(pSrc, tmp, VL);

        vuint32m1_t ind = __riscv_vle32_v_u32m1(tmp, VL);

        vuint32m1_t indImg = __riscv_vand_vx_u32m1(ind, 0x3, VL);

        vuint32m1_t indReal = __riscv_vand_vx_u32m1(ind, 0x12, VL);
        indReal = __riscv_vsrl_vx_u32m1(indReal, 2, VL);

        vfloat32m1_t resReal = __riscv_vrgather_vv_f32m1(tableReal, indReal, 4);
        vfloat32m1_t resImg  = __riscv_vrgather_vv_f32m1(tableImg, indImg, 4);

        __riscv_vse32_v_f32m1(pDstReal, resReal, VL);
        __riscv_vse32_v_f32m1(pDstImg, resImg, VL);
    }
}