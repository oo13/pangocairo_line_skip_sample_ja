
<!--
This file is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This file is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this file.  If not, see <http://www.gnu.org/licenses/>.

Copyright © 2022 OOTA, Masato
-->

# PangoCairoを使った文字列描画で行間を手動で制御するサンプルプログラム
PangoCairoは全自動で文字列を描画してくれるわけですが、行間隔や、パラグラフ間の間隔をプログラムで制御することもできます。これはそのサンプルプログラムです。

上付き・下付き文字やその他の要因で行間隔が適当に可変されるのを止めたいときや、パラグラフ間とそうでない行折返しでの行間隔を変えたいときなどに使えます。

もっと簡単にできるとか、文字間の間隔も制御できる方法があれば教えてください。

English version? There is "source/text/Font.cpp" in <a href="https://github.com/endless-sky/endless-sky/pull/4123">this PR</a> or <a href="https://www.dropbox.com/s/cxurzz9cl9qwj8a/endless-sky-0.9.14_translatable.tar.xz?dl=1">this archive file</a>.

# ビルドの方法
```
gcc -std=gnu99 -Wall -pedantic pangocairo_line_skip_sample_ja.c -lm `pkg-config --cflags --libs pangocairo fontconfig`
```
