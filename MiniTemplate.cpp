/*! @file
	@brief 小型ＭＡＡテンプレートの面倒見ます
	このファイルは MiniTemplate.cpp です。
	@author	SikigamiHNQ
	@date	2011/08/29
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

#ifdef MINI_TEMPLATE

#define MINITEMPLATE_CLASS	TEXT("MINI_TEMPLATE")
#define MT_WIDTH	240
#define MT_HEIGHT	240
//-------------------------------------------------------------------------------------------------


extern HFONT	ghAaFont;		//	AA用フォント

extern INT		gbTmpltDock;	//	テンプレのドッキング
extern BOOLEAN	gbDockTmplView;	//	くっついてるテンプレは見えているか

extern  HWND	ghMainSplitWnd;	//	メインのスプリットバーハンドル
extern  LONG	grdSplitPos;	//	スプリットバーの、左側の、画面右からのオフセット


static  ATOM	gMnTmplAtom;	//!<	
static  HWND	ghMnTmplWnd;	//!<	小型MAAテンプレ本体ウインドウ
static  HWND	ghTitleBxWnd;	//!<	項目コンボックス
static  HWND	ghItemStcWnd;	//!<	内容スタティック・オーナードローでヤる
//	ツールチップはHoverTipで自前で描写セヨ

static  HWND	ghMainWnd;		//!<	編集ビューのある本体ウインドウ

static WNDPROC	gpfOrigMmaaTitleProc;	//!<	
static WNDPROC	gpfOrigMmaaItemProc;	//!<	

static  vector<AAMATRIX>	gvcMmaaTmpls;	//!<	テンプレの保持
//-------------------------------------------------------------------------------------------------


LRESULT	CALLBACK MmaaTmpleProc( HWND, UINT, WPARAM, LPARAM );
VOID	Mma_OnCommand( HWND, INT, HWND, UINT );
VOID	Mma_OnSize( HWND, UINT, INT, INT );
LRESULT	Mma_OnNotify( HWND, INT, LPNMHDR );
VOID	Mma_OnContextMenu( HWND, HWND, UINT, UINT );

UINT	CALLBACK MmaaTmpleItemData( LPTSTR, LPCTSTR, INT );

UINT	MmaaTmpleItemListOn( UINT );
HRESULT	MmaaTmpleItemReload( HWND );

LRESULT	CALLBACK gpfMmaaTitleProc( HWND, UINT, WPARAM, LPARAM );
LRESULT	CALLBACK gpfMmaaItemProc(  HWND, UINT, WPARAM, LPARAM );
LRESULT	Mlv_OnNotify( HWND, INT, LPNMHDR );
//-------------------------------------------------------------------------------------------------


/*!
	小型ＭＡＡテンプレウインドウの作成
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

	INT		spPos;


	WNDCLASSEX	wcex;
	RECT		wdRect, clRect, rect, cbxRect, tbRect, mtbRect;
	LVCOLUMN	stLvColm;

	//	破壊
	if( !(hInstance) && !(hParentWnd) )
	{

		return NULL;
	}

	ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= MmaaTmpleProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= MINITEMPLATE_CLASS;
	wcex.hIconSm		= NULL;

	gMnTmplAtom = RegisterClassEx( &wcex );

	ghMainWnd = hParentWnd;

//テンプレデータ読み出し
//	TemplateItemLoad( AA_BRUSH_FILE, BrushTmpleItemData );


	InitWindowPos( INIT_LOAD, WDP_MMAATPL, &rect );
	if( 0 == rect.right || 0 == rect.bottom )	//	幅高さが０はデータ無し
	{
		GetWindowRect( hParentWnd, &wdRect );
		rect.left   = wdRect.right + 64;
		rect.top    = wdRect.top + 64;
		rect.right  = MT_WIDTH;
		rect.bottom = MT_HEIGHT;
		InitWindowPos( INIT_SAVE , WDP_MMAATPL, &rect );	//	起動時保存
	}

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
		if( InitWindowTopMost( INIT_LOAD, WDP_MMAATPL, 0 ) ){	dwExStyle |=  WS_EX_TOPMOST;	}
		dwStyle = WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_VISIBLE | WS_SYSMENU;
	}

	//	本体
	ghMnTmplWnd = CreateWindowEx( dwExStyle, MINITEMPLATE_CLASS, TEXT("Mimi Maa Template"),
		dwStyle, rect.left, rect.top, rect.right, rect.bottom, hPrWnd, NULL, hInstance, NULL);

	GetClientRect( ghMnTmplWnd, &clRect );

	//	項目名コンボックス
	ghTitleBxWnd = CreateWindowEx( 0, WC_COMBOBOX, TEXT("Mmaa Items"),
		WS_CHILD | WS_VISIBLE | WS_BORDER | CBS_DROPDOWNLIST,
		0, 0, clRect.right, 127, ghMnTmplWnd,
		(HMENU)IDCB_MT_CATEGORY, hInstance, NULL );

	gpfOrigMmaaTitleProc = SubclassWindow( ghTitleBxWnd, gpfMmaaTitleProc );

	//	項目タイトル・ＭＡＡ窓と同じ具合に
	//dItems = gvcMmaaTmpls.size( );
	//for( i = 0; dItems > i; i++ )
	//{
	//	ComboBox_AddString( ghTitleBxWnd, gvcBrTmpls.at( i ).atCtgryName );
	//}
	//ComboBox_SetCurSel( ghTitleBxWnd, 0 );


	GetClientRect( ghTitleBxWnd, &cbxRect );

	ghItemStcWnd = CreateWindowEx( WS_EX_CLIENTEDGE | WS_EX_ACCEPTFILES, WC_STATIC, TEXT(""),
		WS_VISIBLE | WS_CHILD | SS_OWNERDRAW | SS_NOTIFY,
		0, cbxRect.bottom, clRect.right, clRect.bottom - cbxRect.bottom,
		ghMnTmplWnd, (HMENU)IDLV_MT_ITEMSTATIC, hInstance, NULL );
	SetWindowFont( ghItemStcWnd, ghAaFont, TRUE );	//	オーナードローするから不要


	gpfOrigMmaaItemProc = SubclassWindow( ghItemStcWnd, gpfMmaaItemProc );

	BrushTmpleItemListOn( 0 );	//	中身追加

	if( !(gbTmpltDock) )
	{
		ShowWindow( ghMnTmplWnd, SW_SHOW );
		UpdateWindow( ghMnTmplWnd );
	}

	return ghMnTmplWnd;
}
//-------------------------------------------------------------------------------------------------

/*!
	ドッキング状態で発生・くっついてるウインドウがリサイズされたら
	@param[in]	hPrntWnd	くっついてるウインドウハンドル
	@param[in]	pstFrame	使えるサイズ
*/
VOID MmaaTmpleResize( HWND hPrntWnd, LPRECT pstFrame )
{
	RECT	rect, tbRect;
	//INT	iHei, iTak;

	//gbTmpltDock
	if( !(ghMnTmplWnd) )	return;

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

	SetWindowPos( ghMnTmplWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_NOZORDER );

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
LRESULT CALLBACK MmaaTmpleProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_SIZE,        Mma_OnSize );	
		HANDLE_MSG( hWnd, WM_COMMAND,     Mma_OnCommand );	
		HANDLE_MSG( hWnd, WM_NOTIFY,      Mma_OnNotify );	//	コモンコントロールの個別イベント
		HANDLE_MSG( hWnd, WM_CONTEXTMENU, Mma_OnContextMenu );

		case WM_MOUSEWHEEL:	SendMessage( ghItemStcWnd, WM_MOUSEWHEEL, wParam, lParam );	return 0;

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
#error 作りかけ
	switch( id )
	{
		case IDCB_MT_CATEGORY:	//	カテゴリ選択コンボックス
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

		case  IDM_WINDOW_CHANGE:	WindowFocusChange( WND_MMAA,  1 );	break;
		case  IDM_WINDOW_CHG_RVRS:	WindowFocusChange( WND_MMAA, -1 );	break;

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
	AA表示スタティックのサブクラスプロシージャ・ツールチップの処理に必要
	@param[in]	hWnd	リストのハンドル
	@param[in]	msg		ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@return		LRESULT	処理結果とか
*/
LRESULT CALLBACK gpfMmaaItemProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
//		HANDLE_MSG( hWnd, WM_CHAR,        Mai_OnChar );
//		HANDLE_MSG( hWnd, WM_KEYDOWN,     Mai_OnKey );			//	
//		HANDLE_MSG( hWnd, WM_KEYUP,       Mai_OnKey );			//	

		HANDLE_MSG( hWnd, WM_MOUSEMOVE,   Mai_OnMouseMove );	//	マウスいごいた
		HANDLE_MSG( hWnd, WM_LBUTTONUP,   Mai_OnLButtonUp );	//	マウス左ボタンあげ
//		HANDLE_MSG( hWnd, WM_MBUTTONUP,   Mai_OnMButtonUp );	//	マウス中ボタンあげ
//		HANDLE_MSG( hWnd, WM_CONTEXTMENU, Mai_OnContextMenu );	//	コンテキストメニュー発生
		HANDLE_MSG( hWnd, WM_DROPFILES,   Mai_OnDropFiles );	//	ドラグンドロップの受付

#ifdef USE_HOVERTIP
		case WM_MOUSEHOVER:
			HoverTipOnMouseHover( hWnd, wParam, lParam, MmaaItemsHoverTipInfo );
			return 0;

		case WM_MOUSELEAVE:
			HoverTipOnMouseLeave( hWnd );
			gixNowToolTip = -1;
			return 0;
#endif


		default:	break;
	}

	return CallWindowProc( gpfOrigMmaaItemProc, hWnd, msg, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------



#endif	//	MINI_TEMPLATE

