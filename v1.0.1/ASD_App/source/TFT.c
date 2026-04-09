#include <platform_time.h>
#include <ui_layout.h>
#include <stdio.h>
#include <ui_reloj.h>
#include "system.h"
#include "TFT.h"
#include "eve_app.h"
#include "../eve/include/EVE_commands.h"
#include "tft_data.h"
#include "../ASD_App/include/colores.h"
#include "../ASD_App/include/Posicion_Objetos.h"
#include "icons.h"          // desde assets/include (ya lo tienes)
#include "assets_mem.h"     // el mapa que acabas de crear
#include "ft900.h"
#include "ui_bargraph.h"
#include "ui_main_buttons.h"
#include "ui_footer.h"
#include "ui_pagers.h"
#include "../eve/include/EVE.h"      // o donde tengas REG_TOUCH_TRANSFORM_*
#include "../eve/include/HAL.h"      // si vas a usar HAL_MemWrite32
#include "ui_pantallas.h"
#include "ui_counters.h"
#include "ui_ids.h"   // si no está ya visible por tus includes
#include "ui_submenus.h"
#include "ui_view_mgr.h"
#include "ui_view_ids.h"
#include "ui_views.h"
#include "ui_rects.h"

UiBarGraph g_bar;
UiFooter g_footer;
UiReloj g_reloj;         // si lo necesitas afuera
const char *g_user = "Servicio"; // si lo necesitas afuera

//static UiReloj g_reloj;
//static const char *g_user = "Ningun Usuario";
//static UiBarGraph g_bar;
//static UiFooter g_footer;
//extern uint8_t g_screen_idx;
//#define UI_TOTAL_SCREENS 5// total pantallas (según tu UI)


uint32_t num_dl_static = 0; /* amount of bytes in the static part of our display-list */
uint8_t tft_active = 0;
uint16_t toggle_state = 0;
uint16_t horas,minutos,segundos,mseg,display_list_size = 0;
uint16_t signal_Detection;//señal del detector
#define LAYOUT_Y1 66

typedef struct {
    uint8_t  down;
    uint8_t  tag_now;
    uint8_t  pressed_id;   // id que se resalta
    uint8_t  released_id;  // evento: 1 frame
    uint32_t pressed_until_ms;
} UiTouchState;

static uint32_t now_ms(void)
{
    return platform_millis();  // AJUSTA al nombre real en tu plataforma
}

static UiTouchState g_touch = {0};

#define TOUCH_LINGER_MS 150u



//void touch_calibrate(void);
void initStaticBackground(void);


//void touch_calibrate(void)
//{
// EVE_calibrate_write(0xffff09c0, 0x00000192, 0x03705215, 0x00000000, 0xffff43ae, 0x02700000); // COEFICIENTES PRUEBA 2
 //EVE_calibrate_write(0x0000fd55, 0xfffffc64, 0x0006f496, 0xfffffef1, 0x000106f4, 0xfff794c4);
//}

static void touch_apply_transform(void)
{
    EVE_memWrite32(REG_TOUCH_TRANSFORM_A, 0xffff09c0);    // tus coeficientes
    EVE_memWrite32(REG_TOUCH_TRANSFORM_B, 0x00000192);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_C, 0x03705215);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_D, 0x00000000);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_E, 0xffff43ae);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_F, 0x02700000);

    // habilita touch
    EVE_memWrite8(REG_TOUCH_MODE, 3);       // continuous
    EVE_memWrite16(REG_TOUCH_RZTHRESH, 15000); // umbral razonable (ajustable)
}



static void ui_touch_update(UiTouchState *t)
{
    // Lee regs (fuera de burst)
    uint32_t xy  = EVE_memRead32(REG_TOUCH_SCREEN_XY);
    uint16_t rz  = EVE_memRead16(REG_TOUCH_RZ);
    uint32_t raw = EVE_memRead32(REG_TOUCH_RAW_XY);
    uint8_t  tag = EVE_memRead8(REG_TOUCH_TAG);

    uint8_t touching = (rz != 0x7FFF) && (xy != 0x80008000) && (raw != 0xFFFFFFFF);

    t->released_id = 0;

    uint32_t tnow = now_ms();

    if (touching)
    {
        t->down = 1;
        t->tag_now = tag;

        if (tag != 0)
        {
            // “hold”: mientras mantienes, pressed_id = tag actual
            t->pressed_id = tag;
            // renueva el linger continuamente para que se mantenga si se mueve un poco
            t->pressed_until_ms = tnow + TOUCH_LINGER_MS;
        }
    }
    else
    {
        if (t->down)
        {
            // transición DOWN -> UP: evento click
            t->released_id = t->pressed_id;
            // “linger”: conserva el pressed un ratito después de soltar
            t->pressed_until_ms = tnow + TOUCH_LINGER_MS;
        }
        t->down = 0;
        t->tag_now = 0;        // no borres pressed_id aquí: lo borra el timeout
    }

    // Si ya pasó el linger, borra highlight
    if (t->pressed_id != 0 && (int32_t)(tnow - t->pressed_until_ms) > 0)
    {
        t->pressed_id = 0;
    }
}

