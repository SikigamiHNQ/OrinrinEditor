/*! @file
	@brief 標準のシステム インクルード ファイルのインクルード ファイル、または参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイルを記述します。
	このファイルは stdafx.h です。
	@author	SikigamiHNQ
	@date	2011/00/00
*/

/*
Orinrin Editor : AsciiArt Story Editor for Japanese Only
Copyright (C) 2011 - 2012 Orinrin/SikigamiHNQ

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.
If not, see <http://www.gnu.org/licenses/>.
*/
//-------------------------------------------------------------------------------------------------

#pragma once

#define STRICT

//!	無効にする警告
#pragma warning( disable : 4100 )	//!<	引数は函数の本体部で 1 度も参照されません。
//#pragma warning( disable : 4101 )	//!<	ローカル変数は 1 度も使われていません。
#pragma warning( disable : 4201 )	//!<	非標準の拡張機能が使用されています
#pragma warning( disable : 4244 )	//!<	型変換における、データが失われる可能性について。
#pragma warning( disable : 4312 )	//!<	より大きいサイズへの型変換について
//#pragma warning( disable : 4995 )	//!<	名前が避けられた #pragma として記述されています。

#include "targetver.h"

#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")


//#define WIN32_LEAN_AND_MEAN		//	Windows ヘッダーから使用されていない部分を除外します。
// Windows ヘッダー ファイル:
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <windowsX.h>

//	シェルとか
#pragma comment(lib, "shell32.lib")

//	コモンダイヤログ
#include <commdlg.h>
#pragma comment(lib, "Comdlg32.lib")

#include <Commctrl.h>
#pragma comment(lib, "ComCtl32.lib")

#ifndef _ORCOLL

//	IMEの操作に使う
#include <imm.h>
#pragma comment(lib, "imm32.lib")

//	SQLite3
#include "sqlite3.h"
#pragma comment(lib, "sqlite3.lib")


#endif

// C ランタイム ヘッダー ファイル
#include <assert.h>

#define _CRTDBG_MAP_ALLOC	//	メモリリークチェク用
#include <stdlib.h>
#include <malloc.h>
#ifdef _DEBUG
#include <crtdbg.h>
#endif
#include <memory.h>
#include <tchar.h>

#define STRSAFE_NO_CB_FUNCTIONS
#include <strsafe.h>

#pragma warning( disable : 4995 )
#include <shlwapi.h>
#pragma warning( default : 4995 )
#pragma comment(lib, "shlwapi.lib")

//-------------------------------------------------------------------------------------------------

#pragma warning( disable : 4995 )	//	名前が避けられた #pragma として記述されています。
#include <vector>
#include <list>
#include <string>
#include <algorithm>
#pragma warning( default : 4995 )

using namespace	std;	//	このスコープ内ではstd::が省略できる
//-------------------------------------------------------------------------------------------------

//このアプリのGUID、特に意味はない
//	{66D3E881-972B-458B-935E-9E78B926B415}
static CONST GUID gcstGUID = { 0x66D3E881, 0x972B, 0x458B, { 0x93, 0x5E, 0x9E, 0x78, 0xB9, 0x26, 0xB4, 0x15 } };
//-------------------------------------------------------------------------------------------------

//機能ＯＫ
//	行の保持をリストにしてみる
//	ファイル開いた履歴
//	プロフ開いた履歴

#define WORK_LOG_OUT	

#define USE_NOTIFYICON	//	タスクトレイアイコンを有効

//	作成中の機能
//#define FIND_STRINGS	//	文字列検索機能
//#define PAGE_MULTISELECT//	頁一覧の複数選択
#define PAGE_DELAY_LOAD	//	ファイル読み込んだ時は頁展開しない、ひつようになったら開く

#define  FRAME_MLINE	//	枠パーツ複数行
#define AA_INVERSE		//	上下左右反転機能
#define ACCELERATOR_EDIT//	キーのアレ編集
//#define COPY_SWAP

