/*! @file
	@brief スプリットバーの処理します
	このファイルは SplitBar.cpp です。
	@author	SikigamiHNQ
	@date	2011/04/04
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

//	複数使うとき・バーウインドウにTAGしておく

#include "stdafx.h"
#include "SplitBar.h"
//-------------------------------------------------------------------------------------------------

LRESULT	CALLBACK SplitProc( HWND, UINT, WPARAM, LPARAM );

VOID	Spt_OnPaint( HWND );
VOID	Spt_OnLButtonDown( HWND, BOOL, INT, INT, UINT );
VOID	Spt_OnMouseMove( HWND, INT, INT, UINT );
VOID	Spt_OnLButtonUp( HWND, INT, INT, UINT );
//-------------------------------------------------------------------------------------------------


/*!
	スプリットバーのクラスを作成
	@param[in]	hInst	このモジュールのインスタンスハンドル
	@return		登録したクラスアトム
*/
ATOM SplitBarClass( HINSTANCE hInst )
{
	WNDCLASSEX	wcex;

	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= SplitProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInst;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor( NULL, IDC_SIZEWE );
	wcex.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= SPLITBAR_CLASS;
	wcex.hIconSm		= NULL;

	return RegisterClassEx( &wcex );
}
//-------------------------------------------------------------------------------------------------

/*!
	スプリットバーを作る
	@param[in]	hInst	このモジュールのインスタンスハンドル
	@param[in]	hPrWnd	親ウインドウハンドル
	@param[in]	x		クライヤント的な左座標
	@param[in]	y		クライヤント的な上座標
	@param[in]	dHeight	高さ
	@return		スプリットバーのハンドル
*/
HWND SplitBarCreate( HINSTANCE hInst, HWND hPrWnd, INT x, INT y, INT dHeight )
{
	HWND	hWorkWnd;

	hWorkWnd = CreateWindowEx( WS_EX_WINDOWEDGE, SPLITBAR_CLASS, TEXT("SplitBar"),
		WS_CHILD | WS_VISIBLE, x, y, SPLITBAR_WIDTH, dHeight,
		hPrWnd, NULL, hInst, NULL );
	SetWindowLongPtr( hWorkWnd, GWLP_USERDATA, 0 );

	return hWorkWnd;
}
//-------------------------------------------------------------------------------------------------

