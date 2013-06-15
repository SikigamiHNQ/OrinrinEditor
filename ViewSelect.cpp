/*! @file
	@brief 範囲選択の管理をします
	このファイルは ViewSelect.cpp です。
	@author	SikigamiHNQ
	@date	2011/04/22
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

//	TODO:	ALT押しながらカーソル、マウス動かしたらそのときだけ矩形選択になるようにする


extern  HWND	ghPrntWnd;		//	親ウインドウハンドル
extern  HWND	ghViewWnd;		//	このウインドウのハンドル

extern INT		gdDocXdot;		//	キャレットのＸドット・ドキュメント位置
extern INT		gdDocLine;		//	キャレットのＹ行数・ドキュメント位置

//	画面サイズを確認して、移動によるスクロールの面倒みる
extern INT		gdHideXdot;		//	左の隠れ部分
extern INT		gdViewTopLine;	//	表示中の最上部行番号
extern SIZE		gstViewArea;	//	表示領域のサイズ・ルーラー等の領域は無し
extern INT		gdDispingLine;	//	見えてる行数・中途半端に見えてる末端は含まない

//	これらのキーの具合は、GetKeyStateもしくはGetKeyboardStateを使えばいい
extern BOOLEAN	gbShiftOn;		//	シフトが押されている
extern BOOLEAN	gbCtrlOn;		//	コントロールが押されている
extern BOOLEAN	gbAltOn;		//	アルタが押されている

extern BOOLEAN	gbExtract;	

extern POINT	gstCursor;		//	文字を考慮しない、Cursorのドット＆行位置・大域変数でいいのだろうか

//	選択範囲の始点終点・値はＸドットのＹ行数で・矩形用にする
static POINT	gstSqSelBegin;	//!<	始点
static POINT	gstSqSelEnd;	//!<	終点

static POINT	gstSelBgnOrig;	//!<	範囲選択を開始した地点
static POINT	gstSelEndOrig;	//!<	範囲選択を終了した地点

static POINT	gstPrePos;		//!<	直前の選択位置


static BOOLEAN	gbSelecting;	//!<	選択操作中か？
EXTERNED UINT	gbSqSelect;		//!<	矩形選択中である D_SQUARE
//-------------------------------------------------------------------------------------------------

HRESULT	ViewSelStateChange( UINT );
HRESULT	ViewSqSelAdjust( INT );
//-------------------------------------------------------------------------------------------------

/*!
	選択操作中であるかどうかの問い合わせ
	@param[out]	pSqSel	矩形選択中であるかどうか・NULL可
	@return	BOOLEAN	非０選択中である　０選択してない
*/
BOOLEAN IsSelecting( PUINT pSqSel )
{
	if( pSqSel )	*pSqSel = gbSqSelect;

	return gbSelecting;
}
//-------------------------------------------------------------------------------------------------

