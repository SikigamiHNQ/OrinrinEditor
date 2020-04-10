/*! @file
	@brief なんか調整とかそういうの
	このファイルは DocAdjustant.cpp です。
	@author	SikigamiHNQ
	@date	2011/05/10
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

/*
１ドットずらし
目標の幅を１１で割って、余りゲット
１～５なら半角SP追加、６～１０なら全角SP追加して、
目標のドット数になるまで全角SP(11ドット)を半角SP２つ(10ドット)に置き換えていく
数が足りなくなったりしたら操作不可・３８以下で発生する
必要な全角半角SPが求まったら、数が多い方を先において、そこに一つおきに
少ない方をいれていく。

右揃え
最低幅が２２で、あとはパターン。範囲内の一番長い幅をベース０とし、
そこからのへこみ量を０～１０で求める。それ以上は全SPで埋めるだけ
計算出来ないか？とりあえずスペースのみで埋めて、半角が多いようなら
ドット付けてやり直しとか。無限ループに注意

上記二つ、極力全角半角SPを使い、連続した半角が発生したり、どうしても合わない
ようなら、ユニコードの特殊スペースを使って、ピリオドや連続半角をなくすように？

自動調整
基準ドットを決めて、調整に用いるスペースエリアにカーソルをあてて
その行のズレを確認して必要なずらしを行う。
このときはユニコードを使ったり、ピリオドを使ったりして連続半角を追い出す
基準位置はRULERに赤で示すとか

スペースパディングのしかた
・連続半角
・ピリオド
・ユニコード
*/

//-------------------------------------------------------------------------------------------------


extern FILES_ITR	gitFileIt;	//!<	今見てるファイルの本体
extern INT		gixFocusPage;	//!<	注目中のページ・とりあえず０・０インデックス

extern  UINT	gbUniPad;		//!<	パディングにユニコードをつかって、ドットを見せないようにする

static INT		gdDiffLock;		//!<	ずれ調整の基準ドット値
//-------------------------------------------------------------------------------------------------

//	右揃え用空白パヤーン
CONST  TCHAR	gaatDotPtrnPeriod[11][9] = {
	{ TEXT("　　") },		//	22	0
	{ TEXT("　....") },		//	23	1
	{ TEXT(" 　 .") },		//	24	2
	{ TEXT("　　.") },		//	25	3
	{ TEXT("　.....") },	//	26	4
	{ TEXT("　　 ") },		//	27	5
	{ TEXT("　　..") },		//	28	6
	{ TEXT("　......") },	//	29	7
	{ TEXT("　　 .") },		//	30	8
	{ TEXT("　　...") },	//	31	9
	{ TEXT("　 　 ") }		//	32	10
};
#define RIGHT_WALL	TEXT('|')	//	右揃え用壁文字

//	右揃え用空白パヤーンユニコード	20120315
CONST  TCHAR	gaatDotPtrnUnic[11][6] = {
	{ TEXT("　　") },				//	22	0	全SP　全SP
	{ 0x3000,0x3000,0x200A },		//	23	1	全SP　全SP　1dot
	{ 0x3000,0x3000,0x2009 },		//	24	2	全SP　全SP　2dot
	{ 0x3000,0x3000,0x2006 },		//	25	3	全SP　全SP　3dot
	{ 0x3000,0x3000,0x2005 },		//	26	4	全SP　全SP　4dot
	{ TEXT("　　 ") },				//	27	5	全SP　全SP　半SP
	{ 0x3000,0x0020,0x3000,0x200A },//	28	6	全SP　半SP　全SP　1dot
	{ 0x3000,0x0020,0x3000,0x2009 },//	29	7	全SP　半SP　全SP　2dot
	{ 0x3000,0x0020,0x3000,0x2006 },//	30	8	全SP　半SP　全SP　3dot
	{ 0x3000,0x0020,0x3000,0x2005 },//	31	9	全SP　半SP　全SP　4dot
	{ TEXT("　 　 ") }				//	32	10	全SP　半SP　全SP　半SP
};



//	パディング用空白パヤーン・非ユニコード
CONST TCHAR gaatPaddingSpDot[34][9] = {
	{ TEXT("")    },		//	0
	{ TEXT(".")   },		//	1	3
	{ TEXT(".")   },		//	2	3
	{ TEXT(".")   },		//	3	3
	{ TEXT(".")   },		//	4	3
	{ TEXT(" ")   },		//	5		TEXT("..")	6
	{ TEXT("..")  },		//	6
	{ TEXT("..")  },		//	7	6
	{ TEXT(". ")  },		//	8
	{ TEXT("...") },		//	9
	{ TEXT("　")  },		//	10	11
	{ TEXT("　")  },		//	11
	{ TEXT("　")  },		//	12	11	{ TEXT("....") }
	{ TEXT(".　") },		//	13	14
	{ TEXT(".　") },		//	14
	{ TEXT("　 ") },		//	15	16	{ TEXT(".....") }
	{ TEXT("　 ")  },		//	16
	{ TEXT(".　.") },		//	17
	{ TEXT(". 　") },		//	18	19	{ TEXT("......") }
	{ TEXT(". 　") },		//	19
	{ TEXT("..　.") },		//	20
	{ TEXT(" 　 ") },		//	21	{ TEXT(".......") }
	{ TEXT("　　") },		//	22
	{ TEXT("　　")  },		//	23	22	{ TEXT("..　..") }
	{ TEXT("　 . ") },		//	24
	{ TEXT(".　　") },		//	25
	{ TEXT("　 　") },		//	26	27	{ TEXT("...　..") }
	{ TEXT("　 　") },		//	27
	{ TEXT(".　　.")  },	//	28
	{ TEXT(".　 　")  },	//	29	30	{ TEXT("...　...") }
	{ TEXT(".　 　")  },	//	30
	{ TEXT(".　.　.") },	//	31
	{ TEXT("　 　 ")  },	//	32
	{ TEXT("　　　") }		//	33
};

//	パディング用空白パヤーン・ユニコード
CONST TCHAR gaatPaddingSpDotW[11][3] = {
	{ TEXT("") },		//	0	0	0
	{ 8202 },			//	1	1	0
	{ 8201 },			//	2	1	0
	{ 8198 },			//	3	1	0
	{ 8197 },			//	4	1	0
	{ TEXT(' ') },		//	5	0	1
	{ 8202, TEXT(' ') },//	6	1	1
	{ 8201, TEXT(' ') },//	7	1	1
	{ 8194 },			//	8	1	0
	{ 8197, TEXT(' ') },//	9	1	1
	{ 8199 }			//	10	1	0
};						//		U	H

//-------------------------------------------------------------------------------------------------


UINT	SpaceWidthAdjust( INT, PINT, PINT );	//!<	
LPTSTR	SpaceStrAlloc( INT, INT );	//!<	

UINT	DocSpaceDifference( UINT, PINT, INT, UINT );	//!<	

HRESULT	DocRightGuideSet( INT, INT );	//!<	

LPTSTR	DocPaddingSpace( INT , PINT, PINT );	//!<	
//-------------------------------------------------------------------------------------------------


/*!
	使うスペースの数を、減算して調整
	@param[in]		dDot	目標ドット数
	@param[in,out]	pZen	全角ＳＰ数を受けて、調整した個数を返す
	@param[in,out]	pHan	半角ＳＰ数を受けて、調整した個数を返す
	@return			UINT	非０調整ＯＫ・０失敗
*/
UINT SpaceWidthAdjust( INT dDot, PINT pZen, PINT pHan )
{
	INT		dZen, dHan, size;

	dZen = *pZen;
	dHan = *pHan;

	do
	{
		size = (dZen * SPACE_ZEN) + (dHan * SPACE_HAN);

		if( dDot == size )
		{
			*pZen = dZen;
			*pHan = dHan;
			return 1;
		}

		dZen--;		//	全SP:11dot、半SP:5dotなので、
		dHan += 2;	//	全SP減らして半SP弐増やすと1dot縮む
	}
	while(  0 <= dZen );	//	全SPが無くなるとアウト

	return 0;
}
//-------------------------------------------------------------------------------------------------

