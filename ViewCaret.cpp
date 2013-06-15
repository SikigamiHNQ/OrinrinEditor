/*! @file
	@brief キャレットの管理をします
	このファイルは ViewCaret.cpp です。
	@author	SikigamiHNQ
	@date	2011/04/15
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

/*
キャレット(カレット)はメッセージキューに対して１つ作成できる
２つ以上のキャレットが同時に表示されると、UI上好ましくない
フォーカスの有り無しとも無関係に作成破棄できてしまう
Windowsの指針に従って
　１．フォーカスを失ったとき。
　２．アクティブでなくなったとき。
の両タイミング時にキャレットを「非表示」にすべき
*/

//-------------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "OrinrinEditor.h"
//-------------------------------------------------------------------------------------------------

#define CARET_WIDTH	2
//-------------------------------------------------------------------------------------------------

extern HWND		ghPrntWnd;		//!<	親ウインドウハンドル
extern HWND		ghViewWnd;		//!<	描画ウインドウのハンドル


extern INT		gdDocXdot;		//!<	キャレットのＸドット・ドキュメント位置
extern INT		gdDocLine;		//!<	キャレットのＹ行数・ドキュメント位置
extern INT		gdDocMozi;		//!<	キャレットの左側の文字数

//	画面サイズを確認して、移動によるスクロールの面倒みる
extern INT		gdHideXdot;		//!<	左の隠れ部分
extern INT		gdViewTopLine;	//!<	表示中の最上部行番号
extern SIZE		gstViewArea;	//!<	表示領域のサイズ・ルーラー等の領域は無し
extern INT		gdDispingLine;	//!<	見えてる行数・中途半端に見えてる末端は含まない


static HBITMAP	ghbmpCaret;		//!<	キャレット用のビットマップ

static BOOLEAN	gbCaretShow;	//!<	キャレット表示してるか？
//-------------------------------------------------------------------------------------------------


HRESULT	ImeInputBoxPosSet( VOID );

HRESULT	ViewCaretFrameOutCheck( INT, INT, UINT );
//-------------------------------------------------------------------------------------------------


