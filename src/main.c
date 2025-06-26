#define SDL_MAIN_USE_CALLBACKS 1

#include <math.h>
#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_events.h>

#include <main.h>
#include <render.h>
#include <interface.h>
#include <ascii.h>
#include <objects.h>
#include <worldsim.h>
#include <debug/rdebug.h>
#include <debug/memtrack.h>

static SDL_Window* window = NULL;

SDL_Renderer* g_renderer = NULL;

static const ascii2info_t obj[195] = {
    {'#', 0x40d800ff, {7, 42}}, {'#', 0x40d800ff, {34, 42}}, {'#', 0x40d800ff, {61, 42}}, {'#', 0x40d800ff, {88, 42}},
    {'#', 0x40d800ff, {115, 42}}, {'*', 0x000000ff, {20, 50}}, {'*', 0x000000ff, {47, 50}}, {'*', 0x000000ff, {74, 50}},
    {'*', 0x000000ff, {101, 50}}, {'*', 0x000000ff, {34, 63}}, {'*', 0x000000ff, {61, 63}}, {'*', 0x000000ff, {88, 63}},
    {'*', 0x000000ff, {101, 76}}, {'*', 0x000000ff, {20, 76}}, {'*', 0x000000ff, {47, 76}}, {'*', 0x000000ff, {74, 76}},
    {'#', 0x40d800ff, {7, 84}}, {'#', 0x40d800ff, {7, 63}}, {'#', 0x40d800ff, {34, 84}}, {'#', 0x40d800ff, {61, 84}},
    {'#', 0x40d800ff, {88, 84}}, {'#', 0x40d800ff, {115, 84}}, {'#', 0x40d800ff, {115, 63}}, {'+', 0x40d800ff, {19, 63}},
    {'+', 0x40d800ff, {47, 63}}, {'+', 0x40d800ff, {73, 63}}, {'+', 0x40d800ff, {101, 63}}, {'^', 0x40d800ff, {34, 58}},
    {'"', 0x40d800ff, {20, 43}}, {'^', 0x40d800ff, {61, 58}}, {'^', 0x40d800ff, {88, 58}}, {'^', 0x40d800ff, {88, 77}},
    {'^', 0x40d800ff, {34, 77}}, {'^', 0x40d800ff, {61, 77}}, {'"', 0x40d800ff, {47, 43}}, {'"', 0x40d800ff, {74, 43}},
    {'"', 0x40d800ff, {101, 43}}, {'"', 0x40d800ff, {20, 91}}, {'"', 0x40d800ff, {47, 91}}, {'"', 0x40d800ff, {74, 91}},
    {'"', 0x40d800ff, {101, 91}}, {'-', 0x40d800ff, {114, 53}}, {'-', 0x40d800ff, {114, 73}}, {'-', 0x40d800ff, {5, 73}},
    {'-', 0x40d800ff, {5, 53}}, {'=', 0xff0000ff, {134, 39}}, {'=', 0xff0000ff, {150, 39}}, {'=', 0xff0000ff, {166, 39}},
    {'=', 0xff0000ff, {182, 39}}, {'=', 0xff0000ff, {198, 39}}, {'=', 0xff0000ff, {214, 39}}, {'=', 0xff0000ff, {230, 39}},
    {'=', 0xff0000ff, {246, 39}}, {'=', 0xff0000ff, {262, 39}}, {'=', 0xff0000ff, {278, 39}}, {'=', 0x000000ff, {134, 49}},
    {'=', 0x000000ff, {150, 49}}, {'=', 0xff0000ff, {134, 59}}, {'=', 0x000000ff, {134, 69}}, {'=', 0xff0000ff, {134, 79}},
    {'=', 0x000000ff, {134, 89}}, {'=', 0xff0000ff, {134, 99}}, {'=', 0x000000ff, {134, 109}}, {'=', 0xff0000ff, {134, 119}},
    {'=', 0x000000ff, {134, 129}}, {'=', 0xff0000ff, {134, 139}}, {'=', 0x000000ff, {166, 49}}, {'=', 0x000000ff, {182, 49}},
    {'=', 0x000000ff, {198, 49}}, {'=', 0x000000ff, {214, 49}}, {'=', 0x000000ff, {230, 49}}, {'=', 0x000000ff, {246, 49}},
    {'=', 0x000000ff, {262, 49}}, {'=', 0x000000ff, {278, 49}}, {'=', 0xff0000ff, {150, 59}}, {'=', 0xff0000ff, {166, 59}},
    {'=', 0xff0000ff, {182, 59}}, {'=', 0xff0000ff, {198, 59}}, {'=', 0xff0000ff, {214, 59}}, {'=', 0xff0000ff, {230, 59}},
    {'=', 0xff0000ff, {246, 59}}, {'=', 0xff0000ff, {262, 59}}, {'=', 0xff0000ff, {278, 59}}, {'=', 0x000000ff, {150, 69}},
    {'=', 0x000000ff, {166, 69}}, {'=', 0x000000ff, {182, 69}}, {'=', 0x000000ff, {198, 69}}, {'=', 0x000000ff, {214, 69}},
    {'=', 0x000000ff, {230, 69}}, {'=', 0x000000ff, {246, 69}}, {'=', 0x000000ff, {262, 69}}, {'=', 0x000000ff, {278, 69}},
    {'=', 0xff0000ff, {150, 79}}, {'=', 0xff0000ff, {166, 79}}, {'=', 0xff0000ff, {182, 79}}, {'=', 0xff0000ff, {198, 79}},
    {'=', 0xff0000ff, {214, 79}}, {'=', 0xff0000ff, {230, 79}}, {'=', 0xff0000ff, {246, 79}}, {'=', 0xff0000ff, {262, 79}},
    {'=', 0xff0000ff, {278, 79}}, {'=', 0x000000ff, {150, 89}}, {'=', 0x000000ff, {166, 89}}, {'=', 0x000000ff, {182, 89}},
    {'=', 0x000000ff, {198, 89}}, {'=', 0x000000ff, {214, 89}}, {'=', 0x000000ff, {230, 89}}, {'=', 0x000000ff, {246, 89}},
    {'=', 0x000000ff, {262, 89}}, {'=', 0x000000ff, {278, 89}}, {'=', 0xff0000ff, {150, 99}}, {'=', 0xff0000ff, {166, 99}},
    {'=', 0xff0000ff, {182, 99}}, {'=', 0xff0000ff, {198, 99}}, {'=', 0xff0000ff, {214, 99}}, {'=', 0xff0000ff, {230, 99}},
    {'=', 0xff0000ff, {246, 99}}, {'=', 0xff0000ff, {262, 99}}, {'=', 0xff0000ff, {278, 99}}, {'=', 0x000000ff, {150, 109}},
    {'=', 0x000000ff, {166, 109}}, {'=', 0x000000ff, {182, 109}}, {'=', 0x000000ff, {198, 109}}, {'=', 0x000000ff, {214, 109}},
    {'=', 0x000000ff, {230, 109}}, {'=', 0x000000ff, {246, 109}}, {'=', 0x000000ff, {262, 109}}, {'=', 0x000000ff, {278, 109}},
    {'=', 0xff0000ff, {150, 119}}, {'=', 0xff0000ff, {166, 119}}, {'=', 0xff0000ff, {182, 119}}, {'=', 0xff0000ff, {198, 119}},
    {'=', 0xff0000ff, {214, 119}}, {'=', 0xff0000ff, {230, 119}}, {'=', 0xff0000ff, {246, 119}}, {'=', 0xff0000ff, {262, 119}},
    {'=', 0xff0000ff, {278, 119}}, {'=', 0x000000ff, {150, 129}}, {'=', 0x000000ff, {166, 129}}, {'=', 0x000000ff, {182, 129}},
    {'=', 0x000000ff, {198, 129}}, {'=', 0x000000ff, {214, 129}}, {'=', 0x000000ff, {230, 129}}, {'=', 0x000000ff, {246, 129}},
    {'=', 0x000000ff, {262, 129}}, {'=', 0x000000ff, {278, 129}}, {'=', 0xff0000ff, {150, 139}}, {'=', 0xff0000ff, {166, 139}},
    {'=', 0xff0000ff, {182, 139}}, {'=', 0xff0000ff, {198, 139}}, {'=', 0xff0000ff, {214, 139}}, {'=', 0xff0000ff, {230, 139}},
    {'=', 0xff0000ff, {246, 139}}, {'=', 0xff0000ff, {262, 139}}, {'=', 0xff0000ff, {278, 139}}, {'=', 0xff0000ff, {118, 139}},
    {'=', 0xff0000ff, {102, 139}}, {'=', 0xff0000ff, {86, 139}}, {'=', 0xff0000ff, {70, 139}}, {'=', 0xff0000ff, {54, 139}},
    {'=', 0xff0000ff, {38, 139}}, {'=', 0xff0000ff, {22, 139}}, {'=', 0xff0000ff, {6, 139}}, {'=', 0x000000ff, {6, 129}},
    {'=', 0x000000ff, {22, 129}}, {'=', 0x000000ff, {38, 129}}, {'=', 0x000000ff, {54, 129}}, {'=', 0x000000ff, {70, 129}},
    {'=', 0x000000ff, {86, 129}}, {'=', 0x000000ff, {102, 129}}, {'=', 0x000000ff, {118, 129}}, {'=', 0xff0000ff, {6, 119}},
    {'=', 0xff0000ff, {22, 119}}, {'=', 0xff0000ff, {38, 119}}, {'=', 0xff0000ff, {54, 119}}, {'=', 0xff0000ff, {70, 119}},
    {'=', 0xff0000ff, {86, 119}}, {'=', 0xff0000ff, {102, 119}}, {'=', 0xff0000ff, {118, 119}}, {'=', 0x000000ff, {6, 109}},
    {'=', 0x000000ff, {22, 109}}, {'=', 0x000000ff, {38, 109}}, {'=', 0x000000ff, {54, 109}}, {'=', 0x000000ff, {70, 109}},
    {'=', 0x000000ff, {86, 109}}, {'=', 0x000000ff, {102, 109}}, {'=', 0x000000ff, {118, 109}}, {'=', 0xff0000ff, {6, 99}},
    {'=', 0xff0000ff, {22, 99}}, {'=', 0xff0000ff, {38, 99}}, {'=', 0xff0000ff, {54, 99}}, {'=', 0xff0000ff, {70, 99}},
    {'=', 0xff0000ff, {86, 99}}, {'=', 0xff0000ff, {102, 99}}, {'=', 0xff0000ff, {118, 99}}
};

