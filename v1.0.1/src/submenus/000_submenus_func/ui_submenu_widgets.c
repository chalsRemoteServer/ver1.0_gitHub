/*
 * ui_submenu_widgets.c
 *
 *  Created on: 23 feb. 2026
 *      Author: oscar
 */

#include "ui_submenu_widgets.h"
#include "ui_rects.h"
#include "EVE.h"
#include "colores.h"
#include <stdint.h>

uint8_t ui_subm_grid2x3_layout(const uint16_t cx0, const uint16_t cy0, const uint16_t cx1, const uint16_t cy1, uint8_t nItems, uint16_t cardW, uint16_t cardH,
    uint16_t gapX, uint16_t gapY, uint16_t labelGapY, uint16_t labelH, UiRect16 out_cards[6])
{
    uint8_t drawCount = nItems;
    if (drawCount > 6) drawCount = 6;

    if (drawCount == 0) return 0;

    const uint16_t availW = (uint16_t)(cx1 - cx0);
    const uint16_t availH = (uint16_t)(cy1 - cy0);

    // grid fijo: 3 columnas, 2 filas (pero si drawCount <=3 solo usa 1 fila)
    const uint8_t rows = (drawCount <= 3) ? 1 : 2;
    const uint8_t cols = (drawCount <= 3) ? drawCount : 3;

    // alto total por fila = cardH + labelGapY + labelH
    const uint16_t cellH = (uint16_t)(cardH + labelGapY + labelH);

    const uint16_t blockW = (uint16_t)(cols * cardW + (cols > 1 ? (cols - 1) * gapX : 0));
    const uint16_t blockH = (uint16_t)(rows * cellH + (rows > 1 ? (rows - 1) * gapY : 0));

    const uint16_t sx = (uint16_t)(cx0 + (availW > blockW ? (availW - blockW) / 2 : 0));
    const uint16_t sy = (uint16_t)(cy0 + (availH > blockH ? (availH - blockH) / 2 : 0));

    // rellenar out_cards
    for (uint8_t i = 0; i < drawCount; ++i)
    {
        const uint8_t r = (uint8_t)(i / 3);
        const uint8_t c = (uint8_t)(i % 3);

        // si solo hay 1 fila, c va 0..(cols-1) y r=0
        // si hay 2 filas, c=0..2, r=0..1
        if (rows == 1 && c >= cols) break;

        const uint16_t x = (uint16_t)(sx + c * (cardW + gapX));
        const uint16_t y = (uint16_t)(sy + r * (cellH + gapY));

        out_cards[i] = (UiRect16){ x, y, cardW, cardH };
    }

    return drawCount;
}

void ui_subm_draw_primary_grid2x3(const char* title, uint16_t title_x, uint16_t title_y, uint16_t title_font, const UiSubmPrimary* items, uint8_t nItems,
                                 const UiRect16 cards[6], uint8_t drawCount, uint16_t button_font, uint16_t label_font, uint16_t labelGapY)
{
    // Título
    EVE_color_a(255);
    EVE_color_rgb(BLACK);
    if (title && title[0] != '\0')
        EVE_cmd_text_bold(title_x, title_y, title_font, 0, title);

    // Botones + labels
    for (uint8_t i = 0; i < drawCount; ++i)
    {
        if (i >= nItems) break;

        const uint16_t x = cards[i].x;
        const uint16_t y = cards[i].y;
        const uint16_t w = cards[i].w;
        const uint16_t h = cards[i].h;

        // TAG + rect registry para overlay verde
        EVE_tag(items[i].id);
        ui_rect_register(items[i].id, x, y, w, h);

        // botón vacío (tu estilo)
        EVE_cmd_fgcolor(0x9B6200);
        EVE_color_rgb(WHITE);
        EVE_cmd_button(x, y, w, h, button_font, 256, "");

        // ---- ICONO centrado (ARGB1555, 64x64) ----
        if (items[i].icon_mem != 0u)
        {
            const uint16_t iw = 64u;
            const uint16_t ih = 64u;
            const uint16_t ix = (uint16_t)(x + ((w - iw) / 2u));
            const uint16_t iy = (uint16_t)(y + ((h - ih) / 2u));

            // Importante: deja el mismo TAG para que el icono pertenezca al botón.
            // Formato: ARGB1555
            EVE_begin(EVE_BITMAPS);
            EVE_cmd_setbitmap(items[i].icon_mem, EVE_ARGB1555, iw, ih);
            EVE_vertex2f(ix * PRESICION, iy * PRESICION);
            EVE_end();
        }

        // label debajo
        if (items[i].label && items[i].label[0] != '\0')
        {
            const uint16_t lx = (uint16_t)(x + w / 2);
            const uint16_t ly = (uint16_t)(y + h + labelGapY);
            EVE_color_rgb(BLACK);
            EVE_cmd_text(lx, ly, label_font, EVE_OPT_CENTERX, items[i].label);
        }
    }

    EVE_tag(0);
}