static void ui_draw_pressed_overlay(uint8_t id)
{
    uint16_t x,y,w,h;
    if (!ui_get_rect_by_id(id, &x, &y, &w, &h)) return;

    EVE_tag_mask(0);
    EVE_tag(0);
    // Overlay (elige estilo similar al Arduino: borde o fill con alpha)
    EVE_blend_func(EVE_SRC_ALPHA, EVE_ONE_MINUS_SRC_ALPHA);
    EVE_color_a(80);                 // transparencia
    EVE_color_rgb(WHITE);      // o usa un color tuyo (por ejemplo DARK_ORANGE con alpha)

    EVE_begin(EVE_RECTS);
    EVE_vertex2f(x * PRESICION, y * PRESICION);
    EVE_vertex2f((x+w) * PRESICION, (y+h) * PRESICION);
    EVE_end();

    EVE_color_a(255);    // Regresa alpha
}


//void vStart_Pantalla_TFT(void)
//{
//	eve_setup();
//
//}//FIN DE iniciar pantalla TFT-------------

void TFT_init(void)
{
     if(E_OK == EVE_init())
     {
        tft_active = 1;horas=12,minutos=0;segundos=0;mseg=0;
        EVE_memWrite32(REG_PWM_DUTY, 0x30);  /* setup backlight, range is from 0 = off to 0x80 = max */
        EVE_memWrite8(REG_TOUCH_MODE, 3);
        //touch_calibrate();
        touch_apply_transform(); // <-- aquí, al final
        EVE_cmd_inflate(MEM_LOGO, Logo160x41, Logo160x41_len);

        EVE_cmd_inflate(MEM_ICON_HOME,                  	Home_001,                       	Home_001_len);
        EVE_cmd_inflate(MEM_ICON_USUARIO,               	Usuario_002,                    	Usuario_002_len);
        EVE_cmd_inflate(MEM_ICON_CONFIGURACION,         	Configuracion_003,              	Configuracion_003_len);

        EVE_cmd_inflate(MEM_ICON_AJREGISTRO,                AjRegistro_001,                     AjRegistro_001_len);
        EVE_cmd_inflate(MEM_ICON_FREGISTRO,                 FiltRegistro_002,                   FiltRegistro_002_len);
        EVE_cmd_inflate(MEM_ICON_BREGISTRO,         		BorrarReg_003_V2,                   BorrarReg_003_V2_len);
        EVE_cmd_inflate(MEM_ICON_EXPREGISTRO,         		ExpReg_004_V2,                    	ExpReg_004_V2_len);

        EVE_cmd_inflate(MEM_ICON_PRODUCTO,              	Producto_001_V2,                	Producto_001_V2_len);
        EVE_cmd_inflate(MEM_ICON_SISTEMA,               	Sistema_002_V2,                 	Sistema_002_V2_len);
        EVE_cmd_inflate(MEM_ICON_PERIFERICOS,           	Perifericos_003,                	Perifericos_003_len);

        EVE_cmd_inflate(MEM_ICON_ENTRADAS,              	Entradas_004,                		Entradas_004_len);
        EVE_cmd_inflate(MEM_ICON_BOBINA,                	Bobina_005_V2,                  	Bobina_005_V2_len);
        EVE_cmd_inflate(MEM_ICON_USB,           			USB_006,                			USB_006_len);

        EVE_cmd_inflate(MEM_ICON_ADMPROD,              		AdmProd_001_V2,                 	AdmProd_001_V2_len);
        EVE_cmd_inflate(MEM_ICON_DATPROD,               	DatosProd_002_V2,                	DatosProd_002_V2_len);
        EVE_cmd_inflate(MEM_ICON_APRENDER,              	Aprender_003_V2,                 	Aprender_003_V2_len);
        EVE_cmd_inflate(MEM_ICON_AUDITORIA,             	Auditcheck_004,                  	Auditcheck_004_len);
        EVE_cmd_inflate(MEM_ICON_SALIDAS,             		Salidas_005_V2,                  	Salidas_005_V2_len);
        EVE_cmd_inflate(MEM_ICON_AJUSTES,               	Ajustes_001_V2,                  	Ajustes_001_V2_len);

        EVE_cmd_inflate(MEM_ICON_CAMBPRODUCTO,              CambProducto_001_V2,               	CambProducto_001_V2_len);
        EVE_cmd_inflate(MEM_ICON_CREAPRODUCTO,              CreaProducto_002_V2,               	CreaProducto_002_V2_len);
        EVE_cmd_inflate(MEM_ICON_ELIMPRODUCTO,              ElimProducto_003_V2,               	ElimProducto_003_V2_len);

        EVE_cmd_inflate(MEM_ICON_PROBETAS,              	CuerpTest_002_V2,               	CuerpTest_002_V2_len);
        EVE_cmd_inflate(MEM_ICON_TODOSPROD,              	TodosProd_003_V2,               	TodosProd_003_V2_len);
        EVE_cmd_inflate(MEM_ICON_INIMANUAL,              	IniManual_004_V2,               	IniManual_004_V2_len);

        EVE_cmd_inflate(MEM_ICON_MOMR,                      MOMR_003_V2,                       	MOMR_003_V2_len);

        EVE_cmd_inflate(MEM_ICON_ADMUSERS,              	AdmUsers_001_V2,               	 	AdmUsers_001_V2_len);
        EVE_cmd_inflate(MEM_ICON_SHARDWARE,             	SuperHardware_002_V2,           	SuperHardware_002_V2_len);
        EVE_cmd_inflate(MEM_ICON_ESTADISTICAS,          	Estadisticas_003_V2,            	Estadisticas_003_V2_len);
        EVE_cmd_inflate(MEM_ICON_CONFABRICA,             	ConfFabrica_007,               		ConfFabrica_007_len);
        EVE_cmd_inflate(MEM_ICON_LICENCIA,              	Licencia_008_V2,                	Licencia_008_V2_len);

        EVE_cmd_inflate(MEM_ICON_EDITAR,                    Editar_001_V2,                      Editar_001_V2_len);
        EVE_cmd_inflate(MEM_ICON_CREAR,                     Crear_002_V2,                       Crear_002_V2_len);
        EVE_cmd_inflate(MEM_ICON_ELIMINAR,                  Eliminar_003_V2,                    Eliminar_003_V2_len);
        EVE_cmd_inflate(MEM_ICON_COPIAR,                    Copiar_004_V2,                      Copiar_004_V2_len);
        EVE_cmd_inflate(MEM_ICON_LOGIN2GO,                  Login2Go_006_V2,                    Login2Go_006_V2_len);

        EVE_cmd_inflate(MEM_ICON_ENTRADASYS,                EntradasS_001_V2,                   EntradasS_001_V2_len);
        EVE_cmd_inflate(MEM_ICON_UC,                    	UnidadControl_002,                  UnidadControl_002_len);
        EVE_cmd_inflate(MEM_ICON_VERSIONES,                 Versiones_004_V2,                   Versiones_004_V2_len);
        EVE_cmd_inflate(MEM_ICON_DEBUG,                 	Debug_005,                    		Debug_005_len);

        EVE_cmd_inflate(MEM_ICON_CONTADORES,            	Contadores_001,                 	Contadores_001_len);
        EVE_cmd_inflate(MEM_ICON_METALSIGNAL,           	MetalSignal_002,                	MetalSignal_002_len);
        EVE_cmd_inflate(MEM_ICON_DISPLAYS,              	Displays_003,                   	Displays_003_len);

        EVE_cmd_inflate(MEM_ICON_GUARDCONFIG,           	GuardarConfig_001_V2,           	GuardarConfig_001_V2_len);
        EVE_cmd_inflate(MEM_ICON_RESTCONFIG,            	RestConfig_002,                 	RestConfig_002_len);

        EVE_cmd_inflate(MEM_ICON_CARACTERISTICAS,       	Caracteristicas_001_V2,         	Caracteristicas_001_V2_len);
        EVE_cmd_inflate(MEM_ICON_IMPORTAR,              	Importar_002_V2,                	Importar_002_V2_len);
        EVE_cmd_inflate(MEM_ICON_EXPORTAR,              	Exportar_003_V2,                	Exportar_003_V2_len);

        EVE_cmd_inflate(MEM_ICON_SUPERVISION,           	Supervision_002,                	Supervision_002_len);
        EVE_cmd_inflate(MEM_ICON_ADVERTENCIA,           	Advertencia_003,                	Advertencia_003_len);
        EVE_cmd_inflate(MEM_ICON_PRUEBAAUTO,            	AutomatTest_004,                	AutomatTest_004_len);
        EVE_cmd_inflate(MEM_ICON_OPCEXP,            		Expulsion_001_V2,               	Expulsion_001_V2_len);

        EVE_cmd_inflate(MEM_ICON_RECHAZO,               	Rechazo_001,                   	 	Rechazo_001_len);
        EVE_cmd_inflate(MEM_ICON_SENSORAIRE,            	SensorAire_002,                 	SensorAire_002_len);
        EVE_cmd_inflate(MEM_ICON_CONTENEDOR,            	Contenedor_003,                 	Contenedor_003_len);

        EVE_cmd_inflate(MEM_ICON_EXTERNO,               	Externo_001,                    	Externo_001_len);
        EVE_cmd_inflate(MEM_ICON_DIGITAL,               	Digital_002_V2,                 	Digital_002_V2_len);
        EVE_cmd_inflate(MEM_ICON_CONFIGURABLE,          	Configurable_003,               	Configurable_003_len);

        EVE_cmd_inflate(MEM_ICON_DESVFREC,              	DesvFrec_002_V2,                	DesvFrec_002_V2_len);

        EVE_cmd_inflate(MEM_ICON_IMPORTAR,              	Importar_002_V2,                	Importar_002_V2_len);
        EVE_cmd_inflate(MEM_ICON_EXPORTAR,              	Exportar_003_V2,                	Exportar_003_V2_len);

        EVE_memWrite8(REG_DITHER, 0x00);
        EVE_execute_cmd();

        initStaticBackground();
        platform_time_init();
        ui_reloj_init(&g_reloj, 0, 0, 0);
        ui_bargraph_init(&g_bar, /*threshold*/100, /*tol_pct*/10);
        ui_footer_init(&g_footer, FOOTER_ESTADO_CONEXION_CU, 3000);
        ui_screens_init();
        ui_counters_init();
        ui_view_init();

    }//------------------------------------------------------------------
}//-----------------------------------------------------------------------------------------------------


