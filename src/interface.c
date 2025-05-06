#include <math.h>
#include <stdio.h>
#include <SDL3/SDL.h>

#include <interface.h>
#include <render.h>
#include <ascii.h>
#include <debug/rdebug.h>
#include <debug/memtrack.h>

bool g_iterate = 0;
bool g_shift = 0;
bool g_box = 0;
bool g_alt = 0;

f32 g_rscale = 1.0f;
vec2f_t g_roffset = {0.0f, 0.0f};

u32 g_inputmode = INPUT_MODE_STD;

char g_inputbuf[32];
u32 g_inputidx = 0;

char2Idx g_chars[ASCII_MAX_2D_CHARS];
char2Idx g_charidx = 0;
char2Idx g_current = -1;

vec2f_t g_anchor = {0.0f, 0.0f};
vec2f_t g_anchortmp = {0.0f, 0.0f};

frame_t g_baseframe;
frame_t* g_currentframe = &g_baseframe;
u32 g_frameidx = 1;
u32 g_numframes = 1;

input_f inputCallbacks[24];

void initInterface(void)
{
    for (u32 i = 0; i < ASCII_MAX_2D_CHARS; i++)
        g_chars[i] = -1;

    memset(g_inputbuf, 0, sizeof(g_inputbuf));
    memset(inputCallbacks, 0, sizeof(inputCallbacks));

    // keyboard mapping
    inputCallbacks[INPUT_W] = mvUpCallback;
    inputCallbacks[INPUT_A] = mvLeftCallback;
    inputCallbacks[INPUT_S] = mvDownCallback;
    inputCallbacks[INPUT_D] = mvRightCallback;
    inputCallbacks[INPUT_P] = addCharCallback;
    inputCallbacks[INPUT_C] = chColorCallback;
    inputCallbacks[INPUT_Z] = chCharCallback;
    inputCallbacks[INPUT_PGUP] = charUpCallback;
    inputCallbacks[INPUT_PGDOWN] = charDownCallback;
    inputCallbacks[INPUT_X] = createStructCallback;
    inputCallbacks[INPUT_H] = toggleBoxCallback;
    inputCallbacks[INPUT_R] = editAnchorCallback;
    inputCallbacks[INPUT_PLUS] = zoomInCallback;
    inputCallbacks[INPUT_MINUS] = zoomOutCallback;
    inputCallbacks[INPUT_UP] = scrUpCallback;
    inputCallbacks[INPUT_LEFT] = scrLeftCallback;
    inputCallbacks[INPUT_DOWN] = scrDownCallback;
    inputCallbacks[INPUT_RIGHT] = scrRightCallback;
    inputCallbacks[INPUT_TAB] = charUpWrapCallback;
    inputCallbacks[INPUT_O] = addFrameCallback;
    inputCallbacks[INPUT_DOT] = frameStepForward;
    inputCallbacks[INPUT_COMMA] = frameStepBackward;

    g_baseframe.positions = (vec2f_t*) memAllocInit(sizeof(vec2f_t), ANIM_BUF_SIZE);
}

void loadObject(const ascii2info_t* object, u32 len)
{
    rAssert(object);
    rAssert(len);
    
    if (g_charidx + len > ASCII_MAX_2D_CHARS) {
        SDL_Log("Failed to load object, insufficient buf size");
        return;
    }

    charIdx idx;

    for (u32 i = 0; i < len; i++) {
        idx = asciiChar2D(object[i].pos.x, object[i].pos.y, object[i].color, object[i].charID);

        if (idx < 0) {
            SDL_Log("ERROR: create ascii char returned invalid index while loading object, aborting");
            return;
        }

        g_chars[g_charidx++] = idx;
    }

    g_current = g_chars[g_charidx - 1];
}

void updateCharBuf(void)
{
    rAssert(g_currentframe);

    for (u32 i = 0; i < g_charidx; i++) {
        g_charbuf2D[g_chars[i]].xpos = g_currentframe->positions[i].x;
        g_charbuf2D[g_chars[i]].ypos = g_currentframe->positions[i].y;
    }
}

