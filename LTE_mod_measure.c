#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <riscv_vector.h>

typedef struct {
    int16_t re;
    int16_t im;
} T16sc;

static uint64_t getCycles()
{
    uint64_t cycles;
    asm volatile("csrrs %0, 0xc00, x0" : "=r"(cycles));
    return cycles;
}

typedef enum
{
    Bpsk,
    Qpsk,
    Qam16,
    Qam64,
    Qam256,
    QAM_N,
} Modulation_t;

static const char* const modulationNames[] = {
    [Bpsk]   = "Bpsk",
    [Qpsk]   = "Qpsk",
    [Qam16]  = "Qam16",
    [Qam64]  = "Qam64",
    [Qam256] = "Qam256"
};

static const int16_t C_PSK    = (int16_t)(16384.f / sqrt(2.f));
static const int16_t C_16QAM  = (int16_t)(16384.f / sqrt(10.f));
static const int16_t C_64QAM  = (int16_t)(16384.f / sqrt(42.f));
static const int16_t C_256QAM = (int16_t)(16384.f / sqrt(170.f));

static const T16sc modBpsk[] =
{
    {C_PSK, C_PSK}, {-C_PSK, -C_PSK}
};

static const T16sc modQpsk[] =
{
    {C_PSK, C_PSK}, {C_PSK, -C_PSK}, {-C_PSK, C_PSK}, {-C_PSK, -C_PSK}
};

static const T16sc mod16QAM[] =
{
    { C_16QAM,  C_16QAM}, { C_16QAM,  3 * C_16QAM}, { 3 * C_16QAM,  C_16QAM}, { 3 * C_16QAM,  3 * C_16QAM},
    { C_16QAM, -C_16QAM}, { C_16QAM, -3 * C_16QAM}, { 3 * C_16QAM, -C_16QAM}, { 3 * C_16QAM, -3 * C_16QAM},
    {-C_16QAM,  C_16QAM}, {-C_16QAM,  3 * C_16QAM}, {-3 * C_16QAM,  C_16QAM}, {-3 * C_16QAM,  3 * C_16QAM},
    {-C_16QAM, -C_16QAM}, {-C_16QAM, -3 * C_16QAM}, {-3 * C_16QAM, -C_16QAM}, {-3 * C_16QAM, -3 * C_16QAM}
};

static const int16_t mod16QAM_LUT[]  = {
    C_16QAM, 3 * C_16QAM, -C_16QAM, 3 * -C_16QAM, C_16QAM, 3 * C_16QAM, -C_16QAM, 3 * -C_16QAM,
    C_16QAM, 3 * C_16QAM, -C_16QAM, 3 * -C_16QAM, C_16QAM, 3 * C_16QAM, -C_16QAM, 3 * -C_16QAM,
    C_16QAM, 3 * C_16QAM, -C_16QAM, 3 * -C_16QAM, C_16QAM, 3 * C_16QAM, -C_16QAM, 3 * -C_16QAM,
    C_16QAM, 3 * C_16QAM, -C_16QAM, 3 * -C_16QAM, C_16QAM, 3 * C_16QAM, -C_16QAM, 3 * -C_16QAM
};

static const T16sc mod64QAM[] =
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

static const int16_t mod64QAM_LUT[]  = {
    3 * C_64QAM, C_64QAM, 5 * C_64QAM, 7 * C_64QAM, -3 * C_64QAM, -C_64QAM, -7 * C_64QAM, -5 * C_64QAM,
    3 * C_64QAM, C_64QAM, 5 * C_64QAM, 7 * C_64QAM, -3 * C_64QAM, -C_64QAM, -7 * C_64QAM, -5 * C_64QAM,
    3 * C_64QAM, C_64QAM, 5 * C_64QAM, 7 * C_64QAM, -3 * C_64QAM, -C_64QAM, -7 * C_64QAM, -5 * C_64QAM,
    3 * C_64QAM, C_64QAM, 5 * C_64QAM, 7 * C_64QAM, -3 * C_64QAM, -C_64QAM, -7 * C_64QAM, -5 * C_64QAM
};

static const T16sc mod256QAM[] =
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

static const int16_t mod256QAM_LUT[]  = {
     5 * C_256QAM,  7 * C_256QAM,  3 * C_256QAM,  1 * C_256QAM,   11 * C_256QAM,  9 * C_256QAM,   13 * C_256QAM,  15 * C_256QAM,
    -5 * C_256QAM, -7 * C_256QAM, -3 * C_256QAM, -1 * C_256QAM,  -11 * C_256QAM, -9 * C_256QAM,  -13 * C_256QAM, -15 * C_256QAM,
     5 * C_256QAM,  7 * C_256QAM,  3 * C_256QAM,  1 * C_256QAM,   11 * C_256QAM,  9 * C_256QAM,   13 * C_256QAM,  15 * C_256QAM,
    -5 * C_256QAM, -7 * C_256QAM, -3 * C_256QAM, -1 * C_256QAM,  -11 * C_256QAM, -9 * C_256QAM,  -13 * C_256QAM, -15 * C_256QAM,
};

