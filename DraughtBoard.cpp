/*! @file
	@brief ドラフトボードの管理します
	このファイルは DraughtBoard.cpp です。
	@author	SikigamiHNQ
	@date	2011/11/30
*/

/*
Orinrin Editor : AsciiArt Story Editor for Japanese Only
Copyright (C) 2011 Orinrin/SikigamiHNQ

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.
If not, see <http://www.gnu.org/licenses/>.
*/
//-------------------------------------------------------------------------------------------------

/*
Safariのリーディングリストみたいな機能。
エフェクト合成のときとか、気になるAAをいくつか登録しておき、右クリとかで呼び出せるようにしておく。
終わったらリセットとか、削除とか。
MAAから取り込む、クリップボードから取り込む、とかできるといいかも。取り込みはドラッグドロップできない？
使用するヤツをくっつける先は？クリップボード、レイヤボックスとか？
MLT/ASTにエクスポート、みたいなことも出来ると良い。機能自体はオンメモリでいい。UseItLater的なもの。
リストの中身の確認どうするか。なんとかしてサムネイル表示を？PNGにして表示とか？
サムネは128x128くらいで。5x3に並べるとか。スクロールバーも考慮
メニューにポップアップはできないのか
Ctrl+Spaceでサムネイルウインドウ出したり閉じたり。選択して使える。MAAのほうでもなんか出来ない？
右クリにはボード呼出とか、選択範囲をボードに追加とか？
名称：DraughtBoard　ドラフトボードとかいいかも

ボードからの仕様・エヂタに挿入、クルッペボード、レイヤボッキスあたりか

記録形式どうするか。SQLiteかvector？　サムネールイメージをどうするかによって変わるか

MAAのサムネ表示にも使うなら、スクロールバー考慮。呼出キーバインドも考える。Ctrl+Tとか？
*/

#include "stdafx.h"
#include "OrinrinEditor.h"
//-------------------------------------------------------------------------------------------------

//	TODO:	サムネがDCリソース食い過ぎか。非表示のは積極的にOFFるほうがよさそう
//もしくはサムネサイズに縮小して保持するか
//ＤＣまで要らない、ＢＭＰだけでいいかも

#define DRAUGHT_BOARD_CLASS	TEXT("DRAUGHT_BOARD")

//	一枚のパネルサイズ・小さいか？
#define THM_WIDTH	128
#define THM_HEIGHT	128

//	パネルは５ｘ３に並べる
#define TPNL_HORIZ	5
#define TPNL_VERTI	3
//-------------------------------------------------------------------------------------------------

#ifdef DRAUGHT_STYLE

//	MAAのと共通でいける
//typedef struct tagDRAUGHTITEM
//{
//	LPSTR	pcAaText;	//!<	AA文字列・Sjisで確保
//
//	INT		iMaxDot;	//!<	横幅最大ドット数
//	INT		iLines;		//!<	使用行数
//
//	SIZE	stSize;		//!<	ピクセルサイズ
//	HDC		hThumbDC;	//!<	サムネイル用デバイスコンテキスト
//	HBITMAP	hThumbBmp;	//!<	サムネイル用ビットマップハンドル
//	HBITMAP	hOldBmp;	//!<	復帰用
//
//} DRAUGHTITEM, *LPDRAUGHTITEM;
//-------------------------------------------------------------------------------------------------

extern HFONT	ghAaFont;		//	AA用フォント

static  ATOM	gDraughtAtom;	//!<	ウインドウクラスアトム
static  HWND	ghDraughtWnd;	//!<	このウインドウハンドル
static  HWND	ghDraughtTip;	//!<	ツールチップ

EXTERNED UINT	gdClickMode;	//!<	アイテムをクルックしたときの基本動作・０通常挿入　１矩形挿入　２レイヤボックス開く　３UNIクリップ　４SJISクリップ
//クルッペボードへコピるモードはコピーモードスワップに従う

static HDC		ghNonItemDC;	//!<	アイテム無しの絵
static HBITMAP	ghNonItemBMP, ghOldBmp;	
static HPEN		ghLinePen;

static INT		giTarget;		//!<	クルックしたアイテム番号・－１で無し

#ifdef THUMBNAIL_STYLE
static  UINT	gbThumb;		//!<	サムネ状態であるか
static  LONG	gdVwTop;		//!<	表示されてる一番左上の行番号０インデックス
static  HWND	ghScrBarWnd;	//!<	サムネ用スクロールバァー
#endif
static vector<AAMATRIX>	gvcDrtItems;	//!<	
//typedef vector<DRAUGHTITEM>::iterator	DRTI_ITR;	//!<	
//-------------------------------------------------------------------------------------------------

INT		DraughtTargetItemSet( LPPOINT );	//!<	
DOUBLE	DraughtAspectKeeping( LPSIZE, UINT );	//!<	
INT		DraughtItemDelete( CONST INT  );	//!<	
HRESULT	DraughtItemUse( INT );	//!<	

