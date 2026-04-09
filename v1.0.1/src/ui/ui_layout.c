/*
 * ui_layout.c
 *
 *  Created on: 12 feb. 2026
 *      Author: oscar
 */

#include <ui_layout.h>


const UiMainLayout UI_LAYOUT_MAIN =
{
	// VARIABLES RELOJ
    .clock_x = 280, .clock_y = 450,
    .user_x  = 390, .user_y  = 450,
    .font    = 23,

	// VARIABLES GRÁFICA
    .bar_x0 = 170, // Posición X
    .bar_y0 = 220, // Posición Y
    .bar_w  = 330, // Ancho total de la gáfica
    .bar_h  = 140, // Altura total de la gráfica

	.bar_w_px     = 20,
	.bar_gap_px   = 4,
	.bar_margin_r = 4,

	// 3 BOTONES SUPERIOR DERECHA - HOME, USUARIO, CONFIGURACIÓN
	.top_btn_y   = 1,     // Posición inicial en Y
	.top_btn_w   = 70,    // Ancho de los botones
	.top_btn_h   = 70,    // Altura de los botones
	.top_btn_gap = 5,     // Separación entre botones
	.top_btn_x2  = 725,   // botón más a la derecha

	// 3 BOTONES APILADOS A LA DERECHA DE LA GRÁFICA
	.side_btn_w   = 150,
	.side_btn_h   = 60,
	.side_btn_gap = 10,
	.side_btn_x   = 540,  // a la derecha de la gráfica
	.side_btn_y0  = 150,

	// FLECHAS DE NAVEGACIÓN IZQUIERDA/DERECHA
	.arw_w       = 80,
	.arw_h       = 70,
	.arw_y       = 230,
	.arw_x_left  = 5,
	.arw_x_right = 715,

    // RECTÁNGULO DE ESTADOS "FOOTER"
	.footer_x0 = 1,
	.footer_y0 = 71,   // para 800x480 típico
	.footer_w  = 799,
	.footer_h  = 40,

	// PAGERS QUE INDICAN PANTALLA MOSTRADA
	.pager_x0 = 15,
	.pager_y  = 465,
	.pager_span = 150,
	.pager_point_size = (uint16_t)(200), // similar a tu Arduino (~10-12px aprox)

    .content_x0 = 100,   // después de flecha izquierda + margen
    .content_x1 = 700,   // antes de flecha derecha - margen
    .content_y0 = 120,   // debajo del topbar/footer superior
    .content_y1 = 450    // arriba de pagers/reloj

};
