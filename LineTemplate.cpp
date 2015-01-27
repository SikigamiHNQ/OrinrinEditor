/*! @file
	@brief 壱行テンプレートの面倒見ます
	このファイルは LineTemplate.cpp です。
	@author	SikigamiHNQ
	@date	2011/06/17
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

#define LINETEMPLATE_CLASS	TEXT("LINE_TEMPLATE")
#define LT_WIDTH	240
#define LT_HEIGHT	240
//-------------------------------------------------------------------------------------------------

//	ラインテンプレ・クルックまってるやり方だと遅い・サブクラスでWM_LBUTTONDOWNを処理する？
#define LTP_CLICK_NEW
//-------------------------------------------------------------------------------------------------

extern HFONT	ghAaFont;		//	AA用フォント
extern HFONT	ghNameFont;		//	ファイルタブ用フォント

extern INT		gbTmpltDock;	//	頁・壱行テンプレのドッキング
extern BOOLEAN	gbDockTmplView;	//	くっついてるテンプレは見えているか

//extern  HWND	ghMainSplitWnd;	//	メインのスプリットバーハンドル
extern  LONG	grdSplitPos;	//	スプリットバーの、左側の、画面右からのオフセット


static  ATOM	gTmpleAtom;		//!<	
static  HWND	ghTmpleWnd;		//!<	このウインドウハンドル

static  HWND	ghCtgryBxWnd;	//!<	カテゴリコンボックス
static  HWND	ghLvItemWnd;	//!<	アイテム一覧リストビュー
static  HWND	ghLnLvTipWnd;	//!<	壱行リストツールチップ

static  HWND	ghDockTabWnd;	//!<	ドッキングしたときの選択肢タブ


static  UINT	gNowGroup;		//!<	今みてるグループ番号

static  UINT	gLnClmCnt;	//!<	表示カラム数

static WNDPROC	gpfOrigLineCtgryProc;	//!<	
static WNDPROC	gpfOrigLineItemProc;	//!<	

static vector<AATEMPLATE>	gvcTmples;	//!<	テンプレの保持
//-------------------------------------------------------------------------------------------------

LRESULT	CALLBACK LineTmpleProc( HWND, UINT, WPARAM, LPARAM );	//!<	
VOID	Ltp_OnCommand( HWND , INT, HWND, UINT );	//!<	
VOID	Ltp_OnSize( HWND , UINT, INT, INT );	//!<	
VOID	Ltp_OnContextMenu( HWND, HWND, UINT, UINT );	//!<	
#ifndef LTP_CLICK_NEW
LRESULT	Ltp_OnNotify( HWND , INT, LPNMHDR );	//!<	
#endif

UINT	CALLBACK LineTmpleItemData( LPTSTR, LPCTSTR, INT );	//!<	

HRESULT	LineTmpleItemListOn( UINT );	//!<	
HRESULT	LineTmpleItemReload( HWND );	//!<	

HRESULT	TemplateItemSplit( LPTSTR, UINT, PAGELOAD );	//!<	
HRESULT	TemplateItemScatter( LPCTSTR, INT, PAGELOAD );	//!<	

LRESULT	CALLBACK gpfLineCtgryProc( HWND, UINT, WPARAM, LPARAM );	//!<	
LRESULT	CALLBACK gpfLineItemProc(  HWND, UINT, WPARAM, LPARAM );	//!<	
LRESULT	Ltl_OnNotify( HWND , INT, LPNMHDR );						//!<	
#ifdef LTP_CLICK_NEW
VOID	Ltl_OnMouseButtonUp( HWND, UINT, INT, INT, UINT );			//!<	
#endif

HWND	DockingTabCreate( HINSTANCE, HWND, LPRECT );	//!<	
//-------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------

/*!
	壱行テンプレウインドウの作成
	@param[in]	hInstance	アプリのインスタンス
	@param[in]	hParentWnd	親ウインドウのハンドル
	@param[in]	pstFrame	メインクライヤント領域
	@return		作ったビューのウインドウハンドル
*/
HWND LineTmpleInitialise( HINSTANCE hInstance, HWND hParentWnd, LPRECT pstFrame )
{

	WNDCLASSEX	wcex;
	RECT		wdRect, clRect, rect, cbxRect;
	UINT_PTR	dItems, i;
	DWORD		dwExStyle, dwStyle;
	HWND		hPrWnd;
	INT			spPos;

	TTTOOLINFO	stToolInfo;
	LVCOLUMN	stLvColm;


	ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= LineTmpleProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= LINETEMPLATE_CLASS;
	wcex.hIconSm		= NULL;

	gTmpleAtom = RegisterClassEx( &wcex );

//テンプレデータ読み出し
	TemplateItemLoad( AA_LIST_FILE, LineTmpleItemData );


	InitWindowPos( INIT_LOAD, WDP_LNTMPL, &rect );
	if( 0 == rect.right || 0 == rect.bottom )	//	幅高さが０はデータ無し
	{
		GetWindowRect( hParentWnd, &wdRect );
		rect.left   = wdRect.right;
		rect.top    = wdRect.top;
		rect.right  = LT_WIDTH;
		rect.bottom = LT_HEIGHT;
		InitWindowPos( INIT_SAVE, WDP_LNTMPL, &rect );//起動時保存
	}

	//	カラム数確認
	gLnClmCnt = InitParamValue( INIT_LOAD, VL_LINETMP_CLM, 4 );

	if( gbTmpltDock )
	{
		spPos = grdSplitPos - SPLITBAR_WIDTH;	//	右からのオフセット

		hPrWnd    = hParentWnd;
		dwExStyle = 0;
		dwStyle   = WS_CHILD | WS_VISIBLE;

		rect = *pstFrame;	//	クライヤントに使える領域
		rect.left  = rect.right - spPos;
		rect.right = PLIST_DOCK;
		rect.bottom >>= 1;
		rect.top    += rect.bottom;

		//	ブラシと切換タブを作成
		ghDockTabWnd = DockingTabCreate( hInstance, hPrWnd, &rect );
	}
	else
	{
		//	常に最全面に表示を？
		dwExStyle = WS_EX_TOOLWINDOW;
		if( InitWindowTopMost( INIT_LOAD, WDP_LNTMPL, 0 ) ){	dwExStyle |=  WS_EX_TOPMOST;	}
		dwStyle = WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_VISIBLE | WS_SYSMENU;
		hPrWnd = NULL;
	}

	//	ウインドウ作成
	ghTmpleWnd = CreateWindowEx( dwExStyle, LINETEMPLATE_CLASS, TEXT("Line Template"),
		dwStyle, rect.left, rect.top, rect.right, rect.bottom, hPrWnd, NULL, hInstance, NULL);

	GetClientRect( ghTmpleWnd, &clRect );

	//	カテゴリコンボックス
	ghCtgryBxWnd = CreateWindowEx( 0, WC_COMBOBOX, TEXT("category"),
		WS_CHILD | WS_VISIBLE | WS_BORDER | CBS_DROPDOWNLIST,
		0, 0, clRect.right, 127, ghTmpleWnd,
		(HMENU)IDCB_LT_CATEGORY, hInstance, NULL );

	gpfOrigLineCtgryProc = SubclassWindow( ghCtgryBxWnd, gpfLineCtgryProc );

	dItems = gvcTmples.size( );
	for( i = 0; dItems > i; i++ )
	{
		ComboBox_AddString( ghCtgryBxWnd, gvcTmples.at( i ).atCtgryName );
	}
	ComboBox_SetCurSel( ghCtgryBxWnd, 0 );
	gNowGroup = 0;

	GetClientRect( ghCtgryBxWnd, &cbxRect );

	//	アイテム一覧リストビュー
	ghLvItemWnd = CreateWindowEx( 0, WC_LISTVIEW, TEXT("lineitem"),
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LVS_REPORT | LVS_NOSORTHEADER | LVS_NOCOLUMNHEADER,
		0, cbxRect.bottom, clRect.right, clRect.bottom - cbxRect.bottom,
		ghTmpleWnd, (HMENU)IDLV_LT_ITEMVIEW, hInstance, NULL );
	ListView_SetExtendedListViewStyle( ghLvItemWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_LABELTIP | LVS_EX_ONECLICKACTIVATE );

	SetWindowFont( ghLvItemWnd, ghAaFont, TRUE );

	gpfOrigLineItemProc = SubclassWindow( ghLvItemWnd, gpfLineItemProc );	//	サブクラス

	ZeroMemory( &stLvColm, sizeof(LVCOLUMN) );
	stLvColm.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	stLvColm.fmt      = LVCFMT_LEFT;
	stLvColm.pszText  = TEXT("Item");
	stLvColm.cx       = 10;	//	後で合わせるので適当で良い
	for( i = 0; gLnClmCnt > i; i++ )
	{
		stLvColm.iSubItem = i;
		ListView_InsertColumn( ghLvItemWnd, i, &stLvColm );
	}

	LineTmpleItemListOn( 0 );	//	中身追加

	//	リストビューツールチップ
	ghLnLvTipWnd = CreateWindowEx( WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, ghTmpleWnd, NULL, hInstance, NULL );
//	SetWindowFont( ghLnLvTipWnd, GetStockFont(DEFAULT_GUI_FONT), FALSE );

	//	ツールチップをコールバックで割り付け
	ZeroMemory( &stToolInfo, sizeof(TTTOOLINFO) );
	stToolInfo.cbSize   = sizeof(TTTOOLINFO);
	stToolInfo.uFlags   = TTF_SUBCLASS;
	stToolInfo.hinst    = NULL;	//	
	stToolInfo.hwnd     = ghLvItemWnd;
	stToolInfo.uId      = IDLV_LT_ITEMVIEW;
	GetClientRect( ghLvItemWnd, &stToolInfo.rect );
	stToolInfo.lpszText = LPSTR_TEXTCALLBACK;	//	コレを指定するとコールバックになる
	SendMessage( ghLnLvTipWnd, TTM_ADDTOOL, 0, (LPARAM)&stToolInfo );
	SendMessage( ghLnLvTipWnd, TTM_SETMAXTIPWIDTH, 0, 0 );	//	チップの幅。０設定でいい。これしとかないと改行されない


	ShowWindow( ghTmpleWnd, SW_SHOW );
	UpdateWindow( ghTmpleWnd );


	return ghTmpleWnd;
}
//-------------------------------------------------------------------------------------------------