#define DO_TRY_CATCH	//	例外対策してみる
//-------------------------------------------------------------------------------------------------

//	単なる識別名　externで外部参照されてる変数にくっつけておく
#define EXTERNED
//-------------------------------------------------------------------------------------------------

//	WORK_LOG_OUT

//#ifdef _DEBUG

#define TRACE(str,...)	OutputDebugStringPlus( GetLastError(), __FILE__, __LINE__, __FUNCTION__, str, __VA_ARGS__ )

#ifndef _ORCOLL
#define SQL_DEBUG(db)	SqlErrMsgView(db,__LINE__)
#endif

VOID	OutputDebugStringPlus( DWORD, LPSTR, INT, LPSTR, LPTSTR, ... );	//!<	
VOID	SqlErrMsgView( sqlite3 *, DWORD );

//#else
//	#define TRACE(x,...)
//
//  #ifndef _ORCOLL
//	#define SQL_DEBUG(db)
//  #endif
//#endif

#ifdef DO_TRY_CATCH
#define ETC_MSG(str,ret)	ExceptionMessage( str, __FUNCTION__, __LINE__, ret )

LRESULT	ExceptionMessage( LPCSTR, LPCSTR, UINT, LPARAM );
#endif

#define FREE(pp)	{if(pp){free(pp);pp=NULL;}}

//ステータスバー文字列追加マクロ
#define StatusBar_SetText(hwndSB,ipart,ptext)	(BOOLEAN)SNDMSG((hwndSB),SB_SETTEXT,ipart,(LPARAM)(LPCTSTR)(ptext))
//-------------------------------------------------------------------------------------------------

#define MIN_STRING	20
#define SUB_STRING	64
#define MAX_STRING	130
#define BIG_STRING	520

//	ウインドウサイズ
#define W_WIDTH		480
#define W_HEIGHT	400

//	ウインドウサイズ
#define WCL_WIDTH	480
#define WCL_HEIGHT	320

//	ドッキングサイズ
#define PLIST_DOCK	190
#define TMPL_DOCK	150
//-------------------------------------------------------------------------------------------------

#ifdef USE_NOTIFYICON
#define WMP_TRAYNOTIFYICON	(WM_APP+1)
#endif
#define WMP_BRUSH_TOGGLE	(WM_APP+2)
#define WMP_PREVIEW_CLOSE	(WM_APP+3)
//-------------------------------------------------------------------------------------------------

#define BASIC_COLOUR	RGB(0xF0,0xF0,0xF0)	//	Collector用
//-------------------------------------------------------------------------------------------------

#define USER_ITEM_FILE	TEXT("UserItem.ast")
#define USER_ITEM_MAX	16

#define AA_BRUSH_FILE	TEXT("aabrush.txt")		//	塗りつぶし用
#define AA_LIST_FILE	TEXT("aalist.txt")		//	壱行テンプレート

#define AA_MIRROR_FILE	TEXT("hantenX.txt")		//	左右反転参照
#define AA_UPSET_FILE	TEXT("hantenY.txt")		//	上下反転参照

#define MAA_FAVDB_FILE	TEXT("Favorite.qmt")	//	AAリスト用
#define MAA_TREE_CACHE	TEXT("TreeCache.qor")	//	ツリーの中身を取っておく・使わない

#define NAMELESS_DUMMY	TEXT("NoName0.txt")
#define NAME_DUMMY_NAME	TEXT("NoName")
#define NAME_DUMMY_EXT	TEXT("txt")

#define TEMPLATE_DIR	TEXT("Templates")
#define BACKUP_DIR		TEXT("BackUp")
#define PROFILE_DIR		TEXT("Profile")
//-------------------------------------------------------------------------------------------------

//	枠の数	20110707	枠を１０個に増やした
#define FRAME_MAX	20

//	設定について
#define INI_FILE		TEXT("Utuho.ini")
#define FRAME_INI_FILE	TEXT("Satori.ini")
#define MZCX_INI_FILE	TEXT("Koisi.ini")



