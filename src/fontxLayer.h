#pragma once
//=============================================================================
// FontXにより日本語を表示するためのfontxLayer
// [仕様]
//  　1. Pebble SDKの構造に従う
//    2. fontサイズは8x8のみ
//    3. 描画サイズはSMALL,MIDDLE,LAGEの３つで等倍、x2,x3とする
//    4. 文字コードはShift-jis(utf8に対応したいが、その場合fontxではなくなる)
//=============================================================================

typedef Layer FontxLayer;

typedef struct { 
	GColor bg_color;
	GColor fg_color;
	char* text;
} FontxLayerData;


FontxLayer* fontx_layer_create (GRect frame);
void fontx_layer_destroy(FontxLayer *fontx_layer);
GSize fontx_layer_get_content_size(FontxLayer *fontx_layer);
Layer* fontx_layer_get_layer (FontxLayer *fontx_layer);
const char* fontx_layer_get_text (FontxLayer *fontx_layer);
void fontx_layer_set_background_color ( FontxLayer *fontx_layer, GColor color);
//void fontx_layer_set_font ( FontxLayer *fontx_layer, GFont  font);
//void fontx_layer_set_overflow_mode ( FontxLayer *fontx_layer, GTextOverflowMode  line_mode ) ;
void fontx_layer_set_size ( FontxLayer *fontx_layer, const GSize  max_size );
void fontx_layer_set_text ( FontxLayer *fontx_layer, const char* text );
//void fontx_layer_set_fontx_alignment ( FontxLayer *fontx_layer, GTextAlignment  fontx_alignment );
void fontx_layer_set_fontx_color ( FontxLayer *fontx_layer, GColor  color );    

