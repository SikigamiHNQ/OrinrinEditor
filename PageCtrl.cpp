/*! @file
	@brief ファイルのページ単位を管理します
	このファイルは PageCtrl.cpp です。
	@author	SikigamiHNQ
	@date	2011/05/20
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

//	TODO:	各頁の詳細を表示出来るようにする
//	TODO:	選択頁のみ保存とか、なんかそんな機能ほしい

#define PAGELIST_CLASS	TEXT("PAGE_LIST")
#define PL_WIDTH	110
#define PL_HEIGHT	300
//-------------------------------------------------------------------------------------------------

//	ホイホイ共有していいのだらうか

extern FILES_ITR	gitFileIt;	//	今見てるファイルの本体

extern INT		gixFocusPage;	//	注目中のページ・とりあえず０・０インデックス
extern INT		gixDropPage;	//	投下ホット番号
//-------------------------------------------------------------------------------------------------

static HINSTANCE	ghInst;		//!<	このアプリの実存値

static  ATOM	gPageAtom;		//!<	クラスアトム
static  HWND	ghPageWnd;		//!<	このウインドウのハンドル

static  HWND	ghToolWnd;		//!<	ツールバー
static  HWND	ghPageListWnd;	//!<	ページリストビューハンドル

#ifdef PGL_TOOLTIP
static  HWND	ghPageTipWnd;	//!<	ツールチップハンドル
static HFONT	ghPgTipFont;	//!<	ツールチップ用
static LPTSTR	gptPgTipBuf;	//!<	
#endif
static BOOLEAN	gbPgTipView;	//!<	頁ツールティップ表示ON/OFF

static INT		gixPreviSel;	//!<	直前まで選択してた頁

static INT		gixMouseSel;	//!<	マウスカーソル下のアレ
#ifdef PGL_TOOLTIP
static INT	gixPreSel;		//!<	マウスカーソル下のアレ
#endif

static BOOLEAN	gbPgRetFocus;	//!<	頁を選択したら編集窓にフォーカス戻すか

static WNDPROC	gpfOrigPageViewProc;	//!<	頁一覧ビューの元プロシージャ
static WNDPROC	gpfOrigPageToolProc;	//!<	ツールバーの元プロシージャ

static HIMAGELIST	ghPgLstImgLst;	//!<	

extern INT	gbTmpltDock;		//	頁・壱行テンプレのドッキング
extern BOOLEAN	gbDockTmplView;	//	くっついてるテンプレは見えているか

extern  UINT	gdPageByteMax;	//	壱レスの最大バイト数

//extern  HWND	ghMainSplitWnd;	//	メインのスプリットバーハンドル
extern  LONG	grdSplitPos;	//	スプリットバーの、左側の、画面右からのオフセット
//-------------------------------------------------------------------------------------------------

//	ツールバー・新規作成とか
#define PGTB_ITEMS	9
static TBBUTTON gstPgTlBarInfo[] = {
/*新規*/{  0,	IDM_PAGEL_ADD,		TBSTATE_ENABLED | TBSTATE_WRAP,	BTNS_BUTTON | BTNS_AUTOSIZE,	{0, 0}, 0, 0  },	//	
/*挿入*/{  1,	IDM_PAGEL_INSERT,	TBSTATE_ENABLED | TBSTATE_WRAP,	BTNS_BUTTON | BTNS_AUTOSIZE,	{0, 0}, 0, 0  },	//	
/*複製*/{  2,	IDM_PAGEL_DUPLICATE,TBSTATE_ENABLED | TBSTATE_WRAP,	BTNS_BUTTON | BTNS_AUTOSIZE,	{0, 0}, 0, 0  },	//	
/*削除*/{  3,	IDM_PAGEL_DELETE,	TBSTATE_ENABLED | TBSTATE_WRAP,	BTNS_BUTTON | BTNS_AUTOSIZE,	{0, 0}, 0, 0  },	//	
/*統合*/{  4,	IDM_PAGEL_COMBINE,	TBSTATE_ENABLED | TBSTATE_WRAP,	BTNS_BUTTON | BTNS_AUTOSIZE,	{0, 0}, 0, 0  },	//	
/*上移*/{  5,	IDM_PAGEL_UPFLOW,	TBSTATE_ENABLED | TBSTATE_WRAP,	BTNS_BUTTON | BTNS_AUTOSIZE,	{0, 0}, 0, 0  },	//	
/*下移*/{  6,	IDM_PAGEL_DOWNSINK,	TBSTATE_ENABLED | TBSTATE_WRAP,	BTNS_BUTTON | BTNS_AUTOSIZE,	{0, 0}, 0, 0  },	//	
/*名称*/{  7,	IDM_PAGEL_RENAME,	TBSTATE_ENABLED | TBSTATE_WRAP,	BTNS_BUTTON | BTNS_AUTOSIZE,	{0, 0}, 0, 0  },	//	
/*更新*/{  8,	IDM_PAGEL_DETAIL,	TBSTATE_WRAP,					BTNS_BUTTON | BTNS_AUTOSIZE,	{0, 0}, 0, 0  } 	//	
};	//	内容変更したら、ツールバー文字列の設定とかも変更セヨ

//-------------------------------------------------------------------------------------------------


LRESULT	CALLBACK PageListProc( HWND, UINT, WPARAM, LPARAM );
VOID	Plt_OnCommand( HWND, INT, HWND, UINT );
VOID	Plt_OnSize( HWND, UINT, INT, INT );
LRESULT	Plt_OnNotify( HWND, INT, LPNMHDR );
VOID	Plt_OnContextMenu( HWND, HWND, UINT, UINT );

LRESULT	PageListNotify( HWND, LPNMLISTVIEW );
HRESULT	PageListNameChange( INT );
HRESULT	PageListSpinning( HWND, INT, INT );
HRESULT	PageListDuplicate( HWND, INT );
HRESULT PageListCombine( HWND, INT );

HRESULT	PageListJump( INT );

LRESULT	CALLBACK gpfPageViewProc( HWND, UINT, WPARAM, LPARAM );
VOID	Plv_OnMouseMove( HWND, INT, INT, UINT );	//!<	
#ifdef PGL_TOOLTIP
LRESULT	Plv_OnNotify( HWND , INT, LPNMHDR );	//!<	
#endif

LRESULT	CALLBACK gpfPageToolProc( HWND, UINT, WPARAM, LPARAM );

INT_PTR	CALLBACK PageNameDlgProc( HWND, UINT, WPARAM, LPARAM );

#ifdef USE_HOVERTIP
LPTSTR	CALLBACK PageListHoverTipInfo( LPVOID );
#endif
//-------------------------------------------------------------------------------------------------


