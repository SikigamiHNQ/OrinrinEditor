/*! @file
	@brief トレスモードのとき、２ペインの管理します。
	このファイルは ViewTrace.cpp です。
	@author	SikigamiHNQ
	@date	2011/11/29
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

//こっちは閲覧用・背景絵を表示するのはどっちでもいけるようにするのがいいかも

#include "stdafx.h"
#include "OrinrinEditor.h"
//-------------------------------------------------------------------------------------------------

#define VIEW_TRACE_CLASS	TEXT("TRACE_VIEW")
//-------------------------------------------------------------------------------------------------

static HINSTANCE	ghInst;		//!<	現在のインターフェイス

extern  HWND	ghPrntWnd;		//!<	メインウインドウハンドル
extern  HWND	ghViewWnd;		//!<	編集ビューハンドル

static  HWND	ghVwTrcWnd;		//!<	このウインドウ・トレスビューハンドル

//	画面サイズを確認して、移動によるスクロールの面倒みる
extern INT		gdHideXdot;		//!<	左の隠れ部分
extern INT		gdViewTopLine;	//!<	表示中の最上部行番号
extern SIZE		gstViewArea;	//!<	表示領域のドットサイズ・ルーラー等の領域は無し
extern INT		gdDispingLine;	//!<	見えてる行数・中途半端に見えてる末端は含まない

//	フォントは描画毎にデバイスコンテキストに割り付ける必要がある
extern HFONT	ghAaFont;		//!<	AA用フォント
//-------------------------------------------------------------------------------------------------