//
//  Std input callbacks
//
void addCharCallback(void)
{
    if (g_charidx >= ASCII_MAX_2D_CHARS || g_charidx >= ANIM_BUF_SIZE) {
        SDL_Log("Failed to add char, buffer full");
        return;
    }
    
    f32 x = g_current < 0 ? 10.0f : g_charbuf2D[g_chars[g_current]].xpos + 16.0f;
    f32 y = g_current < 0 ? 10.0f : g_charbuf2D[g_chars[g_current]].ypos;

    char2Idx i = asciiChar2D(x, y, COLOR_WHITE, 'A');

    if (i < 0) {
        SDL_Log("ERROR: create ascii char returned invalid index");
        return;
    }

    g_current = g_charidx;
    g_chars[g_charidx++] = i;
}

void mvUpCallback(void)
{
    if (g_charbuf2D && g_current >= 0)
        g_charbuf2D[g_chars[g_current]].ypos -= g_shift ? SHIFT_ACCEL : 1.0f;
}

void mvLeftCallback(void)
{
    if (g_charbuf2D && g_current >= 0)
        g_charbuf2D[g_chars[g_current]].xpos -= g_shift ? SHIFT_ACCEL : 1.0f;
}

void mvDownCallback(void)
{
    if (g_charbuf2D && g_current >= 0)
        g_charbuf2D[g_chars[g_current]].ypos += g_shift ? SHIFT_ACCEL : 1.0f;
}

void mvRightCallback(void)
{
    if (g_charbuf2D && g_current >= 0)
        g_charbuf2D[g_chars[g_current]].xpos += g_shift ? SHIFT_ACCEL : 1.0f;
}

void chColorCallback(void)
{
    g_inputmode = INPUT_MODE_COLOR;
}

void chCharCallback(void)
{
    g_inputmode = INPUT_MODE_CHAR;
}

void charUpCallback(void)
{
    if (g_current > 0)
        g_current--;
}

void charDownCallback(void)
{
    if (g_current >= 0 && g_current + 1 < g_charidx)
        g_current++;
}

void charUpWrapCallback(void)
{
    if (g_current >= 0 && ++g_current >= g_charidx)
        g_current = 0;
}

void createStructCallback(void)
{
    if (!g_charidx)
        return;

    char buf[256];
    ascii2_t* asc;
    u32 idx = 0;
    u32 chr = 0;

    snprintf(buf, 256, "\nconst ascii2info_t obj[%d] = {", g_charidx);
    SDL_Log(buf);

    buf[0] = '\t';
    idx = 1;

    for (;;) {
        asc = g_charbuf2D + g_chars[chr];

        idx += snprintf(buf + idx, 256 - idx, "{\'%c\', 0x%x, {%.0f, %.0f}}, ", asc->charID, asc->color, asc->xpos - g_anchor.x, asc->ypos - g_anchor.y);

        if (++chr >= g_charidx) {
            SDL_Log(buf);
            break;
        }

        if (chr % 4 == 0) {
            SDL_Log(buf);
            buf[0] = '\t';
            idx = 1;
        }
    }

    SDL_Log("};\n");
}

void toggleBoxCallback(void)
{
    g_box = !g_box;
}

void editAnchorCallback(void)
{
    g_inputmode = INPUT_MODE_ANCHOR;
}

void zoomInCallback(void)
{
    if (g_rscale < 2.8f)
        g_rscale += 0.25f;
}

void zoomOutCallback(void)
{
    if (g_rscale > 0.4f)
        g_rscale -= 0.25f;
}

void scrUpCallback(void)
{
    g_roffset.y += g_shift ? roundf(SHIFT_ACCEL * g_rscale) : 1.0f;
}

void scrLeftCallback(void)
{
    g_roffset.x += g_shift ? roundf(SHIFT_ACCEL * g_rscale) : 1.0f;
}

void scrDownCallback(void)
{
    g_roffset.y -= g_shift ? roundf(SHIFT_ACCEL * g_rscale) : 1.0f;
}

void scrRightCallback(void)
{
    g_roffset.x -= g_shift ? roundf(SHIFT_ACCEL * g_rscale) : 1.0f;
}

