/*! @file
	@brief MLTファイルを読んで、中身をメモリに展開します
	このファイルは MaaCatalogue.cpp です。
	@author	SikigamiHNQ
	@date	2011/06/22
*/
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"

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


面倒すぐるのであとでシンク

【　無き２０文字以上は認識しないとかのロジック必要か
*/


#define MLT_SEPARATER	("[SPLIT]")
#define AST_SEPARATER	("[AA]")



//!	MLTの保持
typedef struct tagAAMATRIX
{
	UINT	ixNum;	//!<	通し番号０インデックス
	DWORD	cbItem;	//!<	AAのバイト数
	LPSTR	pcItem;	//!<	読み込んだAAを保持しておくポインタ・SJIS形式のままでいいか？

	CHAR	acAstName[MAX_STRING];	//!<	ASTファイルの名称を持っておく

} AAMATRIX, *LPAAMATRIX;
//-------------------------------------------------------------------------------------------------

static  vector<AAMATRIX>	gvcArts;	//!<	開いたAAの保持
//-------------------------------------------------------------------------------------------------


DWORD	AacInflateMlt( LPSTR, DWORD );
DWORD	AacInflateAst( LPSTR, DWORD );

UINT	AacTitleCheck( LPAAMATRIX );

LPSTR	NextLine( LPSTR );

LRESULT	CALLBACK AacFavInflate( UINT, UINT, UINT, LPCVOID );

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

//	AaTitleClear(  );
//	ASTはここで展開すればいい
	//	展開処理する
	if( isAST ){	rdCount = AacInflateAst( pcFullBuff, readed );	}
	else{			rdCount = AacInflateMlt( pcFullBuff, readed );	}

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

		pcStart = NextLine( pcCaret );	//	次の行からが本番

		pcCaret += 5;	//	[AA][
		cbItem  = pcStart - pcCaret;	//	名前部分の文字数
		cbItem -= 3;	//	]rn

		//	名前確保
		if( 0 < cbItem )
		{
			StringCchCopyNA( stAAbuf.acAstName, MAX_STRING, pcCaret, cbItem );
			AaTitleAddString( iNumber, stAAbuf.acAstName );
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

		//	内容が【○☆】になってるやつを見出しと見なす？
		AacTitleCheck( &stAAbuf );

		gvcArts.push_back( stAAbuf );	//	ベクターに追加

		iNumber++;

		pcCaret = NextLine( pcEnd );

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

		pcEnd = NextLine( pcCaret );
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

		pcEnd = NextLine( pcCaret );
		//NULLなら、改行無し壱行である
		if( *pcEnd )
		{
			cbSize = pcEnd - pcCaret;	//	改行分注意

			pcOpen = pcEnd;
			pcEnd  = NextLine( pcOpen );	//	次行確認
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
	通し番号を受けて、内容を返す
	@param[in]	iNumber	通し番号０インデックス
	@return	LPSTR	AAの内容を、メモリ確保して渡す。受け取った方で開放する
*/
LPSTR AacAsciiArtGet( DWORD iNumber )
{
	size_t	items;
	LPSTR	pcBuff;

	items = gvcArts.size( );
	if( items <= iNumber )	return NULL;

	pcBuff = (LPSTR)malloc( gvcArts.at( iNumber ).cbItem + 1 );
	ZeroMemory( pcBuff, gvcArts.at( iNumber ).cbItem + 1 );
	CopyMemory( pcBuff, gvcArts.at( iNumber ).pcItem, gvcArts.at( iNumber ).cbItem );

	return pcBuff;
}
//-------------------------------------------------------------------------------------------------

/*!
	MLTの保持内容を全破棄
	@return	HRESULT	終了状態コード
*/
HRESULT AacMatrixClear( VOID )
{
	size_t	szItem, i;

	szItem = gvcArts.size();	//	個数確認して

	//	先に領域を開放
	for( i = 0; szItem > i; i++ )
	{
		free( gvcArts.at( i ).pcItem );
	}

	gvcArts.clear();	//	そして全破棄

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	現在行から、次の行の先頭へ移動
	@param[in]	pt	改行を検索開始するところ
	@return		改行の次の位置
*/
LPSTR NextLine( LPSTR pt )
{
	while( *pt && *pt != 0x0D ){	pt++;	}

	if( 0x0D == *pt )
	{
		pt++;
		if( 0x0A == *pt ){	pt++;	}
	}

	return pt;
}
//-------------------------------------------------------------------------------------------------

/*!
	区分ディレクトリ名を受け取って、ＳＱＬから中身を確保する
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



	
		
