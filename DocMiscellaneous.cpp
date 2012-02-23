/*! @file
	@brief その他色々な処理かもだ
	このファイルは DocMiscellaneous.cpp です。
	@author	SikigamiHNQ
	@date	2011/11/17
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

#include "stdafx.h"
#include "OrinrinEditor.h"
//-------------------------------------------------------------------------------------------------

extern list<ONEFILE>	gltMultiFiles;	//!<	複数ファイル保持

extern FILES_ITR	gitFileIt;	//!<	今見てるファイルの本体
//#define gstFile	(*gitFileIt)	//!<	イテレータを構造体と見なす

extern INT		gixFocusPage;	//!<	注目中のページ・とりあえず０・０インデックス
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------


//統計情報

HRESULT DocStatisticsPage( INT iPage, FILES_ITR itFile )
{

	//	使用文字一覧とか・SQLで文字データベース作ればいい



	return S_OK;
}
//-------------------------------------------------------------------------------------------------


