/*! @file
	@brief 左右反転、上下反転します
	このファイルは DocInverseCtrl.cpp です。
	@author	SikigamiHNQ
	@date	2012/04/11
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

//	反転用パーツは、必要になったらロードするようにする



#define IV_MIRROR	1
#define IV_UPSET	0


typedef struct tagINVERSEPARTS
{
	TCHAR	atSrcStr[MIN_STRING];	//!<	元文字列
	TCHAR	atDestStr[MIN_STRING];	//!<	変換後文字列

} INVERSEPARTS, *LPINVERSEPARTS;
//-------------------------------------------------------------------------------------------------

extern list<ONEFILE>	gltMultiFiles;	//	複数ファイル保持
extern FILES_ITR	gitFileIt;		//	今見てるファイルの本体
extern INT			gixFocusPage;	//	注目中のページ・とりあえず０・０インデックス

static  vector<INVERSEPARTS>	gvcMirrorParts;	//!<	左右反転用入替パーツ
static  vector<INVERSEPARTS>	gvcUpsetParts;	//!<	上下反転用入替パーツ
typedef vector<INVERSEPARTS>::iterator	PARTS_ITR;
//-------------------------------------------------------------------------------------------------


HRESULT	DocMirrorTranceLine( INT, INT );
HRESULT	DocMirrorTranceBox( INT, INT );

HRESULT	DocUpsetTranceLine( INT, INT );
HRESULT	DocUpsetTranceBox( INT, INT );

LPTSTR	SeledTextAlloc( LINE_ITR, PINT, PINT );

HRESULT	InversePartsLoad( UINT );
UINT	InversePartsCheck( UINT, LPCTSTR, LPTSTR, UINT_PTR );
//-------------------------------------------------------------------------------------------------

/*!
	初期化と破棄
	@param[in]	dMode	非０初期化　０破棄
	@return		HRESULT	終了状態コード
*/
HRESULT DocInverseInit( UINT dMode )
{

	if( dMode )
	{


	}
	else
	{
		gvcMirrorParts.clear( );
		gvcUpsetParts.clear(  );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

#define INV_ITEMS	3
/*!
	反転パーツをロード
	@param[in]	dMode	非０左右　０上下
	@return		HRESULT	終了状態コード
*/
HRESULT InversePartsLoad( UINT dMode )
{
	CONST WCHAR rtHead = 0xFEFF;	//	ユニコードテキストヘッダ
	WCHAR	rtUniBuf;

	HANDLE	hFile;
	DWORD	readed;

	LPVOID	pBuffer;	//	文字列バッファ用ポインター
	INT		iByteSize;

	LPTSTR	ptString;
	LPSTR	pcText;
	UINT	cchSize, cchLen;
	TCHAR	atFileName[MAX_PATH];

	TCHAR	atBuff[INV_ITEMS][MIN_STRING];
	INT		nYct, nXct;
	UINT	caret, dItem;
	//BOOLEAN	bEmpty = FALSE;


	INVERSEPARTS	stData;

	UINT_PTR	loop;
	list<INVERSEPARTS>	ltParts;	//	パーツ読込バッファ
	list<INVERSEPARTS>::iterator	itParts, itPtPos;	//	バッファのイテレータ


	StringCchCopy( atFileName, MAX_PATH, ExePathGet( ) );
	PathAppend( atFileName, TEMPLATE_DIR );

	if( dMode  )	//	左右
	{
		if( 1 <= gvcMirrorParts.size( ) )	return S_FALSE;

		PathAppend( atFileName, AA_MIRROR_FILE );
	}
	else	//	上下
	{
		if( 1 <= gvcUpsetParts.size(  ) )	return S_FALSE;

		PathAppend( atFileName, AA_UPSET_FILE );
	}

	hFile = CreateFile( atFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile ){	return E_HANDLE;	}

	iByteSize = GetFileSize( hFile, NULL );
	pBuffer = malloc( iByteSize + 2 );
	ZeroMemory( pBuffer, iByteSize + 2 );

	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	ReadFile( hFile, pBuffer, iByteSize, &readed, NULL );
	CloseHandle( hFile );	//	内容全部取り込んだから開放

	//	ユニコードチェック
	CopyMemory( &rtUniBuf, pBuffer, 2 );
	if( rtHead == rtUniBuf )	//	ユニコードヘッダがあれば
	{	//	普通はユニコードじゃない
		ptString = (LPTSTR)pBuffer;
		ptString++;	//	ユニコードヘッダ分進めておく
	}
	else	//	多分SJISであるなら
	{
		pcText = (LPSTR)pBuffer;
		ptString = SjisDecodeAlloc( pcText );	//	SJISの内容をユニコードにする

		FREE( pBuffer );	//	こっちで開放
		pBuffer = ptString;	//	ポイントするところを変更
	}

	StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSize );

	//	取込
	ZeroMemory( atBuff, sizeof(atBuff) );

	//スタイリッシュに本体読込
	dItem = 0;
	for( nYct = 0, nXct = 0, caret = 0; caret <= cchSize; caret++, nXct++ )
	{
		if( MIN_STRING <= nXct ){	nXct = MIN_STRING - 1;	}
		if( INV_ITEMS  <= nYct ){	nYct = INV_ITEMS - 1;	}
 
		atBuff[nYct][nXct] = ptString[caret];

		//	改行までを１セットとし、全角空白で区切って読む
		if( (TEXT('\r') == ptString[caret] && TEXT('\n') == ptString[caret + 1]) || 0x0000 == ptString[caret] )
		{
			//bEmpty = FALSE;
			//if(0 == nXct && 0 == nYct ){	bEmpty = TRUE;	}	//	ラスト改行ありならここに入る・文字列無しなら何もしない

			atBuff[nYct][nXct] = 0x0000;	//	文字列の終点はヌル
			nXct = -1;	//	ループ頭で＋＋されるため、０にあうようにしておく
			nYct = 0;	//	次のエレメントにスタイリッシュに移る
			caret++;	//	改行コード分をエレガントに進める

			//if( (0x0000 != atBuff[0][0]) && (TEXT('\r') != atBuff[0][0]) && !(bEmpty) )	//	内容があるのなら
			if( (0x0000 != atBuff[0][0]) && (0 != dItem) )
			{
				//	壱行目はヘッダである

				ZeroMemory( &stData, sizeof(INVERSEPARTS) );	//	クリンナップ
				StringCchCopy( stData.atSrcStr,  MIN_STRING, atBuff[0] );
				StringCchCopy( stData.atDestStr, MIN_STRING, atBuff[1] );
				ltParts.push_back( stData );	//	まずバッファへ

				//	反対向きも必要
				ZeroMemory( &stData, sizeof(INVERSEPARTS) );
				StringCchCopy( stData.atSrcStr,  MIN_STRING, atBuff[1] );
				StringCchCopy( stData.atDestStr, MIN_STRING, atBuff[0] );
				ltParts.push_back( stData );	//	まずバッファへ

			}//内容が有るかどうか

			dItem++;
			ZeroMemory( atBuff , sizeof(atBuff) );	//	クリンナップ
		}

		if( TEXT('　') == ptString[caret] )
		{
			atBuff[nYct][nXct] = 0x0000;	//	文字列の終点はヌル
			nXct = -1;	//	ループ頭で＋＋されるため、０にあうようにしておく
			nYct++;		//	次のParameterのために
		}
	}

	FREE( pBuffer );

	//	文字数の順番に並べ直すべし
	cchSize = 0;

	loop = ltParts.size();
	while( loop )	//	全体をみないかん
	{
		itParts = ltParts.begin();	//	とりあえず１個目
		StringCchLength( itParts->atSrcStr, MIN_STRING , &cchSize );	//	文字数確認

		for( itPtPos = ltParts.begin(); ltParts.end() != itPtPos; itPtPos++ )
		{
			StringCchLength( itPtPos->atSrcStr, MIN_STRING, &cchLen );
			if( cchSize <  cchLen ){	itParts = itPtPos;	}	//	文字数多かったら変更
		}

		ZeroMemory( &stData, sizeof(INVERSEPARTS) );	//	クリンナップ
		StringCchCopy( stData.atSrcStr,  MIN_STRING, itParts->atSrcStr );
		StringCchCopy( stData.atDestStr, MIN_STRING, itParts->atDestStr );
		if( dMode ){	gvcMirrorParts.push_back( stData );	}	//	左右
		else{			gvcUpsetParts.push_back( stData );	}	//	上下

		ltParts.erase( itParts );	//	記録したらそれは消す

		loop = ltParts.size();	//	残りがあるか
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定行の選択範囲をテキストで確保する・freeは呼んだ方でやる
	@param[in]	itLine	指定行のイテレータ
	@param[out]	*piDot	選択範囲開始位置ドットを返す
	@param[out]	*piMozi	選択範囲のユニコード文字数を返す
	@return				確保したユニコード文字列
*/
LPTSTR SeledTextAlloc( LINE_ITR itLine, PINT piDot, PINT piMozi )
{
	UINT_PTR	j, dLetters;
	INT			iDot, iMozi;
	INT			iSelDot;
	LPTSTR		ptString = NULL;

	UINT_PTR	cchSz;

	wstring	wsSrcBuff;

	//	その行の選択範囲を確保する
	dLetters = itLine->vcLine.size( );

	wsSrcBuff.clear();
	iSelDot = -1;	//	選択開始ドットを記録
	iDot = 0;
	iMozi = 0;

	for( j = 0; dLetters > j; j++ )
	{
		//	選択されている部分を文字列に確保
		if( CT_SELECT & itLine->vcLine.at( j ).mzStyle )
		{
			wsSrcBuff += itLine->vcLine.at( j ).cchMozi;

			iMozi++;
			if( 0 > iSelDot )	iSelDot = iDot;
		}

		iDot +=  itLine->vcLine.at( j ).rdWidth;	//	そこまでのドット数確認
	}

	cchSz = wsSrcBuff.size();
	ptString = (LPTSTR)malloc( (cchSz+2) * sizeof(TCHAR) );
	if( ptString )	StringCchCopy( ptString, (cchSz+2), wsSrcBuff.c_str() );

	if( 0 > iSelDot )	iSelDot = 0;

	if( piDot  )	*piDot  = iSelDot;
	if( piMozi )	*piMozi = iMozi;


	return ptString;
}
//-------------------------------------------------------------------------------------------------

/*!
	選択範囲のＡＡを反転する
	@param[in]	dStyle	矩形かどうか
	@param[in]	dMode	非０左右　０上下
	@param[in]	pXdot	今のドット位置を受けて戻す
	@param[in]	dLine	今の行数
	@return		HRESULT	終了状態コード
*/
HRESULT DocInverseTransform( UINT dStyle, UINT dMode, PINT pXdot, INT dLine )
{
	INT_PTR	iLines;
	INT		iTop, iBtm, iInX;

#ifdef DO_TRY_CATCH
	try{
#endif

	iLines = DocNowFilePageLineCount( );	//	ページ全体の行数

	//	開始地点から開始	//	D_SQUARE
	iTop = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineTop;
	iBtm = (*gitFileIt).vcCont.at( gixFocusPage ).dSelLineBottom;
	if( 0 >  iTop ){	iTop = 0;	}
	if( 0 >  iBtm ){	iBtm = iLines - 1;	}

	//	末端を確認・内容がないなら、使用行戻す
	iInX = DocLineParamGet( iBtm, NULL, NULL );
	if( 0 == iInX ){	 iBtm--;	}

	//	はみ出しないか
	if( iLines <= iTop || iLines <= iBtm )	return E_OUTOFMEMORY;

	//	矩形なら、各行毎に反転処理・全体なら、ＭＡＸ幅に合わせる
	if( dStyle & D_SQUARE )
	{
		if( dMode ){	DocMirrorTranceBox( iTop, iBtm );	}	//	左右
		else{			DocUpsetTranceBox( iTop, iBtm );	}	//	上下
	}
	else
	{
		if( dMode ){	DocMirrorTranceLine( iTop, iBtm );	}	//	左右
		else{			DocUpsetTranceLine( iTop, iBtm );	}	//	上下
	}
	ViewSelPageAll( -1 );

	DocLetterPosGetAdjust( pXdot, dLine, 0 );	//	カーソル位置を適当に補正
	ViewDrawCaret( *pXdot, dLine, 1 );


#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	通常選択のときの左右反転処理
	@param[in]	iTop	開始行
	@param[in]	iBtm	終了行
	@return		HRESULT	終了状態コード
*/
HRESULT DocMirrorTranceLine( INT iTop, INT iBtm )
{
	INT_PTR		iLns;
	INT			iPadd, baseDot, iBytes;
	INT			iDot, iGyou, iMzDot;
	LPTSTR		ptPadd;
	LPTSTR		ptInvStr;
	LPTSTR		ptString = NULL;

	UINT_PTR	cchSz;
	UINT		d;
	TCHAR		atBuff[MIN_STRING];

	BOOLEAN		bFirst = TRUE;

	wstring	wsInvBuff;

	LINE_ITR	itLine;


#ifdef DO_TRY_CATCH
	try{
#endif

	InversePartsLoad( IV_MIRROR );	//	左右のパーツ確認

	//	選択範囲内でもっとも長いドット数を確認
	baseDot = DocPageMaxDotGet( iTop, iBtm );

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, iTop );

	//	左右反転なら、壱行ずつ見ていけばいい
	for( iLns = iTop; iBtm >= iLns; iLns++ )
	{
		wsInvBuff.clear(  );

		iMzDot = DocLineParamGet( iLns, NULL, NULL );
		if( 0 >= iMzDot )	continue;	//	その行の内容がないなら何もしないでおｋ

		iPadd = baseDot - iMzDot;	//	埋めに必要な幅確保
		ptPadd = DocPaddingSpaceMake( iPadd );	//	そこまでを埋める

		//	該当行を確保して
		iBytes = DocLineTextGetAlloc( gitFileIt, gixFocusPage, D_UNI, iLns, (LPVOID *)(&ptString) );
		if( 0 < iBytes )
		{
			StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSz );
			for( d = 0; cchSz > d; )	//	順番に見ていく
			{
				d += InversePartsCheck( IV_MIRROR, &(ptString[d]), atBuff, MIN_STRING );
				wsInvBuff.insert( 0, atBuff );
			}
		}
		FREE( ptString );

		//	元文字列と入れ替え
		cchSz = wsInvBuff.size() + 2;	//	使い回し注意
		ptInvStr = (LPTSTR)malloc( cchSz * sizeof(TCHAR) );
		StringCchCopy( ptInvStr, cchSz, wsInvBuff.c_str() );

		DocLineErase( iLns, &bFirst );	//	先ずその行の内容を削除して
		iDot = 0;	iGyou = iLns;
		if( ptPadd ){	DocInsertString( &iDot, &iGyou, NULL, ptPadd, 0, bFirst );	bFirst  = FALSE;	}
		DocInsertString( &iDot, &iGyou, NULL, ptInvStr, 0, bFirst );	bFirst  = FALSE;
		//	埋め分を書き込んで、ひっくり返った文字列を書き込めばいい

		FREE( ptPadd );
		FREE( ptInvStr );
	}
	//元文字列を先頭から見ていく。文字列として一個ずつ比較し、ヒットしたら
	//入れ替えてDESTの先頭に記録。次々先頭に入れれば左右反転したのができる。
	//ヒットしなかったら元文字をそのまま記録。

	//	末端空白削除が必要

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	矩形選択のときの左右反転処理
	@param[in]	iTop	開始行
	@param[in]	iBtm	終了行
	@return		HRESULT	終了状態コード
*/
HRESULT DocMirrorTranceBox( INT iTop, INT iBtm )
{
	INT_PTR		iLns;
	INT			iGyou;
	INT			iSelDot, iMozi;
	LPTSTR		ptInvStr;
	LPTSTR		ptString = NULL;

	UINT_PTR	cchSz;
	UINT		d;
	TCHAR		atBuff[MIN_STRING];

	BOOLEAN		bFirst = TRUE;

	wstring	wsInvBuff, wsSrcBuff;

	LINE_ITR	itLine;


#ifdef DO_TRY_CATCH
	try{
#endif

	InversePartsLoad( IV_MIRROR );	//	左右のパーツ確認

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, iTop );

	//	左右反転なら、壱行ずつ見ていけばいい
	for( iLns = iTop; iBtm >= iLns; iLns++ )
	{
		wsInvBuff.clear(  );

		//	その行の選択範囲を確保する
		ptString = SeledTextAlloc( itLine, &iSelDot, &iMozi );	itLine++;
		StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSz );

		//	内容を逆転させる
		if( 0 <  cchSz )	//	その行の中身があったら
		{
			for( d = 0; cchSz > d; )
			{
				d += InversePartsCheck( IV_MIRROR, &(ptString[d]), atBuff, MIN_STRING );
				wsInvBuff.insert( 0, atBuff );
			}

			//	元内容削除してアンドゥ記録
			DocStringErase( iSelDot, iLns, NULL, iMozi );
			SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog) , DO_DELETE, ptString, iSelDot, iLns, bFirst );	bFirst = FALSE;
			FREE( ptString );	//	アンドゥ記録してから削除セヨ

			cchSz = wsInvBuff.size() + 2;	//	使い回し注意
			ptInvStr = (LPTSTR)malloc( cchSz * sizeof(TCHAR) );
			StringCchCopy( ptInvStr, cchSz, wsInvBuff.c_str() );

			//	ひっくり返した文字列を挿入
			iGyou = iLns;
			DocInsertString( &iSelDot, &iGyou, NULL, ptInvStr, 0, FALSE );

			FREE( ptInvStr );
		}
	}


#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	通常選択のときの上下反転処理
	@param[in]	iTop	開始行
	@param[in]	iBtm	終了行
	@return		HRESULT	終了状態コード
*/
HRESULT DocUpsetTranceLine( INT iTop, INT iBtm )
{

	INT_PTR		iLns;
	INT			iBytes;
	INT			iDot, iGyou;
	LPTSTR		ptInvStr;
	LPTSTR		ptString = NULL;

	UINT_PTR	cchSz, d, dL;
	TCHAR		atBuff[MIN_STRING];

	BOOLEAN		bFirst = TRUE;

	LINE_ITR	itLine;

	wstring	wsInvBuff;
	vector<wstring>	vcUpset;	//	変換結果の一時保存

#ifdef DO_TRY_CATCH
	try{
#endif

	InversePartsLoad( IV_UPSET );	//	上下のパーツ確認

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, iTop );	//	開始行まで移動

	iLns = (iBtm - iTop) + 1;	//	全体行数
	vcUpset.resize( iLns  );	//	先に確保しておく

	//	上から処理していく
	for( iLns = iTop, dL = 0; iBtm >= iLns; iLns++, dL++ )
	{
		vcUpset.at( dL ).clear( );

		//	該当行を確保して
		iBytes = DocLineTextGetAlloc( gitFileIt, gixFocusPage, D_UNI, iLns, (LPVOID *)(&ptString) );
		if( 0 < iBytes )
		{
			//	前から順番に反転文字と入れ替えていく
			StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSz );
			for( d = 0; cchSz > d; )
			{
				d += InversePartsCheck( IV_UPSET, &(ptString[d]), atBuff, MIN_STRING );
				vcUpset.at( dL ) += wstring(atBuff);	//wsInvBuff.push_back( atBuff );
				//	とりあえず順番にいれておいて、書き込む時にreverseすればいい
			}
		}
		FREE( ptString );
	}

	for( iLns = iTop, dL = vcUpset.size()-1; iBtm >= iLns; iLns++, dL-- )
	{
		DocLineErase( iLns, &bFirst );	//	先ずその行の内容を削除して

		cchSz = vcUpset.at( dL ).size( );	//	内容があれば処理する
		if( 0 < cchSz )
		{
			cchSz += 2;
			ptInvStr = (LPTSTR)malloc( cchSz * sizeof(TCHAR) );
			StringCchCopy( ptInvStr, cchSz, vcUpset.at( dL ).c_str( ) );

			iDot = 0;	iGyou = iLns;
			DocInsertString( &iDot, &iGyou, NULL, ptInvStr, 0, bFirst );	bFirst  = FALSE;

			FREE( ptInvStr );
		}
	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	矩形選択のときの上下反転処理
	@param[in]	iTop	開始行
	@param[in]	iBtm	終了行
	@return		HRESULT	終了状態コード
*/
HRESULT DocUpsetTranceBox( INT iTop, INT iBtm )
{
	INT_PTR		iLns;
	INT			iGyou;
	INT			iSelDot, iMozi;
	LPTSTR		ptInvStr;
	LPTSTR		ptString = NULL;

	UINT_PTR	cchSz, d, dL;
	TCHAR		atBuff[MIN_STRING];

	BOOLEAN		bFirst = TRUE;

	LINE_ITR	itLine, itStart;

	vector<wstring>	vcUpset;	//	変換結果の一時保存

#ifdef DO_TRY_CATCH
	try{
#endif
	InversePartsLoad( IV_UPSET );	//	上下のパーツ確認

	itLine = (*gitFileIt).vcCont.at( gixFocusPage ).ltPage.begin();
	std::advance( itLine, iTop );	//	開始行まで移動
	itStart = itLine;

	iLns = (iBtm - iTop) + 1;	//	全体行数
	vcUpset.resize( iLns  );	//	先に確保しておく

	//	上から壱行ずつ見ていく
	for( iLns = iTop, dL = 0; iBtm >= iLns; iLns++, dL++ )
	{
		vcUpset.at( dL ).clear( );

		//	その行の選択範囲を確保する
		ptString = SeledTextAlloc( itLine, &iSelDot, &iMozi );	itLine++;
		StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSz );

		//	内容を逆転させる
		if( 0 <  cchSz )	//	その行の中身があったら
		{
			//	前から順番に反転文字と入れ替えていく
			for( d = 0; cchSz > d; )
			{
				d += InversePartsCheck( IV_UPSET, &(ptString[d]), atBuff, MIN_STRING );
				vcUpset.at( dL ) += wstring(atBuff);	//wsInvBuff.push_back( atBuff );
				//	とりあえず順番にいれておいて、書き込む時にreverseすればいい
			}
		}
		FREE( ptString );
	}

	//	上から入れ替えていく
	itLine = itStart;
	for( iLns = iTop, dL = vcUpset.size()-1; iBtm >= iLns; iLns++, dL-- )
	{
		//	改めて選択範囲確保
		ptString = SeledTextAlloc( itLine, &iSelDot, &iMozi );	itLine++;
		if( 0 != iMozi )
		{
			//	元内容削除してアンドゥ記録
			DocStringErase( iSelDot, iLns, NULL, iMozi );
			SqnAppendString( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog) , DO_DELETE, ptString, iSelDot, iLns, bFirst );	bFirst = FALSE;
			FREE( ptString );	//	アンドゥ記録してから削除セヨ
		}

		cchSz = vcUpset.at( dL ).size() + 2;	//	使い回し注意
		ptInvStr = (LPTSTR)malloc( cchSz * sizeof(TCHAR) );
		StringCchCopy( ptInvStr, cchSz, vcUpset.at( dL ).c_str() );

		//	元文字列に該当する場所がなかったら、末端にしておく
		if( 0 == iMozi ){	iSelDot = DocLineParamGet( iLns, NULL, NULL );	}

		//	ひっくり返した文字列を挿入
		iGyou = iLns;
		DocInsertString( &iSelDot, &iGyou, NULL, ptInvStr, 0 , bFirst );	bFirst = FALSE;
	
		FREE( ptInvStr );
	}


#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	元文字列を受けて、変換結果を戻す
	@param[in]	dMode		非０左右　０上下
	@param[in]	ptSource	元文字列
	@param[out]	ptOutput	先頭文字の変換結果を戻すバッファ
	@param[in]	cchSz		バッファの文字数
	@return		UINT		元文字列の、変換した文字数。通常１
*/
UINT InversePartsCheck( UINT dMode, LPCTSTR ptSource, LPTSTR ptOutput, UINT_PTR cchSz )
{
	UINT_PTR	dParts;
	UINT_PTR	cchPrt = 1;
	PARTS_ITR	itParts, itBegin, itEnd;

	if( dMode )
	{
		dParts  = gvcMirrorParts.size();
		itBegin = gvcMirrorParts.begin();
		itEnd   = gvcMirrorParts.end();
	}
	else
	{
		dParts  = gvcUpsetParts.size();
		itBegin = gvcUpsetParts.begin();
		itEnd   = gvcUpsetParts.end();
	}

	ZeroMemory( ptOutput, cchSz * sizeof(TCHAR) );

	ptOutput[0] = ptSource[0];	//	デフォルト

	//	変換テーブルが無いならそのままコピって戻せばいい
	if( 0 == dParts ){	return 1;	}

	//	全文字をチェック
	for( itParts = itBegin; itEnd != itParts; itParts++ )
	{
		StringCchLength( itParts->atSrcStr, MIN_STRING, &cchPrt );
		//	テーブルの文字列と比較していく
		if( 0 == StrCmpN( ptSource, itParts->atSrcStr, cchPrt ) )
		{
			StringCchCopy( ptOutput, cchSz, itParts->atDestStr );
			break;
		}
		//	ヒットしたら、変換語文字列をコピーして終わり
	}

//	StringCchLength( ptOutput, cchSz, &cchPrt );	//	長さ確認・しちゃいかん

	return cchPrt;
}
//-------------------------------------------------------------------------------------------------

