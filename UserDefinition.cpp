/*! @file
	@brief ユーザ定義挿入の面倒見ます
	このファイルは UserDefinition.cpp です。
	@author	SikigamiHNQ
	@date	2011/06/16
*/

/*
Orinrin Editor : AsciiArt Story Editor for Japanese Only
Copyright (C) 2011 - 2012 Orinrin/SikigamiHNQ

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

//	ユーザ定義、実は要らない？ＭＬＴ一覧のほうで問題無いのでは

//	ユーザ定義壱個分
typedef struct tagUSERITEMS
{
	TCHAR	atItemName[MAX_STRING];

	vector<ONELINE>	vcUnits;	//!<	全体

} USERITEMS, *LPUSERITEMS;
//-------------------------------------------------------------------------------------------------

extern INT		gdDocLine;		//!<	キャレットのＹ行数・ドキュメント位置


static TCHAR	gatUsDfPath[MAX_PATH];	//!<	

static  UINT	gdItemCnt;	//!<	

static USERITEMS	gstUserItem[USER_ITEM_MAX];	//	固定でいいかな
//-------------------------------------------------------------------------------------------------

UINT	CALLBACK UserDefItemLoad( LPTSTR, LPTSTR, INT );	//!<	

HRESULT	UserDefAppendMenu( HWND );	//!<	
//-------------------------------------------------------------------------------------------------


/*!
	今開いてるUSER定義の内容を全て破棄して新しいの作れる状態にする
	@param[in]	hWnd	メインウインドウハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT UserDefObliterate( HWND hWnd )
{
	UINT_PTR	i, iLine;
	UINT_PTR	j;

	for( i = 0; USER_ITEM_MAX > i; i++ )
	{
		iLine = gstUserItem[i].vcUnits.size( );
		for( j = 0; iLine > j; j++ )
		{
			gstUserItem[i].vcUnits.at( j ).vcLine.clear(   );	//	各行の中身全消し
		}
		gstUserItem[i].vcUnits.clear(  );	//	行を全消し

		ZeroMemory( gstUserItem[i].atItemName, sizeof(gstUserItem[i].atItemName) );
	}

	gdItemCnt = 0;

	//	メニューの内容を、無しにしとえく

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ユーザ定義のアレを読み込む
	@param[in]	hWnd	メインウインドウハンドル
	@param[in]	bFirst	今回が最初であるか
	@return	非０中身取った処理した　０ファイルなかった
*/
INT UserDefInitialise( HWND hWnd, UINT bFirst )
{
	CONST WCHAR rtHead = 0xFEFF;	//	ユニコードテキストヘッダ
	WCHAR	rtUniBuf;

	HANDLE	hFile;
	DWORD	readed;

	LPVOID	pBuffer;	//	文字列バッファ用ポインター
	INT		iByteSize;

	LPTSTR	ptString;
	LPSTR	pcText;
	UINT	cchSize;

	if( bFirst )	//	最初ならパス作っておく
	{
		ZeroMemory( gatUsDfPath, sizeof(gatUsDfPath) );
		StringCchCopy( gatUsDfPath, MAX_PATH, ExePathGet() );
		PathAppend( gatUsDfPath, TEMPLATE_DIR );
		PathAppend( gatUsDfPath, USER_ITEM_FILE );
	}

	hFile = CreateFile( gatUsDfPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile ){	return 0;	}

	//	今の内容破棄
	UserDefObliterate( hWnd );

	iByteSize = GetFileSize( hFile, NULL );
	pBuffer = malloc( iByteSize + 2 );
	ZeroMemory( pBuffer, iByteSize + 2 );
	//	中身をいただく
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	ReadFile( hFile, pBuffer, iByteSize, &readed, NULL );
	CloseHandle( hFile );	//	内容全部取り込んだから開放

	//	ユニコードチャック
	CopyMemory( &rtUniBuf, pBuffer, 2 );
	if( rtHead == rtUniBuf )	//	ユニコードヘッダがあれば
	{
		ptString = (LPTSTR)pBuffer;
		ptString++;	//	ユニコードヘッダ分進めておく
	}
	else
	{
		pcText = (LPSTR)pBuffer;
		//	シフトJISを開く場合、&#0000;の部分をどうにかせんといかん
		ptString = SjisDecodeAlloc( pcText );	//	SJISの内容をユニコードにする

		FREE( pBuffer );	//	こっちで開放
		pBuffer = ptString;	//	ポイントするところを変更
	}

	StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSize );

	//	ASTじゃなきゃダメ
	if( !( StrCmpN( AST_SEPARATERW, ptString, 4 ) ) )
	{
		DocStringSplitAST( ptString , cchSize, UserDefItemLoad );
	}

	FREE( pBuffer );

	UserDefAppendMenu( hWnd );

	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	内容をぶち込む
	@param[in]	ptName	項目の名前
	@param[in]	ptCont	項目の内容
	@param[in]	cchSize	内容の文字数
	@return	非０しょりした　０しなかった
