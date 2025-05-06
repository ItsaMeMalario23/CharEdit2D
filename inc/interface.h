#ifndef INTERFACE_H
#define INTERFACE_H

#include <main.h>
#include <ascii.h>
#include <SDL3/SDL.h>

#define INPUT_MODE_STD      0
#define INPUT_MODE_CHAR     1
#define INPUT_MODE_COLOR    2
#define INPUT_MODE_ANCHOR   3

#define ANIM_BUF_SIZE       64

#define SHIFT_ACCEL         (20.0f)

#define INPUT_UP        0
#define INPUT_LEFT      1
#define INPUT_DOWN      2
#define INPUT_RIGHT     3
#define INPUT_PGUP      4
#define INPUT_PGDOWN    5
#define INPUT_W         6
#define INPUT_A         7
#define INPUT_S         8
#define INPUT_D         9
#define INPUT_C         10
#define INPUT_Z         11
#define INPUT_P         12
#define INPUT_X         13
#define INPUT_Y         14
#define INPUT_H         15
#define INPUT_R         16
#define INPUT_PLUS      17
#define INPUT_MINUS     18
#define INPUT_TAB       19
#define INPUT_O         20
#define INPUT_DOT       21
#define INPUT_COMMA     22

#define INPUT_MAP_SIZE  23

typedef struct frame_s frame_t;

struct frame_s {
    frame_t*    next;
    frame_t*    prev;
    vec2f_t*    positions;
    u64         bufsize;
};

extern u32 g_inputmode;
extern u32 g_inputidx;

extern f32 g_rscale;

extern char g_inputbuf[32];

extern char2Idx g_chars[ASCII_MAX_2D_CHARS];
extern char2Idx g_charidx;
extern char2Idx g_current;

extern bool g_iterate;
extern bool g_shift;
extern bool g_box;
extern bool g_alt;

extern vec2f_t g_anchor;
extern vec2f_t g_anchortmp;
extern vec2f_t g_roffset;

extern u32 g_frameidx;
extern u32 g_numframes;

void initInterface(void);
void loadObject(const ascii2info_t* object, u32 len);
void updateCharBuf(frame_t* restrict frame);
void updateFrameBuf(frame_t* restrict frame);

// Input handlers
SDL_AppResult stdInputMode(SDL_Keycode input);
SDL_AppResult txtInputMode(SDL_Keycode input);
SDL_AppResult anchorInputMode(SDL_Keycode input);

// Local funtions
char getInputChar(char input);
void processTxtInput(void);

#endif