const ascii2info_t brickdata[55] = {
    {'=', 0x66502aff, {0.0000000f, 0.1305556f}}, {'\'', 0x000000ff, {0.0187500f, 0.1194444f}}, {'=', 0x66502aff, {0.0359375f, 0.1305556f}}, {'=', 0x66502aff, {0.0609375f, 0.1305556f}},
    {'\'', 0x000000ff, {0.0796875f, 0.1194444f}}, {'=', 0x66502aff, {0.0968750f, 0.1305556f}}, {'\'', 0x000000ff, {-0.0062500f, 0.0861111f}}, {'=', 0x66502aff, {0.0109375f, 0.0972222f}},
    {'=', 0x66502aff, {0.0359375f, 0.0972222f}}, {'\'', 0x000000ff, {0.0546875f, 0.0861111f}}, {'=', 0x66502aff, {0.0718750f, 0.0972222f}}, {'=', 0x66502aff, {0.0968750f, 0.0972222f}},
    {'_', 0x000000ff, {0.0000000f, 0.1361111f}}, {'_', 0x000000ff, {0.0250000f, 0.1361111f}}, {'-', 0x000000ff, {0.0468750f, 0.1138889f}}, {'_', 0x000000ff, {0.0718750f, 0.1361111f}},
    {'_', 0x000000ff, {0.0968750f, 0.1361111f}}, {'_', 0x000000ff, {0.0000000f, 0.1027778f}}, {'_', 0x000000ff, {0.0250000f, 0.1027778f}}, {'-', 0x000000ff, {0.0468750f, 0.0805556f}},
    {'_', 0x000000ff, {0.0718750f, 0.1027778f}}, {'_', 0x000000ff, {0.0968750f, 0.1027778f}}, {'=', 0x66502aff, {0.0000000f, 0.0638889f}}, {'=', 0x66502aff, {0.0359375f, 0.0638889f}},
    {'=', 0x66502aff, {0.0609375f, 0.0638889f}}, {'=', 0x66502aff, {0.0968750f, 0.0638889f}}, {'\'', 0x000000ff, {0.0187500f, 0.0527778f}}, {'\'', 0x000000ff, {0.0796875f, 0.0527778f}},
    {'_', 0x000000ff, {0.0000000f, 0.0694444f}}, {'_', 0x000000ff, {0.0250000f, 0.0694444f}}, {'_', 0x000000ff, {0.0718750f, 0.0694444f}}, {'_', 0x000000ff, {0.0968750f, 0.0694444f}},
    {'-', 0x000000ff, {0.0468750f, 0.0472222f}}, {'=', 0x66502aff, {0.0109375f, 0.0305556f}}, {'=', 0x66502aff, {0.0359375f, 0.0305556f}}, {'=', 0x66502aff, {0.0718750f, 0.0305556f}},
    {'=', 0x66502aff, {0.0968750f, 0.0305556f}}, {'\'', 0x000000ff, {0.0546875f, 0.0194444f}}, {'\'', 0x000000ff, {-0.0062500f, 0.0194444f}}, {'_', 0x000000ff, {0.0000000f, 0.0361111f}},
    {'_', 0x000000ff, {0.0250000f, 0.0361111f}}, {'_', 0x000000ff, {0.0718750f, 0.0361111f}}, {'_', 0x000000ff, {0.0968750f, 0.0361111f}}, {'-', 0x000000ff, {0.0468750f, 0.0138889f}},
    {'=', 0x66502aff, {0.0000000f, -0.0055556f}}, {'=', 0x66502aff, {0.0359375f, -0.0055556f}}, {'=', 0x66502aff, {0.0609375f, -0.0055556f}}, {'=', 0x66502aff, {0.0968750f, -0.0055556f}},
    {'\'', 0x000000ff, {0.0187500f, -0.0166667f}}, {'\'', 0x000000ff, {0.0796875f, -0.0166667f}}, {'_', 0x000000ff, {0.0000000f, -0.0000000f}}, {'_', 0x000000ff, {0.0250000f, -0.0000000f}},
    {'_', 0x000000ff, {0.0718750f, -0.0000000f}}, {'_', 0x000000ff, {0.0968750f, -0.0000000f}}, {'-', 0x000000ff, {0.0468750f, -0.0222222f}},
};