void ui_subm_draw_side_buttons(uint16_t xRight0, uint16_t y0, uint16_t btnW, uint16_t btnH, uint16_t gapY, const UiSubmSecondary* btns, uint8_t nBtns)
{
    if (nBtns > 4) nBtns = 4;

    uint16_t y = y0;
    for (uint8_t i = 0; i < nBtns; ++i)
    {
        const uint16_t id = btns[i].id;

        EVE_tag(id);
        ui_rect_register(id, xRight0, y, btnW, btnH);
        EVE_cmd_fgcolor(0x9B6200);
        EVE_color_rgb(WHITE);
        EVE_cmd_button(xRight0, y, btnW, btnH, 24, 256, (btns[i].text ? btns[i].text : ""));

        y = (uint16_t)(y + btnH + gapY);
    }

    EVE_tag(0);
}

// A PARTIR DE AQUÍ

static void calc_row(uint16_t gx0, uint16_t gridW, uint16_t btn, uint16_t minGapX, uint8_t nInRow, uint16_t* startX, uint16_t* gapX)
{
    if (nInRow <= 1)
    {
        *gapX = 0;
        *startX = (uint16_t)(gx0 + (gridW > btn ? (gridW - btn)/2 : 0));
        return;
    }

    int32_t rem = (int32_t)gridW - (int32_t)nInRow * (int32_t)btn;
    int32_t g   = rem / (int32_t)(nInRow + 1);
    if (g < (int32_t)minGapX) g = (int32_t)minGapX;

    *gapX = (uint16_t)g;
    *startX = (uint16_t)(gx0 + *gapX);
}

uint8_t ui_subm_grid2x3_layout_dynamic(uint16_t gx0, uint16_t gy0, uint16_t gx1, uint16_t gy1, uint8_t nItems,  uint16_t btn, uint16_t minGapX,
                                       uint16_t gapY, uint16_t labelGapY, uint16_t labelH, UiRect16 out_cards[6])
{
    uint8_t drawCount = nItems;
    if (drawCount > 6u) drawCount = 6u;
    if (drawCount == 0u) return 0u;

    // protección básica
    if (gx1 <= gx0 || gy1 <= gy0) return 0u;

    const uint16_t gridW = (uint16_t)(gx1 - gx0);
    const uint16_t gridH = (uint16_t)(gy1 - gy0);

    const uint8_t nRow0 = (drawCount <= 3u) ? drawCount : 3u;
    const uint8_t nRow1 = (drawCount >  3u) ? (uint8_t)(drawCount - 3u) : 0u;
    const uint8_t rows  = (nRow1 > 0u) ? 2u : 1u;

    const uint16_t cellH  = (uint16_t)(btn + labelGapY + labelH);
    const uint16_t blockH = (uint16_t)(rows * cellH + ((rows > 1u) ? gapY : 0u));

    const uint16_t startY = (uint16_t)(gy0 + ((gridH > blockH) ? ((gridH - blockH) / 2u) : 0u));

    uint16_t x0, gapX;

    // ---- Fila 0 ----
    calc_row(gx0, gridW, btn, minGapX, nRow0, &x0, &gapX);

    for (uint8_t c = 0u; c < nRow0; ++c)
    {
        out_cards[c].x = (uint16_t)(x0 + c * (uint16_t)(btn + gapX));
        out_cards[c].y = startY;
        out_cards[c].w = btn;
        out_cards[c].h = btn;
    }

    // ---- Fila 1 (si aplica) ----
    if (nRow1 > 0u)
    {
        const uint16_t y1 = (uint16_t)(startY + cellH + gapY);

        // Si en la fila 1 solo hay 1 botón (caso de 4 items totales),
        // NO lo centramos: lo colocamos en (fila2, col1).
        if (nRow1 == 1u)
        {
            uint16_t x3, gap3;
            // Calcula startX/gapX como si esta fila tuviera 3 columnas
            calc_row(gx0, gridW, btn, minGapX, 3u, &x3, &gap3);

            // idx = 3 (primer elemento de la segunda fila)
            out_cards[3].x = x3;     // columna 1
            out_cards[3].y = y1;
            out_cards[3].w = btn;
            out_cards[3].h = btn;
        }
        else
        {
            // Caso normal: 2 o 3 botones en la fila 1 -> distribuir/centrar
            calc_row(gx0, gridW, btn, minGapX, nRow1, &x0, &gapX);

            for (uint8_t c = 0u; c < nRow1; ++c)
            {
                const uint8_t idx = (uint8_t)(3u + c);
                out_cards[idx].x = (uint16_t)(x0 + c * (uint16_t)(btn + gapX));
                out_cards[idx].y = y1;
                out_cards[idx].w = btn;
                out_cards[idx].h = btn;
            }
        }
    }


    return drawCount;
}

