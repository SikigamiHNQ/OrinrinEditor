/*! @file
	@brief ツールバーを作ります
	このファイルは ToolBar.cpp です。
	@author	SikigamiHNQ
	@date	2011/09/11
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

#include "stdafx.h"
#include "OrinrinEditor.h"
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------

static  HWND	ghRebarWnd;			//!<	リバー

static  HWND	ghMainTBWnd;		//!<	メインツールバーのウインドウハンドル
static  HWND	ghEditTBWnd;		//!<	編集ツールバーのウインドウハンドル
static  HWND	ghInsertTBWnd;		//!<	挿入ツールバーのウインドウハンドル
static  HWND	ghLayoutTBWnd;		//!<	整形ツールバーのウインドウハンドル
static  HWND	ghViewTBWnd;		//!<	表示ツールバーのウインドウハンドル

static HIMAGELIST	ghMainImgLst;	//!<	
static HIMAGELIST	ghEditImgLst;	//!<	
static HIMAGELIST	ghInsertImgLst;	//!<	
static HIMAGELIST	ghLayoutImgLst;	//!<	
static HIMAGELIST	ghViewImgLst;	//!<	

static WNDPROC	gpfOrigTBProc;		//!<	

extern HMENU	ghHistyMenu;		//	履歴表示する部分・動的に内容作成せないかん
//-------------------------------------------------------------------------------------------------

static LRESULT	CALLBACK gpfToolbarProc( HWND, UINT, WPARAM, LPARAM );
//-------------------------------------------------------------------------------------------------

#define TBT_STRING	32

//	ファイル
#define TB_MAIN_ITEMS	5
static  TBBUTTON	gstMainTBInfo[] = {
	{  0,	IDM_NEWFILE,		TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,						{0, 0}, 0, 0  },	//	
	{  1,	IDM_OPEN,			TBSTATE_ENABLED,	TBSTYLE_DROPDOWN | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{  2,	IDM_OVERWRITESAVE,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,						{0, 0}, 0, 0  },	//	
	{  0,	0,					TBSTATE_ENABLED,	TBSTYLE_SEP,							{0, 0}, 0, 0  },
	{  3,	IDM_GENERAL_OPTION,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,						{0, 0}, 0, 0  } 	//	
};	//	内容変更したら、ツールバー文字列の設定とかも変更セヨ

static CONST TCHAR	gatMainTBtext[TB_MAIN_ITEMS][TBT_STRING] = { 
	{ TEXT("新規作成") },	{ TEXT("ファイル開く") },	{ TEXT("上書き保存") },	{ TEXT("") },
	{ TEXT("一般設定") }
};


//	編集
#define TB_EDIT_ITEMS	21
static  TBBUTTON	gstEditTBInfo[] = {
	{  0,	IDM_UNDO,			TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },	//	アンドゥ
	{  1,	IDM_REDO,			TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },	//	リドゥ
	{  0,	0,					TBSTATE_ENABLED,	TBSTYLE_SEP,						{0, 0}, 0, 0  },
	{  2,	IDM_CUT,			TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },	//	切り取り
	{  3,	IDM_COPY,			TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },	//	コピー
	{  4,	IDM_PASTE,			TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },	//	貼付
	{  5,	IDM_DELETE,			TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },	//	削除
	{  0,	0,					TBSTATE_ENABLED,	TBSTYLE_SEP,						{0, 0}, 0, 0  },
	{  6,	IDM_SJISCOPY,		TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },	//	ＳＪＩＳコピー
	{  7,	IDM_SJISCOPY_ALL,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },	//	頁全体SJISコピー
	{  0,	0,					TBSTATE_ENABLED,	TBSTYLE_SEP,						{0, 0}, 0, 0  },
	{  8,	IDM_ALLSEL,			TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },	//	全選択
	{  9,	IDM_SQSELECT,		TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },	//	矩形選択
	{  0,	0,					TBSTATE_ENABLED,	TBSTYLE_SEP,						{0, 0}, 0, 0  },
	{ 10,	IDM_LAYERBOX,		TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },	//	レイヤボックス起動
	{  0,	0,					TBSTATE_ENABLED,	TBSTYLE_SEP,						{0, 0}, 0, 0  },
	{ 11,	IDM_PAGEL_DIVIDE,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },	//	ページ分割
	{  0,	0,					TBSTATE_ENABLED,	TBSTYLE_SEP,						{0, 0}, 0, 0  },
	{ 12,	IDM_EXTRACTION_MODE,TBSTATE_ENABLED,	TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	抽出モード
	{  0,	0,					TBSTATE_ENABLED,	TBSTYLE_SEP,						{0, 0}, 0, 0  },
	{ 13,	IDM_UNICODE_TOGGLE,	TBSTATE_ENABLED,	TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	ユニコード使用ON/OFF
};

static CONST TCHAR	gatEditTBtext[TB_EDIT_ITEMS][TBT_STRING] = { 
	{ TEXT("元に戻す") },	{ TEXT("やり直し") },	{ TEXT("") },
	{ TEXT("切り取り") },	{ TEXT("コピー") },	{ TEXT("貼付") },	{ TEXT("削除") },	{ TEXT("") },
	{ TEXT("SJISコピー") },	{ TEXT("全体をSJISコピー") },	{ TEXT("") },
	{ TEXT("全選択") },	{ TEXT("矩形選択") },	{ TEXT("") },
	{ TEXT("レイヤボックス") },	{ TEXT("") },
	{ TEXT("次の行以降を新頁に分割") },	{ TEXT("") },
	{ TEXT("部分抽出モード") },	{ TEXT("") },
	{ TEXT("ユニコード空白を使う") }
};


//	挿入
#define  TB_INSERT_ITEMS	6
static  TBBUTTON	gstInsertTBInfo[] = {
	{  0, IDM_IN_UNI_SPACE,		TBSTATE_ENABLED,	TBSTYLE_DROPDOWN | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{  1, IDM_INSTAG_COLOUR,	TBSTATE_ENABLED,	TBSTYLE_DROPDOWN | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{  2, IDM_FRMINSBOX_OPEN,	TBSTATE_ENABLED,	TBSTYLE_DROPDOWN | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{  3, IDM_USERINS_NA,		TBSTATE_ENABLED,	TBSTYLE_DROPDOWN | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{  0, 0,					TBSTATE_ENABLED,	TBSTYLE_SEP,							{0, 0}, 0, 0  },
	{  4, IDM_MOZI_SCR_OPEN,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,						{0, 0}, 0, 0  } 	//	
};

static CONST TCHAR	gatInsertTBtext[TB_INSERT_ITEMS][TBT_STRING] = { 
	{ TEXT("ユニコード空白") },	{ TEXT("色変更タグ") },	{ TEXT("枠") },	{ TEXT("ユーザ定義") },	{ TEXT("") },
	{ TEXT("文字ＡＡ変換ボックス") }
};


//	整形
#define  TB_LAYOUT_ITEMS	19
static  TBBUTTON	gstLayoutTBInfo[] = {
	{  0, IDM_RIGHT_GUIDE_SET,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	右揃え線
	{  1, IDM_INS_TOPSPACE,		TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	行頭に空白挿入
	{  0, 0,					TBSTATE_ENABLED,	TBSTYLE_SEP,		{0, 0}, 0, 0  },
	{  2, IDM_DEL_TOPSPACE,		TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	行頭空白削除
	{  3, IDM_DEL_LASTSPACE,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	行末空白削除
	{  4, IDM_DEL_LASTLETTER,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	行末文字削除
	{  0, 0,					TBSTATE_ENABLED,	TBSTYLE_SEP,		{0, 0}, 0, 0  },
	{ 10, IDM_MIRROR_INVERSE,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{ 11, IDM_UPSET_INVERSE,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{  0, 0,					TBSTATE_ENABLED,	TBSTYLE_SEP,		{0, 0}, 0, 0  },
	{  5, IDM_RIGHT_SLIDE,		TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	右に寄せる
	{  6, IDM_INCREMENT_DOT,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	１ドット増やす
	{  7, IDM_DECREMENT_DOT,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	１ドット減らす
	{  0, 0,					TBSTATE_ENABLED,	TBSTYLE_SEP,		{0, 0}, 0, 0  },
	{  8, IDM_INCR_DOT_LINES,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	１ドット右へ
	{  9, IDM_DECR_DOT_LINES,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	１ドット左へ
	{  0, 0,					TBSTATE_ENABLED,	TBSTYLE_SEP,		{0, 0}, 0, 0  },
	{ 12, IDM_DOT_SPLIT_LEFT,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	真ん中から１ドット右へ
	{ 13, IDM_DOT_SPLIT_RIGHT,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  } 	//	真ん中から１ドット左へ
};

static CONST TCHAR	gatLayoutTBtext[TB_LAYOUT_ITEMS][TBT_STRING] = { 
	{ TEXT("右揃え線") },	{ TEXT("行頭に全角空白追加") },	{ TEXT("") },
	{ TEXT("行頭空白削除") },	{ TEXT("行末空白削除") },	{ TEXT("行末文字削除") },	{ TEXT("") },
	{ TEXT("左右反転") },	{ TEXT("上下反転") },	{ TEXT("") },
	{ TEXT("右に寄せる") },	{ TEXT("１ドット増やす") },	{ TEXT("１ドット減らす") },	{ TEXT("") },
	{ TEXT("全体を１ドット右へ") },	{ TEXT("全体を１ドット左へ") },	{ TEXT("") },
	{ TEXT("指定位置から左に狭める") },	{ TEXT("指定位置から右へ広げる") }
};

//	表示
#define TB_VIEW_ITEMS	5
static  TBBUTTON	gstViewTBInfo[] = {
	{  0, IDM_UNI_PALETTE,			TBSTATE_ENABLED,	TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{  1, IDM_TRACE_MODE_ON,		TBSTATE_ENABLED,	TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{  2, IDM_ON_PREVIEW,			TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },	//	
	{  3, IDM_DRAUGHT_OPEN,			TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },	//	
	{  4, IDM_MAA_THUMBNAIL_OPEN,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  } 	//	
};

static CONST TCHAR	gatViewTBtext[TB_VIEW_ITEMS][TBT_STRING] = { 
	{ TEXT("ユニコード表") },	{ TEXT("トレスモード") },	{ TEXT("プレビュー") },	{ TEXT("ドラフトボード") },	{ TEXT("複数行サムネイル") }
};

//全体
#define TB_BAND_COUNT	5
static CONST REBARLAYOUTINFO	gcstReBarDef[] = {
	{ IDTB_MAIN_TOOLBAR,   180, RBBS_GRIPPERALWAYS | RBBS_CHILDEDGE               },	//	メイン
	{ IDTB_EDIT_TOOLBAR,   450, RBBS_GRIPPERALWAYS | RBBS_CHILDEDGE               },	//	編集
	{ IDTB_INSERT_TOOLBAR, 280, RBBS_GRIPPERALWAYS | RBBS_CHILDEDGE | RBBS_BREAK  },	//	挿入
	{ IDTB_LAYOUT_TOOLBAR, 310, RBBS_GRIPPERALWAYS | RBBS_CHILDEDGE               },	//	整形
	{ IDTB_VIEW_TOOLBAR,   140, RBBS_GRIPPERALWAYS | RBBS_CHILDEDGE               } 	//	表示
};	//	初期構成
//-------------------------------------------------------------------------------------------------

/*!
	ツールバーサブクラス
	WindowsXPで、ツールバーのボタン上でマウスの左ボタンを押したまま右ボタンを押すと、
	それ以降のマウス操作を正常に受け付けなくなる。それの対策
	@param[in]	hWnd	親ウインドウのハンドル
	@param[in]	msg		ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@return	処理した結果とか
*/
LRESULT CALLBACK gpfToolbarProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		case WM_CONTEXTMENU:
			TRACE( TEXT("TOOLBAR CONTEXT[%X]"), hWnd );
			break;

		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			if( SendMessage(hWnd, TB_GETHOTITEM, 0, 0) >= 0 ){	ReleaseCapture(   );	}
			return 0;
	}

	return CallWindowProc( gpfOrigTBProc, hWnd, msg, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------


/*!
	ツールバーをつくる・リバーかも
	@param[in]	hWnd	親ウインドウのハンドル
	@param[in]	lcInst	アプリの実存
*/
VOID ToolBarCreate( HWND hWnd, HINSTANCE lcInst )
{
//	TBADDBITMAP	stToolBmp;
	TCHAR	atBuff[MAX_STRING];

	UINT			ici, resnum, d;
	REBARINFO		stRbrInfo;
	REBARBANDINFO	stRbandInfo;
	REBARLAYOUTINFO	stInfo[TB_BAND_COUNT];

	HBITMAP	hImg, hMsq;

//リバー
	ghRebarWnd = CreateWindowEx( WS_EX_TOOLWINDOW, REBARCLASSNAME, NULL,	//	RBS_VARHEIGHT
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | RBS_BANDBORDERS | RBS_DBLCLKTOGGLE | CCS_NODIVIDER | CCS_TOP,
		0, 0, 0, 0, hWnd, (HMENU)IDRB_REBAR, lcInst, NULL);

	ZeroMemory( &stRbrInfo, sizeof(REBARINFO) );
	stRbrInfo.cbSize = sizeof(REBARINFO);
	SendMessage( ghRebarWnd, RB_SETBARINFO, 0, (LPARAM)&stRbrInfo );

	//	初期値
	ZeroMemory( stInfo, sizeof(stInfo) );
	for( d = 0; TB_BAND_COUNT > d; d++ )
	{
		stInfo[d].wID    = gcstReBarDef[d].wID;
		stInfo[d].cx     = gcstReBarDef[d].cx;
		stInfo[d].fStyle = gcstReBarDef[d].fStyle;
	}
	//	保存状態読込
	InitToolBarLayout( INIT_LOAD, TB_BAND_COUNT, stInfo );
	//	データ有れば書き換わる、なかったら初期値そのまま

//メインツールバー
	ghMainTBWnd = CreateWindowEx( 0, TOOLBARCLASSNAME, TEXT("maintb"),
		WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS | CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER
		, 0, 0, 0, 0, ghRebarWnd, (HMENU)IDTB_MAIN_TOOLBAR, lcInst, NULL);
	//	自動ツールチップスタイルを追加
	SendMessage( ghMainTBWnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_EX_DRAWDDARROWS );

	ghMainImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 4, 1 );
	resnum = IDBMPQ_MAIN_TB_FIRST;
	for( ici = 0; 4 > ici; ici++ )
	{
		hImg = LoadBitmap( lcInst, MAKEINTRESOURCE( (resnum++) ) );
		hMsq = LoadBitmap( lcInst, MAKEINTRESOURCE( (resnum++) ) );
		ImageList_Add( ghMainImgLst, hImg, hMsq );	//	イメージリストにイメージを追加
		DeleteBitmap( hImg );	DeleteBitmap( hMsq );
	}
	SendMessage( ghMainTBWnd, TB_SETIMAGELIST, 0, (LPARAM)ghMainImgLst );
	SendMessage( ghMainTBWnd, TB_SETBUTTONSIZE, 0, MAKELPARAM(16,16) );

	SendMessage( ghMainTBWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );
	//	ツールチップ文字列を設定・ボタンテキストがツールチップになる
	StringCchCopy( atBuff, MAX_STRING, gatMainTBtext[0] );	gstMainTBInfo[0].iString = SendMessage( ghMainTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatMainTBtext[1] );	gstMainTBInfo[1].iString = SendMessage( ghMainTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatMainTBtext[2] );	gstMainTBInfo[2].iString = SendMessage( ghMainTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatMainTBtext[4] );	gstMainTBInfo[4].iString = SendMessage( ghMainTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );

	SendMessage( ghMainTBWnd, TB_ADDBUTTONS, (WPARAM)TB_MAIN_ITEMS, (LPARAM)&gstMainTBInfo );	//	ツールバーにボタンを挿入

	SendMessage( ghMainTBWnd, TB_AUTOSIZE, 0, 0 );	//	ボタンのサイズに合わせてツールバーをリサイズ
//	InvalidateRect( ghMainTBWnd , NULL, TRUE );	//	クライヤント全体を再描画する

	//	ツールバーサブクラス化
	gpfOrigTBProc = SubclassWindow( ghMainTBWnd, gpfToolbarProc );



//編集ツールバー
	ghEditTBWnd = CreateWindowEx( 0, TOOLBARCLASSNAME, TEXT("edittb"),
		WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS | CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER
		, 0, 0, 0, 0, ghRebarWnd, (HMENU)IDTB_EDIT_TOOLBAR, lcInst, NULL);
	//	自動ツールチップスタイルを追加
	SendMessage( ghEditTBWnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS );

	//stToolBmp.hInst = HINST_COMMCTRL;
	//stToolBmp.nID   = IDB_STD_SMALL_COLOR;
	//SendMessage( ghEditTBWnd, TB_ADDBITMAP, 0, (LPARAM)&stToolBmp );
	ghEditImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 14, 1 );
	resnum = IDBMPQ_EDIT_TB_FIRST;
	for( ici = 0; 14 > ici; ici++ )
	{
		hImg = LoadBitmap( lcInst, MAKEINTRESOURCE( (resnum++) ) );
		hMsq = LoadBitmap( lcInst, MAKEINTRESOURCE( (resnum++) ) );
		ImageList_Add( ghEditImgLst, hImg, hMsq );	//	イメージリストにイメージを追加
		DeleteBitmap( hImg );	DeleteBitmap( hMsq );
	}
	SendMessage( ghEditTBWnd, TB_SETIMAGELIST, 0, (LPARAM)ghEditImgLst );

	SendMessage( ghEditTBWnd, TB_SETBUTTONSIZE, 0, MAKELPARAM(16,16) );

	SendMessage( ghEditTBWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );
	//	ツールチップ文字列を設定・ボタンテキストがツールチップになる
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[ 0] );	gstEditTBInfo[ 0].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[ 1] );	gstEditTBInfo[ 1].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[ 3] );	gstEditTBInfo[ 3].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[ 4] );	gstEditTBInfo[ 4].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[ 5] );	gstEditTBInfo[ 5].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[ 6] );	gstEditTBInfo[ 6].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[ 8] );	gstEditTBInfo[ 8].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[ 9] );	gstEditTBInfo[ 9].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[11] );	gstEditTBInfo[11].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[12] );	gstEditTBInfo[12].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[14] );	gstEditTBInfo[14].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[16] );	gstEditTBInfo[16].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[18] );	gstEditTBInfo[18].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatEditTBtext[20] );	gstEditTBInfo[20].iString = SendMessage( ghEditTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	SendMessage( ghEditTBWnd, TB_ADDBUTTONS, (WPARAM)TB_EDIT_ITEMS, (LPARAM)&gstEditTBInfo );	//	ツールバーにボタンを挿入
	SendMessage( ghEditTBWnd, TB_AUTOSIZE, 0, 0 );	//	ボタンのサイズに合わせてツールバーをリサイズ

	//	ツールバーサブクラス化
	gpfOrigTBProc = SubclassWindow( ghEditTBWnd, gpfToolbarProc );


//挿入ツールバー
	ghInsertTBWnd = CreateWindowEx( 0, TOOLBARCLASSNAME, TEXT("inserttb"),
		WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS | CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER,
		0, 0, 0, 0, ghRebarWnd, (HMENU)IDTB_INSERT_TOOLBAR, lcInst, NULL);
	//	自動ツールチップスタイルを追加	ドロップダウンメニューを有効にする
	SendMessage( ghInsertTBWnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_EX_DRAWDDARROWS );

	//stToolBmp.hInst = HINST_COMMCTRL;
	//stToolBmp.nID   = IDB_HIST_SMALL_COLOR;
	//SendMessage( ghInsertTBWnd, TB_ADDBITMAP, 0, (LPARAM)&stToolBmp );
	ghInsertImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 5, 1 );
	resnum = IDBMPQ_INSERT_TB_FIRST;
	for( ici = 0; 4 > ici; ici++ )
	{
		hImg = LoadBitmap( lcInst, MAKEINTRESOURCE( (resnum++) ) );
		hMsq = LoadBitmap( lcInst, MAKEINTRESOURCE( (resnum++) ) );
		ImageList_Add( ghInsertImgLst, hImg, hMsq );	//	イメージリストにイメージを追加
		DeleteBitmap( hImg );	DeleteBitmap( hMsq );
	}
	hImg = LoadBitmap( lcInst, MAKEINTRESOURCE( IDBMP_MOZI_SCRIPT ) );
	hMsq = LoadBitmap( lcInst, MAKEINTRESOURCE( IDBMQ_MOZI_SCRIPT ) );
	ImageList_Add( ghInsertImgLst, hImg, hMsq );	//	イメージリストにイメージを追加
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	SendMessage( ghInsertTBWnd, TB_SETIMAGELIST, 0, (LPARAM)ghInsertImgLst );

	SendMessage( ghInsertTBWnd, TB_SETBUTTONSIZE, 0, MAKELPARAM(16,16) );

	SendMessage( ghInsertTBWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );
	//	ツールチップ文字列を設定・ボタンテキストがツールチップになる
	StringCchCopy( atBuff, MAX_STRING, gatInsertTBtext[ 0] );	gstInsertTBInfo[ 0].iString = SendMessage( ghInsertTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatInsertTBtext[ 1] );	gstInsertTBInfo[ 1].iString = SendMessage( ghInsertTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatInsertTBtext[ 2] );	gstInsertTBInfo[ 2].iString = SendMessage( ghInsertTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatInsertTBtext[ 3] );	gstInsertTBInfo[ 3].iString = SendMessage( ghInsertTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatInsertTBtext[ 5] );	gstInsertTBInfo[ 5].iString = SendMessage( ghInsertTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	SendMessage( ghInsertTBWnd , TB_ADDBUTTONS, (WPARAM)TB_INSERT_ITEMS, (LPARAM)&gstInsertTBInfo );	//	ツールバーにボタンを挿入
	SendMessage( ghInsertTBWnd, TB_AUTOSIZE, 0, 0 );	//	ボタンのサイズに合わせてツールバーをリサイズ

	//	ツールバーサブクラス化
	gpfOrigTBProc = SubclassWindow( ghInsertTBWnd, gpfToolbarProc );


//整形ツールバー
	ghLayoutTBWnd = CreateWindowEx( 0, TOOLBARCLASSNAME, TEXT("layouttb"),
		WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS | CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER,
		0, 0, 0, 0, ghRebarWnd, (HMENU)IDTB_LAYOUT_TOOLBAR, lcInst, NULL);
	//	自動ツールチップスタイルを追加
	SendMessage( ghLayoutTBWnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS );

	//stToolBmp.hInst = HINST_COMMCTRL;
	//stToolBmp.nID   = IDB_VIEW_SMALL_COLOR;
	//SendMessage( ghLayoutTBWnd, TB_ADDBITMAP, 0, (LPARAM)&stToolBmp );
	ghLayoutImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 14, 1 );
	resnum = IDBMPQ_LAYOUT_TB_FIRST;
	for( ici = 0; 12 > ici; ici++ )
	{
		hImg = LoadBitmap( lcInst, MAKEINTRESOURCE( (resnum++) ) );
		hMsq = LoadBitmap( lcInst, MAKEINTRESOURCE( (resnum++) ) );
		ImageList_Add( ghLayoutImgLst, hImg, hMsq );	//	イメージリストにイメージを追加
		DeleteBitmap( hImg );	DeleteBitmap( hMsq );
	}
	hImg = LoadBitmap( lcInst, MAKEINTRESOURCE( IDBMP_SPLIT_LEFT ) );
	hMsq = LoadBitmap( lcInst, MAKEINTRESOURCE( IDBMQ_SPLIT_LEFT ) );
	ImageList_Add( ghLayoutImgLst, hImg, hMsq );	//	イメージリストにイメージを追加
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( lcInst, MAKEINTRESOURCE( IDBMP_SPLIT_RIGHT ) );
	hMsq = LoadBitmap( lcInst, MAKEINTRESOURCE( IDBMQ_SPLIT_RIGHT ) );
	ImageList_Add( ghLayoutImgLst, hImg, hMsq );	//	イメージリストにイメージを追加
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	SendMessage( ghLayoutTBWnd, TB_SETIMAGELIST, 0, (LPARAM)ghLayoutImgLst );


	SendMessage( ghLayoutTBWnd, TB_SETBUTTONSIZE, 0, MAKELPARAM(16,16) );

	SendMessage( ghLayoutTBWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );
	//	ツールチップ文字列を設定・ボタンテキストがツールチップになる
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[ 0] );	gstLayoutTBInfo[ 0].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[ 1] );	gstLayoutTBInfo[ 1].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[ 3] );	gstLayoutTBInfo[ 3].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[ 4] );	gstLayoutTBInfo[ 4].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[ 5] );	gstLayoutTBInfo[ 5].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[ 7] );	gstLayoutTBInfo[ 7].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[ 8] );	gstLayoutTBInfo[ 8].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[10] );	gstLayoutTBInfo[10].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[11] );	gstLayoutTBInfo[11].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[12] );	gstLayoutTBInfo[12].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[14] );	gstLayoutTBInfo[14].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[15] );	gstLayoutTBInfo[15].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[17] );	gstLayoutTBInfo[17].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, gatLayoutTBtext[18] );	gstLayoutTBInfo[18].iString = SendMessage( ghLayoutTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );

	SendMessage( ghLayoutTBWnd , TB_ADDBUTTONS, (WPARAM)TB_LAYOUT_ITEMS, (LPARAM)&gstLayoutTBInfo );	//	ツールバーにボタンを挿入

	SendMessage( ghLayoutTBWnd, TB_AUTOSIZE, 0, 0 );	//	ボタンのサイズに合わせてツールバーをリサイズ

	//	ツールバーサブクラス化
	gpfOrigTBProc = SubclassWindow( ghLayoutTBWnd, gpfToolbarProc );