/*!
	ドッキング状態で発生・ドッキングしてる内容切換タブ
	@param[in]	hInst	実存ハンドル
	@param[in]	hPrWnd	親ウインドウハンドル
	@param[in]	pstRect	使えるサイズ・これの上部にくっつける
*/
HWND DockingTabCreate( HINSTANCE hInst, HWND hPrWnd, LPRECT pstRect )
{
	HWND	hWorkWnd;
	RECT	itRect;
	TCITEM	stTcItem;

	hWorkWnd = CreateWindowEx( 0, WC_TABCONTROL, TEXT("dockseltab"),
		WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | TCS_TABS | TCS_SINGLELINE,
		pstRect->left, pstRect->top, pstRect->right, 10, hPrWnd, (HMENU)IDTB_DOCK_TAB, hInst, NULL );	//	TCS_SINGLELINE
	SetWindowFont( hWorkWnd, ghNameFont, FALSE );

	ZeroMemory( &stTcItem, sizeof(stTcItem) );
	stTcItem.mask = TCIF_TEXT;
	stTcItem.pszText = TEXT("壱行");	TabCtrl_InsertItem( hWorkWnd, 0, &stTcItem );
	stTcItem.pszText = TEXT("ブラシ");	TabCtrl_InsertItem( hWorkWnd, 1, &stTcItem );

	//	選ばれしファイルをタブ的に追加？　タブ幅はウインドウ幅

	TabCtrl_GetItemRect( hWorkWnd, 1, &itRect );
	itRect.bottom  += itRect.top;
	MoveWindow( hWorkWnd, pstRect->left, pstRect->top, pstRect->right, itRect.bottom, TRUE );

	pstRect->top    += itRect.bottom;
	pstRect->bottom -= itRect.bottom;

	return hWorkWnd;
}
//-------------------------------------------------------------------------------------------------

