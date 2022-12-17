/*
  PangoCairoを使った文字列描画で行間を手動で制御するサンプルプログラム

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  Copyright © 2022 OOTA, Masato
*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <fontconfig/fontconfig.h>
#include <pango/pangocairo.h>

/* 各種パラメータ。サンプルプログラムなのでグローバル変数 */

/* フォントのデスクリプタ */
const char* FONT_DESCRIPTOR_NAME = "Serif,Sans";
/* フォントサイズ pixel */
const int FONT_SIZE = 16;
/* 強制する行間の高さ pixel */
const int LINE_HEIGHT = 20;
/* パラグラフ区切りの場合に追加で強制するの行間の高さ pixel */
const int PARAGRAPH_HEIGHT = 4;
/* 出力画像の幅 pixel */
const int OUTPUT_WIDTH = 640;
/* 出力画像の高さ pixel */
const int OUTPUT_HEIGHT = 480;
/* fontconfigの使用を強制するための環境変数 */
char ENV_BACKEND[] = "PANGOCAIRO_BACKEND=fc";



/* Pangoの座標からpixel座標に変換。切り上げる */
int conv_pango_to_pixel(int pango_size)
{
    return ceil((double)(pango_size) / PANGO_SCALE);
}


/* フォント描画に必要な情報 */
struct LayoutInfo {
    cairo_t *cr;
    PangoLayout *layout;
    int font_height;
};



/*
  PangoCairoのセットアップ
  エラーのときは0を返す。
*/
int setup(struct LayoutInfo *out)
{
    /*** FontConfigのセットアップ ***/
    /* システムフォントではなくて、カレントのfontsの下にあるフォントを使う。お好みで */
    if (!FcConfigAppFontAddDir(0, (const FcChar8*)"fonts")) {
        fputs("Error in FcConfigAppFontAddDir().\n", stderr);
        /* ここはエラーしたも無視 */
        fputs("Ignored.\n", stderr);
    }

    /*
      そこにあるfontconfigのコンフィグファイルを使う場合は呼ぶ。お好みで
      エラーがあるとstderrにエラーメッセージを出力するが、エラーは返ってこない。
    */
    FcConfigParseAndLoad(0, (const FcChar8*)"fonts/fonts.cfg", FcTrue);

    /*
      PangoCairoにFontconfigの使用を強制する。
      画像とかテクスチャを作るにはこうするのがよいと思う。
    */
    putenv(ENV_BACKEND);

    /*** Cairoのセットアップ ***/

    /* イメージを出力するためのサーフェスを作る */
    cairo_surface_t *sf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, OUTPUT_WIDTH, OUTPUT_HEIGHT);
    const cairo_status_t cairo_status = cairo_surface_status(sf);
    if (cairo_status != CAIRO_STATUS_SUCCESS) {
        fprintf(stderr, "Error %s in cairo_image_surface_create().\n", cairo_status_to_string(cairo_status));
        return 0;
    }
    /* このサーフェスでCairoの初期化 */
    cairo_t *cr = cairo_create(sf);
    /* サーフェスはもう不要 */
    cairo_surface_destroy(sf);

    /*** Pangoのセットアップ ***/
    /* レイアウトの作成 */
    PangoLayout *layout = pango_cairo_create_layout(cr);
    /* レイアウトのコンテキストを取得 */
    PangoContext *context = pango_layout_get_context(layout);
    /* レイアウトに行折返し方式の指定 */
    pango_layout_set_wrap(layout, PANGO_WRAP_WORD);
    /* フォントデスクリプタを取得 */
    PangoFontDescription *font_desc = pango_font_description_from_string(FONT_DESCRIPTOR_NAME);
    /* フォントデスクリプタにフォントサイズを指定 */
    pango_font_description_set_absolute_size(font_desc, FONT_SIZE * PANGO_SCALE);
    /* 言語を取得 */
    PangoLanguage *lang = pango_language_from_string("ja");
    /* コンテキストに言語を指定 */
    pango_context_set_language(context, lang);
    /* レイアウトにフォントデスクリプタを指定 */
    pango_layout_set_font_description(layout, font_desc);


    /*
      フォントの高さを得る。
      ここでは不要だけど処理のサンプル。
    */
    PangoFontMetrics *metrics = pango_context_get_metrics(context, font_desc, lang);
    const int ascent = pango_font_metrics_get_ascent(metrics);
    const int descent = pango_font_metrics_get_descent(metrics);
    /* ascent + descentがフォントの高さ pixel だと思っていい */
    pango_font_metrics_unref(metrics);
    const int font_height = conv_pango_to_pixel(ascent + descent);

    /* フォントデスクリプタはもう不要 */
    pango_font_description_free(font_desc);


    out->cr = cr;
    out->layout = layout;
    out->font_height = font_height;
    return 1;
}



