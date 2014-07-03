//=============================================================================
// FontXにより日本語を表示するためのfontxLayer
// [仕様]
//  　1. Pebble SDKの構造に従う
//    2. fontサイズは8x8のみ
//    3. 描画サイズはSMALL,MIDDLE,LAGEの３つで等倍、x2,x3とする
//    4. 文字コードはShift-jis(utf8に対応したいが、その場合fontxではなくなる)
//=============================================================================

#include <pebble.h>
#include "fontxLayer.h"

Window *window;
//TextLayer *text_layer;
FontxLayer *fontx_layer;

static ScrollLayer *scroll_layer;

uint8_t text[500];

void handle_init(void) 
{
    APP_LOG(APP_LOG_LEVEL_DEBUG, "handle_init()");

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Trace()");
    
    window = window_create();
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Trace()");
    
    GRect bounds = layer_get_frame(window_get_root_layer(window));
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Trace()");

    //text_layer = text_layer_create(GRect(0, 0, 144, 20));

    // scrollLayer を生成して設定
    scroll_layer = scroll_layer_create(bounds);
    scroll_layer_set_click_config_onto_window(scroll_layer, window);
    
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Trace()");
    // fontx Layerを生成してテキストを追加
    fontx_layer = fontx_layer_create(GRect(0, 0, bounds.size.w, 2000));
    
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Trace()");
    {
        ResHandle RH_TEXT = resource_get_handle(RESOURCE_ID_SAMPLE);
        int16_t size = resource_size(RH_TEXT);
        size = (size > 400)? 400 : size ;
        APP_LOG(APP_LOG_LEVEL_DEBUG, "resource_size %d", size);
        
        resource_load_byte_range(RH_TEXT, 0, text, size);
        text[size] = 0;
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Trace()");
    
    fontx_layer_set_text(fontx_layer, (char*)text);
    //fontx_layer_set_text(fontx_layer, "abcdefgあいうえお新谷");
    
    // fontx layerをscroll layerに設定
    // content_sizeをfontxlayerから取得できるようにする必要あり。
    //GSize size = fontx_layer_get_content_size(fontx_layer);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "size %d", size.h);
    //scroll_layer_set_content_size(scroll_layer, GSize(bounds.size.w, size.h));
    scroll_layer_set_content_size(scroll_layer, GSize(bounds.size.w, 2000));
    scroll_layer_add_child(scroll_layer, fontx_layer_get_layer(fontx_layer));
    
    //inverter_layer = inverter_layer_create(GRect(0, 28, bounds.size.w, 28));
    //scroll_layer_add_child(scroll_layer, inverter_layer_get_layer(inverter_layer));

    layer_add_child(window_get_root_layer(window), scroll_layer_get_layer(scroll_layer));
    
    window_stack_push(window, true);
}

void handle_deinit(void) {
    //text_layer_destroy(text_layer);
    fontx_layer_destroy(fontx_layer);
    scroll_layer_destroy(scroll_layer);
    window_destroy(window);
}

int main(void) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "main()");
  handle_init();
  app_event_loop();
  handle_deinit();
}
