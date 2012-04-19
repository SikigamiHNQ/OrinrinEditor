/*! @file
	@brief ドキュメントの内容の管理をします
	このファイルは DocCore.cpp です。
	@author	SikigamiHNQ
	@date	2011/04/30
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
/*
	文字数は、キャレットの左側の文字数で数える。キャレットが左端なら０文字目
*/

//-------------------------------------------------------------------------------------------------

//	構造体宣言はコモンへ移動

//-------------------------------------------------------------------------------------------------

#define PAGE_LINE_MAX	80
#define LINE_MOZI_MAX	255
//-------------------------------------------------------------------------------------------------

/*
(*pstTexts).acSjis	構造体ポインタはこれでもおｋ

ポインタ確保はこうやる
vector<int>::iterator it = vec.begin();   // vec は vector<int>型の変数
it++;
int* p = &*it;  // 内部要素のアドレスを取得しようとしている。OK

gstFileを、マクロでイテレータポインタに仕立て上げる
フォーカスしてるファイルは常に一つなので、ポインタの中身を変えるか、
もしくはマクロでイテレータ自体にしてしまえばいいか

(*ltrItr).cchMozi;	これでおｋっぽい
*/

//#define FILE_PRELOAD	//	先に頁のvectorを確保してみる・リストで要らない・文字用にいるか？


EXTERNED list<ONEFILE>	gltMultiFiles;	//!<	複数ファイル保持
//イテレータのtypedefはヘッダへ

static LPARAM	gdNextNumber;		//!<	開いたファイルの通し番号・常にインクリ

EXTERNED FILES_ITR	gitFileIt;		//!<	今見てるファイルの本体
//#define gstFile	(*gitFileIt)	//!<	イテレータを構造体と見なす

EXTERNED INT		gixFocusPage;	//!<	注目中のページ・とりあえず０・０インデックス

EXTERNED INT		gixDropPage;	//!<	投下ホット番号

extern  UINT		gbUniRadixHex;	//	ユニコード数値参照が１６進数であるか
extern  UINT		gbCrLfCode;		//	改行コード：０したらば・非０ＹＹ 
//-------------------------------------------------------------------------------------------------

UINT	CALLBACK DocPageLoad( LPTSTR, LPTSTR, INT );
//-------------------------------------------------------------------------------------------------