/*
  このサンプルプログラムの本体。
  textを描画してoutput_filenameにpngファイルとして出力する。
  エラーのときは0を返す。
*/
int render(const char *text, const char *output_filename, const struct LayoutInfo *in)
{
    cairo_t *cr = in->cr;
    PangoLayout *layout = in->layout;
    const int font_height = in->font_height;
    (void)font_height;

    /*** 文字列表示の方法をレイアウトに指定する ***/
    /*
      出力する文字列の幅を指定。
      サーフェスと同じである必要はない。
      -1だと無限幅になるので、改行などによる強制行折返し以外では折り返さないし、
      左寄せ以外にはできない。
    */
    pango_layout_set_width(layout, OUTPUT_WIDTH * PANGO_SCALE);
    /*
      文字列が長すぎる場合の省略の有無、場所(最初、中央、最後)を指定。
      改行などによる強制行折返し以外も折り返す場合は、
      とうぜんNONEしか指定できない。
    */
    pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_NONE);
    /* アライメントを指定。左寄せ、中央寄せ、右寄せ。均等割なら左寄せにする。*/
    pango_layout_set_alignment(layout, PANGO_ALIGN_LEFT);
    /* 均等割にするかどうか。*/
    pango_layout_set_justify(layout, TRUE);

    /** Pangoマークアップサポート **/
    PangoAttrList *al = NULL;
    char *text_without_markup = NULL;
    const char *text_to_render = NULL;
    GError *error = NULL;
    /* マークアップを解析して、文字列とアトリビュートに分ける */
    if (!pango_parse_markup(text, -1, '\0', &al, &text_without_markup, 0, &error)) {
        fputs(error->message, stderr);
        fputs("\nIgnore markups...\n", stderr);
        g_error_free(error);

        /* エラー時は元の文字列にフォールバックさせることにする。お好みで */
        text_to_render = text;
    } else {
        text_to_render = text_without_markup;
        /*
          アトリビュートをレイアウトに設定。
          アトリビュートのないtextを出力するなら呼ぶ必要はない。
        */
        pango_layout_set_attributes(layout, al);
    }
    /*
      文字列をレイアウトに設定。
      マークアップをサポートしないのなら、textを直接指定すればよい。
    */
    pango_layout_set_text(layout, text_to_render, -1);
    /* 取得した文字列とアトリビュートはもう不要。*/
    pango_attr_list_unref(al);
    g_free(text_without_markup);
    al = NULL;
    text_without_markup = NULL;

#if 0
    /* 自分でアトリビュートを作って指定する例 */
    PangoAttribute *ul = pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
    ul->start_index = 0;
    ul->end_index = 3;
    al = pango_layout_get_attributes(layout);
    pango_attr_list_insert(al, ul);
    pango_layout_set_attributes(layout, al);
#endif

#if 0
    /*
      表示する文字列の領域の大きさを得る。
      ここでは不要だけど処理のサンプル。

      イメージ領域からはみださずに描くにはこれが収まっているか確認する必要がある。
    */
    int text_width;
    int text_height;
    pango_layout_get_pixel_size(layout, &text_width, &text_height);
    /*
      PANGO_UNDERLINE_LOWとかを使うと、この領域からはみだす場合がある。
      じっさいに描画される領域を得るにはink_rectを得なければならない。
    */
    PangoRectangle ink_rect;
    pango_layout_get_pixel_extents(layout, &ink_rect, NULL);
    const int ink_width = ink_rect.x + ink_rect.width;
    const int ink_height = ink_rect.y + ink_rect.height;
    text_height = text_height < ink_height ? ink_height : text_height;
    text_width = text_width < ink_width ? ink_width : text_width;
#endif

    /** 描画 **/
    /*
      色の指定。ここでは白。
      Pangoマークアップで指定した色はこれより優先して使用される。
    */
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
#if 0
    /**
      行間を手動で決める必要がないならPangoがぜんぶ自動でやってくれる。
      上付き・下付きなどが収まるように適当に行間を可変させる。
    **/
    cairo_move_to(cr, 0, 0);
    pango_cairo_show_layout(cr, layout);
