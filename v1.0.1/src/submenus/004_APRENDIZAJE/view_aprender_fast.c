/*
 * view_aprender_fast.c
 *
 *  Created on: 23 feb. 2026
 *      Author: oscar
 */


/*
 * view_aprender_fast.c
 * View: "Aprend. rapido"
 */

#include "ui_layout.h"
#include "ui_view_ids.h"
#include "ui_view_mgr.h"
#include "ui_submenu_widgets.h"
#include "EVE.h"
#include "colores.h"
#include "Posicion_Objetos.h"
#include "assets_mem.h"

static void aprender_fast_draw(void)
{
    const UiMainLayout* L = &UI_LAYOUT_MAIN;

    // ---- Título ----
    EVE_color_rgb(WHITE);
    EVE_cmd_text_bold(20, (uint16_t)(L->content_y0 + 10), PRODUCT_FONT_SIZE, EVE_OPT_FLAT, "Aprend. rapido");

    // ---- Texto placeholder ----
    EVE_color_rgb(WHITE);
    EVE_cmd_text(20, (uint16_t)(L->content_y0 + 70), 28, EVE_OPT_FLAT, "En construccion...");

    // ---- Botón X (barra derecha) ----
    const UiSubmSecondary side[] =
    {
        { UI_VID_APRF_BACK, "X" },
    };

    const uint16_t BTN = 94;
    const uint16_t xR0 = (uint16_t)(L->content_x1);
    const uint16_t yR0 = (uint16_t)(L->content_y0);

    ui_subm_draw_side_buttons(xR0, yR0, BTN, BTN, 10, side, 1);
}

static void aprender_fast_on_released(uint16_t id)
{
    switch (id)
    {
        case UI_VID_APRF_BACK:
            ui_view_pop(); // regresa a VIEW_APRENDER
            break;

        default:
            break;
    }
}

const UiView VIEW_APRENDER_FAST =
{
    .id = 4,               // Asegúrate que no choque: 1=REG, 2=ROPT, 3=APRENDER
    .on_enter = 0,
    .on_exit  = 0,
    .update   = 0,
    .draw     = aprender_fast_draw,
    .on_released = aprender_fast_on_released
};