LRESULT CALLBACK DraughtProc( HWND, UINT, WPARAM, LPARAM );
VOID	Drt_OnCommand( HWND , INT, HWND, UINT );		//!<	
VOID	Drt_OnPaint( HWND );							//!<	
//VOID	Drt_OnSize( HWND , UINT, INT, INT );			//!<	
LRESULT	Drt_OnNotify( HWND , INT, LPNMHDR );			//!<	
VOID	Drt_OnLButtonUp( HWND, INT, INT, UINT );		//!<	
VOID	Drt_OnContextMenu( HWND, HWND, UINT, UINT );	//!<	
VOID	Drt_OnDestroy( HWND );							//!<	
VOID	Drt_OnKillFocus( HWND, HWND );					//!<	
VOID	Drt_OnVScroll( HWND , HWND, UINT, INT );		//!<	
VOID	Drt_OnMouseWheel( HWND, INT, INT, INT, UINT );	//!<	

//-------------------------------------------------------------------------------------------------

/*!
	テンポラったAAを表示するウインドウの作成
	@param[in]	hInstance	アプリのインスタンス
	@param[in]	hPtWnd		メイン窓ウインドウハンドル
	@return	特になし
*/
HRESULT DraughtInitialise( HINSTANCE hInstance, HWND hPtWnd )
{
	WNDCLASSEX	wcex;


	if( hInstance )
	{
		ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
		wcex.cbSize			= sizeof(WNDCLASSEX);
		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= DraughtProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= hInstance;
		wcex.hIcon			= NULL;
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wcex.lpszMenuName	= NULL;
		wcex.lpszClassName	= DRAUGHT_BOARD_CLASS;
		wcex.hIconSm		= NULL;

		gDraughtAtom = RegisterClassEx( &wcex );

		ghNonItemDC = NULL;

#ifndef _ORRVW
		//	クルック動作指定ロード・デフォ動作はSJISクリップ
		gdClickMode = InitParamValue( INIT_LOAD, VL_DRAUGHT_MODE, 4 );
		//	Viewerの場合はコピーモードに従う
#endif
	}
	else
	{
		if( ghNonItemDC )
		{
			SelectBitmap( ghNonItemDC, ghOldBmp );
			SelectPen( ghNonItemDC, GetStockPen(NULL_PEN) );
			DeleteObject( ghNonItemDC );
		}
		if( ghNonItemBMP  ){	DeleteBitmap( ghNonItemBMP );	}
		if( ghLinePen ){	DeletePen( ghLinePen  );	}

		DraughtItemDelete( -1 );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------


/*!
	表示用ウインドウを作る
	@param[in]	hInstance	アプリのインスタンス
	@param[in]	hPtWnd		メイン窓ウインドウハンドル
	@param[in]	bThumb		非０MAAのサムネ表示として呼ばれた
	@return	作ったウインドウのハンドル
*/
HWND DraughtWindowCreate( HINSTANCE hInstance, HWND hPtWnd, UINT bThumb )
{
	INT_PTR	iItems;
	INT		iRslt, iScWid = 0;
	HDC		hdc;

	RECT	wdRect, rect;

#ifdef THUMBNAIL_STYLE
	INT		iLines, iStep = 0;
	LONG	rigOffs = 0;
	SCROLLINFO	stScrollInfo;
#endif

	if( !(hPtWnd ) )	//	破壊する・いらない？
	{


		return NULL;
	}

	//	已に出来ていたらそのまま返す
	if( ghDraughtWnd  ){	UpdateWindow( ghDraughtWnd );	return ghDraughtWnd;	}

	gbThumb = bThumb;

	iItems = gvcDrtItems.size( );	//	現在個数・ここでは使わない

	GetWindowRect( hPtWnd, &wdRect );
	rect.left   = wdRect.left + 32;	//	オフセット値に特に意味はない
	rect.top    = wdRect.top  + 32;
	rect.right  = THM_WIDTH  * TPNL_HORIZ;
	rect.bottom = THM_HEIGHT * TPNL_VERTI;

#ifdef THUMBNAIL_STYLE
	if( gbThumb )	//	サムネモード
	{
		gdVwTop = 0;

		iItems = AacItemCount( 0  );	//	現在個数・スクロールバーの刻み設定につかう
		if( 0 >= iItems )	return NULL;	//	アイテム開いてないなら何もしない

		iLines = (iItems + (TPNL_HORIZ-1)) / TPNL_HORIZ;	//	行数・切り上げ処理

		iStep = iLines - TPNL_VERTI;	//	すくろるば～の刻み数
		if( 0 > iStep ){	iStep = 0;	}

		rigOffs = rect.right;

		iScWid = GetSystemMetrics( SM_CXVSCROLL );	//	垂直スクロールバーの幅確保
		rect.right += iScWid;
	}
#endif

	//	ウインドウ作成	TOPMOSTいるか？	
	ghDraughtWnd = CreateWindowEx( WS_EX_TOOLWINDOW,// | WS_EX_TOPMOST | WS_EX_CLIENTEDGE,
		DRAUGHT_BOARD_CLASS, TEXT("Draught Board"), WS_POPUP | WS_VISIBLE | WS_BORDER,
		rect.left, rect.top, rect.right, rect.bottom, NULL, NULL, hInstance, NULL );

#ifdef THUMBNAIL_STYLE
	if( gbThumb )	//	サムネモード
	{
		//	一覧のスクロールバー
		ghScrBarWnd = CreateWindowEx( 0, WC_SCROLLBAR, TEXT("scroll"), WS_VISIBLE | WS_CHILD | SBS_VERT,
			rigOffs, 0, iScWid, rect.bottom, ghDraughtWnd, (HMENU)IDSB_DRT_THUM_SCROLL, hInstance, NULL );

		ZeroMemory( &stScrollInfo, sizeof(SCROLLINFO) );
		stScrollInfo.cbSize = sizeof(SCROLLINFO);
		stScrollInfo.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		stScrollInfo.nMax  = iStep;
		stScrollInfo.nPos  = 0;
		stScrollInfo.nPage = 1;
		stScrollInfo.nTrackPos = 0;
		SetScrollInfo( ghScrBarWnd, SB_CTL, &stScrollInfo, TRUE );
	}
#endif

	if( !(ghNonItemDC) )	//	穴埋め描画用ビットマップ作成
	{
		hdc = GetDC( ghDraughtWnd );

		ghNonItemDC  = CreateCompatibleDC( hdc );
		ghNonItemBMP = CreateCompatibleBitmap( hdc, THM_WIDTH, THM_HEIGHT );

		ghOldBmp = SelectBitmap( ghNonItemDC, ghNonItemBMP );
		SelectFont( ghNonItemDC, ghAaFont );

		iRslt = PatBlt( ghNonItemDC, 0, 0, THM_WIDTH, THM_HEIGHT, WHITENESS );

		ReleaseDC( ghDraughtWnd, hdc );

		SetRect( &rect, 0, 0, THM_WIDTH, THM_HEIGHT );
		iRslt = DrawText( ghNonItemDC, TEXT("NO ITEM"), 7, &rect, DT_CENTER | DT_VCENTER | DT_NOPREFIX | DT_NOCLIP | DT_SINGLELINE );

		SelectFont( ghNonItemDC, GetStockFont(DEFAULT_GUI_FONT) );

		ghLinePen = CreatePen( PS_SOLID, 1, 0 );
		SelectPen( ghNonItemDC, ghLinePen );
	}

	UpdateWindow( ghDraughtWnd );

	return ghDraughtWnd;
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
LRESULT CALLBACK DraughtProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
	//	HANDLE_MSG( hWnd, WM_SIZE,        Drt_OnSize );	
		HANDLE_MSG( hWnd, WM_COMMAND,     Drt_OnCommand );	
	//	HANDLE_MSG( hWnd, WM_NOTIFY,      Drt_OnNotify );	//	コモンコントロールの個別イベント
		HANDLE_MSG( hWnd, WM_LBUTTONUP,   Drt_OnLButtonUp );
		HANDLE_MSG( hWnd, WM_PAINT,       Drt_OnPaint );
		HANDLE_MSG( hWnd, WM_CONTEXTMENU, Drt_OnContextMenu );
		HANDLE_MSG( hWnd, WM_DESTROY,     Drt_OnDestroy );
		HANDLE_MSG( hWnd, WM_KILLFOCUS,   Drt_OnKillFocus );
		HANDLE_MSG( hWnd, WM_VSCROLL,     Drt_OnVScroll );	
		HANDLE_MSG( hWnd, WM_MOUSEWHEEL,  Drt_OnMouseWheel );		//	

//		case WM_CLOSE:	ShowWindow( ghDraughtWnd, SW_HIDE );	return 0;

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
VOID Drt_OnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify )
{
	
	switch( id )
	{
		case IDM_DRAUGHT_INSERTEDIT:
		case IDM_DRAUGHT_INTERRUPTEDIT:
		case IDM_DRAUGHT_LAYERBOX:
		case IDM_DRAUGHT_UNICLIP:
		case IDM_DRAUGHT_SJISCLIP:	DraughtItemUse( id );	DestroyWindow( hWnd );	break;

		case IDM_DRAUGHT_DELETE:	DraughtItemDelete( giTarget );	InvalidateRect( hWnd , NULL, TRUE );	break;
#ifndef _ORRVW
		case IDM_DRAUGHT_EXPORT:	NotifyBalloonExist( TEXT("未実装なのです・・・"), TEXT("あぅあぅ"), NIIF_ERROR );	break;
#endif
		case IDM_DRAUGHT_ALLDELETE:	DraughtItemDelete( -1 );	DestroyWindow( hWnd );	break;

		case IDM_DRAUGHT_CLOSE:		DestroyWindow( hWnd );	break;
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	PAINT。無効領域が出来たときに発生。背景の扱いに注意。背景を塗りつぶしてから、オブジェクトを描画
	@param[in]	hWnd	親ウインドウのハンドル
	@return		無し
*/
VOID Drt_OnPaint( HWND hWnd )
{
	PAINTSTRUCT	ps;
	HDC			hdc, hAaDC;
	HBITMAP		hOldBmp, hBmp;
	UINT		x = 0, y = 0;
	INT_PTR		iItems;
	SIZE		stSize, stOrgSize;

	MAAM_ITR	itItem;


	hdc = BeginPaint( hWnd, &ps );

	SetStretchBltMode( hdc, HALFTONE );

#ifdef THUMBNAIL_STYLE
	if( gbThumb )	//	サムネモード
	{
		iItems = gdVwTop * TPNL_HORIZ;

		for( y = 0; TPNL_VERTI > y; y++ )
		{
			for( x = 0; TPNL_HORIZ > x; x++ )
			{
				hBmp = AacArtImageGet( iItems, &stSize );
				if( hBmp )
				{
					stOrgSize = stSize;
					DraughtAspectKeeping( &stSize, TRUE );

					hAaDC = CreateCompatibleDC( hdc );
					hOldBmp = SelectBitmap( hAaDC, hBmp );

					StretchBlt( hdc, (x * THM_WIDTH), (y * THM_HEIGHT), stSize.cx, stSize.cy,	//	コピー先ＤＣ、左上ＸＹ、幅、高さ
						hAaDC, 0, 0, stOrgSize.cx, stOrgSize.cy,	//	コピー元ＤＣ、左上ＸＹ、幅、高さ
						SRCCOPY );	//	ラスタオペレーションコード
					iItems++;

					SelectBitmap( hAaDC, hOldBmp );
					DeleteDC( hAaDC );
				}
				else
				{
					BitBlt( hdc, (x * THM_WIDTH), (y * THM_HEIGHT), THM_WIDTH, THM_HEIGHT, ghNonItemDC, 0, 0, SRCCOPY );
				}
			}
		}
	}
	else
	{
#endif
		itItem = gvcDrtItems.begin();

		for( y = 0; TPNL_VERTI > y; y++ )
		{
			for( x = 0; TPNL_HORIZ > x; x++ )
			{
				if( itItem != gvcDrtItems.end() )
				{
					stSize = itItem->stSize;
					DraughtAspectKeeping( &stSize, TRUE );

					hAaDC = CreateCompatibleDC( hdc );
					hOldBmp = SelectBitmap( hAaDC, itItem->hThumbBmp );

					StretchBlt( hdc, (x * THM_WIDTH), (y * THM_HEIGHT), stSize.cx, stSize.cy,	//	コピー先ＤＣ、左上ＸＹ、幅、高さ
						hAaDC, 0, 0, itItem->stSize.cx, itItem->stSize.cy,	//	コピー元ＤＣ、左上ＸＹ、幅、高さ
						SRCCOPY );	//	ラスタオペレーションコード	//	itItem->hThumbDC
					itItem++;

					SelectBitmap( hAaDC, hOldBmp );
					DeleteDC( hAaDC );
				}
				else
				{
					BitBlt( hdc, (x * THM_WIDTH), (y * THM_HEIGHT), THM_WIDTH, THM_HEIGHT, ghNonItemDC, 0, 0, SRCCOPY );
				}
			}
		}
#ifdef THUMBNAIL_STYLE
	}
#endif

	for( y = 1; TPNL_HORIZ > y; y++ )
	{
		MoveToEx( hdc, (y * THM_HEIGHT), 0, NULL );
		LineTo( hdc, (y * THM_HEIGHT), (THM_HEIGHT * TPNL_VERTI) );
	}

	for( x = 1; TPNL_VERTI > x; x++ )
	{
		MoveToEx( hdc, 0, (x * THM_WIDTH), NULL );
		LineTo(   hdc, (THM_WIDTH * TPNL_HORIZ), (x * THM_WIDTH) );
	}

	EndPaint( hWnd, &ps );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	フォーカスを失った場合
*/
VOID Drt_OnKillFocus( HWND hWnd, HWND hwndNewFocus )
{
	DestroyWindow( hWnd );

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
LRESULT Drt_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{


	return 0;
}
//-------------------------------------------------------------------------------------------------

/*!
	マウスの左ボタンがうっｐされたとき
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	x			発生したＸ座標値
	@param[in]	y			発生したＹ座標値
	@param[in]	keyFlags	他に押されてるキーについて
*/
VOID Drt_OnLButtonUp( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	POINT	stPos;
	INT		id;

	TRACE( TEXT("LUP %d x %d"), x, y );	//	クライヤント座標

	stPos.x = x;
	stPos.y = y;

	giTarget = DraughtTargetItemSet( &stPos );

	switch( gdClickMode )
	{
#ifndef _ORRVW
		case  0:	id = IDM_DRAUGHT_INSERTEDIT;	break;
		case  1:	id = IDM_DRAUGHT_INTERRUPTEDIT;	break;
		case  2:	id = IDM_DRAUGHT_LAYERBOX;	break;
#endif
		default:
		case  3:	id = IDM_DRAUGHT_UNICLIP;	break;
		case  4:	id = IDM_DRAUGHT_SJISCLIP;	break;
	}

	FORWARD_WM_COMMAND( hWnd, id, ghDraughtWnd, 0, SendMessage );

	return;
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
VOID Drt_OnContextMenu( HWND hWnd, HWND hWndContext, UINT xPos, UINT yPos )
{
	HMENU	hMenu, hSubMenu;
	UINT	dRslt;

	POINT	stPoint, stPos;

	stPoint.x = (SHORT)xPos;	//	画面座標はマイナスもありうる
	stPoint.y = (SHORT)yPos;

	TRACE( TEXT("CTX %d x %d"), stPoint.x, stPoint.y );

	stPos = stPoint;
	ScreenToClient( hWnd, &stPos );
	giTarget = DraughtTargetItemSet( &stPos );

	hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_DRAUGHT_POPUP) );
	hSubMenu = GetSubMenu( hMenu, 0 );

	dRslt = TrackPopupMenu( hSubMenu, 0, stPoint.x, stPoint.y, 0, hWnd, NULL );
	//	選択せずで０か－１？、TPM_RETURNCMD無かったら、選択したらそのメニューのＩＤでWM_COMMANDが発行
	DestroyMenu( hMenu );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ウインドウを閉じるときに発生。
	@param[in]	hWnd	親ウインドウのハンドル
	@return		無し
*/
VOID Drt_OnDestroy( HWND hWnd )
{
	ghDraughtWnd = NULL;
	ghScrBarWnd  = NULL;

	return;
}
//-------------------------------------------------------------------------------------------------


/*!
	ホイール大回転
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	xPos	スクリーンＸ座標
	@param[in]	yPos	スクリーンＹ座標
	@param[in]	zDelta	回転量・１２０単位・WHEEL_DELTA
	@param[in]	fwKeys	他に押されていたキー
*/
VOID Drt_OnMouseWheel( HWND hWnd, INT xPos, INT yPos, INT zDelta, UINT fwKeys )
{
	UINT	dCode;

//	HWND	hChdWnd;
//	POINT	stPoint;

	TRACE( TEXT("POS[%d x %d] DELTA[%d] K[%X]"), xPos, yPos, zDelta, fwKeys );
	//	fwKeys	SHIFT 0x4, CTRL 0x8

	if( 0 < zDelta )		dCode = SB_LINEUP;
	else if( 0 > zDelta )	dCode = SB_LINEDOWN;
	else					dCode = SB_ENDSCROLL;

	Drt_OnVScroll( hWnd, ghScrBarWnd, dCode, 1 );	//	posは多分未使用

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	スクロールの処理
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	hwndCtl	スクロールバーのウインドウハンドル
	@param[in]	code	動作状態コード
	@param[in]	pos		つまみの位置
	@return		処理した内容とか
*/
VOID Drt_OnVScroll( HWND hWnd, HWND hwndCtl, UINT code, INT pos )
{
	INT	maePos;
	SCROLLINFO	stScrollInfo;

//	if( ghScrBarWnd != hwndCtl )	return;

	//	状態をくやしく
	ZeroMemory( &stScrollInfo, sizeof(SCROLLINFO) );
	stScrollInfo.cbSize = sizeof(SCROLLINFO);
	stScrollInfo.fMask = SIF_ALL;
	GetScrollInfo( ghScrBarWnd, SB_CTL, &stScrollInfo );

	maePos = gdVwTop;

	switch( code )	//	スクロール方向に合わせて内容をずらす
	{
		case SB_TOP:
			gdVwTop = 0;
			break;

		case SB_LINEUP:
			gdVwTop--;
			if( 0 > gdVwTop )	gdVwTop = 0;
			break;

		case SB_PAGEUP:
			gdVwTop -= 3;
			if( 0 > gdVwTop )	gdVwTop = 0;
			break;

		case SB_LINEDOWN:
			gdVwTop++;
			if( stScrollInfo.nMax <=  gdVwTop ){	gdVwTop = stScrollInfo.nMax;	}
			break;

		case SB_PAGEDOWN:
			gdVwTop += 3;
			if( stScrollInfo.nMax <=  gdVwTop ){	gdVwTop = stScrollInfo.nMax;	}
			break;

		case SB_BOTTOM:
			gdVwTop = stScrollInfo.nMax;
			break;

		case SB_THUMBTRACK:
			gdVwTop = stScrollInfo.nTrackPos;
			break;

		default:	return;
	}
	//	変化ないならなにもせんでいい
	if( maePos == gdVwTop  )	return;

	InvalidateRect( ghDraughtWnd, NULL, TRUE );
	UpdateWindow( ghDraughtWnd );	//	リアルタイム描画に必要

	stScrollInfo.fMask = SIF_POS;
	stScrollInfo.nPos  = gdVwTop;
	SetScrollInfo( ghScrBarWnd, SB_CTL, &stScrollInfo, TRUE );

	return;
}
//-------------------------------------------------------------------------------------------------

#ifndef _ORRVW

/*!
	編集の選択範囲からいただく
	@param[in]	bSqSel	矩形であるかどうか
*/
UINT DraughtItemAddFromSelect( UINT bSqSel )
{
	LPTSTR	ptString = NULL;
	UINT	cchSize, cbSize;
	LPPOINT	pstPos = NULL;

	UINT_PTR	i, j, iTexts;
	LONG	dMin = 0;
	INT		insDot, yLine, iLines = 0, dOffset;
	LPTSTR	ptSpace = NULL;

	LPSTR	pcArts;
	wstring	wsString;


	//	内部処理なのでUnicode固定
	cbSize = DocSelectTextGetAlloc( D_UNI | bSqSel, (LPVOID *)(&ptString), (bSqSel & D_SQUARE) ? &pstPos : NULL );

	StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSize );

	if( 0 >= cchSize )	return 0;	//	文字列ないならなにもしない

	//	オフセット設定が有る場合、その分を埋める空白が必要
	if( pstPos )	//	最小オフセット値を探して、そこを左端にする
	{
		dMin = pstPos[0].x;

		yLine = 0;
		for( i = 0; cchSize > i; i++ )
		{
			if( CC_CR == ptString[i] && CC_LF == ptString[i+1] )	//	改行であったら
			{
				//	オフセット最小をさがす
				if( dMin > pstPos[yLine].x ){	dMin =  pstPos[yLine].x;	}

				i++;		//	0x0D,0x0Aだから、壱文字飛ばすのがポイント
				yLine++;	//	改行したからFocusは次の行へ
			}
		}
		//	この時点で、yLineは行数になってる
		iLines = yLine;

		//	壱行目の空白を作って閃光入力しておく
		insDot = 0;
		dOffset = pstPos[0].x - dMin;
		ptSpace = DocPaddingSpaceUni( dOffset, NULL, NULL, NULL );
		//	前方空白は無視されるのでユニコード使って問題無い
		StringCchLength( ptSpace, STRSAFE_MAX_CCH, &iTexts );
		for( j = 0; iTexts > j; j++ ){	wsString +=  ptSpace[j];	}
		FREE(ptSpace);
	}

	yLine = 0;	insDot = 0;
	for( i = 0; cchSize > i; i++ )
	{
		if( CC_CR == ptString[i] && CC_LF == ptString[i+1] )	//	改行であったら
		{
			wsString +=  wstring( TEXT("\r\n") );

			i++;		//	0x0D,0x0Aだから、壱文字飛ばすのがポイント
			yLine++;	//	改行したからFocusは次の行へ

			//	オフセット分の空白を作る
			if( pstPos && (iLines > yLine) )
			{
				dOffset = pstPos[yLine].x - dMin;
				ptSpace = DocPaddingSpaceUni( dOffset, NULL, NULL, NULL );
				//	前方空白は無視されるのでユニコード使って問題無い
				StringCchLength( ptSpace, STRSAFE_MAX_CCH, &iTexts );
				for( j = 0; iTexts > j; j++ ){	wsString +=  ptSpace[j];	}
				FREE(ptSpace);
			}
		}
		else if( CC_TAB ==  ptString[i] ){	/*	タブは挿入しない	*/	}
		else{	wsString += ptString[i];	}
	}

	FREE(ptString);
	FREE(pstPos);

	pcArts =  SjisEncodeAlloc( wsString.c_str() );

	DraughtItemAdding( pcArts );

	FREE(pcArts);

	return yLine;
}
//-------------------------------------------------------------------------------------------------
#endif

/*!
	AAテキストを確保して取り込む
	@param[in]	pcArts	ＡＡテキスト
	@return		追加後のアイテム総数
*/
UINT DraughtItemAdding( LPSTR pcArts )
{
	UINT_PTR	cbSize;
	AAMATRIX	stItem;

	INT_PTR	iItems;


	StringCchLengthA( pcArts, STRSAFE_MAX_CCH, &cbSize );

	stItem.pcItem = (LPSTR)malloc( (cbSize + 1) );
	ZeroMemory( stItem.pcItem, (cbSize + 1) );
	StringCchCopyA( stItem.pcItem, (cbSize + 1), pcArts );


	DraughtAaImageing( &stItem );


	gvcDrtItems.push_back( stItem );

	do	//	はみだしてたら？
	{
		iItems = gvcDrtItems.size( );
		if( (TPNL_HORIZ * TPNL_VERTI) < iItems ){	DraughtItemDelete(  0 );	}

	}while( (TPNL_HORIZ * TPNL_VERTI) < iItems );

	return iItems;
}
//-------------------------------------------------------------------------------------------------

/*!
	ＡＡのサムネ用イメージを作る
*/
UINT DraughtAaImageing( LPAAMATRIX pstItem )
{
	UINT_PTR	cchSize;
	LPTSTR		ptTextaa;
	INT		iRslt, iYdot, iXdot, iLine;
	HDC		hdc, hMemDC;
	HBITMAP	hOldBmp;
	RECT	rect;



	ptTextaa = SjisDecodeAlloc( pstItem->pcItem );
	StringCchLength( ptTextaa, STRSAFE_MAX_CCH, &cchSize );

	//	元ＡＡのサイズ確定が必要
	iXdot = TextViewSizeGet( ptTextaa, &iLine );
	iYdot = iLine * LINE_HEIGHT;

	pstItem->iMaxDot = iXdot;
	pstItem->iLines  = iLine;

	if( THM_WIDTH >  iXdot )	iXdot = THM_WIDTH;
	if( THM_HEIGHT > iYdot )	iYdot = THM_HEIGHT;

	pstItem->stSize.cx = iXdot;
	pstItem->stSize.cy = iYdot;

	SetRect( &rect, 0, 0, iXdot, iYdot );
	//	あんまり大きいなら、左上限定とか？

	//	描画用ビットマップ作成
	hdc = GetDC( ghDraughtWnd );

	//	サムネ用BMP・これはフルサイズ	//	pstItem->hThumbDC	pstItem->hOldBmp
	hMemDC = CreateCompatibleDC( hdc );
	pstItem->hThumbBmp = CreateCompatibleBitmap( hdc, rect.right, rect.bottom );
	hOldBmp = SelectBitmap( hMemDC, pstItem->hThumbBmp );
	SelectFont( hMemDC, ghAaFont );

	ReleaseDC( ghDraughtWnd, hdc );

	iRslt = PatBlt( hMemDC, 0, 0, rect.right, rect.bottom, WHITENESS );

	iRslt = DrawText( hMemDC, ptTextaa, cchSize, &rect, DT_LEFT | DT_NOPREFIX | DT_NOCLIP | DT_WORDBREAK );

	SelectFont( hMemDC, GetStockFont(DEFAULT_GUI_FONT) );
	SelectBitmap( hMemDC, hOldBmp );
	DeleteDC( hMemDC );

	FREE( ptTextaa );

	return 0;
}
//-------------------------------------------------------------------------------------------------

/*!
	Targetアイテムを使う・クルップボードへ・他に使いたいときは？
*/
HRESULT DraughtItemUse( INT id )
{
	LPSTR		pcAaItem;
	INT_PTR		iItems, i, iOffset, iTarget;
	UINT_PTR	cbSize;
	UINT		dMode;
	MAAM_ITR	itItem;

#ifdef THUMBNAIL_STYLE
	if( gbThumb )	//	サムネモード
	{
		iOffset = gdVwTop * TPNL_HORIZ;
		iTarget = iOffset + giTarget;

		pcAaItem = AacAsciiArtGet( iTarget );
		if( !(pcAaItem) )	return E_OUTOFMEMORY;

		switch( id )
		{
			case IDM_DRAUGHT_INSERTEDIT:	dMode = MAA_INSERT;		break;
			case IDM_DRAUGHT_INTERRUPTEDIT:	dMode = MAA_INTERRUPT;	break;
			case IDM_DRAUGHT_LAYERBOX:		dMode = MAA_LAYERED;	break;
			default:
			case IDM_DRAUGHT_UNICLIP:		dMode = MAA_UNICLIP;	break;
			case IDM_DRAUGHT_SJISCLIP:		dMode = MAA_SJISCLIP;	break;
		}
		StringCchLengthA( pcAaItem, STRSAFE_MAX_CCH, &cbSize );

		ViewMaaMaterialise( pcAaItem, cbSize, dMode );
		FREE(pcAaItem);
	}
	else
	{
#endif
		iItems = gvcDrtItems.size( );	//	現在個数
		if( 0 >= iItems )	return E_OUTOFMEMORY;

		for( i = 0, itItem = gvcDrtItems.begin(); gvcDrtItems.end() != itItem; i++, itItem++ )
		{
			if( giTarget == i )	//	ヒット
			{
				switch( id )
				{
					case IDM_DRAUGHT_INSERTEDIT:	dMode = MAA_INSERT;		break;
					case IDM_DRAUGHT_INTERRUPTEDIT:	dMode = MAA_INTERRUPT;	break;
					case IDM_DRAUGHT_LAYERBOX:		dMode = MAA_LAYERED;	break;
					default:
					case IDM_DRAUGHT_UNICLIP:		dMode = MAA_UNICLIP;	break;
					case IDM_DRAUGHT_SJISCLIP:		dMode = MAA_SJISCLIP;	break;
				}
				StringCchLengthA( itItem->pcItem, STRSAFE_MAX_CCH, &cbSize );

				ViewMaaMaterialise( itItem->pcItem, cbSize, dMode );
			}
		}
#ifdef THUMBNAIL_STYLE
	}
#endif
	return E_INVALIDARG;
}
//-------------------------------------------------------------------------------------------------

/*!
	対象アイテム削除・REDRAWの面倒は別で見る
	@param[in]	iTarget	対象の通し番号・マイナスなら全削除
	@return	非０削除した　０削除できんかった
*/
INT DraughtItemDelete( CONST INT iTarget )
{
	INT_PTR	iItems, i;
	INT		delCnt;
	MAAM_ITR	itItem;

	iItems = gvcDrtItems.size( );	//	現在個数
	if( 0 >= iItems )	return 0;

	delCnt = 0;
	for( i = 0, itItem = gvcDrtItems.begin(); gvcDrtItems.end() != itItem; i++, itItem++ )
	{
		if( iTarget == i || 0 > iTarget )
		{
			FREE( itItem->pcItem );
		//	SelectBitmap( itItem->hThumbDC, itItem->hOldBmp );
			DeleteBitmap( itItem->hThumbBmp );
		//	DeleteDC( itItem->hThumbDC );
			delCnt++;

			if( iTarget == i )
			{
				gvcDrtItems.erase( itItem );
				return 1;
			}
		}
	}

	if( 0 > iTarget ){	gvcDrtItems.clear( );	}

	return delCnt;
}
//-------------------------------------------------------------------------------------------------

/*!
	クライヤント座標を受け取って、ターゲットアイテム通し番号を出す
	@param[in]	ptPos	クライヤント座標が入った構造体ポインター
	@return		左上から、→に通し番号０インデックス　無効で－１
*/
INT DraughtTargetItemSet( LPPOINT pstPos )
{
	INT	ix, iy, number;

	ix = pstPos->x / THM_WIDTH;
	iy = pstPos->y / THM_HEIGHT;

	if( 0 > ix || TPNL_HORIZ <= ix || 0 > iy || TPNL_VERTI <= iy )	return -1;

	number = ix + iy * TPNL_HORIZ;

	TRACE( TEXT("TARGET %d"), number );

	return number;
}
//-------------------------------------------------------------------------------------------------

/*!
	縦横を受け取って、アスペクト比を維持してサムネサイズに縮小する
	@param[in,out]	pstSize	受け取って、戻す
	@param[in]		bOrgRem	非０サムネサイズより小さいならオリジナルサイズ
	@return	DOUBLE	ZOOM率
*/
DOUBLE DraughtAspectKeeping( LPSIZE pstSize, UINT bOrgRem )
{
	INT	iOrgWid, iOrgHei;
	INT	iZomWid, iZomHei;
	DOUBLE	ddPercent, ddBuff;

	iOrgWid = pstSize->cx;
	iOrgHei = pstSize->cy;
	ddPercent = 1.0;

	if( iOrgWid >= iOrgHei )	//	正方形か横長
	{
		iZomWid = THM_WIDTH;

		if( THM_WIDTH == iOrgWid )	//	ピタリなら何もすることない
		{
			iZomHei = iOrgHei;
		}
		else
		{
			ddPercent = (DOUBLE)THM_WIDTH / (DOUBLE)iOrgWid;
			if( bOrgRem && THM_WIDTH > iOrgWid )	//	サイズ以下ならオリジナルサイズでよろし
			{
				iZomWid = iOrgWid;
				iZomHei = iOrgHei;
			}
			else
			{
				ddBuff  = (DOUBLE)iOrgHei * ddPercent;
				ddBuff += 0.5;	//	四捨五入
				iZomHei = ddBuff;
			}
		}
	}
	else	//	縦長
	{
		iZomHei = THM_HEIGHT;

		if( THM_HEIGHT == iOrgHei )	//	ピタリなら何もすることない
		{
			iZomWid = iOrgWid;
		}
		else
		{
			ddPercent = (DOUBLE)THM_HEIGHT / (DOUBLE)iOrgHei;
			if( bOrgRem && THM_HEIGHT > iOrgHei )	//	サイズ以下ならオリジナルサイズでよろし
			{
				iZomWid = iOrgWid;
				iZomHei = iOrgHei;
			}
			else
			{
				ddBuff  = (DOUBLE)iOrgWid * ddPercent;
				ddBuff += 0.5;	//	四捨五入
				iZomWid = ddBuff;
			}
		}
	}

	pstSize->cx = iZomWid;
	pstSize->cy = iZomHei;

	return ddPercent;
}
//-------------------------------------------------------------------------------------------------


/*!
	文字列をうけとって、行数と最大ドット幅を計算
	@param[in]	ptText	チェキりたいユニコード文字列受け取る
	@param[out]	piLine	行数返す
	@return		最大ドット数
*/
INT TextViewSizeGet( LPCTSTR ptText, PINT piLine )
{
	UINT_PTR	cchSize, i;
	INT		xDot, yLine, dMaxDot;

	wstring	wString;

	StringCchLength( ptText, STRSAFE_MAX_CCH, &cchSize );

	yLine = 1;	dMaxDot = 0;
	for( i = 0; cchSize > i; i++ )
	{
		if( CC_CR == ptText[i] && CC_LF == ptText[i+1] )	//	改行であったら
		{
			//	ドット数確認
			xDot = ViewStringWidthGet( wString.c_str() );
			if( dMaxDot < xDot )	dMaxDot = xDot;

			wString.clear( );
			i++;		//	0x0D,0x0Aだから、壱文字飛ばすのがポイント
			yLine++;	//	改行したから行数数える
		}
		else if( CC_TAB == ptText[i] )
		{
			//	タブは無かったことにする
		}
		else
		{
			wString += ptText[i];
		}
	}

	if( 1 <= wString.size() )	//	最終行確認
	{
		//	ドット数確認
		xDot = ViewStringWidthGet( wString.c_str() );
		if( dMaxDot < xDot )	dMaxDot = xDot;
	}

	if( piLine )	*piLine = yLine;	//	空行だったとしても１行はある
	return dMaxDot;
}
//-------------------------------------------------------------------------------------------------

#endif
