/*
 * ui_bargraph.c
 *
 *  Created on: 12 feb. 2026
 *      Author: oscar
 */


#include "ui_bargraph.h"
#include "platform_time.h"

#include <stdlib.h>     // rand()
#include "EVE.h"
#include "colores.h"    // RED/GREEN/YELLOW + BLACK, etc.
#include "ui_counters.h"

#define GRAPH_MAX_Y 300U
#define COUNTER_SPLIT_TH 100u

static uint16_t map_value_to_y(uint16_t v, uint16_t gy0, uint16_t gy1)
{
    if (v > GRAPH_MAX_Y) v = GRAPH_MAX_Y;
    uint32_t span = (uint32_t)(gy1 - gy0);
    uint32_t y = (uint32_t)gy1 - (span * (uint32_t)v) / GRAPH_MAX_Y;
    return (uint16_t)y;
}

static void draw_axes_and_labels(uint16_t gx0, uint16_t gy0, uint16_t gx1, uint16_t gy1, uint16_t font)
{
    // Ejes (izquierda y abajo)
    EVE_color_rgb(BLACK);
    EVE_begin(EVE_LINES);
    EVE_line_width(6U * PRESICION);

    // Eje Y
    EVE_vertex2f((int16_t)(gx0 * PRESICION), (int16_t)(gy0 * PRESICION));
    EVE_vertex2f((int16_t)(gx0 * PRESICION), (int16_t)(gy1 * PRESICION));

    // Eje X
    EVE_vertex2f((int16_t)(gx0 * PRESICION), (int16_t)(gy1 * PRESICION));
    EVE_vertex2f((int16_t)(gx1 * PRESICION), (int16_t)(gy1 * PRESICION));
    EVE_end();

    // Etiquetas Y: 300/200/100/0
    // (la X de labels la dejamos a la izquierda del eje Y)
    const int16_t label_x = (gx0 > 40) ? (int16_t)(gx0 - 40) : (int16_t)gx0;

    char buf[8];
    EVE_color_rgb(BLACK);

    // 300
    snprintf(buf, sizeof(buf), "300");
    EVE_cmd_text(label_x, (int16_t)(map_value_to_y(300, gy0, gy1) - 10), 24, 0, buf);

    // 200
    snprintf(buf, sizeof(buf), "200");
    EVE_cmd_text(label_x, (int16_t)(map_value_to_y(200, gy0, gy1) - 10), 24, 0, buf);

    // 100
    snprintf(buf, sizeof(buf), "100");
    EVE_cmd_text(label_x, (int16_t)(map_value_to_y(100, gy0, gy1) - 10), 24, 0, buf);

    // 0
    snprintf(buf, sizeof(buf), "0");
    EVE_cmd_text((int16_t)(label_x + 20), (int16_t)(gy1 - 15), 24, 0, buf);

}

static void draw_threshold_line(uint16_t gx0, uint16_t gy0, uint16_t gx1, uint16_t gy1, uint16_t threshold)
{
    uint16_t yT = map_value_to_y(threshold, gy0, gy1);

    EVE_color_rgb(BLACK);
    EVE_begin(EVE_LINES);
    EVE_line_width(8U * PRESICION);

    EVE_vertex2f((int16_t)(gx0 * PRESICION), (int16_t)(yT * PRESICION));
    EVE_vertex2f((int16_t)((gx1 + 1) * PRESICION), (int16_t)(yT * PRESICION));
    EVE_end();
}