/*!
	ページ用りすとびゅ～の作成
	@param[in]	hInstance	アプリのインスタンス
	@param[in]	hParentWnd	親ウインドウのハンドル
	@param[in]	pstFrame	メインクライヤント領域
	@return		作ったビューのウインドウハンドル
*/
HWND PageListInitialise( HINSTANCE hInstance, HWND hParentWnd, LPRECT pstFrame )
{

	LVCOLUMN	stLvColm;
	RECT		tbRect;
	DWORD		dwExStyle, dwStyle;
	TCHAR		atBuff[MAX_STRING];
	HWND		hPrWnd;

	UINT		ici, resnum;
	HBITMAP		hImg, hMsq;
	INT			spPos;

#ifdef PGL_TOOLTIP
	TTTOOLINFO	stToolInfo;
	LOGFONT	stFont;
#endif
	WNDCLASSEX	wcex;
	RECT	wdRect, clRect, rect;

	ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= PageListProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;//MAKEINTRESOURCE(IDC_PGLVPOPUPMENU);
	wcex.lpszClassName	= PAGELIST_CLASS;
	wcex.hIconSm		= NULL;

	gPageAtom = RegisterClassEx( &wcex );

	ghInst = hInstance;

	gixMouseSel = -1;
#ifdef PGL_TOOLTIP
	gixPreSel = -1;
#endif
	gixPreviSel = -1;

	gbPgTipView = InitParamValue( INIT_LOAD, VL_PAGETIP_VIEW, 1 );

	//	フォーカス操作
	gbPgRetFocus = InitParamValue( INIT_LOAD, VL_PGL_RETFCS, 0 );

	InitWindowPos( INIT_LOAD, WDP_PLIST, &rect );
	if( 0 == rect.right || 0 == rect.bottom )	//	幅高さが０はデータ無し
	{
		GetWindowRect( hParentWnd, &wdRect );
		rect.left   = wdRect.left - PL_WIDTH;
		rect.top    = wdRect.top;
		rect.right  = PL_WIDTH;
		rect.bottom = PL_HEIGHT;
		InitWindowPos( INIT_SAVE, WDP_PLIST, &rect );	//	起動時保存
	}

	if( gbTmpltDock )	//	メーンウィンドーにドッキュする
	{
		spPos = grdSplitPos - SPLITBAR_WIDTH;	//	右からのオフセット

		hPrWnd    = hParentWnd;
		dwExStyle = 0;
		dwStyle   = WS_CHILD | WS_VISIBLE;
		rect = *pstFrame;	//	クライヤントに使える領域
		rect.left  = rect.right - spPos;
		rect.right = PLIST_DOCK;
		rect.bottom >>= 1;
	}
	else
	{
		dwExStyle = WS_EX_TOOLWINDOW;
		if( InitWindowTopMost( INIT_LOAD, WDP_PLIST , 0 ) ){	dwExStyle |=  WS_EX_TOPMOST;	}
		dwStyle = WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_VISIBLE | WS_SYSMENU;
		hPrWnd = NULL;
	}
	ghPageWnd = CreateWindowEx( dwExStyle, PAGELIST_CLASS, TEXT("Page List"), dwStyle,
		rect.left, rect.top, rect.right, rect.bottom, hPrWnd, NULL, hInstance, NULL);
	//	ウインドウを作成

	GetClientRect( ghPageWnd, &clRect );

	//	ツールバー・縦のやつ
	ghToolWnd = CreateWindowEx( 0, TOOLBARCLASSNAME, TEXT("pagetoolbar"),
		WS_CHILD | WS_VISIBLE | CCS_NORESIZE | CCS_LEFT | CCS_NODIVIDER | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS,// | TBSTYLE_WRAPABLE,
		0, 0, 0, 0, ghPageWnd, (HMENU)IDTB_PAGE_TOOLBAR, hInstance, NULL);
	//	自動ツールチップスタイルを追加
	SendMessage( ghToolWnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS );

	//stToolBmp.hInst = HINST_COMMCTRL;
	//stToolBmp.nID   = IDB_STD_SMALL_COLOR;
	//tbixStd = SendMessage( ghToolWnd, TB_ADDBITMAP, 0, (LPARAM)&stToolBmp );
	ghPgLstImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 9, 1 );
	resnum = IDBMPQ_PAGE_TB_FIRST;
	for( ici = 0; 9 > ici; ici++ )
	{
		hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( (resnum++) ) );
		hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( (resnum++) ) );
		ImageList_Add( ghPgLstImgLst, hImg, hMsq );	//	イメージリストにイメージを追加
		DeleteBitmap( hImg );	DeleteBitmap( hMsq );
	}
	SendMessage( ghToolWnd, TB_SETIMAGELIST, 0, (LPARAM)ghPgLstImgLst );

	SendMessage( ghToolWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );
	//	ツールチップ文字列を設定・ボタンテキストがツールチップになる
	StringCchCopy( atBuff, MAX_STRING, TEXT("末尾に新規作成\r\nAlt + Shift + I ") );	gstPgTlBarInfo[ 0].iString = SendMessage( ghToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, TEXT("選択頁の次に挿入\r\nAlt + I") );			gstPgTlBarInfo[ 1].iString = SendMessage( ghToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, TEXT("選択頁を複製\r\nAlt + C") );				gstPgTlBarInfo[ 2].iString = SendMessage( ghToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, TEXT("選択頁を削除\r\nAlt + D") );				gstPgTlBarInfo[ 3].iString = SendMessage( ghToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, TEXT("次の頁と統合\r\nAlt + G") );				gstPgTlBarInfo[ 4].iString = SendMessage( ghToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, TEXT("頁を上へ移動\r\nAlt + U") );				gstPgTlBarInfo[ 5].iString = SendMessage( ghToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, TEXT("頁を下へ移動\r\nAlt + J") );				gstPgTlBarInfo[ 6].iString = SendMessage( ghToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, TEXT("頁名称の変更\r\nAlt + N") );				gstPgTlBarInfo[ 7].iString = SendMessage( ghToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );
	StringCchCopy( atBuff, MAX_STRING, TEXT("最新の情報に更新") );						gstPgTlBarInfo[ 8].iString = SendMessage( ghToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuff );

	SendMessage( ghToolWnd, TB_SETROWS, MAKEWPARAM(PGTB_ITEMS,TRUE), (LPARAM)(&tbRect) );

	SendMessage( ghToolWnd, TB_ADDBUTTONS, (WPARAM)PGTB_ITEMS, (LPARAM)&gstPgTlBarInfo );	//	ツールバーにボタンを挿入

	SendMessage( ghToolWnd, TB_GETITEMRECT, 0, (LPARAM)(&tbRect) );
	MoveWindow( ghToolWnd, 0, 0, tbRect.right, rect.bottom, TRUE );
	InvalidateRect( ghToolWnd , NULL, TRUE );	//	クライヤント全体を再描画する

//	サブクラス化
	gpfOrigPageToolProc = SubclassWindow( ghToolWnd, gpfPageToolProc );

	tbRect.bottom  = rect.bottom;
	tbRect.left    = 0;
	tbRect.top     = 0;

