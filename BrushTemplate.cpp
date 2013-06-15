/*! @file
	@brief ブラシテンプレートの面倒見ます
	このファイルは BrushTemplate.cpp です。
	@author	SikigamiHNQ
	@date	2011/06/20
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

#define BRUSHTEMPLATE_CLASS	TEXT("BRUSH_TEMPLATE")
#define BT_WIDTH	240
#define BT_HEIGHT	240

#define BTV_R_MARGIN	18
//-------------------------------------------------------------------------------------------------

#define TB_ITEMS	1
static  TBBUTTON	gstBrTBInfo[] = {
	{ 0,	IDM_BRUSH_ON_OFF,	TBSTATE_ENABLED,	TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0 }	//	On/Off
};	//	
//-------------------------------------------------------------------------------------------------

extern HFONT	ghAaFont;		//	AA用フォント

extern INT		gbTmpltDock;	//	テンプレのドッキング
extern BOOLEAN	gbDockTmplView;	//	くっついてるテンプレは見えているか

//extern  HWND	ghMainSplitWnd;	//	メインのスプリットバーハンドル
extern  LONG	grdSplitPos;	//	スプリットバーの、左側の、画面右からのオフセット

static HIMAGELIST	ghBrushImgLst;

static  UINT	gbBrushMode;	//!<	非零ブラシモード

static  ATOM	gBrTmplAtom;	//!<	
static  HWND	ghBrTmplWnd;	//!<	ブラシパレット本体ウインドウ
static  HWND	ghBrTlBarWnd;	//!<	ツールバー
static  HWND	ghCtgryBxWnd;	//!<	カテゴリコンボックス
static  HWND	ghLvItemWnd;	//!<	内容リストビュー
static  HWND	ghBrLvTipWnd;	//!<	Brushリストツールチップ

static  HWND	ghMainWnd;		//!<	編集ビューのある本体ウインドウ

static  UINT	gNowGroup;		//!<	カテゴリ

static WNDPROC	gpfOrigBrushCtgryProc;	//!<	
static WNDPROC	gpfOrigBrushItemProc;	//!<	

static  UINT	gBrhClmCnt;	//!<	表示カラム数

static WNDPROC	gpfOrigTBProc;	//!<	

static vector<AATEMPLATE>	gvcBrTmpls;	//!<	テンプレの保持
//-------------------------------------------------------------------------------------------------


LRESULT	CALLBACK BrushTmpleProc( HWND, UINT, WPARAM, LPARAM );
VOID	Btp_OnCommand( HWND, INT, HWND, UINT );
VOID	Btp_OnSize( HWND, UINT, INT, INT );
LRESULT	Btp_OnNotify( HWND, INT, LPNMHDR );
VOID	Btp_OnContextMenu( HWND, HWND, UINT, UINT );

UINT	CALLBACK BrushTmpleItemData( LPTSTR, LPCTSTR, INT );

UINT	BrushTmpleItemListOn( UINT );
HRESULT	BrushTmpleItemReload( HWND );

LRESULT	CALLBACK gpfBrushCtgryProc( HWND, UINT, WPARAM, LPARAM );
LRESULT	CALLBACK gpfBrushItemProc(  HWND, UINT, WPARAM, LPARAM );
LRESULT	Blv_OnNotify( HWND, INT, LPNMHDR );

static LRESULT	CALLBACK gpfToolbarProc( HWND, UINT, WPARAM, LPARAM );
//-------------------------------------------------------------------------------------------------


/*!
	ブラシテンプレウインドウの作成
	@param[in]	hInstance	アプリのインスタンス
	@param[in]	hParentWnd	メインウインドウのハンドル
	@param[in]	pstFrame	
	@param[in]	hMaaWnd		複数行テンプレのウインドウハンドル
	@return		作ったビューのウインドウハンドル
*/
HWND BrushTmpleInitialise( HINSTANCE hInstance, HWND hParentWnd, LPRECT pstFrame, HWND hMaaWnd )
{
	DWORD		dwExStyle, dwStyle;
	HWND		hPrWnd;
	UINT_PTR	dItems, i;
	TCHAR		atBuffer[MAX_STRING];

	HBITMAP	hImg, hMsq;
	INT		spPos;


	WNDCLASSEX	wcex;
	RECT		wdRect, clRect, rect, cbxRect, tbRect, mtbRect;
	LVCOLUMN	stLvColm;

	TTTOOLINFO	stToolInfo;


	//	破壊
	if( !(hInstance) && !(hParentWnd) )
	{
		ImageList_Destroy( ghBrushImgLst  );
		return NULL;
	}

	ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= BrushTmpleProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= BRUSHTEMPLATE_CLASS;
	wcex.hIconSm		= NULL;

	gBrTmplAtom = RegisterClassEx( &wcex );

	ghMainWnd = hParentWnd;

	gbBrushMode = FALSE;

//テンプレデータ読み出し
	TemplateItemLoad( AA_BRUSH_FILE, BrushTmpleItemData );


	InitWindowPos( INIT_LOAD, WDP_BRTMPL, &rect );
	if( 0 == rect.right || 0 == rect.bottom )	//	幅高さが０はデータ無し
	{
		GetWindowRect( hParentWnd, &wdRect );
		rect.left   = wdRect.right + 32;
		rect.top    = wdRect.top + 32;
		rect.right  = BT_WIDTH;
		rect.bottom = BT_HEIGHT;
		InitWindowPos( INIT_SAVE, WDP_BRTMPL, &rect );//起動時保存
	}

	//	カラム数確認
	gBrhClmCnt = InitParamValue( INIT_LOAD, VL_BRUSHTMP_CLM, 4 );

	if( gbTmpltDock )
	{
		spPos = grdSplitPos - SPLITBAR_WIDTH;	//	右からのオフセット

		hPrWnd    = hParentWnd;
		dwExStyle = 0;
		dwStyle   = WS_CHILD;

		rect = *pstFrame;	//	クライヤントに使える領域
		rect.left  = rect.right - spPos;
		rect.right = PLIST_DOCK;
		rect.bottom >>= 1;
		rect.top    += rect.bottom;

		DockingTabSizeGet( &mtbRect );
		rect.top    += mtbRect.bottom;
		rect.bottom -= mtbRect.bottom;
	}
	else
	{
		hPrWnd = NULL;
		//	常に最全面に表示を？
		dwExStyle = WS_EX_TOOLWINDOW;
		if( InitWindowTopMost( INIT_LOAD, WDP_BRTMPL, 0 ) ){	dwExStyle |=  WS_EX_TOPMOST;	}
		dwStyle = WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_VISIBLE | WS_SYSMENU;
	}

	//	本体
	ghBrTmplWnd = CreateWindowEx( dwExStyle, BRUSHTEMPLATE_CLASS, TEXT("Brush Template"),
		dwStyle, rect.left, rect.top, rect.right, rect.bottom, hPrWnd, NULL, hInstance, NULL);

	//	ツールバー
	ghBrTlBarWnd = CreateWindowEx( WS_EX_CLIENTEDGE, TOOLBARCLASSNAME, TEXT("brtoolbar"), WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS, 0, 0, 0, 0, ghBrTmplWnd, (HMENU)IDW_BRUSH_TOOL_BAR, hInstance, NULL );

	//	自動ツールチップスタイルを追加
	SendMessage( ghBrTlBarWnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS );

	ghBrushImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 1, 1 );
	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMP_BRUSH_MODE) ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMQ_BRUSH_MODE) ) );
	ImageList_Add( ghBrushImgLst , hImg, hMsq );	//	イメージリストにイメージを追加
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );
	SendMessage( ghBrTlBarWnd, TB_SETIMAGELIST, 0, (LPARAM)ghBrushImgLst );

	SendMessage( ghBrTlBarWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );
	//	ツールチップ文字列を設定・ボタンテキストがツールチップになる
	StringCchCopy( atBuffer, MAX_STRING, TEXT("ブラシモード ON/OFF") );
	gstBrTBInfo[0].iString = SendMessage( ghBrTlBarWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );

	SendMessage( ghBrTlBarWnd , TB_ADDBUTTONS, (WPARAM)TB_ITEMS, (LPARAM)&gstBrTBInfo );	//	ツールバーにボタンを挿入

	SendMessage( ghBrTlBarWnd , TB_AUTOSIZE, 0, 0 );	//	ボタンのサイズに合わせてツールバーをリサイズ
	InvalidateRect( ghBrTlBarWnd , NULL, TRUE );		//	クライアント全体を再描画する命令

	//	ツールバーサブクラス化
	gpfOrigTBProc = SubclassWindow( ghBrTlBarWnd, gpfToolbarProc );

	GetClientRect( ghBrTlBarWnd, &tbRect );





	GetClientRect( ghBrTmplWnd, &clRect );

	//	カテゴリコンボックス
	ghCtgryBxWnd = CreateWindowEx( 0, WC_COMBOBOX, TEXT("BrCategory"),
		WS_CHILD | WS_VISIBLE | WS_BORDER | CBS_DROPDOWNLIST,
		0, tbRect.bottom, clRect.right, 127, ghBrTmplWnd,
		(HMENU)IDCB_BT_CATEGORY, hInstance, NULL );

	gpfOrigBrushCtgryProc = SubclassWindow( ghCtgryBxWnd, gpfBrushCtgryProc );

	dItems = gvcBrTmpls.size( );
	for( i = 0; dItems > i; i++ )
	{
		ComboBox_AddString( ghCtgryBxWnd, gvcBrTmpls.at( i ).atCtgryName );
	}
	ComboBox_SetCurSel( ghCtgryBxWnd, 0 );
	gNowGroup = 0;

	GetClientRect( ghCtgryBxWnd, &cbxRect );

	//	一覧リストビュー
	ghLvItemWnd = CreateWindowEx( 0, WC_LISTVIEW, TEXT("brushitem"),
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LVS_REPORT | LVS_NOSORTHEADER | LVS_NOCOLUMNHEADER | LVS_SINGLESEL,
		0, tbRect.bottom + cbxRect.bottom, clRect.right, clRect.bottom - (cbxRect.bottom + tbRect.bottom),
		ghBrTmplWnd, (HMENU)IDLV_BT_ITEMVIEW, hInstance, NULL );
	ListView_SetExtendedListViewStyle( ghLvItemWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	SetWindowFont( ghLvItemWnd, ghAaFont, TRUE );

	gpfOrigBrushItemProc = SubclassWindow( ghLvItemWnd, gpfBrushItemProc );

	ZeroMemory( &stLvColm, sizeof(LVCOLUMN) );
	stLvColm.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	stLvColm.fmt      = LVCFMT_LEFT;
	stLvColm.pszText  = TEXT("Brush");
	stLvColm.cx       = 10;	//	後で合わせるので適当で良い
	stLvColm.iSubItem = 0;

	for( i = 0; gBrhClmCnt > i; i++ )
	{
		stLvColm.iSubItem = i;
		ListView_InsertColumn( ghLvItemWnd, i, &stLvColm );
	}

	BrushTmpleItemListOn( 0 );	//	中身追加

	//	リストビューツールチップ
	ghBrLvTipWnd = CreateWindowEx( WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, ghBrTmplWnd, NULL, hInstance, NULL );
//	SetWindowFont( ghBrLvTipWnd, GetStockFont(DEFAULT_GUI_FONT), FALSE );

	//	ツールチップをコールバックで割り付け
	ZeroMemory( &stToolInfo, sizeof(TTTOOLINFO) );
	stToolInfo.cbSize   = sizeof(TTTOOLINFO);
	stToolInfo.uFlags   = TTF_SUBCLASS;
	stToolInfo.hinst    = NULL;	//	
	stToolInfo.hwnd     = ghLvItemWnd;
	stToolInfo.uId      = IDLV_BT_ITEMVIEW;
	GetClientRect( ghLvItemWnd, &stToolInfo.rect );
	stToolInfo.lpszText = LPSTR_TEXTCALLBACK;	//	コレを指定するとコールバックになる
	SendMessage( ghBrLvTipWnd, TTM_ADDTOOL, 0, (LPARAM)&stToolInfo );
	SendMessage( ghBrLvTipWnd, TTM_SETMAXTIPWIDTH, 0, 0 );	//	チップの幅。０設定でいい。これしとかないと改行されない


	if( !(gbTmpltDock) )
	{
		ShowWindow( ghBrTmplWnd, SW_SHOW );
		UpdateWindow( ghBrTmplWnd );
	}

	return ghBrTmplWnd;
}
//-------------------------------------------------------------------------------------------------