/*!
	全角スペース、半角スペースの個数を受けて、文字列をつくって返す
	@param[in]	dZen	全角スペースの個数
	@param[in]	dHan	半角スペースの個数
	@return		LPTSTR	作成したスペースの文字列
*/
LPTSTR SpaceStrAlloc( INT dZen, INT dHan )
{
	INT		cchSize, i;
	LPTSTR	ptStr;

	cchSize = dZen + dHan;	//	必要数	↓NULLたみねた分増やす
	ptStr = (LPTSTR)malloc( (cchSize + 1) * sizeof(TCHAR) );
	if( !ptStr )	return NULL;
	ZeroMemory( ptStr, (cchSize + 1) * sizeof(TCHAR) );

	for( i = (cchSize - 1); 0 <= i; )
	{
		if( 0 < dHan )
		{
			ptStr[i--] = TEXT(' ');
			dHan--;
			if( 0 >  i )	break;
		}

		if( 0 < dZen )
		{
			ptStr[i--] = TEXT('　');
			dZen--;
			if( 0 >  i )	break;
		}
	}

	return ptStr;
}
//-------------------------------------------------------------------------------------------------

/*!
	ドット数を受けて、そこに収まるようなスペースの組み合わせを計算・ユニコード空白も使う
	@param[in]	dTgtDot	作成するドット数
	@param[out]	pdZenSp	使用した全角スペースの個数入れる・NULLでもＯＫ
	@param[out]	pdHanSp	使用した半角スペースの個数入れる・NULLでもＯＫ
	@param[out]	pdUniSp	使用したユニコードの個数入れる・NULLでもＯＫ
	@return		LPTSTR	作成したスペースの文字列・開放は呼んだ方で面倒見る
*/
LPTSTR DocPaddingSpaceUni( INT dTgtDot, PINT pdZenSp, PINT pdHanSp, PINT pdUniSp )
{
	INT		dZen, dHan, dUni;
	INT		iCnt, iRem;
	INT		cchSize, i;
	LPTSTR	ptStr = NULL;

	//	幅０だと作れない
	if( 0 >= dTgtDot )	return NULL;

	iCnt =  dTgtDot / SPACE_ZEN;	//	全角で出来るだけ埋める
	iRem =  dTgtDot % SPACE_ZEN;	//	余るか？

	dZen = iCnt;	//	とりあえず必要数

	switch( iRem )	//	それぞれの必要数確保
	{
		case  1:	dUni = 1;	dHan = 0;	break;
		case  2:	dUni = 1;	dHan = 0;	break;
		case  3:	dUni = 1;	dHan = 0;	break;
		case  4:	dUni = 1;	dHan = 0;	break;
		case  5:	dUni = 0;	dHan = 1;	break;
		case  6:	dUni = 1;	dHan = 1;	break;
		case  7:	dUni = 1;	dHan = 1;	break;
		case  8:	dUni = 1;	dHan = 0;	break;
		case  9:	dUni = 1;	dHan = 1;	break;
		case 10:	dUni = 1;	dHan = 0;	break;
		default:	dUni = 0;	dHan = 0;	break;
	}

	cchSize = dZen + dHan + dUni;	//	必要数	↓NULLたみねた分増やす
	ptStr = (LPTSTR)malloc( (cchSize + 1) * sizeof(TCHAR) );
	if( !ptStr )	return NULL;
	ZeroMemory( ptStr, (cchSize + 1) * sizeof(TCHAR) );

	for( i = 0; dZen > i; i++ ){	ptStr[i] = TEXT('　');	}

	switch( iRem )	//	各文字を追加
	{
		case  1:	ptStr[i++] = gaatPaddingSpDotW[1][0];	break;
		case  2:	ptStr[i++] = gaatPaddingSpDotW[2][0];	break;
		case  3:	ptStr[i++] = gaatPaddingSpDotW[3][0];	break;
		case  4:	ptStr[i++] = gaatPaddingSpDotW[4][0];	break;
		case  5:	ptStr[i++] = gaatPaddingSpDotW[5][0];	break;
		case  6:	ptStr[i++] = gaatPaddingSpDotW[6][0];	ptStr[i++] = gaatPaddingSpDotW[6][1];	break;
		case  7:	ptStr[i++] = gaatPaddingSpDotW[7][0];	ptStr[i++] = gaatPaddingSpDotW[7][1];	break;
		case  8:	ptStr[i++] = gaatPaddingSpDotW[8][0];	break;
		case  9:	ptStr[i++] = gaatPaddingSpDotW[9][0];	ptStr[i++] = gaatPaddingSpDotW[9][1];	break;
		case 10:	ptStr[i++] = gaatPaddingSpDotW[10][0];	break;
		default:	break;
	}

	if( pdZenSp )	*pdZenSp = dZen;
	if( pdHanSp )	*pdHanSp = dHan;
	if( pdUniSp )	*pdUniSp = dUni;

	return ptStr;
}
//-------------------------------------------------------------------------------------------------

/*!
	ドット数を受けて、そこに収まるようなスペースの組み合わせを計算
	@param[in]	dTgtDot	作成するドット数
	@param[out]	pdZenSp	使用した全角スペースの個数入れる・NULLでもＯＫ
	@param[out]	pdHanSp	使用した半角スペースの個数入れる・NULLでもＯＫ
	@return		LPTSTR	作成したスペースの文字列・開放は呼んだ方で面倒見る・作成不可ならNULL
*/
LPTSTR DocPaddingSpace( INT dTgtDot, PINT pdZenSp, PINT pdHanSp )
{
	INT		dZen, dHan;
	INT		iCnt, iRem;
	UINT	dRslt;
	LPTSTR	ptStr = NULL;

	//	幅０だと作れない
	if( 0 >= dTgtDot )	return NULL;

	iCnt =  dTgtDot / SPACE_ZEN;	//	全角で出来るだけ埋める
	iRem =  dTgtDot % SPACE_ZEN;	//	余るか？

	dZen = iCnt;	//	とりあえず必要数

	if( 1 <= iRem && iRem <= 5 )	//	半角で埋める
	{
		dHan = 1;
	}
	else if( 6 <= iRem && iRem <= 10 )	//	全角で埋める
	{
		dHan = 0;
		dZen++;
	}
	else	//	ぴったりだった
	{
		dHan = 0;
	}

	//	数調整
	dRslt = SpaceWidthAdjust( dTgtDot, &dZen, &dHan );

	if( dRslt )
	{
		if( pdZenSp )	*pdZenSp = dZen;
		if( pdHanSp )	*pdHanSp = dHan;

		//	メモリ確保して文字列作る
		ptStr = SpaceStrAlloc( dZen, dHan );

		return ptStr;
	}

	return NULL;
}
//-------------------------------------------------------------------------------------------------