//リストビュー	LVS_SHOWSELALWAYS
	ghPageListWnd = CreateWindowEx( 0, WC_LISTVIEW, TEXT("pagelist"),
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | 
#ifdef PAGE_MULTISELECT
		LVS_REPORT | LVS_NOSORTHEADER,
#else
		LVS_REPORT | LVS_NOSORTHEADER | LVS_SINGLESEL,
#endif
		tbRect.right, clRect.top, clRect.right - tbRect.right, clRect.bottom, ghPageWnd,
		(HMENU)IDLV_PAGELISTVIEW, hInstance, NULL );
	ListView_SetExtendedListViewStyle( ghPageListWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

	gpfOrigPageViewProc = SubclassWindow( ghPageListWnd, gpfPageViewProc );

	ZeroMemory( &stLvColm, sizeof(LVCOLUMN) );
	stLvColm.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	stLvColm.fmt = LVCFMT_LEFT;
	stLvColm.pszText = TEXT("No");		stLvColm.cx =  28;	stLvColm.iSubItem = 0;	ListView_InsertColumn( ghPageListWnd, 0, &stLvColm );
	stLvColm.pszText = TEXT("名");		stLvColm.cx =  67;	stLvColm.iSubItem = 1;	ListView_InsertColumn( ghPageListWnd, 1, &stLvColm );
	stLvColm.pszText = TEXT("Byte");	stLvColm.cx =  45;	stLvColm.iSubItem = 2;	ListView_InsertColumn( ghPageListWnd, 2, &stLvColm );
	stLvColm.pszText = TEXT("Line");	stLvColm.cx =  45;	stLvColm.iSubItem = 3;	ListView_InsertColumn( ghPageListWnd, 3, &stLvColm );


//ツールチップ
#ifdef PGL_TOOLTIP
	ghPageTipWnd = CreateWindowEx( WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, ghPageWnd, NULL, hInstance, NULL );

	ViewingFontGet( &stFont );
	stFont.lfHeight = FONTSZ_REDUCE;
	ghPgTipFont = CreateFontIndirect( &stFont );
	SetWindowFont( ghPageTipWnd, ghPgTipFont, TRUE );

	//	ツールチップをコールバックで割り付け
	ZeroMemory( &stToolInfo, sizeof(TTTOOLINFO) );
	stToolInfo.cbSize   = sizeof(TTTOOLINFO);
	stToolInfo.uFlags   = TTF_SUBCLASS;
	stToolInfo.hinst    = NULL;	//	
	stToolInfo.hwnd     = ghPageListWnd;
	stToolInfo.uId      = IDLV_PAGELISTVIEW;
	GetClientRect( ghPageListWnd, &stToolInfo.rect );
	stToolInfo.lpszText = LPSTR_TEXTCALLBACK;	//	コレを指定するとコールバックになる
	SendMessage( ghPageTipWnd, TTM_ADDTOOL, 0, (LPARAM)&stToolInfo );
	SendMessage( ghPageTipWnd, TTM_SETMAXTIPWIDTH, 0, 0 );	//	チップの幅。０設定でいい。これしとかないと改行されない

//	ツールチップのポップディレイは、TTM_SETDELAYTIME で調整出来る
//	wParam	TTDT_INITIAL	表示までの時間
//	lParam	The LOWORD specifies the delay time, in milliseconds. The HIWORD must be zero.
//	SendMessage( ghPageTipWnd, TTM_SETDELAYTIME, TTDT_INITIAL, MAKELPARAM(2222,0) );
	//	効果無し
#endif

	ShowWindow( ghPageWnd, SW_SHOW );
	UpdateWindow( ghPageWnd );

	return ghPageWnd;
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
*/
LRESULT CALLBACK gpfPageToolProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			if( SendMessage(hWnd, TB_GETHOTITEM, 0, 0) >= 0 ){	ReleaseCapture(   );	}
			return 0;
	}

	return CallWindowProc( gpfOrigPageToolProc, hWnd, msg, wParam, lParam );
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
LRESULT CALLBACK PageListProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_COMMAND,     Plt_OnCommand );	
		HANDLE_MSG( hWnd, WM_SIZE,        Plt_OnSize );	
		HANDLE_MSG( hWnd, WM_NOTIFY,      Plt_OnNotify );	//	コモンコントロールの個別イベント
		HANDLE_MSG( hWnd, WM_CONTEXTMENU, Plt_OnContextMenu );

		case WM_DESTROY:
#ifdef PGL_TOOLTIP
			SetWindowFont( ghPageTipWnd, GetStockFont(DEFAULT_GUI_FONT), FALSE );
			DeleteFont( ghPgTipFont );
			FREE( gptPgTipBuf );
