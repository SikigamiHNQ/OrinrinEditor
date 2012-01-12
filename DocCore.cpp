/*! @file
	@brief ドキュメントの内容の管理をします
	このファイルは DocCore.cpp です。
	@author	SikigamiHNQ
	@date	2011/04/30
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
/*
	ファイルの管理する構造体を用意・それをリストでつないでおけば複数ファイルになる
	中身を、壱行毎にリストで管理、各行の内容をこれまた管理する
	内容はユニコードで管理するか・見た目は変わらないはず

	文字数は、キャレットの左側の文字数で数える。キャレットが左端なら０文字目

*/

//	TODO:	複数ファイルを同時に保持できるようにする

//-------------------------------------------------------------------------------------------------

//	構造体宣言はコモンへ移動

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


EXTERNED list<ONEFILE>	gltMultiFiles;	//!<	複数ファイル保持
//イテレータのtypedefはヘッダへ

static LPARAM	gdNextNumber;	//!<	開いたファイルの通し番号・常にインクリ

EXTERNED FILES_ITR	gitFileIt;	//!<	今見てるファイルの本体
#define gstFile	(*gitFileIt)	//!<	イテレータを構造体と見なす

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
HRESULT DocInitialise( LPVOID pVoid )
{
	gdNextNumber = 1;

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	文字存在の範囲外エラーが発生していないか
	@param[in]	iPage	チェックしたい頁番号
	@param[in]	iLine	チェックしたい行番号
	@return	BOOLEAN		非０範囲外エラー　０問題無し
*/
BOOLEAN DocRangeIsError( INT iPage, INT iLine )
{
	INT_PTR	iSize;

	if( 0 > iPage || 0 > iLine )	return TRUE;

	iSize = gstFile.vcCont.size( );
	if( 0 >= iSize || iPage >= iSize )	return TRUE;

	iSize = gstFile.vcCont.at( iPage ).vcPage.size( );
	if( 0 >= iSize || iLine >= iSize )	return TRUE;

	return FALSE;
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
		if( gstFile.dModify )	return S_FALSE;
		//	変更のとき、已に変更の処理してたら何もしなくて良い

		StatusBarSetText( SB_MODIFY, MODIFY_MSG );
	}
	else
	{
		StatusBarSetText( SB_MODIFY, TEXT("") );
	}

	DocMultiFileModify( dMode );

	gstFile.dModify =  dMode;	//	ここで記録しておく

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

	StringCchCopy( atFile, MAX_PATH, gstFile.atFileName );
	if( 0 == atFile[0] ){	StringCchCopy( atFile, MAX_PATH , gstFile.atDummyName );	}

	PathStripPath( atFile );

	if( dMode ){	StringCchCat( atFile, MAX_PATH, MODIFY_MSG );	}

	MultiFileTabRename( gstFile.dUnique, atFile );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ファイルタブを選択した
	@param[in]	uqNumber	選択されたタブの通し番号
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

	AppTitleChange( itNow->atFileName );

	gixFocusPage = itNow->dNowPage;

	DocModifyContent( itNow->dModify );	//	変更したかどうか

	DocCaretPosMemory( INIT_LOAD, &stCaret );	//	先に読み出さないと次でクルヤーされる

	PageListViewChange( gixFocusPage );	//	全部読み込んだのでラストページを表示する

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
	UINT_PTR	i, j, iPage, iLine;
	FILES_ITR	itNow;

	for( itNow = gltMultiFiles.begin( ); itNow != gltMultiFiles.end(); itNow++ )
	{
		iPage = itNow->vcCont.size( );
		for( i = 0; iPage > i; i++ )
		{
			iLine = itNow->vcCont.at( i ).vcPage.size( );
			for( j = 0; iLine > j; j++ )
			{
				itNow->vcCont.at( i ).vcPage.at( j ).vcLine.clear(   );	//	各行の中身全消し
			}
			itNow->vcCont.at( i ).vcPage.clear(  );	//	行を全消し
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
	UINT_PTR	i, j, iPage, iLine;
	UINT_PTR	iCount;
	LPARAM	dNowNum, dPrevi;
	FILES_ITR	itNow;

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
		iLine = itNow->vcCont.at( i ).vcPage.size( );
		for( j = 0; iLine > j; j++ )
		{
			itNow->vcCont.at( i ).vcPage.at( j ).vcLine.clear(   );	//	各行の中身全消し
		}
		itNow->vcCont.at( i ).vcPage.clear(  );	//	行を全消し
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

	MultiFileTabAppend( dNumber, gstFile.atDummyName );

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

	BOOLEAN	bMod = FALSE;
	FILES_ITR	itFiles;

	//	未保存のファイルをチェキ
	for( itFiles = gltMultiFiles.begin(); itFiles != gltMultiFiles.end(); itFiles++ )
	{
		if( itFiles->dModify )
		{
			bMod = TRUE;
			break;
		}
	}

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

	//TCHAR	atBuff[10];
	//ZeroMemory( atBuff, sizeof(atBuff) );

	assert( ptFileName );	//	ファイル開けないのはバグ

	//	ファイル名が空っぽだったら自動的にアウツ！
	if( NULL == ptFileName[0] ){	return 0;	}

	//	レッツオーポン
	hFile = CreateFile( ptFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile ){	return 0;	}

	InitLastOpen( INIT_SAVE, ptFileName );	//	複数ファイルでは意味が無い

	//	新しいファイル置き場の準備
	dNumber = DocMultiFileCreate( NULL );

	iByteSize = GetFileSize( hFile, NULL );
	pBuffer = malloc( iByteSize + 2 );
	ZeroMemory( pBuffer, iByteSize + 2 );

	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	ReadFile( hFile, pBuffer, iByteSize, &readed, NULL );
	CloseHandle( hFile );	//	内容全部取り込んだから開放

	StringCchCopy( gstFile.atFileName, MAX_PATH, ptFileName );

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

/*
	//	拡張子を確認・ドット込みだよ～ん
	ptExten = PathFindExtension( ptFileName );	//	拡張子が無いならNULL、というか末端になる
	if( 0 == *ptExten )
	{
		//	拡張子指定がないなら
	}
	else	//	既存の拡張子があったら
	{
		StringCchCopy( atExBuf, 10, ptExten );
		CharLower( atExBuf );	//	比較のために小文字にしちゃう

		//	既存の拡張子が、ASTならそれを優先する
		if( !( StrCmp( atExBuf, aatExte[0] ) ) )	//	ASTであるなら
*/

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

	AppTitleChange( ptFileName );
	

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
	INT	dmyX = 0, dmyY = 0;

	gixFocusPage = DocPageCreate(  -1 );	//	頁を作成
	PageListInsert( gixFocusPage  );	//	ページリストビューに追加

	//	新しく作ったページにうつる

	if( ptName ){	DocPageNameSet( ptName );	}	//	名前をセットしておく

	if( 0 < cchSize )	//	空行でないのなら
	{
		DocStringAdd( &dmyX, &dmyY, ptCont, cchSize );	//	この中で改行とか面倒見る
	}

	//	再計算しちゃう
	DocPageParamGet( NULL, NULL );	//	DocPageInfoRenew( -1, 0 );

	return 1;
}
//-------------------------------------------------------------------------------------------------

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
	UINT	cchItem;
//	INT		dmyX = 0, dmyY = 0;
	BOOLEAN	bLast;

	ptCaret = ptStr;	//	まずは最初から

	iNumber = 0;	//	通し番号０インデックス
	//	始点にはセパレータ無いものとみなす。連続するセパレータは、空白内容として扱う

	bLast = FALSE;

	do
	{
		ptEnd = StrStr( ptCaret, MLT_SEPARATERW );	//	セパレータを探す
		if( !ptEnd )	//	見つからなかったら＝これが最後なら＝NULL
		{
			ptEnd = ptStr + cchSize;	//	WCHARサイズで計算おｋ？
			bLast = TRUE;
		}
		cchItem = ptEnd - ptCaret;	//	WCHAR単位なので計算結果は文字数のようだ

		if( !(bLast) && 0 < cchItem )	//	最終頁でない場合は末端の改行分引く
		{
			cchItem -= CH_CRLF_CCH;
		}

		pfPageLoad( NULL, ptCaret, cchItem );

		iNumber++;

		ptCaret = NextLineW( ptEnd );

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
	StringCchCopy( gstFile.vcCont.at( gixFocusPage ).atPageName, SUB_STRING, ptName );

	PageListNameSet( gixFocusPage, ptName );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ページ追加処理
	@param[in]	iAdding	この指定ページの次に追加・-1で末端に追加
	@return		UINT	新規作成したページ番号
*/
UINT DocPageCreate( INT iAdding )
{
	INT_PTR		iTotal, iNext;
	UINT_PTR	iAddPage = 0;
	INT		i;

	ONELINE	stLine;
	ONEPAGE	stPage;

	PAGE_ITR	itPage;


	ZeroONELINE( &stLine  );	//	新規作成したら、壱行目が０文字な枠を作る

	//	こっちもZeroONEPAGEとかにまとめるか
	ZeroMemory( stPage.atPageName, sizeof(stPage.atPageName) );
//	stPage.dDotCnt = 0;
	stPage.dByteSz = 0;
	stPage.vcPage.clear(  );
	stPage.vcPage.push_back( stLine );	//	１頁の枠を作って
	stPage.dSelLineTop    =  -1;		//	無効は－１を注意
	stPage.dSelLineBottom =  -1;		//	

	SqnInitialise( &(stPage.stUndoLog) );

	//	今の頁の次に作成
	iTotal = gstFile.vcCont.size(  );

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
		gstFile.vcCont.push_back( stPage  );	//	ファイル構造体に追加

		iAddPage = gstFile.vcCont.size( );
		iAddPage--;	//	末端に追加したんだから、個数数えて－１したら０インデックス番号
	}
	else
	{
		itPage = gstFile.vcCont.begin(  );
		for( i = 0; iNext >  i; i++ ){	itPage++;	}
		gstFile.vcCont.insert( itPage, stPage );

		iAddPage = iNext;
	}

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

	if( 1 >= gstFile.vcCont.size( ) )	return E_ACCESSDENIED;

	//	ここでバックアップを？

	//	街頭位置までイテレータをもっていく
	itPage = gstFile.vcCont.begin(  );
	for( i = 0; iPage > i; i++ ){	itPage++;	}

	SqnFreeAll( &(itPage->stUndoLog) );	//	アンドゥログ削除
	gstFile.vcCont.erase( itPage  );	//	さっくり削除
	gixFocusPage = -1;	//	頁選択無効にする

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
	//	今の表示内容破棄とかいろいろある

	ViewSelPageAll( -1 );	//	範囲選択を破棄

	gixFocusPage = dPageNum;	//	先に変更して

	gstFile.dNowPage = dPageNum;	//	記録

	PageListViewChange( dPageNum );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ONLINE構造体をクルヤーする
	@param[in]	pstLine	クルヤーしたいやつのポインター
	@return	無し
*/
VOID ZeroONELINE( LPONELINE pstLine )
{
//	pstLine->dCaret     = 0;
//	pstLine->dNumber    = 0;
	pstLine->iDotCnt    = 0;
	pstLine->iByteSz    = 0;
	pstLine->dStyle     = 0;
	pstLine->bBadSpace  = FALSE;
	pstLine->vcLine.clear(  );
	pstLine->dFrtSpDot  = 0;
	pstLine->dFrtSpMozi = 0;
//	pstLine->dOffset    = 0;

	return;
}
//-------------------------------------------------------------------------------------------------


/*!
	空白警告ありや
	@param[in]	rdLine	対象の行番号・絶対０インデックスか
	@return	BOOLEAN		非０警告あり　０無し
*/
BOOLEAN DocBadSpaceIsExist( INT rdLine )
{
	//	状態確認
	if( DocRangeIsError( gixFocusPage, rdLine ) ){	return 0;	}
	//	ここの範囲外発生は必然なので特に警告は不要

	return gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).bBadSpace;
}
//-------------------------------------------------------------------------------------------------

/*!
	連続する半角スペース、先頭半角空白が有るかどうかチェキ
	@param[in]	rdLine	対象の行番号・絶対０インデックスか
	@return	UINT		非０警告あり　０無し
*/
UINT DocBadSpaceCheck( INT rdLine )
{
	UINT_PTR	iCount, iRslt;
	BOOLEAN	bWarn;
	TCHAR	ch, chn;
	LTR_ITR	ltrItr, ltrEnd, ltrNext;

//末端空白はDocLineDataGetAllocでも見てる

	//	状態確認
	if( DocRangeIsError( gixFocusPage, rdLine ) )
	{
		TRACE( TEXT("範囲外エラー発生 PAGE[%d], LINE[%d]"), gixFocusPage, rdLine );
		return 0;
	}

	//	文字数確認
	iCount = gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.size( );
	if( 0 == iCount )	//	０なら何も確認することがないので、エラー消して終わり
	{
		//	エラー残ってたら
		if( gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).bBadSpace )
		{
			gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).bBadSpace = 0;
			ViewRedrawSetLine( rdLine );	//	ルーラごと行再描画
		}
		return 0;
	}

	iRslt = 0;

	bWarn = FALSE;
	ltrEnd = gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.end( );

	for( ltrItr = gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.begin(); ltrEnd != ltrItr; ltrItr++ )
	{
		ch = ltrItr->cchMozi;
		//	面倒なので警告は一旦消しておく
		ltrItr->mzStyle &= ~CT_WARNING;

		if( 0xFF < ch ){	bWarn = FALSE;	continue;	}
		//	半角以外なら関係ないので警告消して終わり
		if( isspace( ch ) )
		{
			if( !(bWarn) )	//	まだ無警告だったら
			{
				//	次の文字を確認する
				ltrNext = ltrItr + 1;
				if( ltrNext !=  ltrEnd )	//	次が存在してたら
				{
					chn = ltrNext->cchMozi;
					if( 0xFF >= chn )	//	次が半角で
					{
						if( isspace( chn ) )	//	また空白なら
						{
							ltrItr->mzStyle |= CT_WARNING;
							bWarn = TRUE;	//	次も空白だったら、警告とフラグＯＮ
							iRslt = 1;
						}
					}
				}
			}
			else	//	已に警告出てたらそのまま警告いれちゃう
			{
				ltrItr->mzStyle |= CT_WARNING;
			}
		}
		else{	bWarn = FALSE;	}	//	半角空白以外なら警告モード消して終わり
	}

	//	末端に空白が内かどうか確認
	ltrEnd--;
	if( iswspace( ltrEnd->cchMozi ) ){	iRslt = 1;	}

	//	行頭に半角空白ないか確認
	ch = gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.at( 0 ).cchMozi;
	if( 0xFF >= ch )	//	半角だったら
	{
		if( isspace( ch ) )
		{
			gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.at( 0 ).mzStyle |= CT_WARNING;
			iRslt = 1;
		}
	}

#pragma message ("DocBadSpaceCheck内の更新指令の最適化が必要")

	//	前回までと状況が違っていたら
	if( iRslt != gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).bBadSpace )
	{
//		ViewRedrawSetVartRuler( rdLine );	//	ルーラ再描画
		ViewRedrawSetLine( rdLine );	//	ルーラごと行再描画
	}
	gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).bBadSpace = iRslt;

	return iRslt;
}
//-------------------------------------------------------------------------------------------------

