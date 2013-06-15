/*! @file
	@brief スプリットバーで使う定数や函数です。
	このファイルは SplitBar.h です。
	@author	SikigamiHNQ
	@date	2011/00/00
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

#pragma once

#define STRICT
//-------------------------------------------------------------------------------------------------

#define SPLITBAR_CLASS		TEXT("CSplitBar")	//	スプリットバークラス名
#define SPLITBAR_WIDTH		4	//	スプリットバーの幅
#define SPLITBAR_LEFTLIMIT	120	//	スプリットバーの左位置リミット

#define SIZE_SPLITBAR_MOVED	0xFFFF	//	スプリットバーが動かされた
//-------------------------------------------------------------------------------------------------

ATOM	SplitBarClass( HINSTANCE );
HWND	SplitBarCreate( HINSTANCE, HWND, INT, INT, INT );
VOID	SplitBarPosGet( HWND, LPRECT );
LONG	SplitBarResize( HWND, LPRECT );
