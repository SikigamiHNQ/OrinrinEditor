/*! @file
	@brief MLTファイルを読んで、中身をメモリに展開します
	このファイルは MaaCatalogue.cpp です。
	@author	SikigamiHNQ
	@date	2011/06/22
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
#include "MaaTemplate.h"
//-------------------------------------------------------------------------------------------------

/*

AAが1行もしくは2行で、かつ内容が以下の文字列以外から始まる場合、
そのAAを小見出しとして認識し、menu項目に追加します。

Last Mod
最終更
　　（全角空白2つ）
　 （全角空白＋半角空白）
:
_
（
(
.
/

ただし、「区切り.mlt」・「吹き出し・囲み用.mlt」については、以下の文字列から始まる場合のみ、
そのAAを小見出しとして認識、menu項目に追加します。

【

【　無き２０文字以上は認識しないとかのロジック必要か
*/


#define MLT_SEPARATER	("[SPLIT]")
#define AST_SEPARATER	("[AA]")

//	構造体定義・OrinrinEditor.hへ移動

//-------------------------------------------------------------------------------------------------

static  vector<AAMATRIX>	gvcArts;	//!<	開いたAAの保持

static TCHAR	gatOpenFile[MAX_PATH];	//!<	開いたファイル名

static TCHAR	gatBkUpDir[MAX_PATH];	//!<	MLT書き換えた時のバックアップ作成
//-------------------------------------------------------------------------------------------------


DWORD	AacInflateMlt( LPSTR, DWORD );
DWORD	AacInflateAst( LPSTR, DWORD );

UINT	AacTitleCheck( LPAAMATRIX );

LRESULT	CALLBACK AacFavInflate( UINT, UINT, UINT, LPCVOID );

#ifdef MAA_IADD_PLUS
UINT	AacItemBackUpCreate( LPVOID );
HRESULT	AacItemOutput( HWND );
#endif
//-------------------------------------------------------------------------------------------------

/*!
	バックアップディレクトリーを確保・起動時に壱回呼ばれるだけ
	@param[in]	ptCurrent	基準ディレクトリ
*/
VOID AacBackupDirectoryInit( LPTSTR ptCurrent )
{
	StringCchCopy( gatBkUpDir, MAX_PATH, ptCurrent );
	PathAppend( gatBkUpDir, BACKUP_DIR );
	CreateDirectory( gatBkUpDir, NULL );	//	已にディレクトリがあったら函数が失敗するだけなので問題無い

	return;
}
//-------------------------------------------------------------------------------------------------



/*!
	MLTファイル名を受け取って、展開処理を進める
	@param[in]	hWnd		親ウインドウハンドル
	@param[in]	ptFileName	ファイル名
	@return		DWORD		確保した個数
*/
DWORD AacAssembleFile( HWND hWnd, LPTSTR ptFileName )
{
	HANDLE	hFile;
	DWORD	rdFileSize, readed, rdCount;
	BOOL	isAST;
	LPSTR	pcFullBuff;

	hFile = CreateFile( ptFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile ){	return 0;	}

	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );

	rdFileSize = GetFileSize( hFile, NULL );	//	４Ｇ超えに対応を？

	StringCchCopy( gatOpenFile , MAX_PATH, ptFileName );	//	ファイル名記録

	//	ASTであるか？
	isAST = PathMatchSpec( ptFileName , TEXT("*.ast") );

	//	全体バッファ作って
	pcFullBuff = (LPSTR)malloc( rdFileSize + 1 );
	if( !(pcFullBuff) ){	return 0;	}

	ZeroMemory( pcFullBuff, rdFileSize + 1 );
	//	中身読込
	ReadFile( hFile, pcFullBuff, rdFileSize, &readed, NULL );

	CloseHandle( hFile );	//	内容確保したから閉じておｋ

	AacMatrixClear(   );	//	既存の内容全破壊

//	ASTはここで展開すればいい
	//	展開処理する
	if( isAST ){	rdCount = AacInflateAst( pcFullBuff, readed );	}
	else{			rdCount = AacInflateMlt( pcFullBuff, readed );	}

#pragma message ("旧末尾追加したやつは末端に0x0Dがくっついてる・飛ばして読み込む処理を")

	free( pcFullBuff );	//	開放忘れないように注意セヨ

	return rdCount;
}
//-------------------------------------------------------------------------------------------------