/*!
	ドット数を多少前後してもいいから埋める
	@param[in]	dTgtDot	作成するドット数
	@param[out]	pdZenSp	使用した全角スペースの個数入れる・NULLでもＯＫ
	@param[out]	pdHanSp	使用した半角スペースの個数入れる・NULLでもＯＫ
	@return		LPTSTR	作成したスペースの文字列・開放は呼んだ方で面倒見る・作成不可ならNULL
*/
LPTSTR DocPaddingSpaceWithGap( INT dTgtDot, PINT pdZenSp, PINT pdHanSp )
{
	INT		cchSize, i;
	LPTSTR	ptStr = NULL;

	if( 16 <= dTgtDot )	//	幅を増やしながら収まる範囲をさがす
	{
		i = 0;

		do
		{
			if( 22 < i )	return NULL;	//	無限ループ阻止。大丈夫と思うけど。

			ptStr = DocPaddingSpace( dTgtDot, pdZenSp, pdHanSp );
			dTgtDot++;	i++;

		}while( !(ptStr) );

		return ptStr;
	}


	//	エリアが小さすぎるので例外
	cchSize = 1;	//	必要数	↓NULLたみねた分増やす
	ptStr = (LPTSTR)malloc( (cchSize + 1) * sizeof(TCHAR) );
	if( !ptStr )	return NULL;

	if( 7 >= dTgtDot )	//	半角壱個で目をつぶる
	{
		ptStr[0] = TEXT(' ');
		if( pdZenSp )	*pdZenSp = 0;
		if( pdHanSp )	*pdHanSp = 1;
	}
	else if( 8 <= dTgtDot && dTgtDot <= 15 )	//	全角壱個でごまかす
	{
		ptStr[0] = TEXT('　');
		if( pdZenSp )	*pdZenSp = 1;
		if( pdHanSp )	*pdHanSp = 0;
	}
	ptStr[1] = 0x0000;

	return ptStr;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定ドット位置が含まれている、スペースか非スペースの文字の始点終点ドット数確保
	@param[in]	dDot	指定ドット・これの左の文字で判断
	@param[in]	rdLine	行数
	@param[out]	pLeft	始点ドット・行頭かも
	@param[out]	pRight	終点ドット・行末かも
	@param[out]	pStCnt	開始地点の文字数・NULLでも可
	@param[out]	pCount	間の文字数・NULLでも可
	@param[out]	pIsSp	該当はスペースであるか？
	@return		該当範囲のドット数
*/
INT DocLineStateCheckWithDot( INT dDot, INT rdLine, PINT pLeft, PINT pRight, PINT pStCnt, PINT pCount, PBOOLEAN pIsSp )
{
	UINT_PTR	iCount;
	INT		bgnDot, endDot;
	INT		iBgnCnt, iRngCnt;
	TCHAR	ch, chb;
	UINT	dMozis;
	INT		bSpace;
	LETR_ITR	itMozi, itHead, itTail, itTemp;

	LINE_ITR	itLine;

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, rdLine );

	if( !(pLeft) || !(pRight) || !(pIsSp) ){	return 0;	}

	itMozi = itLine->vcLine.begin( );
	iCount = itLine->vcLine.size( );	//	この行の文字数確認

	//	中身が無いならエラー
	if( 0 >= iCount ){	*pIsSp =  FALSE;	*pLeft =  0;	*pRight = 0;	return 0;	}

	dMozis = DocLetterPosGetAdjust( &dDot , rdLine, 0 );	//	今の文字位置を確認

	if( 1 <= dMozis ){	itMozi += (dMozis-1);	}	//	キャレットの位置の左文字で判定
	//	最初から先頭ならなにもしなくておｋ
	ch = itMozi->cchMozi;
	bSpace = iswspace( ch );


	//	その場所から頭方向に辿って、途切れ目を探す
	itHead = itLine->vcLine.begin( );

	for( ; itHead != itMozi; itMozi-- )
	{
		chb = itMozi->cchMozi;
		if( iswspace( chb ) != bSpace ){	itMozi++;	break;	}
	}
	//	先頭までイッちゃった場合・これが抜けてた
	if( itHead == itMozi )
	{
		chb = itMozi->cchMozi;
		if( iswspace( chb ) != bSpace ){	itMozi++;	}
	}
	//	基準と異なる文字にヒットしたか、先頭位置である

	//	先頭から、ヒット位置まで辿ってドット数と文字数確認
	bgnDot = 0;
	iBgnCnt = 0;
	for( itTemp = itHead; itTemp != itMozi; itTemp++ )
	{
		bgnDot += itTemp->rdWidth;	//	文字幅増やして
		iBgnCnt++;	//	文字数も増やす
	}//もし最初から先頭なら両方０のまま

	itTail = itLine->vcLine.end( );

	//	その場所から、同じグループの所まで確認
	endDot = bgnDot;
	iRngCnt = 0;
	for( ; itTemp != itTail; itTemp++ )
	{
		chb = itTemp->cchMozi;	//	同じタイプである間加算続ける
		if( iswspace( chb ) != bSpace ){	break;	}

		endDot += itTemp->rdWidth;
		iRngCnt++;
	}

	*pLeft  = bgnDot;
	*pRight = endDot;
	*pIsSp  = bSpace ? TRUE : FALSE;

	if( pCount )	*pCount = iRngCnt;
	if( pStCnt )	*pStCnt = iBgnCnt;

	return (endDot - bgnDot);
}
//-------------------------------------------------------------------------------------------------

/*!
	現在のドット位置を含んでいる空白エリアを１ドットずつずらす
	@param[in]	vk		方向・右か左か
	@param[in]	pXdot	今のドット位置を受けて戻す
	@param[in]	dLine	今の行数
	@param[in]	dFirst	アンドゥグループ・非０なら最初の一発　０続き
	@return	UINT	非０ズレ値　０失敗
*/
UINT DocSpaceDifference( UINT vk, PINT pXdot, INT dLine, UINT dFirst )
{
	INT			dTgtDot, dNowDot;
	INT			dBgnDot, dEndDot;
	INT			dBgnCnt, dRngCnt;
	UINT_PTR	cchSize;
	BOOLEAN		bIsSpace;
	LPTSTR		ptSpace;//, ptOldSp;
	INT			dZenSp, dHanSp, dUniSp;
	INT			iDots, iBytes;

	wstring		wsBuffer;
	LETR_ITR	vcLtrBgn, vcLtrEnd, vcItr;

	LINE_ITR	itLine;

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, dLine );

	dNowDot = *pXdot;

	if( 0 == dNowDot )	//	０の場合は強引に移動
	{
		dNowDot = itLine->vcLine.at( 0 ).rdWidth;
	}

	dTgtDot = DocLineStateCheckWithDot( dNowDot, dLine, &dBgnDot, &dEndDot, &dBgnCnt, &dRngCnt, &bIsSpace );
	if( !(bIsSpace) )	return 0;	//	非スペースエリアは意味が無い

	if( VK_RIGHT == vk )		dTgtDot++;	//	右なら増やすってこと
	else if( VK_LEFT == vk )	dTgtDot--;
	else	return 0;	//	関係ないのはアウツ

	//	当てはめるアレを計算する
	ptSpace = DocPaddingSpace( dTgtDot, &dZenSp, &dHanSp );
	if( gbUniPad )
	{
		//	作成不可だったり半角多すぎたら、ユニコード使って作り直し
		if( !(ptSpace) || (dZenSp < dHanSp) )	//	(dZenSp + 1)
		{
			FREE(ptSpace);
			ptSpace = DocPaddingSpaceUni( dTgtDot, &dZenSp, &dHanSp, &dUniSp );
		}
	}

	if( !(ptSpace) )	return 0;	//	作成不可だった場合


	StringCchLength( ptSpace, STRSAFE_MAX_CCH, &cchSize );

	vcLtrBgn  = itLine->vcLine.begin( );
	vcLtrBgn += dBgnCnt;	//	該当位置まで移動して
	vcLtrEnd  = vcLtrBgn;
	vcLtrEnd += dRngCnt;	//	そのエリアの終端も確認

	iDots = 0;	iBytes = 0;
	wsBuffer.clear();
	for( vcItr = vcLtrBgn; vcLtrEnd != vcItr; vcItr++ )
	{
		wsBuffer += vcItr->cchMozi;
		iDots    += vcItr->rdWidth;
		iBytes   += vcItr->mzByte;
	}

	//	該当部分を一旦削除・アンドゥリドゥするなら内容を記録する必要がある
	itLine->vcLine.erase( vcLtrBgn, vcLtrEnd );
	itLine->iByteSz -= iBytes;	if( 0 > itLine->iByteSz ){	itLine->iByteSz = 0;	}
	itLine->iDotCnt -= iDots;	if( 0 > itLine->iDotCnt ){	itLine->iDotCnt = 0;	}

//	DocLineParamGet( yLine, NULL, NULL );	//	行内容の再計算
//あとの函数内で呼ばれまくってる

	//	Space文字列を追加
	dNowDot = dBgnDot;
	DocStringAdd( &dNowDot, &dLine, ptSpace, cchSize );

	*pXdot = dNowDot;

//	cchSize = wsBuffer.size( ) + 1;
//	ptOldSp = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
//	StringCchCopy( ptOldSp, cchSize, wsBuffer.c_str( ) );

	SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, wsBuffer.c_str( ), dBgnDot, dLine, dFirst );
	SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, ptSpace, dBgnDot, dLine, FALSE );	//	弐回目なので確定でよろし

//	FREE( ptOldSp );

	FREE( ptSpace );

	return dTgtDot;
}
//-------------------------------------------------------------------------------------------------

/*!
	現在のドット位置を含んでいる空白エリアを１ドットずらすシーケンス
	@param[in]	vk		方向・右か左か
	@param[in]	pXdot	今のドット位置を受けて戻す
	@param[in]	dLine	今の行数
	@return	INT	非０ズレ値　０失敗
*/
INT DocSpaceShiftProc( UINT vk, PINT pXdot, INT dLine )
{
	INT		dDot, dMozi, dPreByte;

	//	20110720	０文字で操作するとあぼーんするので確認しておく
	dDot = DocLineParamGet( dLine, &dMozi, &dPreByte );
	if( 0 >= dMozi )	return 0;

	dDot = DocSpaceDifference( vk, pXdot, dLine, TRUE );

	DocLetterPosGetAdjust( pXdot, dLine, 0 );	//	この中のDocLineParamGetでバイト数が計算されてる

	ViewRedrawSetLine( dLine );

	ViewDrawCaret( *pXdot, dLine, 1 );

	return dDot;
}
//-------------------------------------------------------------------------------------------------

