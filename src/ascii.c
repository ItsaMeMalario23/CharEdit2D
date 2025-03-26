#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>

#include <ascii.h>
#include <render.h>
#include <debug/rdebug.h>
#include <debug/memtrack.h>

char g_asciiRenderBufMem[ASCII_RENDER_BUF_SIZE];
char g_asciiCharBufMem[ASCII_CHAR_BUF_SIZE];

ascii2_t* g_renderbuf2D = (ascii2_t*) g_asciiRenderBufMem;
ascii3_t* g_renderbuf3D = (ascii3_t*) g_asciiRenderBufMem;

ascii2_t* g_charbuf2D = (ascii2_t*) g_asciiCharBufMem;
ascii3_t* g_charbuf3D = (ascii3_t*) g_asciiCharBufMem;

u32 g_renderbufIdx = 0;
u32 g_charbufIdx = 0;

asciiobj_t g_asciiObjBuf[ASCII_OBJ_BUF_SIZE];
asciiobj_t* g_objidx = g_asciiObjBuf;

u32 g_asciistate = ASCII_RENDER_UNINIT;

u16 g_charMemFragmented = 0;
u16 g_objMemFragmented = 0;

pageinfo_t* g_pagedmem = NULL;

// initialize ascii renderer
void initAscii(u32 renderMode)
{
    rAssert(renderMode == ASCII_RENDER_MODE_2D || renderMode == ASCII_RENDER_MODE_3D);

    memset(g_asciiRenderBufMem, 0, sizeof(g_asciiRenderBufMem));
    memset(g_asciiCharBufMem, 0, sizeof(g_asciiCharBufMem));
    memset(g_asciiObjBuf, 0, sizeof(g_asciiObjBuf));

    g_asciistate = renderMode;
    g_objidx = g_asciiObjBuf;
    g_renderbufIdx = 0;
    g_charbufIdx = 0;
    g_charMemFragmented = 0;
    g_objMemFragmented = 0;
    g_pagedmem = NULL;

    if (renderMode == ASCII_RENDER_MODE_2D) {
        g_renderbuf2D = (ascii2_t*) g_asciiRenderBufMem;
        g_charbuf2D = (ascii2_t*) g_asciiCharBufMem;
        g_renderbuf3D = NULL;
        g_charbuf3D = NULL;
    } else {
        g_renderbuf2D = NULL;
        g_charbuf2D = NULL;
        g_renderbuf3D = (ascii3_t*) g_asciiRenderBufMem;
        g_charbuf3D = (ascii3_t*) g_asciiCharBufMem;
    }
}

// cleanup ascii renderer
void cleanupAscii(void)
{
    if (g_asciistate == ASCII_RENDER_UNINIT) {
        SDL_Log("ASCII cleanup unnecessary, not initialized");
        return;
    }

    if (g_pagedmem)
        freePages();

    g_renderbuf2D = NULL;
    g_renderbuf3D = NULL;
    g_charbuf2D = NULL;
    g_charbuf3D = NULL;
    g_objidx = NULL;
    g_asciistate = ASCII_RENDER_UNINIT;
}

// reset all buffers
void asciiResetAll(void)
{
    if (g_asciistate == ASCII_RENDER_UNINIT) {
        SDL_Log("ASCII reset failed, not initialized");
        return;
    }

    if (g_pagedmem)
        freePages();

    initAscii(g_asciistate);
}

// change between 2D and 3D render mode
void asciiChangeMode(u32 renderMode)
{
    if (g_asciistate == ASCII_RENDER_UNINIT) {
        SDL_Log("ASCII change mode failed, not initialized");
        return;
    }

    if (renderMode != ASCII_RENDER_MODE_2D && renderMode != ASCII_RENDER_MODE_3D) {
        SDL_Log("ASCII change mode failed, invalid render mode");
        return;
    }

    if (g_asciistate == renderMode)
        SDL_Log("ASCII change mode unnecessary, resetting buffers");

    g_asciistate = renderMode;

    asciiResetAll();
}

// render ascii buf
void asciiRenderAll(u32 backgroundColor, u16 clearScr, u16 preserveRenderBuf)
{
    if (g_asciistate == ASCII_RENDER_MODE_2D)
        asciiRender2D(backgroundColor, clearScr, preserveRenderBuf);
    else if (g_asciistate == ASCII_RENDER_MODE_3D)
        asciiRender3D(backgroundColor, clearScr, preserveRenderBuf);
    else
        SDL_Log("Failed to render ASCII, invalid state");
}

