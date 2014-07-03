#include <pebble.h>
#include "fontxLayer.h"
#include "fontx.h"

;
// updateイベント
static void layer_update_proc(FontxLayer *fontx_layer, GContext *ctx)
{
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "-- layer_update_proc()");

    FontxLayerData *fontx_data = (FontxLayerData *)layer_get_data(fontx_layer);
    GRect bounds = layer_get_bounds(fontx_layer);
    
    graphics_context_set_fill_color(ctx, fontx_data->bg_color);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
    graphics_context_set_stroke_color(ctx, fontx_data->fg_color);
    
    
   // APP_LOG(APP_LOG_LEVEL_DEBUG, fontx_layer->text);

    fontx_draw(ctx, bounds, fontx_data->text );
    
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "-- layer_update_proc End");
}

//====================================================================
FontxLayer* fontx_layer_create (GRect frame)
{
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "-- fontx_layer_create()");

    fontx_init(); // fontxの初期化を行う
    
    FontxLayer *fontx_layer = layer_create_with_data(frame, sizeof(FontxLayerData));
    
    FontxLayerData *fontx_data = (FontxLayerData *)layer_get_data(fontx_layer);
    fontx_data->text = 0;  
    fontx_data->bg_color = GColorWhite;
    fontx_data->fg_color = GColorBlack;
    
    layer_set_update_proc(fontx_layer, layer_update_proc);
    layer_mark_dirty(fontx_layer);

    return fontx_layer;
};

void fontx_layer_destroy(FontxLayer *fontx_layer)
{
    FontxLayerData *fontx_data = (FontxLayerData *)layer_get_data(fontx_layer);
    
    if(fontx_data->text != 0)
    {
        free(fontx_data->text);
        fontx_data->text = 0;
    }
    
    layer_destroy(fontx_layer);
    //free(fontx_layer);
};

/**
 * コンテンツのサイズを取得する
 */
GSize fontx_layer_get_content_size(FontxLayer *fontx_layer)
{  
    FontxLayerData *fontx_data = (FontxLayerData *)layer_get_data(fontx_layer);
    return fontx_draw(0, layer_get_bounds(fontx_layer), fontx_data->text );
};

Layer* fontx_layer_get_layer (FontxLayer *fontx_layer)
{
    return fontx_layer;
};

const char* fontx_layer_get_text (FontxLayer *fontx_layer)
{
    FontxLayerData *fontx_data = (FontxLayerData *)layer_get_data(fontx_layer);
    return fontx_data->text;
};

void fontx_layer_set_background_color ( FontxLayer *fontx_layer, GColor color)
{
    FontxLayerData *fontx_data = (FontxLayerData *)layer_get_data(fontx_layer);
    fontx_data->bg_color = color ;
};

//void fontx_layer_set_font ( FontxLayer *fontx_layer, GFont  font);
//void fontx_layer_set_overflow_mode ( FontxLayer *fontx_layer, GTextOverflowMode  line_mode ) ;

void fontx_layer_set_size ( FontxLayer *fontx_layer, const GSize  max_size )
{
    
    GRect rect = layer_get_frame(fontx_layer) ;
    rect.size = max_size;
    layer_set_frame(fontx_layer, rect);
    //FontxLayerData *fontx_data = (FontxLayerData *)layer_get_data(fontx_layer);
    //fontx_data->size = max_size ;
};

void fontx_layer_set_text ( FontxLayer *fontx_layer, const char *text )
{
    FontxLayerData *fontx_data = (FontxLayerData *)layer_get_data(fontx_layer);

    if(fontx_data->text != 0)
    {
        free(fontx_data->text);
        fontx_data->text = 0;
    }
    
    if(text == 0 ) return;
    
    fontx_data->text = malloc(strlen(text)+1);
    strcpy(fontx_data->text, text);
    
    //fontx_layer->text = text ;
};

//void fontx_layer_set_fontx_alignment ( FontxLayer *fontx_layer, GTextAlignment  fontx_alignment );
void fontx_layer_set_fontx_color ( FontxLayer *fontx_layer, GColor  color )
{
    FontxLayerData *fontx_data = (FontxLayerData *)layer_get_data(fontx_layer);
    fontx_data->fg_color = color ;
};

