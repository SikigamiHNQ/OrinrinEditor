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

#define DRAUGHT_BOARD_CLASS	TEXT("DRAUGHT_BOARD")

//	一枚のパネルサイズ・小さいか？
#define THM_WIDTH	128
#define THM_HEIGHT	128

//	パネルは５ｘ３に並べる
#define TPNL_HORIZ	5
#define TPNL_VERTI	3
//-------------------------------------------------------------------------------------------------

#ifdef DRAUGHT_STYLE

typedef struct tagDRAUGHTITEM
{
	LPSTR	pcAaText;	//!<	AA文字列・Sjisで確保
	INT		iMaxDot;	//!<	横幅最大ドット数
	INT		iLines;		//!<	使用行数

	SIZE	stSize;		//!<	ピクセルサイズ
	HDC		hThumbDC;	//!<	サムネイル用デバイスコンテキスト
	HBITMAP	hThumbBmp;	//!<	サムネイル用ビットマップハンドル
	HBITMAP	hOldBmp;	//!<	復帰用

} DRAUGHTITEM, *LPDRAUGHTITEM;
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

static vector<DRAUGHTITEM>	gvcDrtItems;	//!<	
typedef vector<DRAUGHTITEM>::iterator	DRTI_ITR;	//!<	
//-------------------------------------------------------------------------------------------------

INT		DraughtTargetItemSet( LPPOINT );	//!<	
DOUBLE	DraughtAspectKeeping( LPSIZE, UINT );	//!<	
INT		DraughtItemDelete( CONST INT  );	//!<	
HRESULT	DraughtItemUse( INT );	//!<	

