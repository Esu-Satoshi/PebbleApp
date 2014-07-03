#include <pebble.h>
#include "jptext_draw.h"

//#define APP_LOG(level, fmt...) ;
    
#define BLOCK_MAX 3000

typedef struct {
    //char Identifier[6];		// "FONTX2"という文字列が入る
    //uint8_t FontName[8];		// Font名(8文字)
    uint8_t XSize;		// フォントの横幅
    uint8_t YSize;		// フォントの高さ
    //uint8_t CodeType;
    uint16_t Tnum;		// 文字コードテーブルのエントリ数
    /*
    struct {
        uint16_t Start;	// 領域の始まりの文字コード(SJIS)
        uint16_t End;	// 領域の終わりの文字コード(SJIS)
    } Block[BLOCK_MAX];			// Tnum個続く
    */

    uint32_t Base;

    uint8_t Scale; // フォントの表示倍率(1,2,3)
    uint8_t ExpansionType; // 拡大方法(0,1)
} FontInfo;

FontInfo fontInfo ;

ResHandle FONT_RESOURCE;
ResHandle FONT_TABLE ;

typedef uint16_t wchar ;

//==============================================================


//******************************************************************************
//* 文字コード変換処理
//******************************************************************************

//---------------------------------------------------------------------------
//  UTF8 -> UCS2変換
uint16_t char_utf8_to_ucs2(wchar* dest, const char* src)
{
    int i;
    uint32_t ucs4, temp;

    int len;
    static char len_mask[] = {0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe };
    static char mask[]     = {0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01 };
    
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "char_utf8_to_ucs2 0x%x", src[0]);

    // 1文字目を取得
    temp = (unsigned char)*src++;
    
    // 文字数を判定する
    for (len = 1; len < 6; len++) {
        if (temp < len_mask[len-1]) {
            break;
        }
    }
    ucs4 = temp & mask[len-1];
    //APP_LOG(APP_LOG_LEVEL_DEBUG, ">> : 0x%lx : %lx", ucs4, temp);

    // 2文字目以降を取得
    for (i = 1; i < len; i++) {
        temp = (unsigned char)*src++;
        temp &= 0x3f;
        ucs4 = (ucs4 << 6) + temp;
    }

    // もしUCS2以降の文字なら、■に書き換え
    if (ucs4 > 0xffff) { ucs4 = 0x25a0; }

    *dest = (wchar)ucs4;
    
    //APP_LOG(APP_LOG_LEVEL_DEBUG, ": 0x%x : %d", *dest, len);
    return len;
}

/**
 * utf8の文字列コードからフォントポジションに変換する。
 */
wchar char_ucs2_to_fontpos(wchar code)
{
    uint16_t i, pos=0, cnt;
    cnt = 0;

    uint16_t start;	// 領域の始まりの文字コード
    uint16_t end;	// 領域の終わりの文字コード
    uint8_t n[4];
    
    // コードテーブルからフォントデータの位置を取得する。
    for (i = 0; i < fontInfo.Tnum; i++) 
    {
        resource_load_byte_range(FONT_TABLE, i * 4 + 2, n, 4);
        start = n[1] * 256 + n[0];
        end = n[3] * 256 + n[2];
        
        if (start <= code && end >= code) 
        {
            //APP_LOG(APP_LOG_LEVEL_DEBUG, "-- 0x%x, 0x%x, 0x%x, 0x%x %d--", 
            //        code, fontInfo.Block[i].Start, fontInfo.Block[i].End, pos, cnt);
            
            return (code - start + pos) ;
        }
        pos += (end - start + 1);
        cnt++;
    }
    
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "char_ucs2_to_fontpos not bitmap");

    return 0;
}

/**
 * utf8の文字列コードからフォント位置の配列に変換する。
 *
 * dest srcを変換するために必要な十分なバッファ
 * src  utf8の文字コード列
 */
size_t str_utf8_to_fontpos(wchar* dest, const char* src)
{
    const char* src_pos = src ;
    wchar* dest_pos = dest ;
    size_t len = 0 ;
    wchar ucs2 ;
    
    while(*src_pos != 0)
    {
        // ucs2に変換
        src_pos += char_utf8_to_ucs2(&ucs2, src_pos);
        
        // フォントポジションに変換
        *dest_pos = char_ucs2_to_fontpos(ucs2);
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "ucs2 : 0x%x -> pos: 0x%x", ucs2, *dest_pos);
        
        dest_pos ++ ;
        len++ ;
    }
    
    *dest_pos = 0xffff ;
    return len;
}