// render ascii buf
void asciiRender2D(u32 backgroundColor, u16 clearScr, u16 preserveRenderBuf)
{
    rAssert(g_asciistate == ASCII_RENDER_MODE_2D);

    if (clearScr)
        clearScreen(backgroundColor);

    renderBuf2D(g_renderbuf2D, g_renderbufIdx, 0.0f, 0.0f);

    if (!preserveRenderBuf)
        g_renderbufIdx = 0;
}

// render ascii buf
void asciiRender3D(u32 backgroundColor, u16 clearScr, u16 preserveRenderBuf)
{
    rAssert(g_asciistate == ASCII_RENDER_MODE_3D);

    if (clearScr)
        clearScreen(backgroundColor);

    renderBuf3D(g_renderbuf3D, g_renderbufIdx, 0.0f, 0.0f, 0.0f);

    if (!preserveRenderBuf)
        g_renderbufIdx = 0;
}

// render all 2D ascii characters in char buf once
void asciiRenderCharBuf2D(u32 backgroundColor, u32 clearScr, f32 dx, f32 dy)
{
    rAssert(g_asciistate == ASCII_RENDER_MODE_2D);
    rAssert(g_charbuf2D);

    if (clearScr)
        clearScreen(backgroundColor);

    if (!g_charMemFragmented) {
        renderBuf2D(g_charbuf2D, g_charbufIdx, dx, dy);
        return;
    }

    for (u32 i = 0; i < ASCII_MAX_2D_CHARS; i++) {
        if (!g_charbuf2D[i].visible || g_charbuf2D[i].charID < 32)
            continue;

        renderCharColor
        (
            (i16) roundf(g_charbuf2D[i].xpos + dx),
            (i16) roundf(g_charbuf2D[i].ypos + dy),
            ASCII_RENDER_SCALE,
            g_charbuf2D[i].color,
            g_charbuf2D[i].charID
        );
    }
}

// render all 3D ascii characters in char buf once
void asciiRenderCharBuf3D(u32 backgroundColor, u32 clearScr, f32 dx, f32 dy, f32 dz)
{
    rAssert(g_asciistate == ASCII_RENDER_MODE_3D);
    rAssert(g_charbuf3D);

    if (clearScr)
        clearScreen(backgroundColor);

    if (!g_charMemFragmented) {
        renderBuf3D(g_charbuf3D, g_charbufIdx, dx, dy, dz);
        return;
    }

    for (u32 i = 0; i < ASCII_MAX_3D_CHARS; i++) {
        if (!g_charbuf3D[i].visible || g_charbuf3D[i].charID < 32)
            continue;

        renderCharColor
        (
            (i16) roundf(g_charbuf3D[i].xpos + dx),
            (i16) roundf(g_charbuf3D[i].ypos + dy),
            ASCII_RENDER_SCALE,
            g_charbuf3D[i].color,
            g_charbuf3D[i].charID
        );
    }
}

// write 2D object to render buf
void asciiRenderObject2D(asciiobj_t* object)
{
    rAssert(object);
    rAssert(object->data.ascii2);
    rAssert(object->type == ASCII_OBJ_2D);
    rAssert(g_asciistate == ASCII_RENDER_MODE_2D);

    if (g_renderbufIdx + object->len >= ASCII_MAX_2D_CHARS) {
        SDL_Log("Failed to render 2D ASCII object, render buf full");
        return;
    }

    memcpy(g_renderbuf2D + g_renderbufIdx, object->data.ascii2, object->len * sizeof(ascii2_t));

    if (object->xpos > EPSILON || object->xpos < -EPSILON ||
        object->ypos > EPSILON || object->ypos < -EPSILON)
    {
        for (u32 i = 0; i < object->len; i++) {
            (g_renderbuf2D + i)->xpos += object->xpos;
            (g_renderbuf2D + i)->ypos += object->ypos;
        }
    }

    g_renderbufIdx += object->len;
}