/*!
	右揃え線の面倒見る
	@param[in]	pVoid	なにか
	@return		HRESULT	終了状態コード
*/
HRESULT	DocRightGuideline( LPVOID pVoid )
{
	INT	iTop, iBottom, i;

	TRACE( TEXT("右揃え線") );

	iTop    = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	iBottom = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;

	DocRightGuideSet( iTop, iBottom );

	ViewSelPageAll( -1 );	//	選択範囲無くなる

	if( 0 > iTop || 0 > iBottom ){	ViewRedrawSetLine( -1 );	}
	else{	for( i =  iTop; iBottom >= i; i++ ){	ViewRedrawSetLine(  i );	}	}


	DocPageInfoRenew( -1, 1 );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定範囲に右揃え線を付ける
	@param[in]	dTop	開始行・含む・－１で最初から
	@param[in]	dBottom	終了行・含む・－１で最後まで
	@return		HRESULT	終了状態コード
*/
HRESULT DocRightGuideSet( INT dTop, INT dBottom )
{
	//	処理が終わったら、呼んだ方で選択範囲の解除と画面更新すること

	UINT_PTR	iLines, cchSize;
	INT			baseDot, i, j, iMz, nDot, sDot, lDot, iUnt, iPadot;
	TCHAR		ch, atBuffer[MAX_PATH];
	LPTSTR		ptBuffer;
	BOOLEAN		bFirst;
	wstring		wsBuffer;

	//	範囲確認
	iLines = DocNowFilePageLineCount( );
	if( 0 > dTop )		dTop = 0;
	if( 0 > dBottom )	dBottom = iLines - 1;

	//	
	ZeroMemory( atBuffer, sizeof(atBuffer) );
	atBuffer[0] = RIGHT_WALL;
	InitParamString( INIT_LOAD, VS_RGUIDE_MOZI, atBuffer );

	//	一番長いとところを確認
	baseDot = DocPageMaxDotGet( dTop, dBottom );

	bFirst = TRUE;
	//	各行毎に追加する感じで
	for( i = dTop; dBottom >= i; i++ )
	{
		nDot = DocLineParamGet( i , NULL, NULL );	//	呼び出せば中で面倒みてくれる
		sDot = baseDot - nDot;
		iUnt = sDot / SPACE_ZEN;	//	埋める分
		sDot = sDot % SPACE_ZEN;	//	はみ出しドット確認
		//	変数使い回し注意

		iPadot = nDot;
		wsBuffer.clear( );	//	アンドゥバッファ用記録

		for( j = 0; iUnt > j; j++ )
		{
			ch = TEXT('　');	//	入れるのは全角空白確定
			wsBuffer += ch;
			lDot  = DocInputLetter( nDot, i, ch );
			nDot += lDot;
		}

		//	20120315	ユニコードモードならゆにゆにっとする
		if( gbUniPad  ){	iMz = lstrlen( gaatDotPtrnUnic[sDot]  );	}
		else{				iMz = lstrlen( gaatDotPtrnPeriod[sDot] );	}

		//	揃え線までの空白を埋める
		for( j = 0; iMz > j; j++ )
		{
			if( gbUniPad  ){	ch = gaatDotPtrnUnic[sDot][j];	}	//	20120315
			else{			ch = gaatDotPtrnPeriod[sDot][j];	}

			wsBuffer += ch;
			lDot  = DocInputLetter( nDot, i, ch );
			nDot += lDot;
		}

		//	揃え末端文字入れ込む
		wsBuffer += atBuffer[0];
		lDot  = DocInputLetter( nDot, i, atBuffer[0] );
		nDot += lDot;

		DocBadSpaceCheck( i );	//	ここで空白チェキ

		//	入れた文字を統合してアンドゥバッファリング
		cchSize = wsBuffer.size( ) + 1;
		ptBuffer = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
		StringCchCopy( ptBuffer, cchSize, wsBuffer.c_str(  ) );

		SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, ptBuffer, iPadot, i, bFirst );
		bFirst = FALSE;

		FREE( ptBuffer );
	}


	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ずれ調整用の基準ドット値をセット
	@param[in]	yLine	対象行
	@return		INT		設定したドット値
*/
INT DocDiffAdjBaseSet( INT yLine )
{
	INT	dDot = 0;
	TCHAR	atMessage[MAX_STRING];

	TRACE( TEXT("自動調整基準点ロックオン") );

	dDot = DocLineParamGet( yLine, NULL, NULL );

	gdDiffLock = dDot;

	StringCchPrintf( atMessage, MAX_STRING, TEXT("調整基準位置を %d ドットに設定したですぅ"), dDot );
	NotifyBalloonExist( atMessage, TEXT("ロックオン"), NIIF_INFO );

	return dDot;
}
//-------------------------------------------------------------------------------------------------

/*!
	ずれ調整を実行する
	@param[in]	pxDot	調整位置
	@param[in]	yLine	対象行
	@return		INT		調整したドット量
*/
INT DocDiffAdjExec( PINT pxDot, INT yLine )
{
	INT			dMotoDot = 0;
	INT			dBgnDot, dEndDot, dBgnCnt, dRngCnt, iSabun, dTgtDot, nDot;
	UINT_PTR	cchSize, cchPlus;
	BOOLEAN		bIsSpace;
	LPTSTR		ptPlus, ptBuffer;

	wstring		wsDelBuf, wsAddBuf;
	LETR_ITR	vcLtrBgn, vcLtrEnd, vcItr;

	LINE_ITR	itLine;

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, yLine );

	//	調整値の状況を確認
	dTgtDot = DocLineStateCheckWithDot( *pxDot, yLine, &dBgnDot, &dEndDot, &dBgnCnt, &dRngCnt, &bIsSpace );
	if( !(bIsSpace) )
	{
		NotifyBalloonExist( TEXT("連続するスペースの部分にカーソルを当てるですぅ"), TEXT("調整不可能"), NIIF_ERROR );
		return 0;
	}
	//今現在の空白幅を確認


	//	対象行の長さを確認
	dMotoDot = DocLineParamGet( yLine, NULL, NULL );
	iSabun = gdDiffLock - dMotoDot;	//	差分確認・マイナスならはみ出してる

//まず全角半角で埋めて、半角が多いようならピリオド付けて再計算

	dTgtDot += iSabun;	//	変更後のドット数

	if( 41 > dTgtDot )	//	ユニコード使うなら確認しなくても大丈夫？
	{
		NotifyBalloonExist( TEXT("もう少し幅がないと調整できないですぅ"), TEXT("狭すぎ"), NIIF_ERROR );
		return 0;
	}

	//埋め文字列作成
	ptPlus = DocPaddingSpaceWithPeriod( dTgtDot, NULL, NULL, NULL, FALSE );

	if( !(ptPlus) )
	{
		NotifyBalloonExist( TEXT("調整出来なかったですぅ"), TEXT("自動調整失敗"), NIIF_ERROR );
		return 0;
	}

	StringCchLength( ptPlus, STRSAFE_MAX_CCH, &cchPlus );


	vcLtrBgn  = itLine->vcLine.begin( );
	vcLtrBgn += dBgnCnt;	//	該当位置まで移動して
	vcLtrEnd  = vcLtrBgn;
	vcLtrEnd += dRngCnt;	//	そのエリアの終端も確認

	wsDelBuf.clear();
	for( vcItr = vcLtrBgn; vcLtrEnd != vcItr; vcItr++ ){	wsDelBuf +=  vcItr->cchMozi;	}

	//	該当部分を削除
	itLine->vcLine.erase( vcLtrBgn, vcLtrEnd );
	nDot = dBgnDot;

	cchSize = wsDelBuf.size( ) + 1;
	ptBuffer = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
	StringCchCopy( ptBuffer, cchSize, wsDelBuf.c_str( ) );
	SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ptBuffer, dBgnDot, yLine, TRUE );
	FREE( ptBuffer );

//ここで文字列追加
	DocStringAdd( &nDot, &yLine, ptPlus, cchPlus );
	SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, ptPlus, dBgnDot, yLine, FALSE );
	FREE(ptPlus);

//もろもろの位置合わせしておｋ
	*pxDot = nDot;

	DocLetterPosGetAdjust( pxDot, yLine, 0 );

	ViewRedrawSetLine( yLine );

	ViewDrawCaret( *pxDot, yLine, 1 );

	return iSabun;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定されたドット幅を、ピリオドも使って綺麗に確保する・１９幅までなら調整できる・これはこれで必要
	@param[in]	dTgtDot	作成するドット数
	@param[out]	pdZen	使用した全角スペースの個数入れる・NULLでもＯＫ
	@param[out]	pdHan	使用した半角スペースの個数入れる・NULLでもＯＫ
	@param[out]	pdPrd	使用したピリオドの個数入れる・NULLでもＯＫ
	@param[in]	bFull	固定テーブルつかって強引にあわせる？
	@return		LPTSTR	作成したスペースの文字列・開放は呼んだ方で面倒見る・作成不可ならNULL