// --- helpers internos ---
static uint16_t clamp_u16(uint16_t v, uint16_t lo, uint16_t hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void ui_bargraph_init(UiBarGraph *g, uint16_t threshold, uint8_t tol_pct)
{
    if (!g) return;

    for (uint16_t i = 0; i < BAR_HISTORY_MAX; ++i) g->history[i] = 0;
    g->count     = 0;
    g->threshold = clamp_u16(threshold, 0, 300);
    g->tol_pct   = tol_pct;

    g->period_ms = 100; // 3000 igual que Arduino
    g->next_ms   = platform_millis() + g->period_ms;
}

void ui_bargraph_set_threshold(UiBarGraph *g, uint16_t threshold)
{
    if (!g) return;
    g->threshold = clamp_u16(threshold, 0, 300);
}

void ui_bargraph_push(UiBarGraph *g, uint16_t sample_0_300)
{
    if (!g) return;

    uint16_t v = clamp_u16(sample_0_300, 0, 300);

    if (g->count < BAR_HISTORY_MAX) g->count++;

    // shift left
    for (uint16_t i = 0; i + 1 < g->count; ++i) {
        g->history[i] = g->history[i + 1];
    }
    g->history[g->count - 1] = v;
    g->last_value = v;

    // --- contadores (1 incremento por cada muestra nueva) ---
    if (v < COUNTER_SPLIT_TH) {
        ui_counters_increment_error();
    } else {
        ui_counters_increment_metal();
    }

}

void ui_bargraph_update(UiBarGraph *g, uint16_t sample)
{
    if (!g) return;
    uint32_t now = platform_millis();
    if ((int32_t)(now - g->next_ms) >= 0) {
        g->next_ms = now + g->period_ms;
        ui_bargraph_push(g, sample);
    }
}


// Demo: cada period_ms mete una muestra random 0..300
void ui_bargraph_demo_update(UiBarGraph *g)
{
    if (!g) return;

    uint32_t now = platform_millis();
    if ((int32_t)(now - g->next_ms) >= 0) {
        g->next_ms = now + g->period_ms;
        uint16_t v = (uint16_t)(rand() % 301);
        ui_bargraph_push(g, v);
    }
}

void ui_bargraph_draw(const UiBarGraph *g, const UiMainLayout *L)
{
    if (!g || !L) return;
    //if (g->count == 0) return;

    // Rect del plot
    uint16_t gx0 = L->bar_x0;
    uint16_t gy0 = L->bar_y0;
    uint16_t gx1 = (uint16_t)(gx0 + L->bar_w);
    uint16_t gy1 = (uint16_t)(gy0 + L->bar_h);

    if (gx1 <= gx0 || gy1 <= gy0) return;

    // (Opcional) dibuja ejes/grid/umbral si ya tienes funciones equivalentes:
    draw_axes_and_labels(gx0, gy0, gx1, gy1, L->font);
    draw_threshold_line(gx0, gy0, gx1, gy1, g->threshold);

    // 2) Texto inferior: Signal y Umbral. Colócalo debajo del eje X (ajusta el +10/+30 si tu layout no deja espacio)
    EVE_color_rgb(BLACK);

    char bufSignal[32];
    snprintf(bufSignal, sizeof(bufSignal), "Signal: %u", (unsigned)g->last_value);

    char bufUmbral[32];
    snprintf(bufUmbral, sizeof(bufUmbral), "Umbral: %u", (unsigned)g->threshold);

    // Posiciones (debajo de la gráfica)
    const int16_t textY = (int16_t)(gy1 + 12);
    //EVE_cmd_text((int16_t)(gx0 + 10), textY, (int16_t)L->font, 0, bufSignal);
    //EVE_cmd_text((int16_t)(gx0 + 160), textY, (int16_t)L->font, 0, bufUmbral);

    EVE_cmd_text((int16_t)(gx0 + 10), textY, 24, 0, bufSignal);
    EVE_cmd_text((int16_t)(gx0 + 160), textY, 24, 0, bufUmbral);

    EVE_cmd_text_bold((int16_t)(gx0 + 10), 160, 24, 0, "P1: QUESO");


    // tolerancia alrededor del umbral
    uint16_t th = g->threshold;
    uint16_t tol = (uint16_t)(((uint32_t)th * (uint32_t)g->tol_pct) / 100U);
    uint16_t thLow  = (th > tol) ? (uint16_t)(th - tol) : 0;
    uint16_t thHigh = (uint16_t)(th + tol);
    if (thHigh > 300) thHigh = 300;

    uint16_t barW   = L->bar_w_px;
    uint16_t gap    = L->bar_gap_px;
    uint16_t perBar = (uint16_t)(barW + gap);
    if (perBar == 0) return;

    uint16_t w = (uint16_t)(gx1 - gx0);
    uint16_t maxBarsByWidth = (uint16_t)(w / perBar);
    if (maxBarsByWidth == 0) maxBarsByWidth = 1;

    uint16_t maxBars = g->count;
    if (maxBars > maxBarsByWidth) maxBars = maxBarsByWidth;

    // barras de derecha -> izquierda (más reciente a la derecha)
    EVE_begin(EVE_RECTS);

    for (uint16_t i = 0; i < maxBars; ++i)
    {
        uint16_t histIdx = (uint16_t)(g->count - 1 - i);
        uint16_t v = g->history[histIdx]; // 0..300

        // map 0..300 a altura (gy0..gy1)
        uint16_t yTop = (uint16_t)(gy1 - ((uint32_t)(gy1 - gy0) * v) / 300U);

        uint16_t x1 = (uint16_t)(gx1 - L->bar_margin_r - i * perBar);
        uint16_t x0 = (uint16_t)(x1 - barW);
        if (x0 < gx0) x0 = gx0;

        // color por banda
        if (v < thLow) {
            EVE_color_rgb(GREEN);
        } else if (v > thHigh) {
            EVE_color_rgb(RED);
        } else {
            EVE_color_rgb(YELLOW);
        }

        // Rect
        EVE_vertex2f((int16_t)(x0 * PRESICION), (int16_t)(yTop * PRESICION));
        EVE_vertex2f((int16_t)(x1 * PRESICION), (int16_t)(gy1 * PRESICION));
    }

    EVE_end();
}