/*!
	ASTの全体バッファを受け取って、分解して保持
	@param[in]	pcTotal	全体バッファ
	@param[in]	cbTotal	バイト数
	@return		DWORD	確保した個数
*/
DWORD AacInflateAst( LPSTR pcTotal, DWORD cbTotal )
{
	LPSTR	pcCaret;	//	読込開始・現在位置
	LPSTR	pcStart;	//	セパレータの直前
	LPSTR	pcEnd;
	//LPSTR	pcCheck;
	UINT	iNumber;	//	通し番号カウント

	UINT	cbItem;
	BOOLEAN	bLast;
//	CHAR	acName[MAX_STRING];

	AAMATRIX	stAAbuf;//	一つのAAの保持・ベクターに入れる


	bLast = FALSE;
	iNumber = 0;	//	通し番号０インデックス

	pcCaret = StrStrA( pcTotal, AST_SEPARATER );	//	最初のセパレータまで進める
	if( !pcCaret )	return 0;	//	この時点で無かったらおかしい

	do	//	ptCaretは[AA]になってるはず
	{
		ZeroMemory( &stAAbuf, sizeof(AAMATRIX) );

		stAAbuf.ixNum = iNumber;

		pcStart = NextLineA(  pcCaret );	//	次の行からが本番

		pcCaret += 5;	//	[AA][
		cbItem  = pcStart - pcCaret;	//	名前部分の文字数
		cbItem -= 3;	//	]rn

	
		if( 0 < cbItem )	//	名前確保
		{
			StringCchCopyNA( stAAbuf.acAstName, MAX_STRING, pcCaret, cbItem );
			AaTitleAddString( iNumber , stAAbuf.acAstName );	//	見出し追加
		}

		pcCaret = pcStart;	//	本体部分

		pcEnd = StrStrA( pcCaret, AST_SEPARATER );	//	セパレータを探す
		//	この時点でpcEndは次の[AA]をさしてる・もしくはNULL(最後のコマ)
		if( !pcEnd )	//	見つからなかったら＝これが最後なら＝NULL
		{
			pcEnd = pcTotal + cbTotal;	//	CHARサイズで計算おｋ？
			bLast = TRUE;
		}
		stAAbuf.cbItem = pcEnd - pcCaret;	//	CHAR単位であるか

		//＠＠	末端の0x0D単独を外す
		//pcCheck = pcEnd;	pcCheck--;
		//if( 0x0D == *pcCheck )	stAAbuf.cbItem--;

		//	最終頁でない場合は末端の改行分引く
		//if( !(bLast) && 0 < cbItem ){	cbItem -= CH_CRLF_CCH;	}

		stAAbuf.pcItem = (LPSTR)malloc( stAAbuf.cbItem + 2 );
		ZeroMemory( stAAbuf.pcItem, stAAbuf.cbItem + 2 );

		if( 0 >= stAAbuf.cbItem )
		{
			stAAbuf.cbItem = 2;
			StringCchCopyA( stAAbuf.pcItem, stAAbuf.cbItem, (" ") );
		}
		else
		{
			CopyMemory( stAAbuf.pcItem, pcCaret, stAAbuf.cbItem );
		}

		gvcArts.push_back( stAAbuf );	//	ベクターに追加
		//	ここで stAAbuf.pcItem をフリーしてはいけない・vectorで使っている

		iNumber++;

		pcCaret = pcEnd;

	}while( *pcCaret );	//	データ有る限りループで探す

	return iNumber;
}
//-------------------------------------------------------------------------------------------------