// write 3D object to render buf
void asciiRenderObject3D(asciiobj_t* object)
{
    rAssert(object);
    rAssert(object->data.ascii2);
    rAssert(object->type == ASCII_OBJ_3D);
    rAssert(g_asciistate == ASCII_RENDER_MODE_3D);

    if (g_renderbufIdx + object->len >= ASCII_MAX_3D_CHARS) {
        SDL_Log("Failed to render 3D ASCII object, render buf full");
        return;
    }

    memcpy(g_renderbuf3D + g_renderbufIdx, object->data.ascii3, object->len * sizeof(ascii3_t));

    if (object->xpos > EPSILON || object->xpos < -EPSILON ||
        object->ypos > EPSILON || object->ypos < -EPSILON ||
        object->zpos > EPSILON || object->zpos < -EPSILON )
    {
        for (u32 i = 0; i < object->len; i++) {
            (g_renderbuf3D + i)->xpos += object->xpos;
            (g_renderbuf3D + i)->ypos += object->ypos;
            (g_renderbuf3D + i)->zpos += object->zpos;
        }
    }

    g_renderbufIdx += object->len;
}

// bypass render buf, render 2D object directly
void asciiRenderObjectDirect2D(asciiobj_t* object)
{
    rAssert(object);
    rAssert(object->data.ascii2);
    rAssert(object->type == ASCII_OBJ_2D);
    rAssert(g_asciistate == ASCII_RENDER_MODE_2D);

    renderBuf2D(object->data.ascii2, object->len, object->xpos, object->ypos);
}

// bypass render buf, render 3D object directly
void asciiRenderObjectDirect3D(asciiobj_t* object)
{
    rAssert(object);
    rAssert(object->data.ascii3);
    rAssert(object->type == ASCII_OBJ_3D);
    rAssert(g_asciistate == ASCII_RENDER_MODE_3D);

    renderBuf3D(object->data.ascii3, object->len, object->xpos, object->ypos, object->zpos);
}

// create 2D ascii character
char2Idx asciiChar2D(f32 xpos, f32 ypos, u32 color, u32 charID)
{
    rAssert(g_asciistate == ASCII_RENDER_MODE_2D);
    rAssert(g_charbuf2D);
    rAssert(charID > 31);

    char2Idx i = getCharIdx();

    if (i < 0) {
        SDL_Log("Failed to add 2D ASCII character, char buf full");
        return ASCII_INVALID_IDX;
    }

    g_charbuf2D[i].xpos = xpos;
    g_charbuf2D[i].ypos = ypos;
    g_charbuf2D[i].color = color;
    g_charbuf2D[i].charID = charID;
    g_charbuf2D[i].visible = 1;

    return i;
}

// create 3D ascii character
char3Idx asciiChar3D(f32 xpos, f32 ypos, f32 zpos, u32 color, u64 charID)
{
    rAssert(g_asciistate == ASCII_RENDER_MODE_3D);
    rAssert(g_charbuf3D);
    rAssert(charID > 31);

    char3Idx i = getCharIdx();

    if (i < 0) {
        SDL_Log("Failed to add 3D ASCII character, char buf full");
        return ASCII_INVALID_IDX;
    }

    g_charbuf3D[i].xpos = xpos;
    g_charbuf3D[i].ypos = ypos;
    g_charbuf3D[i].zpos = zpos;
    g_charbuf3D[i].color = color;
    g_charbuf3D[i].charID = charID;
    g_charbuf3D[i].visible = 1;

    return i;
}

// create 2D ascii character
char2Idx asciiChar2DIStruct(const ascii2info_t* info)
{
    rAssert(info);

    return asciiChar2D(info->pos.x, info->pos.y, info->color, info->charID);
}

// create 3D ascii character
char2Idx asciiChar3DIStruct(const ascii3info_t* info)
{
    rAssert(info);

    return asciiChar3D(info->pos.x, info->pos.y, info->pos.z, info->color, info->charID);
}

// create 2D ascii object
asciiobj_t* asciiObject2D(const vec2f_t* positions, const u32* colors, const u32* chars, u64 len)
{
    rAssert(chars);
    rAssert(positions);
    rAssert(g_asciistate == ASCII_RENDER_MODE_2D);

    asciiobj_t* object = getAsciiObj(ASCII_OBJ_2D, len);

    if (!object)
        return NULL;

    ascii2_t* chr;

    for (u32 i = 0; i < len; i++) {
        chr = object->data.ascii2 + i;

        chr->xpos = positions[i].x;
        chr->ypos = positions[i].y;
        chr->charID = chars[i];
        chr->visible = 1;

        rAssert(chr->charID > 31);

        if (colors)
            chr->color = colors[i];
        else
            chr->color = COLOR_WHITE;
    }

    return object;
}