void initStaticBackground(void)
{
     EVE_cmd_dlstart(); /* Start the display list */
     EVE_tag(0); /* tag = 0 - do not use the following objects for touch-detection */
     EVE_cmd_bgcolor(WHITE); /* light grey */
     EVE_vertex_format(_FRAC_PRESICION); /* set to 0 - reduce precision for VERTEX2F to 1 pixel instead of 1/16 pixel default */

     /* display the logo */
     EVE_color_mask(1, 1, 1, 1);
     EVE_color_a(128);
     EVE_color_rgb(WHITE);
//
     EVE_begin(EVE_BITMAPS);  // DESCOMENTAR PARA VOLVER A LOGO "ORIGINAL"
     EVE_cmd_setbitmap(MEM_LOGO, EVE_RGB565, 160U, 41U);
     EVE_vertex2f(X_LOGO*PRESICION,Y_LOGO*PRESICION); //EVE_HSIZE - 78, 5);
     EVE_end();

     EVE_color_a(255);
     EVE_color_mask(1,1,1,1);
     EVE_scissor_xy(0,0);
     EVE_scissor_size(EVE_HSIZE, EVE_VSIZE);

     EVE_color_rgb(BLACK);
     EVE_cmd_text(338,18,24,EVE_OPT_CENTERX,VERSION);//"v0.5.10");


     EVE_execute_cmd();
     num_dl_static = EVE_memRead16(REG_CMD_DL);
     EVE_cmd_memcpy(MEM_DL_STATIC, EVE_RAM_DL, num_dl_static);
     EVE_execute_cmd();
}//--------------------------------------------------------------