static const T16sc *getModTable(Modulation_t modType)
{
    switch (modType)
    {
        case Bpsk:
            return modBpsk;
        case Qpsk:
            return modQpsk;
        case Qam16:
            return mod16QAM;
        case Qam64:
            return mod64QAM;
        case Qam256:
            return mod256QAM;
        default:
            return NULL;
    }
} 

void modulation(const uint8_t *pSrc, T16sc *pDst, uint32_t length, const T16sc *modTable)
{
    for (uint32_t i = 0; i < length; i++)
    {
        pDst[i] = modTable[pSrc[i]];
    }
}

void optBPSK(const uint8_t *pSrc, T16sc *pDst, uint32_t length)
{
    for (uint32_t i = 0; i < length;)
    {
        size_t vl =  __riscv_vsetvl_e8m2(length - i);

        vint8m2_t src = __riscv_vle8_v_i8m2(pSrc + i, vl);
        vint16m4_t out = __riscv_vmv_v_x_i16m4(C_PSK, vl);

        vbool4_t mask = __riscv_vmseq_vx_i8m2_b4(src, 1, vl);
        //out = __riscv_vmerge_vxm_i16m4(mask, out, -C_PSK, vl);
        vint16m4_t outDup = __riscv_vmv_v_v_i16m4(out, vl);

        //__riscv_vsseg2e16_v_i16m4((int16_t*)pDst + i * 2, out, outDup, vl);

        i += vl;
    }
}

void optQPSK(const uint8_t *pSrc, T16sc *pDst, uint32_t length)
{
    for (uint32_t i = 0; i < length;)
    {
        size_t vl =  __riscv_vsetvl_e8m2(length - i);

        vint16m4_t outRe = __riscv_vmv_v_x_i16m4(C_PSK, vl);
        vint16m4_t outIm = __riscv_vmv_v_x_i16m4(C_PSK, vl);

        vint8m2_t src = __riscv_vle8_v_i8m2(pSrc + i, vl);
        vint8m2_t srcRe = __riscv_vand_vx_i8m2(src, 0x2, vl);
        vint8m2_t srcIm = __riscv_vand_vx_i8m2(src, 0x1, vl);

        vbool4_t maskRe = __riscv_vmseq_vx_i8m2_b4(srcRe, 0x2, vl);
        //outRe = __riscv_vmerge_vxm_i16m4(maskRe, outRe, -C_PSK, vl);
        vbool4_t maskIm = __riscv_vmseq_vx_i8m2_b4(srcIm, 0x1, vl);
        //outIm = __riscv_vmerge_vxm_i16m4(maskIm, outIm, -C_PSK, vl);

        //__riscv_vsseg2e16_v_i16m4((int16_t*)pDst + i * 2, outRe, outIm, vl);

        i += vl;
    }
}

void optQAM16(const uint8_t *pSrc, T16sc *pDst, uint32_t length)
{
    size_t table_vl =  __riscv_vsetvl_e16m1(sizeof(mod16QAM_LUT) / sizeof(*mod16QAM_LUT));

    vint16m4_t tableRe = __riscv_vle16_v_i16m4(mod16QAM_LUT, table_vl);
    vint16m4_t tableIm = __riscv_vmv_v_v_i16m4(tableRe, table_vl);

    for (uint32_t i = 0; i < length;)
    {
        size_t vl =  __riscv_vsetvl_e8m2(length - i);

        vuint8m2_t src = __riscv_vle8_v_u8m2(pSrc + i, vl);

        vuint8m2_t b2 = __riscv_vand_vx_u8m2(src, 0x4, vl);
        vuint8m2_t b1 = __riscv_vand_vx_u8m2(src, 0x2, vl);
        
        vuint8m2_t swapped = __riscv_vor_vv_u8m2(
            __riscv_vsrl_vx_u8m2(b2, 1, vl),
            __riscv_vsll_vx_u8m2(b1, 1, vl), vl);

        vuint8m2_t grouped = __riscv_vand_vx_u8m2(src, 0x9, vl);
        grouped = __riscv_vor_vv_u8m2(grouped, swapped, vl);

        vuint16m4_t srcRe = __riscv_vwcvtu_x_x_v_u16m4(__riscv_vand_vx_u8m2(grouped, 0xC, vl), vl);
        vuint16m4_t srcIm = __riscv_vwcvtu_x_x_v_u16m4(__riscv_vand_vx_u8m2(grouped, 0x3, vl), vl);

        vint16m4_t outRe = __riscv_vrgather_vv_i16m4(tableRe, srcRe, vl);
        vint16m4_t outIm = __riscv_vrgather_vv_i16m4(tableIm, srcIm, vl);

        //__riscv_vsseg2e16_v_i16m4((int16_t*)pDst + i * 2, outRe, outIm, vl);

        i += vl;
    }
}

