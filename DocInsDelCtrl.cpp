/*! @file
	@brief 文字の追加削除系の面倒みます
	このファイルは DocInsDelCtrl.cpp です。
	@author	SikigamiHNQ
	@date	2011/04/24
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

typedef struct tagPAGENUMINFO
{
	UINT	dStartNum;	//	開始番号
	UINT	bInUnder;	//	非０頁下部　０頁先頭
	UINT	bOverride;	//	行内容に上書
	TCHAR	atStyle[MAX_PATH];	//	書式

} PAGENUMINFO, *LPPAGENUMINFO;
//-------------------------------------------------------------------------------------------------

extern FILES_ITR	gitFileIt;	//	今見てるファイルの本体
//#define gstFile	(*gitFileIt)	//!<	イテレータを構造体と見なす

extern INT		gixFocusPage;	//	注目中のページ・とりあえず０・０インデックス

extern  UINT	gbUniPad;		//	パディングにユニコードをつかって、ドットを見せないようにする
extern  UINT	gbUniRadixHex;	//	ユニコード数値参照が１６進数であるか

extern  UINT	gdRightRuler;	//	右線の位置
//-------------------------------------------------------------------------------------------------

/*
範囲選択中に、ALT左右したら、そこの部分がスライドするとか。壱行選択中のみ？
*/


HRESULT	DocInputReturn( INT, INT );

INT		DocSquareAddPreMod( INT, INT, INT, BOOLEAN );

INT		DocLetterErase( INT, INT, INT );
//-------------------------------------------------------------------------------------------------


//こういうコードは、DLLから使うようにすれば、より精度の良いのとさしかえとか 
/*!
	該当のユニコード文字が、シフトJISに変換出来るかどうか確認
	@param[in]	cchMozi	確認したい文字彝
	@param[out]	pcSjis	変換した結果を入れるバッファへのポインタ・１０バイトとること
	@return				非０転換できる　０無理
*/
BOOLEAN DocIsSjisTrance( TCHAR cchMozi, LPSTR pcSjis )
{
	TCHAR	atMozi[2];
	CHAR	acSjis[10];
	BOOL	bCant = FALSE;
	INT		iRslt;
/*
	シフトJISにできないユニコード文字について
	WideCharToMultiByteで、変換不可が発生した場合のフラグを確認して、
	そいつは「&#dddd;」「&#xhhhh;」に変換するようにする・
*/
	assert( pcSjis );

	atMozi[0] = cchMozi;	atMozi[1] = 0;
	acSjis[0] = 0;	acSjis[1] = 0;	acSjis[2] = 0;

	iRslt = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, atMozi, 1, acSjis, 10, "?", &bCant );

	if( bCant )
	{
	//	TRACE( TEXT("SJIS不可(%d:%X)"), cchMozi, cchMozi );
		if( gbUniRadixHex ){	StringCchPrintfA( acSjis, 10, ("&#x%X;"), cchMozi );	}
		else{					StringCchPrintfA( acSjis, 10, ("&#%d;"),  cchMozi );	}
	}

	StringCchCopyA( pcSjis, 10, acSjis );

	return bCant ? FALSE : TRUE;
}
//-------------------------------------------------------------------------------------------------

/*!
	字のバイト数を確認
	@param[in,out]	pstLet	文字データ入れたり出したり
	@return	INT_PTR	バイト数
*/
INT_PTR DocLetterByteCheck( LPLETTER pstLet )
{
	pstLet->mzByte = strlen( pstLet->acSjis );	//	通常の、もしくはユニコードスタイル

	if( pstLet->mzStyle & CT_CANTSJIS ){	pstLet->mzByte += 4;	}	//	数値参照の先頭の＆は５バイト計算必要

	if( 1 == pstLet->mzByte )	//	１バイトだけど実は違うヤツを探す
	{
		//	半角カタカナ
		if( 0xA1 <= (BYTE)(pstLet->acSjis[0]) && (BYTE)(pstLet->acSjis[0]) <= 0xDF ){	pstLet->mzByte =  2;	}

		//	HTML特殊記号
		else if( '"' == pstLet->acSjis[0] ){	pstLet->mzByte = strlen( ("&quot;") );	}
		else if( '<' == pstLet->acSjis[0] ){	pstLet->mzByte = strlen( ("&lt;") );	}
		else if( '>' == pstLet->acSjis[0] ){	pstLet->mzByte = strlen( ("&gt;") );	}
		else if( '&' == pstLet->acSjis[0] ){	pstLet->mzByte = strlen( ("&amp;") );	}
	}

	return pstLet->mzByte;
}
//-------------------------------------------------------------------------------------------------

/*!
	文字列の改行処理をする
	@param[in]	xDot	キャレットのドット位置
	@param[in]	yLine	対象の行番号・絶対０インデックスか
	@param[in]	bFirst	アンドゥ処理の先頭かどうか
	@return		HRESULT	終了状態コード
*/
HRESULT DocCrLfAdd( INT xDot, INT yLine, BOOLEAN bFirst )
{
	SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, CH_CRLFW, xDot, yLine, bFirst );

	return DocInputReturn( xDot , yLine );
}
//-------------------------------------------------------------------------------------------------

/*!
	指定行のドット位置(キャレット位置)で改行する
	@param[in]	nowDot	今のキャレットのドット位置
	@param[in]	rdLine	対象の行番号・絶対０インデックスか
	@return		HRESULT	終了状態コード
*/
HRESULT DocInputReturn( INT nowDot, INT rdLine )
{
	INT_PTR	iLetter, iLines, iCount;
	ONELINE	stLine;

	LETR_ITR	vcLtrItr, vcLtrEnd;

#ifdef LINE_VEC_LIST
	LINE_ITR	itLine, ltLineItr;

	iLines = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.size( );
#else
	LINE_ITR	vcLineItr;

	iLines = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.size( );
#endif
	if( iLines <= rdLine )	return E_OUTOFMEMORY;

	ZeroONELINE( &stLine );

	//	今の文字位置を確認・現在行なのでずれてはないはず
	iLetter = DocLetterPosGetAdjust( &nowDot, rdLine, 0 );

	//	文字数確認
#ifdef LINE_VEC_LIST
	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, rdLine );

	iCount = itLine->vcLine.size( );
#else
	iCount = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.size( );