/*!
	現在のファイルの頁数を返す
	@return	INT_PTR	頁数
*/
INT_PTR DocPageCount( VOID )
{
	return gstFile.vcCont.size( );
}
//-------------------------------------------------------------------------------------------------

/*!
	現在のページの総行数と文字数とバイト数を返す・ついでにバイト情報とか更新
	@param[in]	pdMozi	文字数入れるバッファへのポインタ・NULLでも可
	@param[in]	pdByte	バイト数入れるバッファへのポインタ・NULLでも可
	@return	UINT	行数
*/
UINT DocPageParamGet( PINT pdMozi, PINT pdByte )
{
	INT_PTR	iLines, i, dMozis = 0;
	INT		dBytes = 0;

	iLines = gstFile.vcCont.at( gixFocusPage ).vcPage.size( );

	for( i = 0; iLines > i; i++ )
	{
		//	改行のバイト数・2ch、YY＝6byte・したらば＝4byte
		if( 1 <= i )	//	弐行目から改行分追加
		{
			if( gbCrLfCode )	dBytes += YY2_CRLF;
			else				dBytes += STRB_CRLF;
		}

		dMozis += gstFile.vcCont.at( gixFocusPage ).vcPage.at( i ).vcLine.size( );
		dBytes += gstFile.vcCont.at( gixFocusPage ).vcPage.at( i ).iByteSz;
	}

	if( pdMozi )	*pdMozi = dMozis;
	if( pdByte )	*pdByte = dBytes;

	gstFile.vcCont.at( gixFocusPage ).dByteSz = dBytes;

	DocPageInfoRenew( -1, 1 );

	return iLines;
}
//-------------------------------------------------------------------------------------------------