/*we are running on 8-bit without DMA,
 optimize some more by using the special EVE_cmd_xxx_burst() functions*/

void TFT_display(void){
    if (tft_active == 0U) return;
    ui_touch_update(&g_touch);    // 0) Touch (sin debug printing por frame)
    // 1) ACCIONES POR TOUCH (solo en released)
    switch (g_touch.released_id){
        case UI_ID_TOP_BTN0: // BOTÓN HOME
             ui_view_clear();
             ui_screens_set(0);
             break;
        case UI_ID_ARROW_RIGHT:  // FLECHA DERECHA - NAVEGA PANTALLAS
             if (!ui_view_is_active()) ui_screens_next();
             break;

        case UI_ID_ARROW_LEFT:   // FLECHA IZQUIERDA - NAVEGA PANTALLAS
             if (!ui_view_is_active()) ui_screens_prev();
             break;

        case UI_ID_TOP_BTN1: // BOTÓN USUARIO (TOP BAR)
             ui_submenu_set(UI_SUB_USUARIO);
             break;

        case UI_ID_TOP_BTN2: // BOTÓN CONFIGURACIÓN (TOP BAR)
             ui_view_push(&VIEW_OPCIONES);
             break;

        case UI_ID_SIDE_BTN0: // BOTON 'LATERAL', PRODUCTO
             ui_submenu_set(UI_SUB_PRODUCTO);
             break;

        case UI_ID_SIDE_BTN1: // BOTÓN 'LATERAL', APRENDIZAJE
             ui_view_push(&VIEW_APRENDER);
             break;

        case UI_ID_SIDE_BTN2: // BOTÓN 'LATERAL', REGISTRO/LOGBOOK
             ui_view_push(&VIEW_REGISTRO);
             break;

        default:
        	 if (ui_view_is_active() && (g_touch.released_id != 0))
        	    {
        	     ui_view_handle_released(g_touch.released_id);
        	    }
             break;
    }

    uint32_t now = now_ms();    // 2) Tiempo actual (si lo ocupas)
    // 3) UPDATES GLOBALES (SIEMPRE, aunque cambies pantalla)
    ui_reloj_update(&g_reloj);
    ui_footer_update(&g_footer);    //    (b) footer dinámico
    ui_bargraph_demo_update(&g_bar);  // TEMP: reemplazar por update real
    ui_screens_update(now);
    ui_view_update(now);

    //ui_bargraph_update(&g_bar, signal_Detection);

    // 4) Construcción DL
    //EVE_start_cmd_burst();
    //EVE_cmd_dlstart_burst();

    //EVE_scissor_xy_burst(0, 0);
    //EVE_scissor_size_burst(EVE_HSIZE, EVE_VSIZE);

//    EVE_color_mask_burst(1,1,1,1);
//    EVE_color_a(255);
//    EVE_blend_func_burst(EVE_SRC_ALPHA, EVE_ONE_MINUS_SRC_ALPHA);
//    EVE_stencil_op_burst(EVE_KEEP, EVE_KEEP);
//    EVE_stencil_func_burst(EVE_ALWAYS, 0, 255);
//    EVE_alpha_func_burst(EVE_ALWAYS, 0);
//
//    EVE_clear_color_rgb_burst(WHITE);
//    EVE_clear_burst(1, 1, 1);
//    EVE_tag_burst(0);
//
//    EVE_cmd_append_burst(MEM_DL_STATIC, num_dl_static);    // 5) Fondo estático
    //EVE_start_cmd();
    EVE_cmd_dlstart();

    EVE_scissor_xy(0, 0);
    EVE_scissor_size(EVE_HSIZE, EVE_VSIZE);

    EVE_color_mask(1,1,1,1);
    EVE_color_a(255);
    EVE_blend_func(EVE_SRC_ALPHA, EVE_ONE_MINUS_SRC_ALPHA);
    EVE_stencil_op(EVE_KEEP, EVE_KEEP);
    EVE_stencil_func(EVE_ALWAYS, 0, 255);
    EVE_alpha_func(EVE_ALWAYS, 0);

    EVE_clear_color_rgb(WHITE);
    EVE_clear(1, 1, 1);
    EVE_tag(0);

    EVE_cmd_append(MEM_DL_STATIC, num_dl_static);
    ui_rects_begin_frame();

    uint8_t grp = UI_GRP_TOPBAR;

    if (!ui_view_is_active()) {
    	grp |= UI_GRP_ARROWS;/* dibuja arrows */
    	}

    //ui_main_buttons_draw_groups(&UI_LAYOUT_MAIN, UI_GRP_TOPBAR | UI_GRP_ARROWS);
    ui_main_buttons_draw_groups(&UI_LAYOUT_MAIN, grp);
    ui_reloj_draw(&g_reloj, &UI_LAYOUT_MAIN, g_user);    // reloj + footer siempre visibles
    ui_footer_draw(&g_footer, &UI_LAYOUT_MAIN);

    // 7) CONTENIDO de pantalla activa (aquí cada pantalla dibuja SOLO el contenido central / submenú)
    //ui_screens_draw();
    if (ui_view_is_active()){
        ui_view_draw();}
    else{ui_screens_draw();}
    if (!ui_view_is_active()) {
    	ui_pagers_draw(&UI_LAYOUT_MAIN, ui_screens_get(), ui_screens_count());}/* dibuja pagers */

    // 9) Overlay pressed
    if (g_touch.pressed_id != 0) {
        ui_draw_pressed_overlay(g_touch.pressed_id);}
    EVE_display();
    EVE_cmd_swap();
    EVE_execute_cmd();
	//printf("Create task\n");



//    EVE_display_burst();
//    EVE_cmd_swap_burst();
//    EVE_end_cmd_burst();
}
