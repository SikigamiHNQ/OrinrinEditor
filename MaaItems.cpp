/*! @file
	@brief ＡＡ一覧の制御をします
	このファイルは MaaItems.cpp です。
	@author	SikigamiHNQ
	@date	2011/06/22
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
#include "MaaTemplate.h"
//-------------------------------------------------------------------------------------------------

typedef struct tagVIEWORDER
{
	UINT	index;
	UINT	dHeight;
	UINT	dUpper;
	UINT	dDownr;

} VIEWORDER, *LPVIEWORDER;


typedef struct tagAATITLE
{
	UINT	number;
	TCHAR	atTitle[MAX_STRING];

} AATITLE, *LPAATITLE;
//-------------------------------------------------------------------------------------------------

//static LOGFONT	gstBaseFont = {
//	FONTSZ_NORMAL,			//	フォントの高さ
//	0,						//	平均幅
//	0,						//	文字送りの方向とX軸との角度
//	0,						//	ベースラインとX軸との角度
//	FW_NORMAL,				//	文字の太さ(0~1000まで・400=nomal)
//	FALSE,					//	イタリック体
//	FALSE,					//	アンダーライン
//	FALSE,					//	打ち消し線
//	DEFAULT_CHARSET,		//	文字セット
//	OUT_OUTLINE_PRECIS,		//	出力精度
//	CLIP_DEFAULT_PRECIS,	//	クリッピング精度
//	PROOF_QUALITY,			//	出力品質
//	VARIABLE_PITCH,			//	固定幅か可変幅
//	TEXT("ＭＳ Ｐゴシック")	//	フォント名
//};
//-------------------------------------------------------------------------------------------------

#define TITLECBX_HEI	200

#define SBP_DIRECT		0xFF

static  HWND	ghItemsWnd;			//!<	リストのハンドル
#ifdef MAA_TOOLTIP
static  HWND	ghToolTipWnd;		//!<	ツールチップ
EXTERNED HFONT	ghTipFont;			//!<	ツールチップ用
#endif

static  HWND	ghComboxWnd;		//!<	見出し用コンボックス

static WNDPROC	gpfOrgAaItemsProc;	//!<	
static WNDPROC	gpfOrgAaTitleCbxProc;	//!<	

static LPTSTR	gptTipBuffer;		//!<	

static INT		gixTopItem;			//!<	一覧の最上位
static INT		gixMaxItem;			//!<	アイテム個数

static  LONG	gixNowSel;			//!<	マウスカーソルがあるところのインデックス
#ifdef USE_HOVERTIP
static  LONG	gixNowToolTip;		//!<	ツールチップ用セレクト
#endif

static  HWND	ghScrollWnd;		//!<	スクロールバー

#ifdef _ORRVW
EXTERNED HFONT	ghAaFont;			//!<	表示用のフォント
#else
static HFONT	ghAaFont;			//!<	表示用のフォント
#endif

static HBRUSH	ghBkBrush;			//!<	表示エリヤの背景色

static  HPEN	ghSepPen;			//!<	区切り線用ペン
static BOOLEAN	gbLineSep;			//!<	AAの分けは線にする

#ifndef _ORRVW
static BOOLEAN	gbMaaRetFocus;		//!<	項目を選択したら編集窓にフォーカス戻すか
#endif

#ifdef MAA_TEXT_FIND
TCHAR	gatFindText[MAX_STRING];	//!<	検索用文字列の保存
#endif

extern  UINT	gbAAtipView;		//!<	非０で、ＡＡツールチップ表示

extern  HWND	ghSplitaWnd;		//!<	スプリットバーハンドル

static vector<VIEWORDER>	gvcViewOrder;	//!<	今見えてるやつの内容
static vector<AATITLE>		gvcAaTitle;		//!<	
//-------------------------------------------------------------------------------------------------

#ifdef MAA_TOOLTIP
LRESULT	Aai_OnNotify( HWND , INT, LPNMHDR );			//!<	
#endif
VOID	Aai_OnMouseMove( HWND, INT, INT, UINT );		//!<	
VOID	Aai_OnLButtonUp( HWND, INT, INT, UINT );		//!<	
VOID	Aai_OnMButtonUp( HWND, INT, INT, UINT );		//!<	
VOID	Aai_OnContextMenu( HWND, HWND, UINT, UINT );	//!<	
VOID	Aai_OnDropFiles( HWND , HDROP );				//!<	

HRESULT	AaItemsFavDelete( LPSTR, UINT );	//!<	

#ifdef MAA_TEXT_FIND
UINT	AacItemFindOnePage( HWND, LPTSTR, INT );	//!<	
#endif

LRESULT	CALLBACK gpfAaItemsProc( HWND, UINT, WPARAM, LPARAM );		//!<	
LRESULT	CALLBACK gpfAaTitleCbxProc( HWND, UINT, WPARAM, LPARAM );	//!<	

#ifndef _ORRVW
INT_PTR	CALLBACK AaItemAddDlgProc( HWND, UINT, WPARAM, LPARAM );	//!<	
#endif

#ifdef USE_HOVERTIP
LPTSTR	CALLBACK AaItemsHoverTipInfo( LPVOID  );	//!<	
#endif
//-------------------------------------------------------------------------------------------------

/*!
	全ツリーやお気にリストの内容を表示するスタティックとか作る
	@param[in]	hWnd	親ウインドウハンドル・NULLなら破壊
	@param[in]	hInst	アプリの実存
	@param[in]	ptRect	クライアント領域のサイズ
	@return		HRESULT	終了状態コード
*/
HRESULT AaItemsInitialise( HWND hWnd, HINSTANCE hInst, LPRECT ptRect )
{
#ifdef MAA_TOOLTIP
	INT		ttSize;
	TTTOOLINFO	stToolInfo;
#endif
	SCROLLINFO	stScrollInfo;
	RECT	rect;
	LOGFONT	stFont;

	COLORREF	dBkColour;

	if( !(hWnd) )
	{
		free( gptTipBuffer );
		SetWindowFont( ghItemsWnd, GetStockFont(DEFAULT_GUI_FONT), FALSE );
#ifdef MAA_TOOLTIP
		SetWindowFont( ghToolTipWnd, GetStockFont(DEFAULT_GUI_FONT), FALSE );
		DeleteFont( ghTipFont );
#endif
		DeleteFont( ghAaFont );
		DeletePen( ghSepPen );
		DeleteBrush( ghBkBrush );
		return S_FALSE;
	}

//ウインドウのサイズは、あとで変更が飛んでくるので、ここでは固定値で作っておｋ

	ghSepPen  = CreatePen( PS_SOLID, 1, RGB(0xAA,0xAA,0xAA) );

	//	１なら区切り線スタイル
	gbLineSep = InitParamValue( INIT_LOAD, VL_MAASEP_STYLE, 0 );

	dBkColour = (COLORREF)InitParamValue( INIT_LOAD, VL_MAA_BKCOLOUR, 0x00FFFFFF );
	ghBkBrush = CreateSolidBrush( dBkColour );

#ifndef _ORRVW
	//	選択したらフォーカスを編集窓に戻す？
	gbMaaRetFocus = InitParamValue( INIT_LOAD, VL_MAA_RETFCS, 0 );
#endif

	gptTipBuffer = NULL;

	gixTopItem = 0;

	gixNowSel = -1;
#ifdef USE_HOVERTIP
	gixNowToolTip = -1;
#endif

#ifdef MAA_TEXT_FIND
	ZeroMemory( gatFindText, sizeof(gatFindText) );
#endif

#ifdef MAA_TOOLTIP
	//	ツールチップ作る
	ghToolTipWnd = CreateWindowEx( WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWnd, NULL, hInst, NULL );
#endif
	//	見出しコンボックス
	ghComboxWnd = CreateWindowEx( 0, WC_COMBOBOX, TEXT(""), WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_NOINTEGRALHEIGHT, TREE_WIDTH + SPLITBAR_WIDTH, 0, ptRect->right - TREE_WIDTH - LSSCL_WIDTH, TITLECBX_HEI, hWnd, (HMENU)IDCB_AAITEMTITLE, hInst, NULL );
	GetClientRect( ghComboxWnd, &rect );
	//	サブクラス化
	gpfOrgAaTitleCbxProc = SubclassWindow( ghComboxWnd, gpfAaTitleCbxProc );

	//	AA一覧のスタティックつくる・オーナードローで描画
	ghItemsWnd = CreateWindowEx( WS_EX_CLIENTEDGE | WS_EX_ACCEPTFILES, WC_STATIC, TEXT(""), WS_VISIBLE | WS_CHILD | SS_OWNERDRAW | SS_NOTIFY, TREE_WIDTH + SPLITBAR_WIDTH, rect.bottom, ptRect->right - TREE_WIDTH - LSSCL_WIDTH, ptRect->bottom - rect.bottom, hWnd, (HMENU)IDSO_AAITEMS, hInst, NULL );
	//DragAcceptFiles( ghItemsWnd, TRUE );	WS_EX_ACCEPTFILESでおｋ
	//	サブクラス化
	gpfOrgAaItemsProc = SubclassWindow( ghItemsWnd, gpfAaItemsProc );

	//	一覧のスクロールバー
	ghScrollWnd = CreateWindowEx( 0, WC_SCROLLBAR, TEXT("scroll"), WS_VISIBLE | WS_CHILD | SBS_VERT, ptRect->right - LSSCL_WIDTH, rect.bottom, LSSCL_WIDTH, ptRect->bottom - rect.bottom, hWnd, (HMENU)IDSB_LISTSCROLL, hInst, NULL );

	ZeroMemory( &stScrollInfo, sizeof(SCROLLINFO) );
	stScrollInfo.cbSize = sizeof(SCROLLINFO);
	stScrollInfo.fMask = SIF_DISABLENOSCROLL;
	SetScrollInfo( ghScrollWnd, SB_CTL, &stScrollInfo, TRUE );

	//	表示用メインフォント
	ViewingFontGet( &stFont );
	ghAaFont = CreateFontIndirect( &stFont );	//	gstBaseFont
	SetWindowFont( ghItemsWnd, ghAaFont, TRUE );

#ifdef MAA_TOOLTIP
	//	ポッパップチップ用・12/9pt兼用
	ttSize = InitParamValue( INIT_LOAD, VL_MAATIP_SIZE, FONTSZ_REDUCE );	//	サイズ確認
	stFont.lfHeight = (FONTSZ_REDUCE == ttSize) ? FONTSZ_REDUCE : FONTSZ_NORMAL;
	ghTipFont = CreateFontIndirect( &stFont );
	SetWindowFont( ghToolTipWnd, ghTipFont, TRUE );

	//	ツールチップをコールバックで割り付け
	ZeroMemory( &stToolInfo, sizeof(TTTOOLINFO) );
	stToolInfo.cbSize   = sizeof(TTTOOLINFO);
	stToolInfo.uFlags   = TTF_SUBCLASS;
	stToolInfo.hinst    = NULL;	//	
	stToolInfo.hwnd     = ghItemsWnd;
	stToolInfo.uId      = IDSO_AAITEMS;
	GetClientRect( ghItemsWnd, &stToolInfo.rect );
	stToolInfo.lpszText = LPSTR_TEXTCALLBACK;	//	コレを指定するとコールバックになる
	SendMessage( ghToolTipWnd, TTM_ADDTOOL, 0, (LPARAM)&stToolInfo );
	SendMessage( ghToolTipWnd, TTM_SETMAXTIPWIDTH, 0, 0 );	//	チップの幅。０設定でいい。これしとかないと改行されない
#endif

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	見出しコンボックスのサブクラスプロシージャ
	@param[in]	hWnd	リストのハンドル
	@param[in]	msg		ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@return	処理結果とか
*/
LRESULT	CALLBACK gpfAaTitleCbxProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_CHAR, Maa_OnChar );
		default:	break;
	}

	return CallWindowProc( gpfOrgAaTitleCbxProc, hWnd, msg, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------


/*!
	見出しコンボックスを空にする
*/
VOID AaTitleClear( VOID )
{
	ComboBox_SetCurSel( ghComboxWnd, -1 );

	while( ComboBox_GetCount( ghComboxWnd ) ){	ComboBox_DeleteString( ghComboxWnd, 0 );	}
	gvcAaTitle.clear( );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	見出しコンボックスに内容追加
	@param[in]	number	通し番号
	@param[in]	pcTitle	入れ込む文字列
	@return	継ぎ足したあとの項目数
*/
INT AaTitleAddString( UINT number, LPSTR pcTitle )
{
	AATITLE	stTitle;
	LPTSTR	ptTitle;

	ptTitle = SjisDecodeAlloc( pcTitle );

	ComboBox_AddString( ghComboxWnd, ptTitle );

	ZeroMemory( &stTitle, sizeof(AATITLE) );
	stTitle.number = number;
	StringCchCopy( stTitle.atTitle, MAX_STRING, ptTitle );

	gvcAaTitle.push_back( stTitle );

	FREE(ptTitle);

	return ComboBox_GetCount( ghComboxWnd );
}
//-------------------------------------------------------------------------------------------------

/*!
	コンボックスコマンド発生
	@param[in]	hWnd		親ウインドウハンドル
	@param[in]	codeNotify	通知メッセージ
*/
VOID AaTitleSelect( HWND hWnd, UINT codeNotify )
{
	INT	iSel;
	INT_PTR	iItems;

//	TRACE( TEXT("COMBOX[%u]"), codeNotify );

	if( CBN_SELCHANGE == codeNotify )	//	選択が変更された
	{
		iSel = ComboBox_GetCurSel( ghComboxWnd );	//	選ばれてるの取って

		iItems = gvcAaTitle.size( );	//	はみ出し確認
		if( iItems <= iSel )	return;

		//	見出しの該当する位置へジャンプ
		Aai_OnVScroll( hWnd, ghScrollWnd, SBP_DIRECT, gvcAaTitle.at( iSel ).number );
	}

	if( CBN_CLOSEUP == codeNotify )
	{
		SetFocus( ghItemsWnd );
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	画面サイズが変わったのでサイズ変更
	@param[in]	hWnd	親ウインドウハンドル
	@param[in]	ptRect	クライアント領域
*/
VOID AaItemsResize( HWND hWnd, LPRECT ptRect )
{
	INT		dWidth, dLeft;
	RECT	sptRect, rect;
#ifdef MAA_TOOLTIP
	TTTOOLINFO	stToolInfo;
#endif

	SplitBarPosGet( ghSplitaWnd, &sptRect );
	//	拡張タブバーの位置確保
//	MaaTabBarSizeGet( &tbRect );

	dWidth = ptRect->right - (sptRect.left + SPLITBAR_WIDTH) - LSSCL_WIDTH;
	dLeft  = ptRect->right - LSSCL_WIDTH;

	MoveWindow( ghComboxWnd, sptRect.left + SPLITBAR_WIDTH, ptRect->top, dWidth, TITLECBX_HEI, TRUE );
	GetClientRect( ghComboxWnd, &rect );

	MoveWindow( ghItemsWnd,  sptRect.left + SPLITBAR_WIDTH, ptRect->top + rect.bottom, dWidth, ptRect->bottom - rect.bottom, TRUE );
	MoveWindow( ghScrollWnd, dLeft, ptRect->top + rect.bottom, LSSCL_WIDTH, ptRect->bottom - rect.bottom, TRUE );

#ifdef MAA_TOOLTIP
	//	必要な所だけいれればおｋ
	ZeroMemory( &stToolInfo, sizeof(TTTOOLINFO) );
	stToolInfo.cbSize = sizeof(TTTOOLINFO);
	stToolInfo.hwnd   = ghItemsWnd;
	stToolInfo.uId    = IDSO_AAITEMS;
	GetClientRect( ghItemsWnd, &stToolInfo.rect );
	SendMessage( ghToolTipWnd, TTM_NEWTOOLRECT, 0, (LPARAM)&stToolInfo );
#endif
	InvalidateRect( ghItemsWnd, NULL, TRUE );
	UpdateWindow( ghItemsWnd );

	return;
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
LRESULT CALLBACK gpfAaItemsProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_CHAR,        Maa_OnChar );
		HANDLE_MSG( hWnd, WM_KEYDOWN,     Aai_OnKey );			//	20120221
		HANDLE_MSG( hWnd, WM_KEYUP,       Aai_OnKey );			//	

		HANDLE_MSG( hWnd, WM_MOUSEMOVE,   Aai_OnMouseMove );	//	マウスいごいた
		HANDLE_MSG( hWnd, WM_LBUTTONUP,   Aai_OnLButtonUp );	//	マウス左ボタンあげ
		HANDLE_MSG( hWnd, WM_MBUTTONUP,   Aai_OnMButtonUp );	//	マウス中ボタンあげ
		HANDLE_MSG( hWnd, WM_CONTEXTMENU, Aai_OnContextMenu );	//	コンテキストメニュー発生
		HANDLE_MSG( hWnd, WM_DROPFILES,   Aai_OnDropFiles );	//	ドラグンドロップの受付
#ifdef MAA_TOOLTIP
		HANDLE_MSG( hWnd, WM_NOTIFY,      Aai_OnNotify );		//	コモンコントロールの個別イベント
#endif


#ifdef USE_HOVERTIP
		case WM_MOUSEHOVER:
			HoverTipOnMouseHover( hWnd, wParam, lParam, AaItemsHoverTipInfo );
			return 0;

		case WM_MOUSELEAVE:
			HoverTipOnMouseLeave( hWnd );
			gixNowToolTip = -1;
			return 0;
#endif


		default:	break;
	}

	return CallWindowProc( gpfOrgAaItemsProc, hWnd, msg, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------

/*!
	AA表示スタティックのオーナードロー・描画
	@param[in]	hWnd			親ウインドウハンドル
	@param[in]	*pstDrawItem	ドロー情報
	@return		なし
*/
VOID AaItemsDrawItem( HWND hWnd, CONST DRAWITEMSTRUCT *pstDrawItem )
{
	UINT_PTR	rdLen;
	INT_PTR	rdLength;
	LPSTR	pcConts = NULL;
	LPTSTR	ptConStr = NULL;
	RECT	rect, drawRect;
	POINT	stPoint;

	VIEWORDER	stVwrder;

	INT		rdNextItem;
	LONG	rdDrawPxTop, rdBottom;	//	描画の最上位、描画領域の高さＭＡＸ
	LONG	rdHeight, rdWidth;	//	アイテムの高さ・描画領域の幅

	HPEN	hOldPen;

	//	項目の外接四角形の取得
	rect = pstDrawItem->rcItem;
	rdBottom = rect.bottom;
	rdWidth  = rect.right;
	rdDrawPxTop = 0;

	SetBkMode( pstDrawItem->hDC , TRANSPARENT );	//	文字描画は背景透過で夜露死苦

	FillRect( pstDrawItem->hDC, &rect, ghBkBrush );	//	一旦背景塗りつぶして

	gvcViewOrder.clear();

	rdNextItem = gixTopItem;
	for( rdDrawPxTop = 0; rdBottom > rdDrawPxTop; rdNextItem++ )
	{
		pcConts = AacAsciiArtGet( rdNextItem );	//	一覧のトップを確認
		if( !pcConts ){	break;	}

		stVwrder.index = rdNextItem;

		ptConStr = SjisDecodeAlloc( pcConts );	//	表示内容ガメてくる
		StringCchLength( ptConStr, STRSAFE_MAX_CCH, &rdLen );
		rdLength = rdLen;

		free( pcConts );
#pragma message ("MAAの行間、ここで正しく計算するべき")
		//	文字列に合わせてRECT確保・計算だけで、まだ書込してない
		DrawText( pstDrawItem->hDC, ptConStr, rdLength, &rect, DT_LEFT | DT_EDITCONTROL | DT_NOPREFIX | DT_CALCRECT );
		drawRect = rect;
		rdHeight = drawRect.bottom;
		drawRect.bottom += rdDrawPxTop;
		drawRect.top     = rdDrawPxTop;
		if( drawRect.right < rdWidth )	drawRect.right = rdWidth;

		stVwrder.dHeight = rdHeight;
		stVwrder.dUpper  = drawRect.top;
		stVwrder.dDownr  = drawRect.bottom;

		//	項目ごとに互い違いの色にするか、区切り線を引く
		if( gbLineSep ){	FillRect( pstDrawItem->hDC, &drawRect, ghBkBrush );	}
		else
		{
			if( 1 & rdNextItem )	FillRect( pstDrawItem->hDC, &drawRect, GetStockBrush(LTGRAY_BRUSH) );
			else					FillRect( pstDrawItem->hDC, &drawRect, ghBkBrush );
		}

		//	ＡＡ描画
		DrawText( pstDrawItem->hDC, ptConStr, rdLength, &drawRect, DT_LEFT | DT_EDITCONTROL | DT_NOPREFIX );

		if( gbLineSep )	//	線を引く
		{
			hOldPen = SelectPen( pstDrawItem->hDC, ghSepPen );
			MoveToEx( pstDrawItem->hDC, drawRect.left, drawRect.bottom-1, NULL );
			LineTo( pstDrawItem->hDC, drawRect.right, drawRect.bottom-1 );
			SelectPen( pstDrawItem->hDC, hOldPen );
		}

		gvcViewOrder.push_back( stVwrder );

		rdDrawPxTop += rdHeight;

		free( ptConStr );
	}
	InvalidateRect( ghScrollWnd, NULL, TRUE );
	UpdateWindow( ghScrollWnd );

	//	カーソル位置確保し直し・ポッパップ表示用
	GetCursorPos( &stPoint );
	ScreenToClient( ghItemsWnd, &stPoint );
	Aai_OnMouseMove( hWnd, stPoint.x, stPoint.y, 0 );

	return;
}
//-------------------------------------------------------------------------------------------------

#if 0

/*!
	オーナードロー・サイズ計測
	@param[in]	hWnd			親ウインドウハンドル
	@param[in]	pstMeasureItem	サイズ情報
	@return		HRESULT			終了状態コード
*/
VOID AaItemsMeasureItem( HWND hWnd, LPMEASUREITEMSTRUCT pstMeasureItem )
{
	HDC		hDC;
	INT		rdLength, rdHeight;
	LPSTR	pcConts;
	RECT	stRect;

	//	項目の文字列取得
	pcConts = AacAsciiArtGet( pstMeasureItem->itemID );
	if( !pcConts )	return;

	rdLength = strlen( pcConts );

	ListBox_GetItemRect( ghItemsWnd, pstMeasureItem->itemID, &stRect );

	hDC = GetDC( ghItemsWnd );
	DrawTextExA( hDC, pcConts, rdLength, &stRect, DT_LEFT | DT_EDITCONTROL | DT_NOPREFIX | DT_CALCRECT );
	ReleaseDC( ghItemsWnd, hDC );

	pstMeasureItem->itemHeight = (stRect.bottom - stRect.top);
	if( 256 <= pstMeasureItem->itemHeight )	pstMeasureItem->itemHeight = 255;

	free( pcConts );

	return;
}
//-------------------------------------------------------------------------------------------------
#endif


/*!
	キーダウンが発生
	@param[in]	hWnd	ウインドウハンドル・ビューのとは限らないので注意セヨ
	@param[in]	vk		押されたキーが仮想キーコードで来る
	@param[in]	fDown	非０ダウン　０アップ
	@param[in]	cRepeat	連続オサレ回数・取れてない？
	@param[in]	flags	キーフラグいろいろ
	@return		無し
*/
VOID Aai_OnKey( HWND hWnd, UINT vk, BOOL fDown, INT cRepeat, UINT flags )
{
	TRACE( TEXT("KEY[%u][%u]"), vk, cRepeat );

	if( !(fDown) )	 return;	//	とりあえずキーアップはすることない

//	反応しないコントロールとか確認セヨ

	switch( vk )
	{
		default:	return;

		case  VK_NEXT:	//	PageDown
			Aai_OnVScroll( hWnd, ghScrollWnd, SB_PAGEDOWN, 0 );
			break;

		case  VK_DOWN:	//	下
			Aai_OnVScroll( hWnd, ghScrollWnd, SB_LINEDOWN, 0 );
			break;

		case  VK_UP:	//	上
			Aai_OnVScroll( hWnd, ghScrollWnd, SB_LINEUP, 0 );
			break;

		case  VK_PRIOR:	//	PageUp
			Aai_OnVScroll( hWnd, ghScrollWnd, SB_PAGEUP, 0 );
			break;
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	マウスが動いたときの処理
	@param[in]	hWnd		親ウインドウハンドル
	@param[in]	x			クライアント座標Ｘ
	@param[in]	y			クライアント座標Ｙ
	@param[in]	keyFlags	押されてる他のボタン
	@return		なし
*/
VOID Aai_OnMouseMove( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	TCHAR		atBuffer[MAX_STRING];
	UINT_PTR	i, max;
	LONG		iItem = -1, bottom;
	BOOLEAN		bReDraw = FALSE;

	INT		iDot = 0, iLine = 0, iByte = 0;

	//	そのときマウスカーソル下にあるアイテムを選択しておく

	if( !( gvcViewOrder.empty() ) )
	{
		max = gvcViewOrder.size();

		bottom = 0;
		for( i = 0; max > i; i++ )
		{
			bottom += gvcViewOrder.at( i ).dHeight;

			if( y < bottom ){	iItem = gvcViewOrder.at(  i ).index;	break;	}
		}
	}
	//	カーソル下が無効なら、iItemは－１状態

#ifdef USE_HOVERTIP
	if( gixNowToolTip != iItem ){	bReDraw =  TRUE;	}
	gixNowSel = iItem;
	gixNowToolTip = gixNowSel;

	if( bReDraw && gbAAtipView ){	HoverTipResist( ghItemsWnd );	}
#endif

#ifdef MAA_TOOLTIP
	if( gixNowSel != iItem ){	bReDraw =  TRUE;	}
	gixNowSel = iItem;

	if( bReDraw && gbAAtipView )	SendMessage( ghToolTipWnd, TTM_UPDATE, 0, 0 );
#endif

	if( bReDraw )
	{
		iDot = AacArtSizeGet( iItem, &iLine, &iByte );
		//pcConts = AacAsciiArtGet( iItem );
		//if( pcConts )
		//{
		//	ptString = SjisDecodeAlloc( pcConts );
		//	FREE( pcConts );
		//	if( ptString )
		//	{
		//		iDot = TextViewSizeGet( ptString, &iLine );
		//		FREE( ptString );
		//	}
		//}

		TRACE( TEXT("MAA MOUSE %3d[%dDOT x %dLINE]%dByte"), iItem+1, iDot, iLine, iByte );
#ifdef _ORRVW
		StringCchPrintf( atBuffer, MAX_STRING, TEXT("%3d[%dDOT x %dLINE]"), iItem+1, iDot, iLine );
#else
		StringCchPrintf( atBuffer, MAX_STRING, TEXT("%3d[%dDOT x %dLINE] %dByte"), iItem+1, iDot, iLine, iByte );
#endif
		StatusBarMsgSet( SBMAA_AXIS, atBuffer );
	}


	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	マウスの左ボタンがうｐしたときの処理
	@param[in]	hWnd			親ウインドウハンドル
	@param[in]	x				クライアント座標Ｘ
	@param[in]	y				クライアント座標Ｙ
	@param[in]	keyFlags		押されてる他のボタン
	@return		なし
*/
VOID Aai_OnLButtonUp( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	AaItemsDoSelect( hWnd, MAA_DEFAULT, TRUE );

#ifndef _ORRVW
	if( gbMaaRetFocus ){	ViewFocusSet(  );	}
#endif
	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	マウスの中ボタンがうｐしたときの処理
	@param[in]	hWnd			親ウインドウハンドル
	@param[in]	x				クライアント座標Ｘ
	@param[in]	y				クライアント座標Ｙ
	@param[in]	keyFlags		押されてる他のボタン
	@return		なし
*/
VOID Aai_OnMButtonUp( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	AaItemsDoSelect( hWnd, MAA_SUBDEFAULT, TRUE );

#ifndef _ORRVW
	if( gbMaaRetFocus ){	ViewFocusSet(  );	}
#endif
	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	スクロールの処理
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	hwndCtl	スクロールバーのウインドウハンドル
	@param[in]	code	動作状態コード・0xFFを特殊コードとして使う
	@param[in]	pos		つまみの位置
	@return		処理した内容とか
*/
VOID Aai_OnVScroll( HWND hWnd, HWND hwndCtl, UINT code, INT pos )
{
	INT	maePos;
	SCROLLINFO	stScrollInfo;

	if( ghScrollWnd != hwndCtl )	return;

	//	状態をくやしく
	ZeroMemory( &stScrollInfo, sizeof(SCROLLINFO) );
	stScrollInfo.cbSize = sizeof(SCROLLINFO);
	stScrollInfo.fMask = SIF_ALL;
	GetScrollInfo( ghScrollWnd, SB_CTL, &stScrollInfo );

	maePos = gixTopItem;

	switch( code )	//	スクロール方向に合わせて内容をずらす
	{
		case SB_TOP:
			gixTopItem = 0;
			break;

		case SB_LINEUP:
		case SB_PAGEUP:
			gixTopItem--;
			if( 0 > gixTopItem )	gixTopItem = 0;
			break;

		case SB_LINEDOWN:
		case SB_PAGEDOWN:
			gixTopItem++;
			if( gixMaxItem <=  gixTopItem ){	gixTopItem = gixMaxItem-1;	}
			break;

		case SB_BOTTOM:
			gixTopItem = gixMaxItem - 1;
			break;

		case SB_THUMBTRACK:
			gixTopItem = stScrollInfo.nTrackPos;
			break;

		case SBP_DIRECT:	//	ユーザコード勝手に追加
			gixTopItem = pos;
			break;

		default:	return;
	}
	//	変化ないならなにもせんでいい
	if( maePos == gixTopItem )	return;

	InvalidateRect( ghItemsWnd, NULL, TRUE );
	UpdateWindow( ghItemsWnd  );	//	リアルタイム描画に必要

	stScrollInfo.fMask = SIF_POS;
	stScrollInfo.nPos  = gixTopItem;
	SetScrollInfo( ghScrollWnd, SB_CTL, &stScrollInfo, TRUE );

	TabMultipleTopMemory( gixTopItem );

	return;
}
//-------------------------------------------------------------------------------------------------

#ifdef MAA_TOOLTIP
/*!
	ノーティファイメッセージの処理
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	idFrom		NOTIFYを発生させたコントロールのＩＤ
	@param[in]	pstNmhdr	NOTIFYの詳細
	@return		処理した内容とか
*/
LRESULT Aai_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{
	UINT_PTR		rdLength;
	LPSTR			pcConts = NULL;
	LPNMTTDISPINFO	pstDispInfo;

	if( TTN_GETDISPINFO ==  pstNmhdr->code )	//	ツールチップの内容の問い合わせだったら
	{
		pstDispInfo = (LPNMTTDISPINFO)pstNmhdr;

		if( !(gbAAtipView) )	//	非表示なら何もしないでおｋ
		{
			ZeroMemory( &(pstDispInfo->szText), sizeof(pstDispInfo->szText) );
			pstDispInfo->lpszText = NULL;
			return 0;
		}

		FREE( gptTipBuffer );

		pcConts = AacAsciiArtGet( gixNowSel );	//	該当するインデックスAAを引っ張ってくる
		if( !pcConts  ){	return 0;	}

		rdLength = strlen( pcConts );	//	文字列の長さ取得

		//gptTipBuffer = (LPTSTR)malloc( sizeof(TCHAR) * (rdLength+1) );
		//ZeroMemory( gptTipBuffer, sizeof(TCHAR) * (rdLength+1) );
		//MultiByteToWideChar( CP_ACP, MB_COMPOSITE, pcConts, -1, gptTipBuffer, (rdLength+1) );

		gptTipBuffer = SjisDecodeAlloc( pcConts );

		pstDispInfo->lpszText = gptTipBuffer;

		free( pcConts );
	}

	return 0;
}
//-------------------------------------------------------------------------------------------------
#endif

/*!
	コンテキストメニュー呼びだし（右クルック）
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	hWndContext	右クルックした子ウインドウハンドル
	@param[in]	xPos		マウスカーソルのスクリーンＸ座標
	@param[in]	yPos		マウスカーソルのスクリーンＹ座標
	@return		なし
*/
VOID Aai_OnContextMenu( HWND hWnd, HWND hWndContext, UINT xPos, UINT yPos )
{
	HMENU	hMenu, hSubMenu;
	UINT	dRslt;
	INT		dOpen;	//	全ツリーとお気にリスト開いてるの
	LPSTR	pcConts = NULL;
	UINT_PTR	rdLength;
	INT		sx, sy;

	dOpen = TabMultipleNowSel(  );	//	開いてるので処理かえる
	//	ACT_ALLTREE	ACT_FAVLIST

#pragma message ("Editorとviewerの、メニューリソースの整合性に注意セヨ")
	hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_AALIST_POPUP) );
	hSubMenu = GetSubMenu( hMenu, 0 );

#ifdef _ORRVW
	//	使用リストのみ、削除を有効に、変更すること・標準で無効にしておく
	if( ACT_FAVLIST ==  dOpen ){	EnableMenuItem( hSubMenu, IDM_MAA_FAV_DELETE , MF_ENABLED );	}
	//	一般アイテムも削除出来るようにする
#endif

	//	ツールチップの表示・非表示のトゴゥ
	if( gbAAtipView ){	CheckMenuItem( hSubMenu, IDM_MAA_AATIP_TOGGLE, MF_CHECKED );	}

#ifndef _ORRVW
	//	フォーカス戻すかどうか
	if( gbMaaRetFocus ){	CheckMenuItem( hSubMenu, IDM_MAA_RETURN_FOCUS, MF_CHECKED );	}
#endif
	//	マルチモニタしてると、座標値がマイナスになることがある。
	sx = (SHORT)xPos;
	sy = (SHORT)yPos;

	//	gixNowSelは、何も無いところだと－１になる

	//	フラグにTPM_RETURNCMDを指定すると、WM_COMMANDが飛ばない
	dRslt = TrackPopupMenu( hSubMenu, TPM_RETURNCMD, sx, sy, 0, hWnd, NULL );	//	TPM_CENTERALIGN | TPM_VCENTERALIGN | 
	DestroyMenu( hMenu );
	switch( dRslt )
	{
		case IDM_MAA_FAV_DELETE:
			if( ACT_FAVLIST == dOpen )	//	使用の場合
			{
				pcConts = AacAsciiArtGet( gixNowSel );	//	該当するインデックスAAを引っ張ってくる
				if( !pcConts ){	return;	}

				rdLength = strlen( pcConts );	//	文字列の長さ取得
				AaItemsFavDelete( pcConts, rdLength );	//	削除Commando発行
				FavContsRedrawRequest( hWnd );	//	再描画しなきゃだね
			}
#ifndef _ORRVW
			else	//	それ以外なら、主タブか副タブ
			{
				AacItemDelete( hWnd, gixNowSel );
			}
#endif
			break;

#ifndef _ORRVW
		case IDM_MAA_INSERT_EDIT:		AaItemsDoSelect( hWnd, MAA_INSERT,   TRUE );	break;
		case IDM_MAA_INTERRUPT_EDIT:	AaItemsDoSelect( hWnd, MAA_INTERRUPT, TRUE );	break;
		case IDM_MAA_SET_LAYERBOX:		AaItemsDoSelect( hWnd, MAA_LAYERED,  TRUE );	break;
#endif
		case IDM_MAA_CLIP_UNICODE:		AaItemsDoSelect( hWnd, MAA_UNICLIP,  TRUE );	break;
		case IDM_MAA_CLIP_SHIFTJIS:		AaItemsDoSelect( hWnd, MAA_SJISCLIP, TRUE );	break;

		case IDM_DRAUGHT_ADDING:		AaItemsDoSelect( hWnd, MAA_DRAUGHT,  TRUE );	break;
#ifdef _ORRVW
		case IDM_DRAUGHT_OPEN:			Maa_OnCommand( hWnd, IDM_DRAUGHT_OPEN, NULL, 0 );	break;
#endif

		case IDM_MAA_AATIP_TOGGLE:
			gbAAtipView = gbAAtipView ? FALSE : TRUE;
			InitParamValue( INIT_SAVE, VL_MAATIP_VIEW, gbAAtipView );
			break;

		case IDM_MAA_SEP_STYLE_TOGGLE:
			gbLineSep = gbLineSep ? FALSE : TRUE;
			InitParamValue( INIT_SAVE, VL_MAASEP_STYLE, gbLineSep );
			InvalidateRect( ghItemsWnd, NULL, TRUE );
			break;

		case IDM_MAA_THUMBNAIL_OPEN:	Maa_OnCommand( hWnd , IDM_MAA_THUMBNAIL_OPEN, NULL, 0 );	break;

		case IDM_MAAITEM_BKCOLOUR:		MaaBackColourChoose( hWnd );	break;

#ifndef _ORRVW

		case IDM_MAA_RETURN_FOCUS:
			gbMaaRetFocus = gbMaaRetFocus ? FALSE : TRUE;
			InitParamValue( INIT_SAVE, VL_MAA_RETFCS, gbMaaRetFocus );
			break;

  #ifdef MAA_IADD_PLUS
		//	途中追加
		case IDM_MAA_ITEM_INSERT:		AacItemInsert( hWnd, gixNowSel );	break;
  #endif
#endif
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ドラッグンドロップの受け入れ
	@param[in]	hWnd	表示スタティックのハンドル
	@param[in]	hDrop	ドロッピンオブジェクトハンドゥ
*/
VOID Aai_OnDropFiles( HWND hWnd, HDROP hDrop )
{
	TCHAR	atFileName[MAX_PATH];
//	LPARAM	dNumber;

	ZeroMemory( atFileName, sizeof(atFileName) );

	DragQueryFile( hDrop, 0, atFileName, MAX_PATH );
	DragFinish( hDrop );

	TRACE( TEXT("AAI DROP[%s]"), atFileName );

	//	ドロッペされたファイルを副タブに展開しちゃう
	TabMultipleDropAdd( GetParent( hWnd ), atFileName );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	MLTファイル名もしくはお気に入り名を受け取って、内容をスタティックに入れる
	@param[in]	hWnd		親ウインドウハンドル
	@param[in]	ptFileName	ファイル名
	@param[in]	type		０(ACT_ALLTREE)：ファイル展開　１(ACT_FAVLIST)：SQL展開　２：副タブより
	@return		HRESULT		終了状態コード
*/
HRESULT AaItemsDoShow( HWND hWnd, LPTSTR ptFileName, UINT type )
{
	SCROLLINFO	stScrollInfo;

	ZeroMemory( &stScrollInfo, sizeof(SCROLLINFO) );
	stScrollInfo.cbSize = sizeof(SCROLLINFO);

	//	描画位置を最初からにして、再描画状態にする
	gixTopItem = 0;

	AaTitleClear(  );	//	ここでクルヤーしてみる

	//	開く
	switch( type )
	{
		case ACT_ALLTREE:	gixMaxItem = AacAssembleFile( hWnd, ptFileName );	break;
		case ACT_FAVLIST:	gixMaxItem = AacAssembleSql( hWnd, ptFileName );	break;
		default:	//	副タブ
			gixMaxItem = AacAssembleFile( hWnd, ptFileName );
			//	ラストメモリー・ここで描画位置を復元する
			gixTopItem = TabMultipleTopMemory( -1 );
			break;
	}


	InvalidateRect( ghItemsWnd, NULL, TRUE );
	UpdateWindow( ghItemsWnd );

	if( 0 == gixMaxItem )
	{
		stScrollInfo.fMask = SIF_DISABLENOSCROLL;
		SetScrollInfo( ghScrollWnd, SB_CTL, &stScrollInfo, TRUE );
		return E_FAIL;
	}

	stScrollInfo.fMask = SIF_ALL;
	stScrollInfo.nMax  = gixMaxItem-1;
	stScrollInfo.nPos  = gixTopItem;
	stScrollInfo.nPage = 1;
	stScrollInfo.nTrackPos = 0;
	SetScrollInfo( ghScrollWnd, SB_CTL, &stScrollInfo, TRUE );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	使ったＡＡをお気に入りＳＱＬにいれる
	@param[in]	pcConts		AAの文字列
	@param[in]	rdLength	バイト数
	@return		HRESULT		終了状態コード
*/
HRESULT AaItemsFavUpload( LPSTR pcConts, UINT rdLength )
{
	LPTSTR	ptBaseName;
	DWORD	dHash;

	ptBaseName = TreeBaseNameGet(  );	//	一覧ベース名取得して

	//	ドロッピンオブジェクトなら無視するので？
	if( !( StrCmp( DROP_OBJ_NAME, ptBaseName ) ) )	return E_ABORT;

	//	そのＡＡのハッシュ値を求めて
	HashData( (LPBYTE)pcConts, rdLength, (LPBYTE)(&(dHash)), 4 );

	//	お気に入りに記録する
	SqlFavUpload( ptBaseName, dHash, pcConts, rdLength );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	お気に入りリストから削除する
	@param[in]	pcConts		AAの文字列
	@param[in]	rdLength	バイト数
	@return		HRESULT		終了状態コード
*/
HRESULT AaItemsFavDelete( LPSTR pcConts, UINT rdLength )
{
	LPTSTR	ptBaseName;
	DWORD	dHash;

	ptBaseName = TreeBaseNameGet(  );	//	一覧ベース名取得して

	//	そのＡＡのハッシュ値を求めて
	HashData( (LPBYTE)pcConts, rdLength, (LPBYTE)(&(dHash)), 4 );

	//	お気に入りから削除する
	SqlFavDelete( ptBaseName, dHash );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------


/*!
	MAA窓の背景色選択ダイヤログの面倒見る
	@param[in]	hWnd	オーナーウインドウハンドル
	@return	HRESULT	終了状態コード
*/
HRESULT MaaBackColourChoose( HWND hWnd )
{
	BOOL	bRslt;
	COLORREF	adColourTemp[16], dColour;
	CHOOSECOLOR	stChColour;

	ZeroMemory( adColourTemp, sizeof(adColourTemp) );

	dColour = (COLORREF)InitParamValue( INIT_LOAD, VL_MAA_BKCOLOUR, 0x00FFFFFF );

	adColourTemp[0] = dColour;

	ZeroMemory( &stChColour, sizeof(CHOOSECOLOR) );
	stChColour.lStructSize  = sizeof(CHOOSECOLOR);
	stChColour.hwndOwner    = hWnd;
//	stChColour.hInstance    = GetModuleHandle( NULL );
	stChColour.rgbResult    = dColour;
	stChColour.lpCustColors = adColourTemp;
	stChColour.Flags        = CC_RGBINIT;

	bRslt = ChooseColor( &stChColour  );	//	色ダイヤログ使う
	if( bRslt )
	{
		dColour =  stChColour.rgbResult;
		InitParamValue( INIT_SAVE, VL_MAA_BKCOLOUR, (INT)dColour );

		DeleteBrush( ghBkBrush );	//	ブラシ作り直し
		ghBkBrush = CreateSolidBrush( dColour );

		InvalidateRect( ghItemsWnd, NULL, TRUE );

		return S_OK;
	}

	return E_ABORT;
}
//-------------------------------------------------------------------------------------------------




/*!
	ホウィール回転が自分の上で発生したか
	@param[in]	hWnd	親ウインドウハンドル
	@param[in]	hChdWnd	マウスカーソルの↓にあった子ウインドウ
	@param[in]	zDelta	回転量・正なら上方向、腐なら下方向へのスクロールと見なす
	@return		非０自分だった　０関係ないね
*/
UINT AaItemsIsUnderCursor( HWND hWnd, HWND hChdWnd, INT zDelta )
{
	UINT	dCode;

	if( ghItemsWnd != hChdWnd )	return 0;

	if( 0 < zDelta )		dCode = SB_LINEUP;
	else if( 0 > zDelta )	dCode = SB_LINEDOWN;
	else					dCode = SB_ENDSCROLL;

	FORWARD_WM_VSCROLL( hWnd, ghScrollWnd, dCode, 0, PostMessage );

	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	ターゲットしているAAを使う処理
	@param[in]	hWnd	多分AA一覧のウインドウハンドル
	@param[in]	dMode	使用モードもしくはデフォで
	@param[in]	dDirct	非０マウス直下ので　０表示トップので
	@return		UINT	非０ＡＡとった　０ＡＡ無かった
*/
UINT AaItemsDoSelect( HWND hWnd, UINT dMode, UINT dDirct )
{
	LPSTR		pcConts = NULL;
	UINT		uRslt;
	UINT_PTR	rdLength;

	//	該当するインデックスAAを引っ張ってくる
	if( dDirct ){	pcConts = AacAsciiArtGet( gixNowSel );	}
	else{			pcConts = AacAsciiArtGet( gixTopItem );	}

	if( !pcConts  ){	return 0;	}

	rdLength = strlen( pcConts );	//	文字列の長さ取得

	uRslt = ViewMaaMaterialise( hWnd, pcConts, rdLength, dMode );	//	本体に飛ばす
	//	EditorとViewerで本体が異なるので注意

	//	ここでお気に入りに入れる
	if( SUCCEEDED( AaItemsFavUpload( pcConts, rdLength ) ) )
	{
		FavContsRedrawRequest( hWnd );
	}

	free( pcConts );

	return 1;
}
//-------------------------------------------------------------------------------------------------


/*!
	ポッパップの文字サイズ変更と表示非表示が入った
	@param[in]	ttSize	文字サイズ・１２か１６しかない
	@param[in]	bView	非０表示　０表示しない
	@return		HRESULT	終了状態コード
*/
HRESULT AaItemsTipSizeChange( INT ttSize, UINT bView )
{
#ifdef MAA_TOOLTIP
	LOGFONT	stFont;
#endif
	gbAAtipView = bView ? TRUE : FALSE;

#ifdef MAA_TOOLTIP
	SetWindowFont( ghToolTipWnd, GetStockFont(DEFAULT_GUI_FONT), FALSE );
	DeleteFont( ghTipFont );

	//	今使ってるヤツぶっ壊してから、新しいのつくってくっつける
	ViewingFontGet( &stFont );	//	gstBaseFont
	stFont.lfHeight = (FONTSZ_REDUCE == ttSize) ? FONTSZ_REDUCE : FONTSZ_NORMAL;
	ghTipFont = CreateFontIndirect( &stFont );

	SetWindowFont( ghToolTipWnd, ghTipFont, TRUE );
#endif

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

#ifdef USE_HOVERTIP
/*!
	HoverTip用のコールバック受取
	@param[in]	pVoid	未定義
	@return	確保した文字列・もしくはNULL
*/
LPTSTR CALLBACK AaItemsHoverTipInfo( LPVOID pVoid )
{
	UINT_PTR	rdLength;
	LPSTR		pcConts = NULL;
	LPTSTR		ptBuffer = NULL;


	if( !(gbAAtipView) ){		return NULL;	}	//	非表示なら何もしないでおｋ
	if( 0 > gixNowToolTip ){	return NULL;	}

	pcConts = AacAsciiArtGet( gixNowToolTip );	//	該当するインデックスのＡＡを引っ張ってくる
	if( !pcConts  ){	return 0;	}

	ptBuffer = SjisDecodeAlloc( pcConts );
	rdLength = lstrlen( ptBuffer  );	//	文字列の長さ取得

	free( pcConts );

	TRACE( TEXT("MAA HOVER CALL %d, by[%d]"), gixNowToolTip, rdLength );

	return ptBuffer;
}
//-------------------------------------------------------------------------------------------------
#endif


#ifdef MAA_TEXT_FIND


/*!
	検索開始ボタン押され・初回と弐回目以降の区別に注意
	@param[in]	hWnd	親ウインドウのハンドル
	@param[in]	bMode	非０ボタンによる検索開始　０前の条件で次の頁から検索開始
	@return		HRESULT	終了状態コード
*/
HRESULT AacFindTextEntry( HWND hWnd, UINT bMode )
{
	TCHAR	atString[MAX_STRING];
	UINT	isNowPage, dRslt;
	INT		iPage, i;

	TRACE( TEXT("MAA：検索始め") );
	//	Ｆ３なら、今の頁から。既存の文字列で
	//	ボタンなら、文字列が異なっていたら指定に従う・同じならつづきから

	ZeroMemory( atString, sizeof(atString) );

	SetDlgItemText( hWnd, IDS_MAA_TXTFIND_MSGBOX, TEXT("") );

	if( bMode )	//	ボタンから
	{
		GetDlgItemText( hWnd, IDE_MAA_TXTFIND_TEXT, atString, MAX_STRING );
		//	エディットボッキスが空ならなんもせん
		if( NULL == atString[0] )	return E_NOTIMPL;

		//	０ファイル先頭から　１今の天辺頁から
		isNowPage = IsDlgButtonChecked( hWnd, IDB_MAA_TXTFIND_TOP_GO ) ? FALSE : TRUE;
	}
	else	//	Ｆ３で
	{
		//	検索条件が無いときはなにもしない
		if( NULL == gatFindText[0] )	return E_NOTIMPL;

		//	バッファからコピーしておく・これにより前回と同じ条件扱いになる
		StringCchCopy( atString, MAX_STRING, gatFindText );
	}

//初回と弐回目以降の区別どうするか
	if( StrCmp( gatFindText, atString ) )	//	異なるから、今回初めてといえる
	{
		//	バッファにコピー
		StringCchCopy( gatFindText, MAX_STRING, atString );

		//	検索開始位置
		if( !(isNowPage)  ){	iPage = 0;	}	//	先頭
		else{	iPage =  gixTopItem + 1;	}	//	今の次の頁から
		if( gixMaxItem <= iPage ){	iPage = 0;	}	//	振り切ったら先頭に戻る

	}
	else	//	同じなら１度処理した後のはず・続きから処理を続ける
	{
		iPage =  gixTopItem + 1;	//	今の次の頁から
		if( gixMaxItem <= iPage ){	iPage = 0;	}	//	振り切ったら先頭に戻る
	}

	dRslt = FALSE;
	//	頁を順番にみていく
	for( i = 0; gixMaxItem > i; i++ )	//	全頁回したら終わり
	{
		dRslt = AacItemFindOnePage( hWnd, atString, iPage );
		if( dRslt ){	break;	}	//	ヒット

		//	今見た頁には無かった場合
		iPage++;
		if( gixMaxItem <= iPage ){	iPage = 0;	}	//	振り切ったら先頭に戻る
	}

	if( dRslt )
	{
		Aai_OnVScroll( hWnd, ghScrollWnd, SBP_DIRECT, iPage );	//	該当する位置へジャンプ
	}
	else
	{
		SetDlgItemText( hWnd, IDS_MAA_TXTFIND_MSGBOX, TEXT("見つからないよ") );
		//MessageBox( hWnd, TEXT("見つからなかったよ"), TEXT("お燐からのお知らせ"), MB_OK );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定の頁を、指定条件を受けて検索する
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	ptFindText	検索したい文字列
	@param[in]	iTargetPage	検索頁
	@return	UINT	非０その頁に文字列があった　０なかった
*/
UINT AacItemFindOnePage( HWND hWnd, LPTSTR ptFindText, INT iTargetPage )
{
	LPSTR	pcItem;
	LPTSTR	ptTarget, ptFindPos;
	INT		iMoziPos;
	UINT	dFound = FALSE;

	//	中身もってくる
	pcItem = AacAsciiArtGet( iTargetPage );
	//	ユニコードに直して比較する
	ptTarget = SjisDecodeAlloc( pcItem );
	FREE( pcItem );

	ptFindPos = FindStringProc( ptTarget, ptFindText, &iMoziPos );	//	Viewer側
	if( ptFindPos ){	dFound = TRUE;	}	//	存在してるか？

	FREE( ptTarget );

	return dFound;
}
//-------------------------------------------------------------------------------------------------


#endif




#ifndef MAA_IADD_PLUS

#ifndef _ORRVW

typedef struct tagITEMADDINFO
{
	LPTSTR	ptContent;			//!<	本文内容
	TCHAR	atSep[MAX_PATH];	//!<	セパレータ内容
	INT		bType;				//!<	非０MLT　０AST

} ITEMADDINFO, *LPITEMADDINFO;
//--------------------------------------

/*!
	ツリーのファイルにアイテム追加・追加したいファイルのパスを受け取る
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	ptFile	追加したいファイルのフルパス
*/
HRESULT AacItemAdding( HWND hWnd, LPTSTR ptFile )
{
	HANDLE	hFile;

	LPTSTR		ptBuffer;//, ptExten;
	LPSTR		pcOutput;
//	TCHAR		atExBuf[10];
	CHAR		acCheck[6];
	DWORD		readed, wrote;
	UINT_PTR	cchSize, cchSep, cbSize;
	ITEMADDINFO	stIaInfo;	

	ZeroMemory( &stIaInfo, sizeof(ITEMADDINFO) );

	ZeroMemory( acCheck, sizeof(acCheck) );

	//	拡張子確認
	if( FileExtensionCheck( ptFile, TEXT(".ast") ) ){	stIaInfo.bType =  0;	}
	else{	stIaInfo.bType =  1;	}

	if( DialogBoxParam( GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAA_IADD_DLG), hWnd, AaItemAddDlgProc, (LPARAM)(&stIaInfo) ) )
	{
		if( stIaInfo.ptContent )	//	中身が有効なら処理する
		{
			hFile = CreateFile( ptFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
			if( INVALID_HANDLE_VALUE != hFile )
			{
				//	ファイルはSJIS型であると見なす
				SetFilePointer( hFile, -2, NULL, FILE_END );	//	末尾の状況を確認
				ReadFile( hFile, acCheck, 6, &readed, NULL );	//	この時点でファイル末尾にくる
				SetFilePointer( hFile,  0, NULL, FILE_END );	//	念のため末尾
				if( acCheck[0] != '\r' || acCheck[1] != '\n' )	//	末尾が改行じゃなかったら
				{
					acCheck[0] = '\r';	acCheck[1] = '\n';	acCheck[2] = NULL;
					WriteFile( hFile, acCheck, 2, &wrote, NULL );	//	改行書いておく
				}

				StringCchLength( stIaInfo.atSep, MAX_PATH, &cchSep );
				StringCchLength( stIaInfo.ptContent, STRSAFE_MAX_CCH, &cchSize );
				cchSize += (cchSep+6);
				ptBuffer = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
				ZeroMemory( ptBuffer, cchSize * sizeof(TCHAR) );

				StringCchPrintf( ptBuffer, cchSize, TEXT("%s%s\r\n"), stIaInfo.atSep, stIaInfo.ptContent );
				pcOutput = SjisEncodeAlloc( ptBuffer );
				cbSize = strlen( pcOutput );

				WriteFile( hFile , pcOutput, cbSize, &wrote, NULL );	//	内容書き込む

				SetEndOfFile( hFile );
				CloseHandle( hFile );
			}

			FREE(stIaInfo.ptContent);
		}

		//	もしメインで開けていたらロード・ていうかツリー側で追加っておかしくね？
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	アイテム追加の面倒見るダイヤログー
	@param[in]	hDlg		ダイヤログハンドル
	@param[in]	message		ウインドウメッセージの識別番号
	@param[in]	wParam		追加の情報１
	@param[in]	lParam		追加の情報２
	@retval 0	メッセージは処理していない
	@retval no0	なんか処理された
*/
INT_PTR CALLBACK AaItemAddDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static LPITEMADDINFO	pstIaInfo;
	static LPTSTR	ptBuffer;
	UINT_PTR	cchSize;
	TCHAR	atName[MAX_PATH];
	INT		id;
	RECT	rect;

	switch( message )
	{
		case WM_INITDIALOG:
			pstIaInfo = (LPITEMADDINFO)(lParam);
			GetClientRect( hDlg, &rect );
			CreateWindowEx( 0, WC_BUTTON, TEXT("今の頁"),         WS_CHILD | WS_VISIBLE, 0, 0, 75, 23, hDlg, (HMENU)IDB_MAID_NOWPAGE, GetModuleHandle(NULL), NULL );
			CreateWindowEx( 0, WC_BUTTON, TEXT("クリップボード"), WS_CHILD | WS_VISIBLE, 75, 0, 120, 23, hDlg, (HMENU)IDB_MAID_CLIPBOARD, GetModuleHandle(NULL), NULL ); 
			CreateWindowEx( 0, WC_EDIT,   TEXT(""),               WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 195, 0, rect.right-195-50, 23, hDlg, (HMENU)IDE_MAID_ITEMNAME, GetModuleHandle(NULL), NULL );
			CreateWindowEx( 0, WC_BUTTON, TEXT("追加"),           WS_CHILD | WS_VISIBLE, rect.right-50, 0, 50, 23, hDlg, (HMENU)IDB_MAID_ADDGO, GetModuleHandle(NULL), NULL );
			CreateWindowEx( 0, WC_EDIT,   TEXT(""),               WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY, 0, 23, rect.right, rect.bottom-23, hDlg, (HMENU)IDE_MAID_CONTENTS, GetModuleHandle(NULL), NULL );
			//	なんでわざわざこうやったのか思い出せない。

			if( pstIaInfo->bType )
			{
				SetDlgItemText( hDlg, IDE_MAID_ITEMNAME, TEXT("名称はASTでないと使用できないのです") );
				EnableWindow( GetDlgItem(hDlg,IDE_MAID_ITEMNAME), FALSE );
				StringCchCopy( pstIaInfo->atSep, MAX_PATH, TEXT("[SPLIT]\r\n") );
			}

			//	とりあえずクリップボードの中身をとる
			ptBuffer = DocClipboardDataGet( NULL );
			if( !(ptBuffer) ){	DocPageGetAlloc( D_UNI , (LPVOID *)(&ptBuffer) );	}
			//	使えないシロモノなら、今の頁の内容を持ってきて表示
			SetDlgItemText( hDlg, IDE_MAID_CONTENTS, ptBuffer );
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			id = LOWORD(wParam);
			switch( id )
			{
				case IDCANCEL:
					FREE(ptBuffer);
					EndDialog(hDlg, 0 );
					return (INT_PTR)TRUE;

				case IDB_MAID_ADDGO:
					if( ptBuffer )
					{
						StringCchLength( ptBuffer, STRSAFE_MAX_CCH, &cchSize );
						cchSize += 2;
						pstIaInfo->ptContent = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
						StringCchCopy( pstIaInfo->ptContent, cchSize, ptBuffer );

						if( !(pstIaInfo->bType) )
						{
							GetDlgItemText( hDlg, IDE_MAID_ITEMNAME, atName, MAX_PATH );
							StringCchPrintf( pstIaInfo->atSep, MAX_PATH, TEXT("[AA][%s]\r\n"), atName );
						}
					}
					FREE(ptBuffer);
					EndDialog(hDlg, 1 );
					return (INT_PTR)TRUE;

				case IDB_MAID_CLIPBOARD:
					FREE(ptBuffer);
					ptBuffer = DocClipboardDataGet( NULL );
					SetDlgItemText( hDlg, IDE_MAID_CONTENTS, ptBuffer );
					return (INT_PTR)TRUE;

				case IDB_MAID_NOWPAGE:
					FREE(ptBuffer);
					DocPageGetAlloc( D_UNI , (LPVOID *)(&ptBuffer) );
					SetDlgItemText( hDlg, IDE_MAID_CONTENTS, ptBuffer );
					return (INT_PTR)TRUE;

				default:	break;
			}
			break;

		case WM_SIZE:
			GetClientRect( hDlg, &rect );
			MoveWindow( GetDlgItem(hDlg,IDE_MAID_ITEMNAME), 195, 0, rect.right-195-50, 23, TRUE );
			MoveWindow( GetDlgItem(hDlg,IDB_MAID_ADDGO),    rect.right-50, 0, 50, 23, TRUE );
			MoveWindow( GetDlgItem(hDlg,IDE_MAID_CONTENTS), 0, 23, rect.right, rect.bottom-23, TRUE );
			break;
	}

	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------
	
	
#endif

#endif	//	MAA_IADD_PLUS