/*!
	なんか初期化
*/
HRESULT DocInitialise( UINT dMode )
{
#ifdef PAGE_DELAY_LOAD
	FILES_ITR	itFile;
	PAGE_ITR	itPage;

#endif

	if( dMode )	//	作成時
	{
		gdNextNumber = 1;
	}
	else
	{
#ifdef PAGE_DELAY_LOAD
		for( itFile = gltMultiFiles.begin( ); itFile != gltMultiFiles.end(); itFile++ )
		{
			for( itPage = itFile->vcCont.begin( ); itPage != itFile->vcCont.end(); itPage++ )
			{
				FREE( itPage->ptRawData );
			}
		}
#endif
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	変更したか
	@param[in]	dMode	非０変更した　０保存したから変更はなかったことに
	@return		HRESULT	終了状態コード
*/
HRESULT DocModifyContent( UINT dMode )
{
	if( dMode )
	{
		if( (*gitFileIt).dModify )	return S_FALSE;
		//	変更のとき、已に変更の処理してたら何もしなくて良い

		StatusBarSetText( SB_MODIFY, MODIFY_MSG );
	}
	else
	{
		StatusBarSetText( SB_MODIFY, TEXT("") );
	}

	DocMultiFileModify( dMode );

	(*gitFileIt).dModify =  dMode;	//	ここで記録しておく

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	新しいファイル置き場を作ってフォーカスする
	@param[in]	ptDmyName	ダミー名を返す。NULL可。MAX_PATHであること
	@return		LPARAM	対応するユニーク番号
*/
LPARAM DocMultiFileCreate( LPTSTR ptDmyName )
{
	ONEFILE	stFile;
	FILES_ITR	itNew;

#ifdef DO_TRY_CATCH
	try{
#endif

	ZeroMemory( stFile.atFileName, sizeof(stFile.atFileName) );
	stFile.dModify   = FALSE;
	stFile.dNowPage  = 0;
	stFile.dUnique   = gdNextNumber++;
	stFile.stCaret.x = 0;
	stFile.stCaret.y = 0;

	ZeroMemory( stFile.atDummyName, sizeof(stFile.atDummyName) );
	StringCchPrintf( stFile.atDummyName, MAX_PATH, TEXT("%s%d.%s"), NAME_DUMMY_NAME, stFile.dUnique, NAME_DUMMY_EXT );

	if( ptDmyName ){	StringCchCopy( ptDmyName, MAX_PATH, stFile.atDummyName );	}

	stFile.vcCont.clear(  );

	gltMultiFiles.push_back( stFile );

	//	新規作成の準備
	gixFocusPage = -1;

	PageListClear(  );	//	ページリストビューも破棄


	itNew = gltMultiFiles.end( );
	itNew--;	//	末端に追加したからこれでおｋ

	gitFileIt = itNew;	//	ファイルなう

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return  ETC_MSG( ("etc error"), 0 );	}
#endif

	return stFile.dUnique;
}
//-------------------------------------------------------------------------------------------------

/*!
	内容を変更したらタブのファイル名に[変更]つける
	@param[in]	dMode	非０変更した　０変更はなかったことに
	@return		HRESULT	終了状態コード
*/
HRESULT DocMultiFileModify( UINT dMode )
{
	TCHAR	atFile[MAX_PATH];	//!<	ファイル名

	StringCchCopy( atFile, MAX_PATH, (*gitFileIt).atFileName );
	if( 0 == atFile[0] ){	StringCchCopy( atFile, MAX_PATH , (*gitFileIt).atDummyName );	}

	PathStripPath( atFile );

	if( dMode ){	StringCchCat( atFile, MAX_PATH, MODIFY_MSG );	}

	MultiFileTabRename( (*gitFileIt).dUnique, atFile );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ファイルタブを選択した
	@param[in]	uqNumber	選択されたファイルのUNIQUE番号
	@return		HRESULT	終了状態コード
*/
HRESULT DocMultiFileSelect( LPARAM uqNumber )
{
	FILES_ITR	itNow;
	POINT	stCaret;

	for( itNow = gltMultiFiles.begin(); itNow != gltMultiFiles.end(); itNow++ )
	{
		if( uqNumber == itNow->dUnique )	break;
	}
	if( itNow == gltMultiFiles.end() )	return E_OUTOFMEMORY;

	ViewSelPageAll( -1 );	//	今開いてる頁の範囲選択を破棄

	PageListClear(  );	//	ページリストビューも破棄

	gitFileIt = itNow;	//	ファイルなう

//	TODO:	初回読み込み時のバイト数計算間違えてるようだ・なおった？

	PageListBuild( NULL );	//	ページリスト作り直し

	AppTitleChange( itNow->atFileName );	//	キャプションの内容も変更

	gixFocusPage = itNow->dNowPage;

	DocModifyContent( itNow->dModify );	//	変更したかどうか

	DocCaretPosMemory( INIT_LOAD, &stCaret );	//	先に読み出さないと次でクルヤーされる

	PageListViewChange( gixFocusPage,  -1 );	//	全部読み込んだのでラストページを表示する

	ViewPosResetCaret( stCaret.x, stCaret.y );	//	Caret位置再設定

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	全内容を破棄
	@return		HRESULT	終了状態コード
*/
HRESULT DocMultiFileDeleteAll( VOID )
{
	UINT_PTR	i, iPage, iLine;
	FILES_ITR	itNow;
	LINE_ITR	itLine;

	for( itNow = gltMultiFiles.begin( ); itNow != gltMultiFiles.end(); itNow++ )
	{
		iPage = itNow->vcCont.size( );
		for( i = 0; iPage > i; i++ )
		{
			iLine  = itNow->vcCont.at( i ).ltPage.size( );

			itLine = itNow->vcCont.at( i ).ltPage.begin();
			for( itLine = itNow->vcCont.at( i ).ltPage.begin(); itLine != itNow->vcCont.at( i ).ltPage.end(); itLine++ )
			{
				itLine->vcLine.clear( );	//	各行の中身全消し
			}
			itNow->vcCont.at( i ).ltPage.clear( );	//	行を全消し

			SqnFreeAll( &(itNow->vcCont.at( i ).stUndoLog) );
		}
		itNow->vcCont.clear(  );	//	ページを全消し
	}

	gltMultiFiles.clear(  );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ファイルタブを閉じるとき・最後の一つは閉じれないようにするか
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	uqNumber	閉じたいタブの通し番号
	@return		LPARAM		開き直したタブの通し番号・失敗したら０
*/
LPARAM DocMultiFileDelete( HWND hWnd, LPARAM uqNumber )
{
	INT			iRslt;
	UINT_PTR	i, iPage, iLine;
	UINT_PTR	iCount;
	LPARAM	dNowNum, dPrevi;
	FILES_ITR	itNow;
	LINE_ITR	itLine;

	//	一つしか開いてないなら閉じない
	iCount = gltMultiFiles.size();
	if( 1 >= iCount )	return 0;

	//	もし変更が残ってるなら注意を促す
	if( gitFileIt->dModify )
	{
		iRslt = MessageBox( hWnd, TEXT("あぅあぅ！？\r\n変更したままなのですよ！　保存して閉じるのですか？"), TEXT("／(^o^)＼"), MB_YESNOCANCEL | MB_ICONQUESTION );
		if( IDCANCEL == iRslt ){	return 0;	}

		if( IDYES == iRslt ){	DocFileSave( hWnd, D_SJIS );	}
	}

	dNowNum = gitFileIt->dUnique;	//	今開いてるヤツの番号

	itNow = gltMultiFiles.begin( );
	itNow++;	//	次のやつの通し番号を確保しておく。
	dPrevi = itNow->dUnique;

	for( itNow = gltMultiFiles.begin( ); itNow != gltMultiFiles.end(); itNow++ )
	{
		if( uqNumber == itNow->dUnique )	break;
		dPrevi = itNow->dUnique;
	}
	if( itNow == gltMultiFiles.end() )	return 0;
	//	もし削除対象が先頭なら、dPreviは次のやつのまま、次以降なら、直前のが入ってるはず

	//	DocContentsObliterate内のやつ

	iPage = itNow->vcCont.size( );
	for( i = 0; iPage > i; i++ )
	{
		iLine = itNow->vcCont.at( i ).ltPage.size( );

		for( itLine = itNow->vcCont.at( i ).ltPage.begin(); itLine != itNow->vcCont.at( i ).ltPage.end(); itLine++ )
		{
			itLine->vcLine.clear( );	//	各行の中身全消し
		}
		itNow->vcCont.at( i ).ltPage.clear( );	//	行を全消し

#ifdef PAGE_DELAY_LOAD
		FREE( itNow->vcCont.at( i ).ptRawData );
#endif
		SqnFreeAll( &(itNow->vcCont.at( i ).stUndoLog) );
	}
	itNow->vcCont.clear(  );	//	ページを全消し

	gltMultiFiles.erase( itNow );	//	本体を消し

	//	常に選択が有効になるので、常時個々の処理もはいるはず
	if( dNowNum == uqNumber )	//	開いてるのを閉じたら
	{
		gixFocusPage = -1;
		DocMultiFileSelect( dPrevi );	//	有効なのを開き直す
	}

	return dPrevi;
}
//-------------------------------------------------------------------------------------------------

/*!
	開いてるタブをもってくる
	@param[in]	iTgt		読み込みたい番号・負数ならファイル数のみ確保
	@param[in]	ptFile		ファイルパスいれる・MAX_PATHであること
	@param[in]	ptIniPath	INIファイルのパス
	@return		HRESULT	終了状態コード
*/
INT DocMultiFileFetch( INT iTgt, LPTSTR ptFile, LPTSTR ptIniPath )
{
	TCHAR	atKeyName[MIN_STRING];
	INT		iCount;

	assert( ptIniPath );

	iCount = GetPrivateProfileInt( TEXT("MultiOpen"), TEXT("Count"), 0, ptIniPath );
	if( 0 > iTgt )	return iCount;

	assert( ptFile );

	if( iCount <= iTgt ){	ptFile[0] = NULL;	return iCount;	}
	//	オーバーしてたら無効にして終了

	StringCchPrintf( atKeyName, MIN_STRING, TEXT("Item%u"), iTgt );

	GetPrivateProfileString( TEXT("MultiOpen"), atKeyName, TEXT(""), ptFile, MAX_PATH, ptIniPath );

	return iCount;
}
//-------------------------------------------------------------------------------------------------

/*!
	開いてるタブを保存する
	@param[in]	ptIniPath	INIファイルのパス
	@return		HRESULT	終了状態コード
*/
HRESULT DocMultiFileStore( LPTSTR ptIniPath )
{
	TCHAR	atKeyName[MIN_STRING], atBuff[MIN_STRING];
	UINT	i;
	FILES_ITR	itNow;

	assert( ptIniPath );

	//	一旦セクションを空にする
	ZeroMemory( atBuff, sizeof(atBuff) );
	WritePrivateProfileSection( TEXT("MultiOpen"), atBuff, ptIniPath );

	//	ファイルを順次記録
	i = 0;
	for( itNow = gltMultiFiles.begin( ); itNow != gltMultiFiles.end(); itNow++ )
	{
		if( NULL != itNow->atFileName[0] )
		{
			StringCchPrintf( atKeyName, MIN_STRING, TEXT("Item%u"), i );
			WritePrivateProfileString( TEXT("MultiOpen"), atKeyName, itNow->atFileName, ptIniPath );
			i++;
		}
	}

	//	個数を記録
	StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), i );
	WritePrivateProfileString( TEXT("MultiOpen"), TEXT("Count"), atBuff, ptIniPath );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	対象ファイルの名前をゲッツ！する
	@param[in]	tabNum	名前を知りたいヤツのタブ番号
	@param[out]	pIsDmy	名無しであるかどうかを返す・名無しなら非０
	@return		LPTSTR	名前バッファのポインター・無効ならNULLを返す
*/
LPTSTR DocMultiFileNameGet( INT tabNum )
{
	INT	i;
	FILES_ITR	itNow;

	//	ヒットするまでサーチ
	for( i = 0, itNow = gltMultiFiles.begin(); itNow != gltMultiFiles.end(); i++, itNow++ )
	{
		if( tabNum == i )	break;
	}
	if( itNow == gltMultiFiles.end() )	return NULL;	//	ヒット無し・アリエナーイ

	//	名無しならダミー名
	if( NULL == itNow->atFileName[ 0] ){	return itNow->atDummyName;	}

	return itNow->atFileName;	//	ファイル名戻す
}
//-------------------------------------------------------------------------------------------------

/*!
	Caret位置を常時記録・ファイル切り替えたときに意味がある
	@param[in]		dMode	非０ロード　０セーブ
	@param[in,out]	pstPos	Caret位置、ドット、行数
*/
VOID DocCaretPosMemory( UINT dMode, LPPOINT pstPos )
{
	if( dMode )	//	ロード
	{
		pstPos->x = gitFileIt->stCaret.x;
		pstPos->y = gitFileIt->stCaret.y;
	}
	else	//	セーブ
	{
		gitFileIt->stCaret.x = pstPos->x;
		gitFileIt->stCaret.y = pstPos->y;
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	編集中のを破棄して新しいの作る・新しいファイルを開く
	@param[in]	hWnd	親にするウインドウハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT DocOpenFromNull( HWND hWnd )
{
	LPARAM	dNumber;

	TCHAR	atDummyName[MAX_PATH];
	//	複数ファイル扱うなら、破棄は不要、新しいファイルインスタンス作って対応

	//	新しいファイル置き場の準備
	dNumber = DocMultiFileCreate( atDummyName );

	MultiFileTabAppend( dNumber, (*gitFileIt).atDummyName );	//	ファイルの新規作成した

	AppTitleChange( atDummyName );

	gixFocusPage = DocPageCreate( -1 );
	PageListInsert( gixFocusPage  );	//	ページリストビューに追加
	DocPageChange( 0 );

	ViewRedrawSetLine( -1 );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ファイル閉じる前に変更を確認
	@param[in]	hWnd	親にするウインドウハンドル
	@param[in]	dMode	非０閉じるメッセージあり　０変更無かったら素通り
	@return	１閉じておｋ　０ダメ
*/
INT DocFileCloseCheck( HWND hWnd, UINT dMode )
{
	INT		rslt, ret;

	TCHAR	atMessage[BIG_STRING];
	BOOLEAN	bMod = FALSE;
	FILES_ITR	itFiles;

	//	未保存のファイルをチェキ
	for( itFiles = gltMultiFiles.begin(); itFiles != gltMultiFiles.end(); itFiles++ )
	{
		if( itFiles->dModify )
		{
			StringCchPrintf( atMessage, BIG_STRING, TEXT("あぅあぅ！？\r\n%s は保存してないのです！\r\nここで保存するのですか？"), itFiles->atFileName[0] ? PathFindFileName( itFiles->atFileName ) : itFiles->atDummyName );
			rslt = MessageBox( hWnd, atMessage, TEXT("／(^o^)＼"), MB_YESNOCANCEL | MB_ICONQUESTION );
			if( IDCANCEL ==  rslt ){	return 0;	}	//	キャンセルなら終わること自体とりやめ
			if( IDYES == rslt ){	DocFileSave( hWnd, D_SJIS );	}	//	保存するならセーブを呼ぶ
			//	NOなら何もせず次を確認
			bMod = TRUE;	//	未保存があった
		}
	}

	if( !(bMod) )	//	未保存がなかったなら確認メッセージ
	{
		rslt = MessageBox( hWnd, TEXT("あぅ？\r\n終わるのですか？"), TEXT("＼(^o^)／"), MB_YESNO | MB_ICONQUESTION );
		if( IDYES == rslt ){	ret = 1;	}
		else{					ret = 0;	}
	}

#if 0
	if( bMod )	//	更新がのこってる＝保存されてないなら
	{
		rslt = MessageBox( hWnd, TEXT("あぅあぅ！？\r\n保存してないファイルがあるのですよ！\r\n終わっちゃっていいのですか？"), TEXT("／(^o^)＼"), MB_YESNO | MB_ICONQUESTION );
		if( IDYES == rslt ){	ret = 1;	}
		else{	ret = 0;	}
	}
	else
	{
		if( dMode )
		{
			rslt = MessageBox( hWnd, TEXT("あぅ？\r\n終わるのですか？"), TEXT("＼(^o^)／"), MB_YESNO | MB_ICONQUESTION );
			if( IDYES == rslt ){	ret = 1;	}
			else{					ret = 0;	}
		}
		else
		{
			ret = 1;
		}
	}
#endif

	return ret;
}
//-------------------------------------------------------------------------------------------------

/*!
	ファイルを確保
	@param[in]	ptFileName	指定されたファイル名で開く
	@return		LPARAM	０失敗　１～成功
*/
LPARAM DocFileInflate( LPTSTR ptFileName )
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

	LPARAM	dNumber;

#ifdef _DEBUG
	DWORD	dTcStart, dTcEnd;
#endif

	//TCHAR	atBuff[10];
	//ZeroMemory( atBuff, sizeof(atBuff) );

#ifdef _DEBUG
	dTcStart = GetTickCount(  );
#endif
	assert( ptFileName );	//	ファイル開けないのはバグ

	//	ファイル名が空っぽだったら自動的にアウツ！
	if( NULL == ptFileName[0] ){	return 0;	}

	//	レッツオーポン
	hFile = CreateFile( ptFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile ){	return 0;	}

	//InitLastOpen( INIT_SAVE, ptFileName );	//	複数ファイルでは意味が無い

	//	新しいファイル置き場の準備
	dNumber = DocMultiFileCreate( NULL );
	if( 0 >= dNumber )	return 0;

	iByteSize = GetFileSize( hFile, NULL );
	pBuffer = malloc( iByteSize + 2 );
	ZeroMemory( pBuffer, iByteSize + 2 );

	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	ReadFile( hFile, pBuffer, iByteSize, &readed, NULL );
	CloseHandle( hFile );	//	内容全部取り込んだから開放

	StringCchCopy( (*gitFileIt).atFileName, MAX_PATH, ptFileName );

	//	ユニコードチェック
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

	//	ファイルの内容を全確認して、先にメモリ確保しておく？


	//	もしASTなら、先頭は[AA]になってるはず・分割は中でやる
	if( StrCmpN( AST_SEPARATERW, ptString, 4 ) )
	{
		DocStringSplitMLT( ptString , cchSize, DocPageLoad );
	}
	else
	{
		DocStringSplitAST( ptString , cchSize, DocPageLoad );
	}

	//	ファイル開いたらキャレットとかスクロールをリセットする
	ViewEditReset(  );

	FREE( pBuffer );	//	＝ptString

	DocPageChange( 0  );	//	全部読み込んだので最初のページを表示する
	PageListViewChange( -1, -1 );	//	直前頁リセット

	AppTitleChange( ptFileName );

#ifdef _DEBUG
	dTcEnd = GetTickCount(  );
	TRACE( TEXT("LOAD START[%u]  END[%u]    ELAPSE[%u]"), dTcStart, dTcEnd, (dTcEnd - dTcStart) );
#endif

	return dNumber;
}
//-------------------------------------------------------------------------------------------------

/*!
	頁を作って内容をぶち込む
	@param[in]	ptName	項目の名前・無い時はNULL
	@param[in]	ptCont	項目の内容
	@param[in]	cchSize	内容の文字数
	@return		UINT	特に意味なし
*/
UINT CALLBACK DocPageLoad( LPTSTR ptName, LPTSTR ptCont, INT cchSize )
{
#ifndef PAGE_DELAY_LOAD
	INT	dmyX = 0, dmyY = 0;
#endif

	gixFocusPage = DocPageCreate(  -1 );	//	頁を作成
	PageListInsert( gixFocusPage  );	//	ページリストビューに追加

	//	新しく作ったページにうつる

	if( ptName ){	DocPageNameSet( ptName );	}	//	名前をセットしておく
#ifdef PAGE_DELAY_LOAD
	(*gitFileIt).vcCont.at( gixFocusPage ).ptRawData = (LPTSTR)malloc( (cchSize+2) * sizeof(TCHAR) );
	ZeroMemory( (*gitFileIt).vcCont.at( gixFocusPage ).ptRawData, (cchSize+2) * sizeof(TCHAR) );

	HRESULT hRslt = StringCchCopy( (*gitFileIt).vcCont.at( gixFocusPage ).ptRawData, (cchSize+2), ptCont );

#else
	if( 0 < cchSize )	//	空行でないのなら
	{
		DocStringAdd( &dmyX, &dmyY, ptCont, cchSize );	//	この中で改行とか面倒見る
	}
#endif
	//	再計算しちゃう
	DocPageParamGet( NULL, NULL );	//	DocPageInfoRenew( -1, 0 );

	return 1;
}
//-------------------------------------------------------------------------------------------------

#ifdef FILE_PRELOAD
/*!
	ＭＬＴもしくはＴＸＴの頁数を調べる
	@param[in]	ptStr	分解対象文字列へのポインター
	@param[in]	cchSize	その文字列の文字数
	@return		UINT	頁数
*/
UINT DocPreloadMLT( LPTSTR ptString, INT cchSize )
{
	LPTSTR	ptCaret;	//	読込開始・現在位置
	LPTSTR	ptEnd;		//	ページの末端位置・セパレータの直前
	INT		iLines, iDots, iMozis;
	UINT	dPage;
	UINT_PTR	cchItem;
	BOOLEAN	bLast = FALSE;

	ptCaret = ptString;	//	まずは最初から

	dPage = 0;

	//	始点にはセパレータ無いものとみなす。連続するセパレータは、空白内容として扱う
	do
	{
		ptEnd = StrStr( ptCaret, MLT_SEPARATERW );	//	セパレータを探す
		if( !ptEnd )	//	見つからなかったら＝これが最後なら＝NULL
		{
			ptEnd = ptString + cchSize;
			bLast = TRUE;
		}
		cchItem = ptEnd - ptCaret;	//	WCHAR単位なので計算結果は文字数のようだ
		//	最終頁でない場合は末端の改行分引く
		if( !(bLast) && 0 < cchItem ){	cchItem -=  CH_CRLF_CCH;	}

		dPage++;

		//	頁の情報を確保
		iLines = DocStringInfoCount( ptCaret, cchItem, &iDots, &iMozis );

		ptCaret = NextLineW( ptEnd );	//	セパレータの次の行からが本体

	}while( *ptCaret  );	//	データ有る限りループで探す


	return dPage;
}
//-------------------------------------------------------------------------------------------------
#endif

/*!
	ＭＬＴもしくはＴＸＴなユニコード文字列を受け取って分解しつつページに入れる
	@param[in]	ptStr		分解対象文字列へのポインター
	@param[in]	cchSize		その文字列の文字数
	@param[in]	pfPageLoad	内容を入れるコールバック函数のアレ
	@return		UINT		作成した頁数
*/
UINT DocStringSplitMLT( LPTSTR ptStr, INT cchSize, PAGELOAD pfPageLoad )
{
	LPTSTR	ptCaret;	//	読込開始・現在位置
	LPTSTR	ptEnd;		//	ページの末端位置・セパレータの直前
	UINT	iNumber;	//	通し番号カウント
#ifdef FILE_PRELOAD
	UINT	dPage;		
#endif
	UINT	cchItem;
//	INT		dmyX = 0, dmyY = 0;
	BOOLEAN	bLast = FALSE;

	ptCaret = ptStr;	//	まずは最初から

	iNumber = 0;	//	通し番号０インデックス
	//	始点にはセパレータ無いものとみなす。連続するセパレータは、空白内容として扱う

#ifdef FILE_PRELOAD
	dPage = DocPreloadMLT( ptStr, cchSize );
#endif

	do
	{
		ptEnd = StrStr( ptCaret, MLT_SEPARATERW );	//	セパレータを探す
		if( !ptEnd )	//	見つからなかったら＝これが最後なら＝NULL
		{
			ptEnd = ptStr + cchSize;	//	WCHARサイズで計算おｋ？
			bLast = TRUE;
		}
		cchItem = ptEnd - ptCaret;	//	WCHAR単位なので計算結果は文字数のようだ

		//	最終頁でない場合は末端の改行分引く
		if( !(bLast) && 0 < cchItem )
		{
			cchItem -=  CH_CRLF_CCH;
			ptCaret[cchItem] = 0;
		}

		pfPageLoad( NULL, ptCaret, cchItem );

		iNumber++;

		ptCaret = NextLineW( ptEnd );	//	セパレータの次の行からが本体

	}while( *ptCaret );	//	データ有る限りループで探す

	return iNumber;
}
//-------------------------------------------------------------------------------------------------

/*!
	ＡＳＴなユニコード文字列を受け取って分解しつつページに入れる
	@param[in]	ptStr		分解対象文字列へのポインター
	@param[in]	cchSize		その文字列の文字数
	@param[in]	pfPageLoad	内容を入れるコールバック函数のアレ
	@return		UINT		作成した頁数
*/
UINT DocStringSplitAST( LPTSTR ptStr, INT cchSize, PAGELOAD pfPageLoad )
{
	LPTSTR	ptCaret;	//	読込開始・現在位置
	LPTSTR	ptStart;	//	セパレータの直前
	LPTSTR	ptEnd;
	UINT	iNumber;	//	通し番号カウント
	UINT	cchItem;
	BOOLEAN	bLast;
	TCHAR	atName[MAX_PATH];

	ptCaret = ptStr;	//	まずは最初から

	iNumber = 0;	//	通し番号０インデックス

	bLast = FALSE;



	do	//	とりあえず一番最初はptCaretは[AA]になってる
	{
		ptStart = NextLineW( ptCaret );	//	次の行からが本番

		ptCaret += 5;	//	[AA][
		cchItem = ptStart - ptCaret;	//	名前部分の文字数
		cchItem -= 3;	//	]rn

		ZeroMemory( atName, sizeof(atName) );	//	名前確保
		if( 0 < cchItem )	StringCchCopyN( atName, MAX_PATH, ptCaret, cchItem );

		ptCaret = ptStart;	//	本体部分

		ptEnd = StrStr( ptCaret, AST_SEPARATERW );	//	セパレータを探す
		//	この時点でptEndは次の[AA]をさしてる・もしくはNULL(最後のコマ)
		if( !ptEnd )	//	見つからなかったら＝これが最後なら＝NULL
		{
			ptEnd = ptStr + cchSize;	//	WCHARサイズで計算おｋ？
			bLast = TRUE;
		}
		cchItem = ptEnd - ptCaret;	//	WCHAR単位なので計算結果は文字数のようだ

		if( !(bLast) && 0 < cchItem )	//	最終頁でない場合は末端の改行分引く
		{
			cchItem -= CH_CRLF_CCH;
			ptCaret[cchItem] = 0;
		}

		pfPageLoad( atName, ptCaret, cchItem );

		iNumber++;

		ptCaret = ptEnd;

	}while( *ptCaret );	//	データ有る限りループで探す

	return iNumber;
}
//-------------------------------------------------------------------------------------------------

/*!
	ＡＳＤなＳＪＩＳ文字列を受け取って分解しつつページに入れる
	@param[in]	pcStr		分解対象SJIS文字列へのポインター
	@param[in]	cchSize		その文字列の文字数
	@param[in]	pfPageLoad	内容を入れるコールバック函数のアレ
	@return		UINT		作成した頁数
*/
UINT DocImportSplitASD( LPSTR pcStr, INT cbSize, PAGELOAD pfPageLoad )
{
//ASDなら、SJISのままで0x01,0x01、0x02,0x02を対応する必要がある
//0x01,0x01が改行、0x02,0x02が説明の区切り、アイテム区切りが改行

	LPSTR	pcCaret;	//	読込開始・現在位置
	LPSTR	pcEnd, pcDesc;
	UINT	iNumber;	//	通し番号カウント
	UINT	cbItem, d;
	BOOLEAN	bLast;

	LPTSTR		ptName, ptCont;
	UINT_PTR	cchItem;


	pcCaret = pcStr;	//	まずは最初から

	iNumber = 0;	//	通し番号０インデックス

	bLast = FALSE;


	do	//	とりやえず実行
	{
		pcEnd = NextLineA( pcCaret );	//	次の行までで１アイテム
		//if( !(*pcEnd) )	//	見つからなかったら＝これが最後なら＝NULL
		//{
		//	pcEnd = pcStr + cbSize;	//	CHARサイズで計算おｋ？
		//	bLast = TRUE;
		//}
		//	中身がNULLなだけで、ポインタは有効なので特に位置計算は不要か
		cbItem  = pcEnd - pcCaret;	//	壱行の文字数

		pcDesc = NULL;
		ptName = NULL;
		ptCont = NULL;

		for( d = 0; cbItem > d; d++ )
		{
			if( (0x0D == pcCaret[d]) && (0x0A == pcCaret[d+1]) )	//	末端であるか
			{
				pcCaret[d]   = 0x00;	//	末端なのでNULLにする
				pcCaret[d+1] = 0x00;

				if( pcDesc ){	ptName =  SjisDecodeAlloc( pcDesc );	}

				break;
			}

			//	処理順番注意
			if( (0x01 == pcCaret[d]) && (0x01 == pcCaret[d+1]) )	//	改行であるか
			{
				pcCaret[d]   = 0x0D;	//	￥ｒ
				pcCaret[d+1] = 0x0A;	//	￥ｎ
				d++;	//	変換したので次に進めるのがよい
			}

			if( (0x02 == pcCaret[d]) && (0x02 == pcCaret[d+1]) )	//	アイテムと説明の区切り
			{
				pcDesc = &(pcCaret[d+2]);	//	説明開始位置

				pcCaret[d]   = 0x00;	//	仕切りなのでNULLにする
				pcCaret[d+1] = 0x00;
				d++;	//	変換したので次に進めるのがよい
			}
		}

		ptCont = SjisDecodeAlloc( pcCaret );	//	作っておく
		StringCchLength( ptCont, STRSAFE_MAX_CCH, &cchItem );

		pfPageLoad( ptName, ptCont, cchItem );

		iNumber++;

		FREE( ptCont );
		FREE( ptName );

		pcCaret = pcEnd;

	}while( *pcCaret  );	//	データ有る限りループで探す

	return iNumber;
}
//-------------------------------------------------------------------------------------------------


/*!
	頁名をセットする
	@param[in]	ptName	セットする頁名称へのポインター
	@return		HRESULT	終了状態コード
*/
HRESULT DocPageNameSet( LPTSTR ptName )
{
	StringCchCopy( (*gitFileIt).vcCont.at( gixFocusPage ).atPageName, SUB_STRING, ptName );

	PageListNameSet( gixFocusPage, ptName );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ページ追加処理
	@param[in]	iAdding	この指定ページの次に追加・-1で末端に追加
	@return	INT	新規作成したページ番号
*/
INT DocPageCreate( INT iAdding )
{
	INT_PTR		iTotal, iNext;
	UINT_PTR	iAddPage = 0;
	INT		i;

	ONELINE	stLine;
	ONEPAGE	stPage;

	PAGE_ITR	itPage;

#ifdef DO_TRY_CATCH
	try{
#endif

	ZeroONELINE( &stLine  );	//	新規作成したら、壱行目が０文字な枠を作る

	//	こっちもZeroONEPAGEとかにまとめるか
	ZeroMemory( stPage.atPageName, sizeof(stPage.atPageName) );
//	stPage.dDotCnt = 0;
	stPage.dByteSz = 0;

	stPage.ltPage.clear(  );
	stPage.ltPage.push_back( stLine );	//	１頁の枠を作って

	stPage.dSelLineTop    =  -1;		//	無効は－１を注意
	stPage.dSelLineBottom =  -1;		//	

#ifdef PAGE_DELAY_LOAD
	stPage.ptRawData = NULL;
#endif
	SqnInitialise( &(stPage.stUndoLog) );

	//	今の頁の次に作成
	iTotal = (*gitFileIt).vcCont.size(  );

	if( 0 <= iAdding )
	{
		iNext = iAdding + 1;	//	次の頁
		if( iTotal <= iNext ){	iNext =  -1;	}	//	全頁より多いなら末端指定
	}
	else
	{
		iNext = -1;
	}

	if( 0 >  iNext )	//	末尾に追加
	{
		(*gitFileIt).vcCont.push_back( stPage  );	//	ファイル構造体に追加

		iAddPage = (*gitFileIt).vcCont.size( );
		iAddPage--;	//	末端に追加したんだから、個数数えて－１したら０インデックス番号
	}
	else
	{
		itPage = (*gitFileIt).vcCont.begin(  );
		for( i = 0; iNext >  i; i++ ){	itPage++;	}
		(*gitFileIt).vcCont.insert( itPage, stPage );

		iAddPage = iNext;
	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return  ETC_MSG( ("etc error"), 0 );	}
#endif

	return iAddPage;	//	追加したページ番号
}
//-------------------------------------------------------------------------------------------------

/*!
	頁を削除
	@param[in]	iPage	削除する頁の番号
	@return		HRESULT	終了状態コード
*/
HRESULT DocPageDelete( INT iPage )
{
	INT	i, iNew;
	PAGE_ITR	itPage;

	if( 1 >= (*gitFileIt).vcCont.size( ) )	return E_ACCESSDENIED;

	//	ここでバックアップを？

	//	街頭位置までイテレータをもっていく
	itPage = (*gitFileIt).vcCont.begin(  );
	for( i = 0; iPage > i; i++ ){	itPage++;	}

	SqnFreeAll( &(itPage->stUndoLog)  );	//	アンドゥログ削除
	(*gitFileIt).vcCont.erase( itPage  );	//	さっくり削除
	gixFocusPage = -1;	//	頁選択無効にする

#ifdef PAGE_DELAY_LOAD
	FREE( itPage->ptRawData );
#endif

	PageListDelete( iPage );

	iNew = iPage - 1;	//	削除したら一つ前の頁へ
	if( 0 > iNew )	iNew = 0;

	DocPageChange( iNew );	//	削除したら頁移動

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ページを変更
	@param[in]	dPageNum	変更したい頁番号
	@return		HRESULT	終了状態コード
*/
HRESULT DocPageChange( INT dPageNum )
{
	INT	iPrePage;

#ifdef PAGE_DELAY_LOAD
	INT	dmyX = 0, dmyY = 0;
	UINT_PTR	cchSize;
#endif

	//	今の表示内容破棄とかいろいろある
#ifdef DO_TRY_CATCH
	try{
#endif

	ViewSelPageAll( -1 );	//	範囲選択を破棄

	iPrePage = gixFocusPage;
	gixFocusPage = dPageNum;	//	先に変更して

	(*gitFileIt).dNowPage = dPageNum;	//	記録

#ifdef PAGE_DELAY_LOAD
	if( (*gitFileIt).vcCont.at( gixFocusPage ).ptRawData )
	{
		TRACE( TEXT("PAGE DELAY LOAD [%d]"), dPageNum );

		StringCchLength( (*gitFileIt).vcCont.at( gixFocusPage ).ptRawData, STRSAFE_MAX_CCH, &cchSize );

		//	ここで、本文を読み込む
		if( 0 < cchSize )	//	空行でないのなら
		{
			DocStringAdd( &dmyX, &dmyY, (*gitFileIt).vcCont.at( gixFocusPage ).ptRawData, cchSize );	//	この中で改行とか面倒見る
		}
		//	再計算しちゃう
		DocPageParamGet( NULL, NULL );	//	DocPageInfoRenew( -1, 0 );

		FREE( (*gitFileIt).vcCont.at( gixFocusPage ).ptRawData );
	}
#endif

	PageListViewChange( dPageNum, iPrePage );

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	頁一覧に表示する内容を送る。
	@param[in]	dPage	頁指定・負数なら現在の頁
	@param[in]	bMode	非０ステータスバー表示・０ステータスバー無視
	@return		HRESULT	終了状態コード
*/
HRESULT DocPageInfoRenew( INT dPage, UINT bMode )
{
	UINT_PTR	dLines;
	UINT		dBytes;
	TCHAR		atBuff[SUB_STRING];

	if( 0 > dPage ){	dPage = gixFocusPage;	}


	dBytes = (*gitFileIt).vcCont.at( dPage ).dByteSz;
	
	if( bMode )
	{
		//	バイト数を入れる
		StringCchPrintf( atBuff, SUB_STRING, TEXT("%d Bytes"), dBytes );
		StatusBarSetText( SB_BYTECNT, atBuff );
	}

	dLines = (*gitFileIt).vcCont.at( dPage ).ltPage.size( );

	PageListInfoSet( dPage, dBytes, dLines );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定行のテキストを指定した文字数からゲットする
	ポインタだけ渡してこっちでメモリ確保する。開放は呼んだ側で
	@param[in]	rdLine		対象の行番号・絶対０インデックスか
	@param[in]	iStart		開始文字位置０インデックス・この文字から開始・常時０でよい？
	@param[out]	*pstTexts	文字とスタイルを格納するバッファを作るためのポインターへのポインター・NULLなら必要文字数だけ返す
	@param[out]	pchLen		確保した文字数・NULLターミネータはノーカン・バイトじゃないぞ
	@param[out]	pdFlag		文字列について・普通のとか連続空白とか・NULL不可
	@return					文字列の使用ドット数
*/
INT DocLineDataGetAlloc( INT rdLine, INT iStart, LPLETTER *pstTexts, PINT pchLen, PUINT pdFlag )
{
	INT		iSize, i = 0, j, dotCnt;
	INT_PTR	iCount, iLines;

	//	始点と終点を使えるようにする	//	－１なら末端

	LINE_ITR	itLine;

	iLines = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.size( );
	if( iLines <=  rdLine )	return -1;


	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, rdLine );

	iCount = itLine->vcLine.size( );
	*pdFlag = 0;

	if( 0 == iCount )	//	文字列の中身がない
	{
		*pchLen = 0;
		dotCnt  = 0;
	}
	else
	{
		if( iStart >= iCount )	return 0;	//	通り過ぎた

		iSize = iCount - iStart;	//	文字数を入れる
		//	色換えの必要があるところまでとか、一塊ずつで面倒見るように
		*pchLen = iSize;
		iSize++;	//	NULLターミネータの為に増やす

		if( !pstTexts )	return 0;	//	入れるところないならここで終わる

		*pstTexts = (LPLETTER)malloc( iSize * sizeof(LETTER) );
		if( !( *pstTexts ) ){	TRACE( TEXT("malloc error") );	return 0;	}

		ZeroMemory( *pstTexts, iSize * sizeof(LETTER) );

		dotCnt = 0;
		for( i = iStart, j = 0; iCount > i; i++, j++ )
		{
			(*pstTexts)[j].cchMozi = itLine->vcLine.at( i ).cchMozi;
			(*pstTexts)[j].rdWidth = itLine->vcLine.at( i ).rdWidth;
			(*pstTexts)[j].mzStyle = itLine->vcLine.at( i ).mzStyle;

			dotCnt += itLine->vcLine.at( i ).rdWidth;
		}

		//	末端がspaceかどうか確認
		if( iswspace( itLine->vcLine.at( iCount-1 ).cchMozi ) )
		{	*pdFlag |= CT_LASTSP;	}
	}

	if( iLines - 1 >  rdLine )	*pdFlag |= CT_RETURN;	//	次の行があるなら改行
	else						*pdFlag |= CT_EOF;		//	ないならこの行末端がEOF

	//	改行の状態を確保
	*pdFlag |= itLine->dStyle;

	return dotCnt;
}
//-------------------------------------------------------------------------------------------------


/*!
	ページ全体を確保する・freeは呼んだ方でやる
	@param[in]	bStyle	１ユニコードかシフトJIS
	@param[out]	*pText	確保した領域を返す・ワイド文字かマルチ文字になる・NULLだと必要バイト数を返すのみ
	@return				確保したバイト数・NULLターミネータも含む
*/
INT DocPageGetAlloc( UINT bStyle, LPVOID *pText )
{
	return DocPageTextGetAlloc( gitFileIt, gixFocusPage, bStyle, pText, FALSE );
}
//-------------------------------------------------------------------------------------------------

/*!
	ページ全体を文字列で確保する・freeは呼んだ方でやる
	@param[in]	itFile	確保するファイル
	@param[in]	dPage	確保する頁番号
	@param[in]	bStyle	１ユニコードかシフトJISで、矩形かどうか
	@param[out]	pText	確保した領域を返す・ワイド文字かマルチ文字になる・NULLだと必要バイト数を返すのみ
	@param[in]	bCrLf	末端に改行付けるか
	@return				確保したバイト数・NULLターミネータ含む
*/
INT DocPageTextGetAlloc( FILES_ITR itFile, INT dPage, UINT bStyle, LPVOID *pText, BOOLEAN bCrLf )
{
	UINT_PTR	iLines, iLetters, j, iSize;
	UINT_PTR	i;
	UINT_PTR	cchSize;

#ifdef PAGE_DELAY_LOAD
	LPSTR		pcStr;
#endif

	string	srString;
	wstring	wsString;

	LINE_ITR	itLines;

#ifdef DO_TRY_CATCH
	try{
#endif

	srString.clear( );
	wsString.clear( );

	//	デフォ的な
	if( 0 > dPage ){	dPage = gixFocusPage;	}


#ifdef PAGE_DELAY_LOAD
	if( itFile->vcCont.at( dPage ).ptRawData )	//	生データ状態なら
	{
		if( bStyle & D_UNI )	//	ユニコードである
		{
			wsString = wstring( itFile->vcCont.at( dPage ).ptRawData );
			if( bCrLf ){	wsString += wstring( CH_CRLFW );	}
		}
		else	//	ShiftJISである
		{
			pcStr = SjisEncodeAlloc( itFile->vcCont.at( dPage ).ptRawData );
			if( pcStr )
			{
				srString = string( pcStr );
				if( bCrLf ){	srString +=  string( CH_CRLFA );	}

				FREE( pcStr );
			}
		}
	}
	else
	{
#endif
		//	全文字を頂く
		iLines = itFile->vcCont.at( dPage ).ltPage.size( );

		for( itLines = itFile->vcCont.at( dPage ).ltPage.begin(), i = 0;
		itLines != itFile->vcCont.at( dPage ).ltPage.end();
		itLines++, i++ )
		{
			iLetters = itLines->vcLine.size( );

			for( j = 0; iLetters > j; j++ )
			{
				srString +=  string( itLines->vcLine.at( j ).acSjis );
				wsString += itLines->vcLine.at( j ).cchMozi;
			}

			if( !(1 == iLines && 0 == iLetters) )	//	壱行かつ零文字は空である
			{
				if( iLines > (i+1) )	//	とりあえずファイル末端改行はここでは付けない
				{
					srString +=  string( CH_CRLFA );
					wsString += wstring( CH_CRLFW );
				}
			}
		}

		if( bCrLf )
		{
			srString +=  string( CH_CRLFA );
			wsString += wstring( CH_CRLFW );
		}

#ifdef PAGE_DELAY_LOAD
	}
#endif

	if( bStyle & D_UNI )
	{
		cchSize = wsString.size(  ) + 1;	//	NULLターミネータ
		iSize = cchSize * sizeof(TCHAR);	//	ユニコードなのでバイト数は２倍である

		if( pText )
		{
			*pText = (LPTSTR)malloc( iSize );
			ZeroMemory( *pText, iSize );
			StringCchCopy( (LPTSTR)(*pText), cchSize, wsString.c_str( ) );
		}
	}
	else
	{
		iSize = srString.size( ) + 1;	//	NULLターミネータ

		if( pText )
		{
			*pText = (LPSTR)malloc( iSize );
			ZeroMemory( *pText, iSize );
			StringCchCopyA( (LPSTR)(*pText), iSize, srString.c_str( ) );
		}
	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (INT)ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return (INT)ETC_MSG( ("etc error") , 0 );	}
#endif

	return (INT)iSize;
}
//-------------------------------------------------------------------------------------------------

/*!
	現在頁の指定行を文字列で確保する・freeは呼んだ方でやる・ファイルと頁指定できたほうがいいか
	@param[in]	itFile	確保するファイル
	@param[in]	dPage	確保する頁番号
	@param[in]	bStyle	ユニコードかシフトJIS
	@param[in]	dTarget	行番号０インデックス
	@param[out]	pText	確保した領域を返す・ワイド文字かマルチ文字になる・NULLだと必要バイト数を返すのみ
	@return		確保したバイト数・NULLターミネータ含む
*/
INT DocLineTextGetAlloc( FILES_ITR itFile, INT dPage, UINT bStyle, UINT dTarget, LPVOID *pText )
{
	UINT_PTR	dLines;
	UINT_PTR	dLetters, j, iSize;
	UINT_PTR	cchSize;

	string	srString;
	wstring	wsString;

	LINE_ITR	itLines;

	//	位置確認
	dLines = itFile->vcCont.at( dPage ).ltPage.size( );
	if( dLines <= dTarget ){	return 0;	}

	itLines = itFile->vcCont.at( dPage ).ltPage.begin();
	std::advance( itLines, dTarget );

	dLetters = itLines->vcLine.size( );
	for( j = 0; dLetters > j; j++ )
	{
		srString +=  string( itLines->vcLine.at( j ).acSjis );
		wsString +=  itLines->vcLine.at( j ).cchMozi;
	}


	if( bStyle & D_UNI )
	{
		cchSize = wsString.size(  ) + 1;	//	NULLターミネータ
		iSize = cchSize * sizeof(TCHAR);	//	ユニコードなのでバイト数は２倍である

		if( pText )
		{
			*pText = (LPTSTR)malloc( iSize );
			ZeroMemory( *pText, iSize );
			StringCchCopy( (LPTSTR)(*pText), cchSize, wsString.c_str( ) );
		}
	}
	else
	{
		iSize = srString.size( ) + 1;	//	NULLターミネータ

		if( pText )
		{
			*pText = (LPSTR)malloc( iSize );
			ZeroMemory( *pText, iSize );
			StringCchCopyA( (LPSTR)(*pText), iSize, srString.c_str( ) );
		}
	}
	return (INT)iSize;
}
//-------------------------------------------------------------------------------------------------

#if 0
/*!
	ページ全体を確保する・freeは呼んだ方でやる
	@param[in]	bStyle	１ユニコードかシフトJIS
	@param[out]	*pText	確保した領域を返す・ワイド文字かマルチ文字になる・NULLだと必要バイト数を返すのみ
	@return				確保したバイト数・NULLターミネータも含む
*/
INT DocPageTextAllGetAlloc( UINT bStyle, LPVOID *pText )
{
	//	SJISの場合は、ユニコード文字は&#dddd;で確保される

	UINT_PTR	iLines, i, iLetters, j;
	UINT_PTR	cchSize;
	INT_PTR		iSize;

#ifdef PAGE_DELAY_LOAD
	LPTSTR		ptData;
	LPSTR		pcStr;
#endif

	string	srString;	//	ユニコード・シフトJISで確保
	wstring	wsString;

	LINE_ITR	itLine;


	srString.clear( );
	wsString.clear( );

#ifdef PAGE_DELAY_LOAD
	if( (*gitFileIt).vcCont.at( gixFocusPage ).ptRawData )	//	生データ状態なら
	{
		ptData = (*gitFileIt).vcCont.at( gixFocusPage ).ptRawData;
		StringCchLength( ptData, STRSAFE_MAX_CCH, &cchSize );

		if( bStyle & D_UNI )	//	ユニコードである
		{
			iSize = (cchSize+1) * sizeof(TCHAR);	//	NULLターミネータ分足す

			if( pText )
			{
				*pText = (LPTSTR)malloc( iSize );
				ZeroMemory( *pText, iSize );
				StringCchCopy( (LPTSTR)(*pText), cchSize, ptData );
			}
		}
		else
		{
			pcStr = SjisEncodeAlloc( ptData );
			if( pcStr )
			{
				StringCchLengthA( pcStr, STRSAFE_MAX_CCH, &cchSize );
				iSize = cchSize + 1;	//	NULLターミネータ分足す

				if( pText ){	*pText =  pcStr;	}
				else{	FREE( pcStr );	}
			}
		}
	}
	else
	{
#endif
		//	ページ全体の行数
		iLines = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.size( );

		itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();

		for( i = 0; iLines > i; i++, itLine++ )
		{
			//	各行の文字数
			iLetters = itLine->vcLine.size( );

			if( bStyle & D_UNI )
			{
				for( j = 0; iLetters > j; j++ )
				{
					wsString += itLine->vcLine.at( j ).cchMozi;
				}

				if( iLines > (i+1) )	wsString += wstring( CH_CRLFW );
			}
			else
			{
				for( j = 0; iLetters > j; j++ )
				{
					srString +=  string( itLine->vcLine.at( j ).acSjis );
				}

				if( iLines > (i+1) )	srString +=  string( CH_CRLFA );
			}
		}

		if( bStyle & D_UNI )	//	ユニコードである
		{
			cchSize = wsString.size(  ) + 1;	//	NULLターミネータ分足す
			iSize = cchSize * sizeof(TCHAR);	//	ユニコードなのでバイト数は２倍である

			if( pText )
			{
				*pText = (LPTSTR)malloc( iSize );
				ZeroMemory( *pText, iSize );
				StringCchCopy( (LPTSTR)(*pText), cchSize, wsString.c_str( ) );
			}
		}
		else
		{
			iSize = srString.size( ) + 1;	//	NULLターミネータ分足す

			if( pText )
			{
				*pText = (LPSTR)malloc( iSize );
				ZeroMemory( *pText, iSize );
				StringCchCopyA( (LPSTR)(*pText), iSize, srString.c_str( ) );
			}
		}
#ifdef PAGE_DELAY_LOAD
	}
#endif

	return iSize;
}
//-------------------------------------------------------------------------------------------------
#endif

/*!
	指定されたページ全体をプレビュー用にSJISで確保する・freeは呼んだ方でやる
	@param[in]	iPage	ターゲット頁番号
	@param[out]	pdBytes	確保したバイト数返す・NULLターミネータも含む
	@return				確保した領域・マルチ文字になる
*/
LPSTR DocPageTextPreviewAlloc( INT iPage, PINT pdBytes )
{
	//	SJISの場合は、ユニコード文字は&#dddd;で確保される

	UINT_PTR	iLines, i, iLetters;
	INT_PTR		iSize;
	LPSTR	pcText = NULL;
	CHAR	acEntity[10];

	string	srString;	//	シフトJISで確保
	LINE_ITR	itLine, itLineEnd;
	LETR_ITR	itLtr;

	srString.clear( );

	if( pdBytes )	*pdBytes = 0;

	if( DocRangeIsError( iPage, 0 ) )	return NULL;

	//	ページ全体の行数
	iLines    = (*gitFileIt).vcCont.at( iPage ).ltPage.size( );

	itLine    = (*gitFileIt).vcCont.at( iPage ).ltPage.begin( );
	itLineEnd = (*gitFileIt).vcCont.at( iPage ).ltPage.end( );

	for( i = 0; itLine != itLineEnd; i++, itLine++ )
	{
		//	各行の文字数
		iLetters = itLine->vcLine.size( );

		for( itLtr = itLine->vcLine.begin(); itLtr != itLine->vcLine.end(); itLtr++ )
		{
			//	HTML的にヤバイ文字をエンティティする
			if( HtmlEntityCheck( itLtr->cchMozi, acEntity, 10 ) )
			{
				srString +=  string( acEntity );
			}
			else
			{
				srString +=  string( itLtr->acSjis );
			}
		}

//全行に改行あってかまわない？
		srString +=  string( "<br>" );
	}

	iSize = srString.size( ) + 1;	//	NULLターミネータ分足す

	pcText = (LPSTR)malloc( iSize );
	ZeroMemory( pcText, iSize );
	StringCchCopyA( pcText, iSize, srString.c_str( ) );

	if( pdBytes )	*pdBytes = iSize;

	return pcText;
}
//-------------------------------------------------------------------------------------------------


/*!
	現在のユニコード数値参照の具合に合わせて文字列をチェインジ
	@return		HRESULT	終了状態コード
*/
HRESULT UnicodeRadixExchange( LPVOID pVoid )
{
	INT_PTR	iPage, iLine, iMozi, dP, dL, dM;
	TCHAR	cchMozi;
	CHAR	acSjis[10];

	LINE_ITR	itLine;

	iPage = (*gitFileIt).vcCont.size(  );

	for( dP = 0; iPage >  dP; dP++ )	//	全頁
	{
		iLine = (*gitFileIt).vcCont.at( dP ).ltPage.size(  );

		itLine = (*gitFileIt).vcCont.at( dP ).ltPage.begin();
		for( dL = 0; iLine >  dL; dL++, itLine++ )	//	全行
		{
			iMozi = itLine->vcLine.size(  );

			for( dM = 0; iMozi >  dM; dM++ )	//	全字
			{
				if( itLine->vcLine.at( dM ).mzStyle & CT_CANTSJIS )
				{
					cchMozi = itLine->vcLine.at( dM ).cchMozi;
					if( gbUniRadixHex ){	StringCchPrintfA( acSjis, 10, ("&#x%X;"), cchMozi );	}
					else{					StringCchPrintfA( acSjis, 10, ("&#%d;"),  cchMozi );	}

					StringCchCopyA( itLine->vcLine.at( dM ).acSjis, 10, acSjis );
//	TODO:	バイト数再計算が必要
				}
			}
		}
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ページ分割処理・カーソル位置の次の行から終わりまでを次の頁へ
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	iNow	今の行
	@return		HRESULT	終了状態コード
*/
HRESULT DocPageDivide( HWND hWnd, HINSTANCE hInst, INT iNow )
{
	INT	iDivLine = iNow + 1;
	INT	iLines, mRslt, iNewPage;
//	INT_PTR	iTotal, iNext;
	ONELINE	stLine;
	LINE_ITR	itLine, itEnd;

	ZeroONELINE( &stLine );

	//mRslt = MessageBox( hWnd, TEXT("分割しちゃったら復帰できないのですよ・・・\r\n本当にバラしていいのですか？"), TEXT("確認なのです。あぅあぅ"), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 );
	mRslt = MessageBoxCheckBox( hWnd, hInst, 1 );
	if( IDNO == mRslt ){	return  E_ABORT;	}

//分割は、アンドゥをリセットすべし
//今の頁の該当部分を削除しちゃう

	iLines = DocPageParamGet( NULL, NULL );
	if( iLines <= iDivLine )	return E_OUTOFMEMORY;

	//	今の頁の次に作成
	//iTotal = (*gitFileIt).vcCont.size(  );
	//iNext = gixFocusPage + 1;	//	次の頁
	//if( iTotal <= iNext ){	iNext =  -1;	}	//	全頁より多いなら末端指定

	iNewPage = DocPageCreate( gixFocusPage );	//	新頁
	PageListInsert( iNewPage  );	//	ページリストビューに追加

	//	空の壱行が作られてるので、削除しておく
	(*gitFileIt).vcCont.at( iNewPage ).ltPage.clear(  );

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin( );
	std::advance( itLine, iDivLine );

	itEnd  = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.end( );

	std::copy(	itLine, itEnd, back_inserter( (*gitFileIt).vcCont.at( iNewPage ).ltPage ) );

	(*gitFileIt).vcCont.at( gixFocusPage ).ltPage.erase( itLine, itEnd );

	SqnFreeAll( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog) );	//	アンドゥログ削除

	//	バイト情報とかの取り直し
	DocPageByteCount( gixFocusPage, NULL );
	DocPageInfoRenew( gixFocusPage, TRUE );

	DocPageByteCount( iNewPage, NULL );
	DocPageInfoRenew( iNewPage, FALSE );

	ViewRedrawSetLine( -1 );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ホットキーによる投下の調整
*/
HRESULT DocThreadDropCopy( VOID )
{
	CHAR	acBuf[260];
	TCHAR	atTitle[64], atInfo[256];
	INT	cbSize, maxPage;//, dFocusBuf;
	LPVOID	pcString = NULL;

//	dFocusBuf = gixFocusPage;	//	現在頁を一旦待避させて
//	gixFocusPage = gixDropPage;	//	投下用頁にして

//	cbSize = DocPageTextAllGetAlloc( D_SJIS, &pcString );
	cbSize = DocPageTextGetAlloc( gitFileIt, gixDropPage, D_SJIS, &pcString, FALSE );

//	gixFocusPage = dFocusBuf;	//	終わったら戻す

	TRACE( TEXT("%d 頁をコピー"), gixDropPage );

	DocClipboardDataSet( pcString, cbSize, D_SJIS );

	ZeroMemory( acBuf, sizeof(acBuf) );
	StringCchCopyNA( acBuf, 260, (LPCSTR)pcString, 250 );
	ZeroMemory( atInfo, sizeof(atInfo) );
	MultiByteToWideChar( CP_ACP, 0, acBuf, (INT)strlen(acBuf), atInfo, 256 );

	StringCchPrintf( atTitle, 64, TEXT("%d 頁をコピーしたのです。あぅ"), gixDropPage + 1 );

	NotifyBalloonExist( atInfo, atTitle, NIIF_INFO );

	FREE( pcString );

	gixDropPage++;	//	次の頁へ

	maxPage = DocPageCount(  );
	if( maxPage <= gixDropPage )	gixDropPage = 0;
	//	最終頁までイッたら先頭に戻る


	return S_OK;
}
//-------------------------------------------------------------------------------------------------
