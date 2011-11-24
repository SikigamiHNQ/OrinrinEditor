/*! @file
	@brief 壱行テンプレートの面倒見ます
	このファイルは LineTemplate.cpp です。
	@author	SikigamiHNQ
	@date	2011/06/17
*/

#include "stdafx.h"
#include "OrinrinEditor.h"
//-------------------------------------------------------------------------------------------------

#define LINETEMPLATE_CLASS	TEXT("LINE_TEMPLATE")
#define LT_WIDTH	240
#define LT_HEIGHT	240
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------

extern HFONT	ghAaFont;		//	AA用フォント
extern HFONT	ghNameFont;		//	ファイルタブ用フォント

extern INT		gbTmpltDock;	//	頁・壱行テンプレのドッキング

#ifdef MAIN_SPLITBAR
extern  HWND	ghMainSplitWnd;	//	メインのスプリットバーハンドル
extern  LONG	grdSplitPos;	//	スプリットバーの、左側の、画面右からのオフセット
#endif

static  ATOM	gTmpleAtom;		//!<	
static  HWND	ghTmpleWnd;		//!<	このウインドウハンドル

static  HWND	ghCtgryBxWnd;	//!<	
static  HWND	ghLvItemWnd;	//!<	
static  HWND	ghLnLvTipWnd;	//!<	壱行リストツールチップ

static  HWND	ghDockTabWnd;	//!<	ドッキングしたときの選択肢用

static  UINT	gNowGroup;		//!<	

static WNDPROC	gpfOrigLineCtgryProc;	//!<	
static WNDPROC	gpfOrigLineItemProc;	//!<	

static  UINT	gLnClmCnt;	//!<	表示カラム数

static vector<AATEMPLATE>	gvcTmples;	//!<	テンプレの保持
//-------------------------------------------------------------------------------------------------

LRESULT	CALLBACK LineTmpleProc( HWND, UINT, WPARAM, LPARAM );	//!<	
VOID	Ltp_OnCommand( HWND , INT, HWND, UINT );	//!<	
VOID	Ltp_OnSize( HWND , UINT, INT, INT );	//!<	
LRESULT	Ltp_OnNotify( HWND , INT, LPNMHDR );	//!<	
VOID	Ltp_OnContextMenu( HWND, HWND, UINT, UINT );	

UINT	CALLBACK LineTmpleItemData( LPTSTR, LPTSTR, INT );	//!<	

HRESULT	LineTmpleItemListOn( UINT );	//!<	

HRESULT	TemplateItemSplit( LPTSTR, UINT, PAGELOAD );	//!<	
HRESULT	TemplateItemScatter( LPTSTR, INT, PAGELOAD );	//!<	

LRESULT	CALLBACK gpfLineCtgryProc( HWND, UINT, WPARAM, LPARAM );	//!<	
LRESULT	CALLBACK gpfLineItemProc(  HWND, UINT, WPARAM, LPARAM );	//!<	
LRESULT	Ltl_OnNotify( HWND, INT, LPNMHDR );

HWND	DockingTabCreate( HINSTANCE, HWND, LPRECT );
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
#ifdef MAIN_SPLITBAR
	INT			spPos;
#endif

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
#ifdef MAIN_SPLITBAR
		spPos = grdSplitPos - SPLITBAR_WIDTH;	//	右からのオフセット
#endif
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

	gpfOrigLineItemProc = SubclassWindow( ghLvItemWnd, gpfLineItemProc );

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
	SetWindowFont( ghLnLvTipWnd, GetStockFont(DEFAULT_GUI_FONT), FALSE );

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
	rect.bottom >>= 1;
	rect.top    += rect.bottom;

	GetWindowRect( ghDockTabWnd, &tbRect );
	tbRect.left    = rect.left;
	tbRect.right   = (grdSplitPos - SPLITBAR_WIDTH);
	tbRect.bottom -= tbRect.top;
	tbRect.top     = rect.top;
	MoveWindow( ghDockTabWnd, tbRect.left, tbRect.top, tbRect.right, tbRect.bottom, TRUE );

	rect.top    += tbRect.bottom;
	rect.bottom -= tbRect.bottom;

	SetWindowPos( ghTmpleWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_NOZORDER );

	return;
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
		HANDLE_MSG( hWnd, WM_NOTIFY,      Ltp_OnNotify );	//	コモンコントロールの個別イベント
		HANDLE_MSG( hWnd, WM_CONTEXTMENU, Ltp_OnContextMenu );

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
	width = rect.right / gLnClmCnt;

	for( i = 0; gLnClmCnt > i; i++ )
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

	//	一体化なら特に出すモノはない
	if( gbTmpltDock )	return;

	stPoint.x = (SHORT)xPos;	//	画面座標はマイナスもありうる
	stPoint.y = (SHORT)yPos;

	hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_TEMPLATE_POPUP) );
	hSubMenu = GetSubMenu( hMenu, 0 );
	AppendMenu( hSubMenu, MF_STRING, IDM_TMPLGROUPSTYLE_TGL, TEXT("カテゴリ表示切替") );

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
UINT CALLBACK LineTmpleItemData( LPTSTR ptName, LPTSTR ptLine, INT cchSize )
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

		stLvi.iItem    = i / gLnClmCnt;
		stLvi.iSubItem = i % gLnClmCnt;
		if( 0 == stLvi.iSubItem )	ListView_InsertItem( ghLvItemWnd, &stLvi );
		else						ListView_SetItem( ghLvItemWnd, &stLvi );
	}

	//	ブチこんだら幅調整
	GetClientRect( ghLvItemWnd, &rect );
	width = rect.right / gLnClmCnt;
	for( i = 0; gLnClmCnt > i; i++ ){	ListView_SetColumnWidth( ghLvItemWnd, i, width );	}

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
	LPTSTR	ptCaret;	//	読込開始・現在位置
	LPTSTR	ptStart;	//	セパレータの直前
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
		ptStart = NextLine( ptCaret );	//	次の行からが本番
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

		ptCaret = NextLine( ptEnd );	//	次の行が次の開始地点

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
HRESULT TemplateItemScatter( LPTSTR ptCont, INT cchSize, PAGELOAD pfCalling )
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

		case WM_MOUSEWHEEL:
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

	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_NOTIFY, Ltl_OnNotify );	//	コモンコントロールの個別イベント

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
	hLvWnd = hWnd;//pstLv->hdr.hwndFrom;
	nmCode = pstLv->hdr.code;

	if( TTN_GETDISPINFO == nmCode )
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

	return CallWindowProc( gpfOrigLineItemProc, hWnd, WM_NOTIFY, (WPARAM)idFrom, (LPARAM)pstNmhdr );
}
//-------------------------------------------------------------------------------------------------