/*!
	該当ページのバイト数と文字数をカウントし直す
	@param[in]	dPage	頁指定・負数なら現在の頁
	@param[out]	pMozi	文字数を入れる・NULL可
*/
INT DocPageByteCount( INT dPage, PINT pMozi )
{
	INT		iBytes, iMozis;
	LINE_ITR	itLine, endLine;
	LTR_ITR		itMozi, endMozi;

	if( 0 > dPage ){	dPage = gixFocusPage;	}

//#error 行数とかの情報もまとめてリニューしたほうがいい

	iBytes = 0;
	iMozis = 0;

	itLine  = gstFile.vcCont.at( dPage ).vcPage.begin();
	endLine = gstFile.vcCont.at( dPage ).vcPage.end();

	for( ; itLine != endLine; itLine++ )
	{
		itMozi  = itLine->vcLine.begin();
		endMozi = itLine->vcLine.end();

		for( ; itMozi != endMozi; itMozi++ )
		{
			iBytes += itMozi->mzByte;
			iMozis++;
		}
	}

	gstFile.vcCont.at( dPage ).dByteSz = iBytes;

	if( pMozi ){	*pMozi = iMozis;	}

	return iBytes;
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


	dBytes = gstFile.vcCont.at( dPage ).dByteSz;
	
	if( bMode )
	{
		//	バイト数を入れる
		StringCchPrintf( atBuff, SUB_STRING, TEXT("%d Bytes"), dBytes );
		StatusBarSetText( SB_BYTECNT, atBuff );
	}

	dLines = gstFile.vcCont.at( dPage ).vcPage.size( );

	PageListInfoSet( dPage, dBytes, dLines );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定範囲の最も長いＸドット数を返す
	@param[in]	dTop	開始行・含む・－１で最初から
	@param[in]	dBottom	終了行・含む・－１で最後まで
	@return	INT		ドット数
*/
INT DocPageMaxDotGet( INT dTop, INT dBottom )
{
	INT		maxDot = 0, thisDot, i;
	UINT_PTR	iLines;

	iLines = gstFile.vcCont.at( gixFocusPage ).vcPage.size( );
	if( 0 > dTop )		dTop = 0;
	if( 0 > dBottom )	dBottom = iLines - 1;

	for( i = dTop; dBottom >= i; i++ )
	{
		thisDot = gstFile.vcCont.at( gixFocusPage ).vcPage.at( i ).iDotCnt;
	//	TRACE( TEXT("MAX:%d THIS:%d"), maxDot, thisDot );
		if( maxDot < thisDot )	maxDot = thisDot;
	}

	return maxDot;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定行の使用ドット数と文字数とバイト数を返す
	@param[in]	rdLine	対象の行番号・絶対０インデックスか
	@param[in]	pdMozi	文字数・NULLでもOK
	@param[in]	pdByte	バイト数・NULLでもOK
	@return	INT		ドット数
*/
INT DocLineParamGet( INT rdLine, PINT pdMozi, PINT pdByte )
{
	INT_PTR	iCount, i, iLines;
	INT		dDotCnt, dByteCnt;

	iLines = gstFile.vcCont.at( gixFocusPage ).vcPage.size( );
	if( iLines <= rdLine )	return -1;


	iCount = gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.size( );

	//	文字数必要なら
	if( pdMozi )	*pdMozi = iCount;

	dDotCnt = 0;
	dByteCnt = 0;
	for( i = 0; iCount > i; i++ )
	{
		dDotCnt += gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.at( i ).rdWidth;

		//	総バイト数
		dByteCnt += gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.at( i ).mzByte;
		
	}

	if( pdByte )	*pdByte = dByteCnt;

	//	自分のサイズいれとく
	gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).iDotCnt = dDotCnt;
	gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).iByteSz = dByteCnt;

	return dDotCnt;
}
//-------------------------------------------------------------------------------------------------