void optQAM16_proxy(const uint8_t *pSrc, T16sc *pDst, uint32_t length)
{
    size_t table_vl =  __riscv_vsetvl_e16m1(sizeof(mod16QAM_LUT) / sizeof(*mod16QAM_LUT));

    vint16m4_t tableRe = __riscv_vle16_v_i16m4(mod16QAM_LUT, table_vl);
    vint16m4_t tableIm = __riscv_vmv_v_v_i16m4(tableRe, table_vl);

    for (uint32_t i = 0; i < length;)
    {
        size_t vl =  __riscv_vsetvl_e8m2(length - i);

        vuint8m2_t src = __riscv_vle8_v_u8m2(pSrc + i, vl);

        vuint8m2_t grouped = __riscv_vand_vx_u8m2(src, 0x9, vl); // assume it is vector unzip

        vuint16m4_t srcRe = __riscv_vwcvtu_x_x_v_u16m4(__riscv_vand_vx_u8m2(grouped, 0xC, vl), vl);
        vuint16m4_t srcIm = __riscv_vwcvtu_x_x_v_u16m4(__riscv_vand_vx_u8m2(grouped, 0x3, vl), vl);

        vint16m4_t outRe = __riscv_vrgather_vv_i16m4(tableRe, srcRe, vl);
        vint16m4_t outIm = __riscv_vrgather_vv_i16m4(tableIm, srcIm, vl);

        //__riscv_vsseg2e16_v_i16m4((int16_t*)pDst + i * 2, outRe, outIm, vl);

        i += vl;
    }
}

void optQAM64(const uint8_t *pSrc, T16sc *pDst, uint32_t length)
{
    size_t table_vl =  __riscv_vsetvl_e16m1(sizeof(mod64QAM_LUT) / sizeof(*mod64QAM_LUT));

    vint16m4_t tableRe = __riscv_vle16_v_i16m4(mod64QAM_LUT, table_vl);
    vint16m4_t tableIm = __riscv_vmv_v_v_i16m4(tableRe, table_vl);

    for (uint32_t i = 0; i < length;)
    {
        size_t vl =  __riscv_vsetvl_e8m2(length - i);

        vuint8m2_t src = __riscv_vle8_v_u8m2(pSrc + i, vl);

        vuint8m2_t b4 = __riscv_vand_vx_u8m2(src, 0x10, vl);
        vuint8m2_t b3 = __riscv_vand_vx_u8m2(src, 0x08, vl);
        vuint8m2_t b2 = __riscv_vand_vx_u8m2(src, 0x04, vl);
        vuint8m2_t b1 = __riscv_vand_vx_u8m2(src, 0x02, vl);
        
        vuint8m2_t swapped = __riscv_vor_vv_u8m2(
            __riscv_vor_vv_u8m2(
                __riscv_vsll_vx_u8m2(b1, 2, vl),
                __riscv_vsrl_vx_u8m2(b2, 1, vl), vl),
            __riscv_vor_vv_u8m2(
                __riscv_vsll_vx_u8m2(b3, 1, vl),
                __riscv_vsrl_vx_u8m2(b4, 2, vl), vl),
            vl);

        vuint8m2_t grouped = __riscv_vand_vx_u8m2(src, 0x21, vl);
        grouped = __riscv_vor_vv_u8m2(grouped, swapped, vl);

        vuint16m4_t srcRe = __riscv_vwcvtu_x_x_v_u16m4(__riscv_vand_vx_u8m2(grouped, 0x38, vl), vl);
        vuint16m4_t srcIm = __riscv_vwcvtu_x_x_v_u16m4(__riscv_vand_vx_u8m2(grouped, 0x07, vl), vl);

        vint16m4_t outRe = __riscv_vrgather_vv_i16m4(tableRe, srcRe, vl);
        vint16m4_t outIm = __riscv_vrgather_vv_i16m4(tableIm, srcIm, vl);

        //__riscv_vsseg2e16_v_i16m4((int16_t*)pDst + i * 2, outRe, outIm, vl);

        i += vl;
    }
}

