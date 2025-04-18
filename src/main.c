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

//
//  Main screen
//
void mainScreen(void)
{
    if (g_rscale > 1.001f || g_rscale < 0.999f)
        SDL_SetRenderScale(g_renderer, g_rscale, g_rscale);

    // render chars
    asciiRenderCharBuf2D(COLOR_BLACK, 1, g_roffset.x, g_roffset.y);

    // render box
    if ((g_box || g_inputmode == INPUT_MODE_CHAR || g_inputmode == INPUT_MODE_COLOR) && g_current >= 0)
        renderHitbox(g_charbuf2D[g_chars[g_current]].xpos - 2.0f + g_roffset.x, g_charbuf2D[g_chars[g_current]].ypos -2.0f + g_roffset.y, 18.0f, 18.0f);

    // render anchor
    if (g_box || g_inputmode == INPUT_MODE_ANCHOR) {
        renderRectangleColor(g_anchor.x + g_anchortmp.x - 1.0f + g_roffset.x, g_anchor.y + g_anchortmp.y - 10.0f + g_roffset.y, 2.0f, 20.0f, COLOR_RED);
        renderRectangleColor(g_anchor.x + g_anchortmp.x - 10.0f + g_roffset.x, g_anchor.y + g_anchortmp.y - 1.0f + g_roffset.y, 20.0f, 2.0f, COLOR_RED);
    }

    SDL_SetRenderScale(g_renderer, 1.0f, 1.0f);

    // render char list
    for (u32 i = 0; i < g_charidx; i++) {
        if (i == g_current)
            renderRectangleColor(1095.0f, 25.0f + (i * 16.0f), 154.0f, 16.0f, COLOR_D_GRAY);

        renderStrColorFmt
        (
            1100, 30 + (i * 16), 0.125f, COLOR_WHITE, "[%c] X: %4.0f Y: %3.0f",
            g_charbuf2D[g_chars[i]].charID, g_charbuf2D[g_chars[i]].xpos, g_charbuf2D[g_chars[i]].ypos
        );
    }

    // render edit input
    if (g_inputmode == INPUT_MODE_CHAR || g_inputmode == INPUT_MODE_COLOR) {
        const char* str;
        char c = 32;

        if (g_inputmode == INPUT_MODE_CHAR)
            str = "CHR";
        else
            str = "COL";

        if (g_inputidx)
            c = g_inputbuf[0];

        renderRectangleColor(25.0f, 675.0f, 96.0f, 16.0f, COLOR_D_GRAY);
        renderStrColorFmt(30, 680, 0.125f, COLOR_WHITE, "EDIT %s: %c", str, c);
    }

    SDL_RenderPresent(g_renderer);
}

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

    puts("c16 r100");
    getCircleCoords2(100, 200, 200, 4);
    puts("c12 r92");
    getCircleCoords2(92, 200, 200, 3);
    puts("c12 r71");
    getCircleCoords2(71, 200, 200, 3);
    puts("c8 r38");
    getCircleCoords2(38, 200, 200, 2);

    initInterface();

    initRenderer();

    initAscii(ASCII_RENDER_MODE_2D);

    if (!loadCharTextures("..\\resources\\ascii\\pressstart\\", 95))
        return SDL_APP_FAILURE;

    loadObject(c38, 8);
    
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
    }
    else if (event->type == SDL_EVENT_KEY_DOWN)
    {
        if (event->key.key == SDLK_LSHIFT || event->key.key == SDLK_RSHIFT) {
            g_shift = 1;
            return SDL_APP_CONTINUE;
        } else if (event->key.key == SDLK_LCTRL || event->key.key == SDLK_RCTRL) {
            g_box = 1;
            return SDL_APP_CONTINUE;
        }

        g_iterate = 1;

        switch (g_inputmode) {
        case INPUT_MODE_STD:
            return stdInputMode(event->key.key);
        
        case INPUT_MODE_CHAR:
        case INPUT_MODE_COLOR:
            return txtInputMode(event->key.key);

        case INPUT_MODE_ANCHOR:
            return anchorInputMode(event->key.key);

        default:
            rAssert(0);
            break;
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