*/
LPTSTR DocPaddingSpaceWithPeriod( INT dTgtDot, PINT pdZen, PINT pdHan, PINT pdPrd, BOOLEAN bFull )
{
	INT	dZenSp, dHanSp, dPrdSp, m, dPre;
	LPTSTR	ptSpace = NULL, ptPlus = NULL;
	UINT	cchSize, cchPlus;


	dPre = dTgtDot;
	dPrdSp = 0;

	do{
		dZenSp =  0;	dHanSp =  0;
		ptSpace = DocPaddingSpace( dTgtDot, &dZenSp, &dHanSp );

		//	作成不可だった場合	半角多すぎても不可
		if( !(ptSpace) || (dZenSp < dHanSp) )	//	(dZenSp + 1)
		{
			FREE(ptSpace);
			if( gbUniPad )	//	上手くいかないようなら、ユニコード使ってやり直す
			{
				ptSpace = DocPaddingSpaceUni( dTgtDot, &dZenSp, &dHanSp, NULL );
				break;
			}
			else
			{
				dPrdSp++;	dTgtDot -= 3;	//	ピリオドは３ドット
			}
		}
		else	//	問題無い文字列ならおｋ
		{
			break;
		}

	}while( dTgtDot >= 19 );	//	これ以上は不可？

	if( !(ptSpace) && bFull )	//	まだ制作できてなく、固定テーブル使うなら
	{
		dPrdSp = 0;
		dTgtDot = dPre;

		StringCchLength( gaatPaddingSpDot[dTgtDot], STRSAFE_MAX_CCH, &cchSize );

		cchSize += 1;
		ptSpace = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
		ZeroMemory( ptSpace, cchSize * sizeof(TCHAR) );

		StringCchCopy( ptSpace, cchSize, gaatPaddingSpDot[dTgtDot] );
	}

	if( ptSpace )
	{
		StringCchLength( ptSpace, STRSAFE_MAX_CCH, &cchSize );
		
		//	ピリオド入れてサイズ調整
		cchPlus = cchSize + dPrdSp + 1;
		ptPlus = (LPTSTR)malloc( cchPlus * sizeof(TCHAR) );
		ZeroMemory( ptPlus, cchPlus * sizeof(TCHAR) );

		StringCchCopy( ptPlus, cchPlus, ptSpace );
		FREE(ptSpace);	//	スペースおしまい

		//	複数ピリオドあったら前後につけるようにしたい
		for( m = 0; dPrdSp > m; m++ ){	StringCchCat( ptPlus , cchPlus, TEXT(".") );	}
	}


	if( pdZen  )	*pdZen = dZenSp;
	if( pdHan  )	*pdHan = dHanSp;
	if( pdPrd  )	*pdPrd = dPrdSp;

	return ptPlus;
}
//-------------------------------------------------------------------------------------------------


/*!
	行頭に、文字(主に空白)を追加
	@param[in]	ch		追加する文字
	@param[in]	pXdot	今のドット位置を受けて戻す・削除に巻き込まれた対応
	@param[in]	dLine	今の行数
	@return		HRESULT	終了状態コード
*/
HRESULT DocTopLetterInsert( TCHAR ch, PINT pXdot, INT dLine )
{
	UINT_PTR	iLines;
	INT			iTop, iBottom, i, xDot = 0;
	BOOLEAN		bFirst = TRUE, bSeled = FALSE;

	TRACE( TEXT("行頭空白を追加") );
	
	//	範囲確認
	iLines  = DocNowFilePageLineCount( );
	iTop    = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	iBottom = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;
	if( 0 <= iTop &&  0 <= iBottom )	bSeled = TRUE;

	if( 0 > iTop )		iTop = 0;
	if( 0 > iBottom )	iBottom = iLines - 1;

	//	選択範囲は、操作した行全体を選択状態にする

	for( i = iTop; iBottom >= i; i++ )	//	範囲内の各行について
	{
		//	先頭位置に文字桃得留。
		xDot = DocInputLetter( 0, i, ch );

		SqnAppendLetter( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, ch, 0, i, bFirst );
		bFirst = FALSE;

		if( bSeled )
		{
			DocRangeSelStateToggle( -1, -1, i, 1 );	//	該当行全体を選択状態にする
			DocReturnSelStateToggle( i, 1 );	//	改行も選択で
		}

		DocBadSpaceCheck( i );
		ViewRedrawSetLine( i );
	}

	//	キャレット位置ずれてたら適当に調整
	*pXdot += xDot;
	DocLetterPosGetAdjust( pXdot, dLine, 0 );	//	キャレット位置適当に調整
	ViewDrawCaret( *pXdot, dLine, 1 );

	DocPageInfoRenew( -1, 1 );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	行頭全角空白及びユニコード空白を削除する
	@param[in]	pXdot	今のドット位置を受けて戻す・削除に巻き込まれた対応
	@param[in]	dLine	今の行数
	@return		HRESULT	終了状態コード
*/
HRESULT DocTopSpaceErase( PINT pXdot, INT dLine )
{
	UINT_PTR	iLines;
	INT			iTop, iBottom, i;
	BOOLEAN		bFirst = TRUE, bSeled = FALSE;
	TCHAR		ch;

	LETR_ITR	vcLtrItr;

	LINE_ITR	itLine;

	//	範囲確認
	iLines  = DocNowFilePageLineCount( );
	iTop    = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	iBottom = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;
	if( 0 <= iTop &&  0 <= iBottom )	bSeled = TRUE;

	if( 0 > iTop )		iTop = 0;
	if( 0 > iBottom )	iBottom = iLines - 1;

	TRACE( TEXT("行頭空白を削除") );
	//	選択範囲は、操作した行全体を選択状態にする

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, iTop );	//	位置合わせ

	for( i = iTop; iBottom >= i; i++, itLine++ )	//	範囲内の各行について
	{
		//	文字があるなら操作する
		if( 0 != itLine->vcLine.size(  ) )
		{
			vcLtrItr = itLine->vcLine.begin( );
			ch = vcLtrItr->cchMozi;

			//	空白かつ半角ではない
			if( ( iswspace( ch ) && TEXT(' ') != ch ) )
			{
				SqnAppendLetter( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ch, 0, i, bFirst );
				bFirst = FALSE;

				DocIterateDelete( vcLtrItr, i );
			}
		}

		if( bSeled )
		{
			DocRangeSelStateToggle( -1, -1, i, 1 );	//	該当行全体を選択状態にする
			DocReturnSelStateToggle( i, 1 );	//	改行も選択で
		}

		DocBadSpaceCheck( i );	//	状態をリセット
		ViewRedrawSetLine( i );
	}

	//	キャレット位置ずれてたら適当に調整
	*pXdot = 0;
	DocLetterPosGetAdjust( pXdot, dLine, 0 );	//	キャレット位置適当に調整
	ViewDrawCaret( *pXdot, dLine, 1 );


	DocPageInfoRenew( -1, 1 );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------
/*!
	行末文字を削除する。ただし空白だったら削除しない
	@param[in]	pXdot	今のドット位置を受けて戻す・削除に巻き込まれた対応
	@param[in]	dLine	今の行数
	@return		HRESULT	終了状態コード
*/
HRESULT DocLastLetterErase( PINT pXdot, INT dLine )
{
	UINT_PTR	iLines;
	INT			iTop, iBottom, i, xDot = 0;
	TCHAR		ch;
	BOOLEAN		bFirst = TRUE, bSeled = FALSE;
	RECT		rect;

	LETR_ITR	vcLtrItr;

	LINE_ITR	itLine;

	//	範囲確認
	iLines  = DocNowFilePageLineCount( );
	iTop    = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	iBottom = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;
	if( 0 <= iTop &&  0 <= iBottom )	bSeled = TRUE;

	if( 0 > iTop )		iTop = 0;
	if( 0 > iBottom )	iBottom = iLines - 1;

	TRACE( TEXT("行末文字削除") );
	//	選択してる場合は、操作行を全選択状態にする
	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, iTop );	//	位置合わせ

	for( i = iTop; iBottom >= i; i++, itLine++ )	//	範囲内の各行について
	{
		//	文字があるなら操作する
		if( 0 != itLine->vcLine.size( ) )
		{
			vcLtrItr = itLine->vcLine.end( );
			vcLtrItr--;	//	終端の一個前が末端文字
			ch = vcLtrItr->cchMozi;

			rect.top    = i * LINE_HEIGHT;
			rect.bottom = rect.top + LINE_HEIGHT;

			if( !( iswspace( ch ) ) )
			{
				xDot  = DocLineParamGet( i, NULL, NULL );

				xDot -= vcLtrItr->rdWidth;

				SqnAppendLetter( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ch, xDot, i, bFirst );
				bFirst = FALSE;

				DocIterateDelete( vcLtrItr, i );

				rect.left  = xDot;
				rect.right = xDot + 40;	//	壱文字＋改行・適当でよろし

				ViewRedrawSetRect( &rect );	//	末端だけ書き換えればいい？

				DocBadSpaceCheck( i );	//	良くないスペースを調べておく
			}
		}

		if( bSeled )
		{
			DocRangeSelStateToggle( -1, -1, i , 1 );	//	該当行全体を選択状態にする
			DocReturnSelStateToggle( i, 1 );	//	改行も選択で
		}
	}

	//	キャレット位置適当に調整
	*pXdot = 0;
	DocLetterPosGetAdjust( pXdot, dLine, 0 );	//	キャレット位置適当に調整
	ViewDrawCaret( *pXdot, dLine, 1 );

	DocPageInfoRenew( -1, 1 );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	行末空白削除の面倒見る・選択行とか
	@param[in]	pXdot	今のドット位置を受けて戻す・削除に巻き込まれた対応
	@param[in]	dLine	今の行数
	@return		HRESULT	終了状態コード
*/
HRESULT DocLastSpaceErase( PINT pXdot, INT dLine )
{
	UINT_PTR	iLines;
	INT			iTop, iBottom, i, xDelDot, xMotoDot;
	BOOLEAN		bFirst = TRUE;
	LPTSTR		ptBuffer = NULL;
	RECT		rect;

	LINE_ITR	itLine;

	TRACE( TEXT("行末空白削除") );

	//	範囲確認
	iLines = DocNowFilePageLineCount( );
	iTop    = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	iBottom = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;

	if( 0 > iTop )		iTop = 0;
	if( 0 > iBottom )	iBottom = iLines - 1;


	ViewSelPageAll( -1 );	//	選択範囲無くなる

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, iTop );	//	位置合わせ

	for( i = iTop; iBottom >= i; i++, itLine++ )
	{
		xMotoDot = itLine->iDotCnt;
		ptBuffer = DocLastSpDel( &(itLine->vcLine) );
		xDelDot  = DocLineParamGet( i, NULL, NULL );	//	サクった後の行末端すなわち削除位置
		
		if( ptBuffer  )
		{
			SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ptBuffer, xDelDot, i , bFirst );
			bFirst = FALSE;
		}

		FREE( ptBuffer );

		DocBadSpaceCheck( i );	//	状態をリセット・中で行書換でいいか？

		rect.top    = i * LINE_HEIGHT;
		rect.bottom = rect.top + LINE_HEIGHT;
		rect.left   = xDelDot;	//	削ったら左側になる
		rect.right  = xMotoDot + 20;	//	元長さ＋改行マーク
		ViewRedrawSetRect( &rect );
//		ViewRedrawSetLine( i );	//	再描画COMMANDO
	}

	//	キャレット位置ずれてたら適当に調整
	DocLetterPosGetAdjust( pXdot, dLine, 0 );	//	キャレット位置適当に調整
	ViewDrawCaret( *pXdot, dLine, 1 );

	DocPageInfoRenew( -1, 1 );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	行末空白削除・コア部分
	@param[in]	*vcTgLine	該当する行のベクターへのポインタ～
	@return		削除した文字列・呼んだ方でfreeせよ