void optQAM64_proxy(const uint8_t *pSrc, T16sc *pDst, uint32_t length)
{
    size_t table_vl =  __riscv_vsetvl_e16m1(sizeof(mod64QAM_LUT) / sizeof(*mod64QAM_LUT));

    vint16m4_t tableRe = __riscv_vle16_v_i16m4(mod64QAM_LUT, table_vl);
    vint16m4_t tableIm = __riscv_vmv_v_v_i16m4(tableRe, table_vl);

    for (uint32_t i = 0; i < length;)
    {
        size_t vl =  __riscv_vsetvl_e8m2(length - i);

        vuint8m2_t src = __riscv_vle8_v_u8m2(pSrc + i, vl);

        vuint8m2_t grouped = __riscv_vand_vx_u8m2(src, 0x9, vl); // assume it is vector unzip

        vuint16m4_t srcRe = __riscv_vwcvtu_x_x_v_u16m4(__riscv_vand_vx_u8m2(grouped, 0x38, vl), vl);
        vuint16m4_t srcIm = __riscv_vwcvtu_x_x_v_u16m4(__riscv_vand_vx_u8m2(grouped, 0x07, vl), vl);

        vint16m4_t outRe = __riscv_vrgather_vv_i16m4(tableRe, srcRe, vl);
        vint16m4_t outIm = __riscv_vrgather_vv_i16m4(tableIm, srcIm, vl);

        //__riscv_vsseg2e16_v_i16m4((int16_t*)pDst + i * 2, outRe, outIm, vl);

        i += vl;
    }
}

void optQAM256(const uint8_t *pSrc, T16sc *pDst, uint32_t length)
{
    size_t table_vl =  __riscv_vsetvl_e16m1(sizeof(mod256QAM_LUT) / sizeof(*mod256QAM_LUT));

    vint16m4_t tableRe = __riscv_vle16_v_i16m4(mod256QAM_LUT, table_vl);
    vint16m4_t tableIm = __riscv_vmv_v_v_i16m4(tableRe, table_vl);

    for (uint32_t i = 0; i < length;)
    {
        size_t vl =  __riscv_vsetvl_e8m2(length - i);

        vuint8m2_t src = __riscv_vle8_v_u8m2(pSrc + i, vl);

        vuint8m2_t b6 = __riscv_vand_vx_u8m2(src, 0x40, vl);
        vuint8m2_t b5 = __riscv_vand_vx_u8m2(src, 0x20, vl);
        vuint8m2_t b4 = __riscv_vand_vx_u8m2(src, 0x10, vl);
        vuint8m2_t b3 = __riscv_vand_vx_u8m2(src, 0x08, vl);
        vuint8m2_t b2 = __riscv_vand_vx_u8m2(src, 0x04, vl);
        vuint8m2_t b1 = __riscv_vand_vx_u8m2(src, 0x02, vl);
        
        vuint8m2_t swapped = __riscv_vor_vv_u8m2(
            __riscv_vor_vv_u8m2(
                __riscv_vsll_vx_u8m2(b1, 3, vl),
                __riscv_vsrl_vx_u8m2(b2, 1, vl), vl),
            __riscv_vor_vv_u8m2(
                __riscv_vsll_vx_u8m2(b3, 2, vl),
                __riscv_vsrl_vx_u8m2(b4, 2, vl), vl),
            vl);

        swapped = __riscv_vor_vv_u8m2(swapped, 
            __riscv_vor_vv_u8m2(
                __riscv_vsll_vx_u8m2(b5, 1, vl),
                __riscv_vsrl_vx_u8m2(b6, 3, vl), vl), 
            vl);

        vuint8m2_t grouped = __riscv_vand_vx_u8m2(src, 0x81, vl);
        grouped = __riscv_vor_vv_u8m2(grouped, swapped, vl);

        vuint16m4_t srcRe = __riscv_vwcvtu_x_x_v_u16m4(__riscv_vand_vx_u8m2(grouped, 0xF0, vl), vl);
        vuint16m4_t srcIm = __riscv_vwcvtu_x_x_v_u16m4(__riscv_vand_vx_u8m2(grouped, 0x0F, vl), vl);

        vint16m4_t outRe = __riscv_vrgather_vv_i16m4(tableRe, srcRe, vl);
        vint16m4_t outIm = __riscv_vrgather_vv_i16m4(tableIm, srcIm, vl);

        //__riscv_vsseg2e16_v_i16m4((int16_t*)pDst + i * 2, outRe, outIm, vl);

        i += vl;
    }
}