const ascii2info_t mariodata[49] = {
    {'c', 0xff0000ff, {0.0062500f, 0.2305555f}}, {'o', 0xff0000ff, {0.0546875f, 0.2305555f}}, {'=', 0xff0000ff, {0.0312500f, 0.2194444f}}, {'-', 0xff0000ff, {0.0750000f, 0.2138889f}},
    {'-', 0xff0000ff, {0.0296875f, 0.2361111f}}, {'\'', 0xffffffff, {0.0593750f, 0.1805556f}}, {'T', 0xffff, {0.0156250f, 0.1861111f}}, {'[', 0xffff, {-0.0062500f, 0.1861111f}},
    {'e', 0xffff, {0.0078125f, 0.1500000f}}, {'~', 0xffff, {0.0718750f, 0.1555556f}}, {'-', 0xffff, {0.0562500f, 0.1472222f}}, {',', 0xfcd303ff, {0.0734375f, 0.1944444f}},
    {'.', 0xfcd303ff, {0.0718750f, 0.2138889f}}, {'K', 0xfcd303ff, {0.0359375f, 0.1472222f}}, {'-', 0xfcd303ff, {0.0562500f, 0.1361111f}}, {'$', 0xfcd303ff, {0.0390625f, 0.1888889f}},
    {'\'', 0xfcd303ff, {0.0078125f, 0.1722222f}}, {',', 0xfcd303ff, {0.0578125f, 0.1805556f}}, {'|', 0xff0000ff, {0.0140625f, 0.1027778f}}, {'|', 0xff0000ff, {0.0484375f, 0.1027778f}},
    {'A', 0xff0000ff, {0.0156250f, 0.0611111f}}, {'A', 0xff0000ff, {0.0500000f, 0.0611111f}}, {'|', 0xff0000ff, {0.0312500f, 0.0611111f}}, {'/', 0xff0000ff, {0.0203125f, 0.0166667f}},
    {'\\', 0xff0000ff, {0.0453125f, 0.0166667f}}, {'o', 0xffff, {-0.0046875f, -0.0055556f}}, {'o', 0xffff, {0.0718750f, -0.0055556f}}, {'\'', 0xffff, {0.0125000f, -0.0250000f}},
    {'\'', 0xffff, {0.0578125f, -0.0250000f}}, {'M', 0xffff, {0.0328125f, 0.1027778f}}, {'R', 0xfcd303ff, {0.0750000f, 0.0638889f}}, {'R', 0xfcd303ff, {-0.0093750f, 0.0638889f}},
    {'/', 0xffff, {-0.0046875f, 0.1083333f}}, {'\\', 0xffff, {0.0687500f, 0.1083333f}}, {'.', 0xfcd303ff, {0.0171875f, 0.0777778f}}, {'.', 0xfcd303ff, {0.0515625f, 0.0777778f}},
    {'^', 0xffff, {0.0015625f, 0.0805556f}}, {'^', 0xffff, {0.0625000f, 0.0805556f}}, {'.', 0xffff, {0.0593750f, 0.1222222f}}, {'.', 0xffff, {0.0078125f, 0.1222222f}},
    {'<', 0xff0000ff, {0.0046875f, 0.0166667f}}, {'>', 0xff0000ff, {0.0609375f, 0.0166667f}}, {'.', 0xff0000ff, {-0.0015625f, 0.0222222f}}, {'.', 0xff0000ff, {0.0703125f, 0.0222222f}},
    {',', 0xff0000ff, {0.0187500f, 0.0444444f}}, {'.', 0xff0000ff, {0.0500000f, 0.0444444f}}, {'.', 0xff0000ff, {0.0546875f, 0.0305556f}}, {'.', 0xff0000ff, {0.0671875f, 0.0444444f}},
    {'.', 0xff0000ff, {0.0015625f, 0.0444444f}},
};

