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

f32 g_rscale = WINDOW_SCALE;
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

f32 g_scrollheight = 0.0f;

static input_f inputMap[INPUT_MAP_SIZE];

//
//  Std input callbacks
//
static void addCharCallback(void)
{
    if (g_charidx >= ASCII_MAX_2D_CHARS) {
        SDL_Log("Failed to add char, buffer full");
        return;
    }
    
    f32 x = g_current < 0 ? 10.0f : g_charbuf2D[g_chars[g_current]].xpos + 16.0f;
    f32 y = g_current < 0 ? 10.0f : g_charbuf2D[g_chars[g_current]].ypos;

    char2Idx i = asciiChar2D(x, y, COLOR_WHITE, '=');

    if (i < 0) {
        SDL_Log("ERROR: create ascii char returned invalid index");
        return;
    }

    g_current = g_charidx;
    g_chars[g_charidx++] = i;
}

static void mvUpCallback(void)
{
    if (g_charbuf2D && g_current >= 0)
        g_charbuf2D[g_chars[g_current]].ypos -= g_shift ? SHIFT_ACCEL : 1.0f;
}

static void mvLeftCallback(void)
{
    if (g_charbuf2D && g_current >= 0)
        g_charbuf2D[g_chars[g_current]].xpos -= g_shift ? SHIFT_ACCEL : 1.0f;
}

static void mvDownCallback(void)
{
    if (g_charbuf2D && g_current >= 0)
        g_charbuf2D[g_chars[g_current]].ypos += g_shift ? SHIFT_ACCEL : 1.0f;
}

static void mvRightCallback(void)
{
    if (g_charbuf2D && g_current >= 0)
        g_charbuf2D[g_chars[g_current]].xpos += g_shift ? SHIFT_ACCEL : 1.0f;
}

static void chColorCallback(void)
{
    g_inputmode = INPUT_MODE_COLOR;
}

static void chCharCallback(void)
{
    g_inputmode = INPUT_MODE_CHAR;
}

static void charUpCallback(void)
{
    if (g_current > 0)
        g_current--;
}

static void charDownCallback(void)
{
    if (g_current >= 0 && g_current + 1 < g_charidx)
        g_current++;
}

static void charUpWrapCallback(void)
{
    if (g_current >= 0 && ++g_current >= g_charidx)
        g_current = 0;
}

static void createStructCallback(void)
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

static void createStructShaderCompatibility(void)
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

        idx += snprintf
        (
            buf + idx, 256 - idx,
            "{\'%c\', 0x%x, {%.7ff, %.7ff}}, ",
            asc->charID,
            asc->color | 0x000000ff,
            (asc->xpos - g_anchor.x) / 640.0f,
            -((asc->ypos - g_anchor.y + (64.0f * ASCII_RENDER_SCALE)) / 360.0f)
        );

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

static void toggleBoxCallback(void)
{
    g_box = !g_box;
}

static void editAnchorCallback(void)
{
    g_inputmode = INPUT_MODE_ANCHOR;
}

static void zoomInCallback(void)
{
    if (g_rscale < 2.8f)
        g_rscale += 0.25f;
}

static void zoomOutCallback(void)
{
    if (g_rscale > 0.4f)
        g_rscale -= 0.25f;
}

static void scrUpCallback(void)
{
    g_roffset.y += g_shift ? roundf(SHIFT_ACCEL * g_rscale) : 1.0f;
}

static void scrLeftCallback(void)
{
    g_roffset.x += g_shift ? roundf(SHIFT_ACCEL * g_rscale) : 1.0f;
}

static void scrDownCallback(void)
{
    g_roffset.y -= g_shift ? roundf(SHIFT_ACCEL * g_rscale) : 1.0f;
}

static void scrRightCallback(void)
{
    g_roffset.x -= g_shift ? roundf(SHIFT_ACCEL * g_rscale) : 1.0f;
}