/*!
	行数とドット値を受け取って、その場所の文字数を返す・キャレット位置ずれてたら直す
	@param[in]	pNowDot	今のキャレット・文字区切りになるように位置を書換
	@param[in]	rdLine	対象の行番号・絶対０インデックスか
	@param[in]	round	０：四捨五入的な　正数：次の文字固定　負数：前の文字固定
	@return		文字数
*/
INT DocLetterPosGetAdjust( PINT pNowDot, INT rdLine, INT round )
{
	INT		i, iCount, iLines;	//	INT_PTR
	INT		iLetter;	//	キャレットの左側の文字数
	INT		dDotCnt = 0, dPrvCnt = 0, rdWidth = 0;

	assert( pNowDot );

	//	行のはみ出しと文字数確認
	iLines = DocLineParamGet( rdLine, &iCount, NULL );
	if( 0 > iLines )	return 0;

	for( i = 0, iLetter = 0; iCount > i; i++, iLetter++ )
	{
		if( dDotCnt >= *pNowDot ){	break;	}

		dPrvCnt = dDotCnt;
		rdWidth = gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.at( i ).rdWidth;
		dDotCnt += rdWidth;
	}	//	振り切るようなら末端

	if( dDotCnt != *pNowDot )	//	もしキャレット位置が文字境界じゃなかったら
	{
		if( 0 <  round )	//	絶対次の文字
		{
			*pNowDot = dDotCnt;
		}
		else if( 0 > round )	//	絶対前の文字
		{
			*pNowDot = dPrvCnt;
			iLetter--;	//	一つ前なので
		}
		else	//	近い方で
		{
			//	境界との距離を確認して、近い方に合わせる
			if( (*pNowDot - dPrvCnt) < (dDotCnt - *pNowDot ) )
			{
				*pNowDot = dPrvCnt;
				iLetter--;	//	一つ前なので
			}
			else	//	次の文字の法が近い
			{
				*pNowDot = dDotCnt;
			}
		}
	}

	return iLetter;
}
//-------------------------------------------------------------------------------------------------