void addFrameCallback(void)
{    
    rAssert(g_currentframe);
    rWarning(g_charidx < ANIM_BUF_SIZE);

    frame_t* tmp = (frame_t*) memAlloc(sizeof(frame_t));

    tmp->positions = (vec2f_t*) memAllocInit(sizeof(vec2f_t), ANIM_BUF_SIZE);
    memcpy(tmp->positions, g_currentframe->positions, g_charidx * sizeof(vec2f_t));

    tmp->prev = g_currentframe;
    tmp->next = g_currentframe->next;

    g_currentframe = tmp;
    g_numframes++;
    g_frameidx++;
}

void frameStepForward(void)
{
    if (g_currentframe && g_currentframe->next) {
        g_currentframe = g_currentframe->next;
        g_frameidx++;
        updateCharBuf();
    }
}

void frameStepBackward(void)
{
    if (g_currentframe && g_currentframe->prev) {
        g_currentframe = g_currentframe->prev;
        g_frameidx--;
        updateCharBuf();
    }
}

//
//  Input handler functions
//
SDL_AppResult stdInputMode(SDL_Keycode input)
{
    switch (input) {
    case SDLK_Q:
        createStructCallback();
        return SDL_APP_SUCCESS;

    case SDLK_UP:
        if (inputCallbacks[INPUT_UP])
            inputCallbacks[INPUT_UP]();
        break;

    case SDLK_DOWN:
        if (inputCallbacks[INPUT_DOWN])
            inputCallbacks[INPUT_DOWN]();
        break;

    case SDLK_LEFT:
        if (inputCallbacks[INPUT_LEFT])
            inputCallbacks[INPUT_LEFT]();
        break;

    case SDLK_RIGHT:
        if (inputCallbacks[INPUT_RIGHT])
            inputCallbacks[INPUT_RIGHT]();
        break;

    case SDLK_W:
        if (inputCallbacks[INPUT_W])
            inputCallbacks[INPUT_W]();
        break;

    case SDLK_A:
        if (inputCallbacks[INPUT_A])
            inputCallbacks[INPUT_A]();
        break;

    case SDLK_S:
        if (inputCallbacks[INPUT_S])
            inputCallbacks[INPUT_S]();
        break;

    case SDLK_D:
        if (inputCallbacks[INPUT_D])
            inputCallbacks[INPUT_D]();
        break;

    case SDLK_C:
        if (inputCallbacks[INPUT_C])
            inputCallbacks[INPUT_C]();
        break;

    case SDLK_Z:
        if (inputCallbacks[INPUT_Z])
            inputCallbacks[INPUT_Z]();
        break;

    case SDLK_P:
        if (inputCallbacks[INPUT_P])
            inputCallbacks[INPUT_P]();
        break;

    case SDLK_PAGEUP:
        if (inputCallbacks[INPUT_PGUP])
            inputCallbacks[INPUT_PGUP]();
        break;

    case SDLK_PAGEDOWN:
        if (inputCallbacks[INPUT_PGDOWN])
            inputCallbacks[INPUT_PGDOWN]();
        break;

    case SDLK_X:
        if (inputCallbacks[INPUT_X])
            inputCallbacks[INPUT_X]();
        break;

    case SDLK_H:
        if (inputCallbacks[INPUT_H])
            inputCallbacks[INPUT_H]();
        break;

    case SDLK_R:
        if (inputCallbacks[INPUT_R])
            inputCallbacks[INPUT_R]();
        break;

    case SDLK_PLUS:
        if (inputCallbacks[INPUT_PLUS])
            inputCallbacks[INPUT_PLUS]();
        break;

    case SDLK_MINUS:
        if (inputCallbacks[INPUT_MINUS])
            inputCallbacks[INPUT_MINUS]();
        break;

    case SDLK_TAB:
        if (inputCallbacks[INPUT_TAB])
            inputCallbacks[INPUT_TAB]();
        break;

    case SDLK_O:
        if (inputCallbacks[INPUT_O])
            inputCallbacks[INPUT_O]();
        break;

    case SDLK_COLON:
        if (inputCallbacks[INPUT_DOT])
            inputCallbacks[INPUT_DOT]();
        break;

    case SDLK_COMMA:
        if (inputCallbacks[INPUT_COMMA])
            inputCallbacks[INPUT_COMMA]();
        break;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult txtInputMode(SDL_Keycode input)
{
    if (input == SDLK_RETURN) {
        processTxtInput();

        g_inputmode = INPUT_MODE_STD;
        g_inputidx = 0;

        return SDL_APP_CONTINUE;
    }

    if (input == SDLK_ESCAPE) {
        g_inputmode = INPUT_MODE_STD;
        g_inputidx = 0;

        return SDL_APP_CONTINUE;
    }

    if (input < 32 && input > 127)
        return SDL_APP_CONTINUE;

    char c = getInputChar(input);

    if (!c)
        return SDL_APP_CONTINUE;

    if (g_inputmode == INPUT_MODE_CHAR || g_inputmode == INPUT_MODE_COLOR) {
        // single char input mode
        g_inputbuf[0] = c;
        g_inputidx = 1;
    } else if (g_inputidx < 32) {
        // multi char input
        g_inputbuf[g_inputidx++] = c;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult anchorInputMode(SDL_Keycode input)
{
    switch (input) {
    case SDLK_RETURN:
        g_anchor.x += g_anchortmp.x;
        g_anchor.y += g_anchortmp.y;
        // intentional falltrough
    case SDLK_ESCAPE:
    case SDLK_R:
        g_inputmode = INPUT_MODE_STD;
        g_anchortmp.x = 0.0f;
        g_anchortmp.y = 0.0f;
        break;

    case SDLK_W:
        g_anchortmp.y -= g_shift ? SHIFT_ACCEL : 1.0f;
        break;

    case SDLK_A:
        g_anchortmp.x -= g_shift ? SHIFT_ACCEL : 1.0f;
        break;

    case SDLK_S:
        g_anchortmp.y += g_shift ? SHIFT_ACCEL : 1.0f;
        break;

    case SDLK_D:
        g_anchortmp.x += g_shift ? SHIFT_ACCEL : 1.0f;
        break;
    }

    return SDL_APP_CONTINUE;
}

//
//  Local functions
//
const char shiftNums[10] = {'=', '!', '\"', '\\', '$', '%', '&', '/', '(', ')'};
const char altNums[10] = {0, 0, 0, 0, 0, 0, '{', '[', ']', '}'};

char getInputChar(char input)
{
    if (g_shift)
    {
        if (input > 96 && input < 123)
            return input - 32;  // to upper

        if (input > 47 && input < 58)
            return shiftNums[input - 48];

        switch (input) {
        case ',':
            return ';';
        case '.':
            return ':';
        case '#':
            return '\'';
        case '+':
            return '*';
        case '-':
            return '_';
        case '<':
            return '>';
        }

        return 0;
    }
    else if (g_alt)
    {
        if (input > 47 && input < 58)
            return altNums[input - 48];

        switch (input) {
        case 'q':
            return '@';
        case '<':
            return '|';
        case '+':
            return '~';
        }

        return 0;
    }
    else
    {
        return input;
    }
}

void processTxtInput(void)
{
    rAssert(g_inputmode == INPUT_MODE_CHAR || g_inputmode == INPUT_MODE_COLOR);
    rAssert(g_inputidx <= 32);
    
    if (!g_inputidx || g_inputbuf[0] < 32 || g_current < 0)
        return;

    if (g_inputmode == INPUT_MODE_CHAR)
    {
        g_charbuf2D[g_chars[g_current]].charID = g_inputbuf[0];
    }
    else
    {
        u32 color = 0x000000ff;

        switch (g_inputbuf[0]) {
        case 'w':
            color = COLOR_WHITE;
            break;
        case 'r':
            color = COLOR_RED;
            break;
        case 'g':
            color = COLOR_L_GREEN;
            break;
        case 'b':
            color = COLOR_BLUE;
            break;
        case 'y':
            color = COLOR_YELLOW;
            break;
        case 'a':
            color = COLOR_GOLD;
            break;
        case 'o':
            color = COLOR_D_GREEN;
            break;
        }

        if (color != 0x000000ff)
            g_charbuf2D[g_chars[g_current]].color = color;
    }
}