#else
    /**
      行間を手動で指定する例。
      手動で変更するので、描画領域の高さが最終的に決まるのはこの後になる。
    **/
    /* 描画する文字列をレイアウトから得る */
    const char *layout_text = pango_layout_get_text(layout);
    /* イテレータをレイアウトから得る */
    PangoLayoutIter *it = pango_layout_get_iter(layout);
    /* イテレータ(今は先頭にいる)のベースラインの位置を得る */
    int y0 = pango_layout_iter_get_baseline(it);
    /* baseline_yの単位はpixel。ここはpixel座標で計算する */
    int baseline_y = conv_pango_to_pixel(y0);
    /* 追加した高さ方向のpixel数を入れる変数。高さを計算する例で使う */
    int sum_extra_y = 0;
    /* イテレータ(今は先頭にいる)の開始位置X座標を得る */
    PangoRectangle logical_rect;
    pango_layout_iter_get_line_extents(it, NULL, &logical_rect);
    /* Cairoの描画開始地点をそのぶんずらす。右寄せとかでこれが効いてくる。*/
    cairo_move_to(cr, conv_pango_to_pixel(logical_rect.x), baseline_y);
    /* レイアウトとCairoを同期 */
    pango_cairo_update_layout(cr, layout);
    /* イテレータ(今は先頭にいる)から行を取得 */
    PangoLayoutLine *line = pango_layout_iter_get_line_readonly(it);
    /* その行をCairoに描画 */
    pango_cairo_show_layout_line(cr, line);
    /** 行ごとの処理 **/
    while (pango_layout_iter_next_line(it)) {
        /* イテレータ(今は次の行の先頭にいる)のベースラインの位置を得る */
        const int y1 = pango_layout_iter_get_baseline(it);
        /* イテレータ(今は次の行の先頭にいる)位置の文字列内のインデックスを得る */
        const int index = pango_layout_iter_get_index(it);
        /* Pangoが計算したこの行と前の行のY座標の差分 pixel単位 */
        const int diff_y = conv_pango_to_pixel(y1 - y0);
        /* この行が空の場合の特殊処理。前の行で終わりと同じにする */
        if (layout_text[index] == '\0') {
            /* この行をなかったことにするので、Pangoが計算した高さからこの行のぶんの高さを減らす */
            sum_extra_y -= diff_y;
            break;
        }
        /* 指定の行間に強制するが、減らす方向にはしないことにする。お好みで */
        int add = diff_y > LINE_HEIGHT ? diff_y : LINE_HEIGHT;
        /* 前の文字が\nだったらパラグラフ区切りとみなし、区切りの高さを追加 */
        if (index > 0 && layout_text[index - 1] == '\n') {
            add += PARAGRAPH_HEIGHT;
        }
        /* この行のベースラインのY座標 */
        baseline_y += add;
        /* Pangoが計算した高さからの差分 */
        sum_extra_y += add - diff_y;
        /* イテレータ(今は行の先頭にいる)の開始位置X座標を得る */
        pango_layout_iter_get_line_extents(it, NULL, &logical_rect);
        /* Cairoの描画開始地点をX, Yともそのぶんずらす */
        cairo_move_to(cr, conv_pango_to_pixel(logical_rect.x), baseline_y);
        /* レイアウトとCairoを同期 */
        pango_cairo_update_layout(cr, layout);
        /* イテレータ(今は行の先頭にいる)から行を取得 */
        line = pango_layout_iter_get_line_readonly(it);
        /* その行をCairoに描画 */
        pango_cairo_show_layout_line(cr, line);

        y0 = y1;
    }
    /* もう使わないのでイテレータを解放 */
    pango_layout_iter_free(it);

#if 0
    /* ここでは使わないが、高さを計算する場合の例 */

    /* 最後はパラグラフ境界とする。これはお好みで */
    text_height += sum_extra_y + PARAGRAPH_HEIGHT;
    /* フォントの高さより強制改行の高さが多いときは足りないので、そのぶんを足す */
    if (LINE_HEIGHT > font_height) {
        text_height += LINE_HEIGHT - font_height;
    }
#endif
#endif

    /* サーフェスを得る */
    cairo_surface_t *sf = cairo_get_target(cr);
    /* サーフェスをファイルに出力 */
    cairo_surface_write_to_png(sf, output_filename);

    return 1;
}



/*
  PangoCairoの後始末
*/
void cleanup(const struct LayoutInfo *in)
{
    cairo_destroy(in->cr);
    g_object_unref(in->layout);
}



/* 入力できる最大文字数。ただのサンプルプログラムだから固定 */
const size_t MAX_CHARS = 4000;

int main(int argc, char *argv[])
{
    /* 出力ファイル名は引数のひとつめ。デフォルトは test.png */
    const char *output_filename = "test.png";
    if (argc > 1) {
        output_filename = argv[1];
    }

    char *buf = malloc(MAX_CHARS + 1);
    if (!buf) {
        fputs("No room memory.\n", stderr);
        exit(EXIT_FAILURE);
    }

    size_t i = 0;
    for ( ; i < MAX_CHARS && !ferror(stdin); i++) {
        int c = fgetc(stdin);
        if (c == EOF) {
            break;
        }
        buf[i] = c;
    }
    buf[i] = '\0';

    struct LayoutInfo info;
    if (!setup(&info)) {
        return EXIT_FAILURE;
    }
    if (!render(buf, output_filename, &info)) {
        return EXIT_FAILURE;
    }
    cleanup(&info);

    return EXIT_SUCCESS;
}