// create 3D ascii object
asciiobj_t* asciiObject3D(const vec3f_t* positions, const u32* colors, const u32* chars, u64 len)
{
    rAssert(chars);
    rAssert(positions);
    rAssert(g_asciistate == ASCII_RENDER_MODE_3D);

    asciiobj_t* object = getAsciiObj(ASCII_OBJ_3D, len);

    if (!object)
        return NULL;

    ascii3_t* chr;

    for (u32 i = 0; i < len; i++) {
        chr = object->data.ascii3 + i;

        chr->xpos = positions[i].x;
        chr->ypos = positions[i].y;
        chr->zpos = positions[i].z;
        chr->charID = chars[i];
        chr->visible = 1;

        rAssert(chr->charID > 31);

        if (colors)
            chr->color = colors[i];
        else
            chr->color = COLOR_WHITE;
    }

    return object;
}

// create 2D ascii object
asciiobj_t* asciiObject2DIStruct(const ascii2info_t* info, u64 len)
{
    rAssert(info);
    rAssert(g_asciistate == ASCII_RENDER_MODE_2D);

    asciiobj_t* object = getAsciiObj(ASCII_OBJ_2D, len);

    if (!object)
        return NULL;

    ascii2_t* chr;

    for (u32 i = 0; i < len; i++) {
        chr = object->data.ascii2 + i;

        chr->xpos = info[i].pos.x;
        chr->ypos = info[i].pos.y;
        chr->color = info[i].color;
        chr->charID = info[i].charID;
        chr->visible = 1;

        rAssert(chr->charID > 31);
    }

    return object;
}

// create 3D ascii object
asciiobj_t* asciiObject3DIStruct(const ascii3info_t* info, u64 len)
{
    rAssert(info);
    rAssert(g_asciistate == ASCII_RENDER_MODE_3D);

    asciiobj_t* object = getAsciiObj(ASCII_OBJ_3D, len);

    if (!object)
        return NULL;

    ascii3_t* chr;

    for (u32 i = 0; i < len; i++) {
        chr = object->data.ascii3 + i;

        chr->xpos = info[i].pos.x;
        chr->ypos = info[i].pos.y;
        chr->zpos = info[i].pos.z;
        chr->color = info[i].color;
        chr->charID = info[i].charID;
        chr->visible = 1;

        rAssert(chr->charID > 31);
    }

    return object;
}

// render 2D char buf to screen
void renderBuf2D(const ascii2_t* buf, u64 len, f32 dx, f32 dy)
{
    rAssert(buf);
    
    for (u32 i = 0; i < len; i++) {
        if (!buf[i].visible || buf[i].charID < 32)
            continue;

        renderCharColor
        (
            (i16) roundf(buf[i].xpos + dx),
            (i16) roundf(buf[i].ypos + dy),
            ASCII_RENDER_SCALE,
            buf[i].color,
            buf[i].charID
        );
    }
}

// render 3D char buf to screen
void renderBuf3D(const ascii3_t* buf, u32 len, f32 dx, f32 dy, f32 dz)
{
    rAssert(buf);

    for (u32 i = 0; i < len; i++) {
        if (!buf[i].visible || buf[i].charID < 32)
            continue;

        renderCharColor
        (
            (i16) roundf(buf[i].xpos + dx),
            (i16) roundf(buf[i].ypos + dy),
            ASCII_RENDER_SCALE,
            buf[i].color,
            buf[i].charID
        );
    }
}
 
// init ascii object in object buf
asciiobj_t* getAsciiObj(u32 type, u32 len)
{
    rAssert(len);
    rAssert(type == ASCII_OBJ_2D || type == ASCII_OBJ_3D);

    if (g_objidx >= g_asciiObjBuf + ASCII_OBJ_BUF_SIZE) {
        SDL_Log("Failed to add ASCII object, object buffer full");
        return NULL;
    }

    g_objidx->type = type;
    g_objidx->len = len;
    g_objidx->visible = 1;

    g_objidx->xpos = 0.0f;
    g_objidx->ypos = 0.0f;
    g_objidx->zpos = 0.0f;

    g_objidx->paged = getAsciiObjMem((void**) &g_objidx->data, type, len);

    return g_objidx++;
}

