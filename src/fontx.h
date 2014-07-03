#pragma once
//=============================================================================
// FontXの表示用関数群
// [仕様]
//  　1. 半角フォントには対応しない→全角コードに変換する。
//    2. fontサイズは8x8のみ
//    3. 描画サイズはSMALL,MIDDLE,LAGEの３つで等倍、x2,x3とする
//    4. 文字コードはShift-jis(utf8に対応したいが、その場合fontxではなくなる)
//=============================================================================

//test

void fontx_init();  // 初期化関数
GSize fontx_draw(GContext *ctx, GRect rect, char *text );
void charcter_draw(GContext *ctx, GRect rect,  GPoint cr_point, uint16_t code );