const ascii2info_t qb[23] = {
    {'*', 0x30100ff, {0.0140625f, 0.1638889f}}, {'*', 0x30100ff, {0.0921875f, 0.1638889f}}, {'*', 0x30100ff, {0.0906250f, 0.0138889f}}, {'*', 0x30100ff, {0.0140625f, 0.0138889f}},
    {'_', 0x30100ff, {0.0046875f, 0.0111111f}}, {'_', 0x30100ff, {0.0281250f, 0.0111111f}}, {'_', 0x30100ff, {0.0531250f, 0.0111111f}}, {'_', 0x30100ff, {0.0781250f, 0.0111111f}},
    {'_', 0x30100ff, {0.1015625f, 0.0111111f}}, {'_', 0x994e00ff, {0.0046875f, 0.2111111f}}, {'_', 0x994e00ff, {0.0281250f, 0.2111111f}}, {'_', 0x994e00ff, {0.0531250f, 0.2111111f}},
    {'_', 0x994e00ff, {0.0781250f, 0.2111111f}}, {'_', 0x994e00ff, {0.1015625f, 0.2111111f}}, {'|', 0x994e00ff, {-0.0046875f, 0.0166667f}}, {'|', 0x994e00ff, {-0.0046875f, 0.0638889f}},
    {'|', 0x994e00ff, {-0.0046875f, 0.1138889f}}, {'|', 0x994e00ff, {-0.0046875f, 0.1611111f}}, {'|', 0x30100ff, {0.1062500f, 0.1611111f}}, {'|', 0x30100ff, {0.1062500f, 0.1138889f}},
    {'|', 0x30100ff, {0.1062500f, 0.0638889f}}, {'|', 0x30100ff, {0.1062500f, 0.0166667f}}, {'!', 0xea9e22ff, {0.0421875f, 0.1555556f}},
};

