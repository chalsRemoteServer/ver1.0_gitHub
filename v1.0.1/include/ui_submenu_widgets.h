/*
 * ui_submenu_widgets.h
 *
 *  Created on: 23 feb. 2026
 *      Author: oscar
 */

#pragma once
#include <stdint.h>
//
//typedef struct {
//    uint16_t id;          // TAG / ID del botón
//    const char* label;    // texto debajo
//
//    // Icono (bitmap en RAM_G)
//    uint32_t icon_mem;      // 0 = sin icono
//    uint16_t icon_w;        // default 64
//    uint16_t icon_h;        // default 64
//    uint8_t  icon_format;   // EVE_ARGB1555 normalmente
//
//} UiSubmPrimary;

typedef struct {
    uint16_t id;          // TAG / ID del botón
    const char* label;    // texto debajo

    uint32_t icon_mem;    // 0 = sin icono, !=0 = dirección en RAM_G (MEM_ICON_...)
} UiSubmPrimary;

typedef struct {
    uint16_t id;          // TAG / ID del botón secundario
    const char* text;     // texto dentro (por ahora: "X", "CFG", "^", "v" o "")
} UiSubmSecondary;

typedef struct {
    uint16_t x, y, w, h;  // rect del botón (sin incluir label)
} UiRect16;

uint8_t ui_subm_grid2x3_layout_dynamic(uint16_t gx0, uint16_t gy0, uint16_t gx1, uint16_t gy1, uint8_t nItems,
    uint16_t btn,            // 94
    uint16_t minGapX,        // ej. 8
    uint16_t gapY,           // ej. 24
    uint16_t labelGapY,      // ej. 8
    uint16_t labelH,         // ej. 24
    UiRect16 out_cards[6]
);

/* Calcula posiciones centradas para hasta 6 items (2x3).
   Devuelve cuántos se van a dibujar realmente (min(n,6)). */
uint8_t ui_subm_grid2x3_layout(
    const uint16_t content_x0, const uint16_t content_y0,
    const uint16_t content_x1, const uint16_t content_y1,
    uint8_t nItems,
    uint16_t cardW, uint16_t cardH,
    uint16_t gapX, uint16_t gapY,
    uint16_t labelGapY, uint16_t labelH,
    UiRect16 out_cards[6]
);

/* Dibuja título + grid 2x3 de botones vacíos + label debajo.
   (No íconos por ahora) */
void ui_subm_draw_primary_grid2x3(
    const char* title,
    uint16_t title_x, uint16_t title_y,
    uint16_t title_font,
    const UiSubmPrimary* items, uint8_t nItems,
    const UiRect16 cards[6], uint8_t drawCount,
    uint16_t button_font,
    uint16_t label_font,
    uint16_t labelGapY
);

/* Dibuja botones secundarios en barra derecha. Dibuja 1..4.
   xRight0 = coordenada x inicial (lado derecho).
   y0 = coordenada y de inicio.
*/
void ui_subm_draw_side_buttons(
    uint16_t xRight0, uint16_t y0,
    uint16_t btnW, uint16_t btnH, uint16_t gapY,
    const UiSubmSecondary* btns, uint8_t nBtns
);