LRESULT CALLBACK DraughtProc( HWND, UINT, WPARAM, LPARAM );
VOID	Drt_OnCommand( HWND , INT, HWND, UINT );	//!<	
VOID	Drt_OnPaint( HWND );						//!<	
//VOID	Drt_OnSize( HWND , UINT, INT, INT );		//!<	
LRESULT	Drt_OnNotify( HWND , INT, LPNMHDR );		//!<	
VOID	Drt_OnLButtonUp( HWND, INT, INT, UINT );	//!<	
VOID	Drt_OnContextMenu( HWND, HWND, UINT, UINT );//!<	
VOID	Drt_OnDestroy( HWND );						//!<	
VOID	Drt_OnKillFocus( HWND, HWND );				//!<	

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

		//	クルック動作指定ロード・デフォ動作はユニコクリップ
		gdClickMode = InitParamValue( INIT_LOAD, VL_DRAUGHT_MODE, 3 );
		//	Viewerの場合はコピーモードに従う
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
*/
HWND DraughtWindowCreate( HINSTANCE hInstance, HWND hPtWnd )
{
	INT_PTR	iItems;
	INT		iRslt;
	HDC		hdc;

	RECT	wdRect, rect;

	if( !(hPtWnd ) )	//	破壊する・いらない？
	{


		return NULL;
	}

	//	已に出来ていたらそのまま返す
	if( ghDraughtWnd  ){	UpdateWindow( ghDraughtWnd );	return ghDraughtWnd;	}

	iItems = gvcDrtItems.size( );	//	現在個数

	GetWindowRect( hPtWnd, &wdRect );
	rect.left   = wdRect.left + 32;	//	オフセット値に特に意味はない
	rect.top    = wdRect.top  + 32;
	rect.right  = THM_WIDTH  * TPNL_HORIZ;
	rect.bottom = THM_HEIGHT * TPNL_VERTI;
	
	//	ウインドウ作成
	ghDraughtWnd = CreateWindowEx( WS_EX_CLIENTEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
		DRAUGHT_BOARD_CLASS, TEXT("Draught Board"), WS_POPUP | WS_VISIBLE,
		rect.left, rect.top, rect.right, rect.bottom, NULL, NULL, hInstance, NULL );

	if( !(ghNonItemDC) )
	{
		//	描画用ビットマップ作成
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

//		case WM_MOUSEWHEEL:	SendMessage( , WM_MOUSEWHEEL, wParam, lParam );	return 0;

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

		case IDM_DRAUGHT_EXPORT:	NotifyBalloonExist( TEXT("未実装なのです・・・"), TEXT("あぅあぅ"), NIIF_ERROR );	break;

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
	HDC			hdc;
	UINT		x = 0, y = 0;
	INT_PTR		iItems;
	SIZE		stSize;

	DRTI_ITR	itItem;


	hdc = BeginPaint( hWnd, &ps );

	SetStretchBltMode( hdc, HALFTONE );

	iItems = gvcDrtItems.size( );

	itItem = gvcDrtItems.begin();
	for( y = 0; TPNL_VERTI > y; y++ )
	{
		for( x = 0; TPNL_HORIZ > x; x++ )
		{
			if( itItem != gvcDrtItems.end() )
			{
				stSize = itItem->stSize;
				DraughtAspectKeeping( &stSize, TRUE );

				StretchBlt( hdc, (x * THM_WIDTH), (y * THM_HEIGHT), stSize.cx, stSize.cy,	//	コピー先ＤＣ、左上ＸＹ、幅、高さ
					itItem->hThumbDC, 0, 0, itItem->stSize.cx, itItem->stSize.cy,	//	コピー元ＤＣ、左上ＸＹ、幅、高さ
					SRCCOPY );	//	ラスタオペレーションコード
					//	((x+1) * THM_WIDTH)	((y+1) * THM_HEIGHT)
				itItem++;
			}
			else
			{
				BitBlt( hdc, (x * THM_WIDTH), (y * THM_HEIGHT), THM_WIDTH, THM_HEIGHT, ghNonItemDC, 0, 0, SRCCOPY );
			}
		}
	}

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
		case  0:	id = IDM_DRAUGHT_INSERTEDIT;	break;
		case  1:	id = IDM_DRAUGHT_INTERRUPTEDIT;	break;
		case  2:	id = IDM_DRAUGHT_LAYERBOX;	break;
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

	ghDraughtWnd = 0;

	return;
}
//-------------------------------------------------------------------------------------------------

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

/*!
	AAテキストを確保して取り込む
	@param[in]	pcArts	ＡＡテキスト
	@return		追加後のアイテム総数
*/
UINT DraughtItemAdding( LPSTR pcArts )
{
	UINT_PTR	cbSize, cchSize;
	LPTSTR		ptTextaa;
	DRAUGHTITEM	stItem;

	RECT	rect;
	INT_PTR	iItems;
	INT		iRslt, iYdot, iXdot, iLine;
	HDC		hdc;

	StringCchLengthA( pcArts, STRSAFE_MAX_CCH, &cbSize );

	stItem.pcAaText = (LPSTR)malloc( (cbSize + 1) );
	ZeroMemory( stItem.pcAaText, (cbSize + 1) );
	StringCchCopyA( stItem.pcAaText, (cbSize + 1), pcArts );

	ptTextaa = SjisDecodeAlloc( pcArts );
	StringCchLength( ptTextaa, STRSAFE_MAX_CCH, &cchSize );

	//	元ＡＡのサイズ確定が必要
	iXdot = TextViewSizeGet( ptTextaa, &iLine );
	iYdot = iLine * LINE_HEIGHT;

	stItem.iMaxDot = iXdot;
	stItem.iLines  = iLine;

	if( THM_WIDTH >  iXdot )	iXdot = THM_WIDTH;
	if( THM_HEIGHT > iYdot )	iYdot = THM_HEIGHT;

	stItem.stSize.cx = iXdot;
	stItem.stSize.cy = iYdot;

	SetRect( &rect, 0, 0, iXdot, iYdot );
	//	あんまり大きいなら、左上限定とか？

	//	描画用ビットマップ作成
	hdc = GetDC( ghDraughtWnd );

	//	サムネ用BMP・これはフルサイズ
	stItem.hThumbDC  = CreateCompatibleDC( hdc );
	stItem.hThumbBmp = CreateCompatibleBitmap( hdc, rect.right, rect.bottom );
	stItem.hOldBmp   = SelectBitmap( stItem.hThumbDC, stItem.hThumbBmp );
	SelectFont( stItem.hThumbDC, ghAaFont );

	ReleaseDC( ghDraughtWnd, hdc );

	iRslt = PatBlt( stItem.hThumbDC, 0, 0, rect.right, rect.bottom, WHITENESS );

	iRslt = DrawText( stItem.hThumbDC, ptTextaa, cchSize, &rect, DT_LEFT | DT_NOPREFIX | DT_NOCLIP | DT_WORDBREAK );

	SelectFont( stItem.hThumbDC, GetStockFont(DEFAULT_GUI_FONT) );

	gvcDrtItems.push_back( stItem );

	do	//	はみだしてたら？
	{
		iItems = gvcDrtItems.size( );
		if( (TPNL_HORIZ * TPNL_VERTI) < iItems ){	DraughtItemDelete(  0 );	}

	}while( (TPNL_HORIZ * TPNL_VERTI) < iItems );

	FREE( ptTextaa );

	return iItems;
}
//-------------------------------------------------------------------------------------------------

/*!
	Targetアイテムを使う・クルップボードへ・他に使いたいときは？
*/
HRESULT DraughtItemUse( INT id )
{
	INT_PTR		iItems, i;
	UINT_PTR	cbSize;
	UINT		dMode;
	DRTI_ITR	itItem;

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
			StringCchLengthA( itItem->pcAaText, STRSAFE_MAX_CCH, &cbSize );

			ViewMaaMaterialise( itItem->pcAaText, cbSize, dMode );
		}
	}

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
	DRTI_ITR	itItem;

	iItems = gvcDrtItems.size( );	//	現在個数
	if( 0 >= iItems )	return 0;

	delCnt = 0;
	for( i = 0, itItem = gvcDrtItems.begin(); gvcDrtItems.end() != itItem; i++, itItem++ )
	{
		if( iTarget == i || 0 > iTarget )
		{
			FREE( itItem->pcAaText );
			SelectBitmap( itItem->hThumbDC, itItem->hOldBmp );
			DeleteBitmap( itItem->hThumbBmp );
			DeleteDC( itItem->hThumbDC );
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

#endif
