/*
 * view_cfg_sistema.c
 *
 *  Created on: 25 feb. 2026
 *      Author: oscar
 */

#include "ui_submenu_widgets.h"
#include "ui_layout.h"
#include "ui_view_ids.h"
#include "ui_view_mgr.h"
#include "colores.h"
#include "EVE.h"
#include "Posicion_Objetos.h"
#include "assets_mem.h"
#include "ui_views.h"

static void opciones_sistema_draw(void)
{
    const UiMainLayout* L = &UI_LAYOUT_MAIN;

    // 1) Items primarios (6)
    const UiSubmPrimary items[] =
    {
        { UI_VID_OPT_SIS_ADMUSERS, 			"Adm. usuarios"     	, MEM_ICON_ADMUSERS     },
        { UI_VID_OPT_SIS_SHARDW,  			"Superv. hardware"      , MEM_ICON_SHARDWARE    },
        { UI_VID_OPT_SIS_ESTADIST,    		"Estadisticas" 			, MEM_ICON_ESTADISTICAS },
        { UI_VID_OPT_SIS_AUDITORIA,   		"Auditoria"             , MEM_ICON_AUDITORIA    },
        { UI_VID_OPT_SIS_CONFFABRICA,       "Conf. fabrica"         , MEM_ICON_CONFABRICA   },
		{ UI_VID_OPT_SIS_LICENCIA, 		    "Licencia"              , MEM_ICON_LICENCIA   },
    };

    const uint8_t nItems = (uint8_t)(sizeof(items)/sizeof(items[0]));

    // 2) Título + área de grid (misma estrategia que registro_opts)
    const uint16_t title_x   = 20;
    const uint16_t title_y   = (uint16_t)(L->content_y0 + 10);
    const uint16_t title_h   = 34;
    const uint16_t title_gap = 20;

    const uint16_t BTN = 94;

    const uint16_t grid_x0 = title_x;
    const uint16_t grid_x1 = (uint16_t)(L->content_x1 + 50);

    const uint16_t grid_y0 = (uint16_t)(title_y + title_h + title_gap);
    const uint16_t grid_y1 = (uint16_t)(L->content_y1);

    UiRect16 cards[6];

    const uint8_t drawCount = ui_subm_grid2x3_layout_dynamic(grid_x0, grid_y0, grid_x1, grid_y1, nItems, BTN,
        10,   // minGapX
        16,   // gapY
        8,    // labelGapY
        24,   // labelH
        cards
    );

    ui_subm_draw_primary_grid2x3("Sistema", title_x, title_y, PRODUCT_FONT_SIZE, items, nItems, cards, drawCount,
        26,   // button_font
        28,   // label_font
        8     // corner radius (si tu widget lo usa)
    );

    // 3) Botón secundario (barra derecha): X
    const UiSubmSecondary side[] =
    {
        { UI_VID_OPT_PROD_BACK, "X" },
    };

    const uint16_t xR0 = (uint16_t)(L->content_x1);
    const uint16_t yR0 = (uint16_t)(L->content_y0);

    ui_subm_draw_side_buttons(xR0, yR0, BTN, BTN, 10, side, 1);
}


static void opciones_sistema_on_released(uint16_t id)
{
    switch (id)
    {
        case UI_VID_OPT_PROD_BACK:
             ui_view_pop();  // regresa a la vista anterior (normalmente main)
             break;

        case UI_VID_OPT_SIS_ADMUSERS:
        	 ui_view_push(&VIEW_OPCIONES_SISTEMA_ADMUSERS);
        	 break;

        case UI_VID_OPT_SIS_SHARDW:
        	 ui_view_push(&VIEW_OPCIONES_SISTEMA_SHARDWARE);
        	 break;

        case UI_VID_OPT_SIS_ESTADIST:
        	 ui_view_push(&VIEW_OPCIONES_SISTEMA_ESTADISTICAS);
        	 break;

        case UI_VID_OPT_SIS_INTERFACES:
        	 //ui_view_push(&VIEW_OPCIONES_PRODUCTO_AUDITORIA);
        	 break;

        case UI_VID_OPT_SIS_AUDITORIA:
        	 ui_view_push(&VIEW_OPCIONES_SISTEMA_AUDITORIA);
        	 break;

        case UI_VID_OPT_SIS_CONFFABRICA:
       	     ui_view_push(&VIEW_OPCIONES_SISTEMA_CONFFABRICA);
             break;

        case UI_VID_OPT_SIS_LICENCIA:
       	     ui_view_push(&VIEW_OPCIONES_SISTEMA_LICENCIA);
             break;

        default:
            break;
    }
}


const UiView VIEW_OPCIONES_SISTEMA =
{
    .id = 24,
    .on_enter = 0,
    .on_exit = 0,
    .update = 0,
    .draw = opciones_sistema_draw,
    .on_released = opciones_sistema_on_released
};