/*!
	フローティングブラシテンプレの位置リセット
	@param[in]	hMainWnd	メインウインドウハンドル
	@return	HRESULT	終了状態コード
*/
HRESULT BrushTmplePositionReset( HWND hMainWnd )
{
	RECT	wdRect, rect;

	GetWindowRect( hMainWnd, &wdRect );
	rect.left   = wdRect.right + 32;
	rect.top    = wdRect.top + 32;
	rect.right  = BT_WIDTH;
	rect.bottom = BT_HEIGHT;

	SetWindowPos( ghBrTmplWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW | SWP_NOZORDER );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ドッキング状態で発生・くっついてるウインドウがリサイズされたら
	@param[in]	hPrntWnd	くっついてるウインドウハンドル
	@param[in]	pstFrame	使えるサイズ
*/
VOID BrushTmpleResize( HWND hPrntWnd, LPRECT pstFrame )
{
	RECT	rect, tbRect;
	//INT	iHei, iTak;

	//gbTmpltDock
	if( !(ghBrTmplWnd) )	return;

	//	非表示なら何もしないでおｋ
	if( !(gbDockTmplView) )	return;

	rect = *pstFrame;	//	クライヤントに使える領域
	rect.left    = rect.right - (grdSplitPos - SPLITBAR_WIDTH);
	rect.right   = (grdSplitPos - SPLITBAR_WIDTH);
	rect.bottom >>= 1;
	rect.top    += rect.bottom;

	DockingTabSizeGet( &tbRect );
	rect.top    += tbRect.bottom;
	rect.bottom -= tbRect.bottom;

	SetWindowPos( ghBrTmplWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_NOZORDER );

	return;
}
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
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			if( SendMessage(hWnd, TB_GETHOTITEM, 0, 0) >= 0 ){	ReleaseCapture(   );	}
			return 0;
	}

	return CallWindowProc( gpfOrigTBProc, hWnd, msg, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------


/*!
	ウインドウプロシージャ
	@param[in]	hWnd	親ウインドウのハンドル
	@param[in]	message	ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@retval 0	メッセージ処理済み
	@retval no0	ここでは処理せず次に回す
*/
LRESULT CALLBACK BrushTmpleProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_SIZE,        Btp_OnSize );	
		HANDLE_MSG( hWnd, WM_COMMAND,     Btp_OnCommand );	
		HANDLE_MSG( hWnd, WM_NOTIFY,      Btp_OnNotify );	//	コモンコントロールの個別イベント
		HANDLE_MSG( hWnd, WM_CONTEXTMENU, Btp_OnContextMenu );

		case WM_MOUSEWHEEL:	SendMessage( ghLvItemWnd, WM_MOUSEWHEEL, wParam, lParam );	return 0;

		case WM_CLOSE:	ShowWindow( ghBrTmplWnd , SW_HIDE );	return 0;

		case WMP_BRUSH_TOGGLE:
			if( gbBrushMode )
			{
				SendMessage( ghBrTlBarWnd, TB_SETSTATE, IDM_BRUSH_ON_OFF, TBSTATE_ENABLED );
				gbBrushMode = FALSE;
			}
			else
			{
				SendMessage( ghBrTlBarWnd, TB_SETSTATE, IDM_BRUSH_ON_OFF, (TBSTATE_CHECKED | TBSTATE_ENABLED) );
				gbBrushMode = TRUE;
			}
			ViewBrushStyleSetting( gbBrushMode, NULL );	//	ビューウインドウにモード付ける
			return gbBrushMode;

		default:	break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------