//表示ツールバー
	ghViewTBWnd = CreateWindowEx( 0, TOOLBARCLASSNAME, TEXT("viewtb"),
		WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS | CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER,
		0, 0, 0, 0, ghRebarWnd, (HMENU)IDTB_VIEW_TOOLBAR, lcInst, NULL);
	//	自動ツールチップスタイルを追加
	SendMessage( ghViewTBWnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS );

//	hImg = ImageList_LoadBitmap( lcInst, MAKEINTRESOURCE(IDBMP_UNIPALETTE), 16, 0, RGB(0xFF,0x00,0xFF) );
//	こうすればMASKまで一発生成できる・２５６色用っぽい
	ghViewImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 5, 1 );
	resnum = IDBMPQ_VIEW_TB_FIRST;
	for( ici = 0; 5 > ici; ici++ )
	{
		hImg = LoadBitmap( lcInst, MAKEINTRESOURCE( (resnum++) ) );
		hMsq = LoadBitmap( lcInst, MAKEINTRESOURCE( (resnum++) ) );
		ImageList_Add( ghViewImgLst, hImg, hMsq );	//	イメージリストにイメージを追加
		DeleteBitmap( hImg );	DeleteBitmap( hMsq );
	}
	SendMessage( ghViewTBWnd, TB_SETIMAGELIST, 0, (LPARAM)ghViewImgLst );

	SendMessage( ghViewTBWnd, TB_SETBUTTONSIZE, 0, MAKELPARAM(16,16) );

	SendMessage( ghViewTBWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );
	//	ツールチップ文字列を設定・ボタンテキストがツールチップになる
	StringCchCopy( atBuff, MAX_STRING , gatViewTBtext[ 0] );	gstViewTBInfo[ 0].iString = SendMessage( ghViewTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatViewTBtext[ 1] );	gstViewTBInfo[ 1].iString = SendMessage( ghViewTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatViewTBtext[ 2] );	gstViewTBInfo[ 2].iString = SendMessage( ghViewTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatViewTBtext[ 3] );	gstViewTBInfo[ 3].iString = SendMessage( ghViewTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING , gatViewTBtext[ 4] );	gstViewTBInfo[ 4].iString = SendMessage( ghViewTBWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );

	SendMessage( ghViewTBWnd, TB_ADDBUTTONS, (WPARAM)TB_VIEW_ITEMS, (LPARAM)&gstViewTBInfo );	//	ツールバーにボタンを挿入
	SendMessage( ghViewTBWnd, TB_AUTOSIZE, 0, 0 );	//	ボタンのサイズに合わせてツールバーをリサイズ

	//	ツールバーサブクラス化
	gpfOrigTBProc = SubclassWindow( ghViewTBWnd, gpfToolbarProc );


	//配置
	ZeroMemory( &stRbandInfo, sizeof(REBARBANDINFO) );
	stRbandInfo.cbSize     = sizeof(REBARBANDINFO);
	stRbandInfo.fMask      = RBBIM_TEXT | RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE | RBBIM_ID;
	stRbandInfo.cxMinChild = 0;
	stRbandInfo.cyMinChild = 25;

	for( d = 0; TB_BAND_COUNT > d; d++ )
	{
		switch( stInfo[d].wID )
		{
			case IDTB_MAIN_TOOLBAR:
				stRbandInfo.lpText    = TEXT("ファイル");
				stRbandInfo.hwndChild = ghMainTBWnd;
				break;

			case IDTB_EDIT_TOOLBAR:
				stRbandInfo.lpText    = TEXT("編集");
				stRbandInfo.hwndChild = ghEditTBWnd;
				break;

			case IDTB_INSERT_TOOLBAR:
				stRbandInfo.lpText    = TEXT("挿入");
				stRbandInfo.hwndChild = ghInsertTBWnd;
				break;

			case IDTB_LAYOUT_TOOLBAR:
				stRbandInfo.lpText    = TEXT("整形");
				stRbandInfo.hwndChild = ghLayoutTBWnd;
				break;

			case IDTB_VIEW_TOOLBAR:
				stRbandInfo.lpText    = TEXT("表示");
				stRbandInfo.hwndChild = ghViewTBWnd;
				break;

			default:	continue;	break;
		}

		stRbandInfo.wID       = stInfo[d].wID;
		stRbandInfo.cx        = stInfo[d].cx;
		stRbandInfo.fStyle    = stInfo[d].fStyle;

		SendMessage( ghRebarWnd, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&stRbandInfo );
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ツールバーのインフォチップのアクセルキーテキストを変更
	@param[in]	pstAccel	内容テーブル・無い時はNULL
	@param[in]	iEntry		テーブルのエントリ数
	@return		HRESULT	終了状態コード
*/
HRESULT ToolBarInfoChange( LPACCEL pstAccel, INT iEntry )
{
	TCHAR	atText[MAX_STRING];
	TBBUTTONINFO	stButtonInfo;

	INT	i;

	ZeroMemory( &stButtonInfo, sizeof(TBBUTTONINFO) );
	stButtonInfo.cbSize  = sizeof(TBBUTTONINFO);
	stButtonInfo.dwMask  = TBIF_TEXT;
	stButtonInfo.pszText = atText;
	stButtonInfo.cchText = MAX_STRING;

	for( i = 0; TB_MAIN_ITEMS > i; i++ )
	{
		if( 0 == gstMainTBInfo[i].idCommand )	continue;

		StringCchCopy( atText, MAX_STRING, gatMainTBtext[i] );
		AccelKeyTextBuild( atText, MAX_STRING , gstMainTBInfo[i].idCommand, pstAccel, iEntry );
		SendMessage( ghMainTBWnd, TB_SETBUTTONINFO, (WPARAM)(gstMainTBInfo[i].idCommand), (LPARAM)&stButtonInfo );
	}

	for( i = 0; TB_EDIT_ITEMS > i; i++ )
	{
		if( 0 == gstEditTBInfo[i].idCommand )	continue;

		StringCchCopy( atText, MAX_STRING, gatEditTBtext[i] );
		AccelKeyTextBuild( atText, MAX_STRING , gstEditTBInfo[i].idCommand, pstAccel, iEntry );
		SendMessage( ghEditTBWnd, TB_SETBUTTONINFO, (WPARAM)(gstEditTBInfo[i].idCommand), (LPARAM)&stButtonInfo );
	}

	for( i = 0; TB_INSERT_ITEMS > i; i++ )
	{
		if( 0 == gstInsertTBInfo[i].idCommand )	continue;

		StringCchCopy( atText, MAX_STRING, gatInsertTBtext[i] );
		AccelKeyTextBuild( atText, MAX_STRING , gstInsertTBInfo[i].idCommand, pstAccel, iEntry );
		SendMessage( ghInsertTBWnd, TB_SETBUTTONINFO, (WPARAM)(gstInsertTBInfo[i].idCommand), (LPARAM)&stButtonInfo );
	}

	for( i = 0; TB_LAYOUT_ITEMS > i; i++ )
	{
		if( 0 == gstLayoutTBInfo[i].idCommand )	continue;

		StringCchCopy( atText, MAX_STRING, gatLayoutTBtext[i] );
		AccelKeyTextBuild( atText, MAX_STRING , gstLayoutTBInfo[i].idCommand, pstAccel, iEntry );
		SendMessage( ghLayoutTBWnd, TB_SETBUTTONINFO, (WPARAM)(gstLayoutTBInfo[i].idCommand), (LPARAM)&stButtonInfo );
	}

	for( i = 0; TB_VIEW_ITEMS > i; i++ )
	{
		if( 0 == gstViewTBInfo[i].idCommand )	continue;

		StringCchCopy( atText, MAX_STRING, gatViewTBtext[i] );
		AccelKeyTextBuild( atText, MAX_STRING , gstViewTBInfo[i].idCommand, pstAccel, iEntry );
		SendMessage( ghViewTBWnd, TB_SETBUTTONINFO, (WPARAM)(gstViewTBInfo[i].idCommand), (LPARAM)&stButtonInfo );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ツールバーの破壊
*/
VOID ToolBarDestroy( VOID )
{
	ImageList_Destroy( ghMainImgLst );
	ImageList_Destroy( ghEditImgLst );
	ImageList_Destroy( ghLayoutImgLst );
	ImageList_Destroy( ghInsertImgLst );
	ImageList_Destroy( ghViewImgLst );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ツールバーの大きさをとる
	@param[in]	pstRect	サイズを戻すバッファ
	@return		HRESULT	終了状態コード
*/
HRESULT ToolBarSizeGet( LPRECT pstRect )
{
	RECT	rect;

	GetWindowRect( ghRebarWnd, &rect );

	rect.right  -= rect.left;
	rect.bottom -= rect.top;
	rect.left    = 0;
	rect.top     = 0;

	SetRect( pstRect, rect.left, rect.top, rect.right, rect.bottom );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	つるば項目をチェックしたり外したり
	@param[in]	itemID	操作するメニューのＩＤ
	@param[in]	bCheck	非０チェック・０はずす
	@return		HRESULT	終了状態コード
*/
HRESULT ToolBarCheckOnOff( UINT itemID, UINT bCheck )
{
	HWND	hTlBrWnd;

	switch( itemID )
	{
		default:	return S_OK;

		case IDM_EXTRACTION_MODE:	hTlBrWnd =  ghEditTBWnd;	break;
		case IDM_SQSELECT:			hTlBrWnd =  ghEditTBWnd;	break;
		case IDM_UNICODE_TOGGLE:	hTlBrWnd =  ghEditTBWnd;	break;

		case IDM_UNI_PALETTE:		hTlBrWnd =  ghViewTBWnd;	break;
		case IDM_TRACE_MODE_ON:		hTlBrWnd =  ghViewTBWnd;	break;
	}

	SendMessage( hTlBrWnd, TB_CHECKBUTTON, itemID, bCheck ? TRUE : FALSE );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ウインドウがサイズ変更されたとき
	@param[in]	hWnd	親ウインドウのハンドル
	@param[in]	state	変更の状態・SIZE_MINIMIZED とか
	@param[in]	cx		クライヤントＸサイズ
	@param[in]	cy		クライヤントＹサイズ
	@return		無し
*/
HRESULT ToolBarOnSize( HWND hWnd, UINT state, INT cx, INT cy )
{
	FORWARD_WM_SIZE( ghRebarWnd, state, cx, cy, SendMessage );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ノーティファイメッセージのダミー生成
	@param[in]	hWnd	親ウインドウのハンドル
	@param[in]	itemID	押されたボタンのＩＤ
*/
VOID ToolBarPseudoDropDown( HWND hWnd, INT itemID )
{
	NMTOOLBAR	stNmToolBar;
	INT	iFrom;


	ZeroMemory( &stNmToolBar, sizeof(NMTOOLBAR) );

	switch( itemID )
	{
		case IDM_IN_UNI_SPACE:
		case IDM_INSTAG_COLOUR:
		case IDM_USERINS_NA:
			stNmToolBar.hdr.hwndFrom = ghInsertTBWnd;
			iFrom = IDTB_INSERT_TOOLBAR;
			break;

		default:	 return;	//	することはない
	}

	stNmToolBar.hdr.idFrom   = iFrom;
	stNmToolBar.hdr.code     = TBN_DROPDOWN;
	stNmToolBar.iItem        = itemID;

	ToolBarOnNotify( hWnd, iFrom, (LPNMHDR)(&stNmToolBar) );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	コンテキストメニュー呼びだしアクション(要は右クルック）リバーのやつ
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	hWndContext	コンテキストが発生したウインドウのハンドル
	@param[in]	xPos		スクリーンＸ座標
	@param[in]	yPos		スクリーンＹ座業
	@return		処理したら非０、何も無かったら０
*/
LRESULT ToolBarOnContextMenu( HWND hWnd, HWND hWndContext, LONG xPos, LONG yPos )
{
	HMENU	hPopupMenu;

	if( ghRebarWnd != hWndContext ){	return 0;	}

	TRACE( TEXT("REBAR CONTEXT[%d x %d]"), xPos, yPos );

	hPopupMenu = CreatePopupMenu(  );
	AppendMenu( hPopupMenu, MF_STRING, IDM_REBER_DORESET, TEXT("配置を初期状態に戻す") );
	TrackPopupMenu( hPopupMenu, 0, xPos, yPos, 0, hWnd, NULL );
	DestroyMenu( hPopupMenu );

	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	ノーティファイメッセージの処理
	@param[in]	hWnd		親ウインドウのハンドル
	@param[in]	idFrom		NOTIFYを発生させたコントロールのＩＤ
	@param[in]	pstNmhdr	NOTIFYの詳細
	@return		処理した内容とか・特に返すモノはない
*/
LRESULT ToolBarOnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{
	INT			iItem, i;
	HWND		hWndFrom;
	HMENU		hPopupMenu = NULL, hMainMenu;
	LPNMTOOLBAR	pstNmToolBar;
	TPMPARAMS	stTpmParam;
	RECT		rect;


	if( IDRB_REBAR == idFrom )
	{
		if( RBN_HEIGHTCHANGE == pstNmhdr->code )
		{
			if( !( AppClientAreaCalc( &rect ) ) )	return 0;
			ViewSizeMove( hWnd, &rect );	//	メインのツールバーで変化があった
			InvalidateRect( hWnd, NULL, TRUE );
			InvalidateRect( ghRebarWnd, NULL, TRUE );
		}
	}

	//	ツールバーのドロップダウンメニュー	IDTB_INSERT_TOOLBAR == idFrom && 
	if( TBN_DROPDOWN == pstNmhdr->code )
	{
		TRACE( TEXT("ドロップダウン発生") );
		pstNmToolBar = (LPNMTOOLBAR)pstNmhdr;

		iItem    = pstNmToolBar->iItem;
		hWndFrom = pstNmToolBar->hdr.hwndFrom;

		hMainMenu = GetMenu( hWnd );

		//	ボタンの座標を取得
		SendMessage( hWndFrom, TB_GETRECT, (WPARAM)iItem, (LPARAM)(&rect) );
		//	他のウィンドウを基準とする座標へ変換
		MapWindowPoints( hWndFrom, HWND_DESKTOP, (LPPOINT)(&rect), 2 );

		ZeroMemory( &stTpmParam, sizeof(TPMPARAMS) );
		stTpmParam.cbSize = sizeof(TPMPARAMS);
		stTpmParam.rcExclude = rect;

		switch( iItem )
		{
			case IDM_OPEN:
				TrackPopupMenuEx( ghHistyMenu, TPM_VERTICAL, rect.left, rect.bottom, hWnd, &stTpmParam );
				break;

			case IDM_IN_UNI_SPACE:
				hPopupMenu = GetSubMenu( GetSubMenu(hMainMenu,2), 0 );
				TrackPopupMenuEx( hPopupMenu, TPM_VERTICAL, rect.left, rect.bottom, hWnd, &stTpmParam );
				break;

			case IDM_INSTAG_COLOUR:
				hPopupMenu = GetSubMenu( GetSubMenu(hMainMenu,2), 1 );
				TrackPopupMenuEx( hPopupMenu, TPM_VERTICAL, rect.left, rect.bottom, hWnd, &stTpmParam );
				break;

			case IDM_FRMINSBOX_OPEN:
				hPopupMenu = CreatePopupMenu(  );
				for( i = 0; FRAME_MAX > i; i++ ){	AppendMenu( hPopupMenu, MF_STRING, (IDM_INSFRAME_ALPHA + i), TEXT("枠") );	}
				FrameNameModifyPopUp( hPopupMenu, 1 );
				TrackPopupMenuEx( hPopupMenu, TPM_VERTICAL, rect.left, rect.bottom, hWnd, &stTpmParam );
				DestroyMenu( hPopupMenu );
				break;

			case IDM_USERINS_NA:
				hPopupMenu = CreatePopupMenu(  );
				UserDefMenuWrite( hPopupMenu, 0 );
				TrackPopupMenuEx( hPopupMenu, TPM_VERTICAL, rect.left, rect.bottom, hWnd, &stTpmParam );
				DestroyMenu( hPopupMenu );
				break;

			default:	break;
		}

	}

	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	リバーの配置を元に戻す
*/
HRESULT ToolBarBandReset( HWND hWnd )
{
	INT		index;
	UINT	d;
	REBARBANDINFO	stRbandInfo;

	TRACE( TEXT("REBAR RESET") );

	ZeroMemory( &stRbandInfo, sizeof(REBARBANDINFO) );
	stRbandInfo.cbSize = sizeof(REBARBANDINFO);
	stRbandInfo.fMask  = RBBIM_STYLE | RBBIM_SIZE;

	for( d = 0; TB_BAND_COUNT > d; d++ )
	{
		index = SendMessage( ghRebarWnd, RB_IDTOINDEX, gcstReBarDef[d].wID, 0 );
		if( 0 > index ){	continue;	}

		SendMessage( ghRebarWnd, RB_MOVEBAND, index, d );

		stRbandInfo.cx     = gcstReBarDef[d].cx;
		stRbandInfo.fStyle = gcstReBarDef[d].fStyle;
		SendMessage( ghRebarWnd, RB_SETBANDINFO, (WPARAM)d, (LPARAM)&stRbandInfo );
	}

	InvalidateRect( ghRebarWnd, NULL, TRUE );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	リバーの配置情報を確認する
*/
UINT ToolBarBandInfoGet( LPVOID pVoid )
{
	UINT	d;
	REBARBANDINFO	stBandInfo;
	REBARLAYOUTINFO	stInfo[TB_BAND_COUNT];

	ZeroMemory( &stBandInfo, sizeof(REBARBANDINFO) );
	stBandInfo.cbSize = sizeof(REBARBANDINFO);
	stBandInfo.fMask = RBBIM_ID | RBBIM_STYLE | RBBIM_SIZE;

	ZeroMemory( stInfo, sizeof(stInfo) );

	for( d = 0; TB_BAND_COUNT > d; d++ )
	{
		SendMessage( ghRebarWnd, RB_GETBANDINFO, (WPARAM)d, (LPARAM)(&stBandInfo) );
		TRACE( TEXT("ID[%u]  CX[%d]  STYLE[%u]"), stBandInfo.wID, stBandInfo.cx, stBandInfo.fStyle );

		stInfo[d].wID    = stBandInfo.wID;
		stInfo[d].cx     = stBandInfo.cx;
		stInfo[d].fStyle = stBandInfo.fStyle;
	}

	InitToolBarLayout( INIT_SAVE, TB_BAND_COUNT, stInfo );	//	保存

	return TB_BAND_COUNT;
}
//-------------------------------------------------------------------------------------------------

/*
次回起動時に位置を復元するために必要な情報はRB_GETBANDINFOメッセージで調べる。

復元するのに必要なのは、バンドの並んでいる順番、バンドの幅、そしてどのバンドで改行するか。

UINT count = SendMessage(hRebar, RB_GETBANDCOUNT, 0, 0);

REBARBANDINFO rbi = {0};
rbi.cbSize = sizeof(REBARBANDINFO);
rbi.fMask = RBBIM_ID | RBBIM_STYLE | RBBIM_SIZE;

DWORD written;
for (UINT i = 0; i < count; ++i)
{
	SendMessage(hRebar, RB_GETBANDINFO, i, (LPARAM)&rbi);

	WriteFile(hfile, &rbi.wID, sizeof(rbi.wID), &written, NULL);
	
	bool br = rbi.fStyle & RBBS_BREAK;
	WriteFile(hfile, &br, sizeof(br), &written, NULL);
	
	WriteFile(hfile, &rbi.cx, sizeof(rbi.cx), &written, NULL);
}

RBBS_BREAKはバンドを挿入した時どうだったかではなく、
RB_GETBANDINFOを送った時点での改行状況を教えてくれる。
ただし１行目の最初のバンドにはRBBS_BREAKが含まれる場合と含まれない場合がある。


[終了時]
char iniSection[50];
REBARBANDINFO bandInfo;
ZeroMemory(&bandInfo, sizeof(bandInfo)); 
bandInfo.cbSize = sizeof(bandInfo);
bandInfo.fMask = RBBIM_ID | RBBIM_SIZE | RBBIM_STYLE;
int barcnt = SendMessage(wprop->hRebar, RB_GETBANDCOUNT, 0, 0);
for(int i = 0; i < barcnt; i++){
    sprintf(iniSection, INISECTION_BAR, i);
    SendMessage(hRebar, RB_GETBANDINFO, (WPARAM)(UINT)i, (LPARAM)(LPREBARBANDINFO)&bandInfo);

    // wID, cx, fStyle を ini ファイルに保存する
    MyWriteProfileInt(iniSection, "ID", bandInfo.wID);
    MyWriteProfileInt(iniSection, "CX", bandInfo.cx);
    MyWriteProfileInt(iniSection, "FL_BREAK", (bandInfo.fStyle & RBBS_BREAK) ? 1 : 0);
    MyWriteProfileInt(iniSection, "FL_HIDDEN", (bandInfo.fStyle & RBBS_HIDDEN) ? 1 : 0);
}


[起動時]
int i;
char iniSection[50];
// CreateToolbarx はバンド内のツールバーを作成する関数
HWND (*hwndproc[])(HWND) = {CreateToolbar1, CreateToolbar2};

REBARBANDINFO rbBand;
ZeroMemory(&rbBand, sizeof(REBARBANDINFO));
rbBand.cbSize = sizeof(REBARBANDINFO);
rbBand.fMask  = RBBIM_ID | RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE | 
RBBIM_SIZE;
rbBand.cxMinChild = 0;
rbBand.cyMinChild = 25;
for(i = 0; i < NUM_TOOL; i++){
    sprintf(iniSection, INISECTION_BAR, i);
    rbBand.wID = SZC_GetProfileInt(iniSection, "ID", 9999);
    rbBand.cx = SZC_GetProfileInt(iniSection, "CX", 100);
    rbBand.fStyle = RBBS_CHILDEDGE;
    if(SZC_GetProfileInt(iniSection, "FL_BREAK", 0)) rbBand.fStyle |= RBBS_BREAK;
    if(SZC_GetProfileInt(iniSection, "FL_HIDDEN", 0)) rbBand.fStyle |= RBBS_HIDDEN;
    rbBand.hwndChild = hwndproc[rbBand.wID](hRebar);
    SendMessage(wprop->hRebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);
}

*/
