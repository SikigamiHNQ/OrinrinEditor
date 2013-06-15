/*! @file
	@brief 操作のログを作ります・アンドゥリドゥ用
	このファイルは DocOperateLog.cpp です。
	@author	SikigamiHNQ
	@date	2011/05/21
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

extern FILES_ITR	gitFileIt;	//!<	今見てるファイルの本体
//#define gstFile	(*gitFileIt)	//!<	イテレータを構造体と見なす

extern INT		gixFocusPage;	//!<	注目中のページ・とりあえず０・０インデックス

static BOOLEAN	gbGroupUndo;	//!<	真ならグループアンドゥをする
//-------------------------------------------------------------------------------------------------

INT	SqnUndoExec( LPUNDOBUFF, PINT, PINT );
INT	SqnRedoExec( LPUNDOBUFF, PINT, PINT );
//-------------------------------------------------------------------------------------------------

/*!
	アンドゥを実行するのかを受ける
	@param[in,out]	pxDot	ドット位置
	@param[in,out]	pyLine	行
	@return		INT			改行処理があったかどうか
*/
INT DocUndoExecute( PINT pxDot, PINT pyLine )
{
	INT	iRslt = 0;

	iRslt = SqnUndoExec( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), pxDot, pyLine );

	DocModifyContent( TRUE );

	return iRslt;
}
//-------------------------------------------------------------------------------------------------

/*!
	リドゥを実行するのかを受ける
	@param[in,out]	pxDot	ドット位置
	@param[in,out]	pyLine	行
	@return		INT			改行処理があったかどうか
*/
INT DocRedoExecute( PINT pxDot, PINT pyLine )
{
	INT	iRslt = 0;

	iRslt = SqnRedoExec( &((*gitFileIt).vcCont.at( gixFocusPage ).stUndoLog), pxDot, pyLine );

	DocModifyContent( TRUE );

	return iRslt;
}
//-------------------------------------------------------------------------------------------------