/*!
	スプリットバーのウインドウプロシージャ
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	message		ウインドウメッセージの識別番号
	@param[in]	wParam		追加の情報１
	@param[in]	lParam		追加の情報２
	@retval 0	メッセージ処理済み
	@retval no0	ここでは処理せず次に回す
*/
LRESULT CALLBACK SplitProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_PAINT,       Spt_OnPaint );		//	画面の更新とか
		HANDLE_MSG( hWnd, WM_LBUTTONDOWN, Spt_OnLButtonDown );	
		HANDLE_MSG( hWnd, WM_MOUSEMOVE,   Spt_OnMouseMove );	
		HANDLE_MSG( hWnd, WM_LBUTTONUP,   Spt_OnLButtonUp );	

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
VOID Spt_OnPaint( HWND hWnd )
{
	PAINTSTRUCT	ps;
	HDC			hdc;

	hdc = BeginPaint( hWnd, &ps );

	EndPaint( hWnd, &ps );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	マウスの左ボタンがダウン(ダブルクルッコ）したときの処理
	@param[in]	hWnd			親ウインドウハンドル
	@param[in]	fDoubleClick	ダブルクルッコされたときなら非０となる
	@param[in]	x				クライアント座標Ｘ
	@param[in]	y				クライアント座標Ｙ
	@param[in]	keyFlags		押されてる他のボタン
*/
VOID Spt_OnLButtonDown( HWND hWnd, BOOL fDoubleClick, INT x, INT y, UINT keyFlags )
{
	if( fDoubleClick )	return;	//	ダブルクルックは何もしない

	//	スプリットバーがクリックされたらサイズ変更モードになる
	SetWindowLongPtr( hWnd, GWLP_USERDATA, 1 );

	//	マウスの動きを監視する
	SetCapture( hWnd );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	マウスが動いたときの処理
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	x			スプリットバー左からの相対座標Ｘ
	@param[in]	y			スプリットバー上からの相対座標Ｙ
	@param[in]	keyFlags	押されてる他のボタン
*/
VOID Spt_OnMouseMove( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	HWND	hPrWnd;
	RECT	stRect;
//	POINT	stPoint;
	LONG	wWidth = 0;
	LONG	wLeft = 0;
	LONG	bSpliting;

	bSpliting = GetWindowLongPtr( hWnd, GWLP_USERDATA );

	//	サイズ変更モードなら、ずりずり動かす
	if( bSpliting )
	{
		hPrWnd = GetParent( hWnd );
		GetClientRect( hPrWnd, &stRect );	//	親ウインドウ
		wWidth = stRect.right;	//	幅確保

		SplitBarPosGet( hWnd, &stRect );	//	スプリットバーの左上と幅高さ
		wLeft = stRect.left + x;

		if( wLeft < SPLITBAR_LEFTLIMIT){	wLeft =  SPLITBAR_LEFTLIMIT;	}
		else if( wLeft >  wWidth - SPLITBAR_LEFTLIMIT ){	wLeft = wWidth - SPLITBAR_LEFTLIMIT;	}

		SetWindowPos( hWnd, HWND_TOP, wLeft, stRect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	マウスボタンが離された
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	x			クライアント座標Ｘ
	@param[in]	y			クライアント座標Ｙ
	@param[in]	keyFlags	押されてる他のボタン
*/
VOID Spt_OnLButtonUp( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	HWND	hPrWnd;
	RECT	stRect;
	//LONG	wWidth = 0;
	//LONG	wHeight = 0;
	LONG	bSpliting;

	bSpliting = GetWindowLongPtr( hWnd, GWLP_USERDATA );

	//	サイズ変更モード終了
	if( bSpliting )
	{
		hPrWnd = GetParent( hWnd );

		ReleaseCapture(   );	//	マウスキャプチャ終了
		SetWindowPos( hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
		GetClientRect( hPrWnd, &stRect );
		//wHeight = stRect.bottom - stRect.top;
		//wWidth  = stRect.right  - stRect.left;

		//	サイズ変更発生を親ウインドウに送信
#ifdef SPLIT_BAR_POS_FIX
		FORWARD_WM_SIZE( hPrWnd, SIZE_SPLITBAR_MOVED, x, y, PostMessage );
#else
		FORWARD_WM_SIZE( hPrWnd, SIZE_RESTORED, x, y, PostMessage );
#endif
	}

	SetWindowLongPtr( hWnd, GWLP_USERDATA, 0 );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	スプリットバーのクライアント上の左上座標と、幅と高さを確保する
	@param[in]	hSplitWnd	対象のスプリットバーハンドル
	@param[out]	ptRect		スプリットバーのサイズを入れる構造体へのポインター
*/
VOID SplitBarPosGet( HWND hSplitWnd, LPRECT ptRect )
{
	HWND	hPrWnd = GetParent( hSplitWnd );
	RECT	clRect;
	POINT	point;
	LONG	wHeight;

	GetWindowRect( hSplitWnd, &clRect );
	wHeight = clRect.bottom - clRect.top;
	point.x = clRect.left;
	point.y = clRect.top;

	ScreenToClient( hPrWnd, &point );

	SetRect( ptRect, point.x, point.y, SPLITBAR_WIDTH, wHeight );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	画面サイズが変わったのでサイズ変更
	@param[in]	hSplitWnd	対象のスプリットバーハンドル
	@param[in]	ptRect		クライアント領域
	@return		画面左からの位置
*/
LONG SplitBarResize( HWND hSplitWnd, LPRECT ptRect )
{
	RECT	rect;

	SplitBarPosGet( hSplitWnd, &rect );	//	左からの位置が変わらない

	SetWindowPos( hSplitWnd, HWND_TOP, rect.left, ptRect->top, SPLITBAR_WIDTH, ptRect->bottom, 0 );

	InvalidateRect( hSplitWnd, NULL, TRUE );
	UpdateWindow( hSplitWnd );

	return rect.left;
}
//-------------------------------------------------------------------------------------------------