//
//  Main screen
//
static i32 midx = -1;

void mainScreen(void)
{
    if (gRenderscale > 1.001f || gRenderscale < 0.999f)
        SDL_SetRenderScale(g_renderer, gRenderscale, gRenderscale);

    // render chars
    asciiRenderCharBuf2D(COLOR_CLEAR, 1, gRenderoffset.x, gRenderoffset.y);

    // render grid
    if (mGrid) {
        for (i32 x = -380; x < (i32) (WINDOW_WIDTH * WINDOW_SCALE) + 400; x += GRID_SIZE)
            renderRectangleColor(x + gRenderoffset.x, -400.0f + gRenderoffset.y, 1.0f, (WINDOW_HEIGHT * WINDOW_SCALE) + 800.0f, COLOR_BLUE);

        for (i32 y = -420; y < (i32) (WINDOW_HEIGHT * WINDOW_SCALE) + 400; y += GRID_SIZE)
            renderRectangleColor(-400.0f + gRenderoffset.x, y + gRenderoffset.y, (WINDOW_WIDTH * WINDOW_SCALE) + 800.0f, 1.0f, COLOR_BLUE);
    }

    // render box
    if ((g_box || gInmode == INPUT_MODE_CHAR || gInmode == INPUT_MODE_COLOR) && gCurrentchar >= 0)
        renderHitbox(g_charbuf2D[gCharbuf[gCurrentchar]].xpos - 2.0f + gRenderoffset.x, g_charbuf2D[gCharbuf[gCurrentchar]].ypos - 2.0f + gRenderoffset.y, 18.0f, 18.0f);
    
    if (midx > -1)
        renderHitbox(g_charbuf2D[gCharbuf[midx]].xpos - 2.0f + gRenderoffset.x, g_charbuf2D[gCharbuf[midx]].ypos - 2.0f + gRenderoffset.y, 18.0f, 18.0f);

    // render anchor
    if (g_box || gInmode == INPUT_MODE_ANCHOR) {
        renderRectangleColor(gAnchorpos.x + gAnchortmp.x - 1.0f + gRenderoffset.x, gAnchorpos.y + gAnchortmp.y - 10.0f + gRenderoffset.y, 2.0f, 20.0f, COLOR_RED);
        renderRectangleColor(gAnchorpos.x + gAnchortmp.x - 10.0f + gRenderoffset.x, gAnchorpos.y + gAnchortmp.y - 1.0f + gRenderoffset.y, 20.0f, 2.0f, COLOR_RED);
    }

    SDL_SetRenderScale(g_renderer, 1.0f, 1.0f);

    // render char list
    for (u32 i = 0; i < gNumchars; i++) {
        if (i == gCurrentchar && !mMultiselect)
            renderRectangleColor((WINDOW_WIDTH * WINDOW_SCALE) - 185.0f, 25.0f + (i * 16.0f) + gRenderscroll, 154.0f, 16.0f, COLOR_M_GRAY);

        renderStrColorFmt
        (
            (WINDOW_WIDTH * WINDOW_SCALE) - 180, 30 + (i * 16) + gRenderscroll, 0.125f, COLOR_WHITE, "[%c] X: %4.0f Y: %3.0f",
            g_charbuf2D[gCharbuf[i]].charID, g_charbuf2D[gCharbuf[i]].xpos, g_charbuf2D[gCharbuf[i]].ypos
        );

        if (mMultiselect) {
            for (u32 k = 0; k < gNumselected; k++) {
                if (gMultiselbuf[k] == gCharbuf[i]) {
                    renderHitbox((WINDOW_WIDTH * WINDOW_SCALE) - 185.0f, 25.0f + (i * 16.0f) + gRenderscroll, 154.0f, 16.0f);
                    break;
                }
            }
        }
    }

    // render edit input
    if (gInmode == INPUT_MODE_CHAR || gInmode == INPUT_MODE_COLOR)
    {
        const char* str;
        char c = 32;

        if (gInmode == INPUT_MODE_CHAR)
            str = "CHR";
        else if (gInmode == INPUT_MODE_COLOR)
            str = "COL";
        else
            str = "<ERR>";

        if (gInidx)
            c = gInputbuf[0];

        renderRectangleColor(25.0f, (WINDOW_HEIGHT * WINDOW_SCALE) - 45.0f, 96.0f, 16.0f, COLOR_M_GRAY);
        renderStrColorFmt(30, (i32) (WINDOW_HEIGHT * WINDOW_SCALE) - 41, 0.125f, COLOR_WHITE, "EDIT %s: %c", str, c);
    }
    else if (gInmode == INPUT_MODE_SCOLOR)
    {
        renderRectangleColor(25.0f, (WINDOW_HEIGHT * WINDOW_SCALE) - 45.0f, 152.0f, 16.0f, COLOR_M_GRAY);
        renderStrColorFmt(30, (i32) (WINDOW_HEIGHT * WINDOW_SCALE) - 41, 0.125f, COLOR_WHITE, "EDIT COL%d: #%.*s", gEditcol, gInidx, gInputbuf);
    }

    // render animation frameinfo
    if (gNumframes > 1) {
        renderRectangleColor((WINDOW_WIDTH * WINDOW_SCALE) - 300.0f, (WINDOW_HEIGHT * WINDOW_SCALE) - 40.0f, 90.0f, 16.0f, COLOR_M_GRAY);
        renderStrColorFmt((WINDOW_WIDTH * WINDOW_SCALE) - 295, (WINDOW_HEIGHT * WINDOW_SCALE) - 35, 0.125f, COLOR_WHITE, "FRAME %d/%d", gFrameidx, gNumframes);
    }

    // render shortcut colors
    renderRectangleColor(25.0f, (WINDOW_HEIGHT * WINDOW_SCALE) - 245.0f, 12.0f, 32.0f, gColors[0]);
    renderRectangleColor(25.0f, (WINDOW_HEIGHT * WINDOW_SCALE) - 195.0f, 12.0f, 32.0f, gColors[1]);
    renderRectangleColor(25.0f, (WINDOW_HEIGHT * WINDOW_SCALE) - 145.0f, 12.0f, 32.0f, gColors[2]);
    renderRectangleColor(25.0f, (WINDOW_HEIGHT * WINDOW_SCALE) - 95.0f, 12.0f, 32.0f, gColors[3]);

    if (gEditcol >= 0)
        renderHitbox(23.0f, (WINDOW_HEIGHT * WINDOW_SCALE) - 247.0f + (50.0f * gEditcol), 36.0f, 36.0f);

    SDL_RenderPresent(g_renderer);
}