/*!
	キャレットを作る
	@param[in]	hWnd	ウインドウのハンドル
	@param[in]	clrMain	キャレットの色
	@param[in]	clrBack	キャレットの背景・未使用にしてみる
	@return		HRESULT	終了状態コード
*/
HRESULT ViewCaretCreate( HWND hWnd, COLORREF clrMain, COLORREF clrBack )
{
	HDC		hdcMem, hdc = GetDC( hWnd );
	HBITMAP	hBmpOld;
	HBRUSH	hBrushCaret, hBrushBack, hBrushOld;


//テスト
	clrBack  = ~(clrMain);	//	反転色をつかう
	clrBack &= 0x00FFFFFF;	//	関係ないところはクルヤーしておく

	//	必要なら、以前のビットマップを破棄する・個々来る前の処理で


	//	キャレットBMPを作成・既存のゑを読み込んでも良い
	ghbmpCaret  = CreateCompatibleBitmap( hdc, CARET_WIDTH, LINE_HEIGHT );
	hdcMem      = CreateCompatibleDC( hdc );
	hBmpOld     = SelectBitmap( hdcMem, ghbmpCaret );

	hBrushCaret = CreateSolidBrush( clrMain );
	hBrushBack  = CreateSolidBrush( clrBack );

	hBrushOld   = SelectBrush( hdcMem, hBrushCaret );
	PatBlt( hdcMem, 0, 0, CARET_WIDTH, LINE_HEIGHT, PATCOPY );
	SelectBrush( hdcMem, hBrushBack );
	PatBlt( hdcMem, 0, 0, CARET_WIDTH, LINE_HEIGHT, PATINVERT );

	SelectBrush( hdcMem, hBrushOld );
	SelectBitmap( hdcMem, hBmpOld );

	DeleteBrush( hBrushCaret );
	DeleteBrush( hBrushBack );
	DeleteDC( hdcMem );

	ReleaseDC( hWnd, hdc );

	//	キャレットを作成する
	CreateCaret( hWnd, ghbmpCaret, CARET_WIDTH, LINE_HEIGHT );

	gbCaretShow = FALSE;


	//	フォーカスを失ったら消しておくとかのご配慮を願います

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	キャレットの破壊
	@return	HRESULT	終了状態コード
*/
HRESULT ViewCaretDelete( VOID )
{
	if( ghbmpCaret )
	{
		DestroyCaret(  );
		DeleteObject( ghbmpCaret );
	}
	ghbmpCaret = NULL;

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	キャレットの再表示
	@return	BOOL	非０表示されてた　０消えてた
*/
BOOL ViewShowCaret( VOID )
{
	BOOL	bRslt;

	bRslt = ShowCaret( ghViewWnd  );

	if( !(bRslt) )
	{
		bRslt = CreateCaret( ghViewWnd, ghbmpCaret, CARET_WIDTH, LINE_HEIGHT );
		TRACE( TEXT("CARET reset %u"), bRslt );
		gbCaretShow = FALSE;
		ViewDrawCaret( gdDocXdot, gdDocLine, 1 );	//	位置を決める
	}

//	ImeInputBoxPosSet(  );

	return bRslt;
}
//-------------------------------------------------------------------------------------------------

/*!
	キャレットの非表示
*/
VOID ViewHideCaret( VOID )
{
	HideCaret( ghViewWnd  );
	gbCaretShow = FALSE;

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	Caretを、色を変えて再構築する
	@param[in]	crtColour	変更する色
	@return		HRESULT		終了状態コード
*/
HRESULT ViewCaretReColour( COLORREF crtColour )
{

	ViewCaretDelete(  );	//	まず既存のヤツを破壊

	ViewCaretCreate( ghViewWnd, crtColour, 0 );

	ViewShowCaret(  );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	キャレットの位置を強制変更
	@param[in]	xDot	描画位置の横ドット数
	@param[in]	yLine	描画する行数
	@return		非０枠内である　０はみ出してる
*/
BOOLEAN ViewPosResetCaret( INT xDot, INT yLine )
{
	if( 0 > xDot )	xDot  = 0;
	if( 0 > yLine )	yLine = 0;

	gdDocLine = yLine;
	gdDocMozi = DocLetterPosGetAdjust( &xDot, yLine, 0 );	//	今の文字位置を確認
	gdDocXdot = xDot;

	return ViewDrawCaret( xDot, yLine, TRUE );
}
//-------------------------------------------------------------------------------------------------

/*!
	キャレットの位置を変更する
	@param[in]	rdXdot	描画位置の横ドット数
	@param[in]	rdLine	描画する行数
	@param[in]	bOnScr	画面からはみ出すようならスクロールさせるか？
	@return		非０枠内である　０はみ出してる
*/
BOOLEAN ViewDrawCaret( INT rdXdot, INT rdLine, BOOLEAN bOnScr )
{
	INT		dX, dY, loop;
	BOOLEAN	bRslt, fRslt, cRslt;
	POINT	stCaret;

	dX = rdXdot;
	dY = rdLine * LINE_HEIGHT;


	stCaret.x = rdXdot;
	stCaret.y = rdLine;
	DocCaretPosMemory( INIT_SAVE, &stCaret );	//	ファイル毎のCaret位置保存


	//	位置情報は、ドキュメント的な位置で操作して、ビューするときだけいろいろする
	//	ここで、カーソル位置が画面からはみ出しそうなら、適切な方向にスクロール
	//	位置情報も修正

	//	スクロールバーからの操作なら、キャレットが画面からはみ出してもよい
	if( bOnScr )	ViewCaretFrameOutCheck( dX, dY, 1 );
	
	ViewPositionTransform( &dX, &dY, 1 );

//キャレット位置がマイナスになるようなら、非表示にする

	//gdViewXdot = dX;
	//gdViewLine = dY;	//	使ってない？

	fRslt = ViewIsPosOnFrame( dX , dY );	//	位置確認
	if( fRslt )
	{
		bRslt = SetCaretPos( dX, dY );	//	移動

		if( !(gbCaretShow) )
		{
			for( loop = 0; 10 > loop; loop++ )
			{
				cRslt = ShowCaret( ghViewWnd  );	//	表示する
				if( cRslt )	break;
			}
		}
		gbCaretShow = TRUE;
	}
	else
	{
		if( gbCaretShow )
		{
			for( loop = 0; 10 > loop; loop++ )
			{
				cRslt = HideCaret( ghViewWnd  );	//	けす
				if( cRslt )	break;
			}
		}
		gbCaretShow = FALSE;
	}


	ViewNowPosStatus(  );

	ImeInputBoxPosSet(  );	//	IME入力枠の位置を変更

	return fRslt;
}
//-------------------------------------------------------------------------------------------------

/*!
	ルーラーとか考慮する前の描画位置ドットを受け取って、
	はみ出すようならスクロール量を操作する・だけ
	@param[in]	dDotX	描画位置の横ドット数・ここでは数値受け取るだけ
	@param[in]	dDotY	描画位置の縦ドット数
	@param[in]	dummy	とくにない？
	@return		HRESULT	終了状態コード
*/
HRESULT ViewCaretFrameOutCheck( INT dDotX, INT dDotY, UINT dummy )
{
	BOOLEAN	bRedraw = FALSE;
	INT	opX, opY;
	INT	bkWid;

	//	左へはみ出し
	if( gdHideXdot > dDotX )	//	隠れ位置よりさらに左へいったら
	{
		gdHideXdot = dDotX;

		bRedraw = TRUE;
	}

	//	右へはみ出し
	opX = dDotX - gdHideXdot;
	if( gstViewArea.cx < (opX + EOF_WIDTH) )
	{
		bkWid = (opX + EOF_WIDTH) - gstViewArea.cx;

		gdHideXdot += bkWid;	//	左隠れ領域を増やす

		bRedraw = TRUE;
	}


	//	上へはみ出し
	if( gdViewTopLine > gdDocLine )
	{
		assert( gdViewTopLine );	//	この段階で０はあり得ない
		gdViewTopLine = gdDocLine;

		bRedraw = TRUE;
	}

	//	下へはみ出し
	opY = gdDocLine - gdViewTopLine;
	if( gdDispingLine <= opY )
	{
		gdViewTopLine = gdDocLine - gdDispingLine + 1;

		bRedraw = TRUE;
	}

	if( bRedraw )	ViewRedrawSetLine( -1 );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------



/*!
	IME入力枠の位置を変更
	@return		HRESULT	終了状態コード
*/
HRESULT ImeInputBoxPosSet( VOID )
{
	COMPOSITIONFORM	stCompForm;
	HIMC	hImc;
	POINT	stPoint;

	hImc = ImmGetContext( ghViewWnd );	//	IMEハンドル確保

	if( hImc )	//	確保出来たら
	{
		GetCaretPos( &stPoint );	//	今のキャレット位置を確認して
		stCompForm.dwStyle = CFS_POINT;	//	位置変更を有効にする
		stCompForm.ptCurrentPos.x = stPoint.x;
		stCompForm.ptCurrentPos.y = stPoint.y;

	//	TRACE( TEXT("CARET [%d x %d]"), stPoint.x, stPoint.y );

		ImmSetCompositionWindow( hImc, &stCompForm );	//	位置変更

		ImmReleaseContext( ghViewWnd , hImc );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	今のキャレット位置をゲッツする
	@param[in]	pXdot	キャレットのＸドット
	@param[in]	pYline	キャレットのＹ行数
	@return	INT	キャレットの左側の文字数
*/
INT ViewCaretPosGet( PINT pXdot, PINT pYline )
{
	if( pXdot ){	*pXdot  = gdDocXdot;	}
	if( pYline ){	*pYline = gdDocLine;	}

	return gdDocMozi;
}
//-------------------------------------------------------------------------------------------------

