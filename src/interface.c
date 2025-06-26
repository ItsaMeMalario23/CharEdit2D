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
bool mGrid = 0;

f32 gRenderscale = WINDOW_SCALE;
f32 gRenderscroll = 0.0f;
vec2f_t gRenderoffset = {0.0f, 0.0f};

u32 gInmode = INPUT_MODE_STD;

char gInputbuf[32];
u32 gInidx = 0;

char2Idx gCharbuf[ASCII_MAX_2D_CHARS];
char2Idx gNumchars = 0;
char2Idx gCurrentchar = -1;

vec2f_t gAnchorpos = {0.0f, 0.0f};
vec2f_t gAnchortmp = {0.0f, 0.0f};

frame_t gBaseframe;
frame_t* gCurrentframe = &gBaseframe;
u32 gFrameidx = 1;
u32 gNumframes = 1;

bool mMultiselect = 0;
u32 gNumselected = 0;
static u32 gMultiselbufsize;
char2Idx* gMultiselbuf = NULL;

i32 gEditcol = -1;
u32 gColors[4] = {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};

static input_f inputMap[INPUT_MAP_SIZE];

//
//  Local functions
//
static const char shiftNums[10] = {'=', '!', '\"', '\\', '$', '%', '&', '/', '(', ')'};
static const char altNums[10] = {0, 0, 0, 0, '?', 96, '{', '[', ']', '}'};

static char getInputChar(char input)
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

static void setCharColor(u32 color)
{
    rAssert(color);

    if (mMultiselect)
    {
        for (u32 i = 0; i < gNumselected; i++) {
            if (gCharbuf[i] >= 0)
                g_charbuf2D[gCharbuf[gMultiselbuf[i]]].color = color;
        }
    }
    else
    {
        if (gCurrentchar < 0)
            return;

        g_charbuf2D[gCharbuf[gCurrentchar]].color = color;
    }
}

