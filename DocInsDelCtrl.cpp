/*! @file
	@brief 文字の追加削除系の面倒みます
	このファイルは DocInsDelCtrl.cpp です。
	@author	SikigamiHNQ
	@date	2011/04/24
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

HRESULT	DocDelayPageNumInsert( FILES_ITR, INT, LPPAGENUMINFO, LPCTSTR );
//-------------------------------------------------------------------------------------------------

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

#ifdef SPMOZI_ENCODE
	if( IsSpMozi( cchMozi ) )	//	機種依存文字変換
	{
		if( gbUniRadixHex ){	StringCchPrintfA( acSjis, 10, ("&#x%X;"), cchMozi );	}
		else{					StringCchPrintfA( acSjis, 10, ("&#%d;"),  cchMozi );	}

		bCant = TRUE;	//	ユニコードのみ文字として扱う
	}
#endif

	StringCchCopyA( pcSjis, 10, acSjis );	//	変換結果を戻す

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
	文字データ作る・不要ならバイト数だけ確保できる
	@param[in]	pstLttr	文字データ入れる構造体へのポインター・NULL可
	@param[in]	ch		データ作りたい文字
	@return		改行の次の位置
*/
INT_PTR DocLetterDataCheck( LPLETTER pstLttr, TCHAR ch )
{
	INT_PTR	iByte;
	LETTER	stTemp;	//	この函数内用・データ不要時に使うダミー君

	if( !(pstLttr) ){	pstLttr = &stTemp;	}	//	ダミー君

	ZeroMemory( pstLttr, sizeof(LETTER) );
	pstLttr->cchMozi = ch;
	pstLttr->rdWidth = ViewLetterWidthGet( ch );
	pstLttr->mzStyle = CT_NORMAL;
	if( iswspace( ch ) ){	pstLttr->mzStyle |= CT_SPACE;	}
	if( !( DocIsSjisTrance( ch, pstLttr->acSjis ) ) ){	pstLttr->mzStyle |= CT_CANTSJIS;	}
	//	非シフトJIS文字を確認
	iByte = DocLetterByteCheck( pstLttr  );	//	バイト数確認

	return iByte;
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

	LINE_ITR	itLine, ltLineItr;

#ifdef DO_TRY_CATCH
	try{
#endif

	iLines = DocNowFilePageLineCount( );

	if( iLines <= rdLine )	return E_OUTOFMEMORY;

	ZeroONELINE( &stLine );

	iLetter = DocLetterPosGetAdjust( &nowDot, rdLine, 0 );	//	今の文字位置を確認

	//	文字数確認
	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, rdLine );

	iCount = itLine->vcLine.size( );

	if( iLetter < iCount )	//	もし行の途中で改行したら？
	{
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


		//	総ドット数再計算
		DocLineParamGet( rdLine,   NULL, NULL );
		DocLineParamGet( rdLine+1, NULL, NULL );
	}
	else	//	末端で改行した
	{
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
	}

	DocBadSpaceCheck( rdLine   );	//	ここで空白チェキ
	DocBadSpaceCheck( rdLine+1 );	//	空白チェキ・次の行も確認

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif

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

	LINE_ITR	itLine;


	iLines = DocNowFilePageLineCount( );

	if( iLines <=  dLine )	return 0;	//	はみ出してたらアウツ！

	iLetter = DocLetterPosGetAdjust( pdDot, dLine, 0 );	//	今の文字位置を確認
	neDot = *pdDot;

//	TRACE( TEXT("後空白[D%d C%d]"), neDot, iLetter );

	if( 0 == iLetter && 0 == dLine )	return 0;	//	先頭かつ最初の行なら、なにもしない

	//	バックスペースとは、壱文字戻ってDELETEである

	if( 0 != iLetter )	//	行の先頭でないなら
	{
		iLetter--;	//	キャレット一つ戻す
		itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
		std::advance( itLine, dLine );

		width = itLine->vcLine.at( iLetter ).rdWidth;
		ch    = itLine->vcLine.at( iLetter ).cchMozi;

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
	DocBadSpaceCheck( dLine );	//	良くないスペースを調べておく

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


	LINE_ITR	itLine;

	iLines = DocNowFilePageLineCount( );
	if( iLines <= yLine )	return 0;	//	はみ出してたらアウツ！

	iLetter = DocLetterPosGetAdjust( &xDot, yLine, 0 );	//	今の文字位置を確認

//	TRACE( TEXT("削除[D%d C%d]"), xDot, iLetter );

	DocLineParamGet( yLine, &iCount, NULL );	//	この行の文字数を斗留

	if( iCount <= iLetter )
	{
		if( iLines <= (yLine+1) )	return 0;	//	完全に末端なら何もしない
		ch = CC_LF;
	}
	else
	{
		itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
		std::advance( itLine, yLine );

		ch = itLine->vcLine.at( iLetter ).cchMozi;
	}

	iCrLf = DocLetterErase( xDot, yLine, iLetter );
	if( 0 > iCrLf ){	return -1;	}

	DocBadSpaceCheck( yLine );	//	良くないスペースを調べておく

	if( 0 < iCrLf )
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
	@return	INT			正：改行削除・０：文字削除　負：異常発生
*/
INT DocLetterErase( INT xDot, INT yLine, INT iLetter )
{
	INT		iCount, iRslt;

	LETR_ITR	vcLtrItr;
	LINE_ITR	itLine;


	iRslt = DocLineParamGet( yLine, &iCount, NULL );	//	この行の文字数を斗留
	if( 0 > iRslt ){	return -1;	}

	//	ここからDELETEの処理
	if( iCount > iLetter )	//	末端でないなら、今の文字消せばOK
	{
		itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
		std::advance( itLine, yLine );

		vcLtrItr  = itLine->vcLine.begin( );
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
	@param[in]	pFirst	アンドゥの非０初めてのグループ　０続きの処理
	@return	HRESULT	終了状態コード
*/
HRESULT DocLineErase( INT yLine, PBOOLEAN pFirst )
{
	INT		dLines, iMozis, i;
	INT_PTR	cbSize, cchSize;
	LPTSTR	ptBuffer;
	wstring	wsString;
	LINE_ITR	itLine;

	wsString.clear( );

	dLines = DocNowFilePageLineCount(  );//DocPageParamGet( NULL, NULL );	//	行数確認
	if( dLines <= yLine )	return E_OUTOFMEMORY;	//	はみ出し確認

	DocLineParamGet( yLine, &iMozis, NULL );	//	指定行の文字数確保

	if( 0 >= iMozis )	return  E_ABORT;	//	文字がないならすること無い

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, yLine );

	for( i = 0; iMozis > i; i++ )	//	全文字を確保
	{
		wsString += itLine->vcLine.at( i ).cchMozi;
	}

	cchSize = wsString.size(  ) + 1;	//	NULLターミネータ分足す
	cbSize = cchSize * sizeof(TCHAR);	//	ユニコードなのでバイト数は２倍である

	ptBuffer = (LPTSTR)malloc( cbSize );
	ZeroMemory( ptBuffer, cbSize );
	StringCchCopy( ptBuffer, cchSize, wsString.c_str(  ) );
	SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ptBuffer, 0, yLine, *pFirst );
	*pFirst = FALSE;

	//	削除処理
	itLine->vcLine.clear();

	DocLineParamGet( yLine, NULL, NULL );	//	行内容の再計算
	DocPageParamGet( NULL, NULL );	//	再計算

	DocBadSpaceCheck( yLine );	//	リセットに必要
	ViewRedrawSetLine( yLine );	//	要らないかも

	return S_OK;
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
	LINE_ITR	itLine;


	width = itLtr->rdWidth;
	bySz  = itLtr->mzByte;

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, dBsLine );

	itLine->vcLine.erase( itLtr );

	itLine->iDotCnt -= width;
	itLine->iByteSz -= bySz;

	(*gitFileIt).vcCont.at( gixFocusPage ).dByteSz -= bySz;

//	DocBadSpaceCheck( dBsLine );	//	ついでに良くないスペースを調べておく
	//	ここで調べると重そうなので、もっと上のほうで纏めてチェキるほうがよい

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
	LINE_ITR	itLine;

	//	アンドゥリドゥは呼んだところで

#ifdef DO_TRY_CATCH
	try{
#endif

	if( 0 == ch )
	{
		TRACE( TEXT("NULL文字が入った") );
		return 0;
	}

	iLines = DocNowFilePageLineCount( );

	if( iLines <= rdLine )
	{
		TRACE( TEXT("OutOfRange 指定[%d] 行数[%d]"), rdLine, iLines );
		return 0;
	}

	iLetter = DocLetterPosGetAdjust( &nowDot, rdLine, 0 );	//	今の文字位置を確認

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, rdLine );	//	対象行までイテレートする

	//	文字数確認
	iCount = itLine->vcLine.size( );

	//	データ作成
	DocLetterDataCheck( &stLetter, ch );	//	指定行のドット位置(キャレット位置)に壱文字追加するとき

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

	(*gitFileIt).vcCont.at( gixFocusPage ).dByteSz += stLetter.mzByte;

//	DocBadSpaceCheck( rdLine );	呼んだところでまとめてやる

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return  ETC_MSG( ("etc error"), 0 );	}
#endif

	return stLetter.rdWidth;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定行のドット位置で文字数分削除する・改行は弐文字占有
	@param[in]	xDot	今のキャレットのドット位置
	@param[in]	yLine	対象の行番号・絶対０インデックスか
	@param[in]	ptDummy	未使用・NULLで
	@param[in]	cchSize	削除する文字数
	@return		INT		０改行無し　１～改行をサクった回数
*/
INT DocStringErase( INT xDot, INT yLine, LPTSTR ptDummy, INT cchSize )
{
	INT	i, iCrLf, iLetter, rdCnt;

	//	今の文字位置・キャレットより末尾方向に削除するので、この位置は変わらない
	iLetter = DocLetterPosGetAdjust( &xDot, yLine, 0 );	//	今の文字位置を確認

	rdCnt = 0;
	for( i = 0; cchSize > i; i++ )	//	DEL連打ってこと
	{
		iCrLf = DocLetterErase( xDot, yLine, iLetter );
		if( 0 >  iCrLf )	break;	//	異常発生
		if( iCrLf ){	i++;	rdCnt++;	}
	}

	DocBadSpaceCheck( yLine );	//	良くないスペースを調べておく

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
INT DocStringAdd( PINT pNowDot, PINT pdLine, LPCTSTR ptStr, INT cchSize )
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
INT DocSquareAdd( PINT pNowDot, PINT pdLine, LPCTSTR ptStr, INT cchSize, LPPOINT *ppstPt )
{
	LPCTSTR		ptCaret, ptSprt;
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
	@param[in]	pFirst	アンドゥの非０初めてのグループ　０続きの処理
	@return		全体の行数
*/
INT DocAdditionalLine( INT addLine, PBOOLEAN pFirst )
{
	UINT_PTR	iLines;
	INT			cbSize, cchMozi, i;
	INT			dBaseDot, dBaseLine;
	LPTSTR		ptBuffer = NULL;

	iLines = DocNowFilePageLineCount( );
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
	SqnAppendString( &(gitFileIt->vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, ptBuffer, dBaseDot, dBaseLine, *pFirst );
	DocStringAdd( &dBaseDot, &dBaseLine, ptBuffer, cchMozi );

	FREE( ptBuffer );

	*pFirst = FALSE;

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
	iLines = DocNowFilePageLineCount( );

	//	全体行数より、追加行数が多かったら、改行増やす
	if( iLines < (dNeedLine + yLine) )
	{
		iMinus = (dNeedLine + yLine) - iLines;	//	追加する行数

		DocAdditionalLine( iMinus, &bFirst );//	bFirst = FALSE;

		//	この頁の行数取り直し
		iLines = DocNowFilePageLineCount( );
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

		ptBuffer = DocPaddingSpaceWithPeriod( iMinus, NULL, NULL, NULL, FALSE );
		if( !ptBuffer )	//	まずは綺麗に納めるのを試みて、ダメならズレありで再計算
		{	ptBuffer = DocPaddingSpaceWithGap( iMinus, NULL, NULL );	}
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
INT DocInsertString( PINT pNowDot, PINT pdLine, PINT pdMozi, LPCTSTR ptText, UINT dStyle, BOOLEAN bFirst )
{
	INT		dBaseDot, dBaseLine, dNeedLine;
	INT		dCrLf, i, dLastLine;
	UINT_PTR	cchSize;
	LPPOINT	pstPoint;

	dBaseDot  = *pNowDot;
	dBaseLine = *pdLine;
	dLastLine = *pdLine;

	if( !(ptText)  )	return 0;	//	挿入文字列がないなら直ぐ終わってよい

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

		dLastLine = DocPageParamGet( NULL, NULL );//再計算必要か？
	}

	if( dCrLf )
	{
		for( i = dBaseLine; dLastLine >= i; i++ ){	ViewRedrawSetLine(  i );	}
	}
	else
	{
		ViewRedrawSetLine( *pdLine );
	}

	if( pdMozi ){	*pdMozi = DocLetterPosGetAdjust( pNowDot, *pdLine , 0 );	}	//	今の文字位置を確認

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
	@param[in]		bSqMode	非０強制矩形貼付・内容増やすならFlagに注意
	@return		０壱行ですんだ　非０複数行に渡った
*/
INT DocInputFromClipboard( PINT pNowDot, PINT pdLine, PINT pdMozi, UINT bSqMode )
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
		NotifyBalloonExist( TEXT("テキストじゃないみたい。\t\n貼り付けられないよ。"), TEXT("お燐からのお知らせ"), NIIF_INFO );
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
		DocSelRangeGet( &dTop, &dBtm );
		dCrLf = DocSelectedDelete( pNowDot, pdLine, dSqSel, TRUE );
		if( dCrLf  )	//	処理した行以降全取っ替え
		{
			iLines = DocPageParamGet( NULL, NULL );	//	再計算も要るかも・・・
			for( i = *pdLine; iLines >= i; i++ ){	ViewRedrawSetLine(  i );	}
		}
		else{	ViewRedrawSetLine( *pdLine );	}

	}

	if( bSqMode )	dStyle |= D_SQUARE;	//	矩形挿入として扱うか
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

//	cbSize = DocPageTextAllGetAlloc( bStyle, &pString );
	cbSize = DocPageGetAlloc( bStyle, &pString );

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
	dLines = DocNowFilePageLineCount( );
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
			DocAdditionalLine( remain, &bFirst );	//	とりあえず改行して
			dLines = DocNowFilePageLineCount( );
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
			CheckDlgButton( hDlg, IDCB_PN_UNDER,    pstInfo->bInUnder );
			CheckDlgButton( hDlg, IDCB_PN_OVERRIDE, pstInfo->bOverride );
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

	maxPage = DocNowFilePageCount(  );	//	頁数を確認

	ZeroMemory( &stInfo, sizeof(PAGENUMINFO) );
	stInfo.dStartNum = 1;

	//	設定を確認
	stInfo.bInUnder  = InitParamValue( INIT_LOAD, VL_PAGE_UNDER,   BST_UNCHECKED );	//	頁番号を最下行に挿入するか
	stInfo.bOverride = InitParamValue( INIT_LOAD, VL_PAGE_OVWRITE, BST_UNCHECKED );	//	該当行の内容を削除して上書するか

	//	文字列フォーマット
	StringCchCopy( stInfo.atStyle, MAX_PATH, TEXT("%u") );	//	デフォ設定
	InitParamString( INIT_LOAD, VS_PAGE_FORMAT, stInfo.atStyle );

	iRslt = DialogBoxParam( hInst, MAKEINTRESOURCE(IDD_PAGENUMBER_DLG), hWnd, PageNumDlgProc, (LPARAM)(&stInfo) );
	if( IDOK == iRslt )	//	挿入する
	{
#pragma message("ディレイロードしたら、頁番号挿入がおかしくなるはず")
		ixNumber = stInfo.dStartNum;	//	開始番号について

		InitParamString( INIT_SAVE, VS_PAGE_FORMAT, stInfo.atStyle );
		//	設定を保存
		InitParamValue( INIT_SAVE, VL_PAGE_UNDER,   stInfo.bInUnder );
		InitParamValue( INIT_SAVE, VL_PAGE_OVWRITE, stInfo.bOverride );

		for( iNow = 0; maxPage > iNow; iNow++, ixNumber++ )
		{
			StringCchPrintf( atText, MAX_PATH, stInfo.atStyle, ixNumber );

			if( NowPageInfoGet( iNow, NULL ) )	//	ディレってないなら０
			{
				//	ディレイ文字列を操作するか
				DocDelayPageNumInsert( gitFileIt, iNow, &stInfo, atText );

				//	展開する・頁が多いと重い・しなくていい
				//DocDelayPageLoad( gitFileIt, iNow );
			}
			else
			{
				gixFocusPage = iNow;	//	内部操作
				if( stInfo.bInUnder )	//	頁最下部に挿入
				{
					if( stInfo.bOverride )	//	該当行消して挿入である
					{
						iLine = DocPageParamGet( NULL, NULL );
						iLine--;	if( 0 > iLine ){	iLine = 0;	}
						DocLineErase( iLine , &bFirst );	//	中でアンドゥ操作変換
					}
					else
					{
						iLine = DocAdditionalLine( 1, &bFirst );//	bFirst =  FALSE;
					}
				}
				else	//	壱行目に挿入
				{
					iDot = 0;	iLine = 0;
					if( stInfo.bOverride )	//	該当行消して挿入である
					{
						DocLineErase( 0 , &bFirst );	//	中でアンドゥ操作変換
					}
					else
					{
						DocInsertString( &iDot, &iLine, NULL, CH_CRLFW, 0, bFirst );	bFirst = FALSE;
					}
					iLine = 0;
				}
				iDot = 0;

				//	頁番号の内容挿入
				DocInsertString( &iDot, &iLine, NULL, atText, 0, bFirst );	bFirst = FALSE;
			}
		}

		//	頁元に戻す
		gixFocusPage = dNowPageBuffer;

		ViewRedrawSetLine( -1 );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ディレイしてる頁に頁番号を挿入
	@param[in]	itFile		対象のファイル
	@param[in]	iPage		そのファイルの頁
	@param[in]	pstInfo		頁番号の処理方法
	@param[in]	ptPageText	挿入する文字列・頁番号
	@return	HRESULT	終了状態コード
*/
HRESULT DocDelayPageNumInsert( FILES_ITR itFile, INT iPage, LPPAGENUMINFO pstInfo, LPCTSTR ptPageText )
{
	UINT_PTR	cchPgTx;	//	頁文字列の文字数
	UINT_PTR	cchSrc, cchSize;
	LPTSTR	ptWork, ptMoto, ptNext, ptCaret, ptPrev;

	//	あり得ないはずだが
	if( !(itFile->vcCont.at( iPage ).ptRawData) )	return E_NOTIMPL;

	ptMoto = itFile->vcCont.at( iPage ).ptRawData;

	StringCchLength( ptPageText, MAX_PATH, &cchPgTx );	//	キメうち注意
	//cchPgTx += 4;	//	改行分の余裕を確保しておく

	StringCchLength( ptMoto, STRSAFE_MAX_CCH, &cchSrc );

	//	ワーク領域確保・本文＋頁文字列＋改行・あまりは気にしない
	cchSize = cchSrc + cchPgTx + 4;
	ptWork = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
	ZeroMemory( ptWork, cchSize * sizeof(TCHAR) );

	//	最後尾なら、先に本文いれて、頁文字列追加するか、末端行の先頭に上書き＋NULL
	//	先頭なら、先に頁文字列いれて、本文追加するか、本文弐行目から追加
	if( pstInfo->bInUnder )	//	末尾
	{
		StringCchCopy( ptWork, cchSize, ptMoto );	//	先に本文

		if( pstInfo->bOverride )	//	上書
		{
			ptNext = ptMoto;
			ptCaret = ptMoto;
			do{
				ptPrev = ptNext;	//	今の開始位置を保存
				ptNext = NextLineW( ptCaret );	//	次を確認
				ptCaret = ptNext;	//	次の開始位置

			}while( ptNext );	//	NULLなら末端までイッたということ

			StringCchCopy( ptPrev, (cchPgTx + 4), ptPageText );
			//	最低でも頁文字列＋余裕分は残ってる
		}
		else	//	挿入
		{
			StringCchCat( ptWork, cchSize, TEXT("\r\n") );	//	何にしても改行
			StringCchCat( ptWork, cchSize, ptPageText );	//	そして追加
		}
	}
	else	//	頭
	{
		StringCchCopy( ptWork, cchSize, ptPageText );	//	先頭である
		StringCchCat( ptWork, cchSize, TEXT("\r\n") );	//	何にしても改行

		if( pstInfo->bOverride )	//	上書
		{
			//	次の行の先頭から追加
			ptNext = NextLineW( ptMoto );
			if( ptNext )	StringCchCat( ptWork, cchSize, ptNext );
			//	次の行が無かったら、何もする必要は無い
		}
		else	//	挿入
		{
			//	挿入ならそのまま追加すればよろし
			StringCchCat( ptWork, cchSize, ptMoto );
		}
	}

	//	メモリ領域付け替えておｋ・先に元文字列はフリーしちゃう
	FREE( itFile->vcCont.at( iPage ).ptRawData );
	itFile->vcCont.at( iPage ).ptRawData = ptWork;

	return S_OK;
}
//-------------------------------------------------------------------------------------------------



