#include <stdint.h>
#include <stddef.h>
#include <math.h>

#include "LTEmod.h"
// LTE Modulation

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

void modulation(const uint32_t *pSrc, T32fc *pDst, uint32_t length)
{
    const T32fc *modTable = mod256QAM;

    for (uint32_t i = 0; i < length; i++)
    {
        pDst[i] = modTable[pSrc[i]];
    }
}



#include <riscv_vector.h>

static const float mod256QAM_IMG[]  = {-15 * C_256QAM, -13 * -C_256QAM, -11 * C_256QAM, -9 * C_256QAM, -7 * C_256QAM, -5 * C_256QAM, -3 * C_256QAM, -C_256QAM,
	15 * C_256QAM, 13 * -C_256QAM, 11 * C_256QAM, 9 * C_256QAM, 7 * C_256QAM, 5 * C_256QAM, 3 * C_256QAM, C_256QAM};

static const float mod256QAM_REAL[]  = {-15 * C_256QAM, -13 * -C_256QAM, -11 * C_256QAM, -9 * C_256QAM, -7 * C_256QAM, -5 * C_256QAM, -3 * C_256QAM, -C_256QAM,
	15 * C_256QAM, 13 * -C_256QAM, 11 * C_256QAM, 9 * C_256QAM, 7 * C_256QAM, 5 * C_256QAM, 3 * C_256QAM, C_256QAM};

#if __riscv_xlen == 32

void static unzip(const uint32_t *origin, unsigned int *dest, int n)
{
    for (int i = 0; i < n; ++i)
    {
        dest[i] = __builtin_riscv_unzip(origin[i]);
    }
}

#else

#include <stdio.h>

void static unzip(const uint32_t *origin, unsigned int *dest, int n)
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

void modulation_opt(const uint32_t *pSrc, T32fc *pDst, uint32_t length)
{
    for (int i = 0; i < length; ++i) {
        unsigned int tmp[1];
        unzip(pSrc, tmp, 1);

        pDst[i].re = mod256QAM_REAL[*tmp & 0xf];
        pDst[i].im = mod256QAM_IMG[(*tmp & 0xf0) >> 2];
    }
}

void modulation_full_opt(const uint32_t *pSrc, float *pDstReal, float *pDstImg, uint32_t length)
{
    vfloat32m1_t tableImg  = __riscv_vle32_v_f32m1(mod256QAM_IMG, 4);
    vfloat32m1_t tableReal = __riscv_vle32_v_f32m1(mod256QAM_REAL, 4);

    const size_t VL = 4; // __riscv_vsetvlmax_e32m1();

    unsigned int tmp[VL];

    for (int i = 0; i < length; i += VL * 4) {
        vuint32m1_t ind0 = __riscv_vle32_v_u32m1(pSrc, VL);
        vuint32m1_t ind1 = __riscv_vle32_v_u32m1(pSrc + 4, VL);
        vuint32m1_t ind2 = __riscv_vle32_v_u32m1(pSrc + 8, VL);
        vuint32m1_t ind3 = __riscv_vle32_v_u32m1(pSrc + 12, VL);

        vuint32m1_t indImg0 = __riscv_vand_vx_u32m1(ind0, 0xf, VL);
        vuint32m1_t indImg1 = __riscv_vand_vx_u32m1(ind1, 0xf, VL);
        vuint32m1_t indImg2 = __riscv_vand_vx_u32m1(ind2, 0xf, VL);
        vuint32m1_t indImg3 = __riscv_vand_vx_u32m1(ind3, 0xf, VL);

        vuint32m1_t indReal0 = __riscv_vand_vx_u32m1(ind0, 0xf0, VL);
        indReal0 = __riscv_vsrl_vx_u32m1(indReal0, 4, VL);
        vuint32m1_t indReal1 = __riscv_vand_vx_u32m1(ind1, 0xf0, VL);
        indReal1 = __riscv_vsrl_vx_u32m1(indReal1, 4, VL);
        vuint32m1_t indReal2 = __riscv_vand_vx_u32m1(ind2, 0xf0, VL);
        indReal2 = __riscv_vsrl_vx_u32m1(indReal2, 4, VL);
        vuint32m1_t indReal3 = __riscv_vand_vx_u32m1(ind3, 0xf0, VL);
        indReal3 = __riscv_vsrl_vx_u32m1(indReal3, 4, VL);

        vfloat32m1_t resReal0 = __riscv_vrgather_vv_f32m1(tableReal, indReal0, 4);
        vfloat32m1_t resImg0  = __riscv_vrgather_vv_f32m1(tableImg, indImg0, 4);
        
	vfloat32m1_t resReal1 = __riscv_vrgather_vv_f32m1(tableReal, indReal1, 4);
        vfloat32m1_t resImg1  = __riscv_vrgather_vv_f32m1(tableImg, indImg1, 4);
        
	vfloat32m1_t resReal2 = __riscv_vrgather_vv_f32m1(tableReal, indReal2, 4);
        vfloat32m1_t resImg2  = __riscv_vrgather_vv_f32m1(tableImg, indImg2, 4);
        
	vfloat32m1_t resReal3 = __riscv_vrgather_vv_f32m1(tableReal, indReal3, 4);
        vfloat32m1_t resImg3  = __riscv_vrgather_vv_f32m1(tableImg, indImg3, 4);

        __riscv_vse32_v_f32m1(pDstReal, resReal0, VL);
        __riscv_vse32_v_f32m1(pDstImg, resImg0, VL);
        
	__riscv_vse32_v_f32m1(pDstReal + 4, resReal1, VL);
        __riscv_vse32_v_f32m1(pDstImg + 4, resImg1, VL);
        
	__riscv_vse32_v_f32m1(pDstReal + 8, resReal2, VL);
        __riscv_vse32_v_f32m1(pDstImg + 8, resImg2, VL);
        
	__riscv_vse32_v_f32m1(pDstReal + 12, resReal3, VL);
        __riscv_vse32_v_f32m1(pDstImg + 12, resImg3, VL);

	pDstReal += VL * 4;
	pDstImg  += VL * 4;
	pSrc 	 += VL * 4;
    }
}
