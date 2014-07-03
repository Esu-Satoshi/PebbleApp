#include <pebble.h>
#include "fontx.h"

typedef struct {
    //char Identifier[6];		// "FONTX2"という文字列が入る
    uint8_t FontName[8];		// Font名(8文字)
    uint8_t XSize;		// フォントの横幅
    uint8_t YSize;		// フォントの高さ
    //uint8_t CodeType;
    uint8_t Tnum;		// 文字コードテーブルのエントリ数
    struct {
    	uint16_t Start;	// 領域の始まりの文字コード(SJIS)
    	uint16_t End;	// 領域の終わりの文字コード(SJIS)
    } Block[100];			// Tnum個続く
    uint32_t Base;
    
    uint8_t Scale; // フォントの表示倍率(1,2,3)
    uint8_t ExpansionType; // 拡大方法(0,1)
} fontHeader;

fontHeader header ;

ResHandle FONT_RESOURCE;


//==============================================================
void fontx_init(void) 
{
    int16_t i;
    uint8_t n[2];
    
    // 全角フォントのみを考慮する。
    FONT_RESOURCE = resource_get_handle(RESOURCE_ID_ZENKAKU_FONT);
    
    resource_load_byte_range(FONT_RESOURCE, 6, header.FontName, 8);
    resource_load_byte_range(FONT_RESOURCE, 17, &header.Tnum, 1);
    if (header.Tnum > 100) { header.Tnum = 100; } // データ量を100以下に制限
    
    // サイズは固定
    header.XSize = 8;
    header.YSize = 8;
    
    header.Scale = 2 ;

    header.Base = header.Tnum * 4 + 18;
    header.ExpansionType = 0;

    // データブロックの取得
    for (i = 0; i < header.Tnum; i++)
    {
        resource_load_byte_range(FONT_RESOURCE, i * 4 + 18, n, 2);
        header.Block[i].Start = n[1] * 256 + n[0];

        resource_load_byte_range(FONT_RESOURCE, i * 4 + 20, n, 2);
        header.Block[i].End = n[1] * 256 + n[0];
    }
}

bool is_hankaku(uint16_t ch) {
    if ( (ch >= 0x20 && ch <= 0x7E) || (ch >= 0xA1 && ch <= 0xDF) ) {
        return true;
    }
    return false;
}

bool is_zenkaku(uint16_t ch) {
    if ( (ch >= 0x8100 && ch <= 0x9FFF) || (ch >= 0xE000 && ch <= 0xFCFF) ) {
        return true;
    }
    return false;
}

/* 半角文字を全角文字のコードに変換する関数 */
uint16_t sjis_han2zen(uint8_t in_code)
{    
    if(in_code >= 0x30 && in_code <= 0x39) return  0x824f + (in_code - 0x30); // 数字
    if(in_code >= 0x41 && in_code <= 0x5a) return  0x8260 + (in_code - 0x41); // A-Z
    if(in_code >= 0x61 && in_code <= 0x7a) return  0x8281 + (in_code - 0x61); // a-z
    
    // カタカナ
    if(in_code >= 0xb1 && in_code <= 0xb5) return 0x8302 + (in_code - 0xb1)*2; //  a - o
    if(in_code >= 0xb6 && in_code <= 0xc1) return 0x834a + (in_code - 0xb6)*2; // ka - ti
    if(in_code >= 0xc2 && in_code <= 0xc4) return 0x8363 + (in_code - 0xc2)*2; // tu - to
    if(in_code >= 0xc5 && in_code <= 0xc9) return 0x8369 + (in_code - 0xc5);   // na - no
    if(in_code >= 0xca && in_code <= 0xce) return 0x836e + (in_code - 0xca)*3; // ha - ho
    if(in_code >= 0xcf && in_code <= 0xd3) return 0x837d + (in_code - 0xcf)*1; // ma - mo
    // TODO: まだ途中
    
    return 0;
}