// find memory for ascii object
bool getAsciiObjMem(void** dst, u32 type, u32 len)
{ 
    u32 count = 0;

    if (type == ASCII_OBJ_2D)
    {
        rAssert(g_asciistate == ASCII_RENDER_MODE_2D);

        for (u32 i = 0; i < ASCII_CHAR_BUF_SIZE; i++) {
            if (!(g_charbuf2D + i)->charID)
                count++;
            else
                count = 0;

            if (count >= len) {
                *(ascii2_t**) dst = g_charbuf2D + i + 1 - len;

                g_charbufIdx += len;

                return 0;
            }
        }

        ascii2_t* ptr = (ascii2_t*) memAlloc(len * sizeof(ascii2_t));

        memPage(ptr, ASCII_OBJ_2D, len);

        *(ascii2_t**) dst = ptr;

        return 1;
    }
    else
    {
        rAssert(g_asciistate == ASCII_RENDER_MODE_3D);

        for (u32 i = 0; i < ASCII_CHAR_BUF_SIZE; i++) {
            if (!(g_charbuf3D + i)->charID)
                count++;
            else
                count = 0;

            if (count >= len) {
                *(ascii3_t**) dst = (ascii3_t*) g_charbuf3D + i + 1 - len;

                g_charbufIdx += len;

                return 0;
            }
        }

        ascii3_t* ptr = (ascii3_t*) memAlloc(len * sizeof(ascii3_t));

        memPage(ptr, ASCII_OBJ_3D, len);

        *(ascii3_t**) dst = ptr;

        return 1;
    }
}

charIdx getCharIdx(void)
{
    if (!g_charMemFragmented && g_asciistate == ASCII_RENDER_MODE_2D)
        return g_charbufIdx < ASCII_MAX_2D_CHARS ? g_charbufIdx++ : ASCII_INVALID_IDX;
    else if (!g_charMemFragmented && g_asciistate == ASCII_RENDER_MODE_3D)
        return g_charbufIdx < ASCII_MAX_3D_CHARS ? g_charbufIdx++ : ASCII_INVALID_IDX;

    if (g_asciistate == ASCII_RENDER_MODE_2D)
    {
        for (u32 i = 0; i < ASCII_MAX_2D_CHARS; i++) {
            if (!g_charbuf2D[i].charID)
                return i;
        }

        return ASCII_INVALID_IDX;
    }
    else if (g_asciistate == ASCII_RENDER_MODE_3D)
    {
        for (u32 i = 0; i < ASCII_MAX_3D_CHARS; i++) {
            if (!g_charbuf3D[i].charID)
                return i;
        }

        return ASCII_INVALID_IDX;
    }
    else
    {
        SDL_Log("Failed to add ASCII char, not initialized");
        return ASCII_INVALID_IDX;
    }
}

// add memory page
void memPage(void* ptr, u32 type, u32 len)
{
    if (!g_pagedmem)
        g_pagedmem = (pageinfo_t*) memAlloc(sizeof(pageinfo_t));

    pageinfo_t* tmp = g_pagedmem;

    rAssert(tmp);

    while (tmp->next)
        tmp = tmp->next;

    tmp->next = (pageinfo_t*) memAlloc(sizeof(pageinfo_t));

    rAssert(tmp->next);

    tmp->next->ptr = ptr;
    tmp->next->len = len;
    tmp->next->type = type;
    tmp->next->next = NULL;

    SDL_Log("ASCII: added memory page");
}

// remove memory page
void removeMemPage(void* ptr)
{
    rAssert(ptr);
    rAssert(g_pagedmem);

    pageinfo_t* tmp;

    if (g_pagedmem->ptr == ptr) {
        tmp = g_pagedmem->next;

        memFree(g_pagedmem->ptr);
        memFree(g_pagedmem);

        g_pagedmem = tmp;

        SDL_Log("ASCII: removed memory page");

        return;
    }

    for (pageinfo_t* i = g_pagedmem; i; i = i->next) {
        if (i->next && i->next->ptr == ptr) {
            tmp = i->next->next;

            memFree(i->next->ptr);
            memFree(i->next);

            i->next = tmp;

            SDL_Log("ASCII: removed memory page");

            return;
        }
    }
}

// free all memory pages
void freePages(void)
{
    if (!g_pagedmem)
        return;

    pageinfo_t* tmp = NULL;

    for (pageinfo_t* i = g_pagedmem; i; i = i->next) {
        memFree(i->ptr);
        memFree(tmp);

        tmp = i;
    }

    memFree(tmp);

    g_pagedmem = NULL;
}