*/
UINT CALLBACK UserDefItemLoad( LPTSTR ptName, LPTSTR ptCont, INT cchSize )
{
	//	枠自体はある
	//	名前をセットしておく

	//	はみ出す分は何もしないよ
	if( USER_ITEM_MAX <= gdItemCnt )	return 0;

	if( ptName )
	{
		StringCchCopy( gstUserItem[gdItemCnt].atItemName, MAX_STRING, ptName );
	}
	else
	{
		StringCchPrintf( gstUserItem[gdItemCnt].atItemName, MAX_STRING, TEXT("(ユーザアイテム No.%d"), gdItemCnt + 1 );
	}

	if( 0 < cchSize )	//	空行でないのなら
	{
		UserDefSetString( &(gstUserItem[gdItemCnt].vcUnits), ptCont, cchSize );	//	この中で改行とか面倒見る
	}

	gdItemCnt++;

	return 1;
}
//-------------------------------------------------------------------------------------------------


/*!
	保持構造体に文字列を記録する
	@param[in]	*pvcUnits	保持ベクータへのポインター
	@param[in]	ptText		記録する文字列
	@param[in]	cchSize		文字数
	@return		HRESULT		終了状態コード
*/
HRESULT UserDefSetString( vector<ONELINE> *pvcUnits, LPTSTR ptText, UINT cchSize )
{
	UINT	i;
	INT		yLine;
	ONELINE	stLine;
	LETTER	stLetter;

	ZeroONELINE( &stLine );

	pvcUnits->push_back( stLine );	//	壱発目

	yLine = 0;
	for( i = 0; cchSize > i; i++ )
	{
		if( CC_CR == ptText[i] && CC_LF == ptText[i+1] )	//	改行であったら
		{
			pvcUnits->push_back( stLine );

			i++;		//	0x0D,0x0Aだから、壱文字飛ばすのがポイント
			yLine++;	//	改行したからFocusは次の行へ
		}
		else if( CC_TAB == ptText[i] )
		{
			//	タブは挿入しない
		}
		else
		{
			ZeroMemory( &stLetter, sizeof(LETTER) );
			stLetter.cchMozi = ptText[i];
			stLetter.rdWidth = ViewLetterWidthGet( ptText[i] );
			stLetter.mzStyle = CT_NORMAL;
			if( iswspace( ptText[i] ) ){	stLetter.mzStyle |= CT_SPACE;	}
			if( !( DocIsSjisTrance( ptText[i], stLetter.acSjis ) ) ){	stLetter.mzStyle |= CT_CANTSJIS;	}
			//	非シフトJIS文字を確認
			DocLetterByteCheck( &stLetter );	//	バイト数確認
			//stLetter.mzByte  = strlen( stLetter.acSjis );

			pvcUnits->at( yLine ).vcLine.push_back( stLetter );

			pvcUnits->at( yLine ).iDotCnt += stLetter.rdWidth;
			pvcUnits->at( yLine ).iByteSz += stLetter.mzByte;
		}
	}

	//	末尾とかの空白も含めてテンプレかなと思ってる

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ユーザ定義の内容をメニューに追加
	@param[in]	hWnd	メインウインドウハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT UserDefAppendMenu( HWND hWnd )
{
	HMENU	hMenu, hSubMenu;

	//	メニュー構造変わったらここも変更・どうにかならんのか
	hMenu = GetMenu( hWnd );
	hSubMenu = GetSubMenu( hMenu, 2 );
	hMenu = hSubMenu;
	hSubMenu = GetSubMenu( hMenu, 10 );

	UserDefMenuWrite( hSubMenu );

	DeleteMenu( hSubMenu, IDM_USERINS_NA, MF_BYCOMMAND );	//	Dummy用

	DrawMenuBar( hWnd );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ユーザ定義メニューの中身をくっつける
*/
HRESULT UserDefMenuWrite( HMENU hMenu )
{
	UINT	i;

	for( i = 0; gdItemCnt > i; i++ )
	{
		AppendMenu( hMenu, MF_STRING, (IDM_USERINS_ITEM_FIRST + i), gstUserItem[i].atItemName );
	}
	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	対象アイテムの対象行の内容をテキストで確保・改行は含まず
	@param[in]	idNum	アイテム番号０～１５
	@param[in]	uLine	行番号
	@return		LPTSTR	確保した内容・開放は呼んだ側が責任もてよ
*/
LPTSTR UserDefTextLineAlloc( UINT idNum, INT uLine )
{
	INT_PTR	iLines, iLetters, i, cchSz;
	LPTSTR	ptText;
	LETR_ITR	itLetter;


	if( gdItemCnt <= idNum )	return NULL;

	iLines = gstUserItem[idNum].vcUnits.size( );
	if( iLines <= uLine )	return NULL;

	iLetters = gstUserItem[idNum].vcUnits.at( uLine ).vcLine.size( );

	cchSz = iLetters + 1;

	ptText = (LPTSTR)malloc( cchSz * sizeof(TCHAR) );
	ZeroMemory( ptText, cchSz * sizeof(TCHAR) );

//	for( i = 0; iLetters > i; i++ ){	ptText[i] = gstUserItem[idNum].vcUnits.at( uLine ).vcLine.at( i ).cchMozi;	}
	for( itLetter = gstUserItem[idNum].vcUnits.at( uLine ).vcLine.begin(), i = 0;
		itLetter != gstUserItem[idNum].vcUnits.at( uLine ).vcLine.end(); i++, itLetter++ )
	{	ptText[i] = itLetter->cchMozi;	}

	ptText[i] = NULL;

	return ptText;
}
//-------------------------------------------------------------------------------------------------

/*!
	選択されたユーザ定義アイテムの処理
	@param[in]	hWnd	多分メインウインドウハンドル
	@param[in]	idNum	アイテム番号０～１５
	@return		HRESULT	終了状態コード
*/
HRESULT UserDefItemInsert( HWND hWnd, UINT idNum )
{
	INT		iLines, yLine, iMinus, i, dmyDot;
	INT_PTR	dNeedLine;
	LPTSTR	ptText;
	BOOLEAN	bFirst = TRUE;


	if( gdItemCnt <= idNum )	return E_OUTOFMEMORY;

	//	今のカーソル行から、行先頭に、各行の内容を挿入していく
	yLine = gdDocLine;

	dNeedLine = gstUserItem[idNum].vcUnits.size( );

	//	まずは頁行数かくぬん
	iLines = DocPageParamGet( NULL, NULL );
	//	全体行数より、追加行数が多かったら、改行増やす
	if( iLines < (dNeedLine + yLine) )
	{
		iMinus = (dNeedLine + yLine) - iLines;	//	追加する行数

		DocAdditionalLine( iMinus, TRUE );
		bFirst = FALSE;

		//	この頁の行数取り直し
		iLines = DocPageParamGet( NULL, NULL );	//	再計算いるか？
	}

	for( i = 0; dNeedLine > i; i++, yLine++ )
	{
		ptText = UserDefTextLineAlloc( idNum, i );

		dmyDot = 0;
		DocInsertString( &dmyDot, &yLine, NULL, ptText, 0, bFirst );	bFirst = FALSE;

		FREE(ptText);
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