#endif

	if( iLetter < iCount )	//	もし行の途中で改行したら？
	{

#ifdef LINE_VEC_LIST
		ltLineItr = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin( );
		std::advance( ltLineItr, (rdLine+1) );

		//	今の行の次の場所に行のデータを挿入
		(*gitFileIt).vcCont.at( gixFocusPage ).ltPage.insert( ltLineItr, stLine );

		//	その行の、文字データの先頭をとる
		ltLineItr = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin( );
		std::advance( ltLineItr, (rdLine+1) );	//	追加した行まで移動

		//	ぶった切った場所を設定しなおして
		itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
		std::advance( itLine, rdLine );

		vcLtrItr  = itLine->vcLine.begin( );
		vcLtrItr += iLetter;	//	今の文字位置を示した
		vcLtrEnd  = itLine->vcLine.end( );	//	末端

		//	その部分を次の行にコピーする
		std::copy( vcLtrItr, vcLtrEnd, back_inserter(ltLineItr->vcLine) );

		//	元の文字列を削除する
		itLine->vcLine.erase( vcLtrItr, vcLtrEnd );
#else
		vcLineItr = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.begin( );
		vcLineItr += (rdLine+1);
		//	今の行の次の場所に行のデータを挿入
		(*gitFileIt).vcCont.at( gixFocusPage ).vcPage.insert( vcLineItr, stLine );

		//	その行の、文字データの先頭をとる
		vcLineItr = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.begin( );
		vcLineItr += (rdLine+1);	//	追加した行まで移動

		//	ぶった切った場所を設定しなおして
		vcLtrItr  = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.begin( );
		vcLtrItr += iLetter;	//	今の文字位置を示した
		vcLtrEnd  = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.end( );	//	末端

		//	その部分を次の行にコピーする
		std::copy( vcLtrItr, vcLtrEnd, back_inserter(vcLineItr->vcLine) );

		//	元の文字列を削除する
		(*gitFileIt).vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.erase( vcLtrItr, vcLtrEnd );
#endif

		//	総ドット数再計算
		DocLineParamGet( rdLine,   NULL, NULL );
		DocLineParamGet( rdLine+1, NULL, NULL );
	}
	else	//	末端で改行した
	{
#ifdef LINE_VEC_LIST
		if( (iLines - 1) == rdLine )	//	EOF的なところ
		{
			(*gitFileIt).vcCont.at( gixFocusPage ).ltPage.push_back( stLine );
		}
		else
		{
			ltLineItr = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin( );
			std::advance( ltLineItr, (rdLine+1) );	//	今の行を示した

			//	次の場所に行のデータを挿入
			(*gitFileIt).vcCont.at( gixFocusPage ).ltPage.insert( ltLineItr, stLine );
		}
#else
		if( (iLines - 1) == rdLine )	//	EOF的なところ
		{
			(*gitFileIt).vcCont.at( gixFocusPage ).vcPage.push_back( stLine );
		}
		else
		{
			vcLineItr = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.begin( );
			vcLineItr += (rdLine+1);	//	今の行を示した

			//	次の場所に行のデータを挿入
			(*gitFileIt).vcCont.at( gixFocusPage ).vcPage.insert( vcLineItr, stLine );
		}
#endif
	}

	DocBadSpaceCheck( rdLine   );	//	ここで空白チェキ
	DocBadSpaceCheck( rdLine+1 );	//	空白チェキ・次の行も確認

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定行のドット位置(キャレット位置)でバックスペース押した
	@param[in]	pdDot	今のキャレットのドット位置の値へのポインター
	@param[in]	pdLine	対象の行番号・絶対０インデックスか
	@return	INT	非０改行あった　０壱行のみ
*/
INT DocInputBkSpace( PINT pdDot, PINT pdLine )
{
	INT_PTR	iLines;
	INT		iLetter, width = 0, neDot, bCrLf = 0;
	INT		dLine = *pdLine;	//	函数内で使う行番号・調整に注意
	TCHAR	ch;


#ifdef LINE_VEC_LIST
	LINE_ITR	itLine;

	iLines = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.size( );
#else
	iLines = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.size( );
#endif
	if( iLines <=  dLine )	return 0;	//	はみ出してたらアウツ！

	//	今の文字位置を確認・現在行なのでずれてはないはず
	iLetter = DocLetterPosGetAdjust( pdDot, dLine, 0 );
	neDot = *pdDot;

//	TRACE( TEXT("後空白[D%d C%d]"), neDot, iLetter );

	if( 0 == iLetter && 0 == dLine )	return 0;	//	先頭かつ最初の行なら、なにもしない

	if( 0 != iLetter )	//	行の先頭でないなら
	{
		iLetter--;	//	キャレット一つ戻す
#ifdef LINE_VEC_LIST
		itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
		std::advance( itLine, dLine );

		width = itLine->vcLine.at( iLetter ).rdWidth;
		ch    = itLine->vcLine.at( iLetter ).cchMozi;
#else
		width = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.at( dLine ).vcLine.at( iLetter ).rdWidth;
		ch    = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.at( dLine ).vcLine.at( iLetter ).cchMozi;
#endif
		*pdDot = neDot - width;	//	文字幅分ドットも戻して
		bCrLf = 0;

		SqnAppendLetter( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ch, *pdDot, dLine, TRUE );
	}
	else	//	行の先頭であるなら
	{
		dLine--;	*pdLine = dLine;	//	前の行に移動して
		neDot = DocLineParamGet( dLine, &iLetter, NULL );
		*pdDot = neDot;	//	CARET位置調整
		bCrLf = 1;

		//	ここでやって問題無いはず
		SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, CH_CRLFW, *pdDot, dLine, TRUE );
	}

	DocLetterErase( *pdDot, dLine, iLetter );

	return bCrLf;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定行のドット位置(キャレット位置)でデリート押した
	@param[in]	xDot	今のキャレットのドット位置
	@param[in]	yLine	対象の行番号・絶対０インデックスか
	@return	INT			非０改行削除・０文字削除
*/
INT DocInputDelete( INT xDot, INT yLine )
{
	INT_PTR	iLines;
	INT		iCount, iLetter, iCrLf;
	TCHAR	ch;

#ifdef LINE_VEC_LIST
	LINE_ITR	itLine;

	iLines = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.size( );
#else
	iLines = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.size( );
#endif
	if( iLines <= yLine )	return 0;	//	はみ出してたらアウツ！

	//	今の文字位置を確認・現在行なのでずれてはないはず
	iLetter = DocLetterPosGetAdjust( &xDot, yLine, 0 );

//	TRACE( TEXT("削除[D%d C%d]"), xDot, iLetter );

	DocLineParamGet( yLine, &iCount, NULL );	//	この行の文字数を斗留

	if( iCount <= iLetter )
	{
		if( iLines <= (yLine+1) )	return 0;	//	完全に末端なら何もしない
		ch = CC_LF;
	}
	else
	{
#ifdef LINE_VEC_LIST
		itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
		std::advance( itLine, yLine );

		ch = itLine->vcLine.at( iLetter ).cchMozi;
#else
		ch = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.at( yLine ).vcLine.at( iLetter ).cchMozi;
#endif
	}

	iCrLf = DocLetterErase( xDot, yLine, iLetter );

	if( iCrLf )
	{
		SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, CH_CRLFW, xDot, yLine, TRUE );
	}
	else
	{
		SqnAppendLetter( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ch, xDot, yLine, TRUE );
	}

	return iCrLf;
}
//-------------------------------------------------------------------------------------------------