#define INIT_LOAD		1
#define INIT_SAVE		0

#define WDP_MVIEW		1
#define WDP_PLIST		2
#define WDP_LNTMPL		3
#define WDP_BRTMPL		4
#define WDP_MAATPL		5
#define WDP_PREVIEW		6

#define FONTSZ_NORMAL	16
#define FONTSZ_REDUCE	12

#define VL_CLASHCOVER	0	//	起動したら１、閉じるときに０にして、CLASHを検知
#define VL_GROUP_UNDO	1	//	グループアンドゥ
#define VL_USE_UNICODE	2	//	ユニコードパディング
#define VL_LAYER_TRANS	3	//	レイヤボックス透明度
#define VL_RIGHT_SLIDE	4	//	右寄せる位置
#define VL_MAA_SPLIT	5	//	ＭＡＡのスプリットバーの位置
#define VL_MAA_LCLICK	6	//	ＭＡＡ一覧でクルッコしたときの標準動作
#define VL_UNILISTLAST	7	//	ユニコード一覧を閉じたときの位置
#define VL_MAATIP_VIEW	8	//	ＭＡＡのＡＡツールチップの表示するか
#define VL_MAATIP_SIZE	9	//	ＭＡＡのＡＡツールチップの文字サイズ・16か12
#define VL_LINETMP_CLM	10	//	壱行テンプレのカラム数
#define VL_BRUSHTMP_CLM	11	//	ブラシテンプレのカラム数
#define VL_UNIRADIX_HEX	12	//	ユニコード数値参照を１６進数にするかどうか
#define VL_BACKUP_INTVL	13	//	バックアップ感覚・デフォ５分くらい？
#define VL_BACKUP_MSGON	14	//	バックアップしたときのメッセージを表示するか？
#define VL_GRID_X_POS	15	//	グリッド線のＸドット数
#define VL_GRID_Y_POS	16	//	グリッド線のＹドット数
#define VL_MAA_TOPMOST	17	//	VIEWERＭＡＡを最前面・EDITORＭＡＡ窓ON/OFF
#define VL_R_RULER_POS	18	//	右ルーラの位置
#define VL_CRLF_CODE	19	//	改行コード：０したらば・非０ＹＹ
//#define VL_SPACE_VIEW	20	//	空白を表示するか　非０表示
#define VL_GRID_VIEW	21	//	グリッド表示するか
#define VL_R_RULER_VIEW	22	//	右ルーラ表示するか
#define VL_PAGETIP_VIEW	23	//	頁一覧のツールチップ表示するか
#define VL_PCOMBINE_NM	24	//	１なら統合Message無し
#define VL_PDIVIDE_NM	25	//	１なら分割Message無し
#define VL_PDELETE_NM	26	//	１なら削除Message無し
#define VL_MAASEP_STYLE	27	//	複数テンプレ・１なら区切り線スタイル
#define VL_USE_BALLOON	28	//	保存確認メッセージ表示するかどうか
#define VL_CLIPFILECNT	29
#define VL_PLS_LN_DOCK	30	//	頁一覧窓はくっつくか
//#define VL_BRUSH_DOCK	31	//	壱行・Brushテンプレ窓はくっつくか
#define VS_PROFILE_NAME	32	//	
#define VS_PAGE_FORMAT	33	//	頁番号挿入の文字列テンプレ
#define VL_SWAP_COPY	34	//	コピー標準をSJISにするか
#define VL_MAIN_SPLIT	35	//	メインのスプリットバーの右からの位置
#define VL_MAXIMISED	36	//	最大化で終わったか？
#define VL_DRT_LCLICK	37	//	ドラフトボードクリックのデフォ動作
#define VL_FIRST_READED	38	//	マニュアル読込した
#define VL_LAST_OPEN	39	//	最後に開いていたファイルを　０開く　１開かない　２毎回選択
#define VL_MAA_MCLICK	40	//	ＭＡＡ一覧でミドゥクルッコしたときの標準動作
#define VL_DRT_MCLICK	41	//	ドラフトボードクリックのデフォ動作
#define VS_FONT_NAME	42	//	メインのフォント名、ＭＳ Ｐゴシック
#define VL_WORKLOG		43	//	動作ログを出力するか