static void addFrameCallback(void)
{    
    rAssert(g_currentframe);

    frame_t* tmp = (frame_t*) memAlloc(sizeof(frame_t));

    *tmp = (frame_t) { .bufsize = ANIM_BUF_SIZE, .positions = NULL, .prev = g_currentframe, .next = g_currentframe->next };

    updateFrameBuf(g_currentframe);
    updateFrameBuf(tmp);

    g_currentframe->next = tmp;
    g_currentframe = tmp;
    g_numframes++;
    g_frameidx++;
}

static void frameStepForward(void)
{
    if (g_currentframe && g_currentframe->next) {
        updateFrameBuf(g_currentframe);
        updateCharBuf(g_currentframe->next);
        g_currentframe = g_currentframe->next;
        g_frameidx++;
    }
}

static void frameStepBackward(void)
{
    if (g_currentframe && g_currentframe->prev) {
        updateFrameBuf(g_currentframe);
        updateCharBuf(g_currentframe->prev);
        g_currentframe = g_currentframe->prev;
        g_frameidx--;
    }
}

void initInterface(void)
{
    for (u32 i = 0; i < ASCII_MAX_2D_CHARS; i++)
        g_chars[i] = -1;

    memset(g_inputbuf, 0, sizeof(g_inputbuf));
    memset(inputMap, 0, sizeof(inputMap));

    // keyboard mapping
    inputMap[INPUT_W] = mvUpCallback;
    inputMap[INPUT_A] = mvLeftCallback;
    inputMap[INPUT_S] = mvDownCallback;
    inputMap[INPUT_D] = mvRightCallback;
    inputMap[INPUT_P] = addCharCallback;
    inputMap[INPUT_C] = chColorCallback;
    inputMap[INPUT_Z] = chCharCallback;
    inputMap[INPUT_PGUP] = charUpCallback;
    inputMap[INPUT_PGDOWN] = charDownCallback;
    inputMap[INPUT_Y] = createStructCallback;
    inputMap[INPUT_X] = createStructShaderCompatibility;
    inputMap[INPUT_H] = toggleBoxCallback;
    inputMap[INPUT_R] = editAnchorCallback;
    inputMap[INPUT_PLUS] = zoomInCallback;
    inputMap[INPUT_MINUS] = zoomOutCallback;
    inputMap[INPUT_UP] = scrUpCallback;
    inputMap[INPUT_LEFT] = scrLeftCallback;
    inputMap[INPUT_DOWN] = scrDownCallback;
    inputMap[INPUT_RIGHT] = scrRightCallback;
    inputMap[INPUT_TAB] = charUpWrapCallback;
    inputMap[INPUT_O] = addFrameCallback;
    inputMap[INPUT_DOT] = frameStepForward;
    inputMap[INPUT_COMMA] = frameStepBackward;

    vec2f_t* tmp = (vec2f_t*) memAllocInit(sizeof(vec2f_t), ANIM_BUF_SIZE);

    g_baseframe = (frame_t) { .next = NULL, .prev = NULL, .positions = tmp, .bufsize = ANIM_BUF_SIZE };
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
        //idx = asciiChar2D(object[i].pos.x * 640.0f, -((object[i].pos.y * 360.0f) - (64.0f * ASCII_RENDER_SCALE)), object[i].color, object[i].charID);
        idx = asciiChar2D(object[i].pos.x, object[i].pos.y, object[i].color, object[i].charID);

        if (idx < 0) {
            SDL_Log("ERROR: create ascii char returned invalid index while loading object, aborting");
            return;
        }

        g_chars[g_charidx++] = idx;
    }

    g_current = g_chars[g_charidx - 1];
}

void updateCharBuf(frame_t* restrict frame)
{
    rAssert(frame);
    rAssert(frame->positions);
    rAssert(frame->bufsize > g_charidx);

    for (u32 i = 0; i < g_charidx; i++) {
        g_charbuf2D[g_chars[i]].xpos = frame->positions[i].x;
        g_charbuf2D[g_chars[i]].ypos = frame->positions[i].y;
    }
}