*/
LPTSTR DocLastSpDel( vector<LETTER> *vcTgLine )
{
	UINT_PTR	cchSize;
	LPTSTR		ptBuffer = NULL;
	wstring		wsDelBuf;
	LETR_ITR	itLtr, itDel;

	if( 0 >= vcTgLine->size( ) )	return NULL;

	itLtr = vcTgLine->end( );
	itLtr--;

	//	末尾から逆に見ていく
	for( ; itLtr != vcTgLine->begin(); itLtr-- )
	{
		if( !( iswspace( itLtr->cchMozi ) ) )	//	空白じゃなくなったら
		{
			itLtr++;	//	その次からがターゲット
			break;
		}
	}
	if( itLtr == vcTgLine->begin() )	//	先頭文字確認
	{
		//	空白じゃなくなったらその次からがターゲット
		if( !( iswspace( itLtr->cchMozi ) ) ){	itLtr++;	}
	}

	//	空白エリアがないなら特にすることはない
	if( itLtr == vcTgLine->end( ) ){	return NULL;	}

	wsDelBuf.clear();
	for( itDel = itLtr; vcTgLine->end( ) != itDel; itDel++ ){	wsDelBuf +=  itDel->cchMozi;	}

	cchSize = wsDelBuf.size( ) + 1;
	ptBuffer = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
	StringCchCopy( ptBuffer, cchSize, wsDelBuf.c_str( ) );

	//	該当部分を削除
	vcTgLine->erase( itLtr, vcTgLine->end( ) );

	return ptBuffer;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定範囲を削除する
	@param[in]		xDot		対象ドット
	@param[in]		yLine		対象行
	@param[in]		dBgnMozi	開始文字位置
	@param[in]		dEndMozi	終端文字位置
	@param[in,out]	pFirst		アンドゥ記録用
	@return	UINT	街頭部分の文字数
*/
UINT DocRangeDeleteByMozi( INT xDot, INT yLine, INT dBgnMozi, INT dEndMozi, PBOOLEAN pFirst )
{
	UINT_PTR	cchSize;
	INT			iBytes;
	LPTSTR		ptBuffer;
	LETR_ITR	vcLtrBgn, vcLtrEnd, vcItr;
	wstring		wsDelBuf;

	LINE_ITR	itLine;

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, yLine );

	vcLtrBgn  = itLine->vcLine.begin( );
	vcLtrEnd  = itLine->vcLine.begin( );
	vcLtrBgn += dBgnMozi;	//	該当位置まで移動して
	vcLtrEnd += dEndMozi;	//	そのエリアの終端も確認

	wsDelBuf.clear();
	iBytes = 0;
	for( vcItr = vcLtrBgn; vcLtrEnd != vcItr; vcItr++ )
	{
		wsDelBuf += vcItr->cchMozi;
		iBytes   += vcItr->mzByte;
	}

	//	該当部分を削除
	itLine->vcLine.erase( vcLtrBgn, vcLtrEnd );
	itLine->iByteSz -= iBytes;
	//	アンドゥバッファ作成
	cchSize = wsDelBuf.size( ) + 1;
	ptBuffer = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
	StringCchCopy( ptBuffer, cchSize, wsDelBuf.c_str( ) );
	SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ptBuffer, xDot, yLine, *pFirst );
	FREE( ptBuffer );	*pFirst = FALSE;

	return (UINT)(cchSize - 1);
}
//-------------------------------------------------------------------------------------------------

/*!
	全体又は選択範囲を右に寄せる
	@param[in]	pXdot	今のドット位置を受けて戻す
	@param[in]	dLine	今の行数
	@return		HRESULT	終了状態コード
*/
HRESULT DocRightSlide( PINT pXdot, INT dLine )
{
	UINT_PTR	iLines;
	INT			iTop, iBottom, i;
	INT			dSliDot, dRitDot, dPaDot, dInBgn;
	INT			dMozi, dLefDot, dAdDot;
	BOOLEAN		bFirst = TRUE;
	LPTSTR		ptBuffer = NULL;

	LINE_ITR	itLine;

	TRACE( TEXT("右寄せ") );

	//	右寄せ限界確認
	dSliDot = InitParamValue( INIT_LOAD, VL_RIGHT_SLIDE, 790 );

	//	範囲確認
	iLines = DocNowFilePageLineCount( );
	iTop    = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	iBottom = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;

	if( 0 > iTop )		iTop = 0;
	if( 0 > iBottom )	iBottom = iLines - 1;


	ViewSelPageAll( -1 );	//	選択範囲無くなる

	dRitDot = DocPageMaxDotGet( iTop, iBottom );	//	一番右のドット確認

	dPaDot = dSliDot - dRitDot;
	if( 0 > dPaDot )
	{
		NotifyBalloonExist( TEXT("はみ出してるみたい"), TEXT("失敗"), NIIF_ERROR );
		return E_FAIL;
	}

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, iTop );	//	位置合わせ

	for( i = iTop; iBottom >= i; i++, itLine++ )
	{
		dAdDot = dPaDot;
		//	行頭の開き状態を確認
		dLefDot = LayerHeadSpaceCheck( &(itLine->vcLine), &dMozi );
		if( 0 < dLefDot )
		{
			dAdDot += dLefDot;
			//	手前に空白があるなら、その分含めてずらし用スペースを計算
			DocRangeDeleteByMozi( 0, i, 0, dMozi, &bFirst );	bFirst = FALSE;
		}

		//	先頭からうめちゃう
		dInBgn = 0;
		ptBuffer = DocPaddingSpaceWithPeriod( dAdDot, NULL, NULL, NULL, TRUE );
		DocInsertString( &dInBgn, &i, NULL, ptBuffer, 0, bFirst );	bFirst = FALSE;
		FREE(ptBuffer);

		ViewRedrawSetLine( i );
	}

	//	キャレット位置適当に調整
	*pXdot = 0;
	DocLetterPosGetAdjust( pXdot, dLine, 0 );	//	キャレット位置適当に調整
	ViewDrawCaret( *pXdot, dLine, 1 );

	DocPageInfoRenew( -1, 1 );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ユニコードの使用不使用とか考慮しつつ埋め空白を作る
	@param[in]	dTgtDot	作成するドット数
	@return		LPTSTR	作成したスペースの文字列・開放は呼んだ方で面倒見る