/*!
	アンドゥの動作設定をいただく
	@return		HRESULT	終了状態コード
*/
HRESULT SqnSetting( VOID )
{
	gbGroupUndo = InitParamValue( INIT_LOAD, VL_GROUP_UNDO, 1 );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	頁データの初期化・頁作成時に呼ぶ
	@param[in]	pstBuff	属するアンドゥバッファ
	@return		HRESULT	終了状態コード
*/
HRESULT SqnInitialise( LPUNDOBUFF pstBuff )
{
	pstBuff->dNowSqn = 0;
	pstBuff->dTopSqn = 0;
	pstBuff->dGrpSqn = 0;

	pstBuff->vcOpeSqn.clear( );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	全ログ破壊
	@param[in]	pstBuff	属するアンドゥバッファ
	@return		HRESULT	終了状態コード
*/
HRESULT SqnFreeAll( LPUNDOBUFF pstBuff )
{
	UINT_PTR	iCount, i;

	iCount = pstBuff->vcOpeSqn.size( );

	for( i = 0; iCount > i; i++ )
	{
		free( pstBuff->vcOpeSqn.at( i ).ptText );
	}

	SqnInitialise( pstBuff );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	バッファ途中かどうか確認して、途中ならそこから後を削除
	@param[in]	pstBuff	属するアンドゥバッファ
	@return		HRESULT	終了状態コード
*/
HRESULT SqnNumberCheck( LPUNDOBUFF pstBuff )
{
	OPSQ_ITR	itOpe, itBuf;

	//	何も無し
	if( pstBuff->dNowSqn == pstBuff->dTopSqn )	return S_FALSE;

	//	何も無い
	if( 0 == pstBuff->dNowSqn ){	SqnFreeAll( pstBuff );	return  S_FALSE;	}

	TRACE( TEXT("UNDO BUF err %d %d"), pstBuff->dNowSqn, pstBuff->dTopSqn );

	itOpe = pstBuff->vcOpeSqn.end( );
	itOpe--;	//	最後のいっこ

	do
	{
		if( pstBuff->dNowSqn == itOpe->ixSequence ){	break;	}

		free( itOpe->ptText );
		itBuf = itOpe;
		itBuf--;
		pstBuff->vcOpeSqn.erase( itOpe );
		itOpe = itBuf;

	}
	while( itOpe != pstBuff->vcOpeSqn.begin( ) );

	pstBuff->dTopSqn = itOpe->ixSequence;

	TRACE( TEXT("UNDO BUF chk %d %d"), itOpe->ixSequence, pstBuff->dTopSqn );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	壱文字追加、又は削除
	@param[in]	pstBuff	属するアンドゥバッファ
	@param[in]	dCmd	操作種類
	@param[in]	ch		文字・削除のときは無効
	@param[in]	xDot	操作したドット位置
	@param[in]	yLine	操作した行数
	@param[in]	bAlone	単独かどうか・単独ら、グループシーケンス番号をインクリして記録
	@return		UINT	グループ番号
*/
UINT SqnAppendLetter( LPUNDOBUFF pstBuff, UINT dCmd, TCHAR ch, INT xDot, INT yLine, UINT bAlone )
{
	UINT	uRslt;
	TCHAR	atBuffer[3];

	ZeroMemory( atBuffer, sizeof(atBuffer) );
	atBuffer[0] = ch;

	//	壱文字固定なので、グループは常に孤独
	uRslt = SqnAppendString( pstBuff, dCmd, atBuffer, xDot, yLine, bAlone );

	DocModifyContent( TRUE );

	return uRslt;
}
//-------------------------------------------------------------------------------------------------

/*!
	矩形の文字列追加、又は削除を記録
	@param[in]	pstBuff	属するアンドゥバッファ
	@param[in]	dCmd	操作種類
	@param[in]	ptStr	文字列
	@param[in]	pstPt	操作した行位置とドット位置
	@param[in]	yLine	操作した行数
	@param[in]	bAlone	単独かどうか・単独ら、グループシーケンス番号をインクリして記録
	@return		UINT	先端番号？
*/
UINT SqnAppendSquare( LPUNDOBUFF pstBuff, UINT dCmd, LPCTSTR ptStr, LPPOINT pstPt, INT yLine, UINT bAlone )
{
	INT	i;
	UINT_PTR	cchMozi, cchSize;
	LPCTSTR		ptCaret, ptSprt;
	OPERATELOG	stOpe;

	//	アンドゥとかで最新位置がずれてたら、そこより新しいの破棄して付け足していく
	//	ここでシーケンス番号チェキ
	SqnNumberCheck( pstBuff );

	//	Group番号調整
	if( bAlone ){	pstBuff->dGrpSqn += 1;	}

	StringCchLength( ptStr, STRSAFE_MAX_CCH, &cchSize );

	ptCaret = ptStr;

	for( i = 0; yLine > i; i++ )	//	行数分回す
	{
		if( !( *ptCaret ) ){	break;	}

		pstBuff->dTopSqn += 1;	//	シーケンスは常時インクリ

		ZeroMemory( &stOpe, sizeof(OPERATELOG) );
		stOpe.dCommando  = dCmd;
		stOpe.ixSequence = pstBuff->dTopSqn;
		stOpe.ixGroup    = pstBuff->dGrpSqn;	//	

		stOpe.rdXdot  = pstPt[i].x;
		stOpe.rdYline = pstPt[i].y;

		ptSprt = StrStr( ptCaret, CH_CRLFW );	//	改行のところまで
		if( !(ptSprt) ){	ptSprt = ptStr + cchSize;	}
		//	末端まで改行がなかったら、末端文字の位置を入れる
		//	末端がピタリ改行になるはず
		cchMozi = ptSprt - ptCaret;	//	そこまでの文字数求めて

		cchMozi++;	//	ﾇﾙﾀｰﾐﾈｰﾀ分
		stOpe.cchSize = cchMozi;
		stOpe.ptText  = (LPTSTR)malloc( cchMozi * sizeof(TCHAR) );	//	必要分確保
		StringCchCopy( stOpe.ptText, cchMozi, ptCaret );

		pstBuff->vcOpeSqn.push_back( stOpe );

		ptCaret = NextLineW( ptSprt );	//	次の行の先頭に移動
	}

	pstBuff->dNowSqn = pstBuff->vcOpeSqn.size( );

	DocModifyContent( TRUE );

	return pstBuff->dNowSqn;
}
//-------------------------------------------------------------------------------------------------

/*!
	文字列追加、又は削除を記録
	@param[in]	pstBuff	属するアンドゥバッファ
	@param[in]	dCmd	操作種類
	@param[in]	ptStr	文字列
	@param[in]	xDot	操作したドット位置
	@param[in]	yLine	操作した行数
	@param[in]	bAlone	単独かどうか・単独ら、グループシーケンス番号をインクリして記録
	@return		UINT	先端番号？
*/
UINT SqnAppendString( LPUNDOBUFF pstBuff, UINT dCmd, LPCTSTR ptStr, INT xDot, INT yLine, UINT bAlone )
{
	UINT_PTR	cchSize;
	OPERATELOG	stOpe;

	//	アンドゥとかで最新位置がずれてたら、そこより新しいの破棄して付け足していく
	//	ここでシーケンス番号チェキ
	SqnNumberCheck( pstBuff );


	pstBuff->dTopSqn += 1;

	stOpe.dCommando  = dCmd;
	stOpe.ixSequence = pstBuff->dTopSqn;

	//	Group番号調整
	if( bAlone ){	pstBuff->dGrpSqn += 1;	}
	stOpe.ixGroup    = pstBuff->dGrpSqn;	//	グループ番号、０に成らないように注意

	stOpe.rdXdot  = xDot;
	stOpe.rdYline = yLine;

	//	入力した文字・もしくは削除した文字
	StringCchLength( ptStr, STRSAFE_MAX_CCH, &cchSize );
	stOpe.cchSize = cchSize;	//	文字数にはヌルターミネータ分は含めない
	cchSize++;	//	ヌルターミネータ分
	stOpe.ptText  = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );	//	必要分確保
	StringCchCopy( stOpe.ptText, cchSize, ptStr );

	pstBuff->vcOpeSqn.push_back( stOpe );

	pstBuff->dNowSqn = pstBuff->vcOpeSqn.size( );

	DocModifyContent( TRUE );

	return pstBuff->dNowSqn;
}
//-------------------------------------------------------------------------------------------------

/*!
	アンドゥを実行
	@param[in]	pstBuff	属するアンドゥバッファ
	@param[in]	pxDot	操作したドット位置
	@param[in]	pyLine	操作した行数
	@return		INT		改行したかどうか
*/
INT SqnUndoExec( LPUNDOBUFF pstBuff, PINT pxDot, PINT pyLine )
{
	OPSQ_ITR	itSqn;
	INT		xDot, yLine, iRslt = 0, dCrLf = 0, yPreLine = 0;
	UINT	dCmd, dGrp, dNow, cchSize;
	UINT	dPreGroup = 0;
	LPTSTR	ptStr;

	if( !(pstBuff) )	return 0;	//	安全対策

#ifdef DO_TRY_CATCH
	try{
#endif

	do
	{
		dNow  = pstBuff->dNowSqn;

		if( 0 >= dNow ){	return dCrLf;	}

		dNow--;	//	位置合わせ
		TRACE( TEXT("UNDO SQNUM:%u"), dNow );

		dCmd  = pstBuff->vcOpeSqn.at( dNow ).dCommando;
		dGrp  = pstBuff->vcOpeSqn.at( dNow ).ixGroup;
		xDot  = pstBuff->vcOpeSqn.at( dNow ).rdXdot;
		yLine = pstBuff->vcOpeSqn.at( dNow ).rdYline;
		
		if( 0 == dPreGroup )	//	１回目は初期化すればおｋ
		{
			dPreGroup = dGrp;
			yPreLine = yLine;
		}
		else	//	２回目以降
		{
			//	別グループなら即離脱
			if( dPreGroup != dGrp ){	break;	}

			//	複数行にわたっているなら、フラグ的にＯＮ
			if( yPreLine != yLine && 0 == dCrLf ){	dCrLf = 1;	}
		}

		TRACE( TEXT("UNDO CMD:%u GRP:%u  D:%d, L:%d"), dCmd, dGrp, xDot, yLine );

		ptStr = pstBuff->vcOpeSqn.at( dNow ).ptText;
//OutOfRangeは、始点ドットが狂ってる場合があるようだ
		switch( dCmd )
		{
			case  DO_INSERT:	//	挿入なら、該当範囲を削除
				StringCchLength( ptStr, STRSAFE_MAX_CCH, &cchSize );
				iRslt = DocStringErase( xDot, yLine, NULL, cchSize );
				break;

			case  DO_DELETE:	//	削除だったら、文字を入れる、すればいい
				iRslt = DocStringAdd( &xDot, &yLine, pstBuff->vcOpeSqn.at( dNow ).ptText, pstBuff->vcOpeSqn.at( dNow ).cchSize );
				break;

			default:	TRACE( TEXT("アンドゥエラー！　[%u]未知のコード"), dCmd );	return dCrLf;
		}

		*pxDot  = xDot;
		*pyLine = yLine;

		if( dCrLf < iRslt ){	dCrLf = iRslt;	}

		pstBuff->dNowSqn -= 1;

		if( !(gbGroupUndo) ){	break;	}	//	グループアンドゥしない

	}while( pstBuff->dNowSqn );

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return  ETC_MSG( ("etc error"), 0 );	}
#endif

	return dCrLf;
}
//-------------------------------------------------------------------------------------------------

/*!
	リドゥを実行
	@param[in]	pstBuff	属するアンドゥバッファ
	@param[in]	pxDot	操作したドット位置
	@param[in]	pyLine	操作した行数
	@return		INT		改行したかどうか
*/
INT SqnRedoExec( LPUNDOBUFF pstBuff, PINT pxDot, PINT pyLine )
{
	OPSQ_ITR	itSqn;
	INT		xDot, yLine, iRslt = 0, dCrLf = 0, yPreLine = 0;
	UINT	dCmd, dGrp, dNow, cchSize;
	UINT	dPreGroup = 0;
	LPTSTR	ptStr;

#ifdef DO_TRY_CATCH
	try{
#endif

	do
	{
		dNow  = pstBuff->dNowSqn;
		if( dNow == pstBuff->vcOpeSqn.size( ) ){	return 0;	}

		//dNow++;	//	位置合わせ
		TRACE( TEXT("REDO SQNUM:%u"), dNow );

		dCmd  = pstBuff->vcOpeSqn.at( dNow ).dCommando;
		dGrp  = pstBuff->vcOpeSqn.at( dNow ).ixGroup;
		xDot  = pstBuff->vcOpeSqn.at( dNow ).rdXdot;
		yLine = pstBuff->vcOpeSqn.at( dNow ).rdYline;

		if( 0 == dPreGroup )	//	１回目は初期化すればおｋ
		{
			dPreGroup = dGrp;
			yPreLine = yLine;
		}
		else	//	２回目以降
		{
			//	別グループなら即離脱
			if( dPreGroup != dGrp ){	break;	}

			//	複数行にわたっているなら、フラグ的にＯＮ
			if( yPreLine != yLine && 0 == dCrLf ){	dCrLf = 1;	}
		}

		TRACE( TEXT("REDO CMD:%u GRP:%u  D:%d, L:%d"), dCmd, dGrp, xDot, yLine );

		ptStr = pstBuff->vcOpeSqn.at( dNow ).ptText;

		switch( dCmd )	//	リドゥの場合は素直に処理すればいい
		{
			case  DO_INSERT:
				iRslt = DocStringAdd( &xDot, &yLine, pstBuff->vcOpeSqn.at( dNow ).ptText, pstBuff->vcOpeSqn.at( dNow ).cchSize );
				break;

			case  DO_DELETE:
				StringCchLength( ptStr, STRSAFE_MAX_CCH, &cchSize );
				iRslt = DocStringErase( xDot, yLine, NULL, cchSize );
				break;

			default:	TRACE( TEXT("リドゥエラー！　[%u]未知のコード"), dCmd );	return dCrLf;
		}

		*pxDot  = xDot;
		*pyLine = yLine;

		if( dCrLf < iRslt ){	dCrLf = iRslt;	}

		pstBuff->dNowSqn += 1;

		if( !(gbGroupUndo) ){	break;	}	//	グループアンドゥしない

	}while( pstBuff->dNowSqn != pstBuff->vcOpeSqn.size( ) );

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return  ETC_MSG( ("etc error"), 0 );	}
#endif

	return dCrLf;
}
//-------------------------------------------------------------------------------------------------