//
//  SDL Init
//
SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialized SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("ASCII-EDIT 2D", WINDOW_WIDTH * WINDOW_SCALE, WINDOW_HEIGHT * WINDOW_SCALE, SDL_WINDOW_BORDERLESS, &window, &g_renderer)) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    initInterface();

    initRenderer();

    initAscii(ASCII_RENDER_MODE_2D);

    if (!loadCharTextures("..\\resources\\ascii\\pressstart\\", 95))
        return SDL_APP_FAILURE;

    loadObject(qb, 23);
    
    SDL_SetRenderScale(g_renderer, WINDOW_SCALE, WINDOW_SCALE);

    return SDL_APP_CONTINUE;
}

//
//  SDL Event
//
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT)
    {
        return SDL_APP_SUCCESS;
    }
    else if (event->type == SDL_EVENT_KEY_UP)
    {
        if (event->key.key == SDLK_LSHIFT || event->key.key == SDLK_RSHIFT)
            g_shift = 0;
        else if (event->key.key == SDLK_LCTRL || event->key.key == SDLK_RCTRL)
            g_box = 0;
        else if (event->key.key == SDLK_LALT || event->key.key == SDLK_RALT)
            g_alt = 0;

        return SDL_APP_CONTINUE;
    }
    else if (event->type == SDL_EVENT_KEY_DOWN)
    {
        if (event->key.key == SDLK_LSHIFT || event->key.key == SDLK_RSHIFT) {
            g_shift = 1;
            return SDL_APP_CONTINUE;
        } else if (event->key.key == SDLK_LCTRL || event->key.key == SDLK_RCTRL) {
            g_box = 1;
            return SDL_APP_CONTINUE;
        } else if (event->key.key == SDLK_LALT || event->key.key == SDLK_RALT) {
            g_alt = 1;
            return SDL_APP_CONTINUE;
        }

        g_iterate = 1;

        switch (gInmode) {
        case INPUT_MODE_STD:
            return stdInputMode(event->key.key);
        
        case INPUT_MODE_CHAR:
        case INPUT_MODE_COLOR:
        case INPUT_MODE_SCOLOR:
            return txtInputMode(event->key.key);

        case INPUT_MODE_ANCHOR:
            return anchorInputMode(event->key.key);

        default:
            rAssert(0);
            break;
        }
    }
    else if (event->type == SDL_EVENT_MOUSE_WHEEL)
    {
        gRenderscroll += event->wheel.y * 16;

        if (gRenderscroll > 0.0f)
            gRenderscroll = 0.0f;

        if ((i32) gRenderscroll < -((gNumchars * 16) - (WINDOW_HEIGHT * WINDOW_SCALE * 0.7f)))
            gRenderscroll = (f32) -((gNumchars * 16) - (WINDOW_HEIGHT * WINDOW_SCALE * 0.7f));
    }
    else if (event->type == SDL_EVENT_MOUSE_MOTION)
    {
        midx = -1;

        for (u32 i = 0; i < gNumchars; i++) {
            if (g_charbuf2D[gCharbuf[i]].xpos > (event->motion.x / gRenderscale) - gRenderoffset.x || (g_charbuf2D[gCharbuf[i]].xpos) + (64.0f * ASCII_RENDER_SCALE) < (event->motion.x / gRenderscale) - gRenderoffset.x)
                continue;

            if (g_charbuf2D[gCharbuf[i]].ypos > (event->motion.y / gRenderscale) - gRenderoffset.y || (g_charbuf2D[gCharbuf[i]].ypos) + (64.0f * ASCII_RENDER_SCALE) < (event->motion.y / gRenderscale) - gRenderoffset.y)
                continue;

            midx = i;
        }
    }
    else if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        if (event->button.button == 1 && midx > -1) {
            if (mMultiselect)
                addCharToSelection(midx);
            else
                gCurrentchar = midx;
        }
    }

    return SDL_APP_CONTINUE;
}

