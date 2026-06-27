#ifndef RENDER_CONTEXT_H
#define RENDER_CONTEXT_H

#include "../cell.h"
#include "../tretis_config.h"

typedef struct render_context_t render_context_t;

render_context_t* createRenderContext(tretis_config_t config);
void destroyRenderContext(render_context_t** rcp);
void renderBegin(render_context_t* rc);
void renderCell(render_context_t* rc, int row, int col, cell_color_t color);
void renderGhostCell(render_context_t* rc, int row, int col, cell_color_t color);
void renderMiniCell(render_context_t* rc, int x, int y, int size, cell_color_t color);
void renderText(render_context_t* rc, int x, int y, const char* text);
void renderEnd(render_context_t* rc);

#endif