VOID DockingTabSizeGet( LPRECT pstRect )
{
	ZeroMemory( pstRect, sizeof(RECT) );

	if( ghDockTabWnd )
	{
		GetWindowRect( ghDockTabWnd, pstRect );
		pstRect->right -= pstRect->left;
		pstRect->bottom -= pstRect->top;
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	壱行ブラシタブのコンテキストメニューか？
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	hWndContext	コンテキストが発生したウインドウのハンドル
	@param[in]	xPos		スクリーンＸ座標
	@param[in]	yPos		スクリーンＹ座業
	@return	HRESULT	S_OK処理した　E_ABORT関係なかった
*/
HRESULT DockingTabContextMenu( HWND hWnd, HWND hWndContext, LONG xPos, LONG yPos )
{
	HMENU	hPopupMenu = NULL;

	//	関係ないなら何もしない
	if( hWndContext != ghDockTabWnd ){	return  E_ABORT;	}

	hPopupMenu = CreatePopupMenu(  );

	if( gbDockTmplView )	AppendMenu( hPopupMenu, MF_STRING, IDM_LINE_BRUSH_TMPL_VIEW, TEXT("テンプレ非表示") );
	else					AppendMenu( hPopupMenu, MF_STRING, IDM_LINE_BRUSH_TMPL_VIEW, TEXT("テンプレ表示") );

	TrackPopupMenu( hPopupMenu, 0, xPos, yPos, 0, hWnd, NULL );
	DestroyMenu( hPopupMenu );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	DOCKINGテンプレ選択タブのハンドル確保
	@return	ハンドル、分離状態ならNULLが返る
*/
HWND DockingTabGet( VOID )
{
	if( gbTmpltDock )	return ghDockTabWnd;

	return NULL;
}
//-------------------------------------------------------------------------------------------------

/*!
	フローティング壱行テンプレの位置リセット
	@param[in]	hMainWnd	メインウインドウハンドル
	@return	HRESULT	終了状態コード
*/
HRESULT LineTmplePositionReset( HWND hMainWnd )
{
	RECT	wdRect, rect;

	GetWindowRect( hMainWnd, &wdRect );
	rect.left   = wdRect.right;
	rect.top    = wdRect.top;
	rect.right  = LT_WIDTH;
	rect.bottom = LT_HEIGHT;

	SetWindowPos( ghTmpleWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW | SWP_NOZORDER );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------


/*!
	ドッキング状態で発生・くっついてるウインドウがリサイズされたら
	@param[in]	hPrntWnd	くっついてるウインドウハンドル
	@param[in]	pstFrame	使えるサイズ
*/
VOID LineTmpleResize( HWND hPrntWnd, LPRECT pstFrame )
{
	RECT	rect, tbRect;


	rect = *pstFrame;	//	クライヤントに使える領域
	rect.left    = rect.right - (grdSplitPos - SPLITBAR_WIDTH);
	rect.right   = (grdSplitPos - SPLITBAR_WIDTH);

	if( gbDockTmplView )	//	壱行ブラシテンプレ見えてる
	{
		rect.bottom >>= 1;	//半分のところに配置
		rect.top    += rect.bottom;	//	オフセット

		GetWindowRect( ghDockTabWnd, &tbRect );

		tbRect.left    = rect.left;
		tbRect.right   = (grdSplitPos - SPLITBAR_WIDTH);	//	幅
		tbRect.bottom -= tbRect.top;	//	高さ
		tbRect.top     = rect.top;
		MoveWindow( ghDockTabWnd, tbRect.left, tbRect.top, tbRect.right, tbRect.bottom, TRUE );
	}
	else
	{
		DockingTabSizeGet( &tbRect );

		tbRect.left    = rect.left;	//	左位置
		tbRect.right   = (grdSplitPos - SPLITBAR_WIDTH);	//	幅
	//	tbRect.bottom -= tbRect.top;	//	高さ
		tbRect.top     = rect.top + (rect.bottom - tbRect.bottom);

		MoveWindow( ghDockTabWnd, tbRect.left, tbRect.top, tbRect.right, tbRect.bottom, TRUE );
		return;

	}

	rect.top    += tbRect.bottom;
	rect.bottom -= tbRect.bottom;
	SetWindowPos( ghTmpleWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_NOZORDER );

	return;
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
LRESULT CALLBACK LineTmpleProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_SIZE,        Ltp_OnSize );	
		HANDLE_MSG( hWnd, WM_COMMAND,     Ltp_OnCommand );	
		HANDLE_MSG( hWnd, WM_CONTEXTMENU, Ltp_OnContextMenu );
#ifndef LTP_CLICK_NEW
		HANDLE_MSG( hWnd, WM_NOTIFY,      Ltp_OnNotify );	//	コモンコントロールの個別イベント
#endif

		case WM_MOUSEWHEEL:	SendMessage( ghLvItemWnd, WM_MOUSEWHEEL, wParam, lParam );	return 0;

		case WM_CLOSE:	ShowWindow( ghTmpleWnd, SW_HIDE );	return 0;

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
VOID Ltp_OnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify )
{
	INT		rslt;
	UINT	dClm;
	LONG_PTR	rdExStyle;

	switch( id )
	{
		case IDCB_LT_CATEGORY:
			if( CBN_SELCHANGE == codeNotify )
			{
				rslt = ComboBox_GetCurSel( ghCtgryBxWnd );
				gNowGroup = rslt;

				LineTmpleItemListOn( rslt );
			}
			break;

		case  IDM_WINDOW_CHANGE:	WindowFocusChange( WND_LINE,  1 );	break;
		case  IDM_WINDOW_CHG_RVRS:	WindowFocusChange( WND_LINE, -1 );	break;

		case IDM_TMPL_GRID_INCREASE:
		case IDM_TMPL_GRID_DECREASE:
			dClm = TemplateGridFluctuate( ghLvItemWnd, ((IDM_TMPL_GRID_INCREASE == id) ? 1 : -1) );
			if( dClm )
			{
				gLnClmCnt = dClm;
				LineTmpleItemListOn( gNowGroup );
				InitParamValue( INIT_SAVE, VL_LINETMP_CLM, gLnClmCnt );
			}
			break;

		case IDM_TMPLT_GROUP_PREV:
			if( 0 < gNowGroup )
			{
				gNowGroup--;
				ComboBox_SetCurSel( ghCtgryBxWnd, gNowGroup );
				LineTmpleItemListOn( gNowGroup );
			}
			break;

		case IDM_TMPLT_GROUP_NEXT:
			if( (gNowGroup + 1) < gvcTmples.size() )
			{
				gNowGroup++;
				ComboBox_SetCurSel( ghCtgryBxWnd, gNowGroup );
				LineTmpleItemListOn( gNowGroup );
			}
			break;

		case IDM_TOPMOST_TOGGLE:	//	常時最全面と通常ウインドウのトグル
			rdExStyle = GetWindowLongPtr( hWnd, GWL_EXSTYLE );
			if( WS_EX_TOPMOST & rdExStyle )
			{
				SetWindowPos( hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
				InitWindowTopMost( INIT_SAVE, WDP_LNTMPL, 0 );
			}
			else
			{
				SetWindowPos( hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
				InitWindowTopMost( INIT_SAVE, WDP_LNTMPL, 1 );
			}
			break;

		case IDM_TMPLGROUPSTYLE_TGL:
			break;

		//	テンプレファイルリロード
		case IDM_TMPLT_RELOAD:	LineTmpleItemReload( hWnd );	break;

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
	@return		なし
*/
VOID Ltp_OnSize( HWND hWnd, UINT state, INT cx, INT cy )
{
	LONG	width;
	UINT	i;
	RECT	cbxRect, rect;

	MoveWindow( ghCtgryBxWnd, 0, 0, cx, 127, TRUE );
	GetClientRect( ghCtgryBxWnd, &cbxRect );

	MoveWindow( ghLvItemWnd, 0, cbxRect.bottom, cx, cy - cbxRect.bottom, TRUE );

	GetClientRect( ghLvItemWnd, &rect );
	width  = rect.right / gLnClmCnt;	//	表示カラム数なので０になることはない

	for( i = 0; gLnClmCnt > i; i++ )
	{
		ListView_SetColumnWidth( ghLvItemWnd, i, width );
	}

	return;
}
//-------------------------------------------------------------------------------------------------

#ifndef LTP_CLICK_NEW
/*!
	ノーティファイメッセージの処理
	@param[in]	hWnd		親ウインドウのハンドル
	@param[in]	idFrom		NOTIFYを発生させたコントロールのＩＤ
	@param[in]	pstNmhdr	NOTIFYの詳細
	@return		処理した内容とか
*/
LRESULT Ltp_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{
	HWND	hLvWnd;
	INT		iPos, iItem, nmCode, iSubItem;
	INT_PTR	items;
	TCHAR	atItem[SUB_STRING];
	LPNMLISTVIEW	pstLv;
	LVHITTESTINFO	stHitTestInfo;

	if( IDLV_LT_ITEMVIEW == idFrom )
	{
		pstLv = (LPNMLISTVIEW)pstNmhdr;

		hLvWnd = pstLv->hdr.hwndFrom;
		nmCode = pstLv->hdr.code;

		//	普通のクルックについて
		if( NM_CLICK == nmCode )
		{
			stHitTestInfo.pt = pstLv->ptAction;
			ListView_SubItemHitTest( hLvWnd, &stHitTestInfo );

			iItem = stHitTestInfo.iItem;
			iSubItem = stHitTestInfo.iSubItem;
			iPos = iItem * gLnClmCnt + iSubItem;

			if( 0 < gvcTmples.size() )
			{
				items = gvcTmples.at( gNowGroup ).vcItems.size( );

				TRACE( TEXT("LINE TMPL[%d x %d]"), iItem, iSubItem );

				if( 0 <= iPos && iPos <  items )	//	なんか選択した
				{
					StringCchCopy( atItem, SUB_STRING, gvcTmples.at( gNowGroup ).vcItems.at( iPos ).c_str( ) );
					ViewInsertTmpleString( atItem );	//	挿入処理

					ViewFocusSet(  );	//	20110720	フォーカスを描画に戻す
				}
			}
			else
			{
				ViewFocusSet(  );	//	20110720	フォーカスを描画に戻す
			}
		}
	}

	return 0;	//	何もないなら０を戻す
}
//-------------------------------------------------------------------------------------------------
#endif

/*!
	コンテキストメニュー呼びだしアクション(要は右クルック）
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	hWndContext	コンテキストが発生したウインドウのハンドル
	@param[in]	xPos		スクリーンＸ座標
	@param[in]	yPos		スクリーンＹ座業
	@return		無し
*/
VOID Ltp_OnContextMenu( HWND hWnd, HWND hWndContext, UINT xPos, UINT yPos )
{
	HMENU	hMenu, hSubMenu;
	UINT	dRslt;
	LONG_PTR	rdExStyle;

	POINT	stPoint;


	stPoint.x = (SHORT)xPos;	//	画面座標はマイナスもありうる
	stPoint.y = (SHORT)yPos;

	hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_TEMPLATE_POPUP) );
	hSubMenu = GetSubMenu( hMenu, 0 );
//	AppendMenu( hSubMenu, MF_STRING, IDM_TMPLGROUPSTYLE_TGL, TEXT("カテゴリ表示切替") );
	//準備中

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
	@return	UINT	特に意味はない
*/
UINT CALLBACK LineTmpleItemData( LPTSTR ptName, LPCTSTR ptLine, INT cchSize )
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
		gvcTmples.push_back( cstItem );
	}

	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	アイテムをコンボックスとリストに展開
	@param[in]	listNum	展開するセット番号・０インデックス
	@return		HRESULT	終了状態コード
*/
HRESULT LineTmpleItemListOn( UINT listNum )
{
	INT			width;
	UINT_PTR	i, items;
	TCHAR		atItem[SUB_STRING];
	LVITEM		stLvi;
	RECT		rect;

	ZeroMemory( atItem, sizeof(atItem) );

	if( 0 >= gvcTmples.size() ){	return E_OUTOFMEMORY;	}

	items = gvcTmples.at( listNum ).vcItems.size( );

	TRACE( TEXT("LINE open NUM[%u] ITEM[%u] GRID[%d]"), listNum, items, gLnClmCnt );

	ListView_DeleteAllItems( ghLvItemWnd );

	ZeroMemory( &stLvi, sizeof(stLvi) );
	stLvi.mask    = LVIF_TEXT;
	stLvi.pszText = atItem;

	for( i = 0; items > i; i++ )
	{
		StringCchCopy( atItem, SUB_STRING, gvcTmples.at( listNum ).vcItems.at( i ).c_str( ) );

		stLvi.iItem     = i / gLnClmCnt;	//	表示カラム数なので０になることはない
		stLvi.iSubItem  = i % gLnClmCnt;
		if( 0 == stLvi.iSubItem )	ListView_InsertItem( ghLvItemWnd, &stLvi );
		else						ListView_SetItem( ghLvItemWnd, &stLvi );
	}

	//	ブチこんだら幅調整
	GetClientRect( ghLvItemWnd, &rect );
	width  = rect.right / gLnClmCnt;	//	表示カラム数なので０になることはない
	for( i = 0; gLnClmCnt > i; i++ ){	ListView_SetColumnWidth( ghLvItemWnd, i, width );	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	アイテムファイル最読込
	@param[in]	hWnd		ウインドウハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT LineTmpleItemReload( HWND hWnd )
{
	TEMPL_ITR	itTmpl;
	

	gNowGroup = 0;	//	とりあえず０に戻す

	for( itTmpl = gvcTmples.begin( ); gvcTmples.end( ) != itTmpl; itTmpl++ ){	itTmpl->vcItems.clear();	}
	gvcTmples.clear(  );	//	一旦内容破壊
	
	//	カテゴリコンボックスの中身を全破壊
	while( ComboBox_GetCount( ghCtgryBxWnd )  ){	ComboBox_DeleteString( ghCtgryBxWnd, 0 );	}

	TemplateItemLoad( AA_LIST_FILE, LineTmpleItemData );	//	再びロード

	for( itTmpl = gvcTmples.begin( ); gvcTmples.end( ) != itTmpl; itTmpl++ )
	{
		ComboBox_AddString( ghCtgryBxWnd, itTmpl->atCtgryName );
	}
	ComboBox_SetCurSel( ghCtgryBxWnd, 0 );

	LineTmpleItemListOn( 0 );	//	０頁を表示

	return S_OK;
}
//-------------------------------------------------------------------------------------------------





/*!
	ファイルを開けてデータ確保開始
	@param[in]	ptFileName	開くファイルの名前
	@param[in]	pfCalling	受け取ったデータを処理する函数へのポインター
	@return		HRESULT	終了状態コード
*/
HRESULT TemplateItemLoad( LPTSTR ptFileName, PAGELOAD pfCalling )
{
	CONST WCHAR rtHead = 0xFEFF;	//	ユニコードテキストヘッダ
	WCHAR	rtUniBuf;

	HANDLE	hFile;
	DWORD	readed;

	LPVOID	pBuffer;	//	文字列バッファ用ポインター
	INT		iByteSize;

	LPTSTR	ptString;
	LPSTR	pcText;
	UINT	cchSize;

	TCHAR	atFileName[MAX_PATH];

	StringCchCopy( atFileName, MAX_PATH, ExePathGet() );
	PathAppend( atFileName, TEMPLATE_DIR );
	PathAppend( atFileName, ptFileName );

	hFile = CreateFile( atFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile ){	return E_HANDLE;	}

	iByteSize = GetFileSize( hFile, NULL );
	pBuffer = malloc( iByteSize + 2 );
	ZeroMemory( pBuffer, iByteSize + 2 );

	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	ReadFile( hFile, pBuffer, iByteSize, &readed, NULL );
	CloseHandle( hFile );	//	内容全部取り込んだから開放

	//	ユニコードチェック
	CopyMemory( &rtUniBuf, pBuffer, 2 );
	if( rtHead == rtUniBuf )	//	ユニコードヘッダがあれば
	{	//	普通はユニコードじゃない
		ptString = (LPTSTR)pBuffer;
		ptString++;	//	ユニコードヘッダ分進めておく
	}
	else	//	多分SJISであるなら
	{
		pcText = (LPSTR)pBuffer;
		ptString = SjisDecodeAlloc( pcText );	//	SJISの内容をユニコードにする

		FREE( pBuffer );	//	こっちで開放
		pBuffer = ptString;	//	ポイントするところを変更
	}

	StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSize );

	TemplateItemSplit( ptString, cchSize, pfCalling );

	FREE( pBuffer );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ListNameなユニコード文字列を受け取って分解しつつページに入れる
	@param[in]	ptStr		分解対象文字列へのポインター
	@param[in]	cchSize		その文字列の文字数
	@param[in]	pfCalling	受け取ったデータを処理する函数へのポインター
	@return		HRESULT		終了状態コード
*/
HRESULT TemplateItemSplit( LPTSTR ptStr, UINT cchSize, PAGELOAD pfCalling )
{
	LPCTSTR	ptCaret;	//	読込開始・現在位置
	LPCTSTR	ptStart;	//	セパレータの直前
	LPTSTR	ptEnd;
	UINT	iNumber;	//	通し番号カウント
	UINT	cchItem;
//	INT		dmyX = 0, dmyY = 0;
	BOOLEAN	bLast;
	TCHAR	atName[MAX_PATH];

	ptCaret = ptStr;	//	まずは最初から

	iNumber = 0;	//	通し番号０インデックス

	bLast = FALSE;

	//	最初の[ListName=まで移動
	ptEnd = StrStr( ptCaret, TMPLE_BEGINW );
	if( !ptEnd )	return E_INVALIDARG;	//	ファイルの中身が違う

	ptCaret = ptEnd;

	do	//	
	{
		ptStart = NextLineW( ptCaret );	//	次の行からが本番
		if( !ptStart )	return  S_FALSE;	//	見つからなかったら

		ptEnd = StrStr( ptCaret, TEXT("=") );	//	=
		ptCaret = ptEnd + 1;	//	名前の所まですすむ
		cchItem = ptStart - ptCaret;	//	名前部分の文字数
		cchItem -= 3;	//	]rn

		ZeroMemory( atName, sizeof(atName) );	//	名前確保
		if( 0 < cchItem ){	StringCchCopyN( atName, MAX_PATH, ptCaret, cchItem );	}
		else{	StringCchPrintf( atName, MAX_PATH, TEXT("Nameless%d"), iNumber );	}

		StringCchLength( atName, MAX_PATH, &cchItem );
		pfCalling( atName, NULL, cchItem );

		ptCaret = ptStart;	//	本体部分

		ptEnd = StrStr( ptCaret, TMPLE_ENDW );	//	終端セパレータを探す
		//	この時点でptEndは[end]をさしてる・NULLはありえない
		if( !ptEnd ){	return  S_FALSE;	}	//	見つからなかったら

		cchItem = ptEnd - ptCaret;	//	WCHAR単位なので計算結果は文字数のようだ

		TemplateItemScatter( ptCaret, cchItem, pfCalling );

		pfCalling( NULL, NULL, 0 );

		iNumber++;

		ptCaret = NextLineW( ptEnd );	//	次の行が次の開始地点

	}while( *ptCaret );	//	データ有る限りループで探す



	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	内容物をバラしてぶちこむ
	@param[in]	ptCont		項目の内容
	@param[in]	cchSize		内容の文字数
	@param[in]	pfCalling	受け取ったデータを処理する函数へのポインター
	@return		HRESULT	終了状態コード
*/
HRESULT TemplateItemScatter( LPCTSTR ptCont, INT cchSize, PAGELOAD pfCalling )
{
	//	改行で区切られた壱行単位のアイテムである
	INT	nowCaret, nYct, nXct, rtcnt;

	TCHAR	hdBuf[MAX_STRING];	//	データを確保

	ZeroMemory( hdBuf, sizeof(hdBuf) );	//	クリンナップ

	nowCaret = 0;
	rtcnt = 0;	//	改行カウント・いらねぇ？
	//スタイリッシュに本体読込
	for( nYct = 0, nXct = 0; nowCaret <= cchSize; nowCaret++, nXct++ )
	{
		if( nXct >= MAX_STRING )	nXct = MAX_STRING - 1;

		hdBuf[nXct] = ptCont[nowCaret];

		if( ( TEXT('\r') == ptCont[nowCaret] && TEXT('\n') == ptCont[nowCaret + 1]) || 0x0000 == ptCont[nowCaret] )
		{
			if( 0 == nXct )	continue;	//	ラスト改行ありならここに入る・文字列無しなら何もしない
			hdBuf[nXct] = 0x0000;	//	文字列の終点はヌル
			//	ここでデータ処理
			pfCalling( NULL, hdBuf, nXct );

			nXct = -1;	//	ループ頭で＋＋されるため、０にあうようにしておく
			nYct = 0;	//	次のエレメントにスタイリッシュに移る
			nowCaret++;	//	改行コード分をエレガントに進める

			rtcnt++;
			ZeroMemory( hdBuf, sizeof(hdBuf) );	//	クリンナップ
		}
	}

	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	格子の数を増減させる
	@param[in]	hLvWnd	対象のリストビューのハンドル
	@param[in]	dFluct	正：増やす　負：減らす
	@return	０：操作無し　１～：その数になった
*/
UINT TemplateGridFluctuate( HWND hLvWnd, INT dFluct )
{
	INT	clmCount, clmNew, i;
	LVCOLUMN	stLvColm;

	if( 0 == dFluct )	return 0;

	//	カラムの数は、リストビューのヘッダーからゲットせないかん
	clmCount = Header_GetItemCount( ListView_GetHeader(hLvWnd) );

	TRACE( TEXT("カラム増減[%u][%d]"), clmCount, dFluct );

	if( 0 > dFluct && 1 >= clmCount )	return 0;	//	１より減らせない

	clmNew =  clmCount + dFluct;	//	カラム数調整して

	if( 0 > dFluct )
	{
		for( i = clmCount; clmNew < i; i-- )
		{
			ListView_DeleteColumn( hLvWnd, (i-1) );
		}
	}

	if( 0 < dFluct )
	{
		ZeroMemory( &stLvColm, sizeof(LVCOLUMN) );
		stLvColm.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		stLvColm.fmt      = LVCFMT_LEFT;
		stLvColm.pszText  = TEXT("Item");
		stLvColm.cx       = 10;	//	あとで調整するので適当で
		for( i = clmCount; clmNew > i; i++ )
		{
			stLvColm.iSubItem = i;
			ListView_InsertColumn( hLvWnd, i, &stLvColm );
		}
	}

	return clmNew;
}
//-------------------------------------------------------------------------------------------------

/*!
	カテゴリコンボックスサブクラス
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	msg		ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@return	なんか処理した結果
*/
LRESULT CALLBACK gpfLineCtgryProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
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
					SendMessage( ghTmpleWnd, WM_COMMAND, wParam, lParam );
					return 0;
			}
			break;

		case WM_MOUSEWHEEL:	//	カテゴリコンボックスでのWHEELでページ送りなると面倒なのでリストビューに送る
			SendMessage( ghLvItemWnd, WM_MOUSEWHEEL, wParam, lParam );
			return 0;

	}

	return CallWindowProc( gpfOrigLineCtgryProc, hWnd, msg, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------

/*!
	アイテムリストビューサブクラス
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	msg		ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@return	なんか処理した結果
*/
LRESULT CALLBACK gpfLineItemProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	INT	id;

//	Ctrl押しながらマウスホイール廻るとまずい？

	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_NOTIFY, Ltl_OnNotify );	//	コモンコントロールの個別イベント

#ifdef LTP_CLICK_NEW
		case WM_LBUTTONDOWN:	//	この部分がないとクルックに反応しない
		case WM_MBUTTONDOWN:
			TRACE( TEXT("LTL_MOUSenAN") );
			return 0;

		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
			Ltl_OnMouseButtonUp( hWnd, msg, (INT)(SHORT)LOWORD(lParam), (INT)(SHORT)HIWORD(lParam), (UINT)(wParam) );
			return 0;
#endif
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
					SendMessage( ghTmpleWnd, WM_COMMAND, wParam, lParam );
					return 0;
			}
			break;
	}

	return CallWindowProc( gpfOrigLineItemProc, hWnd, msg, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------

#ifdef LTP_CLICK_NEW
/*!
	ビューでマウスのボタンがうっｐされたとき
	@param[in]	hWnd		ウインドウハンドル・ビューのとは限らない？
	@param[in]	msg			メッセージ・押されたボタン識別
	@param[in]	x			発生したクライヤントＸ座標値
	@param[in]	y			発生したクライヤントＹ座標値
	@param[in]	keyFlags	他に押されてるキーについて
*/
VOID Ltl_OnMouseButtonUp( HWND hWnd, UINT msg, INT x, INT y, UINT keyFlags )
{
	INT		iPos, iItem, iSubItem;
	INT_PTR	items;
	//TCHAR	atItem[SUB_STRING];
	//LPTSTR	ptStr = NULL, ptItem = NULL;
	//UINT_PTR	cchSz;
	LVHITTESTINFO	stHitTestInfo;

	TRACE( TEXT("LTL_MOUSEB %d x %d"), x, y );

	ZeroMemory( &stHitTestInfo, sizeof(LVHITTESTINFO) );
	stHitTestInfo.pt.x = x;
	stHitTestInfo.pt.y = y;
	ListView_SubItemHitTest( hWnd, &stHitTestInfo );

	iItem = stHitTestInfo.iItem;
	iSubItem = stHitTestInfo.iSubItem;
	iPos = iItem * gLnClmCnt + iSubItem;
	TRACE( TEXT("LINE TMPL[%d x %d][%d]"), iItem, iSubItem, iPos );

	if( 0 < gvcTmples.size() )
	{
		items = gvcTmples.at( gNowGroup ).vcItems.size( );

		if( 0 <= iPos && iPos <  items )	//	なんか選択した
		{
			//cchSz = gvcTmples.at( gNowGroup ).vcItems.at( iPos ).size() + 1;
			//if( SUB_STRING <= cchSz )
			//{
			//	ptStr = (LPTSTR)malloc( cchSz * sizeof(TCHAR) );
			//	ZeroMemory( ptStr, cchSz * sizeof(TCHAR) );
			//	StringCchCopy( ptStr, cchSz, gvcTmples.at( gNowGroup ).vcItems.at( iPos ).c_str( ) );
			//	ptItem = ptStr;
			//}
			//else	//	配列よりデカいなら、ダイナミックにゲット
			//{
			//	StringCchCopy( atItem, SUB_STRING, gvcTmples.at( gNowGroup ).vcItems.at( iPos ).c_str( ) );
			//	ptItem = atItem;
			//}
			//if( WM_LBUTTONUP == msg )		ViewInsertTmpleString( ptItem );	//	挿入処理
			//else if( WM_MBUTTONUP == msg )	LayerBoxVisibalise( GetModuleHandle(NULL), ptItem, 0x00 );
			//FREE( ptStr );

			if( WM_LBUTTONUP == msg )
			{
				ViewInsertTmpleString( gvcTmples.at( gNowGroup ).vcItems.at( iPos ).c_str(  ) );	//	挿入処理
				ViewFocusSet(  );	//	フォーカスを描画に戻す
			}
			else if( WM_MBUTTONUP == msg )
			{
				LayerBoxVisibalise( GetModuleHandle(NULL), gvcTmples.at( gNowGroup ).vcItems.at( iPos ).c_str( ), 0x00 );
			}
		}
	}
	else
	{
		ViewFocusSet(  );	//	フォーカスを描画に戻す
	}

	return;
}
//-------------------------------------------------------------------------------------------------
#endif

/*!
	ノーティファイメッセージの処理
	@param[in]	hWnd		親ウインドウのハンドル
	@param[in]	idFrom		NOTIFYを発生させたコントロールのＩＤ
	@param[in]	pstNmhdr	NOTIFYの詳細
	@return		処理した内容とか
*/
LRESULT Ltl_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
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
	hLvWnd = hWnd;		//pstLv->hdr.hwndFrom;<--ツールチップのハンドルになってるかもだ
	nmCode = pstLv->hdr.code;

	if( TTN_GETDISPINFO == nmCode )
	{
		TRACE( TEXT("LT_TOOL %u"), idFrom );
		if( IDLV_LT_ITEMVIEW == idFrom )
		{
			ZeroMemory( &stHitTestInfo, sizeof(LVHITTESTINFO) );
			GetCursorPos( &(stHitTestInfo.pt) );
			ScreenToClient( hLvWnd, &(stHitTestInfo.pt) );
			ListView_SubItemHitTest( hLvWnd, &stHitTestInfo );

			iItem = stHitTestInfo.iItem;
			iSubItem = stHitTestInfo.iSubItem;
			iPos = iItem * gLnClmCnt + iSubItem;

			TRACE( TEXT("LLvTT[%d]"), iPos );

			pstDispInfo = (LPNMTTDISPINFO)pstNmhdr;

			ZeroMemory( &(pstDispInfo->szText), sizeof(pstDispInfo->szText) );
			pstDispInfo->lpszText = pstDispInfo->szText;

			if( 0 < gvcTmples.size( ) )
			{
				items = gvcTmples.at( gNowGroup ).vcItems.size( );

				if( 0 <= iPos && iPos <  items )	//	なんか選択した
				{
					StringCchCopy( atItem, SUB_STRING, gvcTmples.at( gNowGroup ).vcItems.at( iPos ).c_str( ) );
					iDot = ViewStringWidthGet( atItem );

					StringCchPrintf( pstDispInfo->szText, 80, TEXT("%s [%d Dot]"), atItem, iDot );
				}
			}

			return 0;
		}
	}

	return CallWindowProc( gpfOrigLineItemProc, hWnd, WM_NOTIFY, (WPARAM)idFrom, (LPARAM)pstNmhdr );
}
//-------------------------------------------------------------------------------------------------