//*************************************************************************
//描画処理
//*************************************************************************
void jptext_init(void) 
{
    int16_t i;
    uint8_t n[4];
    
    APP_LOG(APP_LOG_LEVEL_DEBUG, "jptext_init() start");

    // bitmapリソースの読み込み
    FONT_RESOURCE = resource_get_handle(RESOURCE_ID_UCS2_FONT_BITMAP);

    // 全角フォントのみを考慮する。
    FONT_TABLE = resource_get_handle(RESOURCE_ID_UCS2_CODE_TABLE);
    
    //resource_load_byte_range(FONT_RESOURCE, 6, header.FontName, 8);
    resource_load_byte_range(FONT_TABLE, 0, n, 2);
    fontInfo.Tnum =  n[1] * 256 + n[0];
    //if (fontInfo.Tnum > BLOCK_MAX) { fontInfo.Tnum = BLOCK_MAX; } // データ量を100以下に制限
    
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Tnum = %d", fontInfo.Tnum);

    // フォント情報を初期化
    fontInfo.XSize = 8;
    fontInfo.YSize = 8;
    fontInfo.Scale = 2;
    fontInfo.Base = 0;
    fontInfo.ExpansionType = 0;

    /*
    // データブロックの取得
    for (i = 0; i < fontInfo.Tnum; i++)
    {
        resource_load_byte_range(FONT_TABLE, i * 4 + 2, n, 4);
        fontInfo.Block[i].Start = n[1] * 256 + n[0];
        fontInfo.Block[i].End = n[3] * 256 + n[2];
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "Block[%d] = {%x, %x}",i, fontInfo.Block[i].Start, fontInfo.Block[i].End);
    }
    */
    APP_LOG(APP_LOG_LEVEL_DEBUG, "jptext_init() end");
}


/*
    描画範囲内のカーソル位置にcodeで指定されたフォントを描画する。
 */
void charcter_draw(GContext *ctx, GRect rect,  GPoint cr_point,  uint16_t code )
{
    int16_t i, j;
    uint8_t bitmap[8];
    //uint16_t wrd;
    
    uint32_t base = 0;

    //APP_LOG(APP_LOG_LEVEL_DEBUG, "charcter_draw(%d) start", code);

    if( ctx == 0 ) return ;
    
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Base = %ld cr_point = %d, %d", fontInfo.Base, cr_point.x, cr_point.y);

    // 描画位置を取得
    uint16_t px = rect.origin.x + cr_point.x * fontInfo.XSize * fontInfo.Scale ;
    uint16_t py = rect.origin.y + cr_point.y * fontInfo.YSize * fontInfo.Scale ;
    
    // コードテーブルからフォントデータの位置を取得する。
    base = fontInfo.Base + code * 8 ;
    resource_load_byte_range(FONT_RESOURCE, base , bitmap, 8);
    
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "base = %lx", base);

    // フォントデータを描画(ここは後で関数化する)    
    for (i = 0; i < 8; i++) 
    {
        // 1dot毎に描画
        for (j = 0; j < 8; j++) 
        {
            if (bitmap[i] & 0x80) 
            {
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "graphics_draw_rect %d, %d", px+j, py+i);
                graphics_draw_rect(ctx, GRect(px + j*fontInfo.Scale, py + i*fontInfo.Scale, fontInfo.Scale, fontInfo.Scale));
            }
            bitmap[i] = bitmap[i] << 1;
        }
    }
}


// テキストの描画を行う。
// p1: 描画するテキスト(utf8)
// p2: 描画範囲
GSize jptext_text_draw(GContext *ctx, GRect rect, char *text )
{
    GPoint cr_point;
    GSize cr_size;
    uint16_t i;
    size_t text_size;
    
    wchar code;

    //APP_LOG(APP_LOG_LEVEL_DEBUG, "jptext_text_draw() start");

    // 描画出来るカーソル範囲を求める。
    // このプログラムでは半角フォントを扱わないため、全角のフォントサイズのみを考慮する。
    cr_size = GSize(rect.size.w / (fontInfo.XSize * fontInfo.Scale), rect.size.h / (fontInfo.YSize * fontInfo.Scale)) ;

    // カーソルの位置を設定する。
    cr_point = GPoint(0,0);
    
    // utf8からフォント位置の配列に変換する。
    wchar buff[strlen(text)];
    text_size = str_utf8_to_fontpos(buff, text);
    
    for(i=0; i<text_size; i++ )
    {
        code = buff[i];

        if(code == 0x0A)
        {
            // 改行コードの場合、カーソルを移動
            cr_point.x = 0;
            cr_point.y++;
        }
        else 
        {
            if(code != 0xffff) charcter_draw(ctx, rect, cr_point, code);
            cr_point.x ++;
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
    return GSize(rect.size.w, (cr_point.y+1) * (fontInfo.YSize * fontInfo.Scale) ) ;
}