void optQAM256_proxy(const uint8_t *pSrc, T16sc *pDst, uint32_t length)
{
    size_t table_vl =  __riscv_vsetvl_e16m1(sizeof(mod256QAM_LUT) / sizeof(*mod256QAM_LUT));

    vint16m4_t tableRe = __riscv_vle16_v_i16m4(mod256QAM_LUT, table_vl);
    vint16m4_t tableIm = __riscv_vmv_v_v_i16m4(tableRe, table_vl);

    for (uint32_t i = 0; i < length;)
    {
        size_t vl =  __riscv_vsetvl_e8m2(length - i);

        vuint8m2_t src = __riscv_vle8_v_u8m2(pSrc + i, vl);

        vuint8m2_t grouped = __riscv_vand_vx_u8m2(src, 0x9, vl); // assume it is vector unzip

        vuint16m4_t srcRe = __riscv_vwcvtu_x_x_v_u16m4(__riscv_vand_vx_u8m2(grouped, 0xF0, vl), vl);
        vuint16m4_t srcIm = __riscv_vwcvtu_x_x_v_u16m4(__riscv_vand_vx_u8m2(grouped, 0x0F, vl), vl);

        vint16m4_t outRe = __riscv_vrgather_vv_i16m4(tableRe, srcRe, vl);
        vint16m4_t outIm = __riscv_vrgather_vv_i16m4(tableIm, srcIm, vl);

        //__riscv_vsseg2e16_v_i16m4((int16_t*)pDst + i * 2, outRe, outIm, vl);

        i += vl;
    }
}

int main()
{
    size_t lengths[] = {12, 24, 36, 48, 60, 72, 96, 108, 120, 144, 
        180, 192, 216, 240, 288, 300, 324, 360, 384, 432, 480, 540, 
        576, 600, 648, 720, 768, 864, 900, 960, 972, 1080, 1152, 1200};

    size_t lengths_N = sizeof(lengths) / sizeof(*lengths);
    
    size_t repeats = 1000;

    FILE *resOutput = fopen("modulRes.csv", "w");

    #define BENCHMARK_MODULATION(FUNC_TYPE, QAM_I, MOD_FUNC)                        \
    do {                                                                            \
        for (size_t length_i = 0; length_i < lengths_N; ++length_i) {               \
            uint32_t length = lengths[length_i];                                    \
            uint8_t *pSrc = (uint8_t*)malloc(sizeof(uint8_t) * length);             \
            for (size_t i = 0; i < length; ++i) {                                   \
                pSrc[i] = rand();                                                   \
            }                                                                       \
            T16sc *pDst = (T16sc*)malloc(sizeof(T16sc) * length);                   \
                                                                                    \
            fprintf(resOutput, FUNC_TYPE",%s,%u", modulationNames[QAM_I], length);  \
            for (size_t i = 0; i < repeats; ++i) {                                  \
                uint64_t start = getCycles();                                       \
                MOD_FUNC;                                                           \
                uint64_t finish = getCycles();                                      \
                uint64_t elapsed = finish - start;                                  \
                fprintf(resOutput, ",%lu", (unsigned long)elapsed);                 \
                volatile int res = pDst[2].re; /* prevent optimization */           \
                (void)res;                                                          \
            }                                                                       \
            fprintf(resOutput, "\n");                                               \
            free(pSrc);                                                             \
            free(pDst);                                                             \
        }                                                                           \
    } while(0)

    for (size_t qam_i = 0; qam_i < (size_t)QAM_N; ++qam_i ) {
        const T16sc *modTable = getModTable((Modulation_t)qam_i);
        BENCHMARK_MODULATION("origin", qam_i, modulation(pSrc, pDst, length, modTable));
    }

    BENCHMARK_MODULATION("optimized", Bpsk,   optBPSK(pSrc, pDst, length));
    BENCHMARK_MODULATION("optimized", Qpsk,   optQPSK(pSrc, pDst, length));
    BENCHMARK_MODULATION("optimized", Qam16,  optQAM16(pSrc, pDst, length));
    BENCHMARK_MODULATION("optimized", Qam64,  optQAM64(pSrc, pDst, length));
    BENCHMARK_MODULATION("optimized", Qam256, optQAM256(pSrc, pDst, length));

    BENCHMARK_MODULATION("proxy", Qam16,  optQAM16_proxy(pSrc, pDst, length));
    BENCHMARK_MODULATION("proxy", Qam64,  optQAM64_proxy(pSrc, pDst, length));
    BENCHMARK_MODULATION("proxy", Qam256, optQAM256_proxy(pSrc, pDst, length));

    #undef BENCHMARK_MODULATION
}