//
//  SDL Update
//
SDL_AppResult SDL_AppIterate(void* appstate)
{
    SDL_Event next;

    mainScreen();

    SDL_WaitEvent(&next);

    return SDL_AppEvent(NULL, &next);
}

//
//  SDL Exit
//
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    cleanupRenderer();
    cleanupAscii();
}

/*
void getCircleCoords(f32 radius, f32 x1, f32 y1, u32 divisions)
{
    f32 r2 = radius * radius;

    for (u32 i = 0; i <= divisions; i++) {
        f32 seg = sinf(((f32) i / (f32) divisions) * (M_PI / 2)) * (radius * 2);

        //printf("seg %.2f: %.2f\n", ((f32) i / divisions) * (M_PI / 2), seg);

        f32 f = sqrtf(r2 - (x1 * x1) + (2 * x1 * (x1 - radius + seg)) - (((x1 - radius + seg) * (x1 - radius + seg))));

        if (!i || i == divisions) {
            printf("X: {%.1f, %.1f}\n", x1 - radius + seg, y1 + f);
        } else {
            printf("X: {%.1f, %.1f} {%.1f, %.1f}\n", x1 - radius + seg, y1 - f, x1 - radius + seg, y1 + f);
        }
    }
}

void getCircleCoords2(f32 radius, f32 x1, f32 y1, u32 divisions)
{
    f32 r2 = radius * radius;

    for (u32 i = 0; i <= divisions; i++) {
        f32 seg = (1.0f - cosf(((f32) i / (f32) divisions) * (M_PI / 2))) * radius;

        f32 f = sqrtf(r2 - (x1 * x1) + (2 * x1 * (x1 - radius + seg)) - (((x1 - radius + seg) * (x1 - radius + seg))));

        if (i == divisions) {
            if (!i) {
                printf("{\'G\', WHITE, {%.1ff, %.1ff}},\n", x1 - radius + seg, y1 + f);
            } else {
                printf("{\'G\', WHITE, {%.1ff, %.1ff}}, {\'G\', WHITE, {%.1ff, %.1ff}},\n", x1 - radius + seg, y1 - f, x1 - radius + seg, y1 + f);
            }
        } else {
            if (!i) {
                printf("{\'G\', WHITE, {%.1ff, %.1ff}},\n", x1 - radius + seg, y1 + f);
                printf("{\'G\', WHITE, {%.1ff, %.1ff}},\n", x1 + radius - seg, y1 + f);
            } else {
                printf("{\'G\', WHITE, {%.1ff, %.1ff}}, {\'G\', WHITE, {%.1ff, %.1ff}},\n", x1 - radius + seg, y1 - f, x1 - radius + seg, y1 + f);
                printf("{\'G\', WHITE, {%.1ff, %.1ff}}, {\'G\', WHITE, {%.1ff, %.1ff}},\n", x1 + radius - seg, y1 - f, x1 + radius - seg, y1 + f);
            }
        }
    }
}
*/
