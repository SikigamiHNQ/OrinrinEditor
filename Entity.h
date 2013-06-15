/*! @file
	@brief 実体参照一覧です・大文字小文字に注意されたし
	このファイルは Entity.h です。
	@author	SikigamiHNQ
	@date	2011/00/00
*/

/*
Orinrin Editor : AsciiArt Story Editor for Japanese Only
Copyright (C) 2011 - 2013 Orinrin/SikigamiHNQ

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.
If not, see <http://www.gnu.org/licenses/>.
*/
//-------------------------------------------------------------------------------------------------

typedef struct tagENTITYLIST
{
	UINT	dUniCode;
	CHAR	acCodeA[15];
	TCHAR	atCodeW[15];
	LPTSTR	aptInfo;

} ENTITYLIST, *LPENTITYLIST;

//	HTML的にヤバイやつ
CONST ENTITYLIST	gstEttySP[] = {
	{ 0x0022,	("&quot;"),		TEXT("&quot;"),		TEXT("引用符") },	//	34
	{ 0x0026,	("&amp;"),		TEXT("&amp;"),		TEXT("アンパサンド") },	//	38
	{ 0x003C,	("&lt;"),		TEXT("&lt;"),		TEXT("不等号（より小）") },	//	60
	{ 0x003E,	("&gt;"),		TEXT("&gt;"),		TEXT("不等号（より大）") },	//	62
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

//	SJISじゃ表現出来ないアレ
CONST ENTITYLIST	gstEtyAlpha[] = {
	{ 0x00C1,	("&Aacute;"),	TEXT("&Aacute;"),	TEXT("アキュートアクセント付きA") },	//	193		
	{ 0x00E1,	("&aacute;"),	TEXT("&aacute;"),	TEXT("アキュートアクセント付きA小文字") },	//	225		
	{ 0x00C2,	("&Acirc;"),	TEXT("&Acirc;"),	TEXT("サーカムフレックスアクセント付きA") },	//	194		
	{ 0x00E2,	("&acirc;"),	TEXT("&acirc;"),	TEXT("サーカムフレックスアクセント付きA小文字") },	//	226		
	{ 0x00B4,	("&acute;"),	TEXT("&acute;"),	TEXT("アクサンテギュ") },	//	180		
	{ 0x00C6,	("&AElig;"),	TEXT("&AElig;"),	TEXT("AE") },	//	198		
	{ 0x00E6,	("&aelig;"),	TEXT("&aelig;"),	TEXT("AE小文字") },	//	230		
	{ 0x00C0,	("&Agrave;"),	TEXT("&Agrave;"),	TEXT("グレーブアクセント付きA") },	//	192		
	{ 0x00E0,	("&agrave;"),	TEXT("&agrave;"),	TEXT("グレーブアクセント付きA小文字") },	//	224		
	{ 0x2135,	("&alefsym;"),	TEXT("&alefsym;"),	TEXT("アレフ") },	//	8501		
	{ 0x0391,	("&Alpha;"),	TEXT("&Alpha;"),	TEXT("大文字アルファ") },	//	913		
	{ 0x03B1,	("&alpha;"),	TEXT("&alpha;"),	TEXT("小文字アルファ") },	//	945		
	{ 0x2227,	("&and;"),		TEXT("&and;"),		TEXT("及び（合接）") },	//	8743
	{ 0x2220,	("&ang;"),		TEXT("&ang;"),		TEXT("角") },	//	8736
	{ 0x00C5,	("&Aring;"),	TEXT("&Aring;"),	TEXT("上リング付きA") },	//	197		
	{ 0x00E5,	("&aring;"),	TEXT("&aring;"),	TEXT("上リング付きA小文字") },	//	229		
	{ 0x2248,	("&asymp;"),	TEXT("&asymp;"),	TEXT("近似的に等しい、同相") },	//	8776		
	{ 0x00C3,	("&Atilde;"),	TEXT("&Atilde;"),	TEXT("チルド付きA") },	//	195		
	{ 0x00E3,	("&atilde;"),	TEXT("&atilde;"),	TEXT("チルド付きA小文字") },	//	227		
	{ 0x00C4,	("&Auml;"),		TEXT("&Auml;"),		TEXT("ダイエレシス付きA") },	//	196
	{ 0x00E4,	("&auml;"),		TEXT("&auml;"),		TEXT("ダイエレシス付きA小文字") },	//	228
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyBravo[] = {
	{ 0x201E,	("&bdquo;"),	TEXT("&bdquo;"),	TEXT("下付ダブル引用符") },	//	8222		
	{ 0x0392,	("&Beta;"),		TEXT("&Beta;"),		TEXT("大文字ベータ") },	//	914
	{ 0x03B2,	("&beta;"),		TEXT("&beta;"),		TEXT("小文字ベータ") },	//	946
	{ 0x00A6,	("&brvbar;"),	TEXT("&brvbar;"),	TEXT("破断線") },	//	166		
	{ 0x2022,	("&bull;"),		TEXT("&bull;"),		TEXT("黒丸") },	//	8226
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyCharlie[] = {
	{ 0x2229,	("&cap;"),		TEXT("&cap;"),		TEXT("共通集合") },	//	8745
	{ 0x00C7,	("&Ccedil;"),	TEXT("&Ccedil;"),	TEXT("セディラ付きC") },	//	199		
	{ 0x00E7,	("&ccedil;"),	TEXT("&ccedil;"),	TEXT("セディラ付きC小文字") },	//	231		
	{ 0x00B8,	("&cedil;"),	TEXT("&cedil;"),	TEXT("セディラ") },	//	184		
	{ 0x00A2,	("&cent;"),		TEXT("&cent;"),		TEXT("セント記号") },	//	162
	{ 0x03A7,	("&Chi;"),		TEXT("&Chi;"),		TEXT("大文字カイ") },	//	935
	{ 0x03C7,	("&chi;"),		TEXT("&chi;"),		TEXT("小文字カイ") },	//	967
	{ 0x02C6,	("&circ;"),		TEXT("&circ;"),		TEXT("サーカムフレックス") },	//	710
	{ 0x2663,	("&clubs;"),	TEXT("&clubs;"),	TEXT("クラブ") },	//	9827		
	{ 0x2245,	("&cong;"),		TEXT("&cong;"),		TEXT("同形") },	//	8773
	{ 0x00A9,	("&copy;"),		TEXT("&copy;"),		TEXT("著作権表示記号") },	//	169
	{ 0x21B5,	("&crarr;"),	TEXT("&crarr;"),	TEXT("キャリッジリターン") },	//	8629		
	{ 0x222A,	("&cup;"),		TEXT("&cup;"),		TEXT("合併集合") },	//	8746
	{ 0x00A4,	("&curren;"),	TEXT("&curren;"),	TEXT("不特定通貨記号") },	//	164		
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyDelta[] = {
	{ 0x2020,	("&dagger;"),	TEXT("&dagger;"),	TEXT("ダガー") },	//	8224		
	{ 0x2021,	("&Dagger;"),	TEXT("&Dagger;"),	TEXT("ダブルダガー") },	//	8225		
	{ 0x2193,	("&darr;"),		TEXT("&darr;"),		TEXT("下向矢印") },	//	8595
	{ 0x21D3,	("&dArr;"),		TEXT("&dArr;"),		TEXT("下向二重矢印") },	//	8659
	{ 0x00B0,	("&deg;"),		TEXT("&deg;"),		TEXT("度") },	//	176
	{ 0x0394,	("&Delta;"),	TEXT("&Delta;"),	TEXT("大文字デルタ") },	//	916		
	{ 0x03B4,	("&delta;"),	TEXT("&delta;"),	TEXT("小文字デルタ") },	//	948		
	{ 0x2666,	("&diams;"),	TEXT("&diams;"),	TEXT("ダイヤ") },	//	9830		
	{ 0x00F7,	("&divide;"),	TEXT("&divide;"),	TEXT("除算記号") },	//	247		
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyEcho[] = {
	{ 0x00C9,	("&Eacute;"),	TEXT("&Eacute;"),	TEXT("アキュートアクセント付きE") },	//	201		
	{ 0x00E9,	("&eacute;"),	TEXT("&eacute;"),	TEXT("アキュートアクセント付きE小文字") },	//	233		
	{ 0x00CA,	("&Ecirc;"),	TEXT("&Ecirc;"),	TEXT("サーカムフレックスアクセント付きE") },	//	202		
	{ 0x00EA,	("&ecirc;"),	TEXT("&ecirc;"),	TEXT("サーカムフレックスアクセント付きE小文字") },	//	234		
	{ 0x00C8,	("&Egrave;"),	TEXT("&Egrave;"),	TEXT("グレーブアクセント付きE") },	//	200		
	{ 0x00E8,	("&egrave;"),	TEXT("&egrave;"),	TEXT("グレーブアクセント付きE小文字") },	//	232		
	{ 0x2205,	("&empty;"),	TEXT("&empty;"),	TEXT("空集合") },	//	8709		
	{ 0x2003,	("&emsp;"),		TEXT("&emsp;"),		TEXT("m字幅の空白（全角空白）") },	//	8195
	{ 0x2002,	("&ensp;"),		TEXT("&ensp;"),		TEXT("n字幅の空白（半角空白）") },	//	8194
	{ 0x0395,	("&Epsilon;"),	TEXT("&Epsilon;"),	TEXT("大文字イプシロン") },	//	917		
	{ 0x03B5,	("&epsilon;"),	TEXT("&epsilon;"),	TEXT("小文字イプシロン") },	//	949		
	{ 0x2261,	("&equiv;"),	TEXT("&equiv;"),	TEXT("常に等しい、合同") },	//	8801		
	{ 0x0397,	("&Eta;"),		TEXT("&Eta;"),		TEXT("大文字エータ") },	//	919
	{ 0x03B7,	("&eta;"),		TEXT("&eta;"),		TEXT("小文字エータ") },	//	951
	{ 0x00D0,	("&ETH;"),		TEXT("&ETH;"),		TEXT("アイスランド語ETH") },	//	208
	{ 0x00F0,	("&eth;"),		TEXT("&eth;"),		TEXT("アイスランド語ETH小文字") },	//	240
	{ 0x00CB,	("&Euml;"),		TEXT("&Euml;"),		TEXT("ダイエレシス付きE") },	//	203
	{ 0x00EB,	("&euml;"),		TEXT("&euml;"),		TEXT("ダイエレシス付きE小文字") },	//	235
	{ 0x20AC,	("&euro;"),		TEXT("&euro;"),		TEXT("ユーロ記号") },	//	8364
	{ 0x2203,	("&exist;"),	TEXT("&exist;"),	TEXT("存在する（存在限定子）") },	//	8707		
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyFoxtrot[] = {
	{ 0x0192,	("&fnof;"),		TEXT("&fnof;"),		TEXT("小文字f（フック付き）、関数") },	//	402
	{ 0x2200,	("&forall;"),	TEXT("&forall;"),	TEXT("すべての（普通限定子）") },	//	8704		
	{ 0x00BD,	("&frac12;"),	TEXT("&frac12;"),	TEXT("2分の1") },	//	189		
	{ 0x00BC,	("&frac14;"),	TEXT("&frac14;"),	TEXT("4分の1") },	//	188		
	{ 0x00BE,	("&frac34;"),	TEXT("&frac34;"),	TEXT("4分の3") },	//	190		
	{ 0x2044,	("&frasl;"),	TEXT("&frasl;"),	TEXT("分数の斜線") },	//	8260		
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyGolf[] = {
	{ 0x0393,	("&Gamma;"),	TEXT("&Gamma;"),	TEXT("大文字ガンマ") },	//	915		
	{ 0x03B3,	("&gamma;"),	TEXT("&gamma;"),	TEXT("小文字ガンマ") },	//	947		
	{ 0x2265,	("&ge;"),		TEXT("&ge;"),		TEXT("大なりまたは等しい") },	//	8805
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyHotel[] = {
	{ 0x2194,	("&harr;"),		TEXT("&harr;"),		TEXT("左右向矢印") },	//	8596
	{ 0x21D4,	("&hArr;"),		TEXT("&hArr;"),		TEXT("同値") },	//	8660
	{ 0x2665,	("&hearts;"),	TEXT("&hearts;"),	TEXT("ハート") },	//	9829		
	{ 0x2026,	("&hellip;"),	TEXT("&hellip;"),	TEXT("三点リーダ") },	//	8230		
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyIndia[] = {
	{ 0x00CD,	("&Iacute;"),	TEXT("&Iacute;"),	TEXT("アキュートアクセント付きI") },	//	205		
	{ 0x00ED,	("&iacute;"),	TEXT("&iacute;"),	TEXT("アキュートアクセント付きI小文字") },	//	237		
	{ 0x00CE,	("&Icirc;"),	TEXT("&Icirc;"),	TEXT("サーカムフレックスアクセント付きI") },	//	206		
	{ 0x00EE,	("&icirc;"),	TEXT("&icirc;"),	TEXT("サーカムフレックスアクセント付きI小文字") },	//	238		
	{ 0x00A1,	("&iexcl;"),	TEXT("&iexcl;"),	TEXT("逆感嘆符") },	//	161		
	{ 0x00CC,	("&Igrave;"),	TEXT("&Igrave;"),	TEXT("グレーブアクセント付きI") },	//	204		
	{ 0x00EC,	("&igrave;"),	TEXT("&igrave;"),	TEXT("グレーブアクセント付きI小文字") },	//	236		
	{ 0x2111,	("&image;"),	TEXT("&image;"),	TEXT("大文字のI（ブラック体）、虚数部") },	//	8465		
	{ 0x221E,	("&infin;"),	TEXT("&infin;"),	TEXT("無限大") },	//	8734		
	{ 0x222B,	("&int;"),		TEXT("&int;"),		TEXT("積分記号") },	//	8747
	{ 0x0399,	("&Iota;"),		TEXT("&Iota;"),		TEXT("大文字イオタ") },	//	921
	{ 0x03B9,	("&iota;"),		TEXT("&iota;"),		TEXT("小文字イオタ") },	//	953
	{ 0x00BF,	("&iquest;"),	TEXT("&iquest;"),	TEXT("逆疑問符") },	//	191		
	{ 0x2208,	("&isin;"),		TEXT("&isin;"),		TEXT("属する") },	//	8712
	{ 0x00CF,	("&Iuml;"),		TEXT("&Iuml;"),		TEXT("ダイエレシス付きI") },	//	207
	{ 0x00EF,	("&iuml;"),		TEXT("&iuml;"),		TEXT("ダイエレシス付きI小文字") },	//	239
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyJuliette[] = {
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyKilo[] = {
	{ 0x039A,	("&Kappa;"),	TEXT("&Kappa;"),	TEXT("大文字カッパ") },	//	922		
	{ 0x03BA,	("&kappa;"),	TEXT("&kappa;"),	TEXT("小文字カッパ") },	//	954		
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyLima[] = {
	{ 0x039B,	("&Lambda;"),	TEXT("&Lambda;"),	TEXT("大文字ラムダ") },	//	923		
	{ 0x03BB,	("&lambda;"),	TEXT("&lambda;"),	TEXT("小文字ラムダ") },	//	955		
	{ 0x2329,	("&lang;"),		TEXT("&lang;"),		TEXT("左角括弧") },	//	9001
	{ 0x00AB,	("&laquo;"),	TEXT("&laquo;"),	TEXT("始め二重山括弧引用記号、始めギュメ") },	//	171		
	{ 0x2190,	("&larr;"),		TEXT("&larr;"),		TEXT("左向矢印") },	//	8592
	{ 0x21D0,	("&lArr;"),		TEXT("&lArr;"),		TEXT("左向二重矢印") },	//	8656
	{ 0x2308,	("&lceil;"),	TEXT("&lceil;"),	TEXT("左上限") },	//	8968		
	{ 0x201C,	("&ldquo;"),	TEXT("&ldquo;"),	TEXT("左ダブル引用符") },	//	8220		
	{ 0x2264,	("&le;"),		TEXT("&le;"),		TEXT("小なりまたは等しい") },	//	8804
	{ 0x230A,	("&lfloor;"),	TEXT("&lfloor;"),	TEXT("左下限") },	//	8970		
	{ 0x2217,	("&lowast;"),	TEXT("&lowast;"),	TEXT("アスタリスク演算子") },	//	8727		
	{ 0x25CA,	("&loz;"),		TEXT("&loz;"),		TEXT("菱形") },	//	9674
	{ 0x200E,	("&lrm;"),		TEXT("&lrm;"),		TEXT("記述方向制御（左から右へ）") },	//	8206
	{ 0x2039,	("&lsaquo;"),	TEXT("&lsaquo;"),	TEXT("左山括弧（始）") },	//	8249		
	{ 0x2018,	("&lsquo;"),	TEXT("&lsquo;"),	TEXT("左シングル引用符") },	//	8216		
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyMike[] = {
	{ 0x00AF,	("&macr;"),		TEXT("&macr;"),		TEXT("マクロン") },	//	175
	{ 0x2014,	("&mdash;"),	TEXT("&mdash;"),	TEXT("m字幅のダッシュ（全角ダッシュ）") },	//	8212		
	{ 0x00B5,	("&micro;"),	TEXT("&micro;"),	TEXT("マイクロ記号") },	//	181		
	{ 0x00B7,	("&middot;"),	TEXT("&middot;"),	TEXT("中点（ラテン）") },	//	183		
	{ 0x2212,	("&minus;"),	TEXT("&minus;"),	TEXT("負符号、減算記号") },	//	8722		
	{ 0x039C,	("&Mu;"),		TEXT("&Mu;"),		TEXT("大文字ミュー") },	//	924
	{ 0x03BC,	("&mu;"),		TEXT("&mu;"),		TEXT("小文字ミュー") },	//	956
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyNovember[] = {
	{ 0x2207,	("&nabla;"),	TEXT("&nabla;"),	TEXT("ナブラ") },	//	8711		
	{ 0x00A0,	("&nbsp;"),		TEXT("&nbsp;"),		TEXT("ノーブレークスペース") },	//	160
	{ 0x2013,	("&ndash;"),	TEXT("&ndash;"),	TEXT("n字幅のダッシュ（半角ダッシュ）") },	//	8211		
	{ 0x2260,	("&ne;"),		TEXT("&ne;"),		TEXT("等号否定") },	//	8800
	{ 0x220B,	("&ni;"),		TEXT("&ni;"),		TEXT("元として含む") },	//	8715
	{ 0x00AC,	("&not;"),		TEXT("&not;"),		TEXT("否定記号") },	//	172
	{ 0x2209,	("&notin;"),	TEXT("&notin;"),	TEXT("要素の否定、元の否定") },	//	8713		
	{ 0x2284,	("&nsub;"),		TEXT("&nsub;"),		TEXT("真部分集合の否定") },	//	8836
	{ 0x00D1,	("&Ntilde;"),	TEXT("&Ntilde;"),	TEXT("チルド付きN") },	//	209		
	{ 0x00F1,	("&ntilde;"),	TEXT("&ntilde;"),	TEXT("チルド付きN小文字") },	//	241		
	{ 0x039D,	("&Nu;"),		TEXT("&Nu;"),		TEXT("大文字ニュー") },	//	925
	{ 0x03BD,	("&nu;"),		TEXT("&nu;"),		TEXT("小文字ニュー") },	//	957
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyOscar[] = {
	{ 0x00D3,	("&Oacute;"),	TEXT("&Oacute;"),	TEXT("アキュートアクセント付きO") },	//	211		
	{ 0x00F3,	("&oacute;"),	TEXT("&oacute;"),	TEXT("アキュートアクセント付きO小文字") },	//	243		
	{ 0x00D4,	("&Ocirc;"),	TEXT("&Ocirc;"),	TEXT("サーカムフレックスアクセント付きO") },	//	212		
	{ 0x00F4,	("&ocirc;"),	TEXT("&ocirc;"),	TEXT("サーカムフレックスアクセント付きO小文字") },	//	244		
	{ 0x0152,	("&OElig;"),	TEXT("&OElig;"),	TEXT("リガチャOE大文字、円唇前舌広母音") },	//	338		
	{ 0x0153,	("&oelig;"),	TEXT("&oelig;"),	TEXT("リガチャOE小文字、円唇前舌広・中段母音") },	//	339		
	{ 0x00D2,	("&Ograve;"),	TEXT("&Ograve;"),	TEXT("グレーブアクセント付きO") },	//	210		
	{ 0x00F2,	("&ograve;"),	TEXT("&ograve;"),	TEXT("グレーブアクセント付きO小文字") },	//	242		
	{ 0x203E,	("&oline;"),	TEXT("&oline;"),	TEXT("オーバーライン、論理否定記号") },	//	8254		
	{ 0x03A9,	("&Omega;"),	TEXT("&Omega;"),	TEXT("大文字オメガ") },	//	937		
	{ 0x03C9,	("&omega;"),	TEXT("&omega;"),	TEXT("小文字オメガ") },	//	969		
	{ 0x039F,	("&Omicron;"),	TEXT("&Omicron;"),	TEXT("大文字オミクロン") },	//	927		
	{ 0x03BF,	("&omicron;"),	TEXT("&omicron;"),	TEXT("小文字オミクロン") },	//	959		
	{ 0x2295,	("&oplus;"),	TEXT("&oplus;"),	TEXT("直和") },	//	8853		
	{ 0x2228,	("&or;"),		TEXT("&or;"),		TEXT("又は（離接）") },	//	8744
	{ 0x00AA,	("&ordf;"),		TEXT("&ordf;"),		TEXT("女性序数標識") },	//	170
	{ 0x00BA,	("&ordm;"),		TEXT("&ordm;"),		TEXT("男性序数標識") },	//	186
	{ 0x00D8,	("&Oslash;"),	TEXT("&Oslash;"),	TEXT("ストローク付きO") },	//	216		
	{ 0x00F8,	("&oslash;"),	TEXT("&oslash;"),	TEXT("ストローク付きO小文字") },	//	248		
	{ 0x00D5,	("&Otilde;"),	TEXT("&Otilde;"),	TEXT("チルド付きO") },	//	213		
	{ 0x00F5,	("&otilde;"),	TEXT("&otilde;"),	TEXT("チルド付きO小文字") },	//	245		
	{ 0x2297,	("&otimes;"),	TEXT("&otimes;"),	TEXT("テンソル積") },	//	8855		
	{ 0x00D6,	("&Ouml;"),		TEXT("&Ouml;"),		TEXT("ダイエレシス付きO") },	//	214
	{ 0x00F6,	("&ouml;"),		TEXT("&ouml;"),		TEXT("ダイエレシス付きO小文字") },	//	246
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyPapa[] = {
	{ 0x00B6,	("&para;"),		TEXT("&para;"),		TEXT("段落記号") },	//	182
	{ 0x2202,	("&part;"),		TEXT("&part;"),		TEXT("デル、ラウンドディー") },	//	8706
	{ 0x2030,	("&permil;"),	TEXT("&permil;"),	TEXT("パーミル") },	//	8240		
	{ 0x22A5,	("&perp;"),		TEXT("&perp;"),		TEXT("垂直") },	//	8869
	{ 0x03A6,	("&Phi;"),		TEXT("&Phi;"),		TEXT("大文字ファイ") },	//	934
	{ 0x03C6,	("&phi;"),		TEXT("&phi;"),		TEXT("小文字ファイ") },	//	966
	{ 0x03A0,	("&Pi;"),		TEXT("&Pi;"),		TEXT("大文字パイ") },	//	928
	{ 0x03C0,	("&pi;"),		TEXT("&pi;"),		TEXT("小文字パイ") },	//	960
	{ 0x03D6,	("&piv;"),		TEXT("&piv;"),		TEXT("パイ・シンボル") },	//	982
	{ 0x00B1,	("&plusmn;"),	TEXT("&plusmn;"),	TEXT("正又は負符号") },	//	177		
	{ 0x00A3,	("&pound;"),	TEXT("&pound;"),	TEXT("ポンド記号") },	//	163		
	{ 0x2032,	("&prime;"),	TEXT("&prime;"),	TEXT("分") },	//	8242		
	{ 0x2033,	("&Prime;"),	TEXT("&Prime;"),	TEXT("秒") },	//	8243		
	{ 0x220F,	("&prod;"),		TEXT("&prod;"),		TEXT("n項の乗算") },	//	8719
	{ 0x221D,	("&prop;"),		TEXT("&prop;"),		TEXT("比例") },	//	8733
	{ 0x03A8,	("&Psi;"),		TEXT("&Psi;"),		TEXT("大文字プサイ") },	//	936
	{ 0x03C8,	("&psi;"),		TEXT("&psi;"),		TEXT("小文字プサイ") },	//	968
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyQuebec[] = {
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyRomeo[] = {
	{ 0x221A,	("&radic;"),	TEXT("&radic;"),	TEXT("根号") },	//	8730		
	{ 0x232A,	("&rang;"),		TEXT("&rang;"),		TEXT("右角括弧") },	//	9002
	{ 0x00BB,	("&raquo;"),	TEXT("&raquo;"),	TEXT("終わり二重山括弧引用記号、終わりギュメ") },	//	187		
	{ 0x2192,	("&rarr;"),		TEXT("&rarr;"),		TEXT("右向矢印") },	//	8594
	{ 0x21D2,	("&rArr;"),		TEXT("&rArr;"),		TEXT("ならば（合意）") },	//	8658
	{ 0x2309,	("&rceil;"),	TEXT("&rceil;"),	TEXT("右上限") },	//	8969		
	{ 0x201D,	("&rdquo;"),	TEXT("&rdquo;"),	TEXT("右ダブル引用符（終）") },	//	8221		
	{ 0x211C,	("&real;"),		TEXT("&real;"),		TEXT("大文字のR（ブラック体）、実数部") },	//	8476
	{ 0x00AE,	("&reg;"),		TEXT("&reg;"),		TEXT("登録商標記号") },	//	174
	{ 0x230B,	("&rfloor;"),	TEXT("&rfloor;"),	TEXT("右下限") },	//	8971		
	{ 0x03A1,	("&Rho;"),		TEXT("&Rho;"),		TEXT("大文字ロー") },	//	929
	{ 0x03C1,	("&rho;"),		TEXT("&rho;"),		TEXT("小文字ロー") },	//	961
	{ 0x200F,	("&rlm;"),		TEXT("&rlm;"),		TEXT("記述方向制御（右から左へ）") },	//	8207
	{ 0x203A,	("&rsaquo;"),	TEXT("&rsaquo;"),	TEXT("右山括弧（終）") },	//	8250		
	{ 0x2019,	("&rsquo;"),	TEXT("&rsquo;"),	TEXT("右シングル引用符") },	//	8217		
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtySierra[] = {
	{ 0x201A,	("&sbquo;"),	TEXT("&sbquo;"),	TEXT("下付引用符") },	//	8218		
	{ 0x0160,	("&Scaron;"),	TEXT("&Scaron;"),	TEXT("キャロン付きS") },	//	352		
	{ 0x0161,	("&scaron;"),	TEXT("&scaron;"),	TEXT("キャロン付きS小文字") },	//	353		
	{ 0x22C5,	("&sdot;"),		TEXT("&sdot;"),		TEXT("ドット演算子") },	//	8901
	{ 0x00A7,	("&sect;"),		TEXT("&sect;"),		TEXT("節記号") },	//	167
	{ 0x00AD,	("&shy;"),		TEXT("&shy;"),		TEXT("ソフトハイフン") },	//	173
	{ 0x03A3,	("&Sigma;"),	TEXT("&Sigma;"),	TEXT("大文字シグマ") },	//	931		
	{ 0x03C3,	("&sigma;"),	TEXT("&sigma;"),	TEXT("小文字シグマ") },	//	963		
	{ 0x03C2,	("&sigmaf;"),	TEXT("&sigmaf;"),	TEXT("小文字ファイナル・シグマ") },	//	962		
	{ 0x223C,	("&sim;"),		TEXT("&sim;"),		TEXT("チルダ演算子") },	//	8764
	{ 0x2660,	("&spades;"),	TEXT("&spades;"),	TEXT("スペード") },	//	9824		
	{ 0x2282,	("&sub;"),		TEXT("&sub;"),		TEXT("真部分集合") },	//	8834
	{ 0x2286,	("&sube;"),		TEXT("&sube;"),		TEXT("部分集合") },	//	8838
	{ 0x2211,	("&sum;"),		TEXT("&sum;"),		TEXT("n項の加算") },	//	8721
	{ 0x2283,	("&sup;"),		TEXT("&sup;"),		TEXT("真部分集合（逆方向）") },	//	8835
	{ 0x00B9,	("&sup1;"),		TEXT("&sup1;"),		TEXT("上付き1") },	//	185
	{ 0x00B2,	("&sup2;"),		TEXT("&sup2;"),		TEXT("上付き2") },	//	178
	{ 0x00B3,	("&sup3;"),		TEXT("&sup3;"),		TEXT("上付き3") },	//	179
	{ 0x2287,	("&supe;"),		TEXT("&supe;"),		TEXT("部分集合（逆方向）") },	//	8839
	{ 0x00DF,	("&szlig;"),	TEXT("&szlig;"),	TEXT("ドイツ語エスツェット") },	//	223		
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyTango[] = {
	{ 0x03A4,	("&Tau;"),		TEXT("&Tau;"),		TEXT("大文字タウ") },	//	932
	{ 0x03C4,	("&tau;"),		TEXT("&tau;"),		TEXT("小文字タウ") },	//	964
	{ 0x2234,	("&there4;"),	TEXT("&there4;"),	TEXT("ゆえに") },	//	8756		
	{ 0x0398,	("&Theta;"),	TEXT("&Theta;"),	TEXT("大文字シータ") },	//	920		
	{ 0x03B8,	("&theta;"),	TEXT("&theta;"),	TEXT("小文字シータ") },	//	952		
	{ 0x03D1,	("&thetasym;"),	TEXT("&thetasym;"),	TEXT("小文字シータ・シンボル") },	//	977		
	{ 0x2009,	("&thinsp;"),	TEXT("&thinsp;"),	TEXT("せまい空白") },	//	8201		
	{ 0x00DE,	("&THORN;"),	TEXT("&THORN;"),	TEXT("アイスランド語THORN") },	//	222		
	{ 0x00FE,	("&thorn;"),	TEXT("&thorn;"),	TEXT("アイスランド語THORN小文字") },	//	254		
	{ 0x02DC,	("&tilde;"),	TEXT("&tilde;"),	TEXT("小チルダ") },	//	732		
	{ 0x00D7,	("&times;"),	TEXT("&times;"),	TEXT("乗算記号") },	//	215		
	{ 0x2122,	("&trade;"),	TEXT("&trade;"),	TEXT("登録商標記号") },	//	8482		
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyUniform[] = {
	{ 0x00DA,	("&Uacute;"),	TEXT("&Uacute;"),	TEXT("アキュートアクセント付きU") },	//	218		
	{ 0x00FA,	("&uacute;"),	TEXT("&uacute;"),	TEXT("アキュートアクセント付きU小文字") },	//	250		
	{ 0x2191,	("&uarr;"),		TEXT("&uarr;"),		TEXT("上向矢印") },	//	8593
	{ 0x21D1,	("&uArr;"),		TEXT("&uArr;"),		TEXT("上向二重矢印") },	//	8657
	{ 0x00DB,	("&Ucirc;"),	TEXT("&Ucirc;"),	TEXT("サーカムフレックスアクセント付きU") },	//	219		
	{ 0x00FB,	("&ucirc;"),	TEXT("&ucirc;"),	TEXT("サーカムフレックスアクセント付きU小文字") },	//	251		
	{ 0x00D9,	("&Ugrave;"),	TEXT("&Ugrave;"),	TEXT("グレーブアクセント付きU") },	//	217		
	{ 0x00F9,	("&ugrave;"),	TEXT("&ugrave;"),	TEXT("グレーブアクセント付きU小文字") },	//	249		
	{ 0x00A8,	("&uml;"),		TEXT("&uml;"),		TEXT("ウムラウト、ダイエレシス") },	//	168
	{ 0x03D2,	("&upsih;"),	TEXT("&upsih;"),	TEXT("ウプシロン（フック・シンボル付き）") },	//	978		
	{ 0x03A5,	("&Upsilon;"),	TEXT("&Upsilon;"),	TEXT("大文字ウプシロン") },	//	933		
	{ 0x03C5,	("&upsilon;"),	TEXT("&upsilon;"),	TEXT("小文字ウプシロン") },	//	965		
	{ 0x00DC,	("&Uuml;"),		TEXT("&Uuml;"),		TEXT("ダイエレシス付きU") },	//	220
	{ 0x00FC,	("&uuml;"),		TEXT("&uuml;"),		TEXT("ダイエレシス付きU小文字") },	//	252
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyVictor[] = {
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyWhiskey[] = {
	{ 0x2118,	("&weierp;"),	TEXT("&weierp;"),	TEXT("大文字のP（スクリプト体）、WeierstrassのP関数") },	//	8472		
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyXray[] = {
	{ 0x039E,	("&Xi;"),		TEXT("&Xi;"),		TEXT("大文字グザイ") },	//	926
	{ 0x03BE,	("&xi;"),		TEXT("&xi;"),		TEXT("小文字グザイ") },	//	958
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyYankee[] = {
	{ 0x00DD,	("&Yacute;"),	TEXT("&Yacute;"),	TEXT("アキュートアクセント付きY") },	//	221		
	{ 0x00FD,	("&yacute;"),	TEXT("&yacute;"),	TEXT("アキュートアクセント付きY小文字") },	//	253		
	{ 0x00A5,	("&yen;"),		TEXT("&yen;"),		TEXT("円記号") },	//	165
	{ 0x00FF,	("&yuml;"),		TEXT("&yuml;"),		TEXT("ダイエレシス付きY小文字") },	//	255
	{ 0x0178,	("&Yuml;"),		TEXT("&Yuml;"),		TEXT("ダイエレシス付きY") },	//	376
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	gstEtyZulu[] = {
	{ 0x0396,	("&Zeta;"),		TEXT("&Zeta;"),		TEXT("大文字ゼータ") },	//	918
	{ 0x03B6,	("&zeta;"),		TEXT("&zeta;"),		TEXT("小文字ゼータ") },	//	950
	{ 0x200D,	("&zwj;"),		TEXT("&zwj;"),		TEXT("幅なし接続子") },	//	8205
	{ 0x200C,	("&zwnj;"),		TEXT("&zwnj;"),		TEXT("幅なし非接続子") },	//	8204
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("末端ダミー") }
};

CONST ENTITYLIST	*gpstEntitys[] = { gstEtyAlpha, gstEtyBravo,
	gstEtyCharlie, gstEtyDelta, gstEtyEcho, gstEtyFoxtrot, gstEtyGolf,
	gstEtyHotel, gstEtyIndia, gstEtyJuliette, gstEtyKilo, gstEtyLima,
	gstEtyMike, gstEtyNovember, gstEtyOscar, gstEtyPapa, gstEtyQuebec,
	gstEtyRomeo, gstEtySierra, gstEtyTango, gstEtyUniform, gstEtyVictor,
	gstEtyWhiskey, gstEtyXray, gstEtyYankee, gstEtyZulu };

/*

	{ 0x00A0,	("&nbsp;"),		TEXT("&nbsp;"),		TEXT("ノーブレークスペース") },	//	160
	{ 0x00A1,	("&iexcl;"),	TEXT("&iexcl;"),	TEXT("逆感嘆符") },	//	161
	{ 0x00A2,	("&cent;"),		TEXT("&cent;"),		TEXT("セント記号") },	//	162
	{ 0x00A3,	("&pound;"),	TEXT("&pound;"),	TEXT("ポンド記号") },	//	163
	{ 0x00A4,	("&curren;"),	TEXT("&curren;"),	TEXT("不特定通貨記号") },	//	164
	{ 0x00A5,	("&yen;"),		TEXT("&yen;"),		TEXT("円記号") },	//	165
	{ 0x00A6,	("&brvbar;"),	TEXT("&brvbar;"),	TEXT("破断線") },	//	166
	{ 0x00A7,	("&sect;"),		TEXT("&sect;"),		TEXT("節記号") },	//	167
	{ 0x00A8,	("&uml;"),		TEXT("&uml;"),		TEXT("ウムラウト、ダイエレシス") },	//	168
	{ 0x00A9,	("&copy;"),		TEXT("&copy;"),		TEXT("著作権表示記号") },	//	169
	{ 0x00AA,	("&ordf;"),		TEXT("&ordf;"),		TEXT("女性序数標識") },	//	170
	{ 0x00AB,	("&laquo;"),	TEXT("&laquo;"),	TEXT("始め二重山括弧引用記号、始めギュメ") },	//	171
	{ 0x00AC,	("&not;"),		TEXT("&not;"),		TEXT("否定記号") },	//	172
	{ 0x00AD,	("&shy;"),		TEXT("&shy;"),		TEXT("ソフトハイフン") },	//	173
	{ 0x00AE,	("&reg;"),		TEXT("&reg;"),		TEXT("登録商標記号") },	//	174
	{ 0x00AF,	("&macr;"),		TEXT("&macr;"),		TEXT("マクロン") },	//	175
	{ 0x00B0,	("&deg;"),		TEXT("&deg;"),		TEXT("度") },	//	176
	{ 0x00B1,	("&plusmn;"),	TEXT("&plusmn;"),	TEXT("正又は負符号") },	//	177
	{ 0x00B2,	("&sup2;"),		TEXT("&sup2;"),		TEXT("上付き2") },	//	178
	{ 0x00B3,	("&sup3;"),		TEXT("&sup3;"),		TEXT("上付き3") },	//	179
	{ 0x00B4,	("&acute;"),	TEXT("&acute;"),	TEXT("アクサンテギュ") },	//	180
	{ 0x00B5,	("&micro;"),	TEXT("&micro;"),	TEXT("マイクロ記号") },	//	181
	{ 0x00B6,	("&para;"),		TEXT("&para;"),		TEXT("段落記号") },	//	182
	{ 0x00B7,	("&middot;"),	TEXT("&middot;"),	TEXT("中点（ラテン）") },	//	183
	{ 0x00B8,	("&cedil;"),	TEXT("&cedil;"),	TEXT("セディラ") },	//	184
	{ 0x00B9,	("&sup1;"),		TEXT("&sup1;"),		TEXT("上付き1") },	//	185
	{ 0x00BA,	("&ordm;"),		TEXT("&ordm;"),		TEXT("男性序数標識") },	//	186
	{ 0x00BB,	("&raquo;"),	TEXT("&raquo;"),	TEXT("終わり二重山括弧引用記号、終わりギュメ") },	//	187
	{ 0x00BC,	("&frac14;"),	TEXT("&frac14;"),	TEXT("4分の1") },	//	188
	{ 0x00BD,	("&frac12;"),	TEXT("&frac12;"),	TEXT("2分の1") },	//	189
	{ 0x00BE,	("&frac34;"),	TEXT("&frac34;"),	TEXT("4分の3") },	//	190
	{ 0x00BF,	("&iquest;"),	TEXT("&iquest;"),	TEXT("逆疑問符") },	//	191
	{ 0x00C0,	("&Agrave;"),	TEXT("&Agrave;"),	TEXT("グレーブアクセント付きA") },	//	192
	{ 0x00C1,	("&Aacute;"),	TEXT("&Aacute;"),	TEXT("アキュートアクセント付きA") },	//	193
	{ 0x00C2,	("&Acirc;"),	TEXT("&Acirc;"),	TEXT("サーカムフレックスアクセント付きA") },	//	194
	{ 0x00C3,	("&Atilde;"),	TEXT("&Atilde;"),	TEXT("チルド付きA") },	//	195
	{ 0x00C4,	("&Auml;"),		TEXT("&Auml;"),		TEXT("ダイエレシス付きA") },	//	196
	{ 0x00C5,	("&Aring;"),	TEXT("&Aring;"),	TEXT("上リング付きA") },	//	197
	{ 0x00C6,	("&AElig;"),	TEXT("&AElig;"),	TEXT("AE") },	//	198
	{ 0x00C7,	("&Ccedil;"),	TEXT("&Ccedil;"),	TEXT("セディラ付きC") },	//	199
	{ 0x00C8,	("&Egrave;"),	TEXT("&Egrave;"),	TEXT("グレーブアクセント付きE") },	//	200
	{ 0x00C9,	("&Eacute;"),	TEXT("&Eacute;"),	TEXT("アキュートアクセント付きE") },	//	201
	{ 0x00CA,	("&Ecirc;"),	TEXT("&Ecirc;"),	TEXT("サーカムフレックスアクセント付きE") },	//	202
	{ 0x00CB,	("&Euml;"),		TEXT("&Euml;"),		TEXT("ダイエレシス付きE") },	//	203
	{ 0x00CC,	("&Igrave;"),	TEXT("&Igrave;"),	TEXT("グレーブアクセント付きI") },	//	204
	{ 0x00CD,	("&Iacute;"),	TEXT("&Iacute;"),	TEXT("アキュートアクセント付きI") },	//	205
	{ 0x00CE,	("&Icirc;"),	TEXT("&Icirc;"),	TEXT("サーカムフレックスアクセント付きI") },	//	206
	{ 0x00CF,	("&Iuml;"),		TEXT("&Iuml;"),		TEXT("ダイエレシス付きI") },	//	207
	{ 0x00D0,	("&ETH;"),		TEXT("&ETH;"),		TEXT("アイスランド語ETH") },	//	208
	{ 0x00D1,	("&Ntilde;"),	TEXT("&Ntilde;"),	TEXT("チルド付きN") },	//	209
	{ 0x00D2,	("&Ograve;"),	TEXT("&Ograve;"),	TEXT("グレーブアクセント付きO") },	//	210
	{ 0x00D3,	("&Oacute;"),	TEXT("&Oacute;"),	TEXT("アキュートアクセント付きO") },	//	211
	{ 0x00D4,	("&Ocirc;"),	TEXT("&Ocirc;"),	TEXT("サーカムフレックスアクセント付きO") },	//	212
	{ 0x00D5,	("&Otilde;"),	TEXT("&Otilde;"),	TEXT("チルド付きO") },	//	213
	{ 0x00D6,	("&Ouml;"),		TEXT("&Ouml;"),		TEXT("ダイエレシス付きO") },	//	214
	{ 0x00D7,	("&times;"),	TEXT("&times;"),	TEXT("乗算記号") },	//	215
	{ 0x00D8,	("&Oslash;"),	TEXT("&Oslash;"),	TEXT("ストローク付きO") },	//	216
	{ 0x00D9,	("&Ugrave;"),	TEXT("&Ugrave;"),	TEXT("グレーブアクセント付きU") },	//	217
	{ 0x00DA,	("&Uacute;"),	TEXT("&Uacute;"),	TEXT("アキュートアクセント付きU") },	//	218
	{ 0x00DB,	("&Ucirc;"),	TEXT("&Ucirc;"),	TEXT("サーカムフレックスアクセント付きU") },	//	219
	{ 0x00DC,	("&Uuml;"),		TEXT("&Uuml;"),		TEXT("ダイエレシス付きU") },	//	220
	{ 0x00DD,	("&Yacute;"),	TEXT("&Yacute;"),	TEXT("アキュートアクセント付きY") },	//	221
	{ 0x00DE,	("&THORN;"),	TEXT("&THORN;"),	TEXT("アイスランド語THORN") },	//	222
	{ 0x00DF,	("&szlig;"),	TEXT("&szlig;"),	TEXT("ドイツ語エスツェット") },	//	223
	{ 0x00E0,	("&agrave;"),	TEXT("&agrave;"),	TEXT("グレーブアクセント付きA小文字") },	//	224
	{ 0x00E1,	("&aacute;"),	TEXT("&aacute;"),	TEXT("アキュートアクセント付きA小文字") },	//	225
	{ 0x00E2,	("&acirc;"),	TEXT("&acirc;"),	TEXT("サーカムフレックスアクセント付きA小文字") },	//	226
	{ 0x00E3,	("&atilde;"),	TEXT("&atilde;"),	TEXT("チルド付きA小文字") },	//	227
	{ 0x00E4,	("&auml;"),		TEXT("&auml;"),		TEXT("ダイエレシス付きA小文字") },	//	228
	{ 0x00E5,	("&aring;"),	TEXT("&aring;"),	TEXT("上リング付きA小文字") },	//	229
	{ 0x00E6,	("&aelig;"),	TEXT("&aelig;"),	TEXT("AE小文字") },	//	230
	{ 0x00E7,	("&ccedil;"),	TEXT("&ccedil;"),	TEXT("セディラ付きC小文字") },	//	231
	{ 0x00E8,	("&egrave;"),	TEXT("&egrave;"),	TEXT("グレーブアクセント付きE小文字") },	//	232
	{ 0x00E9,	("&eacute;"),	TEXT("&eacute;"),	TEXT("アキュートアクセント付きE小文字") },	//	233
	{ 0x00EA,	("&ecirc;"),	TEXT("&ecirc;"),	TEXT("サーカムフレックスアクセント付きE小文字") },	//	234
	{ 0x00EB,	("&euml;"),		TEXT("&euml;"),		TEXT("ダイエレシス付きE小文字") },	//	235
	{ 0x00EC,	("&igrave;"),	TEXT("&igrave;"),	TEXT("グレーブアクセント付きI小文字") },	//	236
	{ 0x00ED,	("&iacute;"),	TEXT("&iacute;"),	TEXT("アキュートアクセント付きI小文字") },	//	237
	{ 0x00EE,	("&icirc;"),	TEXT("&icirc;"),	TEXT("サーカムフレックスアクセント付きI小文字") },	//	238
	{ 0x00EF,	("&iuml;"),		TEXT("&iuml;"),		TEXT("ダイエレシス付きI小文字") },	//	239
	{ 0x00F0,	("&eth;"),		TEXT("&eth;"),		TEXT("アイスランド語ETH小文字") },	//	240
	{ 0x00F1,	("&ntilde;"),	TEXT("&ntilde;"),	TEXT("チルド付きN小文字") },	//	241
	{ 0x00F2,	("&ograve;"),	TEXT("&ograve;"),	TEXT("グレーブアクセント付きO小文字") },	//	242
	{ 0x00F3,	("&oacute;"),	TEXT("&oacute;"),	TEXT("アキュートアクセント付きO小文字") },	//	243
	{ 0x00F4,	("&ocirc;"),	TEXT("&ocirc;"),	TEXT("サーカムフレックスアクセント付きO小文字") },	//	244
	{ 0x00F5,	("&otilde;"),	TEXT("&otilde;"),	TEXT("チルド付きO小文字") },	//	245
	{ 0x00F6,	("&ouml;"),		TEXT("&ouml;"),		TEXT("ダイエレシス付きO小文字") },	//	246
	{ 0x00F7,	("&divide;"),	TEXT("&divide;"),	TEXT("除算記号") },	//	247
	{ 0x00F8,	("&oslash;"),	TEXT("&oslash;"),	TEXT("ストローク付きO小文字") },	//	248
	{ 0x00F9,	("&ugrave;"),	TEXT("&ugrave;"),	TEXT("グレーブアクセント付きU小文字") },	//	249
	{ 0x00FA,	("&uacute;"),	TEXT("&uacute;"),	TEXT("アキュートアクセント付きU小文字") },	//	250
	{ 0x00FB,	("&ucirc;"),	TEXT("&ucirc;"),	TEXT("サーカムフレックスアクセント付きU小文字") },	//	251
	{ 0x00FC,	("&uuml;"),		TEXT("&uuml;"),		TEXT("ダイエレシス付きU小文字") },	//	252
	{ 0x00FD,	("&yacute;"),	TEXT("&yacute;"),	TEXT("アキュートアクセント付きY小文字") },	//	253
	{ 0x00FE,	("&thorn;"),	TEXT("&thorn;"),	TEXT("アイスランド語THORN小文字") },	//	254
	{ 0x00FF,	("&yuml;"),		TEXT("&yuml;"),		TEXT("ダイエレシス付きY小文字") },	//	255
	{ 0x0152,	("&OElig;"),	TEXT("&OElig;"),	TEXT("リガチャOE大文字、円唇前舌広母音") },	//	338
	{ 0x0153,	("&oelig;"),	TEXT("&oelig;"),	TEXT("リガチャOE小文字、円唇前舌広・中段母音") },	//	339
	{ 0x0160,	("&Scaron;"),	TEXT("&Scaron;"),	TEXT("キャロン付きS") },	//	352
	{ 0x0161,	("&scaron;"),	TEXT("&scaron;"),	TEXT("キャロン付きS小文字") },	//	353
	{ 0x0178,	("&Yuml;"),		TEXT("&Yuml;"),		TEXT("ダイエレシス付きY") },	//	376
	{ 0x0192,	("&fnof;"),		TEXT("&fnof;"),		TEXT("小文字f（フック付き）、関数") },	//	402
	{ 0x02C6,	("&circ;"),		TEXT("&circ;"),		TEXT("サーカムフレックス") },	//	710
	{ 0x02DC,	("&tilde;"),	TEXT("&tilde;"),	TEXT("小チルダ") },	//	732
	{ 0x0391,	("&Alpha;"),	TEXT("&Alpha;"),	TEXT("大文字アルファ") },	//	913
	{ 0x0392,	("&Beta;"),		TEXT("&Beta;"),		TEXT("大文字ベータ") },	//	914
	{ 0x0393,	("&Gamma;"),	TEXT("&Gamma;"),	TEXT("大文字ガンマ") },	//	915
	{ 0x0394,	("&Delta;"),	TEXT("&Delta;"),	TEXT("大文字デルタ") },	//	916
	{ 0x0395,	("&Epsilon;"),	TEXT("&Epsilon;"),	TEXT("大文字イプシロン") },	//	917
	{ 0x0396,	("&Zeta;"),		TEXT("&Zeta;"),		TEXT("大文字ゼータ") },	//	918
	{ 0x0397,	("&Eta;"),		TEXT("&Eta;"),		TEXT("大文字エータ") },	//	919
	{ 0x0398,	("&Theta;"),	TEXT("&Theta;"),	TEXT("大文字シータ") },	//	920
	{ 0x0399,	("&Iota;"),		TEXT("&Iota;"),		TEXT("大文字イオタ") },	//	921
	{ 0x039A,	("&Kappa;"),	TEXT("&Kappa;"),	TEXT("大文字カッパ") },	//	922
	{ 0x039B,	("&Lambda;"),	TEXT("&Lambda;"),	TEXT("大文字ラムダ") },	//	923
	{ 0x039C,	("&Mu;"),		TEXT("&Mu;"),		TEXT("大文字ミュー") },	//	924
	{ 0x039D,	("&Nu;"),		TEXT("&Nu;"),		TEXT("大文字ニュー") },	//	925
	{ 0x039E,	("&Xi;"),		TEXT("&Xi;"),		TEXT("大文字グザイ") },	//	926
	{ 0x039F,	("&Omicron;"),	TEXT("&Omicron;"),	TEXT("大文字オミクロン") },	//	927
	{ 0x03A0,	("&Pi;"),		TEXT("&Pi;"),		TEXT("大文字パイ") },	//	928
	{ 0x03A1,	("&Rho;"),		TEXT("&Rho;"),		TEXT("大文字ロー") },	//	929
	{ 0x03A3,	("&Sigma;"),	TEXT("&Sigma;"),	TEXT("大文字シグマ") },	//	931
	{ 0x03A4,	("&Tau;"),		TEXT("&Tau;"),		TEXT("大文字タウ") },	//	932
	{ 0x03A5,	("&Upsilon;"),	TEXT("&Upsilon;"),	TEXT("大文字ウプシロン") },	//	933
	{ 0x03A6,	("&Phi;"),		TEXT("&Phi;"),		TEXT("大文字ファイ") },	//	934
	{ 0x03A7,	("&Chi;"),		TEXT("&Chi;"),		TEXT("大文字カイ") },	//	935
	{ 0x03A8,	("&Psi;"),		TEXT("&Psi;"),		TEXT("大文字プサイ") },	//	936
	{ 0x03A9,	("&Omega;"),	TEXT("&Omega;"),	TEXT("大文字オメガ") },	//	937
	{ 0x03B1,	("&alpha;"),	TEXT("&alpha;"),	TEXT("小文字アルファ") },	//	945
	{ 0x03B2,	("&beta;"),		TEXT("&beta;"),		TEXT("小文字ベータ") },	//	946
	{ 0x03B3,	("&gamma;"),	TEXT("&gamma;"),	TEXT("小文字ガンマ") },	//	947
	{ 0x03B4,	("&delta;"),	TEXT("&delta;"),	TEXT("小文字デルタ") },	//	948
	{ 0x03B5,	("&epsilon;"),	TEXT("&epsilon;"),	TEXT("小文字イプシロン") },	//	949
	{ 0x03B6,	("&zeta;"),		TEXT("&zeta;"),		TEXT("小文字ゼータ") },	//	950
	{ 0x03B7,	("&eta;"),		TEXT("&eta;"),		TEXT("小文字エータ") },	//	951
	{ 0x03B8,	("&theta;"),	TEXT("&theta;"),	TEXT("小文字シータ") },	//	952
	{ 0x03B9,	("&iota;"),		TEXT("&iota;"),		TEXT("小文字イオタ") },	//	953
	{ 0x03BA,	("&kappa;"),	TEXT("&kappa;"),	TEXT("小文字カッパ") },	//	954
	{ 0x03BB,	("&lambda;"),	TEXT("&lambda;"),	TEXT("小文字ラムダ") },	//	955
	{ 0x03BC,	("&mu;"),		TEXT("&mu;"),		TEXT("小文字ミュー") },	//	956
	{ 0x03BD,	("&nu;"),		TEXT("&nu;"),		TEXT("小文字ニュー") },	//	957
	{ 0x03BE,	("&xi;"),		TEXT("&xi;"),		TEXT("小文字グザイ") },	//	958
	{ 0x03BF,	("&omicron;"),	TEXT("&omicron;"),	TEXT("小文字オミクロン") },	//	959
	{ 0x03C0,	("&pi;"),		TEXT("&pi;"),		TEXT("小文字パイ") },	//	960
	{ 0x03C1,	("&rho;"),		TEXT("&rho;"),		TEXT("小文字ロー") },	//	961
	{ 0x03C2,	("&sigmaf;"),	TEXT("&sigmaf;"),	TEXT("小文字ファイナル・シグマ") },	//	962
	{ 0x03C3,	("&sigma;"),	TEXT("&sigma;"),	TEXT("小文字シグマ") },	//	963
	{ 0x03C4,	("&tau;"),		TEXT("&tau;"),		TEXT("小文字タウ") },	//	964
	{ 0x03C5,	("&upsilon;"),	TEXT("&upsilon;"),	TEXT("小文字ウプシロン") },	//	965
	{ 0x03C6,	("&phi;"),		TEXT("&phi;"),		TEXT("小文字ファイ") },	//	966
	{ 0x03C7,	("&chi;"),		TEXT("&chi;"),		TEXT("小文字カイ") },	//	967
	{ 0x03C8,	("&psi;"),		TEXT("&psi;"),		TEXT("小文字プサイ") },	//	968
	{ 0x03C9,	("&omega;"),	TEXT("&omega;"),	TEXT("小文字オメガ") },	//	969
	{ 0x03D1,	("&thetasym;"),	TEXT("&thetasym;"),	TEXT("小文字シータ・シンボル") },	//	977
	{ 0x03D2,	("&upsih;"),	TEXT("&upsih;"),	TEXT("ウプシロン（フック・シンボル付き）") },	//	978
	{ 0x03D6,	("&piv;"),		TEXT("&piv;"),		TEXT("パイ・シンボル") },	//	982
	{ 0x2002,	("&ensp;"),		TEXT("&ensp;"),		TEXT("n字幅の空白（半角空白）") },	//	8194
	{ 0x2003,	("&emsp;"),		TEXT("&emsp;"),		TEXT("m字幅の空白（全角空白）") },	//	8195
	{ 0x2009,	("&thinsp;"),	TEXT("&thinsp;"),	TEXT("せまい空白") },	//	8201
	{ 0x200C,	("&zwnj;"),		TEXT("&zwnj;"),		TEXT("幅なし非接続子") },	//	8204
	{ 0x200D,	("&zwj;"),		TEXT("&zwj;"),		TEXT("幅なし接続子") },	//	8205
	{ 0x200E,	("&lrm;"),		TEXT("&lrm;"),		TEXT("記述方向制御（左から右へ）") },	//	8206
	{ 0x200F,	("&rlm;"),		TEXT("&rlm;"),		TEXT("記述方向制御（右から左へ）") },	//	8207
	{ 0x2013,	("&ndash;"),	TEXT("&ndash;"),	TEXT("n字幅のダッシュ（半角ダッシュ）") },	//	8211
	{ 0x2014,	("&mdash;"),	TEXT("&mdash;"),	TEXT("m字幅のダッシュ（全角ダッシュ）") },	//	8212
	{ 0x2018,	("&lsquo;"),	TEXT("&lsquo;"),	TEXT("左シングル引用符") },	//	8216
	{ 0x2019,	("&rsquo;"),	TEXT("&rsquo;"),	TEXT("右シングル引用符") },	//	8217
	{ 0x201A,	("&sbquo;"),	TEXT("&sbquo;"),	TEXT("下付引用符") },	//	8218
	{ 0x201C,	("&ldquo;"),	TEXT("&ldquo;"),	TEXT("左ダブル引用符") },	//	8220
	{ 0x201D,	("&rdquo;"),	TEXT("&rdquo;"),	TEXT("右ダブル引用符（終）") },	//	8221
	{ 0x201E,	("&bdquo;"),	TEXT("&bdquo;"),	TEXT("下付ダブル引用符") },	//	8222
	{ 0x2020,	("&dagger;"),	TEXT("&dagger;"),	TEXT("ダガー") },	//	8224
	{ 0x2021,	("&Dagger;"),	TEXT("&Dagger;"),	TEXT("ダブルダガー") },	//	8225
	{ 0x2022,	("&bull;"),		TEXT("&bull;"),		TEXT("黒丸") },	//	8226
	{ 0x2026,	("&hellip;"),	TEXT("&hellip;"),	TEXT("三点リーダ") },	//	8230
	{ 0x2030,	("&permil;"),	TEXT("&permil;"),	TEXT("パーミル") },	//	8240
	{ 0x2032,	("&prime;"),	TEXT("&prime;"),	TEXT("分") },	//	8242
	{ 0x2033,	("&Prime;"),	TEXT("&Prime;"),	TEXT("秒") },	//	8243
	{ 0x2039,	("&lsaquo;"),	TEXT("&lsaquo;"),	TEXT("左山括弧（始）") },	//	8249
	{ 0x203A,	("&rsaquo;"),	TEXT("&rsaquo;"),	TEXT("右山括弧（終）") },	//	8250
	{ 0x203E,	("&oline;"),	TEXT("&oline;"),	TEXT("オーバーライン、論理否定記号") },	//	8254
	{ 0x2044,	("&frasl;"),	TEXT("&frasl;"),	TEXT("分数の斜線") },	//	8260
	{ 0x20AC,	("&euro;"),		TEXT("&euro;"),		TEXT("ユーロ記号") },	//	8364
	{ 0x2118,	("&weierp;"),	TEXT("&weierp;"),	TEXT("大文字のP（スクリプト体）、WeierstrassのP関数") },	//	8472
	{ 0x2111,	("&image;"),	TEXT("&image;"),	TEXT("大文字のI（ブラック体）、虚数部") },	//	8465
	{ 0x211C,	("&real;"),		TEXT("&real;"),		TEXT("大文字のR（ブラック体）、実数部") },	//	8476
	{ 0x2122,	("&trade;"),	TEXT("&trade;"),	TEXT("登録商標記号") },	//	8482
	{ 0x2135,	("&alefsym;"),	TEXT("&alefsym;"),	TEXT("アレフ") },	//	8501
	{ 0x2190,	("&larr;"),		TEXT("&larr;"),		TEXT("左向矢印") },	//	8592
	{ 0x2191,	("&uarr;"),		TEXT("&uarr;"),		TEXT("上向矢印") },	//	8593
	{ 0x2192,	("&rarr;"),		TEXT("&rarr;"),		TEXT("右向矢印") },	//	8594
	{ 0x2193,	("&darr;"),		TEXT("&darr;"),		TEXT("下向矢印") },	//	8595
	{ 0x2194,	("&harr;"),		TEXT("&harr;"),		TEXT("左右向矢印") },	//	8596
	{ 0x21B5,	("&crarr;"),	TEXT("&crarr;"),	TEXT("キャリッジリターン") },	//	8629
	{ 0x21D0,	("&lArr;"),		TEXT("&lArr;"),		TEXT("左向二重矢印") },	//	8656
	{ 0x21D1,	("&uArr;"),		TEXT("&uArr;"),		TEXT("上向二重矢印") },	//	8657
	{ 0x21D2,	("&rArr;"),		TEXT("&rArr;"),		TEXT("ならば（合意）") },	//	8658
	{ 0x21D3,	("&dArr;"),		TEXT("&dArr;"),		TEXT("下向二重矢印") },	//	8659
	{ 0x21D4,	("&hArr;"),		TEXT("&hArr;"),		TEXT("同値") },	//	8660
	{ 0x2200,	("&forall;"),	TEXT("&forall;"),	TEXT("すべての（普通限定子）") },	//	8704
	{ 0x2202,	("&part;"),		TEXT("&part;"),		TEXT("デル、ラウンドディー") },	//	8706
	{ 0x2203,	("&exist;"),	TEXT("&exist;"),	TEXT("存在する（存在限定子）") },	//	8707
	{ 0x2205,	("&empty;"),	TEXT("&empty;"),	TEXT("空集合") },	//	8709
	{ 0x2207,	("&nabla;"),	TEXT("&nabla;"),	TEXT("ナブラ") },	//	8711
	{ 0x2208,	("&isin;"),		TEXT("&isin;"),		TEXT("属する") },	//	8712
	{ 0x2209,	("&notin;"),	TEXT("&notin;"),	TEXT("要素の否定、元の否定") },	//	8713
	{ 0x220B,	("&ni;"),		TEXT("&ni;"),		TEXT("元として含む") },	//	8715
	{ 0x220F,	("&prod;"),		TEXT("&prod;"),		TEXT("n項の乗算") },	//	8719
	{ 0x2211,	("&sum;"),		TEXT("&sum;"),		TEXT("n項の加算") },	//	8721
	{ 0x2212,	("&minus;"),	TEXT("&minus;"),	TEXT("負符号、減算記号") },	//	8722
	{ 0x2217,	("&lowast;"),	TEXT("&lowast;"),	TEXT("アスタリスク演算子") },	//	8727
	{ 0x221A,	("&radic;"),	TEXT("&radic;"),	TEXT("根号") },	//	8730
	{ 0x221D,	("&prop;"),		TEXT("&prop;"),		TEXT("比例") },	//	8733
	{ 0x221E,	("&infin;"),	TEXT("&infin;"),	TEXT("無限大") },	//	8734
	{ 0x2220,	("&ang;"),		TEXT("&ang;"),		TEXT("角") },	//	8736
	{ 0x2227,	("&and;"),		TEXT("&and;"),		TEXT("及び（合接）") },	//	8743
	{ 0x2228,	("&or;"),		TEXT("&or;"),		TEXT("又は（離接）") },	//	8744
	{ 0x2229,	("&cap;"),		TEXT("&cap;"),		TEXT("共通集合") },	//	8745
	{ 0x222A,	("&cup;"),		TEXT("&cup;"),		TEXT("合併集合") },	//	8746
	{ 0x222B,	("&int;"),		TEXT("&int;"),		TEXT("積分記号") },	//	8747
	{ 0x2234,	("&there4;"),	TEXT("&there4;"),	TEXT("ゆえに") },	//	8756
	{ 0x223C,	("&sim;"),		TEXT("&sim;"),		TEXT("チルダ演算子") },	//	8764
	{ 0x2245,	("&cong;"),		TEXT("&cong;"),		TEXT("同形") },	//	8773
	{ 0x2248,	("&asymp;"),	TEXT("&asymp;"),	TEXT("近似的に等しい、同相") },	//	8776
	{ 0x2260,	("&ne;"),		TEXT("&ne;"),		TEXT("等号否定") },	//	8800
	{ 0x2261,	("&equiv;"),	TEXT("&equiv;"),	TEXT("常に等しい、合同") },	//	8801
	{ 0x2264,	("&le;"),		TEXT("&le;"),		TEXT("小なりまたは等しい") },	//	8804
	{ 0x2265,	("&ge;"),		TEXT("&ge;"),		TEXT("大なりまたは等しい") },	//	8805
	{ 0x2282,	("&sub;"),		TEXT("&sub;"),		TEXT("真部分集合") },	//	8834
	{ 0x2283,	("&sup;"),		TEXT("&sup;"),		TEXT("真部分集合（逆方向）") },	//	8835
	{ 0x2284,	("&nsub;"),		TEXT("&nsub;"),		TEXT("真部分集合の否定") },	//	8836
	{ 0x2286,	("&sube;"),		TEXT("&sube;"),		TEXT("部分集合") },	//	8838
	{ 0x2287,	("&supe;"),		TEXT("&supe;"),		TEXT("部分集合（逆方向）") },	//	8839
	{ 0x2295,	("&oplus;"),	TEXT("&oplus;"),	TEXT("直和") },	//	8853
	{ 0x2297,	("&otimes;"),	TEXT("&otimes;"),	TEXT("テンソル積") },	//	8855
	{ 0x22A5,	("&perp;"),		TEXT("&perp;"),		TEXT("垂直") },	//	8869
	{ 0x22C5,	("&sdot;"),		TEXT("&sdot;"),		TEXT("ドット演算子") },	//	8901
	{ 0x2308,	("&lceil;"),	TEXT("&lceil;"),	TEXT("左上限") },	//	8968
	{ 0x2309,	("&rceil;"),	TEXT("&rceil;"),	TEXT("右上限") },	//	8969
	{ 0x230A,	("&lfloor;"),	TEXT("&lfloor;"),	TEXT("左下限") },	//	8970
	{ 0x230B,	("&rfloor;"),	TEXT("&rfloor;"),	TEXT("右下限") },	//	8971
	{ 0x2329,	("&lang;"),		TEXT("&lang;"),		TEXT("左角括弧") },	//	9001
	{ 0x232A,	("&rang;"),		TEXT("&rang;"),		TEXT("右角括弧") },	//	9002
	{ 0x25CA,	("&loz;"),		TEXT("&loz;"),		TEXT("菱形") },	//	9674
	{ 0x2660,	("&spades;"),	TEXT("&spades;"),	TEXT("スペード") },	//	9824
	{ 0x2663,	("&clubs;"),	TEXT("&clubs;"),	TEXT("クラブ") },	//	9827
	{ 0x2665,	("&hearts;"),	TEXT("&hearts;"),	TEXT("ハート") },	//	9829
	{ 0x2666,	("&diams;"),	TEXT("&diams;"),	TEXT("ダイヤ") },	//	9830

*/