/*!
	COMMANDメッセージの受け取り。ボタン押されたとかで発生
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	id			メッセージを発生させた子ウインドウの識別子	LOWORD(wParam)
	@param[in]	hWndCtl		メッセージを発生させた子ウインドウのハンドル	lParam
	@param[in]	codeNotify	通知メッセージ	HIWORD(wParam)
	@return		なし
*/
VOID Btp_OnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify )
{
	INT			rslt;
	UINT		dClm;
	LRESULT		lRslt;
	LONG_PTR	rdExStyle;
	TCHAR		atItem[SUB_STRING];

	ZeroMemory( atItem, sizeof(atItem) );

	switch( id )
	{
		case IDCB_BT_CATEGORY:	//	カテゴリ選択コンボックス
			if( 0 < gvcBrTmpls.size() )
			{
				if( CBN_SELCHANGE == codeNotify )	//	選択変更されたら
				{
					rslt = ComboBox_GetCurSel( ghCtgryBxWnd );
					gNowGroup = rslt;

					BrushTmpleItemListOn( rslt );

					//	Brush解除
					gbBrushMode = FALSE;
					SendMessage( ghBrTlBarWnd, TB_SETSTATE, IDM_BRUSH_ON_OFF, TBSTATE_ENABLED );
					StringCchCopy( atItem, SUB_STRING, gvcBrTmpls.at( gNowGroup ).vcItems.at( 0 ).c_str( ) );
					ViewBrushStyleSetting( gbBrushMode, atItem );
				}
			}
			break;

		case IDM_BRUSH_ON_OFF:	//	ここに来る時点でON/OFF切り替わってる
			lRslt = SendMessage( ghBrTlBarWnd, TB_GETSTATE, IDM_BRUSH_ON_OFF, 0 );
			gbBrushMode = (lRslt & TBSTATE_CHECKED) ? TRUE : FALSE;
			ViewBrushStyleSetting( gbBrushMode, NULL );	//	状態をおくる
			break;

		case  IDM_WINDOW_CHANGE:	WindowFocusChange( WND_BRUSH,  1 );	break;
		case  IDM_WINDOW_CHG_RVRS:	WindowFocusChange( WND_BRUSH, -1 );	break;

		case IDM_TMPL_GRID_INCREASE:
		case IDM_TMPL_GRID_DECREASE:
			dClm = TemplateGridFluctuate( ghLvItemWnd, ((IDM_TMPL_GRID_INCREASE == id) ? 1 : -1) );
			if( dClm )
			{
				gBrhClmCnt = dClm;
				BrushTmpleItemListOn( gNowGroup );
				InitParamValue( INIT_SAVE, VL_BRUSHTMP_CLM, gBrhClmCnt );
			}
			break;

		case IDM_TMPLT_GROUP_PREV:
			if( 0 < gNowGroup )
			{
				gNowGroup--;
				ComboBox_SetCurSel( ghCtgryBxWnd, gNowGroup );
				BrushTmpleItemListOn( gNowGroup );
			}
			break;

		case IDM_TMPLT_GROUP_NEXT:
			if( (gNowGroup + 1) < gvcBrTmpls.size() )
			{
				gNowGroup++;
				ComboBox_SetCurSel( ghCtgryBxWnd, gNowGroup );
				BrushTmpleItemListOn( gNowGroup );
			}
			break;

		case IDM_TOPMOST_TOGGLE:	//	常時最全面と通常ウインドウのトグル
			rdExStyle = GetWindowLongPtr( hWnd, GWL_EXSTYLE );
			if( WS_EX_TOPMOST & rdExStyle )
			{
				SetWindowPos( hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
				InitWindowTopMost( INIT_SAVE, WDP_BRTMPL, 0 );
			}
			else
			{
				SetWindowPos( hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
				InitWindowTopMost( INIT_SAVE, WDP_BRTMPL, 1 );
			}
			break;

		//	テンプレファイルリロード
		case IDM_TMPLT_RELOAD:	BrushTmpleItemReload( hWnd );	break;

		default:	break;
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	サイズ変更された
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	state	なにかの状態
	@param[in]	cx		変更されたクライヤント幅
	@param[in]	cy		変更されたクライヤント高さ
*/
VOID Btp_OnSize( HWND hWnd, UINT state, INT cx, INT cy )
{
	LONG	width;
	UINT	i;
	RECT	cbxRect, tbrRect, rect;

	if( !(ghBrTlBarWnd) )	return;
	MoveWindow( ghBrTlBarWnd, 0, 0, 0, 0, TRUE );	//	ツールバーは数値なくても勝手に合わせてくれる
	GetClientRect( ghBrTlBarWnd, &tbrRect );

	if( !(ghCtgryBxWnd) )	return;
	MoveWindow( ghCtgryBxWnd, 0, tbrRect.bottom, cx, 127, TRUE );
	GetClientRect( ghCtgryBxWnd, &cbxRect );

	if( !(ghLvItemWnd) )	return;
	MoveWindow( ghLvItemWnd, 0, (cbxRect.bottom + tbrRect.bottom), cx, cy - (cbxRect.bottom + tbrRect.bottom), TRUE );

	GetClientRect( ghLvItemWnd, &rect );
	width = rect.right / gBrhClmCnt;

	for( i = 0; gBrhClmCnt > i; i++ )
	{
		ListView_SetColumnWidth( ghLvItemWnd, i, width );
	}


	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ノーティファイメッセージの処理
	@param[in]	hWnd		親ウインドウのハンドル
	@param[in]	idFrom		NOTIFYを発生させたコントロールのＩＤ
	@param[in]	pstNmhdr	NOTIFYの詳細
	@return		処理した内容とか
*/
LRESULT Btp_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{
	HWND	hLvWnd;
	INT		iPos, iItem, nmCode, iSubItem;
	INT_PTR	items;
	TCHAR	atItem[SUB_STRING];
	LPNMLISTVIEW	pstLv;
	LVHITTESTINFO	stHitTestInfo;

	if( IDLV_BT_ITEMVIEW == idFrom )
	{
		pstLv = (LPNMLISTVIEW)pstNmhdr;

		hLvWnd = pstLv->hdr.hwndFrom;
		nmCode = pstLv->hdr.code;

		stHitTestInfo.pt = pstLv->ptAction;
		ListView_SubItemHitTest( hLvWnd, &stHitTestInfo );

		iItem = stHitTestInfo.iItem;
		iSubItem = stHitTestInfo.iSubItem;
		iPos = iItem * gBrhClmCnt + iSubItem;

		//	普通のクルックについて
		if( NM_CLICK == nmCode )
		{
			TRACE( TEXT("BRUSH TMPL[%d x %d]"), iItem, iSubItem );

			if( 0 < gvcBrTmpls.size() )
			{
				items = gvcBrTmpls.at( gNowGroup ).vcItems.size( );

				if( 0 <= iPos && iPos <  items )	//	なんか選択した
				{
					StringCchCopy( atItem, SUB_STRING, gvcBrTmpls.at( gNowGroup ).vcItems.at( iPos ).c_str( ) );

					//	ブラシセット
					gbBrushMode = TRUE;
					SendMessage( ghBrTlBarWnd, TB_SETSTATE, IDM_BRUSH_ON_OFF, (TBSTATE_CHECKED | TBSTATE_ENABLED) );
					//	ビューウインドウにモード付ける
					ViewBrushStyleSetting( gbBrushMode, atItem );

					ViewFocusSet(  );	//	20110720	フォーカスを描画に戻す
				}
			}
			else
			{
				ViewFocusSet(  );
			}
		}
	}

	return 0;	//	何もないなら０を戻す
}
//-------------------------------------------------------------------------------------------------

/*!
	コンテキストメニュー呼びだしアクション(要は右クルック）
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	hWndContext	コンテキストが発生したウインドウのハンドル
	@param[in]	xPos		スクリーンＸ座標
	@param[in]	yPos		スクリーンＹ座業
	@return		無し
*/
VOID Btp_OnContextMenu( HWND hWnd, HWND hWndContext, UINT xPos, UINT yPos )
{
	HMENU	hMenu, hSubMenu;
	UINT	dRslt;
	LONG_PTR	rdExStyle;

	POINT	stPoint;

	stPoint.x = (SHORT)xPos;	//	画面座標はマイナスもありうる
	stPoint.y = (SHORT)yPos;

	hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_TEMPLATE_POPUP) );
	hSubMenu = GetSubMenu( hMenu, 0 );

	//	一体化なら手前表示を削除
	if( gbTmpltDock ){	DeleteMenu( hSubMenu, IDM_TOPMOST_TOGGLE, MF_BYCOMMAND );	}

	rdExStyle = GetWindowLongPtr( hWnd, GWL_EXSTYLE );
	if( WS_EX_TOPMOST & rdExStyle ){	CheckMenuItem( hSubMenu , IDM_TOPMOST_TOGGLE, MF_BYCOMMAND | MF_CHECKED );	}

	dRslt = TrackPopupMenu( hSubMenu, 0, stPoint.x, stPoint.y, 0, hWnd, NULL );
	//	選択せずで０か－１？、選択したらそのメニューのＩＤでWM_COMMANDが発行
	DestroyMenu( hMenu );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	セット名称と、内容物（どっちかのみ有効）を壱行づつ受け取る
	@param[in]	ptName	セット名称・無効ならNULL・こっちの存在優先
	@param[in]	ptLine	項目の内容・無効ならNULL・両方NULLで末端処理
	@param[in]	cchSize	どっちかの内容の文字数
	@return		特に意味はない
*/
UINT CALLBACK BrushTmpleItemData( LPTSTR ptName, LPCTSTR ptLine, INT cchSize )
{
//	両方NULLだったら、本体に追加処理をすれ
	static AATEMPLATE	cstItem;


	if( ptName )	//	セット名称・開始でもある
	{
		StringCchCopy( cstItem.atCtgryName, SUB_STRING, ptName );
		cstItem.vcItems.clear(  );
	}
	else if( ptLine )	//	本体データ
	{
		cstItem.vcItems.push_back( wstring( ptLine ) );
	}
	else	//	どっちもNULL、セット終了
	{
		gvcBrTmpls.push_back( cstItem );
	}

	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	アイテムをリストに展開
	@param[in]	listNum	展開したいセット番号・０インデックス
	@return	UINT	アイテムの個数
*/
UINT BrushTmpleItemListOn( UINT listNum )
{
	INT			width;
	UINT_PTR	i, items;
	TCHAR		atItem[SUB_STRING];
	LVITEM		stLvi;
	RECT		rect;

	if( 0 >= gvcBrTmpls.size() ){	return 0;	}

	ZeroMemory( atItem, sizeof(atItem) );

	items = gvcBrTmpls.at( listNum ).vcItems.size( );

	TRACE( TEXT("BRUSH open NUM[%u] ITEM[%u] GRID[%d]"), listNum, items, gBrhClmCnt );

	ListView_DeleteAllItems( ghLvItemWnd );

	ZeroMemory( &stLvi, sizeof(stLvi) );
	stLvi.mask    = LVIF_TEXT;
	stLvi.pszText = atItem;

	for( i = 0; items > i; i++ )
	{
		StringCchCopy( atItem, SUB_STRING, gvcBrTmpls.at( listNum ).vcItems.at( i ).c_str( ) );

		stLvi.iItem    = i / gBrhClmCnt;
		stLvi.iSubItem = i % gBrhClmCnt;
		if( 0 == stLvi.iSubItem )	ListView_InsertItem( ghLvItemWnd, &stLvi );
		else						ListView_SetItem( ghLvItemWnd, &stLvi );
	}

	//	ブチこんだら幅調整
	GetClientRect( ghLvItemWnd, &rect );
	width = rect.right / gBrhClmCnt;
	for( i = 0; gBrhClmCnt > i; i++ ){	ListView_SetColumnWidth( ghLvItemWnd, i, width );	}

	return items;
}
//-------------------------------------------------------------------------------------------------


/*!
	アイテムファイル最読込
	@param[in]	hWnd		ウインドウハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT BrushTmpleItemReload( HWND hWnd )
{
	TEMPL_ITR	itTmpl;

	gNowGroup = 0;	//	とりあえず０に戻す

	gbBrushMode = 0;	//	選択消して空にしておく
	ViewBrushStyleSetting( gbBrushMode, TEXT("") );

	for( itTmpl = gvcBrTmpls.begin( );  gvcBrTmpls.end( ) != itTmpl; itTmpl++ ){	itTmpl->vcItems.clear();	}
	gvcBrTmpls.clear(  );	//	一旦内容破壊
	
	//	カテゴリコンボックスの中身を全破壊
	while( ComboBox_GetCount( ghCtgryBxWnd )  ){	ComboBox_DeleteString( ghCtgryBxWnd, 0 );	}

	TemplateItemLoad( AA_BRUSH_FILE, BrushTmpleItemData );	//	再びロード

	for( itTmpl = gvcBrTmpls.begin( );  gvcBrTmpls.end( ) != itTmpl; itTmpl++ )
	{
		ComboBox_AddString( ghCtgryBxWnd, itTmpl->atCtgryName );
	}
	ComboBox_SetCurSel( ghCtgryBxWnd, 0 );

	BrushTmpleItemListOn( 0 );	//	０頁を表示

	return S_OK;
}
//-------------------------------------------------------------------------------------------------


/*!
	カテゴリコンボックスサブクラス
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	msg		ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
*/
LRESULT CALLBACK gpfBrushCtgryProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	INT	id;

	switch( msg )
	{
		case WM_COMMAND:
			id = LOWORD(wParam);
			switch( id )
			{
				case IDM_WINDOW_CHANGE:
				case IDM_WINDOW_CHG_RVRS:
				case IDM_TMPL_GRID_INCREASE:
				case IDM_TMPL_GRID_DECREASE:
				case IDM_TMPLT_GROUP_PREV:
				case IDM_TMPLT_GROUP_NEXT:
					SendMessage( ghBrTmplWnd, WM_COMMAND, wParam, lParam );
					return 0;
			}
			break;

		case WM_MOUSEWHEEL:
			SendMessage( ghLvItemWnd, WM_MOUSEWHEEL, wParam, lParam );
			return 0;
	}

	return CallWindowProc( gpfOrigBrushCtgryProc, hWnd, msg, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------

/*!
	アイテムリストビューサブクラス
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	msg		ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
*/
LRESULT CALLBACK gpfBrushItemProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	INT	id;

	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_NOTIFY, Blv_OnNotify );	//	コモンコントロールの個別イベント

		case WM_COMMAND:
			id = LOWORD(wParam);
			switch( id )
			{
				case IDM_WINDOW_CHANGE:
				case IDM_WINDOW_CHG_RVRS:
				case IDM_TMPL_GRID_INCREASE:
				case IDM_TMPL_GRID_DECREASE:
				case IDM_TMPLT_GROUP_PREV:
				case IDM_TMPLT_GROUP_NEXT:
					SendMessage( ghBrTmplWnd, WM_COMMAND, wParam, lParam );
					return 0;
			}
			break;
	}

	return CallWindowProc( gpfOrigBrushItemProc, hWnd, msg, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------

/*!
	ノーティファイメッセージの処理
	@param[in]	hWnd		親ウインドウのハンドル
	@param[in]	idFrom		NOTIFYを発生させたコントロールのＩＤ
	@param[in]	pstNmhdr	NOTIFYの詳細
	@return		処理した内容とか
*/
LRESULT Blv_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{
	HWND	hLvWnd;
	INT		iPos, iItem, nmCode, iSubItem, iDot;
	INT_PTR	items;
	TCHAR	atItem[SUB_STRING];
	LVHITTESTINFO	stHitTestInfo;
	LPNMLISTVIEW	pstLv;
	LPNMTTDISPINFO	pstDispInfo;

	pstLv = (LPNMLISTVIEW)pstNmhdr;

	//	リストビュー自体のプロシージャなので
	hLvWnd = hWnd;
	nmCode = pstLv->hdr.code;

	if( TTN_GETDISPINFO == nmCode )
	{
		if( IDLV_BT_ITEMVIEW == idFrom )
		{
			ZeroMemory( &stHitTestInfo, sizeof(LVHITTESTINFO) );
			GetCursorPos( &(stHitTestInfo.pt) );
			ScreenToClient( hLvWnd, &(stHitTestInfo.pt) );
			ListView_SubItemHitTest( hLvWnd, &stHitTestInfo );

			iItem = stHitTestInfo.iItem;
			iSubItem = stHitTestInfo.iSubItem;
			iPos = iItem * gBrhClmCnt + iSubItem;

			TRACE( TEXT("BLvTT[%d]"), iPos );

			pstDispInfo = (LPNMTTDISPINFO)pstNmhdr;

			ZeroMemory( &(pstDispInfo->szText), sizeof(pstDispInfo->szText) );
			pstDispInfo->lpszText = pstDispInfo->szText;

			if( 0 < gvcBrTmpls.size( ) )
			{
				items = gvcBrTmpls.at( gNowGroup ).vcItems.size( );

				if( 0 <= iPos && iPos <  items )	//	なんか選択した
				{
					StringCchCopy( atItem, SUB_STRING, gvcBrTmpls.at( gNowGroup ).vcItems.at( iPos ).c_str( ) );
					iDot = ViewStringWidthGet( atItem );

					StringCchPrintf( pstDispInfo->szText, 80, TEXT("%s [%d Dot]"), atItem, iDot );
				}
			}

			return 0;
		}
	}

	return CallWindowProc( gpfOrigBrushItemProc, hWnd, WM_NOTIFY, (WPARAM)idFrom, (LPARAM)pstNmhdr );
}
//-------------------------------------------------------------------------------------------------