/*!
	カーソル操作したときとか、マウスクルックでカーソル移動したときとかに呼ばれる
	@param[in]	pVoid	なにか
	@return		HRESULT	終了状態コード
*/
HRESULT ViewSelPositionSet( LPVOID pVoid )
{
	INT	iBgn, iEnd, iDmy = 0;

	//	ルーラ再描画・Ｘ移動無しなら描画の必要は無い
	if( (gstPrePos.x != gdDocXdot) )
	{
		iBgn =  gstPrePos.x;	if( 0 > iBgn )	iBgn = 0;
		iEnd =  gstPrePos.x + 1;
		ViewPositionTransform( &iBgn, &iDmy, 1 );
		ViewPositionTransform( &iEnd, &iDmy, 1 );
		ViewRulerRedraw( iBgn, iEnd );	//	更新!?範囲をいれる

		iBgn =  gdDocXdot;	if( 0 > iBgn )	iBgn = 0;
		iEnd =  gdDocXdot + 1;
		ViewPositionTransform( &iBgn, &iDmy, 1 );
		ViewPositionTransform( &iEnd, &iDmy, 1 );
		ViewRulerRedraw( iBgn, iEnd );	//	更新!?範囲をいれる
	}

	gstPrePos.x = gdDocXdot;
	gstPrePos.y = gdDocLine;

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	矩形選択モードのON/OFFトグル
	@param[in]	bMode	非０メニューから　０範囲選択処理の中から
	@param[in]	pVoid	なにか
	@return	UINT	非０矩形モードＯＮ　０矩形モードＯＦＦ
*/
UINT ViewSqSelModeToggle( UINT bMode, LPVOID pVoid )
{
	POINT	point;

	TRACE( TEXT("矩形選択ON/OFF") );

	//	選択動作中はモード変更しない
	if( gbSelecting )	return gbSqSelect;	//	なぜかRETURNが抜けてた？

	if( bMode )	//	20120313
	{
		gbSqSelect ^= D_SQUARE;
	}
	else
	{
		//gbSqSelect &= ~D_SQUARE;	//	一旦解除して
		if( gbAltOn ){	gbSqSelect |=  D_SQUARE;	}
		//	20120323	Alt押されてたらON、違うなら素通り
	}

	//	開始しても終了しても初期化するのは変わらない
	gstSqSelBegin.x = -1;
	gstSqSelBegin.y = -1;
	gstSqSelEnd.x   = -1;
	gstSqSelEnd.y   = -1;

	MenuItemCheckOnOff( IDM_SQSELECT , gbSqSelect );	//	メニューチェック

	OperationOnStatusBar(  );

	//	カーソルを変更してみる・矩形ならクロスで
	if( D_SQUARE & gbSqSelect )
	{
		SetClassLongPtr( ghViewWnd, GCLP_HCURSOR, (LONG_PTR)(LoadCursor( NULL, IDC_CROSS ) ) );
	}
	else
	{
		SetClassLongPtr( ghViewWnd, GCLP_HCURSOR, (LONG_PTR)(LoadCursor( NULL, IDC_IBEAM ) ) );
	}
	GetCursorPos( &point );
	SetCursorPos( point.x, point.y );

	return gbSqSelect;
}
//-------------------------------------------------------------------------------------------------

/*!
	選択範囲確認して、始点終点同じだったら解除する
	@param[in]	dMode	零：特に考慮なし　非零：選択開始強制モード・でも使ってない
	@return	非０始点終点が異なる　０同じ
*/
UINT ViewSelRangeCheck( UINT dMode )
{

	//	始点終点が同じ位置＝何も選択していない
	if( gstSelBgnOrig.x == gstSelEndOrig.x && gstSelBgnOrig.y == gstSelEndOrig.y )
	{
		TRACE( TEXT("範囲消滅による選択解除") );
		if( IsSelecting( NULL ) )
		{
			TRACE( TEXT("選択中であったら範囲解除") );
			ViewSelPageAll( -1 );	//	中の中でDocSelRangeSet
		}

		//	矩形用
		gstSqSelBegin.x = -1;
		gstSqSelBegin.y = -1;
		gstSqSelEnd.x   = -1;
		gstSqSelEnd.y   = -1;

		gstSelBgnOrig.x = -1;
		gstSelBgnOrig.y = -1;
		gstSelEndOrig.x = -1;
		gstSelEndOrig.y = -1;

		return 0;
	}

	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	選択するかどうか確認して、始点とかいれるかも・キーマウス両方から来る
	キャレットの移動処理の前に、直前の位置を覚えておく必要がある・そこが原点
	@param[in]	dMode	零：特に考慮なし　非零：選択開始強制モード
	@return	HRESULT		終了状態コード
*/
HRESULT ViewSelMoveCheck( UINT dMode )
{

	if( gbExtract && dMode )	//	抽出モードなら
	{
		ViewSelStateChange( FALSE );

		return S_OK;
	}

	if( gbSelecting )
	{
		if( gbShiftOn || dMode )	//	シフト押されてるか、ドラッグ選択中である
		{
			ViewSelStateChange( FALSE );

			ViewSelRangeCheck( dMode );
		}
		else
		{
			TRACE( TEXT("他操作による選択解除") );
			ViewSelPageAll( -1 );

			//	矩形用
			gstSqSelBegin.x = -1;
			gstSqSelBegin.y = -1;
			gstSqSelEnd.x   = -1;
			gstSqSelEnd.y   = -1;

			gstSelBgnOrig.x = -1;
			gstSelBgnOrig.y = -1;
			gstSelEndOrig.x = -1;
			gstSelEndOrig.y = -1;
		}
	}
	else
	{
		//	未選択状態で、シフトオサレながらカーソルの移動があったら
		//	もしくはドラッグ選択なら
		if( gbShiftOn || dMode )
		{
			//	ALT押しながら選択開始したら、矩形選択をToggleする
		//	if( gbAltOn )	//	20120313
				ViewSqSelModeToggle( 0, NULL );

			TRACE( TEXT("STATE[%d %d %d]"), gbShiftOn, gbAltOn, dMode );

			//	矩形用
			gstSqSelBegin = gstPrePos;	//	描画位置？
			gstSqSelEnd.x = gdDocXdot;
			gstSqSelEnd.y = gdDocLine;

			gstSelBgnOrig = gstSqSelBegin;
			gstSelEndOrig = gstSqSelEnd;

			gbSelecting   = TRUE;	//	選択処理開始
			TRACE( TEXT("選択処理開始[%d:%d]"), gstSqSelBegin.x, gstSqSelBegin.y );

			ViewSelStateChange( TRUE );
		}

	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ページ全体の選択状態をON/OFFする
	@param[in]	dForce	０无　＋選択状態　ー選択解除
	@return		全体文字数
*/
INT ViewSelPageAll( INT dForce )
{
	TRACE( TEXT("全選択[%d]"), dForce );

	if( 0 < dForce )		gbSelecting =  TRUE;	//	選択処理開始
	else if( 0 > dForce )	gbSelecting = FALSE;	//	選択処理終了
	else					return 0;	//	０なら処理しない

	return DocPageSelStateToggle( dForce );
}
//-------------------------------------------------------------------------------------------------

/*!
	更新された選択範囲の、選択状態のON/OFFする
	@param[in]	dFirst	選択開始したときかどうか・使ってない
	@return		HRESULT	終了状態コード
*/
HRESULT ViewSelStateChange( UINT dFirst )
{
	//	直前のカーソル位置から、今の選択範囲END位置の間の文字
	//	これは範囲更新された範囲に含むやつの処理
	INT		dBeginDot, dEndDot, dStep = 0;
	INT		dBaseLine, dJpLn;
//	LONG	dBuffer;

	//	直前の状態から行またぎあったかどうか-
	if( gstPrePos.y != gdDocLine )
	{
		dStep = gdDocLine - gstPrePos.y;	//	マイナス方向に注意セヨ
		
		TRACE( TEXT("選択で行またぎ発生：D[%d] L[%d] St[%d]"), gdDocXdot, gdDocLine, dStep );

		//	元々キャレットのあった行の処理
		//	逆方向への選択処理はここでやっていけるはず
		if( 0 <  dStep )	//	末尾に向かって
		{
			dBeginDot =  gstPrePos.x;	//	キャレット位置から
			dEndDot   =  -1;	//	行終端まで
		}
		else	//	差分０ならそもそもここまで来ないからおｋ
		{
			dBeginDot = 0;
			dEndDot   = gstPrePos.x;
		}

		gstSqSelEnd.x =  gdDocXdot;	//	洗濯業範囲指定に使う

		dBaseLine = gstPrePos.y;	//	元々キャレットの存在していた行
	}
	else	//	またいでない
	{
		if( gstPrePos.x < gdDocXdot )	//	末尾に向かって
		{
			dBeginDot = gstPrePos.x;
			dEndDot   = gdDocXdot;
		}
		else	//	銭湯に向かって
		{
			dBeginDot = gdDocXdot;
			dEndDot   = gstPrePos.x;
		}


		dBaseLine = gdDocLine;	//	元々キャレットの存在していた行
	}

	gstSelEndOrig.x = gdDocXdot;
	gstSelEndOrig.y = gdDocLine;

	//	選択範囲、左上右下調整・Orig位置は絶対的な内容のはず・Orig値を元にすればいいか？
	if( gstSelBgnOrig.y >  gstSelEndOrig.y )	//	開始より手前にイッたら
	{
		gstSqSelBegin.y = gdDocLine;
		gstSqSelEnd.y   = gstSelBgnOrig.y;
	}
	else	//	通常なら
	{
		gstSqSelBegin.y = gstSelBgnOrig.y;
		gstSqSelEnd.y   = gdDocLine;
	}

	if( gstSelBgnOrig.x >  gstSelEndOrig.x )	//	開始より手前にイッたら
	{
		gstSqSelBegin.x = gdDocXdot;	//	矩形用？
		gstSqSelEnd.x   = gstSelBgnOrig.x;
	}
	else	//	通常なら
	{
		gstSqSelBegin.x = gstSelBgnOrig.x;
		gstSqSelEnd.x   = gdDocXdot;
	}


	//	開始位置を挟むとき、それを失わないように修正
	//	上端、下端も失わないように注意セヨ

	TRACE( TEXT("[%d:%d][%d:%d]"), gstSqSelBegin.x, gstSqSelBegin.y, gstSqSelEnd.x, gstSqSelEnd.y );

	DocSelRangeSet( gstSqSelBegin.y, gstSqSelEnd.y );

	if( gbSqSelect )	//	矩形の時は専用に処理する
	{
		ViewSqSelAdjust( dBaseLine );
	}
	else
	{
		//	元々キャレットのあった行の選択を調整
		DocRangeSelStateToggle( dBeginDot, dEndDot, dBaseLine, 0 );

		//	行またぎ発生してたら
		if( 1 <= dStep )
		{
			DocReturnSelStateToggle( dBaseLine, 0 );

			//	間に１行以上存在してたらそっちを埋める
			for( dJpLn = (dBaseLine + 1); gdDocLine > dJpLn; dJpLn++ )
			{
				DocReturnSelStateToggle( dJpLn, 0 );
				DocRangeSelStateToggle( 0, -1, dJpLn, 0 );
			}

			DocRangeSelStateToggle( 0, gdDocXdot, gdDocLine, 0 );
		}

		if( -1 >= dStep )
		{
		//	DocReturnSelStateToggle( dBaseLine, 0 );	//	ここには要らない？

			//	間に１行以上存在してたらそっちを埋める
			for( dJpLn = (gdDocLine + 1); dBaseLine > dJpLn; dJpLn++ )
			{
				DocReturnSelStateToggle( dJpLn, 0 );
				DocRangeSelStateToggle( 0, -1, dJpLn, 0 );
			}

			DocReturnSelStateToggle( gdDocLine, 0 );
			DocRangeSelStateToggle( gdDocXdot, -1, gdDocLine, 0 );
		}
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	選択開始地点より前に選択しているか
	@param[in]	line	現在行
	@return	UINT		非０いる！いる！
*/
UINT ViewSelBackCheck( INT line )
{
	//	選択範囲、左上右下調整・Orig位置は絶対的な内容のはず・Orig値を元にすればいいか？

	TRACE( TEXT("LINE[%d] ST[%d]"), line, gstSelBgnOrig.y );

	//	開始から上の行イッたら
	if( gstSelBgnOrig.y >= line ){	return TRUE;	}

	return FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	矩形選択の調整
	@param[in]	dBaseLine	元々キャレットのあった行番号
	@return		HRESULT	終了状態コード
*/
HRESULT ViewSqSelAdjust( INT dBaseLine )
{
	INT	i, xDotBegin, xDotEnd, xDotLast;
	//	もっと良いやり方ないか

#pragma message ("ここで、選択範囲全体の処理が何度も行われているので重たい")

	//	マウスクルックとかで、行単位で選択が変更された場合
	//	上が開いてる・開いてる所の選択解除
	if( dBaseLine < gstSqSelBegin.y )
	{
		for( i = dBaseLine; gstSqSelBegin.y > i; i++ )
		{
			DocRangeSelStateToggle( 0, -1, i, -1 );
		}
	}
	//	下が開いてる
	if( gstSqSelEnd.y < dBaseLine )
	{
		for( i = gstSqSelEnd.y + 1; dBaseLine >= i; i++ )
		{
			DocRangeSelStateToggle( 0, -1, i, -1 );
		}
	}

	for( i = gstSqSelBegin.y; gstSqSelEnd.y >= i; i++ )
	{
		xDotBegin = gstSqSelBegin.x;
		DocLetterPosGetAdjust( &xDotBegin, i, 0 );	//	各行のキャレット位置の調整

		//	20110720	終端位置どうにかなる？
	//	xDotEnd = gstSqSelEnd.x;	//元
		//末端位置とカーソル位置を比べて、より遠い方を採用・逆方向へ対処するために確認が必要
		xDotEnd = (gstSqSelEnd.x < gstCursor.x) ? gstCursor.x : gstSqSelEnd.x;
		DocLetterPosGetAdjust( &xDotEnd, i, 0 );

		//	末端確認
		xDotLast = DocLineParamGet( i, NULL, NULL );

		if( 0 < xDotBegin )	//	先頭から選択範囲直前まで
		{
			DocRangeSelStateToggle( 0, xDotBegin, i, -1 );
		}

		//	選択範囲
		DocRangeSelStateToggle( xDotBegin, xDotEnd, i, 1 );

		if( xDotEnd < xDotLast )	//	選択範囲の終わりから末端まで
		{
			DocRangeSelStateToggle( xDotEnd, -1, i, -1 );
		}
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	カーソルのある、空白列もしくは文字列を選択状態にする
	@param[in]	pVoid	なにか
	@return		HRESULT	終了状態コード
*/
HRESULT ViewSelAreaSelect( LPVOID pVoid )
{
	INT		iBeginDot, iEndDot, iStCnt, iCount;
	INT		iRangeDot;
	BOOLEAN	bIsSpase;

	DocPageSelStateToggle(  FALSE );	//	一旦選択状態は解除

	iRangeDot = DocLineStateCheckWithDot( gdDocXdot, gdDocLine, &iBeginDot, &iEndDot, &iStCnt, &iCount, &bIsSpase );
															//	始点ドット・終点ドット・開始地点の文字数・間の文字数・該当はスペースであるか
	gdDocXdot = iBeginDot;	//	選択範囲として移動する
	ViewSelMoveCheck( FALSE );
	ViewSelPositionSet( NULL );

	//	ドラッグ移動を模擬的に行う

	gdDocXdot = iEndDot;	//	選択範囲として移動する

	ViewDrawCaret( gdDocXdot, gdDocLine, 1 );	//	ここでキャレットも移動

	ViewSelMoveCheck( TRUE );
	ViewSelPositionSet( NULL );	//	移動した位置を記録

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