/*!
	現在位置から１文字前後したときの位置を調べる
	@param[in]	nowDot		今のキャレットのドット位置
	@param[in]	rdLine		対象の行番号・絶対０インデックスか
	@param[in]	bDirect		移動方向　(-)先頭へ　(+)末尾へ
	@param[out]	pdAbsDot	移動後の絶対ドット数を入れる・NULLでも可
	@param[out]	pbJump		改行を超えたらTRUE、超えなかったらFALSE・NULLでも可
	@return		ずれたドット数
*/
INT DocLetterShiftPos( INT nowDot, INT rdLine, INT bDirect, PINT pdAbsDot, PBOOLEAN pbJump )
{
	INT_PTR	iCount, iLetter, iLines;	//	キャレットの左側の文字数
	INT		dLtrDot = 0;//dDotCnt = 0;

	iLines = gstFile.vcCont.at( gixFocusPage ).vcPage.size( );
	if( iLines <=  rdLine )	return -1;



	if( 0 == bDirect )
	{
		if( pdAbsDot ){	*pdAbsDot = nowDot;	}
		if( pbJump ){	*pbJump = FALSE;	}
		return nowDot;	//	移動方向無しでは意味が無い
	}

	iCount = gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.size( );	//	この行の文字数確認して

	//	文字数カウント
	iLetter = DocLetterPosGetAdjust( &nowDot, rdLine, 0 );

	//	左端で左へ移動、もしくは、右端で右へ移動ということは隣の行への移動になる
	if( ((0 == iLetter) && (0 > bDirect)) || ((iCount <= iLetter) && (0 < bDirect)) )
	{
		if((0 > bDirect) && (0 == rdLine))	return 0;
		//	（左端かつ）先頭行で左へ移動するのなら、改行は発生しない

		if( pbJump ){	*pbJump =  TRUE;	}

		return 0;	//	範囲外なので０にしておく
	}

	if( 0 > bDirect )	//	左(先頭)へ移動
	{
		dLtrDot = gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.at( iLetter-1 ).rdWidth;
		//	直前の文字の幅を確認
		nowDot -= dLtrDot;
	}//	dDotCnt

	if( 0 < bDirect )	//	右(末尾)へ移動
	{
		dLtrDot = gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.at( iLetter ).rdWidth;
		//	直後の文字の幅を確認
		nowDot += dLtrDot;
	}//	dDotCnt

	if( pdAbsDot )	*pdAbsDot = nowDot;//dDotCnt

	return dLtrDot;	//	ドット数戻してＯＫ
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

	assert( pdFlag );
	assert( pchLen );

	iLines = gstFile.vcCont.at( gixFocusPage ).vcPage.size( );
	if( iLines <=  rdLine )	return -1;

	iCount = (gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.size( ) );

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
			(*pstTexts)[j].cchMozi = gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.at( i ).cchMozi;
			(*pstTexts)[j].rdWidth = gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.at( i ).rdWidth;
			(*pstTexts)[j].mzStyle = gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.at( i ).mzStyle;

			dotCnt += gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.at( i ).rdWidth;

		}

		//	末端がspaceかどうか確認
		if( iswspace( gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.at( iCount-1 ).cchMozi ) )
		{	*pdFlag |= CT_LASTSP;	}
	}

	if( iLines - 1 >  rdLine )	*pdFlag |= CT_RETURN;	//	次の行があるなら改行
	else						*pdFlag |= CT_EOF;		//	ないならこの行末端がEOF

	//	改行の状態を確保
	*pdFlag |= gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).dStyle;

	return dotCnt;
}
//-------------------------------------------------------------------------------------------------