*/
LPTSTR DocPaddingSpaceMake( INT dTgtDot )
{
	LPTSTR	ptReplc = NULL;
	INT		dZenSp, dHanSp, dUniSp;

	//	幅０だと作れない
	if( 0 >= dTgtDot )	return NULL;

	if( gbUniPad )
	{
		ptReplc = DocPaddingSpace( dTgtDot, &dZenSp, &dHanSp );
		//	作成不可だったり半角多すぎたら、ユニコード使って作り直し
		if( !(ptReplc) || (dZenSp < dHanSp) )	//	(dZenSp + 1)
		{
			FREE(ptReplc);
			ptReplc = DocPaddingSpaceUni( dTgtDot, &dZenSp, &dHanSp, &dUniSp );
		}
	}
	else	//	ユニコード空白使わないなら
	{
		ptReplc = DocPaddingSpaceWithGap( dTgtDot, &dZenSp, &dHanSp );
	}

	return ptReplc;
}
//-------------------------------------------------------------------------------------------------

/*!
	ＡＡ全体を、１dotずつずらす。文字なら空白に置き換えながら
	@param[in]	vk		方向・右か左か
	@param[in]	pXdot	今のドット位置を受けて戻す
	@param[in]	dLine	今の行数
	@return		HRESULT	終了状態コード
*/
HRESULT DocPositionShift( UINT vk, PINT pXdot, INT dLine )
{
	UINT_PTR	iLines, cchSz;
	INT			iTop, iBottom, i;
	INT			wid, iDot, iLin, iMzCnt;
	INT			iTgtWid, iLefDot, iRitDot;
	BOOLEAN		bFirst = TRUE, bSeled = FALSE, bDone = FALSE;
	BOOLEAN		bRight;	//	非０右へ　０左へ
	BOOLEAN		bIsSp;
	LPTSTR		ptRepl;
	TCHAR		ch, chOneSp;

	LPUNDOBUFF	pstUndoBuff;	

	LETR_ITR	vcLtrItr;
	LINE_ITR	itLine;



	chOneSp = gaatPaddingSpDotW[1][0];

	pstUndoBuff = &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog);


	TRACE( TEXT("全体ずらし") );

	if( VK_RIGHT == vk )		bRight = TRUE;
	else if( VK_LEFT == vk )	bRight = FALSE;
	else	return E_INVALIDARG;

	//	範囲確認
	iLines  = DocNowFilePageLineCount( );
	iTop    = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	iBottom = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;
	if( 0 <= iTop &&  0 <= iBottom )	bSeled = TRUE;

	if( 0 > iTop )		iTop = 0;
	if( 0 > iBottom )	iBottom = iLines - 1;

	//	そのままだと容量が狂う・一旦選択状態を解除して計算しなおす
	if( bSeled ){	DocPageSelStateToggle( -1 );	}

	//	壱行ずつ面倒見ていく
	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, iTop );	//	位置合わせ

	for( i = iTop; iBottom >= i; i++, itLine++ )
	{
		//	文字があるなら操作する
		if( 0 != itLine->vcLine.size(  ) )
		{
			//	先頭文字を確認
			vcLtrItr = itLine->vcLine.begin( );
			ch  = vcLtrItr->cchMozi;
			wid = vcLtrItr->rdWidth;	//	文字幅

			bDone = FALSE;

			if( !(iswspace(ch)) )	//	空白ではなく
			{
				if( bRight )	//	右ずらしなら
				{
					//	先頭に1dotスペース足せばおｋ
					DocInputLetter( 0, i, chOneSp );
					SqnAppendLetter( pstUndoBuff, DO_INSERT, chOneSp, 0, i, bFirst );	bFirst = FALSE;
					bDone = TRUE;	//	処理しちゃった
				}
				else	//	左イクなら、先頭文字を空白にして調整する
				{
					ptRepl = DocPaddingSpaceMake( wid );	//	必要な空白確保
					StringCchLength( ptRepl, STRSAFE_MAX_CCH, &cchSz );
					//	今の文字を削除
					SqnAppendLetter( pstUndoBuff, DO_DELETE, ch, 0, i, bFirst );	bFirst = FALSE;
					DocIterateDelete( vcLtrItr, i );
					//	そして先頭に空白をアッー！
					iDot = 0;	iLin = i;
					DocStringAdd( &iDot, &iLin, ptRepl, cchSz );
					SqnAppendString( pstUndoBuff, DO_INSERT, ptRepl, 0, i, bFirst );	bFirst = FALSE;
					FREE(ptRepl);	//	開放忘れないように
				}
			}
			//	この先Beginイテレータ無効

			if( !(bDone) )	//	未処理であるなら・この時点で、先頭文字は空白確定
			{
				//	空白範囲を確認
				iTgtWid = DocLineStateCheckWithDot( 0, i, &iLefDot, &iRitDot, NULL, &iMzCnt, &bIsSp );
				if( bRight )	iTgtWid++;	//	方向に合わせて
				else			iTgtWid--;	//	ドット数を求める
				if( 0 > iTgtWid )	iTgtWid = 0;	//	マイナスは無いと思うけど念のため

				ptRepl = DocPaddingSpaceMake( iTgtWid );	//	必要な空白確保
				//	ターゲット幅が０ならNULLなので、先頭文字の削除だけでおｋ

				DocRangeDeleteByMozi( 0, i, 0, iMzCnt, &bFirst );	//	元の部分削除して

				if( ptRepl )	//	必要な文字を入れる
				{
					StringCchLength( ptRepl, STRSAFE_MAX_CCH, &cchSz );
					iDot = 0;	iLin = i;
					DocStringAdd( &iDot, &iLin, ptRepl, cchSz );
					SqnAppendString( pstUndoBuff, DO_INSERT, ptRepl, 0, i, bFirst );	bFirst = FALSE;
					FREE(ptRepl);	//	開放忘れないように
				}
			}

			if( bSeled )	//	選択状態でヤッてたのなら、選択状態を維持する
			{
				DocRangeSelStateToggle( -1, -1, i , 1 );	//	該当行全体を選択状態にする
				//	次の行があるなら改行も選択で
				if( iBottom > i )	DocReturnSelStateToggle( i, 1 );
			}

			DocBadSpaceCheck( i );	//	状態をリセット
			ViewRedrawSetLine( i );
		}
	}

	if( bSeled )	//	選択範囲はり直し
	{
		(*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop    = iTop;
		(*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom = iBottom;
	}


	//	キャレット位置調整
	iDot = 0;
	DocLetterPosGetAdjust( &iDot, dLine, 0 );	//	キャレット位置適当に調整
	ViewDrawCaret( iDot, dLine, 1 );

	DocPageByteCount( gitFileIt, gixFocusPage, NULL, NULL );
	DocPageInfoRenew( -1, 1 );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	行頭半角空白をユニコードに変換・ファイルコア函数
*/
HRESULT DocHeadHalfSpaceExchange( HWND hWnd )
{
	UINT_PTR	iLines;
	INT			iTop, iBottom, i;
	INT			xDot;
	BOOLEAN		bFirst = TRUE, bSeled = FALSE;
	TCHAR		ch;

	LETR_ITR	vcLtrItr;
	LINE_ITR	itLine;


	//	範囲確認
	iLines  = DocNowFilePageLineCount( );
	iTop    = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	iBottom = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;
	if( 0 <= iTop &&  0 <= iBottom )	bSeled = TRUE;

	if( 0 > iTop )		iTop = 0;
	if( 0 > iBottom )	iBottom = iLines - 1;

	TRACE( TEXT("行頭半角をユニコードに") );

	ViewSelPageAll( -1 );	//	選択範囲無くなる

	//	容量計算、バッド空白の確認と再描画必要

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, iTop );	//	行の位置合わせ

	for( i = iTop; iBottom >= i; i++, itLine++ )	//	範囲内の各行について
	{
		//	文字があるなら操作する
		if( 0 != itLine->vcLine.size(  ) )
		{
			vcLtrItr = itLine->vcLine.begin( );
			ch = vcLtrItr->cchMozi;

			if( TEXT(' ') == ch )	//	半角なら
			{
				//	一旦削除して
				SqnAppendLetter( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_DELETE, ch, 0, i, bFirst );
				bFirst = FALSE;
				DocIterateDelete( vcLtrItr, i );

				//	先頭位置に5dotユニコード空白をアッー！。
				ch  = (TCHAR)0x2004;
				xDot = DocInputLetter( 0, i, ch );
				SqnAppendLetter( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), DO_INSERT, ch, 0, i, bFirst );
			}

			DocBadSpaceCheck( i );	//	状態をリセット
			ViewRedrawSetLine( i );
		}
	}

	//	キャレット位置はずれない

	DocPageInfoRenew( -1, 1 );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

#ifdef DOT_SPLIT_MODE
/*!
	キャレット位置から、左右に１dotずつずらす。文字なら空白に置き換えながら
	@param[in]	vk		方向・右か左か
	@param[in]	pXdot	今のドット位置を受けて戻す
	@param[in]	dLine	今の行数
	@return		HRESULT	終了状態コード
*/
HRESULT DocCentreWidthShift( UINT vk, PINT pXdot, INT dLine )
{
	UINT_PTR	iLines, cchSz;
	UINT		dRslt, dFirst;
	INT			iBaseDot, iTop, iBottom, iBufDot;
	INT			wid, iDot, iLin, iMzCnt;
	INT			iFnlDot;
	BOOLEAN		bSeled = FALSE;
	BOOLEAN		bRight;	//	非０右へ　０左へ
	LPTSTR		ptRepl;
	TCHAR		ch, chOneSp;

	LPUNDOBUFF	pstUndoBuff;	

	LETR_ITR	vcLtrItr;
	LINE_ITR	itLine;

	chOneSp = gaatPaddingSpDotW[1][0];	//	幅1dot・文字間に挿入

	//中心部分が空白なら、その空白を伸び縮みさせる。
	//文字と文字の間開けるなら、抽芯になってる字の左側を開けるようにする
	//潰すときは、抽芯字を空白に置き換えて、それを縮める

	pstUndoBuff = &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog);

	iBaseDot = *pXdot;	//	基準点、なるべく動かないようにせないかん
	TRACE( TEXT("中間ずらし %dDOT"), iBaseDot );

	iFnlDot = iBaseDot;

	if( VK_RIGHT == vk )		bRight = TRUE;
	else if( VK_LEFT == vk )	bRight = FALSE;
	else	return E_INVALIDARG;

	if(  0 == iBaseDot )	//	基準が０なら、全体左右ずらしってこと
	{
		return DocPositionShift( vk, pXdot, dLine );
	}

	//	範囲確認
	iLines  = DocNowFilePageLineCount( );
	iTop    = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	iBottom = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;
	if( 0 <= iTop &&  0 <= iBottom )	bSeled = TRUE;

	if( 0 > iTop )		iTop = 0;
	if( 0 > iBottom )	iBottom = iLines - 1;

	//	そのままだと容量が狂う・一旦選択状態を解除して計算しなおす
	if( bSeled ){	DocPageSelStateToggle( -1 );	}

	//	壱行ずつ面倒見ていく
	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, iTop );	//	位置合わせ