/*!
	MLTの全体バッファを受け取って、分解して保持
	@param[in]	pcTotal	全体バッファ
	@param[in]	cbTotal	バイト数
	@return		DWORD	確保した個数
*/
DWORD AacInflateMlt( LPSTR pcTotal, DWORD cbTotal )
{
	LPSTR	pcCaret;	//	読込開始・現在位置
	LPSTR	pcEnd;		//	一つのAAの末端位置・セパレータの直前
	//LPSTR	pcCheck;
	DWORD	iNumber;	//	通し番号カウント
	AAMATRIX	stAAbuf;//	一つのAAの保持・ベクターに入れる

	pcCaret = pcTotal;	//	まずは最初から

	iNumber = 0;	//	通し番号０インデックス
	//	始点にはセパレータ無いものとみなす。連続するセパレータは、改行のみとして扱う

	do
	{
		ZeroMemory( &stAAbuf, sizeof(AAMATRIX) );

		stAAbuf.ixNum = iNumber;

		pcEnd = StrStrA( pcCaret, MLT_SEPARATER );	//	セパレータを探す
		if( !pcEnd )	//	見つからなかったら＝これが最後なら＝NULL
		{
			pcEnd = pcTotal + cbTotal;
		}
		stAAbuf.cbItem = pcEnd - pcCaret;	//	バイト数なのでこれでいいはず

		//＠＠	末端の0x0D単独を外す
		//if( 0 != stAAbuf.cbItem )
		//{
		//	pcCheck = pcEnd;	pcCheck--;
		//	if( 0x0D == *pcCheck )	stAAbuf.cbItem--;
		//}

		stAAbuf.pcItem = (LPSTR)malloc( stAAbuf.cbItem + 2 );
		ZeroMemory( stAAbuf.pcItem, stAAbuf.cbItem + 2 );

		if( 0 >= stAAbuf.cbItem )
		{
			stAAbuf.cbItem = 2;
			StringCchCopyA( stAAbuf.pcItem, stAAbuf.cbItem, (" ") );
		}
		else
		{
			CopyMemory( stAAbuf.pcItem, pcCaret, stAAbuf.cbItem );
		}

		AacTitleCheck( &stAAbuf );	//	内容確認してできそうなら見出しにする

		gvcArts.push_back( stAAbuf );	//	ベクターに追加
		//	ここで stAAbuf.pcItem をフリーしてはいけない・vectorで使っている

		iNumber++;

		pcCaret = NextLineA( pcEnd );

	}while( *pcCaret );	//	データ有る限りループで探す


	return iNumber;
}
//-------------------------------------------------------------------------------------------------