void updateFrameBuf(frame_t* restrict frame)
{
    rAssert(frame);

    u64 size = frame->bufsize;

    while (size <= g_charidx)
        size += 32;

    rAssert(size < 1024);

    if (frame->positions && frame->bufsize != size)
    {
        frame->positions = (vec2f_t*) memRealloc(frame->positions, sizeof(vec2f_t) * size);
        frame->bufsize = size;
    }
    else if (!frame->positions)
    {
        frame->positions = (vec2f_t*) memAlloc(sizeof(vec2f_t) * size);
        frame->bufsize = size;
    }

    for (u32 i = 0; i < g_charidx; i++) {
        frame->positions[i].x = g_charbuf2D[g_chars[i]].xpos;
        frame->positions[i].y = g_charbuf2D[g_chars[i]].ypos;
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
        if (inputMap[INPUT_UP])
            inputMap[INPUT_UP]();
        break;

    case SDLK_DOWN:
        if (inputMap[INPUT_DOWN])
            inputMap[INPUT_DOWN]();
        break;

    case SDLK_LEFT:
        if (inputMap[INPUT_LEFT])
            inputMap[INPUT_LEFT]();
        break;

    case SDLK_RIGHT:
        if (inputMap[INPUT_RIGHT])
            inputMap[INPUT_RIGHT]();
        break;

    case SDLK_W:
        if (inputMap[INPUT_W])
            inputMap[INPUT_W]();
        break;

    case SDLK_A:
        if (inputMap[INPUT_A])
            inputMap[INPUT_A]();
        break;

    case SDLK_S:
        if (inputMap[INPUT_S])
            inputMap[INPUT_S]();
        break;

    case SDLK_D:
        if (inputMap[INPUT_D])
            inputMap[INPUT_D]();
        break;

    case SDLK_C:
        if (inputMap[INPUT_C])
            inputMap[INPUT_C]();
        break;

    case SDLK_Z:
        if (inputMap[INPUT_Z])
            inputMap[INPUT_Z]();
        break;

    case SDLK_P:
        if (inputMap[INPUT_P])
            inputMap[INPUT_P]();
        break;

    case SDLK_PAGEUP:
        if (inputMap[INPUT_PGUP])
            inputMap[INPUT_PGUP]();
        break;

    case SDLK_PAGEDOWN:
        if (inputMap[INPUT_PGDOWN])
            inputMap[INPUT_PGDOWN]();
        break;

    case SDLK_X:
        if (inputMap[INPUT_X])
            inputMap[INPUT_X]();
        break;

    case SDLK_Y:
        if (inputMap[INPUT_Y])
            inputMap[INPUT_Y]();
        break;

    case SDLK_H:
        if (inputMap[INPUT_H])
            inputMap[INPUT_H]();
        break;

    case SDLK_R:
        if (inputMap[INPUT_R])
            inputMap[INPUT_R]();
        break;

    case SDLK_PLUS:
        if (inputMap[INPUT_PLUS])
            inputMap[INPUT_PLUS]();
        break;

    case SDLK_MINUS:
        if (inputMap[INPUT_MINUS])
            inputMap[INPUT_MINUS]();
        break;

    case SDLK_TAB:
        if (inputMap[INPUT_TAB])
            inputMap[INPUT_TAB]();
        break;

    case SDLK_O:
        if (inputMap[INPUT_O])
            inputMap[INPUT_O]();
        break;

    case SDLK_M:
    case SDLK_COLON:
        if (inputMap[INPUT_DOT])
            inputMap[INPUT_DOT]();
        break;

    case SDLK_N:
    case SDLK_COMMA:
        if (inputMap[INPUT_COMMA])
            inputMap[INPUT_COMMA]();
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