static void processTxtInput(void)
{
    rAssert(gInmode == INPUT_MODE_CHAR || gInmode == INPUT_MODE_COLOR || gInmode == INPUT_MODE_SCOLOR);
    rAssert(gInidx <= 32);
    
    if (!gInidx || gInputbuf[0] < 32 || (gCurrentchar < 0 && gInmode != INPUT_MODE_SCOLOR))
        return;

    if (gInmode == INPUT_MODE_CHAR)
    {
        g_charbuf2D[gCharbuf[gCurrentchar]].charID = gInputbuf[0];
    }
    else if (gInmode == INPUT_MODE_COLOR)
    {
        u32 color = 0x000000ff;

        switch (gInputbuf[0]) {
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
            setCharColor(color);
    }
    else if (gInmode == INPUT_MODE_SCOLOR)
    {
        if (gInidx != 6 || gEditcol < 0) {
            SDL_Log("Inidx %d edtc %d", gInidx, gEditcol);
            return;
        }

        gInputbuf[6] = '\0';

        u32 tmp = strtoul(gInputbuf, NULL, 16);

        gColors[gEditcol] = (tmp << 8) | 0xff;
        gEditcol = -1;
    }
}

//
//  Std input callbacks
//
static void addCharCallback(void)
{
    if (gNumchars >= ASCII_MAX_2D_CHARS) {
        SDL_Log("Failed to add char, buffer full");
        return;
    }
    
    f32 x = gCurrentchar < 0 ? 10.0f : g_charbuf2D[gCharbuf[gCurrentchar]].xpos + 16.0f;
    f32 y = gCurrentchar < 0 ? 10.0f : g_charbuf2D[gCharbuf[gCurrentchar]].ypos;

    char2Idx i = asciiChar2D(x, y, COLOR_WHITE, '=');

    if (i < 0) {
        SDL_Log("ERROR: create ascii char returned invalid index");
        return;
    }

    gCurrentchar = gNumchars;
    gCharbuf[gNumchars++] = i;
}

static void mvUpCallback(void)
{
    if (g_charbuf2D && !mMultiselect && gCurrentchar >= 0) {
        g_charbuf2D[gCharbuf[gCurrentchar]].ypos -= g_shift ? SHIFT_ACCEL : 1.0f;
        return;
    }

    for (u32 i = 0; i < gNumselected; i++) {
        if (gCharbuf[i] >= 0)
            g_charbuf2D[gCharbuf[gMultiselbuf[i]]].ypos -= g_shift ? SHIFT_ACCEL : 1.0f;
    }
}

static void mvLeftCallback(void)
{
    if (g_charbuf2D && !mMultiselect && gCurrentchar >= 0) {
        g_charbuf2D[gCharbuf[gCurrentchar]].xpos -= g_shift ? SHIFT_ACCEL : 1.0f;
        return;
    }

    for (u32 i = 0; i < gNumselected; i++) {
        if (gCharbuf[i] >= 0)
            g_charbuf2D[gCharbuf[gMultiselbuf[i]]].xpos -= g_shift ? SHIFT_ACCEL : 1.0f;
    }
}

static void mvDownCallback(void)
{
    if (g_charbuf2D && !mMultiselect && gCurrentchar >= 0) {
        g_charbuf2D[gCharbuf[gCurrentchar]].ypos += g_shift ? SHIFT_ACCEL : 1.0f;
        return;
    }

    for (u32 i = 0; i < gNumselected; i++) {
        if (gCharbuf[i] >= 0)
            g_charbuf2D[gCharbuf[gMultiselbuf[i]]].ypos += g_shift ? SHIFT_ACCEL : 1.0f;
    }
}

static void mvRightCallback(void)
{
    if (g_charbuf2D && !mMultiselect && gCurrentchar >= 0) {
        g_charbuf2D[gCharbuf[gCurrentchar]].xpos += g_shift ? SHIFT_ACCEL : 1.0f;
        return;
    }

    for (u32 i = 0; i < gNumselected; i++) {
        if (gCharbuf[i] >= 0)
            g_charbuf2D[gCharbuf[gMultiselbuf[i]]].xpos += g_shift ? SHIFT_ACCEL : 1.0f;
    }
}

static void chColorCallback(void)
{
    gInmode = INPUT_MODE_COLOR;
}

static void chCharCallback(void)
{
    gInmode = INPUT_MODE_CHAR;
}

static void charUpCallback(void)
{
    if (gCurrentchar > 0)
        gCurrentchar--;
}

static void charDownCallback(void)
{
    if (gCurrentchar >= 0 && gCurrentchar + 1 < gNumchars)
        gCurrentchar++;
}

static void charUpWrapCallback(void)
{
    if (gCurrentchar >= 0 && ++gCurrentchar >= gNumchars)
        gCurrentchar = 0;
}

static void createStructCallback(void)
{
    if (!gNumchars)
        return;

    char buf[256];
    ascii2_t* asc;
    u32 idx = 0;
    u32 chr = 0;

    snprintf(buf, 256, "\nconst ascii2info_t obj[%d] = {", gNumchars);
    SDL_Log(buf);

    buf[0] = '\t';
    idx = 1;

    for (;;) {
        asc = g_charbuf2D + gCharbuf[chr];

        idx += snprintf(buf + idx, 256 - idx, "{\'%c\', 0x%x, {%.0f, %.0f}}, ", asc->charID, asc->color, asc->xpos - gAnchorpos.x, asc->ypos - gAnchorpos.y);

        if (++chr >= gNumchars) {
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
    if (!gNumchars)
        return;

    char buf[256];
    ascii2_t* asc;
    u32 idx = 0;
    u32 chr = 0;

    snprintf(buf, 256, "\nconst ascii2info_t obj[%d] = {", gNumchars);
    SDL_Log(buf);

    buf[0] = '\t';
    idx = 1;

    for (;;) {
        asc = g_charbuf2D + gCharbuf[chr];

        idx += snprintf
        (
            buf + idx, 256 - idx,
            "{\'%c\', 0x%x, {%.7ff, %.7ff}}, ",
            asc->charID,
            asc->color | 0x000000ff,
            (asc->xpos - gAnchorpos.x) / 640.0f,
            -((asc->ypos - gAnchorpos.y + (64.0f * ASCII_RENDER_SCALE)) / 360.0f)
        );

        if (++chr >= gNumchars) {
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
    gInmode = INPUT_MODE_ANCHOR;
}

static void zoomInCallback(void)
{
    if (gRenderscale < 2.8f)
        gRenderscale += 0.25f;
}

static void zoomOutCallback(void)
{
    if (gRenderscale > 0.4f)
        gRenderscale -= 0.25f;
}

static void scrUpCallback(void)
{
    gRenderoffset.y += g_shift ? roundf(SHIFT_ACCEL * gRenderscale) : 1.0f;
}

static void scrLeftCallback(void)
{
    gRenderoffset.x += g_shift ? roundf(SHIFT_ACCEL * gRenderscale) : 1.0f;
}

static void scrDownCallback(void)
{
    gRenderoffset.y -= g_shift ? roundf(SHIFT_ACCEL * gRenderscale) : 1.0f;
}

static void scrRightCallback(void)
{
    gRenderoffset.x -= g_shift ? roundf(SHIFT_ACCEL * gRenderscale) : 1.0f;
}

static void addFrameCallback(void)
{    
    rAssert(gCurrentframe);

    frame_t* tmp = (frame_t*) memAlloc(sizeof(frame_t));

    *tmp = (frame_t) { .bufsize = ANIM_BUF_SIZE, .positions = NULL, .prev = gCurrentframe, .next = gCurrentframe->next };

    updateFrameBuf(gCurrentframe);
    updateFrameBuf(tmp);

    gCurrentframe->next = tmp;
    gCurrentframe = tmp;
    gNumframes++;
    gFrameidx++;
}

static void frameStepForward(void)
{
    if (gCurrentframe && gCurrentframe->next) {
        updateFrameBuf(gCurrentframe);
        updateCharBuf(gCurrentframe->next);
        gCurrentframe = gCurrentframe->next;
        gFrameidx++;
    }
}

static void frameStepBackward(void)
{
    if (gCurrentframe && gCurrentframe->prev) {
        updateFrameBuf(gCurrentframe);
        updateCharBuf(gCurrentframe->prev);
        gCurrentframe = gCurrentframe->prev;
        gFrameidx--;
    }
}

static void toggleMultiselect(void)
{
    rAssert(gMultiselbuf);

    mMultiselect = !mMultiselect;
    gNumselected = 0;
}

static void color1(void)
{
    if (g_shift) {
        gInmode = INPUT_MODE_SCOLOR;
        gEditcol = 0;
        return;
    }

    setCharColor(gColors[0]);
}

static void color2(void)
{
    if (g_shift) {
        gInmode = INPUT_MODE_SCOLOR;
        gEditcol = 1;
        return;
    }

    setCharColor(gColors[1]);
}

static void color3(void)
{
    if (g_shift) {
        gInmode = INPUT_MODE_SCOLOR;
        gEditcol = 2;
        return;
    }

    setCharColor(gColors[2]);
}

static void color4(void)
{
    if (g_shift) {
        gInmode = INPUT_MODE_SCOLOR;
        gEditcol = 3;
        return;
    }

    setCharColor(gColors[3]);
}

static void toggleGrid(void)
{
    mGrid = !mGrid;
}

//
//  Public functions
//

void initInterface(void)
{
    for (u32 i = 0; i < ASCII_MAX_2D_CHARS; i++)
        gCharbuf[i] = -1;

    memset(gInputbuf, 0, sizeof(gInputbuf));
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
    inputMap[INPUT_M] = toggleMultiselect;
    inputMap[INPUT_1] = color1;
    inputMap[INPUT_2] = color2;
    inputMap[INPUT_3] = color3;
    inputMap[INPUT_4] = color4;
    inputMap[INPUT_G] = toggleGrid;

    vec2f_t* tmp = (vec2f_t*) memAllocInit(sizeof(vec2f_t), ANIM_BUF_SIZE);

    gBaseframe = (frame_t) { .next = NULL, .prev = NULL, .positions = tmp, .bufsize = ANIM_BUF_SIZE };

    gNumselected = 0;
    gMultiselbufsize = 128;
    gMultiselbuf = (char2Idx*) memAlloc(sizeof(char2Idx) * gMultiselbufsize);
}

void loadObject(const ascii2info_t* object, u32 len)
{
    rAssert(object);
    rAssert(len);
    
    if (gNumchars + len > ASCII_MAX_2D_CHARS) {
        SDL_Log("Failed to load object, insufficient buf size");
        return;
    }

    charIdx idx;

    for (u32 i = 0; i < len; i++) {
        idx = asciiChar2D(object[i].pos.x * 640.0f, -((object[i].pos.y * 360.0f) - (64.0f * ASCII_RENDER_SCALE)), object[i].color, object[i].charID);
        //idx = asciiChar2D(object[i].pos.x, object[i].pos.y, object[i].color, object[i].charID);

        if (idx < 0) {
            SDL_Log("ERROR: create ascii char returned invalid index while loading object, aborting");
            return;
        }

        gCharbuf[gNumchars++] = idx;
    }

    gCurrentchar = gCharbuf[gNumchars - 1];
}

void updateCharBuf(frame_t* restrict frame)
{
    rAssert(frame);
    rAssert(frame->positions);
    rAssert(frame->bufsize > gNumchars);

    for (u32 i = 0; i < gNumchars; i++) {
        g_charbuf2D[gCharbuf[i]].xpos = frame->positions[i].x;
        g_charbuf2D[gCharbuf[i]].ypos = frame->positions[i].y;
    }
}

void updateFrameBuf(frame_t* restrict frame)
{
    rAssert(frame);

    u64 size = frame->bufsize;

    while (size <= gNumchars)
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

    for (u32 i = 0; i < gNumchars; i++) {
        frame->positions[i].x = g_charbuf2D[gCharbuf[i]].xpos;
        frame->positions[i].y = g_charbuf2D[gCharbuf[i]].ypos;
    }
}

void addCharToSelection(i32 idx)
{
    rAssert(gMultiselbuf);
    
    if (idx < 0) {
        SDL_Log("[ERROR] Failed to add char to selection, invalid id");
        return;
    }

    if (gNumselected >= gMultiselbufsize)
        gMultiselbuf = (char2Idx*) memRealloc(gMultiselbuf, (gMultiselbufsize += 128));

    // remove if exists
    for (u32 i = 0; i < gNumselected; i++) {
        if (gMultiselbuf[i] == idx) {
            if (i == gNumselected - 1) {
                gNumselected--;
                return;
            }

            memmove(gMultiselbuf + i, gMultiselbuf + i + 1, sizeof(char2Idx) * (gNumselected + i));
            gNumselected--;
            return;
        }
    }

    // add if not exists
    gMultiselbuf[gNumselected++] = idx;
}

//
//  Input handler functions
//

SDL_AppResult txtInputMode(SDL_Keycode input)
{
    if (input == SDLK_RETURN) {
        processTxtInput();

        gInmode = INPUT_MODE_STD;
        gInidx = 0;

        return SDL_APP_CONTINUE;
    }

    if (input == SDLK_ESCAPE) {
        gInmode = INPUT_MODE_STD;
        gInidx = 0;

        return SDL_APP_CONTINUE;
    }

    if (input == SDLK_BACKSPACE)     // backspace
    {
        if (gInidx)
            gInidx--;

        return SDL_APP_CONTINUE;
    }
    else if (input < 32 && input > 127)     // ignore
    {
        return SDL_APP_CONTINUE;
    }

    char c = getInputChar(input);

    if (!c)
        return SDL_APP_CONTINUE;

    if (gInmode == INPUT_MODE_CHAR || gInmode == INPUT_MODE_COLOR) {
        // single char input mode
        gInputbuf[0] = c;
        gInidx = 1;
    } else if (gInidx < 32) {
        // multi char input
        gInputbuf[gInidx++] = c;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult anchorInputMode(SDL_Keycode input)
{
    switch (input) {
    case SDLK_RETURN:
        gAnchorpos.x += gAnchortmp.x;
        gAnchorpos.y += gAnchortmp.y;
        // intentional falltrough
    case SDLK_ESCAPE:
    case SDLK_R:
        gInmode = INPUT_MODE_STD;
        gAnchortmp.x = 0.0f;
        gAnchortmp.y = 0.0f;
        break;

    case SDLK_W:
        gAnchortmp.y -= g_shift ? SHIFT_ACCEL : 1.0f;
        break;

    case SDLK_A:
        gAnchortmp.x -= g_shift ? SHIFT_ACCEL : 1.0f;
        break;

    case SDLK_S:
        gAnchortmp.y += g_shift ? SHIFT_ACCEL : 1.0f;
        break;

    case SDLK_D:
        gAnchortmp.x += g_shift ? SHIFT_ACCEL : 1.0f;
        break;
    }

    return SDL_APP_CONTINUE;
}

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

    case SDLK_COLON:
        if (inputMap[INPUT_DOT])
            inputMap[INPUT_DOT]();
        break;

    case SDLK_COMMA:
        if (inputMap[INPUT_COMMA])
            inputMap[INPUT_COMMA]();
        break;

    case SDLK_M:
        if (inputMap[INPUT_M])
            inputMap[INPUT_M]();
        break;

    case SDLK_1:
        if (inputMap[INPUT_1])
            inputMap[INPUT_1]();
        break;

    case SDLK_2:
        if (inputMap[INPUT_2])
            inputMap[INPUT_2]();
        break;

    case SDLK_3:
        if (inputMap[INPUT_3])
            inputMap[INPUT_3]();
        break;

    case SDLK_4:
        if (inputMap[INPUT_4])
            inputMap[INPUT_4]();
        break;

    case SDLK_G:
        if (inputMap[INPUT_G])
            inputMap[INPUT_G]();
        break;
    }

    return SDL_APP_CONTINUE;
}