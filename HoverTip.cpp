/*! @file
	@brief 自作のツールチップの制御
	このファイルは HoverTip.cpp です。
	@author	SikigamiHNQ
	@date	2012/06/08
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


/*
ツールチップのサブクラスできないか？

ツールチップビューを作るには
基本的にマウスムーブで発生するはず
制御用のハンドルつくるか？
TrackMouseEvent WM_MOUSEHOVER WM_MOUSELEAVE をつかう
WM_MOUSEHOVERが来たら、リストビューとかのハンドル渡してツールチップポッパップをコールする。
コールされたら、そのハンドルに対してDISPINFO的なmessageを送る
もしくはコールバック函数を指定するとか。
内容ゲットしたら、作成して表示。単体しかあり得ないので、
前のがあったら破壊するように。
非表示のままWM_MOUSELEAVEが来たらキャンセルか？

フローティングウインドウつくってそこに表示・ドラフトボードと同じように
表示枠の親はデスクトップか無しでいい。
マウスクルックで閉じる・３０秒経過で閉じる
数pixelのマウスムーブを検知して閉じる

ポップアップのサイズは、文字列で決める。縁取り２ｐｘくらいで。
背景と文字色は GetSysColor でとれる

SystemParametersInfo  SPI_GETMOUSEHOVERTIME  SPI_GETMOUSEHOVERWIDTH で調整出来る

*/

/*
出来るだけ内容は隠蔽する
使用側では、WM_MOUSEHOVER のキャッチする。発生ポイントの座標受け取る
内容の呼出は、コールバックするか？表示内容の文字列ポインタを受け取る
開放はこっちでやればいい？

チップ自体はスタティックでオーナードロー
*/
#ifdef USE_HOVERTIP



#define HOVER_TIPS_CLASS	TEXT("HOVER_TIPS")

#define HOVER_TIME	15000	//!<	デフォルト表示ｍｓ
#define HOVER_TMID	1234	//!<	タイマＩＤ
#define HOVER_MOVE	   8	//!<	これ以上マウスムーブが入ったら消す閾値

#define HOVER_DELAY	1000	//!<	マウス停止からの出現待ち時間
//-------------------------------------------------------------------------------------------------

static  ATOM	gTipAtom;	//!<	ウインドウクラスアトム
static  HWND	ghTipWnd;	//!<	ホバーチップのウインドウハンドル
static HFONT	ghTipFont;	//!<	ツールチップ用

static  UINT	gdMoveVol;	//!<	移動量カウント

static LPTSTR	gptContent;	//!<	表示内容
static RECT		gstContSize;//!<	表示大きさ
//-------------------------------------------------------------------------------------------------


VOID	HoverTipClose( HWND );	//!<	

LRESULT	CALLBACK HoverTipProc( HWND, UINT, WPARAM, LPARAM );	//!<	
VOID	Htp_OnPaint( HWND );						//!<	
VOID	htp_OnTimer( HWND, UINT );					//!<	
VOID	Htp_OnKillFocus( HWND, HWND );				//!<	
VOID	Htp_OnLButtonUp( HWND, INT, INT, UINT );	//!<	
VOID	Htp_OnMButtonUp( HWND, INT, INT, UINT );	//!<	
VOID	Htp_OnRButtonUp( HWND, INT, INT, UINT );	//!<	
VOID	Htp_OnMouseMove( HWND, INT, INT, UINT );	//!<	
//-------------------------------------------------------------------------------------------------

