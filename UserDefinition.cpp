/*! @file
	@brief ユーザ定義挿入の面倒見ます
	このファイルは UserDefinition.cpp です。
	@author	SikigamiHNQ
	@date	2011/06/16
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

//	ユーザ定義壱個分
typedef struct tagUSERITEMS
{
	TCHAR	atItemName[MAX_STRING];

	vector<ONELINE>	vcUnits;	//!<	全体

} USERITEMS, *LPUSERITEMS;
//-------------------------------------------------------------------------------------------------

extern INT		gdDocLine;		//!<	キャレットのＹ行数・ドキュメント位置


static TCHAR	gatUsDfPath[MAX_PATH];	//!<	

static  UINT	gdItemCnt;	//!<	登録されているアイテム数

static USERITEMS	gstUserItem[USER_ITEM_MAX];	//!<	ユーザアイテムの保持
//-------------------------------------------------------------------------------------------------

UINT	CALLBACK UserDefItemLoad( LPTSTR, LPCTSTR, INT );	//!<	ユーザ定義の内容をぶち込む

HRESULT	UserDefAppendMenu( HWND );	//!<	ユーザ定義の内容をメニューに追加
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
UINT CALLBACK UserDefItemLoad( LPTSTR ptName, LPCTSTR ptCont, INT cchSize )
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
HRESULT UserDefSetString( vector<ONELINE> *pvcUnits, LPCTSTR ptText, UINT cchSize )
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
			DocLetterDataCheck( &stLetter, ptText[i] );	//	保持構造体に文字列を記録するとき

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

#pragma message("メニュー構造変わったらユーザ定義の位置であるここも変更")
	hMenu = GetMenu( hWnd );
	hSubMenu = GetSubMenu( hMenu, 2 );
	hMenu = hSubMenu;
	hSubMenu = GetSubMenu( hMenu, 10 );

	UserDefMenuWrite( hSubMenu, 1 );

	DeleteMenu( hSubMenu, IDM_USERINS_NA, MF_BYCOMMAND );	//	Dummy用

	DrawMenuBar( hWnd );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ユーザ定義メニューの中身をくっつける
	@param[in]	hMenu	くっつけるメニューのハンドル
	@param[in]	bMode		非０メニューキー付ける　０付けない
*/
HRESULT UserDefMenuWrite( HMENU hMenu, UINT bMode )
{
	UINT	i;
	TCHAR	atBuffer[MAX_PATH];

	for( i = 0; gdItemCnt > i; i++ )
	{
		if( bMode )
		{
			StringCchPrintf( atBuffer, MAX_PATH, TEXT("%s(&%c)"), gstUserItem[i].atItemName, i+'A' );
		}
		else
		{
			StringCchCopy( atBuffer, MAX_PATH, gstUserItem[i].atItemName );
		}
		AppendMenu( hMenu, MF_STRING, (IDM_USERINS_ITEM_FIRST + i), atBuffer );
	}
	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	アイテムの名前を引っ張る
	@param[in]	dNumber	アイテム番号０インデックス
	@param[out]	ptNamed	名前入れるバッファへのポインター
	@param[in]	cchSize	バッファの文字数・バイトじゃないぞ
	@return		HRESULT	終了状態コード
*/
HRESULT UserDefItemNameget( UINT dNumber, LPTSTR ptNamed, UINT_PTR cchSize )
{
	//	はみ出し確認
	if( gdItemCnt <= dNumber )	return E_OUTOFMEMORY;

	StringCchCopy( ptNamed, cchSize, gstUserItem[dNumber].atItemName );

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


	//	はみ出したらアウツ！
	if( gdItemCnt <= idNum )	return E_OUTOFMEMORY;

	//	今のカーソル行から、行先頭に、各行の内容を挿入していく
	yLine = gdDocLine;

	dNeedLine = gstUserItem[idNum].vcUnits.size( );

	//	まずは頁行数かくぬん
	iLines = DocPageParamGet( NULL , NULL );	//	行数確認・入れ替えていけるか
	//	全体行数より、追加行数が多かったら、改行増やす
	if( iLines < (dNeedLine + yLine) )
	{
		iMinus = (dNeedLine + yLine) - iLines;	//	追加する行数

		DocAdditionalLine( iMinus, &bFirst );//	bFirst = FALSE;

		//	この頁の行数取り直し
		iLines = DocPageParamGet( NULL , NULL );	//	再計算いるか？
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

