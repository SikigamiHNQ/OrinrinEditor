/*! @file
	@brief キーボード入力やマウスクルックの面倒みる
	このファイルは ViewKeyButton.cpp です。
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

/*
MK_CONTROL	Ctrl キーが押されている場合に設定します。
MK_LBUTTON	マウスの左ボタンが押されている場合に設定します。
MK_MBUTTON	マウスの中央ボタンが押されている場合に設定します。
MK_RBUTTON	マウスの右ボタンが押されている場合に設定します。
MK_SHIFT	Shift キーが押されている場合に設定します。
*/
//-------------------------------------------------------------------------------------------------

extern HWND		ghPrntWnd;		//!<	親ウインドウハンドル
extern HWND		ghViewWnd;		//!<	このウインドウのハンドル

extern INT		gdXmemory;		//		直前のＸ位置を覚えておく

extern INT		gdDocXdot;		//!<	キャレットのＸドット・ドキュメント位置
extern INT		gdDocLine;		//!<	キャレットのＹ行数・ドキュメント位置
extern INT		gdDocMozi;		//!<	キャレットの左側の文字数

//	画面サイズを確認して、移動によるスクロールの面倒みる
extern INT		gdHideXdot;		//!<	左の隠れ部分
extern INT		gdViewTopLine;	//!<	表示中の最上部行番号
extern SIZE		gstViewArea;	//!<	表示領域のサイズ・ルーラー等の領域は無し
extern INT		gdDispingLine;	//!<	見えてる行数・中途半端に見えてる末端は含まない

extern BOOLEAN	gbExtract;	

extern  UINT	gbUniPad;		//!<	パディングにユニコードをつかって、ドットを見せないようにする

//	これらのキーの具合は、GetKeyStateもしくはGetKeyboardStateを使えばいい
EXTERNED BOOLEAN	gbShiftOn;	//!<	シフトが押されている
EXTERNED BOOLEAN	gbCtrlOn;	//!<	コントロールが押されている
EXTERNED BOOLEAN	gbAltOn;	//!<	アルタが押されている

EXTERNED POINT	gstCursor;		//!<	文字を考慮しない、Cursorのドット＆行位置

EXTERNED UINT	gbBrushMode;	//!<	非零ブラシモード
static TCHAR	gatBrushPtn[SUB_STRING];	//!<	ブラシパヤーン

static  UINT	gdSqFillCnt;	//!<	矩形選択を、IME文字列で塗りつぶした時の文字数

static  UINT	gbLDoubleClick;	//!<	ダブルクルックした

static POINT	gstLClicken;	//!<	左クルックした位置
static  UINT	gbDragMoved;	//!<	選択範囲をドラッグで移動しようとしている
//-------------------------------------------------------------------------------------------------

HRESULT	ViewBrushFilling( VOID );

HRESULT	ViewScriptedLineFeed( VOID );
//-------------------------------------------------------------------------------------------------

//CreateAcceleratorTable メモ