/*!
	初期化
	@param[in]	hInstance	アプリのインスタンス
	@param[in]	hPtWnd		メイン窓ウインドウハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT HoverTipInitialise( HINSTANCE hInstance, HWND hPtWnd )
{
	LOGFONT	stFont;
	WNDCLASSEX	wcex;
	INT	ttSize;

	if( hInstance )
	{
		gptContent = NULL;
		gdMoveVol = 0;

		//	表示チップウインドウクラス作成
		ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
		wcex.cbSize			= sizeof(WNDCLASSEX);
		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= HoverTipProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= hInstance;
		wcex.hIcon			= NULL;
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= (HBRUSH)(COLOR_INFOBK+1);	//	ツールチップコントロールの背景色
		wcex.lpszMenuName	= NULL;
		wcex.lpszClassName	= HOVER_TIPS_CLASS;
		wcex.hIconSm		= NULL;

		gTipAtom = RegisterClassEx( &wcex );
	
		//	表示チップウインドウ作成 | WS_EX_TOPMOST
		ghTipWnd = CreateWindowEx( WS_EX_TOOLWINDOW, HOVER_TIPS_CLASS, TEXT("InfoTip"), WS_POPUP | WS_BORDER, 0, 0, 15, 15, NULL, NULL, hInstance, NULL );
		//	最初は非表示

		//	表示フォント作成
		ViewingFontGet( &stFont );
		ttSize = InitParamValue( INIT_LOAD, VL_MAATIP_SIZE, FONTSZ_REDUCE );	//	サイズ確認
		stFont.lfHeight = (FONTSZ_REDUCE == ttSize) ? FONTSZ_REDUCE : FONTSZ_NORMAL;
		ghTipFont = CreateFontIndirect( &stFont );
	}
	else
	{
		DeleteFont( ghTipFont );

		DestroyWindow( ghTipWnd );

		FREE( gptContent );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	MouseHoverの登録する
	@param[in]	hTgtWnd	チェック対象のウインドウのハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT HoverTipResist( HWND hTgtWnd )
{
	//	Hover固定・LEAVEは使わないか

	TRACKMOUSEEVENT	stTrackMsEv;

	TRACE( TEXT("MOUSE HOVER RESIST") );

	ZeroMemory( &stTrackMsEv, sizeof(TRACKMOUSEEVENT) );
	stTrackMsEv.cbSize      = sizeof(TRACKMOUSEEVENT);
	stTrackMsEv.dwFlags     = TME_HOVER | TME_LEAVE;
	stTrackMsEv.hwndTrack   = hTgtWnd;
	stTrackMsEv.dwHoverTime = HOVER_DELAY;	//	時間、そのうち調整出来るように	HOVER_DEFAULT
	TrackMouseEvent( &stTrackMsEv );


	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ポッパップの文字サイズ変更
	@param[in]	ttSize	文字サイズ・１２か１６しかない
	@return		HRESULT	終了状態コード
*/
HRESULT HoverTipSizeChange( INT ttSize )
{
	LOGFONT	stFont;

	DeleteFont( ghTipFont );

	//	今使ってるヤツぶっ壊してから、新しいのつくってくっつける
	ViewingFontGet( &stFont );
	stFont.lfHeight = (FONTSZ_REDUCE == ttSize) ? FONTSZ_REDUCE : FONTSZ_NORMAL;
	ghTipFont = CreateFontIndirect( &stFont );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------



/*!
	WM_MOUSEHOVERを受け取る
	@param[in]	hEvWnd		発生したウインドウのハンドル
	@param[in]	wParam		追加情報１
	@param[in]	lParam		追加情報２
	@param[in]	pfInfoGet	表示内容をいただくコールバック函数
	@return	処理したら０
*/
LRESULT HoverTipOnMouseHover( HWND hEvWnd, WPARAM wParam, LPARAM lParam, HOVERTIPDISP pfInfoGet )
{
	INT		x, y;
	UINT	keyFlags;
	LPTSTR	ptText;

	HDC		hdc;
	HFONT	hOldFnt;

	RECT	deskRect;
	POINT	point;
	INT		xSub, ySub;

//	HOVERTIPINFO	stTipInfo;

	TRACE( TEXT("MOUSE HOVER RISING") );

	keyFlags = (UINT)wParam;
	//	Indicates whether various virtual keys are down.
	//	This parameter can be one or more of the following values.
	//	Value		Meaning
	//	MK_CONTROL	The CTRL key is depressed.
	//	MK_LBUTTON	The left mouse button is depressed.
	//	MK_MBUTTON	The middle mouse button is depressed.
	//	MK_RBUTTON	The right mouse button is depressed.
	//	MK_SHIFT	The SHIFT key is depressed.
	//	MK_XBUTTON1	The first X button is down.
	//	MK_XBUTTON2	The second X button is down.

	x = (INT)(SHORT)LOWORD(lParam);
	y = (INT)(SHORT)HIWORD(lParam);
	//	The low-order word specifies the x-coordinate of the cursor.
	//	The coordinate is relative to the upper-left corner of the client area.
	//	The high-order word specifies the y-coordinate of the cursor.
	//	The coordinate is relative to the upper-left corner of the client area.

	point.x = x;
	point.y = y;
	ClientToScreen( hEvWnd, &point );

//	ZeroMemory( &stTipInfo, sizeof(HOVERTIPINFO) );

	FREE( gptContent );

	ptText = pfInfoGet( NULL );
	if( !(ptText) )	//	なんかおかしい
	{
		return 0;
	}

	gptContent = ptText;

	hdc = GetDC( ghTipWnd );

	SetRect( &gstContSize, 0, 0, 2222, 100 );	//	サイズは適当・横幅は大きく余裕が必要

	hOldFnt = SelectFont( hdc , ghTipFont );	//	サイズ併せる必要がある

	//	必要な領域を確認
	DrawText( hdc, gptContent, -1, &gstContSize, DT_LEFT | DT_CALCRECT | DT_NOPREFIX );
	TRACE( TEXT("HOVER Size[ %d x %d, %d : %d"), gstContSize.left, gstContSize.top, gstContSize.right, gstContSize.bottom );

	SelectFont( hdc, hOldFnt );

	//	縁取り
	gstContSize.right  += 4;
	gstContSize.bottom += 4;

	//	デスクトップサイズ確保
	GetWindowRect( GetDesktopWindow(), &deskRect );
	//	画面よりデカいならカット
	if( gstContSize.right  >  deskRect.right  ){	gstContSize.right  = deskRect.right;	}
	if( gstContSize.bottom >  deskRect.bottom ){	gstContSize.bottom = deskRect.bottom;	}

	//	右下にはみ出すなら、表示出来るところまでオフセットする
	xSub = (point.x + gstContSize.right) - deskRect.right;
	if( 0 <  xSub ){	point.x -= xSub;	}
	ySub = (point.y + gstContSize.bottom) - deskRect.bottom;
	if( 0 <  ySub ){	point.y -= ySub;	}

	gdMoveVol = 0;
	//	位置合わせして表示
//	SetWindowPos( ghTipWnd, HWND_TOP, (point.x + 1), (point.y + 1), gstContSize.right, gstContSize.bottom, SWP_SHOWWINDOW );
	SetWindowPos( ghTipWnd, HWND_TOPMOST, (point.x + 1), (point.y + 1), gstContSize.right, gstContSize.bottom, SWP_SHOWWINDOW | SWP_NOACTIVATE );
	SetTimer( ghTipWnd , HOVER_TMID, HOVER_TIME, NULL );	//	消し用タイマ

	ReleaseDC( ghTipWnd, hdc );

	//	ドラフトボードと同じように、フローティングウインドウ作って、WM_PAINT で描画すればいい


	return 0;	//	If an application processes this message, it should return zero.
}
//-------------------------------------------------------------------------------------------------

/*!
	WM_MOUSELEAVEを受け取る
	@param[in]	hEvWnd		発生したウインドウのハンドル
	@return	処理したら０
*/
LRESULT HoverTipOnMouseLeave( HWND hEvWnd )
{
	TRACE( TEXT("MOUSE LEAVE RISING") );

	//	ここで、マウスカーソルがチップの上にくるとヤバイ
	//HoverTipClose( ghTipWnd );

	return 0;	//	If an application processes this message, it should return zero.
}
//-------------------------------------------------------------------------------------------------

/*!
	HoverTipを閉じる
	@param[in]	hWnd	ウインドウハンドル
*/
VOID HoverTipClose( HWND hWnd )
{
	KillTimer( hWnd, HOVER_TMID );
	ShowWindow( hWnd, SW_HIDE );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ウインドウプロシージャ
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	message	ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@retval 0	メッセージ処理済み
	@retval no0	ここでは処理せず次に回す
*/
LRESULT CALLBACK HoverTipProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_LBUTTONUP,   Htp_OnLButtonUp );	//	マウスクルック・纏めてもおｋ？
		HANDLE_MSG( hWnd, WM_MBUTTONUP,   Htp_OnMButtonUp );
		HANDLE_MSG( hWnd, WM_RBUTTONUP,   Htp_OnRButtonUp );
		HANDLE_MSG( hWnd, WM_MOUSEMOVE,   Htp_OnMouseMove );
		HANDLE_MSG( hWnd, WM_KILLFOCUS,   Htp_OnKillFocus );	//	フォーカスを失った
		HANDLE_MSG( hWnd, WM_PAINT,       Htp_OnPaint );
		HANDLE_MSG( hWnd, WM_TIMER,       htp_OnTimer );

		default:	break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------

/*!
	PAINT。無効領域が出来たときに発生。背景の扱いに注意。背景を塗りつぶしてから、オブジェクトを描画
	@param[in]	hWnd	親ウインドウのハンドル
	@return		無し
*/
VOID Htp_OnPaint( HWND hWnd )
{
	PAINTSTRUCT	ps;
	HDC			hdc;
	HFONT		hOldFnt;
	RECT	rect;

	hdc = BeginPaint( hWnd, &ps );

	hOldFnt = SelectFont( hdc, ghTipFont );
	SetTextColor( hdc, GetSysColor( COLOR_INFOTEXT ) );
	SetBkMode( hdc, TRANSPARENT );

	rect = gstContSize;
	rect.left += 2;
	rect.top  += 2;

	DrawText( hdc, gptContent, -1, &rect, DT_LEFT | DT_NOPREFIX );

	SelectFont( hdc, hOldFnt );

	EndPaint( hWnd, &ps );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	タイマイベント発生
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	id		タイマＩＤ
*/
VOID htp_OnTimer( HWND hWnd, UINT id )
{
	//	関係ない場合・先ずありえないハズだが
	if( HOVER_TMID != id )	return;

	HoverTipClose( hWnd );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	フォーカスを失った場合
	@param[in]	hWnd			ウインドウハンドル
	@param[in]	hwndNewFocus	フォーカスを得たウインドウのハンドル
*/
VOID Htp_OnKillFocus( HWND hWnd, HWND hwndNewFocus )
{
//	HoverTipClose( hWnd );	//	TOPMOSTなら要らない

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	マウスの左ボタンがうっｐされたとき
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	x			発生したＸ座標値
	@param[in]	y			発生したＹ座標値
	@param[in]	keyFlags	他に押されてるキーについて
*/
VOID Htp_OnLButtonUp( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	TRACE( TEXT("HTP LUP %d x %d"), x , y );	//	クライヤント座標

	HoverTipClose( hWnd );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	マウスの中ボタンがうっｐされたとき
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	x			発生したＸ座標値
	@param[in]	y			発生したＹ座標値
	@param[in]	keyFlags	他に押されてるキーについて
*/
VOID Htp_OnMButtonUp( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	TRACE( TEXT("HTP MUP %d x %d"), x , y );	//	クライヤント座標

	HoverTipClose( hWnd );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	マウスの右ボタンがうっｐされたとき
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	x			発生したＸ座標値
	@param[in]	y			発生したＹ座標値
	@param[in]	keyFlags	他に押されてるキーについて
*/
VOID Htp_OnRButtonUp( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	TRACE( TEXT("HTP RUP %d x %d"), x , y );	//	クライヤント座標

	HoverTipClose( hWnd );

	return;
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
VOID Htp_OnMouseMove( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	gdMoveVol++;
	if( HOVER_MOVE < gdMoveVol )
	{
		HoverTipClose( hWnd );
	}

	return;
}
//-------------------------------------------------------------------------------------------------

#endif