// テキストの描画を行う。
// p1: 描画するテキスト(shift-jis)
// p2: 描画範囲
GSize fontx_draw(GContext *ctx, GRect rect, char *text )
{
    GPoint cr_point;
    GSize cr_size;
    uint16_t i;
    size_t text_size;
    
    uint16_t code;

    // 描画出来るカーソル範囲を求める。
    // このプログラムでは半角フォントを扱わないため、全角のフォントサイズのみを考慮する。
    cr_size = GSize(rect.size.w / (header.XSize * header.Scale), rect.size.h / (header.YSize * header.Scale)) ;

    // カーソルの位置を設定する。
    cr_point = GPoint(0,0);
    
    // 描画範囲を初期化
    
    // 文字解析しつつ描画
    text_size = strlen(text);
    for(i=0; i<text_size; i++ )
    {
        code = text[i];
        // 全角か半角か確認
        if( is_hankaku(code) )
        {
            // 半角の場合、全角コードに変換する
            code = sjis_han2zen(code);
            if(code != 0) charcter_draw(ctx, rect, cr_point, code);
            cr_point.x ++;
        }
        else if( is_zenkaku(code<<8) )
        {
            // 全角の場合、もう1バイト取り出し、２バイトコードにする。
            code = (code << 8) + text[++i] ;
            charcter_draw(ctx, rect, cr_point, code);
            cr_point.x ++;
        }
        else if(code == 0x0A)
        {
            // 改行コードの場合、カーソルを移動
            cr_point.x = 0;
            cr_point.y++;
        }
        
        // カーソルの範囲を確認        
        if( cr_point.x >= cr_size.w )
        {
            // x方向が範囲外の場合は改行
            cr_point.x = 0;
            cr_point.y ++;
        }
        
        if( cr_point.y >= cr_size.h )
        {
            // y方向が範囲外の場合は描画終了
            break;
        }
    }
    
    // 描画に使用した範囲を戻す
    APP_LOG(APP_LOG_LEVEL_DEBUG, "cr_size = %d, %d cr_point = %d, %d", cr_size.w, cr_size.h, cr_point.x, cr_point.y);
    return GSize(rect.size.w, (cr_point.y+1) * (header.YSize * header.Scale) ) ;
}

/*
    描画範囲内のカーソル位置にcodeで指定されたフォントを描画する。
 */
void charcter_draw(GContext *ctx, GRect rect,  GPoint cr_point,  uint16_t code )
{
    int16_t i, j;
    uint8_t buf[2];
    uint16_t wrd;
    
    uint32_t base = 0, cnt = 0;

    if( ctx == 0 ) return ;
    
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "code = %x cr_point = %d, %d", code, cr_point.x, cr_point.y);

    // 描画位置を取得
    uint16_t px = rect.origin.x + cr_point.x * header.XSize * header.Scale ;
    uint16_t py = rect.origin.y + cr_point.y * header.YSize * header.Scale ;
    
    // 1フォントのデータサイズを計算
    uint32_t line_byte_size = ((header.XSize + 7) / 8) ;
    uint32_t font_byte_size = line_byte_size * header.YSize ;
    
    // コードテーブルからフォントデータの位置を取得する。
    for (i = 0; i < header.Tnum; i++) 
    {
        if (header.Block[i].Start <= code && header.Block[i].End >= code) 
        {
            base = header.Base + (code - header.Block[i].Start + cnt) * font_byte_size ;
            break;
        }
        cnt += (header.Block[i].End - header.Block[i].Start + 1);
    }
    
    if(base==0) return ;
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "base = %lx", base);

    // フォントデータを描画(ここは後で関数化する)
    
    for (i = 0; i < header.YSize; i++) 
    {
        // 1行分のデータを取得
        buf[0] = 0x00; buf[1] = 0x00;
        resource_load_byte_range(FONT_RESOURCE, base + (i * line_byte_size), buf, line_byte_size);
        wrd = (buf[0] << 8) + buf[1];

        // 1dot毎に描画
        for (j = 0; j < header.XSize; j++) 
        {
            if (wrd & 0x8000) 
            {
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "graphics_draw_rect %d, %d", px+j, py+i);
                graphics_draw_rect(ctx, GRect(px + j*header.Scale, py + i*header.Scale, header.Scale, header.Scale));
            }
            wrd = wrd << 1;
        }
    }
}
