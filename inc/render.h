#ifndef RENDER_H
#define RENDER_H

#include <main.h>

#define COLOR_RED       0xff0000ff
#define COLOR_GREEN     0x00ff00ff
#define COLOR_BLUE      0x0000ffff
#define COLOR_YELLOW    0xfce803ff
#define COLOR_GOLD      0xfcd303ff
#define COLOR_PURPLE    0xa103fcff
#define COLOR_AZURE     0x03ecfcff
#define COLOR_OLIVE     0x017505ff
#define COLOR_BLACK     0x000000ff
#define COLOR_WHITE     0xffffffff
#define COLOR_WWHITE    0xe0e0ceff
#define COLOR_L_YELLOW  0xffee9cff
#define COLOR_L_GRAY    0xa3a4a4ff
#define COLOR_M_GRAY    0x464646ff
#define COLOR_D_GRAY    0x262626ff
#define COLOR_D_GREEN   0x003000ff
#define COLOR_M_GREEN   0x0d7000ff
#define COLOR_L_GREEN   0x40d800ff

#define COLOR_HITBOXES  COLOR_RED

#define COLOR_CLEAR     COLOR_D_GRAY

#define MAX_TEXTURES        4
#define MAX_ASCII_TEXTURES  128

#define RENDER_TXT_MAX_LEN  256

#define TEXTURE_LOGO    0
#define TEXTURE_BIRD    1
#define TEXTURE_PIPE    2
#define TEXTURE_CLOUD   3

void initRenderer(void);
void cleanupRenderer(void);

void clearScreen(u32 color);
void setColor(u32 color);

void renderRectangleColor(f32 xpos, f32 ypos, f32 width, f32 height, u32 color);
void renderRectangle(f32 xpos, f32 ypos, f32 width, f32 height);
void renderHitbox(f32 xpos, f32 ypos, f32 width, f32 height);

bool loadTextures(const texinfo_t* textures, u32 numTextures);
bool loadTexture(const char* path, bool noInterpolation, u8 textureID);
bool loadCharTextures(const char* path, u32 numChars);
void setTextureColor(u32 textureID, u32 color);

void renderTexture(i16 xpos, i16 ypos, u16 scale, u16 textureID);
void renderTextureFlip(i16 xpos, i16 ypos, u8 scale, bool vFlip, bool hFlip, u8 textureID);
void renderTextureRotate(i16 xpos, i16 ypos, u16 rotation, u8 scale, u8 textureID);

void renderChar(i16 xpos, i16 ypos, f32 scale, char charID);
void renderCharColor(i16 xpos, i16 ypos, f32 scale, u32 color, char charID);
void renderStr(i16 xpos, i16 ypos, f32 scale, const char* str);
void renderStrColor(i32 xpos, i32 ypos, f32 scale, u32 color, const char* str);
void renderStrColorFmt(i32 xpos, i32 ypos, f32 scale, u32 color, const char* fmt, ... );
void renderStrColorCentered(i64 ypos, f32 scale, u32 color, const char* str);
void renderStrColorFmtCentered(i64 ypos, f32 scale, u32 color, const char* fmt, ... );

#endif