//なんか時々連続空白ができる

	dFirst = TRUE;
	//	順番に処理していく
	for( iLin = iTop; iBottom >= iLin; iLin++, itLine++ )
	{
		iDot = itLine->iDotCnt;
		if( iBaseDot >=  iDot ){	continue;	}
		//	操作位置に満たないのなら、何もする必要は無い

		//	操作開始
		iDot = iBaseDot;	//	調整位置確定
		iMzCnt = DocLetterPosGetAdjust( &iDot, iLin, -1 );	//	常に左側をみる
		//	操作する位置の文字確認

		//	該当位置が空白なら、伸び縮み兼用
		iBufDot = iDot;	//	値ズレるのでそのまま使うとイケない
		dRslt = DocSpaceDifference( vk, &iBufDot, iLin, dFirst );	//	iBufDotはズラしたら使えない
		if( dRslt  )	//	ズラし成功
		{
			if( iLin == dLine ){	iFnlDot =  iBaseDot;	}	//	ずらした後の位置の面倒見る
			dFirst = FALSE;
		}
		else	//	返り値０なら、文字なので処理を
		{
			vcLtrItr = itLine->vcLine.begin( );
			std::advance( vcLtrItr, iMzCnt );	//	注目位置の文字まで移動して
			ch  = vcLtrItr->cchMozi;
			wid = vcLtrItr->rdWidth;	//	該当の文字の幅を確認

			if( bRight )	//	右に動かす
			{
				if( iswspace( ch ) )	//	右側の文字は空白であったら
				{
					iBufDot = iDot + wid;	//	その空白を延ばす
					DocSpaceDifference( vk, &iBufDot, iLin, dFirst );	//	iBufDotは使えない
					if( iLin == dLine ){	iFnlDot =  iBaseDot;	}	//	ずらした後の位置の面倒見る
				}
				else
				{
					SqnAppendLetter( pstUndoBuff, DO_INSERT, chOneSp, iDot, iLin, dFirst );
					DocInputLetter( iDot, iLin, chOneSp );	//	その場所に1dotスペース足せばおｋ
					if( iLin == dLine ){	iFnlDot = iDot +  1;	}	//	ずらした後の位置の面倒見る
				}
				dFirst = FALSE;
			}
			else	//	左に動かす
			{
				if( iLin == dLine ){	iFnlDot =  iDot;	}	//	ずらす前の位置の面倒見る

				//	今の文字を削除
				SqnAppendLetter( pstUndoBuff, DO_DELETE, ch, iDot, iLin, dFirst );	dFirst = FALSE;
				DocIterateDelete( vcLtrItr , iLin );
				if( 2 <= wid )	//	幅が１なら、削除だけでおｋ
				{
					ptRepl = DocPaddingSpaceMake( wid-1 );	//	必要な空白確保
					StringCchLength( ptRepl , STRSAFE_MAX_CCH, &cchSz );	//	文字数確認
					SqnAppendString( pstUndoBuff, DO_INSERT, ptRepl, iDot, iLin, dFirst );	dFirst = FALSE;
					DocStringAdd( &iDot, &iLin, ptRepl, cchSz );	//	そして先頭に空白をアッー！
					FREE(ptRepl);	//	開放忘れないように
				}
			}
		}

		if( bSeled )	//	選択状態でヤッてたのなら、選択状態を維持する
		{
			if( iLin == iTop )
			{
				iDot = iBaseDot;
				DocLetterPosGetAdjust( &iDot, iLin, 0 );	//	キャレット位置適当に調整
				DocRangeSelStateToggle( iDot, -1, iLin , 1 );	//	該当行全体を選択状態にする
			}
			else
			{
				DocRangeSelStateToggle( -1, -1, iLin , 1 );	//	該当行全体を選択状態にする
			}

			//	次の行があるなら改行も選択で
			if( iBottom > iLin )	DocReturnSelStateToggle( iLin, 1 );
		}

		DocBadSpaceCheck( iLin );	//	状態をリセット
		ViewRedrawSetLine( iLin );
	}

	if( bSeled )	//	選択範囲はり直し
	{
		(*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop    = iTop;
		(*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom = iBottom;
	}

	//	キャレット位置調整
	DocLetterPosGetAdjust( &iFnlDot, dLine, 0 );	//	キャレット位置適当に調整
	*pXdot = iFnlDot;	//	位置を戻す
	ViewDrawCaret( iFnlDot, dLine, 1 );
	//	再描画
	DocPageByteCount( gitFileIt, gixFocusPage, NULL, NULL );
	DocPageInfoRenew( -1, 1 );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------
#endif

