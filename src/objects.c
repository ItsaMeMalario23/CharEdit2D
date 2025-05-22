#include <objects.h>

#define BLACK   0x00000000
#define WHITE   0xffffff00
#define GOLD    0xfcd30300
#define RED     0xff000000
#define LGREEN  0x40d80000
#define MGREEN  0x0d700000
#define DGREEN  0x00300000

const ascii2info_t o_asciiBird[16] = {
    {'\\', WHITE, {  2, 33}}, {'-', WHITE, {14, -5}}, {'-', WHITE, {31, -5}}, {'\\', WHITE, {47,  2}},
    { '(', WHITE, {-12,  9}}, {'@',  GOLD, { 3, 10}}, {'O', WHITE, {32,  6}}, { '>',   RED, {55, 18}},
    {'\'', BLACK, { 33, 10}}, {'_', WHITE, {18, 32}}, {'_', WHITE, {33, 32}}, { '/', WHITE, {49, 32}},
    { '>',   RED, { 46, 18}}, {')', WHITE, {17, 11}}, {'B',  GOLD, {17, 28}}, { 'D',  GOLD, {34, 28}}
};

const ascii2info_t o_asciiPipeHeadTop[37] = {
    {'_', LGREEN, {  0, -14}}, {'_', LGREEN, {16, -14}}, {'_', LGREEN, { 32, -14}}, { '_', LGREEN, {48, -14}},
    {'_', LGREEN, { 64, -14}},
    {'#', DGREEN, { 42,  12}}, {'E', DGREEN, {58,  12}}, {'#', DGREEN, { 42,  28}}, { 'E', DGREEN, {58,  28}},
    {'_', DGREEN, {  1,  -2}}, {']', DGREEN, {16,  12}}, {']', DGREEN, { 16,  28}},
    {'!', MGREEN, { 28,  12}}, {'!', MGREEN, {28,  28}}, {'#', MGREEN, {  2,  28}}, {'\"', MGREEN, { 2,  17}},
    {'=', MGREEN, {  3,   0}}, {'=', MGREEN, {22,   0}}, {'=', MGREEN, { 42,   0}}, { '=', MGREEN, {61,   0}},
    {'`', MGREEN, { 66,  16}}, {'`', MGREEN, {66,  25}}, {'`', MGREEN, { 66,  34}},
    {'|', LGREEN, {-10,   1}}, {'|', LGREEN, {72,   1}}, {'|', LGREEN, {-10,  17}}, { '|', LGREEN, {72,  17}},
    {'|', LGREEN, {-10,  33}}, {'|', LGREEN, {72,  33}},
    {'T', LGREEN, {  0,  45}}, {'=', LGREEN, {16,  41}}, {'=', LGREEN, { 32,  41}}, { '=', LGREEN, {48,  41}},
    {'T', LGREEN, { 62,  45}},
    {'_', MGREEN, { 14,  41}}, {'_', MGREEN, {32,  41}}, {'_', MGREEN, { 50,  41}}
};

const ascii2info_t o_asciiPipeHeadBot[39] = {
    {'_', MGREEN, { 14, -23}}, {'_', MGREEN, {32, -23}}, {'_', MGREEN, { 50, -23}}, 
    {'-', LGREEN, {  0,  -5}}, {'=', LGREEN, {16,  -7}}, {'=', LGREEN, { 32,  -7}}, { '=', LGREEN, {48, -7}},
    {'-', LGREEN, { 62,  -5}}, 
    {'|', LGREEN, {-10,  33}}, {'|', LGREEN, {72,  33}}, {'|', LGREEN, {-10,  17}}, { '|', LGREEN, {72, 17}},
    {'|', LGREEN, {-10,   1}}, {'|', LGREEN, {72,   1}}, 
    {'`', MGREEN, { 66,  10}}, {'`', MGREEN, {66,  20}}, {'`', MGREEN, { 66,  30}}, 
    {'=', MGREEN, {  3,  34}}, {'=', MGREEN, {22,  34}}, {'=', MGREEN, { 42,  34}}, { '=', MGREEN, {61, 34}}, 
    {'!', MGREEN, { 28,  22}}, {'!', MGREEN, {28,   6}}, {'#', MGREEN, {  2,   6}}, {'\"', MGREEN, { 2, 25}}, 
    {'_', DGREEN, {  1,  20}}, {']', DGREEN, {16,  22}}, {']', DGREEN, { 16,   6}}, 
    {'#', DGREEN, { 42,  22}}, {'E', DGREEN, {58,  22}}, {'#', DGREEN, { 42,   6}}, { 'E', DGREEN, {58,  6}}, 
    {'_', LGREEN, {  0,  32}}, {'_', LGREEN, {16,  32}}, {'_', LGREEN, { 32,  32}}, { '_', LGREEN, {48, 32}},
    {'_', LGREEN, { 64,  32}}, {'|', LGREEN, { 0, -12}}, {'|', LGREEN, { 62, -12}}
};