//増やしたら、函数内に取扱つくっておくこと

#define CLRV_BASICPEN	101
#define CLRV_BASICBK	102
#define CLRV_GRIDLINE	103
#define CLRV_CRLFMARK	104
#define CLRV_CANTSJIS	105
//-------------------------------------------------------------------------------------------------

//	ステータスバーの内
#define SB_MODIFY	0	//	変更
#define SB_OP_STYLE	1	//	オペ状況
#define SB_MOUSEPOS	2	//	マウスカーソル位置
#define SB_CURSOR	3	//	カーソル位置のドット値とか
#define SB_LAYER	4	//	レイヤボックスの位置
#define SB_BYTECNT	5	//	バイト数
#define SB_SELBYTE	6	//	選択範囲のバイト数
//-------------------------------------------------------------------------------------------------

//	窓番号
#define WND_MAIN	1
#define WND_MAAT	2
#define WND_PAGE	3
#define WND_LINE	4
#define WND_BRUSH	5
#define WND_TAIL	5	//	末端ダミー

//	モード
#define M_DESTROY	0
#define M_CREATE	1
#define M_EXISTENCE	2

#define MAA_DEFAULT		0xFF
#define MAA_SUBDEFAULT	0xFE
#define MAA_INSERT		0
#define MAA_INTERRUPT	1
#define MAA_LAYERED		2
#define MAA_UNICLIP		3
#define MAA_SJISCLIP	4
#define MAA_DRAUGHT		5

#define LASTOPEN_DO		0
#define LASTOPEN_NON	1
#define LASTOPEN_ASK	2

//	指示コード・かぶらないように
#define D_SJIS		0x00	//	シフトJIS
#define D_UNI		0x01	//	ユニコード
#define D_SQUARE	0x02	//	矩形選択
#define D_INVISI	0x10	//	不可視状態の特別処理
#define D_RENAME	0x80	//	名前を付けて保存
//-------------------------------------------------------------------------------------------------

//	ImgCtl.dllに準拠
#define ISAVE_BMP	0x1
//#define ISAVE_JPEG	0x2
#define ISAVE_PNG	0x3
//-------------------------------------------------------------------------------------------------

#define CLIP_FORMAT	TEXT("ORINRIN_EDITOR_STYLE")	//!<	識別用・特に意味はない
#define CLIP_SQUARE	TEXT("MSDEVColumnSelect")	//!<	矩形選択識別子・VCのやつ
//-------------------------------------------------------------------------------------------------


//!	キーコード割り当て
#define	VK_0	0x30
#define	VK_1	0x31
#define	VK_2	0x32
#define	VK_3	0x33
#define	VK_4	0x34
#define	VK_5	0x35
#define	VK_6	0x36
#define	VK_7	0x37
#define	VK_8	0x38
#define	VK_9	0x39

//	必要なボタンに名前付けをしておく
#define	VK_A	0x41
#define	VK_B	0x42
#define	VK_C	0x43
#define	VK_D	0x44
#define	VK_E	0x45
#define	VK_F	0x46
#define	VK_G	0x47
#define	VK_H	0x48
#define	VK_I	0x49
#define	VK_J	0x4A
#define	VK_K	0x4B
#define	VK_L	0x4C
#define	VK_M	0x4D
#define	VK_N	0x4E
#define	VK_O	0x4F
#define	VK_P	0x50
#define	VK_Q	0x51
#define	VK_R	0x52
#define	VK_S	0x53
#define	VK_T	0x54
#define	VK_U	0x55
#define	VK_V	0x56
#define	VK_W	0x57
#define	VK_X	0x58
#define	VK_Y	0x59
#define	VK_Z	0x5A
//-------------------------------------------------------------------------------------------------