/*!
	文字列の行数を数える・ユニコード用
	@param[in]	ptStr	数えたい文字列
	@param[in]	cchSize	文字数
	@return		行数
*/
INT DocLineCount( LPTSTR ptStr, UINT cchSize )
{
	INT	lineCnt = 0;
	LPTSTR	ptEnd, ptCaret;

	ptCaret = ptStr;

	do
	{
		ptEnd = StrStr( ptCaret, CH_CRLFW );	//	改行を探す
		if( !ptEnd )	//	見つからなかったら＝これが最後なら＝NULL
		{
			lineCnt++;
			break;
		}

		ptCaret = NextLineW( ptEnd );

		lineCnt++;

	}while( *ptCaret );	//	データ有る限りループで探す

	TRACE( TEXT("LINE COUNT [%d]"), lineCnt );

	return lineCnt;
}
//-------------------------------------------------------------------------------------------------


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
	INT_PTR		iSize;

	string	srString;	//	ユニコード・シフトJISで確保
	wstring	wsString;

	srString.clear( );
	wsString.clear( );

	//	ページ全体の行数
	iLines = gstFile.vcCont.at( gixFocusPage ).vcPage.size( );

	for( i = 0; iLines > i; i++ )
	{
		//	各行の文字数
		iLetters = gstFile.vcCont.at( gixFocusPage ).vcPage.at( i ).vcLine.size( );

		if( bStyle & D_UNI )
		{
			for( j = 0; iLetters > j; j++ )
			{
				wsString += gstFile.vcCont.at( gixFocusPage ).vcPage.at( i ).vcLine.at( j ).cchMozi;
			}

			if( iLines > (i+1) )	wsString += wstring( CH_CRLFW );
		}
		else
		{
			for( j = 0; iLetters > j; j++ )
			{
				srString +=  string( gstFile.vcCont.at( gixFocusPage ).vcPage.at( i ).vcLine.at( j ).acSjis );
			}

			if( iLines > (i+1) )	srString +=  string( CH_CRLFA );
		}
	}

	if( bStyle & D_UNI )	//	ユニコードである
	{
		iSize = wsString.size( ) + 1;	//	NULLターミネータ分足す
		iSize *= 2;	//	ユニコードなのでバイト数は２倍である

		if( pText )
		{
			*pText = (LPTSTR)malloc( iSize );
			ZeroMemory( *pText, iSize );
			StringCchCopy( (LPTSTR)(*pText), iSize, wsString.c_str( ) );
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

	return iSize;
}
//-------------------------------------------------------------------------------------------------

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
	LTR_ITR		itLtr;

	srString.clear( );

	if( pdBytes )	*pdBytes = 0;

	if( DocRangeIsError( iPage, 0 ) )	return NULL;

	//	ページ全体の行数
	iLines = gstFile.vcCont.at( iPage ).vcPage.size( );

	itLine    = gstFile.vcCont.at( iPage ).vcPage.begin( );
	itLineEnd = gstFile.vcCont.at( iPage ).vcPage.end( );

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

	iPage = gstFile.vcCont.size(  );

	for( dP = 0; iPage >  dP; dP++ )	//	全頁
	{
		iLine = gstFile.vcCont.at( dP ).vcPage.size(  );

		for( dL = 0; iLine >  dL; dL++ )	//	全行
		{
			iMozi = gstFile.vcCont.at( dP ).vcPage.at( dL ).vcLine.size(  );

			for( dM = 0; iMozi >  dM; dM++ )	//	全字
			{
				if( gstFile.vcCont.at( dP ).vcPage.at( dL ).vcLine.at( dM ).mzStyle & CT_CANTSJIS )
				{
					cchMozi = gstFile.vcCont.at( dP ).vcPage.at( dL ).vcLine.at( dM ).cchMozi;
					if( gbUniRadixHex ){	StringCchPrintfA( acSjis, 10, ("&#x%X;"), cchMozi );	}
					else{					StringCchPrintfA( acSjis, 10, ("&#%d;"),  cchMozi );	}

					StringCchCopyA( gstFile.vcCont.at( dP ).vcPage.at( dL ).vcLine.at( dM ).acSjis, 10, acSjis );
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
	//iTotal = gstFile.vcCont.size(  );
	//iNext = gixFocusPage + 1;	//	次の頁
	//if( iTotal <= iNext ){	iNext =  -1;	}	//	全頁より多いなら末端指定

	iNewPage = DocPageCreate( gixFocusPage );	//	新頁
	PageListInsert( iNewPage  );	//	ページリストビューに追加

	//	空の壱行が作られてるので、削除しておく
	gstFile.vcCont.at( iNewPage ).vcPage.clear(  );

	itLine = gstFile.vcCont.at( gixFocusPage ).vcPage.begin( );
	itLine += iDivLine;

	itEnd  = gstFile.vcCont.at( gixFocusPage ).vcPage.end( );

	std::copy(	itLine, itEnd, back_inserter( gstFile.vcCont.at( iNewPage ).vcPage ) );

	gstFile.vcCont.at( gixFocusPage ).vcPage.erase( itLine, itEnd );

	SqnFreeAll( &(gstFile.vcCont.at( gixFocusPage ).stUndoLog) );	//	アンドゥログ削除

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
	INT	cbSize, dFocusBuf, maxPage;
	LPVOID	pcString = NULL;

	dFocusBuf = gixFocusPage;	//	現在頁を一旦待避させて

	gixFocusPage = gixDropPage;	//	投下用頁にして

	cbSize = DocPageTextAllGetAlloc( D_SJIS, &pcString );

	gixFocusPage = dFocusBuf;	//	終わったら戻す

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

