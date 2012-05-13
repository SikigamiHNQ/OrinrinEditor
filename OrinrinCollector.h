/*! @file
	@brief アプリ全体で使う定数や函数です
	このファイルは OrinrinCollector.h です。
	@author	SikigamiHNQ
	@date	2011/10/06
*/

/*
Orinrin Collector : Clipboard Auto Stocker for Japanese Only
Copyright (C) 2011 - 2012 Orinrin/SikigamiHNQ

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.
If not, see <http://www.gnu.org/licenses/>.
*/
//-------------------------------------------------------------------------------------------------

#pragma once

#define STRICT

#include "resource.h"
//-------------------------------------------------------------------------------------------------

// このコード モジュールに含まれる関数の宣言
ATOM	InitWndwClass( HINSTANCE );
BOOL	InitInstance( HINSTANCE, INT );
LRESULT	CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

BOOLEAN	Cls_OnCreate( HWND, LPCREATESTRUCT );		//!<	本体の WM_CREATE の処理・固定Editとかつくる
VOID	Cls_OnCommand( HWND , INT, HWND, UINT );	//!<	本体の WM_COMMAND の処理
VOID	Cls_OnPaint( HWND );						//!<	本体の WM_PAINT の処理・枠線描画とか
VOID	Cls_OnDestroy( HWND );						//!<	本体の WM_DESTROY の処理・BRUSHとかのオブジェクトの破壊を忘れないように
HBRUSH	Cls_OnCtlColor( HWND , HDC, HWND, INT );	//!<	
VOID	Cls_OnHotKey( HWND, INT, UINT, UINT );		//!<	
VOID	Cls_OnDrawClipboard( HWND );				//!<	
VOID	Cls_OnChangeCBChain( HWND, HWND, HWND );	//!<	

VOID		WndTagSet( HWND, LONG_PTR );			//!<	
LONG_PTR	WndTagGet( HWND );						//!<	

BOOLEAN	SelectFileDlg( HWND, LPTSTR, UINT_PTR );	//!<	

HRESULT	ToolTipSetting( HWND, UINT, LPTSTR );		//!<	

HRESULT	TasktrayIconAdd( HWND );					//!<	
VOID	TaskTrayIconEvent( HWND, UINT, UINT );		//!<	
HRESULT	TaskTrayIconCaptionChange( HWND );			//!<	
HRESULT	TaskTrayIconBalloon( HWND, LPTSTR, LPTSTR, DWORD );	//!<	

INT		InitParamValue( UINT, UINT, INT );			//!<	
HRESULT	InitClipStealOpen( UINT, UINT, LPTSTR );	//!<	
HRESULT	InitSettingSave( HWND, UINT );				//!<	

HRESULT	FileListViewInit( HWND );				//!<	
INT		FileListViewAdd( HWND, LPTSTR );		//!<	
HRESULT	FileListViewGet( HWND, INT, LPTSTR );	//!<	
INT		FileListViewDelete( HWND  );			//!<	

BOOLEAN	FileTypeCheck( LPTSTR );			//!<	

HMENU	CreateFileSelMenu( HWND, UINT );	//!<	

HRESULT	ClipStealDoing( HWND  );		//!<	
LPTSTR	ClipboardDataGet( LPVOID  );	//!<	

UINT	RegHotModExchange( UINT , BOOLEAN );	//!<	

LPTSTR	SjisDecodeAlloc( LPSTR );		//!<	
LPSTR	SjisEncodeAlloc( LPCTSTR );		//!<	
LPSTR	SjisEntityExchange( LPCSTR );	//!<	
TCHAR	UniRefCheck( LPSTR );			//!<	