const ascii2info_t o_asciiPipeSection[9] = {
    { '|', LGREEN, { 0, 61}}, {'-', DGREEN, {10, 55}}, {']', DGREEN, {24, 61}}, 
    { '!', MGREEN, {34, 61}}, {'E', DGREEN, {47, 61}}, {'|', LGREEN, {62, 61}}, 
    {'\"', MGREEN, {12, 67}}, {'`', MGREEN, {56, 63}}, {'`', MGREEN, {56, 69}}
};

const ascii2info_t o_circleww[16] = {
    {'G', WHITE, {100.0, 200.0}}, {'G', WHITE, {300.0, 200.0}}, {'G', WHITE, {139.0, 120.7}}, {'G', WHITE, {139.0, 279.3}}, 
    {'G', WHITE, {176.5, 102.8}}, {'G', WHITE, {176.5, 297.2}}, {'G', WHITE, {211.1, 100.6}}, {'G', WHITE, {211.1, 299.4}},
    {'G', WHITE, {241.4, 109.0}}, {'G', WHITE, {241.4, 291.0}}, {'G', WHITE, {266.3, 125.1}}, {'G', WHITE, {266.3, 274.9}}, 
    {'G', WHITE, {284.8, 147.0}}, {'G', WHITE, {284.8, 253.0}}, {'G', WHITE, {296.2, 172.5}}, {'G', WHITE, {296.2, 227.5}},
};

const ascii2info_t o_circle[16] = {
    {'G', WHITE, {100.0f, 200.0f}},
    {'G', WHITE, {300.0f, 200.0f}},
    {'G', WHITE, {107.6f, 161.7f}}, {'G', WHITE, {107.6f, 238.3f}},
    {'G', WHITE, {292.4f, 161.7f}}, {'G', WHITE, {292.4f, 238.3f}},
    {'G', WHITE, {129.3f, 129.3f}}, {'G', WHITE, {129.3f, 270.7f}},
    {'G', WHITE, {270.7f, 129.3f}}, {'G', WHITE, {270.7f, 270.7f}},
    {'G', WHITE, {161.7f, 107.6f}}, {'G', WHITE, {161.7f, 292.4f}},
    {'G', WHITE, {238.3f, 107.6f}}, {'G', WHITE, {238.3f, 292.4f}},
    {'G', WHITE, {200.0f, 100.0f}}, {'G', WHITE, {200.0f, 300.0f}},
};

const ascii2info_t c92[12] = {
    {'G', WHITE, {108.0f, 200.0f}},
    {'G', WHITE, {292.0f, 200.0f}},
    {'G', WHITE, {120.3f, 154.0f}}, {'G', WHITE, {120.3f, 246.0f}},
    {'G', WHITE, {279.7f, 154.0f}}, {'G', WHITE, {279.7f, 246.0f}},
    {'G', WHITE, {154.0f, 120.3f}}, {'G', WHITE, {154.0f, 279.7f}},
    {'G', WHITE, {246.0f, 120.3f}}, {'G', WHITE, {246.0f, 279.7f}},
    {'G', WHITE, {200.0f, 108.0f}}, {'G', WHITE, {200.0f, 292.0f}},
};

const ascii2info_t c71[12] = {
    {'G', WHITE, {129.0f, 200.0f}},
    {'G', WHITE, {271.0f, 200.0f}},
    {'G', WHITE, {138.5f, 164.5f}}, {'G', WHITE, {138.5f, 235.5f}},
    {'G', WHITE, {261.5f, 164.5f}}, {'G', WHITE, {261.5f, 235.5f}},
    {'G', WHITE, {164.5f, 138.5f}}, {'G', WHITE, {164.5f, 261.5f}},
    {'G', WHITE, {235.5f, 138.5f}}, {'G', WHITE, {235.5f, 261.5f}},
    {'G', WHITE, {200.0f, 129.0f}}, {'G', WHITE, {200.0f, 271.0f}},
};

const ascii2info_t c38[8] = {
    {'G', WHITE, {162.0f, 200.0f}},
    {'G', WHITE, {238.0f, 200.0f}},
    {'G', WHITE, {173.1f, 173.1f}}, {'G', WHITE, {173.1f, 226.9f}},
    {'G', WHITE, {226.9f, 173.1f}}, {'G', WHITE, {226.9f, 226.9f}},
    {'G', WHITE, {200.0f, 162.0f}}, {'G', WHITE, {200.0f, 238.0f}},
};

const ascii2info_t pillartop[13] = {
    {'=', 0xffffffff, {2, -2}}, {'=', 0xffffffff, {18, -2}}, {'=', 0xffffffff, {34, -2}}, {'=', 0xffffffff, {50, -2}},
    {'H', 0xffffffff, {7, 11}}, {'H', 0xffffffff, {26, 11}}, {'H', 0xffffffff, {44, 11}}, {'|', 0xffffffff, {6, 29}},
    {'-', 0xffffffff, {25, 29}}, {'|', 0xffffffff, {43, 29}}, {'|', 0xffffffff, {6, 49}}, {'-', 0xffffffff, {25, 49}},
    {'|', 0xffffffff, {43, 49}},
};