/*!
	シフト、コントロール、アルトキーの状態を確認する
*/
VOID ViewCombiKeyCheck( VOID )
{
//	SHIFT,CONTROL,ALTのキーの具合は、GetKeyStateもしくはGetKeyboardStateを使えばいい
	gbShiftOn = (0x8000 & GetKeyState(VK_SHIFT)) ? TRUE : FALSE;
	gbCtrlOn  = (0x8000 & GetKeyState(VK_CONTROL)) ? TRUE : FALSE;
	gbAltOn   = (0x8000 & GetKeyState(VK_MENU)) ? TRUE : FALSE;

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	編集ビューのキーダウンが発生
	@param[in]	hWnd	ウインドウハンドル・ビューのとは限らないので注意セヨ
	@param[in]	vk		押されたキーが仮想キーコードで来る
	@param[in]	fDown	非０ダウン　０アップ
	@param[in]	cRepeat	連続オサレ回数・取れてない？
	@param[in]	flags	キーフラグいろいろ
	@return		無し
*/
VOID Evw_OnKey( HWND hWnd, UINT vk, BOOL fDown, INT cRepeat, UINT flags )
{
	INT		bXdirect = 0;	//	Ｘの移動方向
	UINT	dXwidth;	//	Ｘの移動ドット
	INT		dDot, bCrLf, iLines, i;
	BOOLEAN	bJump = FALSE, bMemoryX = FALSE;
	BOOLEAN	bSelect = FALSE;
	UINT	bSqSel  = 0;

	ViewCombiKeyCheck(  );

	ViewSelPositionSet( NULL );	//	操作直前の位置
	//	中でルーラーのドローが発生してる。Ctrlとか押しっぱでちらつく

#ifdef DO_TRY_CATCH
	try{
#endif

	if( fDown )
	{
		switch( vk )
		{
			case VK_RIGHT:	bXdirect =  1;	bMemoryX = TRUE;	break;

			case VK_LEFT:	bXdirect = -1;	bMemoryX = TRUE;	break;

			case VK_DOWN:	//	行を下へ
				gdDocLine++;
				dDot = DocLineParamGet( gdDocLine, NULL, NULL );	//	次の行が無かったら死にます
				if( -1 == dDot ){	gdDocLine--;	}	//	戻しておく
				break;

			case VK_UP:	//	行を上へ
				if( 0 < gdDocLine ){	gdDocLine--;	}	//	０なら変化無し
				break;

			case VK_PRIOR:	//	PageUp
				gdDocLine -= 10;
				if( 0 > gdDocLine ){	gdDocLine = 0;	}
				break;

			case VK_NEXT:	//	PageDown
				gdDocLine += 10;
				iLines = DocNowFilePageLineCount(  );//DocPageParamGet( NULL, NULL );	//	行数確保
				if( iLines <= gdDocLine ){	gdDocLine =  iLines - 1;	}
				break;

			case VK_END:	//	Ctrl+Endで末尾へ
				if( gbCtrlOn ){	gdDocLine = DocNowFilePageLineCount(  ) - 1;	}	//	DocPageParamGet( NULL, NULL )	//	行数確保
				gdDocXdot = DocLineParamGet( gdDocLine, &gdDocMozi, NULL );
				bMemoryX = TRUE;
				break;

			case VK_HOME:	//	Ctrl+Homeで先頭へ
				gdDocXdot = 0;	gdDocMozi = 0;	bMemoryX = TRUE;
				if( gbCtrlOn  ){	gdDocLine = 0;	}
				break;


			case VK_DELETE:	//	DELキー入力
				bSelect = IsSelecting( &bSqSel );
				iLines = DocNowFilePageLineCount(  );//DocPageParamGet( NULL, NULL );	//	行数確保
				if( bSelect )	//	選択状態なら、そこだけ削除する
				{
					bCrLf = DocSelectedDelete( &gdDocXdot , &gdDocLine, bSqSel, TRUE );
				}
				else
				{
					bCrLf = DocInputDelete( gdDocXdot , gdDocLine );
				}
				//	負で異常発生
				if( 0 <  bCrLf )	//	処理した行以降全取っ替え
				{
					for( i = gdDocLine; iLines >= i; i++  ){	ViewRedrawSetLine(  i );	}
				}
				else{	ViewRedrawSetLine( gdDocLine  );	}
				ViewDrawCaret( gdDocXdot, gdDocLine, 1 );	//	キャレット位置を決める
				DocPageInfoRenew( -1, 1 );
				bMemoryX = TRUE;
				return;

			case VK_PROCESSKEY:
				//	大日本帝國的言語変換をしてるときはキー入力は全部ここにくる・シフトとかは別っぽい
				return;

			default:
		//		TRACE( TEXT("vk[0x%04X]"), vk );
				return;
		}
		//	IMEがONのときのオサレは、vk：0xE5連発・確定された文字はCHARへ

		if( bMemoryX )	gdXmemory = gdDocXdot;
		else			gdDocXdot = gdXmemory;

	
		DocLetterPosGetAdjust( &gdDocXdot, gdDocLine, 0 );	//	キャレット位置調整
		//	ここで文字位置のインクリ・デクリの面倒みて、ドットと行位置を変更
		dXwidth = DocLetterShiftPos( gdDocXdot, gdDocLine, bXdirect, NULL, &bJump );

		if( 0 > bXdirect )	//	左・先頭へ向かって
		{
			gdDocXdot -= dXwidth;
			if( 0 >  gdDocXdot )	gdDocXdot = 0;

			if( bJump )
			{
				if( 0 < gdDocLine ){	gdDocLine--;	}	//	０なら変化無し

				//	隣の行の末尾に移動する
				dDot = DocLineParamGet( gdDocLine, NULL, NULL );
				gdDocXdot = dDot;
			}
		}

		if( 0 < bXdirect )	//	右・末尾へ向かって
		{
			gdDocXdot += dXwidth;
			if( bJump )
			{
				gdDocLine++;

				//	次の行が無かったら
				dDot = DocLineParamGet( gdDocLine, NULL, NULL );
				if( 0 >  dDot ){	gdDocLine--;	}	//	戻しておく
				else{	gdDocXdot = 0;	}	//	次の行へ移動して行頭へ
			}
		}

		gdDocMozi = DocLetterPosGetAdjust( &gdDocXdot, gdDocLine, 0 );	//	今の文字位置を確認

		ViewSelMoveCheck( FALSE );	//	位置決まったら、選択状態をCheck

		if( bMemoryX )	gdXmemory = gdDocXdot;

		ViewDrawCaret( gdDocXdot, gdDocLine, 1 );	//	最終的な位置にキャレット位置を変更
	}
	else	//	キー離され
	{
		//	ない？
	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	ETC_MSG( err.what(), 0 );	 return;	}
	catch( ... ){	ETC_MSG( ("etc error"), 0 );	 return;	}
#endif

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	編集ビューの文字キーオサレが発生
	@param[in]	hWnd	ウインドウハンドル・ビューのとは限らないので注意セヨ
	@param[in]	ch		押された文字
	@param[in]	cRepeat	キーリピート回数・効いてない？
	@return		無し
*/
VOID Evw_OnChar( HWND hWnd, TCHAR ch, INT cRepeat )
{
	BOOLEAN	bSelect, bFirst;
	UINT	bSqSel = 0;
	INT		isctrl, bCrLf, iLines, i;
	//	バックスペースとか改行0x0Dもくる
	TCHAR	atCh[2];


	ViewCombiKeyCheck(  );

	bSelect = IsSelecting( &bSqSel );	//	選択状態であるか	矩形選択中であるか

	isctrl = iswcntrl( ch );	//	制御文字であるか
	//	Ctrl+Zとかは制御文字で来るので注意
	if( isctrl )	//	制御文字はBSとReturn以外は無視でいいか
	{
		TRACE( TEXT("制御文字[%04X]"), ch );

		//	TABはこっちに来る

		if( VK_RETURN == ch )	//	改行
		{
			TRACE( TEXT("Enter Shift[%d]"), gbShiftOn );
			if( gbShiftOn )
			{
				ViewScriptedLineFeed(  );	//	Shift押しながらの場合、台詞改行とする。
			}
			else
			{
				bFirst = TRUE;
				if( bSelect )	//	選択状態で有る場合・削除
				{
					bCrLf = DocSelectedDelete( &gdDocXdot , &gdDocLine, bSqSel, bFirst );	bFirst = FALSE;
				}

				DocCrLfAdd( gdDocXdot , gdDocLine, bFirst );	//	中のほうで空白チェックやってる
				ViewRedrawSetLine( gdDocLine );

				gdDocXdot = 0;	gdDocMozi = 0;	gdDocLine++;	//	次の行に移る
				ViewDrawCaret( gdDocXdot, gdDocLine, 1 );	//	位置を決める
				gdXmemory = gdDocXdot;	//	最新位置記憶
				//	改行した行以降全取っ替え
				iLines = DocPageParamGet( NULL, NULL );	//	表示変更してるか
				for( i = gdDocLine; iLines >= i; i++ ){	ViewRedrawSetLine(  i );	}
			}
		}

		if( VK_BACK == ch )	//	BackSpace
		{
			TRACE( TEXT("BACKSP [%d][%d:%d]"), bSelect, gdDocXdot, gdDocLine );
			iLines = DocNowFilePageLineCount(  );//DocPageParamGet( NULL, NULL );	//	行数確保
			if( bSelect )	//	選択状態なら、そこだけ削除する
			{
				bCrLf = DocSelectedDelete( &gdDocXdot , &gdDocLine, bSqSel, TRUE );
			}
			else
			{
				bCrLf = DocInputBkSpace( &gdDocXdot , &gdDocLine );
			}
			if( bCrLf  )	//	処理した行以降全取っ替え
			{
				for( i = gdDocLine; iLines >= i; i++  ){	ViewRedrawSetLine(  i );	}
			}
			else{	ViewRedrawSetLine( gdDocLine  );	}

			ViewDrawCaret( gdDocXdot, gdDocLine, 1 );	//	キャレット位置を決める
			gdXmemory = gdDocXdot;	//	最新位置記憶
		}

		return;
	}

	TRACE( TEXT("入力文字[%c]"), ch );

	if( 0 < gdSqFillCnt )	//	IME経由で矩形塗り潰しが発生している
	{
		gdSqFillCnt--;
		TRACE( TEXT("キャンセル[%u]"), gdSqFillCnt );
		return;
	}

	iLines = DocPageParamGet( NULL, NULL );	//	再計算してるかも・・・
	bCrLf = 0;
	if( bSelect )	//	選択状態で有る場合・矩形なら塗り潰し、通常なら削除
	{
		if( bSqSel )	//	矩形選択中なら塗りつぶして終了
		{
			atCh[0] = ch;	atCh[1] = NULL;
			DocSelectedBrushFilling( atCh, &gdDocXdot ,&gdDocLine );
			ViewDrawCaret( gdDocXdot, gdDocLine, 1 );	//	キャレット位置を決める
			DocPageInfoRenew( -1, 1 );
			return;
		}
		else{	bCrLf = DocSelectedDelete( &gdDocXdot , &gdDocLine, 0, TRUE );	}
		//	通常選択中なら、一旦その範囲を削除する
	}

	DocInsertLetter( &gdDocXdot, gdDocLine, ch );	//	行に追加
	//	中でアンドゥバッファリング
	gdDocMozi = DocLetterPosGetAdjust( &gdDocXdot, gdDocLine, 0 );	//	今の文字位置を確認
	ViewDrawCaret( gdDocXdot, gdDocLine, 1 );	//	位置移動

	gdXmemory = gdDocXdot;	//	最新位置記憶

	if( bCrLf  )	//	処理した行以降全取っ替え
	{
		for( i = gdDocLine; iLines > i; i++ ){	ViewRedrawSetLine(  i );	}
	}
	else{	ViewRedrawSetLine( gdDocLine  );	}

	DocPageInfoRenew( -1, 1 );

	return;
}
//-------------------------------------------------------------------------------------------------


/*!
	ビューでマウスの左ボタンがダウンされたとき
	@param[in]	hWnd			ウインドウハンドル・ビューのとは限らないので注意セヨ
	@param[in]	fDoubleClick	非０ダブルクルックされた場合
	@param[in]	x				発生したＸ座標値
	@param[in]	y				発生したＹ座標値
	@param[in]	keyFlags		他に押されてるキーについて
*/
VOID Evw_OnLButtonDown( HWND hWnd, BOOL fDoubleClick, INT x, INT y, UINT keyFlags )
{
	INT		dX, dY;	//	
	INT		dDot, dMaxDot, dLine, iMaxLine;	//	
	UINT	dRslt;

	SetFocus( hWnd );	//	マウスインでフォーカス

	dX = x;
	dY = y;
	ViewPositionTransform( &dX, &dY, 0 );	//	ここで、ドキュメント位置に変更

	ViewCombiKeyCheck(  );


	//	マイナスのときはルーラーか行番号エリア
	if( 0 > dX )	dX = 0;
	if( 0 > dY )	dY = 0;
	
	dDot  = dX;
	dLine = dY / LINE_HEIGHT;

	if( fDoubleClick )	//	ダブルクルック
	{
		TRACE( TEXT("マウス左ダブルクルック[%d / %d]%d:%d:%d"), dDot, dLine, gbShiftOn, gbCtrlOn, gbAltOn );

		ViewSelAreaSelect( NULL );

		gbLDoubleClick = TRUE;
		return;	//	これ以降は処理しなくて良いはず
	}

	TRACE( TEXT("マウス左ダウン[%d / %d]%d:%d:%d"), dDot, dLine, gbShiftOn, gbCtrlOn, gbAltOn );

	SetCapture( hWnd  );	//	マウスキャプチャ

	//	有効な位置かどうか確認

	//	行数確認して、はみ出してたら末端にしておく
	iMaxLine = DocNowFilePageLineCount(  );//DocPageParamGet( NULL, NULL );	//	行数確認かも・・・
	if( iMaxLine <=dLine )	dLine = iMaxLine - 1;

	//	その行のドット数を確認して、はみ出してたら末端にしておく
	dMaxDot = DocLineParamGet( dLine, NULL, NULL );
	if( dMaxDot <=dDot )	dDot = dMaxDot;

	//	文字位置に合わせて調整
	gdDocMozi = DocLetterPosGetAdjust( &dDot, dLine, 0 );	//	今の文字位置を確認
	gdDocXdot = dDot;
	gdDocLine = dLine;
	//	この時点で移動は確定

	gstLClicken.x = gdDocXdot;
	gstLClicken.y = gdDocLine;

	//	Upのほうに移動してみる	20120328
//	if( !(gbExtract) )	ViewBrushFilling(  );	//	ブラシする
	//	抽出モード中は処理しない

	gdXmemory = gdDocXdot;
	ViewDrawCaret( gdDocXdot, gdDocLine, 1 );	//	位置移動


	dRslt = DocLetterSelStateGet( gdDocXdot, gdDocLine );
	//	TRACE(	TEXT("Sel %d[%u]"), dRslt, gdDocXdot );
	if( dRslt ){	gbDragMoved = TRUE;	}	//	選択範囲のドラッグ移動の可能性
	else{	ViewSelMoveCheck( FALSE );	}	//	選択状態で、クルッコだけすると、選択解除
	//クルックした箇所が選択状態であれば解除しない・非選択エリアならここで解除する

	ViewSelPositionSet( NULL );	//	移動した位置を記録と再描画

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ビューでマウスを動かしたとき
	@param[in]	hWnd		ウインドウハンドル・多分ビューのやつ
	@param[in]	x			発生したクライヤントＸ座標値
	@param[in]	y			発生したクライヤントＹ座標値
	@param[in]	keyFlags	他に押されてるキーについて
*/
VOID Evw_OnMouseMove( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	TCHAR	atString[SUB_STRING];
	INT		dX, dY;
	INT		dDot, dMaxDot, dLine, iMaxLine;	//	

	//	ダブルクルック操作後は何もしない
	if( gbLDoubleClick ){	 return;	}

	dX = x;
	dY = y;
	ViewPositionTransform( &dX, &dY, 0 );	//	ここで、ドキュメント位置に変更

	ViewCombiKeyCheck(  );	//	範囲選択しようとしてるか

	//	マイナスのときはルーラーか行番号エリア
	if( 0 > dY )	dY = 0;

	dLine = dY / LINE_HEIGHT;

	if( 0 > dX )
	{
		dX = 0;
		if( (keyFlags & MK_LBUTTON) )	//	ドラッグ中であるなら
		{
			//	その行のドット数を確認して、常に末端にカーソルがあると仮定する
			dX = DocLineParamGet( dLine, NULL, NULL );
			//	バック選択中なら、逆に先頭にカーソルが来るようにする
			if( ViewSelBackCheck( dLine ) ){	dX =  0;	}
		}
	}

	dDot  = dX;

	gstCursor.x = dDot;
	gstCursor.y = dLine;	//	Cursor位置記憶

	if( (keyFlags & MK_LBUTTON) )
	{
		//	行数確認して、はみ出してたら末端にしておく
		iMaxLine = DocNowFilePageLineCount(  );//DocPageParamGet( NULL, NULL );
		if( iMaxLine <= dLine ){	dLine = iMaxLine - 1;	}

		//	その行のドット数を確認して、はみ出してたら末端にしておく
		dMaxDot = DocLineParamGet( dLine, NULL, NULL );
		if( dMaxDot <=dDot )	dDot = dMaxDot;

		//	文字位置に合わせて調整
		gdDocMozi = DocLetterPosGetAdjust( &dDot, dLine, 0 );	//	今の文字位置を確認
		gdDocXdot = dDot;
		gdDocLine = dLine;

		ViewDrawCaret( gdDocXdot, gdDocLine, 1 );	//	位置移動

		//	ドラッグ移動は選択モード突入・移動に考慮
		if( !(gbDragMoved) ){	ViewSelMoveCheck( TRUE );	}

		ViewSelPositionSet( NULL );	//	移動した位置を記録
	}

	StringCchPrintf( atString, SUB_STRING, TEXT("MOUSE %d[dot] %d[line]"), gstCursor.x, gstCursor.y );
	MainStatusBarSetText( SB_MOUSEPOS, atString );
//	TRACE( atString );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ビューでマウスの左ボタンがうっｐされたとき
	@param[in]	hWnd			ウインドウハンドル・ビューのとは限らないので注意セヨ
	@param[in]	x				発生したＸ座標値
	@param[in]	y				発生したＹ座標値
	@param[in]	keyFlags		他に押されてるキーについて
*/
VOID Evw_OnLButtonUp( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	UINT	dRslt, bSqSel;

	INT		iCrLf;
	LPTSTR	ptString = NULL;
	UINT	cbSize;

	INT		xPos, yPos;

	TRACE( TEXT("マウス左アップ[%d / %d]"), x, y );

	//	ダブルクルック操作後はすることはない
	if( gbLDoubleClick ){	gbLDoubleClick =  FALSE;	 return;	}

	ViewSelRangeCheck( FALSE  );	//	とりあえず選択範囲の様子確認

	ReleaseCapture(   );	//	マウスキャプチャ解除

	//	Downのほうから	20120328
	if( !(gbExtract) )	//	抽出モード中は処理しない
	{
		ViewBrushFilling(  );	//	ブラシする

		//	最終的にここで解除
		if( (gstLClicken.x == gdDocXdot) && (gstLClicken.y == gdDocLine) )
		{
			ViewSelMoveCheck( FALSE );
			gbDragMoved = FALSE;
		}

		if( gbDragMoved )	//	ドラッグ移動完了・ここに移動させる
		{
			//	キャレットが選択範囲中なら何もしない
			dRslt = DocLetterSelStateGet( gdDocXdot, gdDocLine );
			if( !(dRslt)  )
			{
				IsSelecting( &bSqSel );	//	矩形であるか
				//選択範囲をコピペしてから。内部処理なのでユニコードでよろし
				cbSize = DocSelectTextGetAlloc( D_UNI | bSqSel, (LPVOID *)(&ptString), NULL );
				iCrLf = DocInsertString( &gdDocXdot, &gdDocLine, NULL, ptString, bSqSel, TRUE );
				xPos = gdDocXdot;	yPos = gdDocLine;
				FREE( ptString );
				//	選択範囲を削除
				DocSelRangeReset( NULL , NULL );	//	選択範囲がずれてまうのでリセット
				iCrLf = DocSelectedDelete( &gdDocXdot, &gdDocLine, bSqSel, FALSE );

				//REDRAW指示
				ViewRedrawSetLine( -1 );	//	範囲不明なので全画面書換

				ViewPosResetCaret( xPos, yPos );	//	キャレット移動
			}
			gbDragMoved = FALSE;
		}
	}
	

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ビューでマウスの右ボタンがダウンされたとき・コンテキストメニューの前
	@param[in]	hWnd			ウインドウハンドル・ビューのとは限らないので注意セヨ
	@param[in]	fDoubleClick	非０ダブルクルックされた場合
	@param[in]	x				発生したＸ座標値
	@param[in]	y				発生したＹ座標値
	@param[in]	keyFlags		他に押されてるキーについて
*/
VOID Evw_OnRButtonDown( HWND hWnd, BOOL fDoubleClick, INT x, INT y, UINT keyFlags )
{
	//	キャレット移動のみ面倒見る
	//	20110704	選択中はキャレット移動しないようにする

	INT		dX, dY;	//	
	INT		dDot, dMaxDot, dLine, iMaxLine;	//	

	SetFocus( hWnd );	//	マウスインでフォーカス

	if( IsSelecting( NULL ) )	//	選択作業中であるならなにもしない
	{
		TRACE( TEXT("[%X]マウス右ダウン　%d:%d　選択中"), hWnd, x, y );
		return;
	}

	dX = x;
	dY = y;
	ViewPositionTransform( &dX, &dY, 0 );	//	ここで、ドキュメント位置に変更

	dDot  = dX;
	dLine = dY / LINE_HEIGHT;

	TRACE( TEXT("[%X]マウス右ダウン[%d:%d[%d] / %d:%d:%d]"), hWnd, dX, dY, dLine, gbShiftOn, gbCtrlOn, gbAltOn );


	if( 0 <= dX || 0 <= dY )	//	マイナスのときはルーラーか行番号エリア
	{
		//	函数にしておくか

		//	有効な位置かどうか確認

		//	行数確認して、はみ出してたら末端にしておく
		iMaxLine = DocNowFilePageLineCount(  );//DocPageParamGet( NULL, NULL );
		if( iMaxLine <=dLine )	dLine = iMaxLine - 1;

		//	その行のドット数を確認して、はみ出してたら末端にしておく
		dMaxDot = DocLineParamGet( dLine, NULL, NULL );
		if( dMaxDot <=dDot )	dDot = dMaxDot;

		//	文字位置に合わせて調整
		gdDocMozi = DocLetterPosGetAdjust( &dDot, dLine, 0 );	//	今の文字位置を確認
		gdDocXdot = dDot;
		gdDocLine = dLine;

		ViewDrawCaret( gdDocXdot, gdDocLine, 1 );	//	位置移動

		ViewSelMoveCheck( FALSE );

		ViewSelPositionSet( NULL );	//	移動した位置を記録

	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ホイール大回転
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	xPos	スクリーンＸ座標
	@param[in]	yPos	スクリーンＹ座標
	@param[in]	zDelta	回転量・１２０単位・WHEEL_DELTA
	@param[in]	fwKeys	他に押されていたキー
*/
VOID Evw_OnMouseWheel( HWND hWnd, INT xPos, INT yPos, INT zDelta, UINT fwKeys )
{
	UINT	dCode;

	HWND	hChdWnd;
	POINT	stPoint;

	TRACE( TEXT("POS[%d x %d] DELTA[%d] K[%X]"), xPos, yPos, zDelta, fwKeys );
	//	fwKeys	SHIFT 0x4, CTRL 0x8

	stPoint.x = xPos;
	stPoint.y = yPos;
	ScreenToClient( ghPrntWnd, &stPoint );
	hChdWnd = ChildWindowFromPointEx( ghPrntWnd, stPoint, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED | CWP_SKIPTRANSPARENT );


	if( ghViewWnd != hChdWnd )	return;

	if( 0 < zDelta )		dCode = SB_LINEUP;
	else if( 0 > zDelta )	dCode = SB_LINEDOWN;
	else					dCode = SB_ENDSCROLL;

	//	posを、ホイールフラグにすればいい
	FORWARD_WM_VSCROLL( ghViewWnd, NULL, dCode, 1, PostMessage );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ユニコード空白の挿入
	@param[in]	dCommando	種類
	@return	INT	文字幅
*/
INT ViewInsertUniSpace( UINT dCommando )
{
	INT	width;
	TCHAR	ch;

	TRACE( TEXT("挿入：ユニコード空白") );

	switch( dCommando )
	{
		case IDM_IN_01SPACE:	ch  = (TCHAR)0x200A;	break;	//	8202
		case IDM_IN_02SPACE:	ch  = (TCHAR)0x2009;	break;	//	8201
		case IDM_IN_03SPACE:	ch  = (TCHAR)0x2006;	break;	//	8198
		case IDM_IN_04SPACE:	ch  = (TCHAR)0x2005;	break;	//	8197
		case IDM_IN_05SPACE:	ch  = (TCHAR)0x2004;	break;	//	8196
		case IDM_IN_08SPACE:	ch  = (TCHAR)0x2002;	break;	//	8194
		case IDM_IN_10SPACE:	ch  = (TCHAR)0x2007;	break;	//	8199
		case IDM_IN_16SPACE:	ch  = (TCHAR)0x2003;	break;	//	8195
		default:	return 0;
	}

	width = DocInsertLetter( &gdDocXdot, gdDocLine, ch );	//	行に追加
	//	中でアンドゥバッファリング
	gdDocMozi = DocLetterPosGetAdjust( &gdDocXdot, gdDocLine, 0 );
	ViewDrawCaret( gdDocXdot, gdDocLine, 1 );	//	位置移動
	ViewRedrawSetLine( gdDocLine  );	//	ビューの書き直しが必要

	DocPageInfoRenew( -1, 1 );

	return width;
}
//-------------------------------------------------------------------------------------------------

/*!
	台詞用改行の処理する
*/
HRESULT ViewScriptedLineFeed( VOID )
{
//押した行の、最後の空白＝文字列の先頭部分を探し、
//次の行をそこまで空白パディングして、カーソル移動・行を増やすのとは違う処理
//めり込んでたらカーソル移動だけ・次の行が無かったら、増やしてパディングする

//空白が出っ張ってる行でやったら、そこを基準点とする処理する

	INT		dLines, iTgtDot, iLastDot, iLineDot, iPadDot;
	INT		iPrvDot, iChkDot;
	BOOLEAN	bIsSp, bFirst = TRUE, bJump;
	UINT	dStyle = 0;
	LPTSTR	ptSpace;

	iChkDot = gdDocXdot;
	iTgtDot = 0;	//	安全確認

	while( iChkDot )
	{
		//	文字列の開始地点を探す。iTgtDotがその位置のはず
		DocLineStateCheckWithDot( iChkDot, gdDocLine, &iTgtDot, &iLastDot, NULL, NULL, &bIsSp );

		//	チェック部分が空白ではなく、先頭までイッてしまったら
		if(  0 == iTgtDot && !(bIsSp) ){	break;	}	//	すなわち先頭部分まで移動

		//	該当箇所が最初からスペースだったら、キャレット位置を基準点にする・壱行空け用
		if( bIsSp ){	iTgtDot = iChkDot;	break;	}

		DocLetterShiftPos( iTgtDot, gdDocLine, -1, &iPrvDot, &bJump );	//	壱文字戻る
		//	戻ったところも空白かどうかチェック
		DocLineStateCheckWithDot( iPrvDot, gdDocLine, &iChkDot, &iLastDot, NULL, NULL, &bIsSp );
		if( bIsSp ){	break;	}	//	引き続き空白なら、そこまでとする。

		iChkDot = iPrvDot;	//	一つ戻った所からチェック続行
	}

	TRACE( TEXT("TEXT START D[%d] L[%d]"), iTgtDot, gdDocLine );

	dLines = DocNowFilePageLineCount(  );//DocPageParamGet( NULL , NULL );	//	行数確保
	if( (dLines - 1) <= gdDocLine )	//	最終行だったら
	{
		DocAdditionalLine( 1, &bFirst );//	bFirst = FALSE;
		ViewRedrawSetLine( gdDocLine );
	}//末端に壱行追加

	gdDocLine++;	//	次の行に移動

	iLineDot = DocLineParamGet( gdDocLine, NULL, NULL );	//	次の行のドット数確認
	if( iTgtDot <= iLineDot )	//	ターゲットドット位置より長かったら
	{
		gdDocXdot = iTgtDot;	//	とりやえず合わせる
		DocLetterPosGetAdjust( &gdDocXdot, gdDocLine, 0 );	//	Caret位置調整
		ViewDrawCaret( gdDocXdot, gdDocLine, 1 );	//	位置移動
	}
	else
	{
		iPadDot = iTgtDot - iLineDot;	//	埋めるのに必要なドット数
		ptSpace = DocPaddingSpaceMake( iPadDot );	//	埋め空白作る
		gdDocXdot = iLineDot;	//	末端に合わせる
		//	空白文字列追加
		DocInsertString( &gdDocXdot, &gdDocLine, NULL, ptSpace, dStyle, bFirst );	bFirst = FALSE;
		//	キャレット位置変更は↑でやってる
		FREE(ptSpace);
	}

	ViewRedrawSetLine( gdDocLine  );	//	今操作した行を書き直し


	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	したらば用色指定タグの挿入
	@param[in]	dCommando	種類
	@return	INT	文字幅
*/
INT ViewInsertColourTag( UINT dCommando )
{
	UINT	dCrLf;
	INT		dDot;
	TCHAR	atString[MAX_STRING];

	switch( dCommando )
	{
		case IDM_INSTAG_WHITE:	StringCchCopy( atString, MAX_STRING, COLOUR_TAG_WHITE );	break;
		case IDM_INSTAG_BLUE:	StringCchCopy( atString, MAX_STRING, COLOUR_TAG_BLUE  );	break;
		case IDM_INSTAG_BLACK:	StringCchCopy( atString, MAX_STRING, COLOUR_TAG_BLACK );	break;
		case IDM_INSTAG_RED:	StringCchCopy( atString, MAX_STRING, COLOUR_TAG_RED   );	break;
		case IDM_INSTAG_GREEN:	StringCchCopy( atString, MAX_STRING, COLOUR_TAG_GREEN );	break;
		default:	return 0;
	}

	dDot = gdDocXdot;

	dCrLf = DocInsertString( &gdDocXdot, &gdDocLine, &gdDocMozi, atString, 0, TRUE );

	dDot = gdDocXdot- dDot;

	DocPageInfoRenew( -1, 1 );

	return dDot;
}
//-------------------------------------------------------------------------------------------------

/*!
	テンプレからの文字列を挿入
	@param[in]	ptText	挿入する文字列
	@return		使ったドット数
*/
INT ViewInsertTmpleString( LPCTSTR ptText )
{
	UINT	dCrLf;
	INT		dDot;

	dDot = gdDocXdot;

	dCrLf = DocInsertString( &gdDocXdot, &gdDocLine, &gdDocMozi, ptText, 0, TRUE );

	dDot = gdDocXdot- dDot;

	DocPageInfoRenew( -1, 1 );

	return dDot;
}
//-------------------------------------------------------------------------------------------------

/*!
	ブラシモードの設定をうける
	@param[in]	bBrushOn	ブラシモードか否か
	@param[in]	ptPattern	使用するブラシパヤーン
	@return		HRESULT	終了状態コード
*/
HRESULT ViewBrushStyleSetting( UINT bBrushOn, LPTSTR ptPattern )
{
	gbBrushMode = bBrushOn;

	//	メニューのcheckTOGGLE
	SendMessage( ghPrntWnd, WMP_BRUSH_TOGGLE, (WPARAM)bBrushOn, (LPARAM)IDM_BRUSH_STYLE );

	if( ptPattern ){	StringCchCopy( gatBrushPtn, SUB_STRING, ptPattern );	}

	OperationOnStatusBar(  );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ブラシモードなら、ブラシ的処理をする
	@return		HRESULT	終了状態コード
*/
HRESULT ViewBrushFilling( VOID )
{
	INT			dTgDot;
	INT			dLeft, dRight, iBgnMozi, iCntMozi, rslt;
	BOOLEAN		bSpace, bFirst = TRUE;
	LPTSTR		ptBuff;
	wstring		wsBuff;


	if( !(gbBrushMode) )	return S_FALSE;

	//	選択範囲があるなら、そっちを優先して塗りつぶす
	rslt = DocSelectedBrushFilling( gatBrushPtn, &gdDocXdot, &gdDocLine );
	if( rslt )	return S_OK;

	//	クルッコ位置が空白かどうか
	dTgDot = DocLineStateCheckWithDot( gdDocXdot, gdDocLine, &dLeft, &dRight, &iBgnMozi, &iCntMozi, &bSpace );
	if( !(bSpace)  )	return S_FALSE;

//塗り潰し文字列作成
	ptBuff = BrushStringMake( dTgDot, gatBrushPtn );
	if( !(ptBuff) )
	{
		NotifyBalloonExist( TEXT("ブラシを選んでおいてね"), TEXT("操作ミス"), NIIF_INFO );
		return E_OUTOFMEMORY;
	}

	//	その範囲の空白を削除して
	DocRangeDeleteByMozi( dLeft, gdDocLine, iBgnMozi, (iBgnMozi + iCntMozi), &bFirst );
	//	ブラシ文字列をぶち込む
	DocInsertString( &dLeft, &gdDocLine, NULL, ptBuff, 0, bFirst );	bFirst = FALSE;

	//	再度調整・キャレット移動はこの後
	gdDocMozi = DocLetterPosGetAdjust( &gdDocXdot, gdDocLine, 0 );

	FREE(ptBuff);

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定の幅とパターンで、ブラシる文字列を作る。余りは適当に埋める・ずれても気にしない
	@param[in]	dDotLen		作成するドット幅
	@param[in]	ptPattern	使用するブラシパヤーン
	@return		LPTSTR	作成した文字列・開放は呼んだ方で面倒見る・作成出来なかったらNULL
*/
LPTSTR BrushStringMake( INT dDotLen, LPTSTR ptPattern )
{
	INT			dPtnDot, dCnt, dAmr, i, wid;
#if 0
	INT			chk;
#endif
	UINT_PTR	cchSize;
	LPTSTR		ptBuff;//, ptPadd = NULL;
	wstring		wsBuff;

//塗り潰し文字列作成
	wsBuff.clear( );

	//	ブラシの幅確認して
	dPtnDot = ViewStringWidthGet( ptPattern );
	if( 0 >= dPtnDot ||  0 >= dDotLen ){	return NULL;	}
	//	なんかいろいろおかしかったら終わる

	dCnt = dDotLen / dPtnDot;	//	必要な数を確認
	dAmr = dDotLen - (dCnt * dPtnDot);	//	余り

	//	規定の文字列作成
	for( i = 0; dCnt > i; i++ ){	wsBuff += ptPattern;	}

	//	あまりを出来る限り埋める
	i = 0;
	while( 0 < dAmr )	//	余り幅が無くなったら終わり
	{
		if(  0 == ptPattern[i] )	break;	//	それ以上パヤーンがなかったら

		wid = ViewLetterWidthGet( ptPattern[i] );	//	順番に見ていく
#if 0
		if( gbUniPad )	//	ユニコード空白使うなら、はみ出す直前で終わっておく
		{
			chk = dAmr - wid;
			if( 0 > chk )	break;
		}
#endif
		wsBuff += ptPattern[i];
		dAmr -= wid;	//	壱文字毎の幅で埋めていく
		i++;
	}

	cchSize = wsBuff.size( ) + 8;	//	とりあえず余裕しておく
	ptBuff = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
	ZeroMemory( ptBuff, cchSize * sizeof(TCHAR) );
	StringCchCopy( ptBuff, cchSize, wsBuff.c_str( ) );
#if 0
	if( gbUniPad )	//	ユニコード空白で、余った数ドットを埋めておく
	{
		ptPadd = DocPaddingSpaceUni( dAmr, NULL, NULL, NULL );
		if( ptPadd )
		{
			StringCchCat( ptBuff, cchSize, ptPadd );
			FREE(ptPadd);
		}
	}
#endif

	return ptBuff;
}
//-------------------------------------------------------------------------------------------------

/*!
	WM_IME_COMPOSITIONメッセージ
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	wParam		追加の情報１
	@param[in]	lParam		追加の情報２
*/
VOID Evw_OnImeComposition( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
	HIMC	hImc;
	LONG	cbSize;
	LPTSTR	ptBuffer;
	UINT_PTR	cchSize;

	BOOLEAN	bSelect = FALSE;
	UINT	bSqSel  = 0;

	TRACE( TEXT("WM_IME_COMPOSITION[0x%X][0x%X]"), wParam, lParam );

	bSelect = IsSelecting( &bSqSel );
	gdSqFillCnt = 0;

	if( (GCS_RESULTSTR & lParam) && bSelect && bSqSel )
	{
		hImc = ImmGetContext( ghViewWnd );	//	IMEハンドル確保

		cbSize = ImmGetCompositionString( hImc, GCS_RESULTSTR, NULL, 0 );
		//	得られるのはバイト数・文字数じゃない

		//	確定した文字列を確保
		cbSize += 2;
		ptBuffer = (LPTSTR)malloc( cbSize );
		ZeroMemory( ptBuffer, cbSize );
		ImmGetCompositionString( hImc, GCS_RESULTSTR, ptBuffer, cbSize );
		TRACE( TEXT("COMPOSITION MOZI[%d][%s]"), cbSize, ptBuffer );
		ImmReleaseContext( ghViewWnd , hImc );

		//	塗り潰し処理
		DocSelectedBrushFilling( ptBuffer, &gdDocXdot ,&gdDocLine );
		ViewDrawCaret( gdDocXdot, gdDocLine, 1 );	//	キャレット位置を決める
		DocPageInfoRenew( -1, 1 );

		StringCchLength( ptBuffer, cbSize, &cchSize );
		gdSqFillCnt = cchSize;

		FREE(ptBuffer);
	}
/*
GCS_COMPREADSTR			0x0001	現在の編集のリーディング文字列が取得された、または最新化された
GCS_COMPREADATTR		0x0002	現在の編集のリーディング文字列が取得された、または最新化された
GCS_COMPREADCLAUSE		0x0004	編集文字列のリーディング文字列の文節情報が取得された、または最新化された
GCS_COMPSTR				0x0008	現在の編集文字列が取得された、または最新化された
GCS_COMPATTR			0x0010	編集文字列のアトリビュートが取得された、または最新化された
GCS_COMPCLAUSE			0x0020	編集文字列の文節情報が取得された、または最新化された
GCS_CURSORPOS			0x0080	編集文字列のカーソル位置が取得した、または最新化された
GCS_DELTASTART			0x0100	編集文字列の変化の開始位置が取得された、または最新化された
GCS_RESULTREADSTR		0x0200	リーディング文字列を取得した、または最新化された
GCS_RESULTREADCLAUSE	0x0400	リーディング文字列の文節情報が取得された、または最新化された
GCS_RESULTSTR			0x0800	確定文字列が取得された、または最新化された
GCS_RESULTCLAUSE		0x1000	確定文字列の文節情報が取得された、または最新化された
*/
	return;
}
//-------------------------------------------------------------------------------------------------

