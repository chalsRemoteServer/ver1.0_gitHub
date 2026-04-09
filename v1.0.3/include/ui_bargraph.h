/*
 * ui_bargraph.h
 *
 *  Created on: 12 feb. 2026
 *      Author: oscar
 */

#pragma once
#include <stdint.h>
#include "ui_layout.h"

#ifndef BAR_HISTORY_MAX
#define BAR_HISTORY_MAX 16
#endif

typedef struct
{
    uint16_t history[BAR_HISTORY_MAX];
    uint16_t count;

    uint16_t threshold;     // 0..300
    uint8_t  tol_pct;       // ej. 10 (%)

    uint16_t last_value;    // <-- NUEVO: último valor mostrado (para texto "Signal")

    // Para demo/updates temporizados
    uint32_t next_ms;
    uint32_t period_ms;
} UiBarGraph;

typedef enum
{
    UI_BARGRAPH_MODE_BARS = 0,
    UI_BARGRAPH_MODE_LINE = 1
} UiBarGraphMode;

void ui_bargraph_init(UiBarGraph *g, uint16_t threshold, uint8_t tol_pct);
void ui_bargraph_set_threshold(UiBarGraph *g, uint16_t threshold);
void ui_bargraph_push(UiBarGraph *g, uint16_t sample_0_300);

// demo opcional
void ui_bargraph_demo_update(UiBarGraph *g);
void ui_bargraph_draw(const UiBarGraph *g, const UiMainLayout *L);
void ui_bargraph_draw_mode(const UiBarGraph *g, const UiMainLayout *L, UiBarGraphMode mode);
