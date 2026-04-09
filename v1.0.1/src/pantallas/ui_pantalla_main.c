
#include "ui_pantallas.h"
#include "ui_main_buttons.h"
#include "ui_footer.h"
#include "ui_bargraph.h"
#include "ui_pagers.h"
#include "ui_layout.h"

// usa tus globales existentes o pásalos por getters
extern UiBarGraph g_bar;
//extern UiFooter   g_footer;
//extern UiMainLayout UI_LAYOUT_MAIN;

//extern uint16_t signal_Detection; // como ya lo tienes

static void main_on_enter(void){
    // equivalente a pantalla1_on_enter() de Arduino
    // ejemplo: reset buffers/labels/etc si aplica
}

static void main_update(uint32_t now_ms){
    // OJO: si tu gráfica se alimenta aquí, sólo actualizaría en esta pantalla.
    // Lo resolveremos en el paso 6 (update global).
    (void)now_ms;
}

//static void main_draw(void){
//    // Dibuja layout principal: topbar/sidebar/navbars (si aplica)
//    ui_main_buttons_draw_groups(&UI_LAYOUT_MAIN, UI_GRP_SIDEBAR); // solo en main
//    ui_bargraph_draw(&g_bar, &UI_LAYOUT_MAIN);    // Dibuja gráfica
//    ui_footer_draw(&g_footer, &UI_LAYOUT_MAIN);    // Footer
//    ui_pagers_draw(&UI_LAYOUT_MAIN, ui_screens_get(), UI_TOTAL_SCREENS);    // Pagers: activo = pantalla actual
//}

static void main_draw(void){
    ui_main_buttons_draw_groups(&UI_LAYOUT_MAIN, UI_GRP_SIDEBAR); // PRODUCTO/APRENDIZAJE/REGISTRO
    ui_bargraph_draw(&g_bar, &UI_LAYOUT_MAIN);                    // gráfica visible solo en main
}


const UiScreen g_screen_main = {
    .on_enter = main_on_enter,
    .on_exit  = 0,
    .update   = main_update,
    .draw     = main_draw
};