/*!
	指定行のドット位置(キャレット位置)で壱文字削除
	@param[in]	xDot	今のキャレットのドット位置の値
	@param[in]	yLine	対象の行番号・絶対０インデックスか
	@param[in]	iLetter	対象の文字位置
	@return	INT			非０改行削除・０文字削除
*/
INT DocLetterErase( INT xDot, INT yLine, INT iLetter )
{
	INT		iCount;

	LETR_ITR	vcLtrItr;
#ifdef LINE_VEC_LIST
	LINE_ITR	itLine;
#endif

	DocLineParamGet( yLine, &iCount, NULL );	//	この行の文字数を斗留

	//	ここからDELETEの処理
	if( iCount > iLetter )	//	末端でないなら、今の文字消せばOK
	{
#ifdef LINE_VEC_LIST
		itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
		std::advance( itLine, yLine );

		vcLtrItr  = itLine->vcLine.begin( );
#else
		vcLtrItr  = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.at( yLine ).vcLine.begin( );
#endif
		vcLtrItr += iLetter;	//	今の文字位置を示した

		DocIterateDelete( vcLtrItr, yLine );
		return 0;
	}
	else
	{
		DocLineCombine( yLine );
		return 1;
	}

//	return 0;	//	制御が渡らないコード
}
//-------------------------------------------------------------------------------------------------

/*!
	指定行の内容を削除する・改行はそのまま
	@param[in]	yLine	対象の行番号
	@param[in]	bFirst	アンドゥの非０初めてのグループ　０続きの処理
	@return		アンドゥ状況：削除してたらFALSE、ナニもしなかったら受けたのスルー
*/
BOOLEAN DocLineErase( INT yLine, BOOLEAN bFirst )
{
	INT		dLines, iMozis, i;
	INT_PTR	cbSize, cchSize;
	LPTSTR	ptBuffer;
	wstring	wsString;
#ifdef LINE_VEC_LIST
	LINE_ITR	itLine;
#endif

	wsString.clear( );

	dLines = DocPageParamGet( NULL, NULL );
	if( dLines <= yLine )	return bFirst;	//	はみ出し確認

	DocLineParamGet( yLine, &iMozis, NULL );	//	指定行の文字数確保

	if( 0 >= iMozis )	return bFirst;	//	文字がないならすること無い

#ifdef LINE_VEC_LIST
	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, yLine );

	for( i = 0; iMozis > i; i++ )	//	全文字を確保
	{
		wsString += itLine->vcLine.at( i ).cchMozi;
	}
#else
	for( i = 0; iMozis > i; i++ )	//	全文字を確保
	{
		wsString += (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.at( yLine ).vcLine.at( i ).cchMozi;
	}
#endif

	cchSize = wsString.size(  ) + 1;	//	NULLターミネータ分足す
	cbSize = cchSize * 2;	//	ユニコードなのでバイト数は２倍である

	ptBuffer = (LPTSTR)malloc( cbSize );
	ZeroMemory( ptBuffer, cbSize );
	StringCchCopy( ptBuffer, cchSize, wsString.c_str(  ) );
	SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ptBuffer, 0, yLine, bFirst );
	bFirst = FALSE;

	//	削除処理
#ifdef LINE_VEC_LIST
	itLine->vcLine.clear();
#else
	(*gitFileIt).vcCont.at( gixFocusPage ).vcPage.at( yLine ).vcLine.clear();
#endif

	DocLineParamGet( yLine, NULL, NULL );	//	再計算
	DocPageParamGet( NULL, NULL );	//	再計算
	DocBadSpaceCheck( yLine );	//	リセットに必要
	ViewRedrawSetLine( yLine );	//	要らないかも

	return bFirst;
}
//-------------------------------------------------------------------------------------------------

/*!
	対象文字のイテレータと行を受けて、その文字を削除する
	@param[in]	itLtr	対象文字のイテレータ
	@param[in]	dBsLine	対象の行番号・絶対０インデックスか
	@return	INT			非０改行削除・０文字削除
*/
INT DocIterateDelete( LETR_ITR itLtr, INT dBsLine )
{
	INT	width = 0, bySz;
#ifdef LINE_VEC_LIST
	LINE_ITR	itLine;
#endif

	width = itLtr->rdWidth;
	bySz  = itLtr->mzByte;

#ifdef LINE_VEC_LIST
	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, dBsLine );

	itLine->vcLine.erase( itLtr );

	itLine->iDotCnt -= width;
	itLine->iByteSz -= bySz;
#else
	(*gitFileIt).vcCont.at( gixFocusPage ).vcPage.at( dBsLine ).vcLine.erase( itLtr );

	(*gitFileIt).vcCont.at( gixFocusPage ).vcPage.at( dBsLine ).iDotCnt -= width;
	(*gitFileIt).vcCont.at( gixFocusPage ).vcPage.at( dBsLine ).iByteSz -= bySz;
#endif
	(*gitFileIt).vcCont.at( gixFocusPage ).dByteSz -= bySz;

	DocBadSpaceCheck( dBsLine );	//	ついでに良くないスペースを調べておく

	return width;
}
//-------------------------------------------------------------------------------------------------

/*!
	対象行の、次の行を、対象行の末尾にくっつける。末端でDELETE操作
	@param[in]	dBsLine	対象の行番号・絶対０インデックス
	@return		特に意味はない
	@return		HRESULT	終了状態コード
*/
HRESULT DocLineCombine( INT dBsLine )
{
	LETR_ITR	vcLtrItr, vcLtrNxItr, vcLtrNxEnd;

#ifdef LINE_VEC_LIST
	LINE_ITR	itLine, itLineNx, ltLineItr;

	itLineNx = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLineNx, dBsLine+1 );
	
	if( itLineNx == (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.end() )	return E_ACCESSDENIED;

	//	選択範囲ある時にアンドゥして、選択範囲が死んでる状態で切り取りするとここで落ちる
	vcLtrNxItr = itLineNx->vcLine.begin(  );	//	次の行の先頭
	vcLtrNxEnd = itLineNx->vcLine.end(  );	//	次の行の尻尾

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, dBsLine );
	std::copy( vcLtrNxItr, vcLtrNxEnd, back_inserter( itLine->vcLine ) );

	DocLineParamGet( dBsLine , NULL, NULL );	//	呼び出せば中で面倒みてくれる

	ltLineItr  = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin(  );
	std::advance( ltLineItr, dBsLine+1 );	//	次の行

	(*gitFileIt).vcCont.at( gixFocusPage ).ltPage.erase( ltLineItr );
