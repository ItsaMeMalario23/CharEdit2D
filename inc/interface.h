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
#define INPUT_H         14
#define INPUT_R         15
#define INPUT_PLUS      16
#define INPUT_MINUS     17
#define INPUT_TAB       18
#define INPUT_O         19
#define INPUT_DOT       20
#define INPUT_COMMA     21

typedef struct frame_s frame_t;

struct frame_s {
    frame_t*    next;
    frame_t*    prev;
    vec2f_t*    positions;
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

extern asciianim_t g_animation;
extern u32 g_animframe;

void initInterface(void);
void loadObject(const ascii2info_t* object, u32 len);
void updateCharBuf(void);

// Std input callbacks
void addCharCallback(void);
void mvUpCallback(void);
void mvLeftCallback(void);
void mvDownCallback(void);
void mvRightCallback(void);
void chColorCallback(void);
void chCharCallback(void);
void charUpCallback(void);
void charDownCallback(void);
void charUpWrapCallback(void);
void createStructCallback(void);
void toggleBoxCallback(void);
void editAnchorCallback(void);
void zoomInCallback(void);
void zoomOutCallback(void);
void scrUpCallback(void);
void scrLeftCallback(void);
void scrDownCallback(void);
void scrRightCallback(void);
void addFrameCallback(void);
void frameStepForward(void);
void frameStepBackward(void);

// Input handlers
SDL_AppResult stdInputMode(SDL_Keycode input);
SDL_AppResult txtInputMode(SDL_Keycode input);
SDL_AppResult anchorInputMode(SDL_Keycode input);

// Local funtions
char getInputChar(char input);
void processTxtInput(void);

#endif