#endif
			ImageList_Destroy( ghPgLstImgLst );
			return 0;

		case WM_CLOSE:	ShowWindow( ghPageWnd, SW_HIDE );	return 0;

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
VOID Plt_OnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify )
{
	INT	iPage, iItem, mRslt, iDiff;
	LONG_PTR	rdExStyle;
#ifdef PAGE_MULTISELECT
	INT	iNxItem = 0, iCount, i;
#endif

	switch( id )
	{
		case  IDM_WINDOW_CHANGE:	WindowFocusChange( WND_PAGE,  1 );	return;
		case  IDM_WINDOW_CHG_RVRS:	WindowFocusChange( WND_PAGE, -1 );	return;

		case IDM_PAGEL_ADD:	//	末尾新規作成はいつでも有効
			iPage = DocPageCreate( -1 );
			PageListInsert( iPage );	//	ページリストビューに追加
			DocPageChange( iPage );
			DocModifyContent( TRUE );
			DocFileBackup( hWnd );	//	バックアップしておく
			ViewFocusSet(  );	//	フォーカス戻す
			return;

		case IDM_TOPMOST_TOGGLE:	//	常時最全面と通常ウインドウのトグル
			rdExStyle = GetWindowLongPtr( hWnd, GWL_EXSTYLE );
			if( WS_EX_TOPMOST & rdExStyle )
			{
				SetWindowPos( hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
				InitWindowTopMost( INIT_SAVE, WDP_PLIST, 0 );
			}
			else
			{
				SetWindowPos( hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
				InitWindowTopMost( INIT_SAVE, WDP_PLIST, 1 );
			}
			return;

		case IDM_PAGEL_AATIP_TOGGLE:	//	内容ポッパップするか
			gbPgTipView = gbPgTipView ? FALSE : TRUE;
			InitParamValue( INIT_SAVE, VL_PAGETIP_VIEW, gbPgTipView );
			return;

		case IDM_PAGEL_RETURN_FOCUS:	//	頁選択したらフォーカス戻すか
			gbPgRetFocus = gbPgRetFocus ? FALSE : TRUE;
			InitParamValue( INIT_SAVE, VL_PGL_RETFCS, gbPgRetFocus );
			return;


		default:	break;
	}

	//	選択されてる個数を確保・なんかヘンになってる
//	ListView_GetSelectedCount

	//	選択されてる項目を確保
	iItem = ListView_GetNextItem( ghPageListWnd, -1, LVNI_ALL | LVNI_SELECTED );
#ifdef PAGE_MULTISELECT
	iCount = ListView_GetItemCount( ghPageListWnd );

	if( 0 <= iItem )	//	複数選択されてるかどうか確認
	{
		iNxItem = ListView_GetNextItem( ghPageListWnd, iItem, LVNI_ALL | LVNI_SELECTED );
	}
#endif

	//	選択されてるモノがないと無意味
	if( 0 >  iItem ){	iItem = gixFocusPage;	}
	//	未選択なら、現在頁が選択されているモノと見なす

	//	複数選択なら、今のPageをCurrentにするか

	switch( id )
	{
		case IDM_PAGE_PREV:	//	前の頁へ移動
			iDiff = iItem - 1;
			PageListJump( iDiff );
			return;

		case IDM_PAGE_NEXT:	//	次の頁へ移動
			iDiff = iItem + 1;
			PageListJump( iDiff );
			return;
		//	20120110	頁移動したら変更有りになっちゃうの修正


		case IDM_PAGEL_INSERT:	//	任意位置新規作成
#ifdef PAGE_MULTISELECT
			if( 0 <= iNxItem ){	return;	}	//	複数選択してたら処理しない
#endif
			iPage = DocPageCreate( iItem );
			PageListInsert( iPage );	//	ページリストビューに追加
			DocPageChange( iPage );
			DocFileBackup( hWnd );	//	バックアップしておく
			break;

		case IDM_PAGEL_RENAME:	//	頁名称変更
#ifdef PAGE_MULTISELECT
			if( 0 <= iNxItem ){	return;	}	//	複数選択してたら処理しない
#endif
			if( FAILED( PageListNameChange( iItem ) ) ){	 return;	}
			break;

		case IDM_PAGEL_DELETE:	//	この頁を削除
#ifdef PAGE_MULTISELECT
			if( 0 <= iNxItem )	//	複数選択してるのなら、いつでも確認入れる
			{
				mRslt = MessageBox( hWnd, TEXT("複数の頁を削除しようとしてるよ。\r\n本当に削除していいのかい？"), TEXT("お燐からの確認"), MB_YESNO | MB_DEFBUTTON2 );
				if( IDYES == mRslt )
				{
					for( i = 0; iCount > i; i++ )	//	一応リミット
					{
						iItem = ListView_GetNextItem( ghPageListWnd, -1, LVNI_ALL | LVNI_SELECTED );
						if( 0 > iItem ){	break;	}
						DocPageDelete( iItem , -1 );
					}
				}
			}
			else
			{
#endif
				mRslt = MessageBoxCheckBox( hWnd, ghInst, 2 );	//	確認入れて
				if( IDYES == mRslt ){	DocPageDelete( iItem , -1 );	}
#ifdef PAGE_MULTISELECT
			}
#endif
			break;

		case IDM_PAGEL_DIVIDE:	//	分割はビューのメニュー
			break;

		case IDM_PAGEL_COMBINE:	//	統合
#ifdef PAGE_MULTISELECT
			if( 0 <= iNxItem ){	return;	}	//	複数選択してたら処理しない
#endif
			//	確認入れて
			mRslt = MessageBoxCheckBox( hWnd, ghInst, 0 );
			if( IDYES == mRslt ){	PageListCombine( hWnd , iItem );	}
			break;

		case IDM_PAGEL_UPFLOW:		//	↑移動
#ifdef PAGE_MULTISELECT
			if( 0 <= iNxItem ){	return;	}	//	複数選択してたら処理しない
#endif
			PageListSpinning( hWnd, iItem, 1 );
			break;

		case IDM_PAGEL_DOWNSINK:	//	↓移動
#ifdef PAGE_MULTISELECT
			if( 0 <= iNxItem ){	return;	}	//	複数選択してたら処理しない
#endif
			PageListSpinning( hWnd, iItem, -1 );
			break;

		case IDM_PAGEL_DUPLICATE:	//	頁複製
#ifdef PAGE_MULTISELECT
			if( 0 <= iNxItem ){	return;	}	//	複数選択してたら処理しない
#endif
			PageListDuplicate( hWnd, iItem );
			break;

		case IDM_PAGEL_DETAIL:
			TRACE( TEXT("最新の情報に更新") );
			return;

		default:	TRACE( TEXT("未実装[%d]"), id );	return;
	}

	//	更新になるやつはこっちに戻す
	DocModifyContent( TRUE );

	ViewFocusSet(  );	//	フォーカス戻す

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
VOID Plt_OnSize( HWND hWnd, UINT state, INT cx, INT cy )
{
	RECT	tbRect;
#ifdef PGL_TOOLTIP
	TTTOOLINFO	stToolInfo;
#endif

	tbRect.right = 0;
	if( ghToolWnd )
	{
		SendMessage( ghToolWnd, TB_GETITEMRECT, 0, (LPARAM)(&tbRect) );
		MoveWindow( ghToolWnd, 0, 0, tbRect.right, cy, TRUE );
	}

	MoveWindow( ghPageListWnd, tbRect.right, 0, cx - tbRect.right, cy, TRUE );

#ifdef PGL_TOOLTIP
	//	必要な所だけいれればおｋ
	ZeroMemory( &stToolInfo, sizeof(TTTOOLINFO) );
	stToolInfo.cbSize = sizeof(TTTOOLINFO);
	stToolInfo.hwnd   = ghPageListWnd;
	stToolInfo.uId    = IDLV_PAGELISTVIEW;
	GetClientRect( ghPageListWnd, &stToolInfo.rect );
	SendMessage( ghPageTipWnd, TTM_NEWTOOLRECT, 0, (LPARAM)&stToolInfo );
#endif
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
LRESULT Plt_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{
	//	ページリストビュー
	if( IDLV_PAGELISTVIEW == idFrom ){	return PageListNotify( hWnd, (LPNMLISTVIEW)pstNmhdr );	}

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
VOID Plt_OnContextMenu( HWND hWnd, HWND hWndContext, UINT xPos, UINT yPos )
{
	HMENU	hMenu, hSubMenu;
	UINT	dRslt;
	INT		iCount, iItem;
	BOOLEAN	bSel;
	LONG_PTR	rdExStyle;

	POINT	stPoint;

	stPoint.x = (SHORT)xPos;	//	画面座標はマイナスもありうる
	stPoint.y = (SHORT)yPos;

	bSel = FALSE;
	iCount  = ListView_GetItemCount( ghPageListWnd );
	iItem = ListView_GetNextItem( ghPageListWnd, -1, LVNI_ALL | LVNI_SELECTED);
	if( 0 <= iItem )	bSel = TRUE;


	hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_PGLVPOPUPMENU) );
	hSubMenu = GetSubMenu( hMenu, 0 );

	//	頁が１しかないなら、削除を無効に
	if( 1 >= iCount ){	EnableMenuItem( hSubMenu, IDM_PAGEL_DELETE, MF_GRAYED );	}

	if( gbTmpltDock )
	{
		EnableMenuItem( hSubMenu, IDM_TOPMOST_TOGGLE, MF_GRAYED );
	}
	else
	{
		//	最前面であるか
		rdExStyle = GetWindowLongPtr( hWnd, GWL_EXSTYLE );
		if( WS_EX_TOPMOST & rdExStyle ){	CheckMenuItem( hSubMenu , IDM_TOPMOST_TOGGLE, MF_BYCOMMAND | MF_CHECKED );	}
	}

	//	ポッパップ表示有るか？
	if( gbPgTipView ){	CheckMenuItem( hSubMenu, IDM_PAGEL_AATIP_TOGGLE, MF_CHECKED );	}

	//	選択でフォーカス戻りか？
	if( gbPgRetFocus ){	CheckMenuItem( hSubMenu, IDM_PAGEL_RETURN_FOCUS, MF_CHECKED );	}


	//	未選択なら、選択してないと使えない機能を無効に
	if( !(bSel) )
	{
		EnableMenuItem( hSubMenu, IDM_PAGEL_INSERT, MF_GRAYED );	//	任意作成
		EnableMenuItem( hSubMenu, IDM_PAGEL_DELETE, MF_GRAYED );	//	削除
		EnableMenuItem( hSubMenu, IDM_PAGEL_COMBINE, MF_GRAYED );	//	統合
		EnableMenuItem( hSubMenu, IDM_PAGEL_UPFLOW, MF_GRAYED );	//	浮上
		EnableMenuItem( hSubMenu, IDM_PAGEL_DOWNSINK, MF_GRAYED );	//	沈降
		EnableMenuItem( hSubMenu, IDM_PAGEL_DUPLICATE, MF_GRAYED );	//	複製
		EnableMenuItem( hSubMenu, IDM_PAGEL_RENAME, MF_GRAYED );	//	名称
	}

	dRslt = TrackPopupMenu( hSubMenu, 0, stPoint.x, stPoint.y, 0, hWnd, NULL );
	//	選択せずで０か－１？、選択したらそのメニューのＩＤでWM_COMMANDが発行
	DestroyMenu( hMenu );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	フローティング頁一覧の位置リセット
	@param[in]	hMainWnd	メインウインドウハンドル
	@return	HRESULT	終了状態コード
*/
HRESULT PageListPositionReset( HWND hMainWnd )
{
	RECT	wdRect, rect;

	GetWindowRect( hMainWnd, &wdRect );
	rect.left   = wdRect.left - PL_WIDTH;
	rect.top    = wdRect.top;
	rect.right  = PL_WIDTH;
	rect.bottom = PL_HEIGHT;

	SetWindowPos( ghPageWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW | SWP_NOZORDER );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/* !
	親ウインドウが移動したり大きさ変わったら
	@param[in]	hPrntWnd	親ウインドウハンドル
	@param[in]	pstFrame	クライアントサイズ
*/
VOID PageListResize( HWND hPrntWnd, LPRECT pstFrame )
{
	RECT	rect, tbRect;
//gbTmpltDock

	rect = *pstFrame;	//	クライヤントに使える領域
	rect.left  = rect.right - (grdSplitPos - SPLITBAR_WIDTH);	//	左位置
	rect.right = (grdSplitPos - SPLITBAR_WIDTH);	//	幅
	if( gbDockTmplView )
	{
		rect.bottom >>= 1;
	}
	else
	{
		DockingTabSizeGet( &tbRect );	//	タブの高さが要る
		rect.bottom -= tbRect.bottom;
	}

	SetWindowPos( ghPageWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	リストびゅーのNOTIFYメッセージ処理
	@param[in]	hWnd	親ウインドウハンドル
	@param[in]	pstLv	追加情報構造体へのポインター
	@return		LRESULT	なんかいろいろやった
*/
LRESULT PageListNotify( HWND hWnd, LPNMLISTVIEW pstLv )
{
	HWND	hLvWnd;
	INT		iCount, iItem, nmCode;//, iPage;

	INT		iSel;

	DWORD	lvClmn;
	INT		lvLine;
	LPNMLVCUSTOMDRAW	pstDraw;

	PAGEINFOS	stInfo;

	hLvWnd = pstLv->hdr.hwndFrom;
	nmCode = pstLv->hdr.code;
	//	なんらかのアクションの起こったROW位置をゲットする 
	iCount = ListView_GetItemCount( hLvWnd );
	iItem  = pstLv->iItem;

	//	普通のクルックについて
	if( NM_CLICK == nmCode )
	{
		if( 0 <= iItem )	//	該当ページへの移動
		{
			TRACE( TEXT("ページ選択[%d]"), iItem );
			DocPageChange( iItem );

			//	フォーカス戻す？
			if( gbPgRetFocus  ){	ViewFocusSet(  );	}
		}
	}

	//	ダブルクルック
	if( NM_DBLCLK == nmCode )
	{
		if( 0 <= iItem )	//	頁名称DIALOGUEをニョキ
		{
			if( SUCCEEDED( PageListNameChange( iItem ) ) )
			{
				DocModifyContent( TRUE );
			}
		}
	}

	if( NM_RETURN == nmCode )
	{
//#ifdef PAGE_MULTISELECT

		iSel = ListView_GetNextItem( hLvWnd, -1, LVNI_ALL | LVNI_SELECTED );
		if( 0 > iSel )	return 0;	//	選択してなかったら終わり

		TRACE( TEXT("NM_RETURN[%d]"), iSel );
	}
//	NM_KEYDOWN	NM_CHAR	関知できず


//カスタムドロー・サブクラスの中だと上手くいかない・Why?
	if( NM_CUSTOMDRAW == nmCode )
	{
		pstDraw = (LPNMLVCUSTOMDRAW)pstLv;

		//	ここで扱う処理内容を返す。いわゆる初回登録
		if( pstDraw->nmcd.dwDrawStage == CDDS_PREPAINT ){		return CDRF_NOTIFYSUBITEMDRAW;	}
		if( pstDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT ){	return CDRF_NOTIFYSUBITEMDRAW;	}

		if( pstDraw->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT|CDDS_SUBITEM) )//(CDDS_ITEMPREPAINT|CDDS_SUBITEM)
		{
			lvLine = pstDraw->nmcd.dwItemSpec;	//	行・頁番号になる
			lvClmn = pstDraw->iSubItem;			//	カラム

			if( gixFocusPage == lvLine )	//	今の行・とりあえず青
			{
				pstDraw->clrTextBk = 0x00FF8080;
			}
			else if( gixPreviSel == lvLine )	//	前の行・とりやえず灰
			{
				pstDraw->clrTextBk = 0x00CCCCDD;
			}
			else
			{
				pstDraw->clrTextBk = 0xFF000000;	//	これでデフォ色指定・多分
			}

			if( 0 == lvClmn )
			{
				if( NowPageInfoGet( lvLine, NULL ) )
				{
					pstDraw->clrTextBk = 0x00C0C0C0;
				}
				//else
				//{
				//	pstDraw->clrTextBk = 0xFF000000;
				//}
			}

			//	再描画せずともリヤルに変わる・バイト数計算のところで再描画してる？
			if( 2 == lvClmn )
			{
				stInfo.dMasqus = PI_BYTES;
				NowPageInfoGet( lvLine, &stInfo );

				if( gdPageByteMax <  (UINT)(stInfo.iBytes) )	pstDraw->clrTextBk = 0x000000FF;
				//else	pstDraw->clrTextBk = 0xFF000000;
			}
			//else
			//{
			//	pstDraw->clrTextBk = 0xFF000000;	//	これでデフォ色指定
			//}

			return CDRF_NEWFONT;
		}
	}

	return 0;	//	通常は０
}
//-------------------------------------------------------------------------------------------------

/*!
	リストビュークリヤ
	@return	HRESULT	終了状態コード
*/
HRESULT PageListClear( VOID )
{
	ListView_DeleteAllItems( ghPageListWnd );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	開いている頁内容を変更
	@param[in]	iPage		開く頁０インデックス・表示は１インデックスなので注意
	@param[in]	iPrePage	直前まで選択してた頁・切り替えた場合は－１
	@return		HRESULT	終了状態コード
*/
HRESULT PageListViewChange( INT iPage, INT iPrePage )
{
	//	フォーカスページは、ここに来る前に変更しておくこと

	LONG	iItem;

	gixPreviSel = iPrePage;	//	直前の選択頁を記録しておく

	InvalidateRect( ghPageListWnd, NULL, TRUE );

	iItem = ListView_GetItemCount( ghPageListWnd );
	if( iItem <= iPage || 0 > iPage )	return E_OUTOFMEMORY;

	//	選択状態を変更
	ListView_SetItemState( ghPageListWnd, iPage, LVIS_SELECTED, LVIS_SELECTED );

	//	ビューを書き直し
	ViewEditReset(  );

	//	ここで開いた頁を投下ホットキー番号にセット
	gixDropPage = iPage;

//	ViewFocusSet(  );	//	ここではフォーカスしないほうが良いか


	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定の位置にページを追加
	@param[in]	iBefore	－１なら末尾追加
	@return		HRESULT	終了状態コード
*/
HRESULT PageListInsert( INT iBefore )
{
	//	リストビューの途中挿入ってできたっけ？
	UINT	iItem, i;
	TCHAR	atBuffer[MIN_STRING];
	LVITEM	stLvi;

	iItem = ListView_GetItemCount( ghPageListWnd );

	//	とりあえず末端に追加・途中追加は後で

	ZeroMemory( &stLvi, sizeof(stLvi) );
	stLvi.mask  = LVIF_TEXT;

	//	一覧番号は常にincrementでいいのか？

	ZeroMemory( atBuffer, sizeof(atBuffer) );

	if( 0 > iBefore )	//	末尾追加なら壱行増やせばおｋ
	{
		stLvi.iItem = iItem;

		StringCchPrintf( atBuffer, MIN_STRING, TEXT("%u"), iItem + 1 );
		stLvi.pszText  = atBuffer;
	}
	else
	{
		stLvi.iItem = iBefore;
		stLvi.pszText  = TEXT("");
	}

	stLvi.iSubItem = 0;
	ListView_InsertItem( ghPageListWnd, &stLvi );

	stLvi.pszText  = TEXT("");
	stLvi.iSubItem =  1;
	ListView_SetItem( ghPageListWnd, &stLvi );

	stLvi.pszText = TEXT("0");	//	byte
	stLvi.iSubItem =  2;
	ListView_SetItem( ghPageListWnd, &stLvi );

	stLvi.pszText = TEXT("1");	//	line
	stLvi.iSubItem =  3;
	ListView_SetItem( ghPageListWnd, &stLvi );


	if( 0 <= iBefore )
	{
		//	連番振り直し
		iItem = ListView_GetItemCount( ghPageListWnd );
		for( i = iBefore; iItem > i; i++ )
		{
			StringCchPrintf( atBuffer, MIN_STRING, TEXT("%u"), i + 1 );
			ListView_SetItemText( ghPageListWnd, i, 0, atBuffer );
		}
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	頁リストを再構成する
	@return		HRESULT	終了状態コード
*/
HRESULT PageListBuild( LPVOID pVoid )
{
	INT			i, iLastPage;
	PAGE_ITR	itPage;
	LVITEM		stLvi;
	TCHAR		atBuffer[MIN_STRING];

	ZeroMemory( &stLvi, sizeof(stLvi) );
	stLvi.mask  = LVIF_TEXT;

#pragma message ("頁一覧再構成・項目注意")

	i = 0;
	for( itPage = (*gitFileIt).vcCont.begin(); itPage != (*gitFileIt).vcCont.end(); itPage++ )
	{
		stLvi.iItem    = i;
		StringCchPrintf( atBuffer, MIN_STRING, TEXT("%u"), i + 1 );
		stLvi.pszText  = atBuffer;
		stLvi.iSubItem = 0;
		ListView_InsertItem( ghPageListWnd, &stLvi );

		stLvi.pszText  = itPage->atPageName;
		stLvi.iSubItem =  1;
		ListView_SetItem( ghPageListWnd, &stLvi );

		StringCchPrintf( atBuffer, MIN_STRING, TEXT("%d"), itPage->dByteSz );
		stLvi.pszText  = atBuffer;
		stLvi.iSubItem =  2;
		ListView_SetItem( ghPageListWnd, &stLvi );

		StringCchPrintf( atBuffer, MIN_STRING, TEXT("%u"), itPage->ltPage.size() );
		stLvi.iSubItem =  3;
		ListView_SetItem( ghPageListWnd, &stLvi );

		i++;
	}

	//	見てた頁をリストに露出させる
	iLastPage = (*gitFileIt).dNowPage;
	ListView_EnsureVisible( ghPageListWnd, iLastPage, FALSE );	//	対象頁のラインまでスクロールさせちゃう

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	選ばれた頁を上下移動
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	iPage	移動させる頁番号
	@param[in]	bDir	正：上へ　負：下へ
	@return		HRESULT	終了状態コード
*/
HRESULT PageListSpinning( HWND hWnd, INT iPage, INT bDir )
{
	INT	iItem, i = 0;
	PAGE_ITR	itPage, itSwap;

	iItem = ListView_GetItemCount( ghPageListWnd );

	if( 0 == bDir ){	return E_INVALIDARG;	}	//	あまり意味はない

	//	これ以上↑にいけないなら何もしない
	if( (0 == iPage) && (0 < bDir) ){	return  E_ABORT;	}

	//	これ以上↓にイケナイなら何もしない
	if( (iItem <= (iPage+1)) && (0 > bDir) ){	return  E_ABORT;	}

	TRACE( TEXT("頁移動処理[%d]"), iPage );

	//	街頭位置までイテレータをもっていく
	itPage = (*gitFileIt).vcCont.begin(  );
	std::advance( itPage, iPage );

	//	スワップ対象
	if( 0 <  bDir ){	itSwap = itPage - 1;	i = iPage - 1;	}	//	↑に移動なら直前のやつと交換
	if( 0 >  bDir ){	itSwap = itPage + 1;	i = iPage + 1;	}	//	↓なら直後のとチェンジゲッター

	iter_swap( itPage, itSwap );	//	スワッピング！

	//	表示内容入れ替えて、選択をしておく
	PageListViewRewrite( iPage );
	PageListViewRewrite( i );

	//	選択状態を変更
	ListView_SetItemState( ghPageListWnd, i, LVIS_SELECTED, LVIS_SELECTED );

	DocPageChange( i );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	一覧の指定の頁お削除する
	@param[in]	iPage	サクッた頁番号
	@return		HRESULT	終了状態コード
*/
HRESULT PageListDelete( INT iPage )
{
	UINT	iItem, i;
	TCHAR	atBuffer[MIN_STRING];

	ListView_DeleteItem( ghPageListWnd, iPage );
	//	Deleteしたら、リストビューは自動で詰められる

	//	連番振り直し
	iItem = ListView_GetItemCount( ghPageListWnd );
	for( i = 0; iItem > i; i++ )
	{
		StringCchPrintf( atBuffer, MIN_STRING, TEXT("%u"), i + 1 );
		ListView_SetItemText( ghPageListWnd, i, 0, atBuffer );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	各頁の状況・引数はそのうち構造体にしたほうがいいかも
	@param[in]	iPage	頁番号
	@param[in]	dByte	バイト数
	@param[in]	dLine	行数
	@return		HRESULT	終了状態コード
*/
HRESULT PageListInfoSet( INT iPage, INT dByte, INT dLine )
{
	INT		iPageCnt;
	TCHAR	atBuffer[MIN_STRING];

	iPageCnt = ListView_GetItemCount( ghPageListWnd );
	if( iPageCnt <= iPage ){	return E_OUTOFMEMORY;	}

	StringCchPrintf( atBuffer, MIN_STRING, TEXT("%d"), dByte );	//	byte
	ListView_SetItemText( ghPageListWnd, iPage, 2, atBuffer );

	StringCchPrintf( atBuffer, MIN_STRING, TEXT("%d"), dLine );	//	line
	ListView_SetItemText( ghPageListWnd, iPage, 3, atBuffer );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ある頁の情報を書き換えする
	@param[in]	iPage	頁番号
	@return		HRESULT	終了状態コード
*/
HRESULT PageListViewRewrite( INT iPage )
{
#pragma message ("頁一覧再描画・項目注意")
	UINT_PTR	dLines;
	UINT		dBytes;
	INT			iPageCount, i;
	TCHAR	atBuffer[MIN_STRING];

	iPageCount = ListView_GetItemCount( ghPageListWnd );
	if( iPageCount <= iPage )	return E_OUTOFMEMORY;

	if( 0 >  iPage )	//	再帰で全描画
	{
		for( i = 0; iPageCount > i; i++ )
		{
			PageListViewRewrite( i );
		}

		return S_OK;
	}

	StringCchPrintf( atBuffer, MIN_STRING, TEXT("%d"), iPage + 1 );
	ListView_SetItemText( ghPageListWnd, iPage, 0, atBuffer );

	ListView_SetItemText( ghPageListWnd, iPage, 1, (*gitFileIt).vcCont.at( iPage ).atPageName );

	dBytes = (*gitFileIt).vcCont.at( iPage ).dByteSz;
	StringCchPrintf( atBuffer, MIN_STRING, TEXT("%d"), dBytes );	//	byte
	ListView_SetItemText( ghPageListWnd, iPage, 2, atBuffer );

	dLines = (*gitFileIt).vcCont.at( iPage ).ltPage.size( );
	StringCchPrintf( atBuffer, MIN_STRING, TEXT("%d"), dLines );	//	line
	ListView_SetItemText( ghPageListWnd, iPage, 3, atBuffer );



	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	フォーカス頁を移動
	@param[in]	iDiff	移動先・範囲外かもしれない
	@return		HRESULT	終了状態コード
*/
HRESULT PageListJump( INT iDiff )
{
	INT	iItem;

	if( 0 >  iDiff )	return E_OUTOFMEMORY;

	iItem = ListView_GetItemCount( ghPageListWnd );
	if( iItem <= iDiff )	return E_OUTOFMEMORY;

	ListView_EnsureVisible( ghPageListWnd, iDiff, FALSE );

	DocPageChange( iDiff );

	ViewFocusSet(  );	//	フォーカス戻す

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	名称変更ダイヤログボックスのメセージハンドラだってばよ
	@param[in]	hDlg	ダイヤログハンドル
	@param[in]	message	ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@retval 0	メッセージは処理していない
	@retval no0	なんか処理された
*/
INT_PTR CALLBACK PageNameDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static INT cdPage;
//	TCHAR	atBuffer[MAX_PATH];

	switch( message )
	{
		case WM_INITDIALOG:
			cdPage = lParam;	//	編集する頁番号
			Edit_SetText( GetDlgItem(hDlg,IDE_PAGENAME), (*gitFileIt).vcCont.at( cdPage ).atPageName );
			SetFocus( GetDlgItem(hDlg,IDE_PAGENAME) );
			return (INT_PTR)FALSE;

		case WM_COMMAND:
			if( IDOK == LOWORD(wParam) )
			{
				Edit_GetText( GetDlgItem(hDlg,IDE_PAGENAME), (*gitFileIt).vcCont.at( cdPage ).atPageName, SUB_STRING );
				EndDialog( hDlg, IDOK );
				return (INT_PTR)TRUE;
			}

			if( IDCANCEL == LOWORD(wParam) )
			{
				EndDialog( hDlg, IDCANCEL );
				return (INT_PTR)TRUE;
			}

			break;
	}
	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	頁名前変更ダイヤログとか
	@param[in]	dPage	頁番号
	@return		HRESULT	終了状態コード
*/
HRESULT PageListNameChange( INT dPage )
{
	INT_PTR	iRslt;

	iRslt = DialogBoxParam( ghInst, MAKEINTRESOURCE(IDD_PAGE_NAME_DLG), ghPageWnd, PageNameDlgProc, dPage );
	if( IDOK == iRslt )
	{
		PageListNameSet( dPage, (*gitFileIt).vcCont.at( dPage ).atPageName );
		return S_OK;
	}

	return E_ABORT;
}
//-------------------------------------------------------------------------------------------------

/*!
	頁名前をセット
	@param[in]	dPage	頁番号
	@param[in]	ptName	頁名称
	@return		HRESULT	終了状態コード
*/
HRESULT PageListNameSet( INT dPage, LPTSTR ptName )
{
	INT		iPageCount;
	LVITEM	stLvi;

	iPageCount = ListView_GetItemCount( ghPageListWnd );
	if( iPageCount <= dPage )	return E_OUTOFMEMORY;

	ZeroMemory( &stLvi, sizeof(stLvi) );
	stLvi.mask     = LVIF_TEXT;
	stLvi.iItem    = dPage;
	stLvi.pszText  = ptName;
	stLvi.iSubItem =  1;	//	名前
	ListView_SetItem( ghPageListWnd, &stLvi );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	今開いている頁の名前をセット
	@param[in]	ptName	頁名称・６３文字以内
	@return		HRESULT	終了状態コード
*/
HRESULT PageListNameRewrite( LPTSTR ptName )
{
	UINT_PTR	cchSize;

	StringCchLength( ptName, STRSAFE_MAX_CCH, &cchSize );

	//	はみ出さないように、サイズ末端を強制NULL
	if( SUB_STRING <= cchSize ){	ptName[(SUB_STRING-1)] = NULL;	}

	//	本体書き換えて
	StringCchCopy( (*gitFileIt).vcCont.at( gixFocusPage ).atPageName, SUB_STRING, ptName );

	PageListNameSet( gixFocusPage, ptName );	//	表示も書換

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	名前の付いている頁があるか
	@param[in]	itFile	チェック対象ファイルのイテレータ
	@return	INT	非０名前付きがあった　０なかった
*/
INT PageListIsNamed( FILES_ITR itFile )
{
	UINT_PTR	iPageCount, i;

	iPageCount = itFile->vcCont.size(  );
	for( i = 0; iPageCount > i; i++ )
	{
		if( 0 != itFile->vcCont.at( i ).atPageName[0] ){	return TRUE;	}
	}

	return FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	フォーカス頁の内容を、次の頁を作ってコピーする
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	iNowPage	フォーカスしてる頁
	@return		HRESULT		終了状態コード
*/
HRESULT PageListDuplicate( HWND hWnd, INT iNowPage )
{
	INT		iNewPage;
//	INT_PTR	iNext, iTotal;
	LINE_ITR	itLine;

	TRACE( TEXT("頁複製") );

	//iTotal = DocNowFilePageCount(  );
	//iNext = iNowPage + 1;	//	次の頁
	//if( iTotal <= iNext ){	iNext =  -1;	}	//	全頁より多いなら末端指定

	iNewPage = DocPageCreate( iNowPage );	//	新頁
	PageListInsert( iNewPage  );	//	ページリストビューに追加

	//	空の壱行が作られてるので、削除しておく
	(*gitFileIt).vcCont.at( iNewPage ).ltPage.clear(  );

	std::copy(	(*gitFileIt).vcCont.at( iNowPage ).ltPage.begin(),
				(*gitFileIt).vcCont.at( iNowPage ).ltPage.end(),
				back_inserter( (*gitFileIt).vcCont.at( iNewPage ).ltPage ) );

#pragma message ("PageListDuplicate 作った頁の内容の再計算いるか？")

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	フォーカス頁の次の頁と統合する
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	iNowPage	フォーカスしてる頁
	@return		HRESULT		終了状態コード
*/
HRESULT PageListCombine( HWND hWnd, INT iNowPage )
{
	 INT	iLastLine, iLastDot;
	 INT	iNext;
	INT_PTR	iTotal;
	ONELINE	stLine;
	LINE_ITR	itLine;

	ZeroONELINE( &stLine );

	TRACE( TEXT("頁統合") );

	iTotal = DocNowFilePageCount(  );

	//	頁数が足りないならナニもしない
	if( 1 >= iTotal )	return E_ACCESSDENIED;


	iNext = iNowPage + 1;	//	次の頁
	if( iTotal <= iNext ){	return E_OUTOFMEMORY;	}	//	末端頁なら何もしない

	//	区切りとして改行入れて
	(*gitFileIt).vcCont.at( iNowPage ).ltPage.push_back( stLine );

	//	先に状況チェックして、未ロードなら、キャレット位置を末端にしてロードして、元に戻す
	if( (*gitFileIt).vcCont.at( iNext ).ptRawData )	//	生データ状態だったら
	{
		//	今の末端位置チェック
		iLastLine = DocNowFilePageLineCount(  ) - 1;
		iLastDot  = DocLineParamGet( iLastLine, NULL, NULL );

		DocInsertString( &iLastDot, &iLastLine, NULL, (*gitFileIt).vcCont.at( iNext ).ptRawData, 0, TRUE );
	}
	else
	{
		//	次の頁の全体をコピーしちゃう
		std::copy(	(*gitFileIt).vcCont.at( iNext ).ltPage.begin(),
					(*gitFileIt).vcCont.at( iNext ).ltPage.end(),
					back_inserter( (*gitFileIt).vcCont.at( iNowPage ).ltPage ) );
	}

	SqnFreeAll( &((*gitFileIt).vcCont.at( iNowPage ).stUndoLog) );	//	アンドゥログ削除

	DocPageDelete( iNext , -1 );	//	次の頁は削除しちゃう

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	アイテムリストビューサブクラス
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	msg		ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
*/
LRESULT CALLBACK gpfPageViewProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
//	INT	id;

	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_MOUSEMOVE, Plv_OnMouseMove );	
		HANDLE_MSG( hWnd, WM_COMMAND,   Plt_OnCommand );	
#ifdef PGL_TOOLTIP
		HANDLE_MSG( hWnd, WM_NOTIFY,    Plv_OnNotify  );	//	コモンコントロールの個別イベント
#endif

#ifdef USE_HOVERTIP
		case WM_MOUSEHOVER:
			HoverTipOnMouseHover( hWnd, wParam, lParam, PageListHoverTipInfo );
			return 0;

		case WM_MOUSELEAVE:
			HoverTipOnMouseLeave( hWnd );
			gixMouseSel = -1;
			return 0;
#endif
	}

	return CallWindowProc( gpfOrigPageViewProc, hWnd, msg, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------

/*!
	マウスが動いたときの処理
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	x			クライアント座標Ｘ
	@param[in]	y			クライアント座標Ｙ
	@param[in]	keyFlags	押されてる他のボタン
	@return		なし
*/
VOID Plv_OnMouseMove( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	LVHITTESTINFO	stHitInfo;
	INT	iItem;
	BOOLEAN	bReDraw = FALSE;

	//	そのときマウスカーソル下にあるアイテムを選択しておく

	ZeroMemory( &stHitInfo, sizeof(LVHITTESTINFO) );
	stHitInfo.pt.x = 10;	//	高さが重要なのでここは適当でいい
	stHitInfo.pt.y = y;

	iItem = ListView_HitTest( hWnd, &stHitInfo );
	if( gixMouseSel != iItem )	bReDraw = TRUE;
	gixMouseSel = iItem;

#ifdef USE_HOVERTIP
	if( bReDraw ){	HoverTipResist( ghPageListWnd );	}
#endif

#ifdef PGL_TOOLTIP
	if( bReDraw ){	SendMessage( ghPageTipWnd, TTM_UPDATE, 0, 0 );	}
#endif

//	TRACE( TEXT("PLV MM %d,%d,%d [%d]"), iItem, stHitInfo.iItem, stHitInfo.iSubItem, bReDraw );

	return;
}
//-------------------------------------------------------------------------------------------------

#ifdef PGL_TOOLTIP
/*!
	ノーティファイメッセージの処理
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	idFrom		NOTIFYを発生させたコントロールのＩＤ
	@param[in]	pstNmhdr	NOTIFYの詳細
	@return		処理した内容とか
*/
LRESULT Plv_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{
	INT				dBytes;
	UINT_PTR		rdLength;
	LPNMTTDISPINFO	pstDispInfo;

	if( TTN_GETDISPINFO == pstNmhdr->code )	//	ツールチップの内容の問い合わせだったら
	{
		if( IDLV_PAGELISTVIEW == idFrom )
		{
			pstDispInfo = (LPNMTTDISPINFO)pstNmhdr;

			if( gixPreSel != gixMouseSel )
			{

				ZeroMemory( pstDispInfo->szText, sizeof(pstDispInfo->szText) );
				pstDispInfo->lpszText = NULL;

				FREE( gptPgTipBuf );

				if( !(gbPgTipView) ){	return 0;	}	//	非表示なら何もしないでおｋ
				if( 0 > gixMouseSel ){	return 0;	}

				TRACE( TEXT("1 TTN_GETDISPINFO %d  %X"), gixMouseSel, pstDispInfo->uFlags );
				gixPreSel = gixMouseSel;

				//	該当ページから引っ張る
				dBytes = DocPageTextGetAlloc( gitFileIt, gixMouseSel, D_UNI, (LPVOID *)(&gptPgTipBuf), FALSE );

				if( gptPgTipBuf )
				{
					StringCchLength( gptPgTipBuf, STRSAFE_MAX_CCH, &rdLength );
					//if( 2 <= rdLength )
					//{
					//	//	末端に余計な改行があるので消しておく
					//	gptPgTipBuf[rdLength-1] = NULL;
					//	gptPgTipBuf[rdLength-2] = NULL;
					//	rdLength -= 2;
					//}
					//	余計な改行は無くなった

					pstDispInfo->lpszText = gptPgTipBuf;
				}
			}
			else
			{
				TRACE( TEXT("2 TTN_GETDISPINFO %d  %X"), gixMouseSel, pstDispInfo->uFlags );

				ZeroMemory( pstDispInfo->szText, sizeof(pstDispInfo->szText) );
				pstDispInfo->lpszText = gptPgTipBuf;
			}

			return 0;
		}
	}

//	処理なかったら続ける？
	return CallWindowProc( gpfOrigPageViewProc, hWnd, WM_NOTIFY, (WPARAM)idFrom, (LPARAM)pstNmhdr );

	//	無限ループしてないか、大丈夫か
}
//-------------------------------------------------------------------------------------------------
#endif

#ifdef USE_HOVERTIP
/*!
	HoverTip用のコールバック受取
	@param[in]	pVoid	未定義
	@return	確保した文字列・もしくはNULL
*/
LPTSTR CALLBACK PageListHoverTipInfo( LPVOID pVoid )
{
	INT		dBytes;
	LPTSTR	ptBuffer = NULL;

	if( !(gbPgTipView) ){	return NULL;	}	//	非表示なら何もしないでおｋ
	if( 0 > gixMouseSel ){	return NULL;	}

	//	該当ページから引っ張る
	dBytes = DocPageTextGetAlloc( gitFileIt, gixMouseSel, D_UNI, (LPVOID *)(&ptBuffer), FALSE );
	TRACE( TEXT("HOVER CALL %d, by[%d]"), gixMouseSel, dBytes );

	return ptBuffer;
}
//-------------------------------------------------------------------------------------------------
#endif