#else
	LINE_ITR	vcLineItr;

	vcLtrNxItr = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.at( dBsLine+1 ).vcLine.begin(  );	//	次の行の先頭
	vcLtrNxEnd = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.at( dBsLine+1 ).vcLine.end(  );	//	次の行の尻尾

	std::copy( vcLtrNxItr, vcLtrNxEnd, back_inserter( (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.at( dBsLine ).vcLine ) );

	DocLineParamGet( dBsLine , NULL, NULL );	//	呼び出せば中で面倒みてくれる

	vcLineItr  = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.begin(  );
	vcLineItr += (dBsLine+1);	//	次の行

	(*gitFileIt).vcCont.at( gixFocusPage ).vcPage.erase( vcLineItr );
#endif
	DocBadSpaceCheck( dBsLine );	//	ついでに良くないスペースを調べておく

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定行のドット位置(キャレット位置)に壱文字追加してアンドゥ記録する
	@param[in]	pxDot	挿入するドット位置へのポインター
	@param[in]	yLine	対象の行番号・絶対０インデックスか
	@param[in]	ch		追加したい文字
	@return		INT		追加した文字のドット数
*/
INT DocInsertLetter( PINT pxDot, INT yLine, TCHAR ch )
{
	INT	width;

	SqnAppendLetter( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, ch, *pxDot, yLine, TRUE );

	width = DocInputLetter( *pxDot, yLine, ch );
	*pxDot += width;	//	途中でもいける

	DocBadSpaceCheck( yLine );	//	ここで空白チェキ

	return width;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定行のドット位置(キャレット位置)に壱文字追加する・この函数内ではアンドゥの面倒は見ない
	@param[in]	nowDot	挿入するドット位置
	@param[in]	rdLine	対象の行番号・絶対０インデックスか
	@param[in]	ch		追加したい文字
	@return		INT		追加した文字のドット数
*/
INT DocInputLetter( INT nowDot, INT rdLine, TCHAR ch )
{
	INT_PTR	iLetter, iCount, iLines;
	LETTER	stLetter;
	LETR_ITR	vcItr;

	//	アンドゥリドゥは呼んだところで

#ifdef LINE_VEC_LIST
	LINE_ITR	itLine;

	iLines = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.size( );
#else
	iLines = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.size( );
#endif
	if( iLines <=  rdLine )	return 0;

	//	今の文字位置を確認・現在行なのでずれてはないはず
	iLetter = DocLetterPosGetAdjust( &nowDot, rdLine, 0 );

	//	文字数確認
#ifdef LINE_VEC_LIST
	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, rdLine );

	iCount = itLine->vcLine.size( );
#else
	iCount = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.size( );
#endif
	//	データ作成
	ZeroMemory( &stLetter, sizeof(LETTER) );
	stLetter.cchMozi = ch;
	stLetter.rdWidth = ViewLetterWidthGet( ch );
	stLetter.mzStyle = CT_NORMAL;
	if( iswspace( ch ) )	stLetter.mzStyle |= CT_SPACE;
	//	非シフトJIS文字を確認
	if( !( DocIsSjisTrance(ch,stLetter.acSjis) ) )	stLetter.mzStyle |= CT_CANTSJIS;
	//	文字のバイトサイズ確認
	DocLetterByteCheck( &stLetter );


#ifdef LINE_VEC_LIST
	if( iLetter >=  iCount )	//	文字数同じなら末端に追加ということ
	{
		itLine->vcLine.push_back( stLetter );
	}
	else	//	そうでないなら途中に追加
	{
		vcItr = itLine->vcLine.begin( );
		vcItr += iLetter;
		itLine->vcLine.insert( vcItr, stLetter );
	}

	itLine->iDotCnt += stLetter.rdWidth;
	itLine->iByteSz += stLetter.mzByte;

#else
	if( iLetter >=  iCount )	//	文字数同じなら末端に追加ということ
	{
		(*gitFileIt).vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.push_back( stLetter );
	}
	else	//	そうでないなら途中に追加
	{
		vcItr = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.begin( );
		vcItr += iLetter;
		(*gitFileIt).vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.insert( vcItr, stLetter );
	}

	(*gitFileIt).vcCont.at( gixFocusPage ).vcPage.at( rdLine ).iDotCnt += stLetter.rdWidth;
	(*gitFileIt).vcCont.at( gixFocusPage ).vcPage.at( rdLine ).iByteSz += stLetter.mzByte;
#endif
	(*gitFileIt).vcCont.at( gixFocusPage ).dByteSz += stLetter.mzByte;

//	DocBadSpaceCheck( rdLine );	呼んだところでまとめてやる

	return stLetter.rdWidth;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定行のドット位置で文字数分削除する・改行は弐文字占有
	@param[in]	xDot	今のキャレットのドット位置
	@param[in]	yLine	対象の行番号・絶対０インデックスか
	@param[in]	ptDummy	未使用・NULLで
	@param[in]	cchSize	文字列の文字数・ヌルターミネータ含まず
	@return		INT		０改行無し　１～改行をサクった回数
*/
INT DocStringErase( INT xDot, INT yLine, LPTSTR ptDummy, INT cchSize )
{
	INT	i, dCrLf, iLetter, rdCnt;

	//	今の文字位置・キャレットより末尾方向に削除するので、この位置は変わらない
	iLetter = DocLetterPosGetAdjust( &xDot, yLine, 0 );

	rdCnt = 0;
	for( i = 0; cchSize > i; i++ )	//	DEL連打ってこと
	{
		dCrLf = DocLetterErase( xDot, yLine, iLetter );
		if( dCrLf ){	i++;	rdCnt++;	}
	}

	return rdCnt;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定行のドット位置(キャレット位置)に文字列を追加する・こっちが下位函数
	@param[in]	pNowDot	今のキャレットのドット位置のポインター
	@param[in]	pdLine	対象の行番号・絶対０インデックスか
	@param[in]	ptStr	追加したい文字列
	@param[in]	cchSize	文字列の文字数・ヌルターミネータ含まず
	@return		INT		０改行無し　１～改行した回数
*/
INT DocStringAdd( PINT pNowDot, PINT pdLine, LPTSTR ptStr, INT cchSize )
{
	INT	i, insDot, dLn, dCrLf;

	assert( ptStr );

	dCrLf  = 0;
	dLn    = *pdLine;
	insDot = *pNowDot;

#ifdef DO_TRY_CATCH
	try{
#endif
	for( i = 0; cchSize > i; i++ )
	{
		if( CC_CR == ptStr[i] && CC_LF == ptStr[i+1] )	//	改行であったら
		{
			DocInputReturn( insDot, *pdLine );
			i++;			//	0x0D,0x0Aだから、壱文字飛ばすのがポイント
			(*pdLine)++;	//	改行したからFocusは次の行へ
			insDot =  0;	//	そして行の先頭である
			dCrLf++;		//	改行した回数カウント
		}
		else if( CC_TAB == ptStr[i] )
		{
			//	タブは挿入しない
		}
		else
		{
			insDot += DocInputLetter( insDot, *pdLine, ptStr[i] );
		}
	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (INT)ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return (INT)ETC_MSG( ("etc error"), 0 );	}
#endif


#ifdef DO_TRY_CATCH
	try{
#endif
	//	ここで空白チェキ・開始行から終了行までブンブンする
	for( i = dLn; *pdLine >= i; i++ )
	{
		DocBadSpaceCheck( i );
	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (INT)ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return (INT)ETC_MSG( ("etc error"), 0 );	}
#endif
	//	アンドゥリドゥはここではなく呼んだほうで面倒見るほうがいい

	*pNowDot = insDot;

	return dCrLf;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定行のドット位置(キャレット位置)に文字列を矩形で追加する
	@param[in]	pNowDot	今のキャレットのドット位置のポインター
	@param[in]	pdLine	対象の行番号・絶対０インデックスか
	@param[in]	ptStr	追加したい文字列
	@param[in]	cchSize	文字列の文字数・ヌルターミネータ含まず
	@param[out]	*ppstPt	各挿入場所を入れるバッファのポインタをポインタ
	@return		INT		処理した行数
*/
INT DocSquareAdd( PINT pNowDot, PINT pdLine, LPTSTR ptStr, INT cchSize, LPPOINT *ppstPt )
{
	LPTSTR		ptCaret, ptSprt;
	UINT_PTR	cchMozi;
	INT			dCrLf;
	INT			dBaseDot, dBaseLine;

	LPPOINT	pstBuf;

	dCrLf = 0;

	ptCaret = ptStr;
	dBaseLine = *pdLine;

	do
	{
		dBaseDot  = *pNowDot;
		DocLetterPosGetAdjust( &dBaseDot, dBaseLine, 0 );	//	場所合わせ

		ptSprt = StrStr( ptCaret, CH_CRLFW );	//	改行のところまで
		if( !(ptSprt) ){	ptSprt = ptStr + cchSize;	}
		//	末端まで改行がなかったら、末端文字の位置を入れる
		cchMozi = ptSprt - ptCaret;	//	そこまでの文字数求めて
		
		//	各行の追加場所を覚えておく
		pstBuf = (LPPOINT)realloc( *ppstPt, ( sizeof(POINT) * (dCrLf+1) ) );

		if( pstBuf ){	*ppstPt = pstBuf;	}
		else{	TRACE( TEXT("fatal mem error") );	return 0;	}
		pstBuf += dCrLf;

		pstBuf->x = dBaseDot;
		pstBuf->y = dBaseLine;
		DocStringAdd( &dBaseDot, &dBaseLine, ptCaret, cchMozi );

		ptCaret = NextLineW( ptSprt );	//	次の行の先頭に移動
		if( *ptCaret  ){	dBaseLine++;	}	//	行位置も進める

		dCrLf++;

	}while( *ptCaret  );	//	データ有る限りループで探す

	*pdLine  = dBaseLine;	//	末端位置を書き戻す
	*pNowDot = dBaseDot;

	return dCrLf;
}
//-------------------------------------------------------------------------------------------------

/*!
	現在頁の末端に改行を追加する
	@param[in]	addLine	追加する行数
	@param[in]	bFirst	アンドゥの非０初めてのグループ　０続きの処理
	@return		全体の行数
*/
INT DocAdditionalLine( INT addLine, BOOLEAN bFirst )
{
	UINT_PTR	iLines;
	INT			cbSize, cchMozi, i;
	INT			dBaseDot, dBaseLine;
	LPTSTR		ptBuffer = NULL;

#ifdef LINE_VEC_LIST
	iLines = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.size( );
#else
	iLines = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.size( );
#endif
	//	この頁の行数

	//	追加するのは最終行の末端
	dBaseLine = iLines - 1;

	cchMozi = CH_CRLF_CCH * addLine;	//	改行の文字数＋ぬるたーみねーた
	cbSize  = (cchMozi + 1) * sizeof(TCHAR);
	ptBuffer = (LPTSTR)malloc( cbSize );

	ZeroMemory( ptBuffer, cbSize );
	for( i = 0; addLine >  i; i++ )
	{
		StringCchCat( ptBuffer, cchMozi + 1, CH_CRLFW );
		ViewRedrawSetLine( dBaseLine + i  );	//	再描画行を確定しておく
	}

	dBaseDot = DocLineParamGet( dBaseLine, NULL, NULL );
	SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, ptBuffer, dBaseDot, dBaseLine, bFirst );
	DocStringAdd( &dBaseDot, &dBaseLine, ptBuffer, cchMozi );

	FREE( ptBuffer );

	return iLines;
}
//-------------------------------------------------------------------------------------------------

/*!
	矩形貼付をする前に、	場の状況を確認して、必要なら整形する
	@param[in]	xDot		追加開始するドット位置
	@param[in]	yLine		追加開始する行番号
	@param[in]	dNeedLine	使う行数
	@param[in]	bFirst		アンドゥの非０初めてのグループ　０続きの処理
	@return		INT		非０しなかった　０処理した
*/
INT DocSquareAddPreMod( INT xDot, INT yLine, INT dNeedLine, BOOLEAN bFirst )
{
//	行増やすのと、所定の位置までスペースで埋める
	INT_PTR	iLines;
	INT		iBaseDot, iBaseLine, iMinus, i;
	UINT	cchBuf;
	LPTSTR	ptBuffer = NULL;


	//	この頁の行数
#ifdef LINE_VEC_LIST
	iLines = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.size( );
#else
	iLines = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.size( );
#endif
	//	全体行数より、追加行数が多かったら、改行増やす
	if( iLines < (dNeedLine + yLine) )
	{
		iMinus = (dNeedLine + yLine) - iLines;	//	追加する行数

		DocAdditionalLine( iMinus, bFirst );	bFirst = FALSE;

		//	この頁の行数取り直し
#ifdef LINE_VEC_LIST
		iLines = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.size( );
#else
		iLines = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.size( );
#endif
	}

	//	各行のドット数を確認して、足りないところをパディングする
	//多い分には問題無し・足りないのが３以下なら無視、あとは適当にパディングを作る
	//良い感じの数が無かったら増減して作れるまでさがす
	for( i = 0; dNeedLine > i; i++ )
	{
		iBaseLine = yLine + i;
		iBaseDot  = DocLineParamGet( iBaseLine, NULL, NULL );
		//	基準から存在ドットを引くと、＋なら足りない
		iMinus    = xDot - iBaseDot;

		if( gbUniPad  ){	if( 0 >= iMinus )	continue;	}
		else{	if( 3 >= iMinus )	continue;	}	//	余るか３以下なら気にする必要は無い

		ptBuffer = DocPaddingSpaceMake( iMinus );//DocPaddingSpaceWithPeriod( iMinus, NULL, NULL, NULL, FALSE );
		//if( !ptBuffer )	//	まずは綺麗に納めるのを試みて、ダメならズレありで再計算
		//{	ptBuffer = DocPaddingSpaceWithGap( iMinus, NULL, NULL );	}
		if( !ptBuffer ){	continue;	}
		StringCchLength( ptBuffer, STRSAFE_MAX_CCH, &cchBuf );

		SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, ptBuffer, iBaseDot, iBaseLine, bFirst );
		bFirst = FALSE;
		DocStringAdd( &iBaseDot, &iBaseLine, ptBuffer, cchBuf );

		FREE( ptBuffer );
	}


	return bFirst;
}
//-------------------------------------------------------------------------------------------------

/*!
	文字列（矩形もアリ）を挿入する
	@param[in,out]	pNowDot	今のキャレットのドット位置
	@param[in,out]	pdLine	対象の行番号・絶対０インデックスか
	@param[in,out]	pdMozi	今のキャレットの文字数・NULLでもおｋ
	@param[in]		ptText	挿入する文字列
	@param[in]		dStyle	矩形かどうか、不可視特別か
	@param[in]		bFirst	アンドゥ用・これが最初のアクションか
	@return		０壱行ですんだ　非０複数行に渡った
*/
INT DocInsertString( PINT pNowDot, PINT pdLine, PINT pdMozi, LPTSTR ptText, UINT dStyle, BOOLEAN bFirst )
{
	INT		dBaseDot, dBaseLine, dNeedLine;
	INT		dCrLf, i, dLastLine;
	UINT_PTR	cchSize;
	LPPOINT	pstPoint;

	dBaseDot  = *pNowDot;
	dBaseLine = *pdLine;
	dLastLine = *pdLine;

	if( !(ptText)  )	return 0;

	StringCchLength( ptText, STRSAFE_MAX_CCH, &cchSize );

	if( dStyle &  D_SQUARE )	//	矩形用
	{
		//	使う行数
		dNeedLine = DocStringInfoCount( ptText, cchSize, NULL, NULL );

		bFirst = DocSquareAddPreMod( *pNowDot, *pdLine, dNeedLine, bFirst );
		//	中でアンドゥ追加までやる。

		pstPoint = NULL;	//	NULL化必須
		dCrLf = DocSquareAdd( pNowDot, pdLine, ptText, cchSize, &pstPoint );

		//	貼付前の整形を含めて１Groupとして扱う必要がある
		SqnAppendSquare( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, ptText, pstPoint, dCrLf, bFirst );
		bFirst = FALSE;

		FREE( pstPoint );

		dLastLine = *pdLine;
	}
	else
	{
		//	この中で改行とか面倒見る
		dCrLf = DocStringAdd( pNowDot, pdLine, ptText, cchSize );

		SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, ptText, dBaseDot, dBaseLine, bFirst );
		bFirst = FALSE;

		dLastLine = DocPageParamGet( NULL, NULL );
	}

	if( dCrLf )
	{
		for( i = dBaseLine; dLastLine >= i; i++ ){	ViewRedrawSetLine(  i );	}
	}
	else
	{
		ViewRedrawSetLine( *pdLine );
	}

	if( pdMozi ){	*pdMozi = DocLetterPosGetAdjust( pNowDot, *pdLine , 0 );	}

	//	ヤバイ状態のときは操作しないようにする
	if( !(D_INVISI & dStyle) )	ViewDrawCaret( *pNowDot, *pdLine, TRUE );


	return dCrLf;
}
//-------------------------------------------------------------------------------------------------

/*!
	クリップボードの文字列を挿入する・いわゆる貼り付け
	@param[in,out]	pNowDot	今のキャレットのドット位置
	@param[in,out]	pdLine	対象の行番号・絶対０インデックスか
	@param[in,out]	pdMozi	今のキャレットの文字数
	@return		０壱行ですんだ　非０複数行に渡った
*/
INT DocInputFromClipboard( PINT pNowDot, PINT pdLine, PINT pdMozi )
{
	LPTSTR	ptString = NULL;
	UINT	cchSize, dStyle = 0, i, j;
	INT		dCrLf, dTop, dBtm;
	BOOLEAN	bSelect;
	UINT	dSqSel, iLines;


	//	クリップボードからデータを頂く
	ptString = DocClipboardDataGet( &dStyle );
	if( !(ptString) )
	{
		NotifyBalloonExist( TEXT("有効なデータがないのです。あぅあぅ"), TEXT("貼付られないのです"), NIIF_INFO );
		return 0;
	}

	StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSize );

	//	タブをヌく
	for( i = 0; cchSize > i; )
	{
		if( CC_TAB == ptString[i] )
		{
			for( j = i; cchSize > j; j++ )
			{
				ptString[j] = ptString[j+1];
			}
			cchSize--;
			continue;
		}
		i++;
	}

	bSelect = IsSelecting( &dSqSel );	//	選択状態であるか
	if( bSelect )
	{
		iLines = DocPageParamGet( NULL, NULL );
		DocSelRangeGet( &dTop, &dBtm );
		dCrLf = DocSelectedDelete( pNowDot, pdLine, dSqSel, TRUE );
		if( dCrLf  )	//	処理した行以降全取っ替え
		{
			for( i = *pdLine; iLines > i; i++ ){	ViewRedrawSetLine(  i );	}
		}
		else{	ViewRedrawSetLine( *pdLine );	}

	}

	dCrLf = DocInsertString( pNowDot, pdLine, pdMozi, ptString, dStyle, TRUE );

	FREE( ptString );

	DocPageInfoRenew( -1, 1 );

	return dCrLf;
}
//-------------------------------------------------------------------------------------------------

/*!
	選択範囲のデータをクリップボードする
	@param[in]	bStyle	１ユニコードかシフトJISで、矩形かどうか
	@return				コピーしたバイト数・NULLターミネータも含む
*/
INT DocExClipSelect( UINT bStyle )
{
	INT	cbSize;
	LPVOID	pString = NULL;

	//	SJISの場合は、ユニコード文字は&#dddd;で確保される

	cbSize = DocSelectTextGetAlloc( bStyle, &pString, NULL );

	TRACE( TEXT("BYTE:%d"), cbSize );

	//	もし選択範囲なかったら、Focus行の内容をコピるとか

	DocClipboardDataSet( pString, cbSize, bStyle );

	FREE( pString );

	return cbSize;
}
//-------------------------------------------------------------------------------------------------

/*!
	クリップボードのデータをいただく
	@param[in]	pdStyle	矩形かどうかを確認
	@return		確保した文字列・mallocしてるので、函数呼んだ方でfree忘れないように
*/
LPTSTR DocClipboardDataGet( PUINT pdStyle )
{
	LPTSTR	ptString = NULL, ptClipTxt;
	LPSTR	pcStr, pcClipTp;	//	変換用臨時
	DWORD	cbSize;
	UINT	ixSqrFmt, dEnumFmt;
	INT		ixCount, iC;
	HANDLE	hClipData;

	ixSqrFmt = RegisterClipboardFormat( CLIP_SQUARE );

	//	クリップボードの中身をチェキ・どっちにしてもユニコードテキストフラグはある
	if( IsClipboardFormatAvailable( CF_UNICODETEXT ) )
	{
		if( pdStyle )	//	矩形であるか
		{
			if( IsClipboardFormatAvailable( ixSqrFmt ) ){	*pdStyle = D_SQUARE;	}
		}

		OpenClipboard( NULL );	//	クリップボードをオーポンする
		//	開けっ放しだと他のアプリに迷惑なのですぐ閉めるように

		dEnumFmt = 0;	//	初期値は０
		ixCount = CountClipboardFormats(  );
		for( iC = 0; ixCount > iC; iC++ )
		{	//	順番に列挙して、先にヒットしたフォーマットで扱う
			dEnumFmt = EnumClipboardFormats( dEnumFmt );
			if( CF_UNICODETEXT == dEnumFmt || CF_TEXT == dEnumFmt ){	break;	}
		}
		if( 0 == dEnumFmt ){	return NULL;	}
		//	それ以上列挙が無いか、函数失敗なら０になる

		//	クリップボードのデータをゲッツ！
		//	ハンドルのオーナーはクリップボードなので、こちらからは操作しないように
		//	中身の変更などもってのほかである
		hClipData = GetClipboardData( dEnumFmt );

		if( CF_UNICODETEXT == dEnumFmt )
		{
			//	取得データを処理。TEXTなら、ハンドルはグローバルメモリハンドル
			//	新たにコピーされたらハンドルは無効になるので、中身をコピーせよ
			ptClipTxt = (LPTSTR)GlobalLock( hClipData );
			cbSize    = GlobalSize( (HGLOBAL)hClipData );
			//	確保出来るのはバイトサイズ・テキストだと末尾のNULLターミネータ含む

			if( 0 < cbSize )
			{
				ptString = (LPTSTR)malloc( cbSize );
				StringCchCopy( ptString, (cbSize / 2), ptClipTxt );
			}
		}
		else	//	非ユニコードが優先されている場合
		{
			pcClipTp = (LPSTR)GlobalLock( hClipData );
			cbSize   = GlobalSize( (HGLOBAL)hClipData );

			if( 0 < cbSize )
			{
				pcStr = (LPSTR)malloc( cbSize );
				StringCchCopyA( pcStr, cbSize, pcClipTp );

				ptString = SjisDecodeAlloc( pcStr );	//	SJISの内容をユニコードにする
				free( pcStr );
			}
		}


		//	使い終わったら閉じておく
		GlobalUnlock( hClipData );
		CloseClipboard(  );
	}

	return ptString;
}
//-------------------------------------------------------------------------------------------------

/*!
	クリップボードに文字列貼り付け
	@param[in]	pDatum	貼り付けたい文字列・ユニかSJIS
	@param[in]	cbSize	文字列の、NULLを含んだバイト数
	@param[in]	dStyle	矩形かとかそういう指定
	@return	HRESULT	終了状態コード
*/
HRESULT DocClipboardDataSet( LPVOID pDatum, INT cbSize, UINT dStyle )
{
	HGLOBAL	hGlobal;
	HANDLE	hClip;
	LPVOID	pBuffer;
	HRESULT	hRslt;
	UINT	ixFormat, ixSqrFmt;

	//	オリジナルフォーマット名を定義しておく
	ixFormat = RegisterClipboardFormat( CLIP_FORMAT );
	ixSqrFmt = RegisterClipboardFormat( CLIP_SQUARE );

	//	クリップするデータは共有メモリに入れる
	hGlobal = GlobalAlloc( GHND, cbSize );
	pBuffer = GlobalLock( hGlobal );
	CopyMemory( pBuffer, pDatum, cbSize );
	GlobalUnlock( hGlobal );

	//	クリップボードオーポン
	OpenClipboard( NULL );

	//	中身を消しちゃう
	EmptyClipboard(  );

	//	共有メモリにブッ込んだデータをクリッペする
	if( dStyle & D_UNI )	hClip = SetClipboardData( CF_UNICODETEXT, hGlobal );
	else					hClip = SetClipboardData( CF_TEXT, hGlobal );

	if( hClip )
	{
		SetClipboardData( ixFormat, hGlobal );
		//	クリッポが上手くいったら、オリジナル名でも記録しておく

		if( dStyle & D_SQUARE  )	//	矩形選択だったら
		{
			SetClipboardData( ixSqrFmt, hGlobal );
		}

		hRslt = S_OK;
	}
	else
	{
		//	登録失敗の場合は、自分で共有メモリを破壊せないかん
		GlobalFree( hGlobal );
		hRslt = E_OUTOFMEMORY;
	}

	//	クリップボード閉じる・閉じたら即CHAINが発生する・函数内で発生させてる？
	CloseClipboard(  );

	TRACE( TEXT("COPY DONE") );

	return hRslt;
}
//-------------------------------------------------------------------------------------------------

/*!
	クリップボードに１文字だけ入れる
	@param[in]	ch	入れたい文字
	@return	HRESULT	終了状態コード
*/
HRESULT DocClipLetter( TCHAR ch )
{
	TCHAR	atBuff[3];

	ZeroMemory( atBuff, sizeof(atBuff) );
	atBuff[0] = ch;

	DocClipboardDataSet( atBuff, 4, D_UNI );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	頁全体をコピーする
	@param[in]	bStyle	ユニコードかシフトJIS
	@return		HRESULT	終了状態コード
*/
HRESULT DocPageAllCopy( UINT bStyle )
{
	INT	cbSize;
	LPVOID	pString = NULL;

	//	SJISの場合は、ユニコード文字は&#dddd;で確保される

	cbSize = DocPageTextAllGetAlloc( bStyle, &pString );

	TRACE( TEXT("BYTE:%d"), cbSize );

	DocClipboardDataSet( pString, cbSize, bStyle );

	FREE( pString );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------


/*!
	各行の末端から８００くらいまでを、指定した文字で埋める。
	@param[in]	ptFill	埋め文字列
*/
HRESULT DocScreenFill( LPTSTR ptFill )
{
	UINT_PTR	dLines, dRiDot, cchSize;
	BOOLEAN		bSel = TRUE, bFirst;
	INT			iTop, iBottom, i, iUnt, j, remain;
	INT			nDot, sDot, mDot;
	LPTSTR		ptBuffer;
	wstring		wsBuffer;

	//	現在行数と、右ドット数・ルーラ位置を使う
#ifdef LINE_VEC_LIST
	dLines = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.size( );
#else
	dLines = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.size( );
#endif
	dRiDot = gdRightRuler;

	//	選択範囲あるならそっち優先。ないなら画面全体
	iTop    = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	iBottom = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;
	if( 0 > iTop || 0 > iBottom ){	iTop = 0;	iBottom = dLines - 1;	bSel = FALSE;	}

	ViewSelPageAll( -1 );	//	選択範囲無くなる

	//	埋め文字列の幅
	mDot = ViewStringWidthGet( ptFill );

	bFirst = TRUE;
	//	各行毎に追加する感じで
	for( i = iTop; iBottom >= i; i++ )
	{
		nDot = DocLineParamGet( i , NULL, NULL );	//	呼び出せば中で面倒みてくれる
		sDot = dRiDot - nDot;	//	残りドット
		if( 0 >= sDot ){	continue;	}	//	右端超えてるならなにもせんでいい

		iUnt = (sDot / mDot) + 1;	//	埋める分・はみ出し・適当で良い

		//	入れる文字列作成
		wsBuffer.clear( );
		for( j = 0; iUnt > j; j++ ){	wsBuffer += wstring( ptFill );	}

		cchSize = wsBuffer.size( ) + 1;
		ptBuffer = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
		StringCchCopy( ptBuffer, cchSize, wsBuffer.c_str( ) );
		//	末端にブチこむ
		DocInsertString( &nDot, &i, NULL, ptBuffer, 0, bFirst );	bFirst = FALSE;
		FREE(ptBuffer);

		DocBadSpaceCheck( i );	//	ここで空白チェキ・あまり意味はないが色換えは必要
	}

	if( !(bSel) )	//	非選択状態で
	{
		remain = 40 - dLines;	//	とりあえず４０行とする
		if( 0 < remain )	//	足りないなら
		{
			DocAdditionalLine( remain, bFirst );	//	とりあえず改行して
#ifdef LINE_VEC_LIST
			dLines = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.size( );
#else
			dLines = (*gitFileIt).vcCont.at( gixFocusPage ).vcPage.size( );
#endif
			iUnt  = (dRiDot / mDot) + 1;	//	埋める分・はみ出し・適当で良い

			//	入れる文字列作成
			wsBuffer.clear( );
			for( j = 0; iUnt > j; j++ ){	wsBuffer += wstring( ptFill );	}
			cchSize = wsBuffer.size( ) + 1;
			ptBuffer = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
			StringCchCopy( ptBuffer, cchSize, wsBuffer.c_str( ) );

			iTop    = iBottom + 1;
			iBottom = dLines - 1;

			for( i = iTop; iBottom >= i; i++ )
			{
				//	末端にブチこむ
				nDot = DocLineParamGet( i , NULL, NULL );	//	多分０のハズ
				DocInsertString( &nDot, &i, NULL, ptBuffer, 0, bFirst );	bFirst = FALSE;

				DocBadSpaceCheck( i );	//	ここで空白チェキ・あまり意味はないが色換えは必要
			}

			FREE(ptBuffer);
		}
	}

	DocPageInfoRenew( -1, 1 );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------


/*!
	頁番号挿入設定ダイヤログのメッセージハンドラです。
	@param[in]	hDlg		ダイヤログハンドル
	@param[in]	message		ウインドウメッセージの識別番号
	@param[in]	wParam		追加の情報１
	@param[in]	lParam		追加の情報２
	@retval 0	メッセージは処理していない
	@retval no0	なんか処理された
*/
INT_PTR CALLBACK PageNumDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static LPPAGENUMINFO	pstInfo;
	INT		id;
	HWND	hWndCtl;
	UINT	codeNotify;

	switch( message )
	{
		case WM_INITDIALOG:
			pstInfo = (LPPAGENUMINFO)lParam;
			SetDlgItemInt(  hDlg, IDE_PN_START, pstInfo->dStartNum, FALSE );
			SetDlgItemText( hDlg, IDE_PN_STYLE, pstInfo->atStyle );
		//	CheckDlgButton( hDlg, IDCB_PN_UNDER, pstInfo->bInUnder ? BST_CHECKED : BST_UNCHECKED );
			//	上書きは通常ＯＦＦでよろし
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			id = LOWORD(wParam);
			hWndCtl = (HWND)lParam;
			codeNotify = HIWORD(wParam);

			if( IDOK == id )
			{
				pstInfo->dStartNum = GetDlgItemInt( hDlg, IDE_PN_START, NULL, FALSE );
				GetDlgItemText( hDlg, IDE_PN_STYLE, pstInfo->atStyle, MIN_STRING );
				pstInfo->bInUnder  = IsDlgButtonChecked( hDlg, IDCB_PN_UNDER );
				pstInfo->bOverride = IsDlgButtonChecked( hDlg, IDCB_PN_OVERRIDE );
				EndDialog(hDlg, IDOK );
				return (INT_PTR)TRUE;
			}
			if( IDCANCEL == id )
			{
				EndDialog(hDlg, IDCANCEL );
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	頁番号挿入のアレ
	@param[in]	hInst	アプリの実存
	@param[in]	hWnd	親ウインドウハンドル・NULLで破壊処理
	@return		HRESULT	終了状態コード
*/
HRESULT DocPageNumInsert( HINSTANCE hInst, HWND hWnd )
{
	INT			dNowPageBuffer;
	INT			iLine, iDot;
	INT_PTR		iRslt, maxPage, iNow;
	UINT		ixNumber;
	BOOLEAN		bFirst = TRUE;
	TCHAR		atText[MAX_PATH];
	PAGENUMINFO	stInfo;

	//	今の頁を待避
	dNowPageBuffer = gixFocusPage;

	maxPage = DocPageCount(  );

	ZeroMemory( &stInfo, sizeof(PAGENUMINFO) );
	stInfo.dStartNum = 1;

	StringCchCopy( stInfo.atStyle, MAX_PATH, TEXT("%u") );	//	初期値
	InitParamString( INIT_LOAD, VS_PAGE_FORMAT, stInfo.atStyle );

	iRslt = DialogBoxParam( hInst, MAKEINTRESOURCE(IDD_PAGENUMBER_DLG), hWnd, PageNumDlgProc, (LPARAM)(&stInfo) );
	if( IDOK == iRslt )
	{
		ixNumber = stInfo.dStartNum;

		InitParamString( INIT_SAVE, VS_PAGE_FORMAT, stInfo.atStyle );

		for( iNow = 0; maxPage > iNow; iNow++, ixNumber++ )
		{
			StringCchPrintf( atText, MAX_PATH, stInfo.atStyle, ixNumber );

			gixFocusPage = iNow;	//	内部操作

			if( stInfo.bInUnder )
			{
				if( stInfo.bOverride )
				{
					iLine = DocPageParamGet( NULL, NULL );
					iLine--;	if( 0 > iLine ){	iLine = 0;	}
					bFirst = DocLineErase( iLine, bFirst );
				}
				else
				{
					iLine = DocAdditionalLine( 1 , bFirst );	bFirst =  FALSE;
				}
			}
			else
			{
				iDot = 0;	iLine = 0;
				if( stInfo.bOverride )
				{
					bFirst = DocLineErase( 0, bFirst );
				}
				else
				{
					DocInsertString( &iDot, &iLine, NULL, CH_CRLFW, 0, bFirst );	bFirst = FALSE;
				}
				iLine = 0;
			}
			iDot = 0;

			DocInsertString( &iDot, &iLine, NULL, atText, 0, bFirst );	bFirst = FALSE;
		}

		//	頁元に戻す
		gixFocusPage = dNowPageBuffer;

		ViewRedrawSetLine( -1 );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------


