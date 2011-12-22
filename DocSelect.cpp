/*! @file
	@brief 選択範囲の面倒みます
	このファイルは DocSelect.cpp です。
	@author	SikigamiHNQ
	@date	2011/04/27
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

extern FILES_ITR	gitFileIt;	//	今見てるファイルの本体
#define gstFile	(*gitFileIt)	//!<	イテレータを構造体と見なす

extern INT		gixFocusPage;	//	注目中のページ・とりあえず０・０インデックス

extern  UINT	gbUniPad;		//	パディングにユニコードをつかって、ドットを見せないようにする

extern  UINT	gbCrLfCode;		//	改行コード：０したらば・非０ＹＹ 

extern  UINT	gbCpModSwap;	//	SJISとユニコードコピーを入れ替える	//@@コピー処理

static INT		gdSelByte;		//!<	選択範囲のバイト数
//-------------------------------------------------------------------------------------------------


INT		DocLetterSelStateToggle( INT, INT, INT );
VOID	DocSelectedByteStatus( VOID );
//-------------------------------------------------------------------------------------------------

/*!
	選択範囲がある開始行と終了行を登録
	@param[in]	dTop	選択範囲開始行
	@param[in]	dBottom	選択範囲終了行
	@return		HRESULT	終了状態コード
*/
HRESULT DocSelRangeSet( INT dTop, INT dBottom )
{
	TRACE( TEXT(" 選択レンジセット[%d - %d]"), dTop, dBottom );

	gstFile.vcCont.at( gixFocusPage ).dSelLineTop    = dTop;
	gstFile.vcCont.at( gixFocusPage ).dSelLineBottom = dBottom;

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	選択範囲がある開始行と終了行を取得
	@param[in]	pdTop	選択範囲開始行いれるバッファえのぽうんた
	@param[in]	pdBtm	選択範囲終了行いれるバッファえのぽいんた
	@return		HRESULT	終了状態コード
*/
HRESULT DocSelRangeGet( PINT pdTop, PINT pdBtm )
{
	if( pdTop ){	*pdTop = gstFile.vcCont.at( gixFocusPage ).dSelLineTop;	}
	if( pdBtm ){	*pdBtm = gstFile.vcCont.at( gixFocusPage ).dSelLineBottom;	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	示されたドット位置の直後の文字の選択状態をON/OFFして、該当文字の幅を返す・単独では呼ばれない？
	@param[in]	nowDot	対象のドット位置
	@param[in]	rdLine	対象の行番号・ドキュメントの０インデックス
	@param[in]	dForce	０斗愚留　＋選択状態　ー選択解除
	@return		該当文字のドット数
*/
INT DocLetterSelStateToggle( INT nowDot, INT rdLine, INT dForce )
{
	UINT	dStyle, maeSty;
	INT		dLtrDot = 0, iLetter, dByte;
	INT_PTR	iLines;

	iLines = gstFile.vcCont.at( gixFocusPage ).vcPage.size( );
	if( iLines <= rdLine )	return 0;

	iLetter = DocLetterPosGetAdjust( &nowDot, rdLine, 0 );

	//	直後の文字の幅を確認
	dLtrDot = gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.at( iLetter ).rdWidth;
	dByte   = gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.at( iLetter ).mzByte;

	//	フラグ操作
	dStyle = gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.at( iLetter ).mzStyle;
	maeSty = dStyle;
	if( 0 == dForce ){		dStyle ^=  CT_SELECT;	}
	else if( 0 < dForce ){	dStyle |=  CT_SELECT;	}
	else if( 0 > dForce ){	dStyle &= ~CT_SELECT;	}
	gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).vcLine.at( iLetter ).mzStyle = dStyle;

	if( maeSty != dStyle )	//	フラグ操作されてたら
	{
		if( CT_SELECT & dStyle )	gdSelByte += dByte;
		else						gdSelByte -= dByte;

		if( 0 >  gdSelByte )	gdSelByte = 0;
		//	０未満になったら本当はおかしい
	}

	return dLtrDot;	//	ドット数戻してＯＫ
}
//-------------------------------------------------------------------------------------------------

/*!
	指定された行の始点終点ドット位置の区間の文字の選択状態をON/OFFする
	@param[in]	dBgnDot	開始ドット位置・マイナスなら０
	@param[in]	dEndDot	終了ドット位置・マイナスなら行末端
	@param[in]	rdLine	対象の行番号・ドキュメントの０インデックス
	@param[in]	dForce	０斗愚留　＋選択状態　ー選択解除
	@return		該当文字のドット数
*/
INT DocRangeSelStateToggle( INT dBgnDot, INT dEndDot, INT rdLine, INT dForce )
{
	UINT_PTR	iLines;
	INT	dLtrDot = 0, dMaxDots, dDot;
	RECT	rect;

	iLines = gstFile.vcCont.at( gixFocusPage ).vcPage.size( );
	if( (INT)iLines <=  rdLine )	return 0;

	dMaxDots = DocLineParamGet( rdLine, NULL, NULL );
	//	範囲調整
	if( 0 > dBgnDot )	dBgnDot = 0;
	if( 0 > dEndDot )	dEndDot = dMaxDots;

	for( dDot = dBgnDot; dEndDot > dDot;  )
	{
		dDot += DocLetterSelStateToggle( dDot, rdLine, dForce );
	}

	dLtrDot = dDot - dBgnDot;

	rect.left   = dBgnDot;
	rect.top    = rdLine * LINE_HEIGHT;
	rect.right  = dEndDot;
	rect.bottom = rect.top + LINE_HEIGHT;

//	ViewRedrawSetLine( rdLine );
	ViewRedrawSetRect( &rect );

	DocSelectedByteStatus(  );

	return dLtrDot;	//	ドット数戻してＯＫ
}
//-------------------------------------------------------------------------------------------------

/*!
	指定された行の改行の選択状態をON/OFFする
	@param[in]	rdLine	対象の行番号・ドキュメントの０インデックス
	@param[in]	dForce	０斗愚留　＋選択状態　ー選択解除
	@return		HRESULT	終了状態コード
*/
HRESULT DocReturnSelStateToggle( INT rdLine, INT dForce )
{
	UINT_PTR	iLines;
	UINT		dStyle, maeSty;
	INT			iLnDot, dByte;
	RECT		rect;

	iLines = gstFile.vcCont.at( gixFocusPage ).vcPage.size( );
	if( (INT)iLines <=  rdLine )	return E_OUTOFMEMORY;

	iLnDot = DocLineParamGet( rdLine, NULL, NULL );//gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).iDotCnt;
	//	フラグ操作
	dStyle = gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).dStyle;
	maeSty = dStyle;
	if( 0 == dForce ){		dStyle ^=  CT_SELRTN;	}
	else if( 0 < dForce ){	dStyle |=  CT_SELRTN;	}
	else if( 0 > dForce ){	dStyle &= ~CT_SELRTN;	}
	gstFile.vcCont.at( gixFocusPage ).vcPage.at( rdLine ).dStyle = dStyle;

	if( maeSty != dStyle )	//	フラグ操作されてたら
	{
		if( gbCrLfCode )	dByte = YY2_CRLF;
		else				dByte = STRB_CRLF;

		if( CT_SELRTN & dStyle )	gdSelByte += dByte;
		else						gdSelByte -= dByte;

		if( 0 >  gdSelByte )	gdSelByte = 0;
		//	０未満になったら本当はおかしい

		DocSelectedByteStatus(  );
	}

	rect.left   = iLnDot;
	rect.top    = rdLine * LINE_HEIGHT;
	rect.right  = iLnDot + 20;	//	たぶんこれくらい
	rect.bottom = rect.top + LINE_HEIGHT;

//	ViewRedrawSetLine( rdLine );
	ViewRedrawSetRect( &rect );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ページ全体の選択状態をON/OFFする
	@param[in]	dForce	０无　＋選択状態　ー選択解除
	@return		全体文字数
*/
INT DocPageSelStateToggle( INT dForce )
{
	UINT_PTR	iLines, ln, iLetters, mz;
	UINT		dStyle;
	INT			iTotal, iDot, iWid;
	RECT		inRect;

	if( 0 == dForce )	return 0;	//	０なら処理しない

	if( 0 > gixFocusPage )	return 0;	//	特殊な状況下では処理しない

	iTotal = 0;
	iLines = gstFile.vcCont.at( gixFocusPage ).vcPage.size( );
	for( ln = 0; iLines > ln; ln++ )
	{
		iDot = 0;	//	そこまでのドット数をため込む
		inRect.top    = ln * LINE_HEIGHT;
		inRect.bottom = inRect.top + LINE_HEIGHT;

		iLetters = gstFile.vcCont.at( gixFocusPage ).vcPage.at( ln ).vcLine.size( );	//	この行の文字数確認して
		//	壱文字ずつ、全部をチェキっていく
		for( mz = 0; iLetters > mz; mz++ )
		{
			//	直前の状態
			dStyle = gstFile.vcCont.at( gixFocusPage ).vcPage.at( ln ).vcLine.at( mz ).mzStyle;
			iWid   = gstFile.vcCont.at( gixFocusPage ).vcPage.at( ln ).vcLine.at( mz ).rdWidth;

			inRect.left  = iDot;
			inRect.right = iDot + iWid;

			if( 0 < dForce )
			{
				gstFile.vcCont.at( gixFocusPage ).vcPage.at( ln ).vcLine.at( mz ).mzStyle |=  CT_SELECT;
				if( !(dStyle & CT_SELECT) )	ViewRedrawSetRect( &inRect );
			}
			else
			{
				gstFile.vcCont.at( gixFocusPage ).vcPage.at( ln ).vcLine.at( mz ).mzStyle &= ~CT_SELECT;
				if( dStyle & CT_SELECT )	ViewRedrawSetRect( &inRect );
			}

			iDot += iWid;
			iTotal++;
		}

		//	壱行終わったら末尾状況確認。改行・本文末端に改行はない・選択のときのみ
		dStyle = gstFile.vcCont.at( gixFocusPage ).vcPage.at( ln ).dStyle;
		inRect.left  = iDot;
		inRect.right = iDot + 20;	//	改行描画エリア・大体これくらい
		if( 0 < dForce )
		{
			if( iLines > ln+1 )
			{
				gstFile.vcCont.at( gixFocusPage ).vcPage.at( ln ).dStyle |=  CT_SELRTN;
				if( !(dStyle & CT_SELRTN) )	ViewRedrawSetRect( &inRect );
			}
		}
		else
		{
			gstFile.vcCont.at( gixFocusPage ).vcPage.at( ln ).dStyle &=  ~CT_SELRTN;
			if( dStyle & CT_SELRTN )	ViewRedrawSetRect( &inRect );
		}
	}

	if( 0 < dForce )
	{
		DocSelRangeSet(  0, iLines - 1 );
		DocPageParamGet( NULL, &gdSelByte );
	}
	else
	{
		DocSelRangeSet( -1, -1 );
		gdSelByte = 0;
	}
	DocSelectedByteStatus(  );

//	ViewRedrawSetLine( -1 );	//	画面表示更新

	return iTotal;
}
//-------------------------------------------------------------------------------------------------

/*!
	選択されてるバイト数をステータスバーに表示
*/
VOID DocSelectedByteStatus( VOID )
{
	TCHAR	atBuffer[MIN_STRING];
	
	if( gdSelByte )
	{
		StringCchPrintf( atBuffer, MIN_STRING, TEXT("SEL %d Bytes"), gdSelByte );
		StatusBarSetText( SB_SELBYTE, atBuffer );
	}
	else
	{
		StatusBarSetText( SB_SELBYTE, TEXT("") );
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	選択されているところを全削除しちゃう
	@param[in]	pdDot	キャレットドット位置・書き換える必要がある
	@param[in]	pdLine	行番号・書き換える必要がある
	@param[in]	bSqSel	矩形選択してるのかどうか・D_SQUARE
	@return	非０改行あった　０壱行のみ
*/
INT DocSelectedDelete( PINT pdDot, PINT pdLine, UINT bSqSel )
{
//	UINT_PTR	iLines;
	UINT_PTR	iMozis;
	INT			i, j, dBeginX = 0, dBeginY = 0, cbSize;
	INT			iLct, k, bCrLf;
	LPTSTR		ptText;
	LPPOINT		pstPt;

	LTR_ITR	itLtr, itEnd, itHead, itTail;


	bSqSel &= D_SQUARE;	//	矩形ビットだけ残す

	//	ページ全体の行数
//	iLines = gstFile.vcCont.at( gixFocusPage ).vcPage.size( );

	i = gstFile.vcCont.at( gixFocusPage ).dSelLineTop;
	j = gstFile.vcCont.at( gixFocusPage ).dSelLineBottom;
	TRACE( TEXT("範囲削除[T%d - B%d]"), i, j );
	if( 0 > i ){	return 0;	}	//	選択範囲が無かった


//アンドゥバッファリングの準備
	iLct = j - i + 1;	//	含まれる行なので、数えるの注意
	cbSize = DocSelectTextGetAlloc( D_UNI | bSqSel, (LPVOID *)(&ptText), NULL );
	pstPt = (LPPOINT)malloc( iLct * sizeof(POINT) );
	ZeroMemory( pstPt, iLct * sizeof(POINT) );
	k = iLct - 1;

	bCrLf = iLct - 1;

	dBeginY = i;	//	選択肢のある行

	for( ; i <= j; j--, k-- )
	{
		iMozis = gstFile.vcCont.at( gixFocusPage ).vcPage.at( j ).vcLine.size( );
		if( 0 < iMozis )
		{
			itLtr = gstFile.vcCont.at( gixFocusPage ).vcPage.at( j ).vcLine.begin(  );
			itEnd = gstFile.vcCont.at( gixFocusPage ).vcPage.at( j ).vcLine.end(  );

			itHead = itEnd;
			itTail = itEnd;

			dBeginX = 0;

			//	最初の選択部分を検索
			for( ; itLtr != itEnd; itLtr++ )
			{
				if( CT_SELECT & itLtr->mzStyle )
				{
					itHead =  itLtr;
					break;
				}

				dBeginX += itLtr->rdWidth;	//	意味があるのは最後のところなので、常時上書きでおｋ
			}

			//	選択されてない所まで検索
			for( ; itLtr != itEnd; itLtr++ )
			{
				if( !(CT_SELECT & itLtr->mzStyle) )
				{
					itTail =  itLtr;
					break;
				}
			}
		}

		pstPt[k].x = dBeginX;
		pstPt[k].y = j;

		if( 0 < iMozis )
		{
			//	該当範囲を削除・末端は、該当部分の直前までが対象・末端自体は非対象
			gstFile.vcCont.at( gixFocusPage ).vcPage.at( j ).vcLine.erase( itHead, itTail );
		}

		//	改行が含まれていたら
		if( CT_SELRTN & gstFile.vcCont.at( gixFocusPage ).vcPage.at( j ).dStyle )
		{
			DocLineCombine( j );
		}

		DocLineParamGet( j, NULL, NULL );

		//	改行サクるとこれによりatが無効になる？

		//	ページ全体の行数再設定？
//		iLines = gstFile.vcCont.at( gixFocusPage ).vcPage.size( );

	}

	ViewSelPageAll( -1 );	//	選択範囲無くなる

	//	カーソル位置移動せないかん
	*pdDot = dBeginX;	*pdLine = dBeginY;

	if( bSqSel ){	SqnAppendSquare( &(gstFile.vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ptText, pstPt, iLct, TRUE );	}
	else{		SqnAppendString( &(gstFile.vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ptText, dBeginX, dBeginY, TRUE );	}

	FREE( ptText );

	FREE( pstPt );

	return bCrLf;
}
//-------------------------------------------------------------------------------------------------

/*!
	選択範囲を指定文字列で塗りつぶす
	@param[in]	ptBrush	ブラシ文字列・NULLなら空白
	@param[in]	pdDot	キャレットドット位置・書き換える必要がある
	@param[in]	pdLine	行番号・書き換える必要がある
	@return	非０塗った　０してない
*/
INT DocSelectedBrushFilling( LPTSTR ptBrush, PINT pdDot, PINT pdLine )
{
	UINT_PTR	iMozis;
	UINT_PTR	cchSize;
	INT			i, j, dBeginX = 0, dBeginY = 0;
	INT			iLct, dTgtDot, dBgnDot, dNowDot;
	BOOLEAN		bFirst;

	LPTSTR		ptReplc = NULL, ptDeled;
//	INT			dZenSp, dHanSp, dUniSp;

	wstring		wsBuffer;
	LTR_ITR	itLtr, itEnd, itHead, itTail;

	bFirst = TRUE;

	i = gstFile.vcCont.at( gixFocusPage ).dSelLineTop;
	j = gstFile.vcCont.at( gixFocusPage ).dSelLineBottom;
	TRACE( TEXT("範囲確認[T%d - B%d]"), i, j );
	if( 0 > i ){	return 0;	}	//	選択範囲が無かった

	dBeginY = i;	//	選択肢のある行
	dBeginX = 0;

	//	壱行ずつ処理していく
	for( iLct = i; j >= iLct; iLct++ )
	{
		//	文字数確認して
		iMozis = gstFile.vcCont.at( gixFocusPage ).vcPage.at( iLct ).vcLine.size( );
		if( 0 < iMozis )
		{
			itLtr = gstFile.vcCont.at( gixFocusPage ).vcPage.at( iLct ).vcLine.begin(  );
			itEnd = gstFile.vcCont.at( gixFocusPage ).vcPage.at( iLct ).vcLine.end(  );

			itHead = itEnd;
			itTail = itEnd;

			dBgnDot = 0;
			dTgtDot = 0;

			//	最初の選択部分を検索
			for( ; itLtr != itEnd; itLtr++ )
			{
				if( CT_SELECT & itLtr->mzStyle )
				{
					itHead =  itLtr;
					dTgtDot = itLtr->rdWidth;
					itLtr++;	//	次の文字を参照
					break;
				}

				dBgnDot += itLtr->rdWidth;
			}
			if( iLct == i ){	dBeginX = dBgnDot;	}	//	意味があるのは最後のところ

			//	選択されてない所まで検索
			for( ; itLtr != itEnd; itLtr++ )
			{
				if( !(CT_SELECT & itLtr->mzStyle) )
				{
					itTail =  itLtr;
					break;
				}
				dTgtDot += itLtr->rdWidth;	//	ドット数を確認
			}

			//	当てはめるアレを計算する
			if( ptBrush )
			{
				ptReplc = BrushStringMake( dTgtDot, ptBrush );
			}
			else	//	空白指定ということ
			{
				ptReplc = DocPaddingSpaceMake( dTgtDot );
			}

			//	ここで、埋め文字列が作成不可なら、この行の処理は飛ばす
			if( !(ptReplc) )	continue;


			//	該当部分の内容を記録＜アンドゥ用
			wsBuffer.clear();
			for( itLtr = itHead; itLtr != itTail; itLtr++ )
			{
				wsBuffer += itLtr->cchMozi;
			}

			cchSize = wsBuffer.size( ) + 1;
			ptDeled = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
			StringCchCopy( ptDeled, cchSize, wsBuffer.c_str( ) );

			//	該当部分を削除
			gstFile.vcCont.at( gixFocusPage ).vcPage.at( iLct ).vcLine.erase( itHead, itTail );

			//	ブラシ文字列で埋める
			StringCchLength( ptReplc, STRSAFE_MAX_CCH, &cchSize );
			dNowDot = dBgnDot;
			DocStringAdd( &dNowDot, &iLct, ptReplc, cchSize );

			SqnAppendString( &(gstFile.vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ptDeled, dBgnDot, iLct, bFirst );	bFirst = FALSE;
			SqnAppendString( &(gstFile.vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, ptReplc, dBgnDot, iLct, bFirst );

			FREE( ptDeled );

			FREE( ptReplc );

			ViewRedrawSetLine( iLct );
		}

	}

	ViewSelPageAll( -1 );	//	選択範囲無くなる

	//	カーソル位置移動せないかん
	*pdDot = dBeginX;	*pdLine = dBeginY;

	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	ページ全体から、選択されている文字列を確保する・freeは呼んだ方でやる
	@param[in]		bStyle	１ユニコードかシフトJISで、矩形かどうか
	@param[in,out]	*pText	確保した領域を返す・ワイド文字かマルチ文字になる・NULLだと必要バイト数を返すのみ
	@param[in,out]	*pstPt	選択範囲の行番号と開始ドット位置をメモリして返す・開放は呼んだほうでやる・NULLなら何もしない
	@return					確保したバイト数・NULLターミネータも含む
*/
INT DocSelectTextGetAlloc( UINT bStyle, LPVOID *pText, LPPOINT *pstPt )
{
	//	指定行の指定範囲をコピーするようにすればいい
	//	SJISの場合は、ユニコード文字は&#ddddd;で確保される
	//	もしかしたら&#xhhhh;かもしれない

	UINT_PTR	iLines, i, j, iLetters;
	INT_PTR		iSize;
	INT			d, k, m, iLn;
	BOOLEAN		bNoSel;
	LPPOINT		pstPoint = NULL;

	string	srString;	//	ユニコード・シフトJISで確保
	wstring	wsString;

	srString.clear( );
	wsString.clear( );

	//	ページ全体の行数
	iLines = gstFile.vcCont.at( gixFocusPage ).vcPage.size( );

	//	開始地点から開始
	d = gstFile.vcCont.at( gixFocusPage ).dSelLineTop;
	k = gstFile.vcCont.at( gixFocusPage ).dSelLineBottom;
	TRACE( TEXT("選択内容確保[%d - %d]"), d, k );
	if( 0 > d ){	d = 0;	}
//	if( 0 > k ){	k = iLines -  1;	}	テストで外してる

	if( pstPt )
	{
		iLn = k - d + 1;	//	行の数
		if( 0 < iLn ){	pstPoint = (LPPOINT)malloc( iLn * sizeof(POINT) );	}
		*pstPt = pstPoint;
	}

	for( m = 0, i = d; iLines > i; i++, m++ )
	{
		if( pstPoint  ){	pstPoint[m].x = 0;	pstPoint[m].y = i;	}

		//	各行の文字数
		iLetters = gstFile.vcCont.at( gixFocusPage ).vcPage.at( i ).vcLine.size( );

		bNoSel = TRUE;
		for( j = 0; iLetters > j; j++ )
		{
			//	選択されている部分を文字列に確保
			if( CT_SELECT & gstFile.vcCont.at( gixFocusPage ).vcPage.at( i ).vcLine.at( j ).mzStyle )
			{
				bNoSel = FALSE;

				if( bStyle & D_UNI )	wsString += gstFile.vcCont.at( gixFocusPage ).vcPage.at( i ).vcLine.at( j ).cchMozi;
				else	srString +=  string( gstFile.vcCont.at( gixFocusPage ).vcPage.at( i ).vcLine.at( j ).acSjis );
			}

			if( bNoSel && pstPt )	pstPoint[m].x += gstFile.vcCont.at( gixFocusPage ).vcPage.at( i ).vcLine.at( j ).rdWidth;
		}

		if( bStyle & D_SQUARE  )	//	矩形のときは容赦なく改行
		{
			if( bStyle & D_UNI )	wsString += wstring( CH_CRLFW );
			else					srString +=  string( CH_CRLFA );
		}
		else
		{
			//	改行が含まれていたらその分確保
			if( CT_SELRTN & gstFile.vcCont.at( gixFocusPage ).vcPage.at( i ).dStyle )
			{
				if( bStyle & D_UNI )	wsString += wstring( CH_CRLFW );
				else					srString +=  string( CH_CRLFA );
			}
		}

		//	選択範囲末端までイッたらおしまい
		if( (INT)i == gstFile.vcCont.at( gixFocusPage ).dSelLineBottom )	break;
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
	抽出対象領域を取り出す
	@param[in]	hInst	実存値
	@return		HRESULT	終了状態コード
*/
HRESULT DocExtractExecute( HINSTANCE hInst )
{
	INT	dOffDot, dCount;
	BOOLEAN	bLnFirst, bMzFirst, bIsVoid;
	LPTSTR	ptSpace, ptString;
	UINT_PTR	cch;//, i;

	LINE_ITR	itLnFirst, itLnLast, itLnErate, itLnEnd;
	LTR_ITR		itMozi, itMzEnd;

	wstring	wsBuffer;


	if( 0 >= gstFile.vcCont.size() )	return S_FALSE;

	//	開始行と終止行・オフセット量を検索
	itLnErate = gstFile.vcCont.at( gixFocusPage ).vcPage.begin();
	itLnEnd   = gstFile.vcCont.at( gixFocusPage ).vcPage.end();

	itLnFirst = itLnErate;
	itLnLast  = itLnEnd;

	dOffDot = DocPageMaxDotGet( -1, -1 );	//	MAX位置を初期にしとけばおｋ

	bLnFirst = TRUE;

	for( ; itLnEnd != itLnErate; itLnErate++ )	//	行サーチ
	{
		itMozi  = itLnErate->vcLine.begin();
		itMzEnd = itLnErate->vcLine.end();

		dCount = 0;

		for( ; itMzEnd != itMozi; itMozi++ )	//	文字サーチ
		{
			if( CT_SELECT & itMozi->mzStyle )	//	選択状態発見
			{
				if( bLnFirst )	//	最初の行が未発見であれば
				{
					itLnFirst = itLnErate;	//	今の行を記録する
					bLnFirst = FALSE;
				}
				itLnLast  = itLnErate;	//	選択状態があるので終止行を更新

				//	そこまでのオフセット量よりさらに小さければ更新
				if( dOffDot > dCount )	dOffDot = dCount;

				break;	//	次の行に移動
			}

			dCount += itMozi->rdWidth;	//	そこまでのドット数をため込む

		}
	}
	if( itLnLast != itLnEnd )	 itLnLast++;	//	終止の次の行を示しておく

	if( bLnFirst )	return  S_FALSE;	//	選択範囲がなかったら死にます

	wsBuffer.clear();

	//	開始行から内容を確保していく
	for( itLnErate = itLnFirst; itLnLast != itLnErate; itLnErate++ )	//	行サーチ
	{
		itMozi  = itLnErate->vcLine.begin();
		itMzEnd = itLnErate->vcLine.end();

		bMzFirst = TRUE;
		bIsVoid  = FALSE;
		dCount   = 0;

		for( ; itMzEnd != itMozi; itMozi++ )	//	文字サーチ
		{
			if( CT_SELECT & itMozi->mzStyle )	//	選択状態発見
			{
				if( bIsVoid )	//	直前まで未選択状態
				{
					if( bMzFirst )	//	最初の空白部分であれば
					{
						dCount -= dOffDot;	//	オフセットする
						if( 0 > dCount )	dCount = 0;
						bMzFirst = FALSE;
					}

					//	埋めSpaceを作る・不可ならNULLが返る
					ptSpace = DocPaddingSpaceMake( dCount );
					if( ptSpace )
					{
						wsBuffer += ptSpace;
						//StringCchLength( ptSpace, STRSAFE_MAX_CCH, &cch );
						//for( i = 0; cch > i; i++ )
						//{
						//	wsBuffer += (ptSpace[i]);
						//}
						FREE(ptSpace);
					}
				}

				wsBuffer += itMozi->cchMozi;
				dCount =  0;	//	リセット
				bIsVoid = FALSE;
			}
			else
			{
				dCount += itMozi->rdWidth;	//	そこまでのドット数をため込む
				bIsVoid = TRUE;
			}
		}

		wsBuffer += CH_CRLFW;	//	改行追加
	}
	//	この時点で、wsBufferに全体が入っているはず

	cch = wsBuffer.size( ) + 1;
	ptString = (LPTSTR)malloc( cch * sizeof(TCHAR) );
	StringCchCopy( ptString, cch, wsBuffer.c_str( ) );

	if( hInst )	//	実存してるならレイヤボックスへ
	{
		LayerBoxVisibalise( hInst, ptString, 0x00 );
	}
	else	//	ないならクルッペボード
	{
//@@コピー処理
		DocClipboardDataSet( ptString, cch * sizeof(TCHAR), gbCpModSwap ? D_SJIS : D_UNI );
	}


	FREE(ptString);

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