/*!
	見出しになり得るTangoを探す
	@param[in]	pstItem	チェックするターゲット内容
	@return		UINT	非０見出しあった　０なかった
*/
UINT AacTitleCheck( LPAAMATRIX pstItem )
{
	LPSTR	pcCaret, pcEnd, pcOpen;
	LPSTR	pcLine;
	UINT	cbSize, d;

	if( 4 > pstItem->cbItem )	return 0;	//	小さすぎたら何も無い

	//	とりあえず先頭壱行目について
	pcCaret = pstItem->pcItem;

	//	先頭について
	if( !( strncmp( pcCaret, "【", 2 ) ) )	//	ブランケットであるか
	{
		pcOpen = pcCaret + 2;	//	内容開始位置

		cbSize  = pstItem->cbItem;
		cbSize -= 2;	//	壱行の場合を見越して計算

		pcEnd = NextLineA( pcCaret );
		//	中身がNULLなら、改行無し壱行である
		if( *pcEnd )
		{
			pcCaret = pcOpen;
			cbSize = pcEnd - pcCaret;
		}
		//	ここで、cbSizeは壱行の長さになる
		pcLine = (LPSTR)malloc( cbSize+1 );
		ZeroMemory( pcLine, cbSize+1 );
		CopyMemory( pcLine, pcOpen, cbSize );
		pcCaret = pcLine;
		for( d = 0; cbSize > d; d++ )
		{
			if( !(strncmp( pcCaret, "】", 2 ) ) )
			{
				*pcCaret = NULL;
				break;
			}
			pcCaret++;
		}
		//ここまでで、pcLineに【】の文字列が残っているはず
		AaTitleAddString( pstItem->ixNum, pcLine );

		FREE(pcLine);
	}
	else
	{
		//	先頭が特定の文字であるなら見出しではない
		if( ':' ==  pcCaret[0] )	return 0;
		if( '_' ==  pcCaret[0] )	return 0;
		if( '(' ==  pcCaret[0] )	return 0;
		if( '.' ==  pcCaret[0] )	return 0;
		if( '/' ==  pcCaret[0] )	return 0;
		//	先頭数文字が特定の文字列であるなら見出しではない
		if( !( strncmp( pcCaret, "（", 2 ) ) )	return 0;
		if( !( strncmp( pcCaret, "　 ", 3 ) ) )	return 0;	//	全角空白＋半角空白
		if( !( strncmp( pcCaret, "　　", 4 ) ) )	return 0;	//	全角空白2つ
		if( !( strncmp( pcCaret, "最終更", 4 ) ) )	return 0;
		if( !( strncmp( pcCaret, "Last Mod", 8 ) ) )	return 0;

		//	見出しと見なして内容をゲットする。
		cbSize = pstItem->cbItem;	//	壱行の場合

		pcEnd = NextLineA( pcCaret );
		//NULLなら、改行無し壱行である
		if( *pcEnd )
		{
			cbSize = pcEnd - pcCaret;	//	改行分注意

			pcOpen = pcEnd;
			pcEnd  = NextLineA( pcOpen );	//	次行確認
			//	ここで、３行目以降があれば見出しとは見なさない
			if( *pcEnd )	return 0;
		}

		//	ここで、cbSizeは壱行の長さになる

		//	もし、行が妙に長かったら、見出しとは見なさない
		if( 42 <= cbSize )	return 0;

		pcLine = (LPSTR)malloc( cbSize );
		ZeroMemory( pcLine, cbSize );
		CopyMemory( pcLine, pcCaret, cbSize-2 );	//	改行分カット

		AaTitleAddString( pstItem->ixNum, pcLine );

		FREE(pcLine);
	}

	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	確保してるアイテム数を返す
*/
INT_PTR AacItemCount( UINT reserve )
{
	return gvcArts.size();
}
//-------------------------------------------------------------------------------------------------

/*!
	通し番号を受けて、HBITMAPとサイズを返す
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	iNumber	通し番号０インデックス
	@param[out]	pstSize	大きさ
	@param[out]	pstArea	ドットｘライン
	@return	HBITMAP	AAの内容のビットマップを返す。已にあるならそのまま、アイテムないならNULL
*/
HBITMAP AacArtImageGet( HWND hWnd, INT iNumber, LPSIZE pstSize, LPSIZE pstArea )
{
	INT_PTR		iItems, i;
	MAAM_ITR	itArts;

	pstSize->cx = 0;
	pstSize->cy = 0;

	pstArea->cx = 0;
	pstArea->cy = 0;

	iItems = gvcArts.size( );
	if( iItems <= iNumber ){	return NULL;	}	//	はみ出しの場合

	itArts = gvcArts.begin();
	for( i = 0; iNumber >  i; i++ ){	itArts++;	}


	//	確保済の場合	hThumbDC
	if( itArts->hThumbBmp )
	{
		pstSize->cx = itArts->stSize.cx;
		pstSize->cy = itArts->stSize.cy;

		pstArea->cx = itArts->iMaxDot;
		pstArea->cy = itArts->iLines;

		return	itArts->hThumbBmp;
	}


	//	未確保の場合
	DraughtAaImageing( hWnd, &(*itArts) );

	pstSize->cx = itArts->stSize.cx;
	pstSize->cy = itArts->stSize.cy;

	pstArea->cx = itArts->iMaxDot;
	pstArea->cy = itArts->iLines;

	return itArts->hThumbBmp;
}
//-------------------------------------------------------------------------------------------------

/*!
	通し番号を受けて、内容を返す
	@param[in]	iNumber	通し番号０インデックス
	@return	LPSTR	AAの内容を、メモリ確保して渡す。受け取った方で開放する
*/
LPSTR AacAsciiArtGet( DWORD iNumber )
{
	size_t	items;
	LPSTR	pcBuff;
//	MAAM_ITR	itMaam;

	items = gvcArts.size( );
	if( items <= iNumber )	return NULL;

	pcBuff = (LPSTR)malloc( gvcArts.at( iNumber ).cbItem + 1 );
	ZeroMemory( pcBuff, gvcArts.at( iNumber ).cbItem + 1 );
	CopyMemory( pcBuff, gvcArts.at( iNumber ).pcItem, gvcArts.at( iNumber ).cbItem );

	return pcBuff;
}
//-------------------------------------------------------------------------------------------------

/*!
	通し番号を受けて、内容の最大ドット数と行数とバイト数を戻す
	@param[in]	iNumber	通し番号０インデックス
	@param[out]	piLine	行数返す
	@param[out]	pBytes	バイト数返す（Editorのみ）
	@return		最大ドット数
*/
INT AacArtSizeGet( DWORD iNumber, PINT piLine, PINT pBytes )
{
	INT	iDot, cx, cy, iByte = 0;
	size_t	items;
	LPSTR	pcConts;
	LPTSTR	ptString;

	items = gvcArts.size( );
	if( items <=  iNumber ){	return 0;	}

	if( 0 >= gvcArts.at( iNumber ).stSize.cx || 0 >= gvcArts.at( iNumber ).stSize.cy )
	{
		pcConts = (LPSTR)malloc( gvcArts.at( iNumber ).cbItem + 1 );
		if( pcConts )
		{
			TRACE( TEXT("AA Size Calculate[%d]"), iNumber );

			ZeroMemory( pcConts, gvcArts.at( iNumber ).cbItem + 1 );
			CopyMemory( pcConts, gvcArts.at( iNumber ).pcItem, gvcArts.at( iNumber ).cbItem );

			ptString = SjisDecodeAlloc( pcConts );
			FREE( pcConts );
			if( ptString )
			{
				cx = TextViewSizeGet( ptString, &cy );
#ifndef _ORRVW
				DocRawDataParamGet( ptString, NULL, &iByte );
#endif
				FREE( ptString );

				gvcArts.at( iNumber ).stSize.cx = cx;
				gvcArts.at( iNumber ).stSize.cy = cy;
				gvcArts.at( iNumber ).iByteSize = iByte;
			}
		}
	}

	iDot    = gvcArts.at( iNumber ).stSize.cx;
	if( piLine ){	*piLine = gvcArts.at( iNumber ).stSize.cy;	}
	if( pBytes ){	*pBytes = gvcArts.at( iNumber ).iByteSize;	}

	return iDot;
}
//-------------------------------------------------------------------------------------------------

/*!
	MLTの保持内容を全破棄
	@return	HRESULT	終了状態コード
*/
HRESULT AacMatrixClear( VOID )
{
	MAAM_ITR	itArts;

	//	先に領域を開放
	for( itArts = gvcArts.begin(); itArts != gvcArts.end(); itArts++ )
	{
		FREE( itArts->pcItem );

		if( itArts->hThumbBmp )
		{
	//		SelectBitmap( itArts->hThumbDC, itArts->hOldBmp );
			DeleteBitmap( itArts->hThumbBmp );
	//		DeleteDC( itArts->hThumbDC );
		}
	}

	gvcArts.clear();	//	そして全破棄

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	使用の区分ディレクトリ名を受け取って、ＳＱＬから中身を確保する
	@param[in]	hWnd		親ウインドウハンドル・NULLなら破壊
	@param[in]	ptBlockName	区分ディレクトリ名
	@return		DWORD		確保した個数
*/
DWORD AacAssembleSql( HWND hWnd, LPCTSTR ptBlockName )
{
	DWORD	ixItems;

	ixItems = SqlFavCount( ptBlockName, NULL );

	AacMatrixClear(   );	//	既存の内容全破壊

	if( 0 == ixItems )	return 0;

	//	コールバックして中身をベクターに確保
	SqlFavArtEnum( ptBlockName, AacFavInflate );

	return ixItems;
}
//-------------------------------------------------------------------------------------------------

/*!
	コールバックでお気に入りリストの中身をゲッツ
	@param[in]	dLength	バイト数
	@param[in]	dummy	未使用
	@param[in]	fake	未使用
	@param[in]	pcConts	AA本体
	@return		UINT	確保した個数
*/
LRESULT CALLBACK AacFavInflate( UINT dLength, UINT dummy, UINT fake, LPCVOID pcConts )
{
	UINT_PTR	iNumber;	//	通し番号
	AAMATRIX	stAAbuf;	//	一つのAAの保持・ベクターに入れる

	iNumber = gvcArts.size( );	//	個数が出てくるので、０インデックスになる

	TRACE( TEXT("NUM[%d] byte[%d]"), iNumber, dLength );

	ZeroMemory( &stAAbuf, sizeof(AAMATRIX) );

	stAAbuf.ixNum = iNumber;

	stAAbuf.cbItem = dLength;	//	バイト数なのでこれでいいはず
	stAAbuf.pcItem = (LPSTR)malloc( stAAbuf.cbItem + 2 );
	ZeroMemory( stAAbuf.pcItem, stAAbuf.cbItem + 2 );

	CopyMemory( stAAbuf.pcItem, pcConts, stAAbuf.cbItem );

	gvcArts.push_back( stAAbuf );	//	ベクターに追加

	return 0;
}
//-------------------------------------------------------------------------------------------------



#ifdef MAA_IADD_PLUS

#ifndef _ORRVW


typedef struct tagITEMADDINFO
{
	LPTSTR	ptContent;			//!<	本文内容
	TCHAR	atSep[MAX_STRING];	//!<	セパレータ内容
	INT		bType;				//!<	非０MLT　０AST

} ITEMADDINFO, *LPITEMADDINFO;
//--------------------------------------


/*!
	アイテム追加の面倒見るダイヤログー
	@param[in]	hDlg		ダイヤログハンドル
	@param[in]	message		ウインドウメッセージの識別番号
	@param[in]	wParam		追加の情報１
	@param[in]	lParam		追加の情報２
	@retval 0	メッセージは処理していない
	@retval no0	なんか処理された
*/
INT_PTR CALLBACK AaItemAddDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static LPITEMADDINFO	pstIaInfo;
	static LPTSTR	ptBuffer;
	UINT_PTR	cchSize;
	TCHAR	atName[MAX_PATH];
	INT		id;
	RECT	rect;

	switch( message )
	{
		case WM_INITDIALOG:
			pstIaInfo = (LPITEMADDINFO)(lParam);
			GetClientRect( hDlg, &rect );
			CreateWindowEx( 0, WC_BUTTON, TEXT("今の頁"),         WS_CHILD | WS_VISIBLE, 0, 0, 75, 23, hDlg, (HMENU)IDB_MAID_NOWPAGE, GetModuleHandle(NULL), NULL );
			CreateWindowEx( 0, WC_BUTTON, TEXT("クリップボード"), WS_CHILD | WS_VISIBLE, 75, 0, 120, 23, hDlg, (HMENU)IDB_MAID_CLIPBOARD, GetModuleHandle(NULL), NULL ); 
			CreateWindowEx( 0, WC_EDIT,   TEXT(""),               WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 195, 0, rect.right-195-50, 23, hDlg, (HMENU)IDE_MAID_ITEMNAME, GetModuleHandle(NULL), NULL );
			CreateWindowEx( 0, WC_BUTTON, TEXT("追加"),           WS_CHILD | WS_VISIBLE, rect.right-50, 0, 50, 23, hDlg, (HMENU)IDB_MAID_ADDGO, GetModuleHandle(NULL), NULL );
			CreateWindowEx( 0, WC_EDIT,   TEXT(""),               WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY, 0, 23, rect.right, rect.bottom-23, hDlg, (HMENU)IDE_MAID_CONTENTS, GetModuleHandle(NULL), NULL );
			//	なんでわざわざこうやったのか思い出せない。

			if( pstIaInfo->bType )
			{
				SetDlgItemText( hDlg, IDE_MAID_ITEMNAME, TEXT("名称はASTでないと使用できないよ") );
				EnableWindow( GetDlgItem(hDlg,IDE_MAID_ITEMNAME), FALSE );
				StringCchCopy( pstIaInfo->atSep, MAX_STRING, TEXT("[SPLIT]\r\n") );
			}

			//	とりあえずクリップボードの中身をとる
			ptBuffer = DocClipboardDataGet( NULL );
			if( !(ptBuffer) ){	DocPageGetAlloc( D_UNI , (LPVOID *)(&ptBuffer) );	}
			//	使えないシロモノなら、今の頁の内容を持ってきて表示
			SetDlgItemText( hDlg, IDE_MAID_CONTENTS, ptBuffer );
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			id = LOWORD(wParam);
			switch( id )
			{
				case IDCANCEL:
					FREE(ptBuffer);
					EndDialog(hDlg, 0 );
					return (INT_PTR)TRUE;

				case IDB_MAID_ADDGO:	//	追加実行
					if( ptBuffer )
					{
						StringCchLength( ptBuffer, STRSAFE_MAX_CCH, &cchSize );
						cchSize += 4;
						pstIaInfo->ptContent = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
						StringCchCopy( pstIaInfo->ptContent, cchSize, ptBuffer );
						//	スプリッタ用の改行
						StringCchCat( pstIaInfo->ptContent, cchSize, CH_CRLFW );

						if( !(pstIaInfo->bType) )	//	ASTならタイトル頂く
						{
							GetDlgItemText( hDlg, IDE_MAID_ITEMNAME, atName, MAX_PATH );
					//		StringCchPrintf( pstIaInfo->atSep, MAX_STRING, TEXT("[AA][%s]\r\n"), atName );	//	このままではタグが多重に保存されてまう
							StringCchCopy( pstIaInfo->atSep, MAX_STRING, atName );
						}
					}
					FREE(ptBuffer);
					EndDialog(hDlg, 1 );
					return (INT_PTR)TRUE;

				case IDB_MAID_CLIPBOARD:
					FREE(ptBuffer);
					ptBuffer = DocClipboardDataGet( NULL );
					SetDlgItemText( hDlg, IDE_MAID_CONTENTS, ptBuffer );
					return (INT_PTR)TRUE;

				case IDB_MAID_NOWPAGE:
					FREE(ptBuffer);
					DocPageGetAlloc( D_UNI , (LPVOID *)(&ptBuffer) );
					SetDlgItemText( hDlg, IDE_MAID_CONTENTS, ptBuffer );
					return (INT_PTR)TRUE;

				default:	break;
			}
			break;

		case WM_SIZE:
			GetClientRect( hDlg, &rect );
			MoveWindow( GetDlgItem(hDlg,IDE_MAID_ITEMNAME), 195, 0, rect.right-195-50, 23, TRUE );
			MoveWindow( GetDlgItem(hDlg,IDB_MAID_ADDGO),    rect.right-50, 0, 50, 23, TRUE );
			MoveWindow( GetDlgItem(hDlg,IDE_MAID_CONTENTS), 0, 23, rect.right, rect.bottom-23, TRUE );
			break;
	}

	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	通し番号を受けて、それを削除
	@param[in]	hWnd	親ウインドウハンドル
	@param[in]	iNumber	通し番号０インデックス・マイナスなら何もしない
	@return	HRESULT	終了状態コード
*/
HRESULT AacItemDelete( HWND hWnd, LONG iNumber )
{
	UINT		curSel;
	MAAM_ITR	itMaaItem;

	if( 0 > iNumber )	return E_OUTOFMEMORY;

	itMaaItem = gvcArts.begin();
	std::advance( itMaaItem , iNumber );	//	目標まで進める

	//デリート確認がいる

	gvcArts.erase( itMaaItem );	//	デリート処理

	AacItemBackUpCreate( NULL );	//	バックアップしておく

	AacItemOutput( hWnd );	//	ここでファイルを出力更新

	//	追加処理したらリロードする
	curSel = TabMultipleNowSel(  );
	AaItemsDoShow( hWnd, gatOpenFile, curSel );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	通し番号を受けて、そこの手前にアイテム追加
	@param[in]	hWnd	親ウインドウハンドル
	@param[in]	iNumber	通し番号０インデックス・マイナスなら末尾に追加とする
	@return	HRESULT	終了状態コード
*/
HRESULT AacItemInsert( HWND hWnd, LONG iNumber )
{
	UINT_PTR	cbSize;
	LPSTR		pcName;
	INT			curSel;
	AAMATRIX	stAAbuf;	//	一つのAAの保持・ベクターに入れる
	ITEMADDINFO	stIaInfo;

	ZeroMemory( &stAAbuf, sizeof(AAMATRIX) );
	ZeroMemory( &stIaInfo, sizeof(ITEMADDINFO) );

	//	拡張子確認
	if( FileExtensionCheck( gatOpenFile, TEXT(".ast") ) ){	stIaInfo.bType =  0;	}
	else{	stIaInfo.bType =  1;	}

	if( DialogBoxParam( GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAA_IADD_DLG), hWnd, AaItemAddDlgProc, (LPARAM)(&stIaInfo) ) )
	{
		if( stIaInfo.ptContent )	//	中身が有効なら処理する
		{
			//	名前確保
			pcName = SjisEncodeAlloc( stIaInfo.atSep  );	//	変換
			StringCchLengthA( pcName, STRSAFE_MAX_CCH, &cbSize );
			StringCchCopyA( stAAbuf.acAstName, MAX_STRING, pcName );
			FREE( pcName );
			stAAbuf.cbItem = cbSize;
			//	本体確保
			stAAbuf.pcItem = SjisEncodeAlloc( stIaInfo.ptContent  );

			if( 0 <= iNumber )	gvcArts.insert( gvcArts.begin() + iNumber, stAAbuf );
			else				gvcArts.push_back( stAAbuf );
		//	ここで stAAbuf.pcItem をフリーしてはいけない・vectorで使っている

			FREE( stIaInfo.ptContent );

			AacItemBackUpCreate( NULL );	//	バックアップしておく

			AacItemOutput( hWnd );	//	ここでファイルを出力更新

			//追加処理したらリロードする・コンボックスクルヤーとか注意
		//	AaTitleClear(  );	//	AaItemsDoShowの中でやってるからここには不要
			curSel = TabMultipleNowSel(  );
			AaItemsDoShow( hWnd, gatOpenFile, curSel );
		}
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	変更結果を保存する前に、バックアップとっておく
*/
UINT AacItemBackUpCreate( LPVOID pVoid )
{
	TCHAR	atOutFile[MAX_PATH], atFileName[MAX_PATH];

	//	ファイル名確保
	StringCchCopy( atFileName, MAX_PATH, PathFindFileName( gatOpenFile ) );
	//	適当に拡張子付けておく
	StringCchCat( atFileName, MAX_PATH, TEXT(".abk") );
	//	パスを作る
	StringCchCopy( atOutFile, MAX_PATH, gatBkUpDir );
	PathAppend( atOutFile, atFileName );	//	Backupファイル名

	//	ファイルをコピーする
	CopyFile( gatOpenFile, atOutFile, FALSE );	//	既存のファイルは上書きする

	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	更新した内容をアウトプットする
*/
HRESULT AacItemOutput( HWND hWnd )
{
	BOOLEAN	isAST;
	CHAR	acSep[MAX_STRING];

	HANDLE	hFile;
	DWORD	wrote;
	UINT_PTR	cbSize;

	INT_PTR	i, iPage;

	MAAM_ITR	itAamx;


	//	拡張子確認
	if( FileExtensionCheck( gatOpenFile, TEXT(".ast") ) ){	isAST = TRUE;	}
	else{	isAST = FALSE;	}

	hFile = CreateFile( gatOpenFile, GENERIC_WRITE, 0, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile )
	{
		MessageBox( hWnd, TEXT("ファイルを開けなかったよ"), TEXT("お燐からのお知らせ"), MB_OK | MB_ICONERROR );
		return E_HANDLE;
	}

	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );

	iPage = gvcArts.size();
	//	全部出力していく
	for( i = 0, itAamx = gvcArts.begin(); gvcArts.end() != itAamx; itAamx++, i++ )
	{
		//	区切りを出力	余計な改行が本文にくっつくのを修正
		ZeroMemory( acSep, sizeof(acSep) );	cbSize = 0;
		if( isAST )	//	ASTファイルである
		{
			StringCchPrintfA( acSep , MAX_STRING, ("[AA][%s]%s"), itAamx->acAstName, CH_CRLFA );
		}
		else	//	MLTである。TXTかもしれない。
		{
			if( 1 <= i )	//	ファイル先頭には不要
			{
				StringCchPrintfA( acSep, MAX_STRING, ("%s%s"), MLT_SEPARATERA, CH_CRLFA );
			}
		}
		StringCchLengthA( acSep , MAX_STRING, &cbSize );	//	長さ確認、MLTの場合注意
		if( cbSize ){	WriteFile( hFile, acSep, cbSize, &wrote, NULL );	}

		//	本文出力
		StringCchLengthA( itAamx->pcItem, STRSAFE_MAX_CCH, &cbSize );
		WriteFile( hFile, itAamx->pcItem, cbSize, &wrote, NULL );
	}

	SetEndOfFile( hFile );
	CloseHandle( hFile );


	return S_OK;
}
//-------------------------------------------------------------------------------------------------

#endif

#endif	//	MAA_IADD_PLUS
