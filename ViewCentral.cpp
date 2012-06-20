/*! @file
	@brief 編集ビューの管理とか描画します
	このファイルは ViewCentral.cpp です。
	@author	SikigamiHNQ
	@date	2011/04/15
*/

/*
Orinrin Editor : AsciiArt Story Editor for Japanese Only
Copyright (C) 2011 - 2012 Orinrin/SikigamiHNQ

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

//	
//	編集ウインドウの制御する・タブとかも？

//	初回起動時は、説明ＡＳＴを表示する
#define FIRST_STEP	TEXT("説明書.ast")

//-------------------------------------------------------------------------------------------------

#define EDIT_VIEW_CLASS	TEXT("EDIT_VIEW")
//-------------------------------------------------------------------------------------------------

/*

描画用文字列の受け取り
文字自体と、描画指定コードによる構造体配列でやりとりするとか

ペンと背景は都度変更・前回と同じなら入れ替え不要で

画面サイズを常に意識する

描画函数はカプセル化しておく。描画函数は、文字列のデータを受け取って、
同じ色とか背景のやつでテキストつくって、色と文字列で渡せばいい
ルーラーと行番号の描画枠の分をずらすのは描画カプセル函数の中でやればいい
カーソル位置とかも同じ・描画領域原点で意識すればいい

上部ルーラーに半行分、行番号表示に３文字と、空白、１ドット線使う

各描画函数と、Ｄｏｃ系函数が受け取るＸＹ値は、内容を完全に表示したときの、
ドキュメント自体の絶対位置・ルーラーとか、スクロールは考慮されていない

各描画函数内で、描画位置シフト函数を挟んで、描画位置をトランスフォームする

*/

//!	色見本のドロー用
typedef struct tagCOLOUROBJECT
{
	COLORREF	dTextColour;	//	文字色
	HBRUSH		hBackBrush;		//	背景色
	HPEN		hGridPen;		//	グリッド
	HPEN		hCrLfPen;		//	改行マーク
	HBRUSH		hUniBackBrs;	//	ユニコード文字背景

} COLOUROBJECT, *LPCOLOUROBJECT;
//-------------------------------------------------------------------------------------------------

static HINSTANCE	ghInst;		//!<	現在のインターフェイス

EXTERNED HWND	ghPrntWnd;		//!<	メインウインドウハンドル
EXTERNED HWND	ghViewWnd;		//!<	このウインドウのハンドル

extern  HWND	ghPgVwWnd;		//	ページリスト
extern  HWND	ghMaaWnd;		//	複数行ＡＡテンプレ
extern  HWND	ghLnTmplWnd;	//	壱行テンプレ
extern  HWND	ghBrTmplWnd;	//	ブラシテンプレ
extern BOOLEAN	gbDockTmplView;	//	くっついてるテンプレは見えているか


EXTERNED INT	gdXmemory;		//!<	直前のＸ位置を覚えておく
EXTERNED INT	gdDocXdot;		//!<	キャレットのＸドット・ドキュメント位置
EXTERNED INT	gdDocLine;		//!<	キャレットのＹ行数・ドキュメント位置
EXTERNED INT	gdDocMozi;		//!<	キャレットの左側の文字数

//	画面サイズを確認して、移動によるスクロールの面倒みる
EXTERNED INT	gdHideXdot;		//!<	左の隠れ部分
EXTERNED INT	gdViewTopLine;	//!<	表示中の最上部行番号
EXTERNED SIZE	gstViewArea;	//!<	表示領域のドットサイズ・ルーラー等の領域は無し
EXTERNED INT	gdDispingLine;	//!<	見えてる行数・中途半端に見えてる末端は含まない



EXTERNED BOOLEAN	gbExtract;	//!<	抽出モード中

//	フォントは描画毎にデバイスコンテキストに割り付ける必要がある
EXTERNED HFONT	ghAaFont;		//!<	AA用フォント
static HFONT	ghRulerFont;	//!<	ルーラー用フォント
static HFONT	ghNumFont4L;	//!<	行番号用フォント４桁用
static HFONT	ghNumFont5L;	//!<	行番号用フォント５桁用
static HFONT	ghNumFont6L;	//!<	行番号用フォント６桁用


static INT		gdAutoDiffBase;	//!<	自動調整のベース

static  UINT	gdUseMode;		//!<	MAAからの左クルックによる使用スタイルの指示
static  UINT	gdUseSubMode;	//!<	MAAからの中クルックによる使用スタイルの指示

static  UINT	gdSpaceView;	//!<	空白を表示する

EXTERNED UINT	gdGridXpos;		//!<	グリッド線のＸ間隔
EXTERNED UINT	gdGridYpos;		//!<	グリッド線のＹ間隔
EXTERNED UINT	gdRightRuler;	//!<	右線の位置

static BOOLEAN	gbGridView;		//!<	グリッド表示するか
static BOOLEAN	gbRitRlrView;	//!<	右線表示するか

static  UINT	gdWheelLine;	//!<	マウスホウィールの行移動量のOS標準

extern  UINT	gbSqSelect;		//		矩形選択中である
extern  UINT	gbBrushMode;	//		ブラシ中である

extern INT		gixFocusPage;	//	注目中のページ・とりあえず０・０インデックス

extern INT		gbTmpltDock;	//	ページ窓のドッキング

extern  HWND	ghMainSplitWnd;	//	メインのスプリットバーハンドル
extern  LONG	grdSplitPos;	//	スプリットバーの、左側の、画面右からのオフセット

#ifdef PLUGIN_ENABLE
//---------------------------------------------------------------------
//[_16in] Add - 2012/05/01
//
//-- プラグイン関係
extern plugin::PLUGIN_FILE_LIST gPluginList;
#endif
//-------------------------------------------------------------------------------------------------

//	使用する色

/*
色を確保する函数
ViewMoziColourGet
ViewBackColourGet
用意していく・統合を？
*/

//	色
static COLORREF	gaColourTable[] = { 
	0x000000,
	0xFFFFFF, 0xABABAB, 0x0000FF, 0xAAAAAA, 0x000000,
	0xFFFFFF, 0x8080FF, 0xC0C000, 0xC0C000, 0x101010,
	0xEEEEEE, 0xFFCCCC, 0xFF0000, 0xE0E0E0, 0x00FFFF
};

#define CLRT_BASICPEN	0	//	基本文字色
#define CLRT_BASICBK	1	//	基本背景色
#define CLRT_SELECTBK	2	//	選択状態の背景色
#define CLRT_SPACEWARN	3	//	連続空白警告色
#define CLRT_SPACELINE	4	//	スペースの色
#define CLRT_CARETFD	5	//	キャレット色
#define CLRT_CARETBK	6	//	キャレット背景
#define CLRT_LASTSPWARN	7	//	行端空白の警告色
#define CLRT_CRLF_MARK	8	//	改行の色
#define CLRT_EOF_MARK	9	//	ＥＯＦの色
#define CLRT_RULER		10	//	ルーラの色
#define CLRT_RULERBK	11	//	ルーラの背景色
#define CLRT_CANTSJIS	12	//	非SJIS文字の背景色
#define CLRT_CARET_POS	13	//	ルーラーのCARET表示色
#define CLRT_GRID_LINE	14	//	グリッド線の色
#define CLRT_FINDBACK	15	//	検索ヒット文字列の背景色

//	ペン
#define PENS_MAX	6
static  HPEN	gahPen[PENS_MAX];
#define PENT_CRLF_MARK	0
#define PENT_RULER		1
#define PENT_SPACEWARN	2
#define PENT_SPACELINE	3
#define PENT_CARET_POS	4
#define PENT_GRID_LINE	5

//	ブラシ
#define BRUSHS_MAX	6
static  HBRUSH	gahBrush[BRUSHS_MAX];
#define BRHT_BASICBK	0
#define BRHT_RULERBK	1
#define BRHT_SELECTBK	2
#define BRHT_LASTSPWARN	3
#define BRHT_CANTSJISBK	4
#define BRHT_FINDBACK	5

//-------------------------------------------------------------------------------------------------

//static LOGFONT	gstBaseFont = {
//	FONTSZ_NORMAL,			//	フォントの高さ
//	0,						//	平均幅
//	0,						//	文字送りの方向とX軸との角度
//	0,						//	ベースラインとX軸との角度
//	FW_NORMAL,				//	文字の太さ(0~1000まで・400=nomal)
//	FALSE,					//	イタリック体
//	FALSE,					//	アンダーライン
//	FALSE,					//	打ち消し線
//	DEFAULT_CHARSET,		//	文字セット
//	OUT_OUTLINE_PRECIS,		//	出力精度
//	CLIP_DEFAULT_PRECIS,	//	クリッピング精度
//	PROOF_QUALITY,			//	出力品質
//	VARIABLE_PITCH,			//	固定幅か可変幅
//	TEXT("ＭＳ Ｐゴシック")	//	フォント名
//};

//-------------------------------------------------------------------------------------------------

LRESULT	CALLBACK ViewWndProc( HWND, UINT, WPARAM, LPARAM );
BOOLEAN	Evw_OnCreate( HWND, LPCREATESTRUCT );		//!<	WM_CREATE の処理・固定Editとかつくる
VOID	Evw_OnCommand( HWND , INT, HWND, UINT );	//!<	WM_COMMAND の処理
VOID	Evw_OnPaint( HWND );						//!<	WM_PAINT の処理・枠線描画とか
VOID	Evw_OnDestroy( HWND );						//!<	WM_DESTROY の処理・BRUSHとかのオブジェクトの破壊を忘れないように
VOID	Evw_OnVScroll( HWND, HWND, UINT, INT );
VOID	Evw_OnHScroll( HWND, HWND, UINT, INT );
VOID	Evw_OnContextMenu( HWND, HWND, UINT, UINT );


//	親ウインドウから回す必要が有る
//VOID	Evw_OnKey( HWND, UINT, BOOL, INT, UINT );	//!<	
//VOID	Evw_OnChar( HWND, TCHAR, INT );				//!<	

HRESULT	ViewScrollBarAdjust( LPVOID );

HRESULT	ViewRedrawDo( HWND, HDC );
HRESULT	ViewDrawMetricLine( HDC,UINT );
BOOLEAN	ViewDrawTextOut( HDC, INT, UINT, LPLETTER, UINT_PTR );
BOOLEAN	ViewDrawSpace( HDC, INT, UINT, LPTSTR, UINT_PTR, UINT );
HRESULT	ViewDrawReturnMark( HDC, INT, INT, UINT );
INT		ViewDrawEOFMark( HDC, INT, INT, UINT );

HRESULT	ViewDrawRuler( HDC );
HRESULT	ViewDrawLineNumber( HDC );

VOID	OperationUndoRedo( INT, PINT, PINT );

//	配色変更
HRESULT	ViewColourEditDlg( HWND );
INT_PTR	CALLBACK ColourEditDlgProc( HWND, UINT, WPARAM, LPARAM );
UINT	ColourEditChoose( HWND, LPCOLORREF );
INT_PTR	ColourEditDrawItem( HWND, CONST LPDRAWITEMSTRUCT, LPCOLOUROBJECT );

//-------------------------------------------------------------------------------------------------

VOID AaFontCreate( UINT bMode )
{
	LOGFONT	stFont;

	ViewingFontGet( &stFont );

	if( bMode ){	ghAaFont = CreateFontIndirect( &stFont );	}	//	gstBaseFont
	else{			DeleteFont( ghAaFont  );	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ビューウインドウの作成
	@param[in]	hInstance	アプリのインスタンス
	@param[in]	hParentWnd	親ウインドウのハンドル
	@param[in]	pstFrame	親ウインドウのクライヤントサイズ
	@param[in]	ptArgv		コマンドラインで渡されたファイル名・無ければ０クルヤ状態・NULLではない
	@return		作ったビューのウインドウハンドル
*/
HWND ViewInitialise( HINSTANCE hInstance, HWND hParentWnd, LPRECT pstFrame, LPTSTR ptArgv )
{
	TCHAR		atFile[MAX_PATH], atFirstStep[MAX_PATH];
	WNDCLASSEX	wcex;
	RECT		vwRect, rect;

	LOGFONT		stFont;

//	INT			iNewPage;
	INT			iFiles, i;
	LPARAM		dNumber;
	BOOLEAN		bOpen = FALSE;

	INT			spPos, iOpMode, iRslt;

	ghInst = hInstance;

	//	マウスホウィールの行移動量のＯＳ設定をゲッツ！
	gdWheelLine = 0;
	SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &gdWheelLine, 0 );
	if( 0 == gdWheelLine )	gdWheelLine = 3;	//	デフォは３

	//	デフォ色有るならそれを指定
	gaColourTable[CLRT_SELECTBK] = GetSysColor( COLOR_HIGHLIGHT );

	//	初期状態はここで上書きすればいい
	gaColourTable[CLRT_BASICPEN]  = InitColourValue( INIT_LOAD, CLRV_BASICPEN, gaColourTable[CLRT_BASICPEN] );
	gaColourTable[CLRT_BASICBK]   = InitColourValue( INIT_LOAD, CLRV_BASICBK,  gaColourTable[CLRT_BASICBK] );
	gaColourTable[CLRT_GRID_LINE] = InitColourValue( INIT_LOAD, CLRV_GRIDLINE, gaColourTable[CLRT_GRID_LINE] );
	gaColourTable[CLRT_CRLF_MARK] = InitColourValue( INIT_LOAD, CLRV_CRLFMARK, gaColourTable[CLRT_CRLF_MARK] );
	gaColourTable[CLRT_CANTSJIS]  = InitColourValue( INIT_LOAD, CLRV_CANTSJIS, gaColourTable[CLRT_CANTSJIS] );

	//	背景色作成
	gahBrush[BRHT_BASICBK]    = CreateSolidBrush( gaColourTable[CLRT_BASICBK] );
	gahBrush[BRHT_RULERBK]    = CreateSolidBrush( gaColourTable[CLRT_RULERBK] );
	gahBrush[BRHT_SELECTBK]   = CreateSolidBrush( gaColourTable[CLRT_SELECTBK] );
	gahBrush[BRHT_LASTSPWARN] = CreateSolidBrush( gaColourTable[CLRT_SPACEWARN] );
	gahBrush[BRHT_CANTSJISBK] = CreateSolidBrush( gaColourTable[CLRT_CANTSJIS] );
	gahBrush[BRHT_FINDBACK]   = CreateSolidBrush( gaColourTable[CLRT_FINDBACK] );

	//	ペンも作成
	gahPen[PENT_CRLF_MARK] = CreatePen( PS_SOLID, 1, gaColourTable[CLRT_CRLF_MARK] );
	gahPen[PENT_RULER]     = CreatePen( PS_SOLID, 1, gaColourTable[CLRT_RULER] );
	gahPen[PENT_SPACEWARN] = CreatePen( PS_SOLID, 1, gaColourTable[CLRT_SPACEWARN] );
	gahPen[PENT_SPACELINE] = CreatePen( PS_SOLID, 1, gaColourTable[CLRT_SPACELINE] );
	gahPen[PENT_CARET_POS] = CreatePen( PS_SOLID, 1, gaColourTable[CLRT_CARET_POS] );
	gahPen[PENT_GRID_LINE] = CreatePen( PS_SOLID, 1, gaColourTable[CLRT_GRID_LINE] );

	//	専用のウインドウクラス作成
	ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	= ViewWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_IBEAM);
	wcex.hbrBackground	= gahBrush[BRHT_BASICBK];
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= EDIT_VIEW_CLASS;
	wcex.hIconSm		= NULL;

	RegisterClassEx( &wcex );

	gdHideXdot = 0;
	gdViewTopLine  = 0;

	gdAutoDiffBase = 0;

	//	グリッド線表示制御
	gdGridXpos   = InitParamValue( INIT_LOAD, VL_GRID_X_POS, 54 );
	gdGridYpos   = InitParamValue( INIT_LOAD, VL_GRID_Y_POS, 54 );
	gbGridView   = InitParamValue( INIT_LOAD, VL_GRID_VIEW, 0 );
	MenuItemCheckOnOff( IDM_GRID_VIEW_TOGGLE, gbGridView );

	//	右ルーラ表示制御
	gdRightRuler = InitParamValue( INIT_LOAD, VL_R_RULER_POS, 800 );
	gbRitRlrView = InitParamValue( INIT_LOAD, VL_R_RULER_VIEW, 1 );
	MenuItemCheckOnOff( IDM_RIGHT_RULER_TOGGLE, gbRitRlrView );

	//	空白表示制御
	gdSpaceView = InitParamValue( INIT_LOAD, VL_SPACE_VIEW, TRUE );
	MenuItemCheckOnOff( IDM_SPACE_VIEW_TOGGLE, gdSpaceView );
	OperationOnStatusBar(  );

	ghPrntWnd =  hParentWnd;	//	親ウインドウハンドル記録


	rect = *pstFrame;
	if( gbTmpltDock )
	{
		spPos = grdSplitPos;	//	右からのオフセット
		rect.right -= spPos;
	};

	ghViewWnd = CreateWindowEx( 0, EDIT_VIEW_CLASS, NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL,
		rect.left, rect.top, rect.right, rect.bottom, hParentWnd, NULL, hInstance, NULL);

	if( !ghViewWnd ){	return NULL;	}


	ViewingFontGet( &stFont );	//	stFont = gstBaseFont;
	stFont.lfPitchAndFamily = FIXED_PITCH;
	StringCchCopy( stFont.lfFaceName, LF_FACESIZE, TEXT("ＭＳ ゴシック") );
	ghNumFont4L = CreateFontIndirect( &stFont );

	stFont.lfHeight = 13;
	ghNumFont5L = CreateFontIndirect( &stFont );

	stFont.lfHeight = 11;
	ghNumFont6L = CreateFontIndirect( &stFont );

	stFont.lfHeight = FONTSZ_REDUCE;
	stFont.lfPitchAndFamily = VARIABLE_PITCH;
	StringCchCopy( stFont.lfFaceName, LF_FACESIZE, TEXT("MS UI Gothic") );
	ghRulerFont = CreateFontIndirect( &stFont );

	GetClientRect( ghViewWnd, &vwRect );	//	スクロールバーは含んでない
	gstViewArea.cx = vwRect.right - LINENUM_WID;
	gstViewArea.cy = vwRect.bottom - RULER_AREA;

	gdDispingLine = gstViewArea.cy / LINE_HEIGHT;

	DocInitialise( TRUE );

	ZeroMemory( atFile, sizeof(atFile) );

	bOpen = FALSE;

	iOpMode = InitParamValue( INIT_LOAD, VL_LAST_OPEN, LASTOPEN_DO );	//	ラストオーポン・とりやえず開く
	if( LASTOPEN_ASK <= iOpMode )	//	質問型
	{
		iRslt = MessageBox( NULL, TEXT("最後に開いていたファイルを開くかい？"), TEXT("お燐からの確認"), MB_YESNO | MB_ICONQUESTION );
		if( IDYES == iRslt )	iOpMode = LASTOPEN_DO;
		else					iOpMode = LASTOPEN_NON;
	}

	//	INIのラストオーポン記録を確認
	if( iOpMode ){	iFiles =  0;	}	//	開いていたファイルは無いとする
	else{	iFiles = InitMultiFileTabOpen( INIT_LOAD, -1, NULL );	}

	for( i = 0; iFiles >= i; i++ )	//	コマンドラインオーポンも監視
	{
		//	最後にコマンドラインオーポンを確かめる
		if( iFiles == i ){	StringCchCopy( atFile, MAX_PATH, ptArgv );	}
		else{				InitMultiFileTabOpen( INIT_LOAD, i, atFile );	}

		dNumber = DocFileInflate( atFile  );
		if( 0 < dNumber )	//	有効なら
		{
			if( !(bOpen) )
			{
				MultiFileTabFirst( atFile );	//	最初のいっこ
				bOpen = TRUE;
			}
			else
			{
				MultiFileTabAppend( dNumber, atFile );	//	起動時の前回オーポンを開く
			}
			AppTitleChange( atFile );
		}
	}

	//オーポン記録がなければ、説明ＡＳＴを表示する
	if( 0 == InitParamValue( INIT_LOAD, VL_FIRST_READED, 0 ) )
	{
		GetModuleFileName( hInstance, atFirstStep, MAX_PATH );	//	実行ファイルの
		PathRemoveFileSpec( atFirstStep );		//	同じ所に
		PathAppend( atFirstStep, FIRST_STEP );	//	説明ＡＳＴを置いておく

		dNumber = DocFileInflate( atFirstStep );
		if( 0 < dNumber )	//	有効なら
		{
			if( !(bOpen) )
			{
				MultiFileTabFirst( atFirstStep );	//	最初のいっこ
				bOpen = TRUE;
			}
			else
			{
				MultiFileTabAppend( dNumber , atFirstStep );	//	説明ＡＳＴを開く
			}
			AppTitleChange( atFirstStep );

			InitParamValue( INIT_SAVE, VL_FIRST_READED, 1 );
		}
	}

	if( !(bOpen) )	//	完全に開けなかったら
	{
		DocActivateEmptyCreate( atFile );
		//DocMultiFileCreate( atFile );	//	新しいファイル置き場の準備・ここで返り血は要らない
		//iNewPage = DocPageCreate( -1 );	//	ページ作っておく
		//PageListInsert( iNewPage  );	//	ページリストビューに追加
		//DocPageChange( 0 );
		//MultiFileTabFirst( atFile );	//	完全新規作成
		//AppTitleChange( atFile );
	}

	ViewScrollBarAdjust( NULL );


	ShowWindow( ghViewWnd, SW_SHOW );
	UpdateWindow( ghViewWnd );

	//	キャレットつくっちゃうおｚ		
	ViewCaretCreate( ghViewWnd, gaColourTable[CLRT_CARETFD], gaColourTable[CLRT_CARETBK] );

	//	ルーラーとかに注意
	gdDocXdot = 0;
	gdDocMozi = 0;
	gdDocLine = 0;
	ViewDrawCaret( gdDocXdot, gdDocLine, 1 );	//	位置を決める

	gdXmemory = 0;

	ViewNowPosStatus(  );	//	初期値現出

	return ghViewWnd;
}
//-------------------------------------------------------------------------------------------------

/*!
	基本文字色とキャレット色を確保する
	@return	COLORREF	色
*/
COLORREF ViewMoziColourGet( LPCOLORREF pCrtColour )
{
	if( pCrtColour )	*pCrtColour = gaColourTable[CLRT_CARETFD];
	//	インデックス指定でゲットする方式のほうがいいかも
	return gaColourTable[CLRT_BASICPEN];
}
//-------------------------------------------------------------------------------------------------

//	背景色ゲット
COLORREF ViewBackColourGet( LPVOID pVoid )
{
	return gaColourTable[CLRT_BASICBK];
}
//-------------------------------------------------------------------------------------------------


/*!
	親ウインドウが移動したり大きさ変わったら
	@param[in]	hPrntWnd	親ウインドウハンドル
	@param[in]	pstFrame	クライアントサイズ
	@return		HRESULT		終了状態コード
*/
HRESULT ViewSizeMove( HWND hPrntWnd, LPRECT pstFrame )
{
	LONG	iLeftPos;
	RECT	rect;

	//	左上の位置を調整
	rect = *pstFrame;

	if( gbTmpltDock )
	{
		iLeftPos = SplitBarResize( ghMainSplitWnd, pstFrame );	//	スプリットバー
		grdSplitPos = rect.right - iLeftPos;

		PageListResize( hPrntWnd, pstFrame );
		LineTmpleResize( hPrntWnd, pstFrame );
		BrushTmpleResize( hPrntWnd, pstFrame );

		rect.right -= grdSplitPos;
		InitParamValue( INIT_SAVE, VL_MAIN_SPLIT, grdSplitPos );
	};

	//	ビューのサイズ変更
	SetWindowPos( ghViewWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW );

	GetClientRect( ghViewWnd, &rect );
	gstViewArea.cx = rect.right - LINENUM_WID;
	gstViewArea.cy = rect.bottom - RULER_AREA;

	gdDispingLine = gstViewArea.cy / LINE_HEIGHT;

	//	画面サイズ変更したときのスクロールバーと表示位置の追従
	//	行が収まらないならそのまま、収まっちゃうようなら、表示位置を最上位にする
	ViewScrollBarAdjust( NULL );

	ViewDrawCaret( gdDocXdot, gdDocLine, 1 );	//	カーソル位置再描画

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	スクロールバーの調整をする
	@param[in]	pVoid	特になし
	@return		HRESULT	終了状態コード
*/
HRESULT ViewScrollBarAdjust( LPVOID pVoid )
{
	INT	dMargin, dRange, dDot, dPos, dLines;

//Ｘバー　画面右には余裕必要か？
	dMargin = gstViewArea.cx / 2;	//	一画面の半分？

	dDot = DocPageMaxDotGet( -1, -1 );
	dRange = dMargin + dDot;
	dRange -= gstViewArea.cx;

	if( 0 >= dRange )
	{
		EnableScrollBar( ghViewWnd, SB_HORZ, ESB_DISABLE_BOTH );
		if( 0 != gdHideXdot )	//	フルに収まるけど、位置がズレていた場合
		{
			gdHideXdot = 0;
		}
	}
	else
	{
		EnableScrollBar( ghViewWnd, SB_HORZ, ESB_ENABLE_BOTH );
		SetScrollRange( ghViewWnd, SB_HORZ, 0, dRange, TRUE );
		dPos = gdHideXdot;
		if( 0 > dPos )	dPos = 0;
		SetScrollPos( ghViewWnd, SB_HORZ, dPos, TRUE );
	}

//Ｙバー　現在行数をスクロールに使う
	dLines = DocNowFilePageLineCount(  );//DocPageParamGet( NULL, NULL );	//	行数確保
	dRange = dLines - gdDispingLine;	//	全行数ー表示数＝必要SCROLL段階

//	TRACE( TEXT("SCL Y Line[%d] Range[%d] Top[%d]"), dLines, dRange, gdViewTopLine );

	if( 0 >= dRange )
	{
		EnableScrollBar( ghViewWnd, SB_VERT, ESB_DISABLE_BOTH );
		//	フルに収まるけど、位置がズレていた場合
		if( 0 != gdViewTopLine )
		{
			gdViewTopLine = 0;
			ViewRedrawSetLine( -1 );	//	ここで大きくスクロールする、かも
		}
	}
	else
	{
		EnableScrollBar( ghViewWnd, SB_VERT, ESB_ENABLE_BOTH );
		SetScrollRange( ghViewWnd, SB_VERT, 0, dRange, TRUE );
		dPos = gdViewTopLine;
		if( 0 > dPos )	dPos = 0;
		SetScrollPos( ghViewWnd, SB_VERT, dPos, TRUE );
	}




	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	素の描画位置ドットを受け取って、ルーラやスクロールを考慮した表示位置に変換する
	@param[in]	pDotX	描画位置の横ドット数・NULL不可
	@param[in]	pDotY	描画位置の縦ドット数・NULL不可
	@param[in]	bTrans	非０文書位置＞描画位置　０描画位置＞文書位置
	@return		HRESULT	終了状態コード
*/
HRESULT ViewPositionTransform( PINT pDotX, PINT pDotY, BOOLEAN bTrans )
{
	assert( pDotX );
	assert( pDotY );

	if( bTrans )	//	考慮する
	{
		*pDotX = *pDotX + LINENUM_WID;	//	行番号表示領域分シフト
		*pDotX = *pDotX - gdHideXdot;	//	隠れ領域分左へ

		*pDotY = *pDotY + RULER_AREA;	//	ルーラー表示領域分シフト
		*pDotY = *pDotY - (gdViewTopLine*LINE_HEIGHT);
	}
	else	//	外す
	{
		*pDotX = *pDotX + gdHideXdot;	//	隠れ領域分右へ
		*pDotX = *pDotX - LINENUM_WID;	//	行番号表示領域分シフト

		*pDotY = *pDotY - RULER_AREA;	//	ルーラー表示領域分シフト
		*pDotY = *pDotY + (gdViewTopLine*LINE_HEIGHT);
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定された描画ポイントが、表示枠内であるか
	@param[in]	xx	描画位置のＸドット位置
	@param[in]	yy	描画位置のＹドット位置
	@return		非０枠内である　０はみ出してる
*/
BOOLEAN ViewIsPosOnFrame( INT xx, INT yy )
{
	POINT	stPoint;
	RECT	stRect;

	SetRect( &stRect, 0, 0, gstViewArea.cx, gstViewArea.cy );

	//	ルーラと行番号の分、原点位置を左上にシフト
	stPoint.x = xx - LINENUM_WID;
	stPoint.y = yy - RULER_AREA;

	return PtInRect( &stRect, stPoint );
}
//-------------------------------------------------------------------------------------------------

/*!
	描画領域のドット数と行数をゲット
	@param[in]	pdXdot	ドット数を入れるバッファへのポインタ
	@return		行数
*/
INT ViewAreaSizeGet( PINT pdXdot )
{
	if( pdXdot )	*pdXdot = gstViewArea.cx;

	return gdDispingLine;
}
//-------------------------------------------------------------------------------------------------

/*!
	キャレット位置とスクロールをリセット
	@return		HRESULT	終了状態コード
*/
HRESULT ViewEditReset( VOID )
{
	gdDocXdot = 0;
	gdDocLine = 0;
	gdDocMozi = 0;

	//gdViewXdot = 0;
	//gdViewLine = 0;

	gdHideXdot = 0;
	gdViewTopLine = 0;

	ViewDrawCaret( 0, 0, TRUE );

	ViewScrollBarAdjust( NULL );

	ViewRedrawSetLine( -1 );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	編集ビューのウインドウプロシージャ
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	message		ウインドウメッセージの識別番号
	@param[in]	wParam		追加の情報１
	@param[in]	lParam		追加の情報２
	@retval 0	メッセージ処理済み
	@retval no0	ここでは処理せず次に回す
*/
LRESULT CALLBACK ViewWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	HIMC	hImc;
	LOGFONT	stFont;

	switch( message )
	{
		HANDLE_MSG(hWnd, WM_CREATE,         Evw_OnCreate );			//	画面の構成パーツを作る。ボタンとか
		HANDLE_MSG(hWnd, WM_PAINT,          Evw_OnPaint );			//	画面の更新とか
		HANDLE_MSG(hWnd, WM_COMMAND,        Evw_OnCommand );		//	ボタン押されたとかのコマンド処理
		HANDLE_MSG(hWnd, WM_DESTROY,        Evw_OnDestroy );		//	ソフト終了時の処理
		HANDLE_MSG(hWnd, WM_VSCROLL,        Evw_OnVScroll );		//	
		HANDLE_MSG(hWnd, WM_HSCROLL,        Evw_OnHScroll );		//	
		HANDLE_MSG(hWnd, WM_KEYDOWN,        Evw_OnKey );			//	
		HANDLE_MSG(hWnd, WM_KEYUP,          Evw_OnKey );			//	
		HANDLE_MSG(hWnd, WM_CHAR,           Evw_OnChar );			//	
		HANDLE_MSG(hWnd, WM_MOUSEMOVE,      Evw_OnMouseMove );		//	
		HANDLE_MSG(hWnd, WM_MOUSEWHEEL,     Evw_OnMouseWheel );		//	
		HANDLE_MSG(hWnd, WM_LBUTTONDOWN,    Evw_OnLButtonDown );	//	
		HANDLE_MSG(hWnd, WM_LBUTTONDBLCLK,  Evw_OnLButtonDown );	//	
		HANDLE_MSG(hWnd, WM_LBUTTONUP,      Evw_OnLButtonUp );		//	
		HANDLE_MSG(hWnd, WM_RBUTTONDOWN,    Evw_OnRButtonDown );	//	
		HANDLE_MSG(hWnd, WM_CONTEXTMENU,    Evw_OnContextMenu );	//	

/* void Cls_OnSetFocus(HWND hwnd, HWND hwndOldFocus) */
		case WM_SETFOCUS:
			TRACE( TEXT("VIEW_WM_SETFOCUS[0x%X][0x%X]"), wParam, lParam );
			ViewShowCaret(  );
			break;

/* void Cls_OnKillFocus(HWND hwnd, HWND hwndNewFocus) */
		case WM_KILLFOCUS:
			TRACE( TEXT("VIEW_WM_KILLFOCUS[0x%X][0x%X]"), wParam, lParam );
			ViewHideCaret(  );
			break;

/* void Cls_OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized) */
		case WM_ACTIVATE:
			TRACE( TEXT("VIEW_WM_ACTIVATE[0x%X][0x%X]"), wParam, lParam );
			if( WA_INACTIVE == LOWORD(wParam) ){	ViewHideCaret(  );	}
			break;

		case WM_IME_NOTIFY:
			TRACE( TEXT("WM_IME_NOTIFY[0x%X][0x%X]"), wParam, lParam );
			break;

		case WM_IME_REQUEST:
			TRACE( TEXT("WM_IME_REQUEST[0x%X][0x%X]"), wParam, lParam );
			break;

		case WM_IME_STARTCOMPOSITION:	//	変換で文字入力を開始したら発生する
			TRACE( TEXT("WM_IME_STARTCOMPOSITION[0x%X][0x%X]"), wParam, lParam );
				hImc = ImmGetContext( ghViewWnd );	//	IMEハンドル確保
				if( hImc )	//	確保出来たら
				{
					ViewingFontGet( &stFont );
					ImmSetCompositionFont( hImc , &stFont );	//	gstBaseFont
					ImmReleaseContext( ghViewWnd , hImc );
				}
			break;

		case WM_IME_ENDCOMPOSITION:
			TRACE( TEXT("WM_IME_ENDCOMPOSITION[0x%X][0x%X]"), wParam, lParam );
			break;

		case WM_IME_COMPOSITION:
			Evw_OnImeComposition( hWnd, wParam, lParam );
			break;

		default:	break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------

/*!
	編集ビューのクリエイト。
	@param[in]	hWnd			親ウインドウのハンドル
	@param[in]	lpCreateStruct	アプリケーションの初期化内容
	@return	TRUE	クリエイトできたらTRUE
*/
BOOLEAN Evw_OnCreate( HWND hWnd, LPCREATESTRUCT lpCreateStruct )
{
	HINSTANCE lcInst = lpCreateStruct->hInstance;	//	受け取った初期化情報から、インスタンスハンドルをひっぱる
	UNREFERENCED_PARAMETER(lcInst);



	return TRUE;
}
//-------------------------------------------------------------------------------------------------

/*!
	編集ビューのCOMMANDメッセージの受け取り。ボタン押されたとかで発生
	@param[in]	hWnd		親ウインドウのハンドル
	@param[in]	id			メッセージを発生させた子ウインドウの識別子	LOWORD(wParam)
	@param[in]	hWndCtl		メッセージを発生させた子ウインドウのハンドル	lParam
	@param[in]	codeNotify	通知メッセージ	HIWORD(wParam)
	@return		なし
*/
VOID Evw_OnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify )
{
	OperationOnCommand( ghPrntWnd, id, hWndCtl, codeNotify );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	編集ビューのPAINT。無効領域が出来たときに発生。背景の扱いに注意。背景を塗りつぶしてから、オブジェクトを描画
	@param[in]	hWnd	親ウインドウのハンドル
	@return		無し
*/
VOID Evw_OnPaint( HWND hWnd )
{
	PAINTSTRUCT	ps;
	HDC			hdc;

	hdc = BeginPaint( hWnd, &ps );

	ViewRedrawDo( hWnd, hdc );

	EndPaint( hWnd, &ps );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	編集ビューのウインドウを閉じるときに発生。デバイスコンテキストとか確保した画面構造のメモリとかも終了。
	@param[in]	hWnd	親ウインドウのハンドル
	@return		無し
*/
VOID Evw_OnDestroy( HWND hWnd )
{
	UINT	i;

	SetWindowFont( hWnd, GetStockFont(DEFAULT_GUI_FONT), FALSE );
	DeleteFont( ghRulerFont );
	DeleteFont( ghNumFont4L );
	DeleteFont( ghNumFont5L );
	DeleteFont( ghNumFont6L );

	ViewCaretDelete(  );

	for( i = 0; PENS_MAX > i; i++ )
	{
		DeletePen( gahPen[i] );
	}

	for( i = 0; BRUSHS_MAX > i; i++ )
	{
		DeleteBrush( gahBrush[i] );
	}

	DocMultiFileDeleteAll(  );

	PostQuitMessage( 0 );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	編集ビューの横スクロールバーが操作された
	@param[in]	hWnd	ウインドウハンドル・ビューのとは限らないので注意セヨ
	@param[in]	hWndCtl	スクロールバーのハンドル・コントロールにくっついているので０である
	@param[in]	code	スクロールコード
	@param[in]	pos		スクロールボックス（つまみ）の位置
	@return		無し
*/
VOID Evw_OnHScroll( HWND hWnd, HWND hWndCtl, UINT code, INT pos )
{
	SCROLLINFO	stScrollInfo;
	INT	dDot = gdHideXdot;

	//	状態をくやしく
	ZeroMemory( &stScrollInfo, sizeof(SCROLLINFO) );
	stScrollInfo.cbSize = sizeof(SCROLLINFO);
	stScrollInfo.fMask = SIF_ALL;
	GetScrollInfo( hWnd, SB_HORZ, &stScrollInfo );

	switch( code )	//	スクロール方向に合わせて内容をずらす
	{
		case SB_LINEUP:	//	△押した
			dDot--;
			break;

		case SB_PAGEUP: //	バー押した
			dDot -= gstViewArea.cx / 5;
			break;

		case SB_THUMBTRACK:	//	ツマミで移動
			dDot = stScrollInfo.nTrackPos;
			break;

		case SB_PAGEDOWN:
			dDot += gstViewArea.cx / 5;
			break;

		case SB_LINEDOWN:
			dDot++;
			break;

		default:	return;
	}

	if( 0 > dDot )	dDot = 0;
	if( stScrollInfo.nMax < dDot )	dDot = stScrollInfo.nMax;

	gdHideXdot = dDot;

	stScrollInfo.fMask = SIF_POS;
	stScrollInfo.nPos  = dDot;
	SetScrollInfo( ghViewWnd, SB_HORZ, &stScrollInfo, TRUE );

	ViewRedrawSetLine( -1 );

#if 0
	//	キャレットを追従・しないほうがいい？
	if( gdHideXdot >  gdDocXdot )	gdDocXdot = gdHideXdot + 5;	//	適当な値
	if( (gdHideXdot+gstViewArea.cx-EOF_WIDTH) <= gdDocXdot )	gdDocXdot = (gstViewArea.cx - EOF_WIDTH);
	DocLetterPosGetAdjust( &gdDocXdot, gdDocLine, 0, 0 );
#endif

	ViewDrawCaret( gdDocXdot, gdDocLine, 0 );	//	位置を決める

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	編集ビューの縦スクロールバーが操作された
	@param[in]	hWnd	ウインドウハンドル・ビューのとは限らないので注意セヨ
	@param[in]	hWndCtl	スクロールバーのハンドル・コントロールにくっついているので０である
	@param[in]	code	スクロールコード
	@param[in]	pos		スクロールボックス（つまみ）の位置
	@return		無し
*/
VOID Evw_OnVScroll( HWND hWnd, HWND hWndCtl, UINT code, INT pos )
{
	SCROLLINFO	stScrollInfo;
	INT	dPos = gdViewTopLine, iLines, dPrev;

	//	posを、ホイールフラグにする

	//	総行数より、表示領域のほうが大きかったら処理しない
	iLines = DocNowFilePageLineCount(  );//DocPageParamGet( NULL, NULL );	//	行数確保
	if( gdDispingLine >= iLines )	return;

	//	状態をくやしく
	ZeroMemory( &stScrollInfo, sizeof(SCROLLINFO) );
	stScrollInfo.cbSize = sizeof(SCROLLINFO);
	stScrollInfo.fMask = SIF_ALL;
	GetScrollInfo( hWnd, SB_VERT, &stScrollInfo );

	TRACE( TEXT("code[%d] pos[%d] dPos[%d] InfoMax[%d]"), code, pos, dPos, stScrollInfo.nMax );

	dPrev = dPos;

	switch( code )	//	スクロール方向に合わせて内容をずらす
	{
		case SB_LINEUP:	//	△押した
			if( pos ){	dPos = dPos - gdWheelLine;	}
			else{	 dPos--;	}
			break;

		case SB_PAGEUP: //	バー押した
			dPos -= gdDispingLine / 2;
			break;

		case SB_THUMBTRACK:	//	ツマミで移動
			dPos = stScrollInfo.nTrackPos;
			break;

		case SB_PAGEDOWN:
			dPos += gdDispingLine / 2;
			break;

		case SB_LINEDOWN:
			if( pos ){	dPos = dPos + gdWheelLine;	}
			else{	 dPos++;	}
			break;

		default:	return;
	}

	if( 0 > dPos )	dPos = 0;
	if( stScrollInfo.nMax < dPos )	dPos = stScrollInfo.nMax;

	gdViewTopLine = dPos;

	stScrollInfo.fMask = SIF_POS;
	stScrollInfo.nPos  = dPos;
	SetScrollInfo( ghViewWnd, SB_VERT, &stScrollInfo, TRUE );

	//	スクロールしてなかったら更新しない
	if( dPrev != dPos ){	ViewRedrawSetLine( -1 );	}


#if 0
	//	キャレットを追従・しないほうがいい？
	if( gdViewTopLine >  gdDocLine )	gdViewTopLine = ++gdDocLine;
	if( (gdViewTopLine+gdDispingLine) <= gdDocLine )	gdDocLine--;
	DocLetterPosGetAdjust( &gdDocXdot, gdDocLine, 0, 0 );
#endif

	ViewDrawCaret( gdDocXdot, gdDocLine, 0 );	//	位置を決める

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	コンテキストメニュー呼びだしアクション(要は右クルック）
	@param[in]	hWnd		ウインドウハンドル・ビューのとは限らないので注意セヨ
	@param[in]	hWndContext	コンテキストが発生したウインドウのハンドル
	@param[in]	xPos		スクリーンＸ座標
	@param[in]	yPos		スクリーンＹ座業
	@return		無し
*/
VOID Evw_OnContextMenu( HWND hWnd, HWND hWndContext, UINT xPos, UINT yPos )
{
	INT	posX, posY;

	HMENU	hSubMenu;
	UINT	dRslt;

	posX = (SHORT)xPos;	//	画面座標はマイナスもありうる
	posY = (SHORT)yPos;

	TRACE( TEXT("VIEW_WM_CONTEXTMENU %d x %d"), posX, posY );


	hSubMenu = CntxMenuGet(  );

	CheckMenuItem( hSubMenu , IDM_SQSELECT,           gbSqSelect   ? MF_CHECKED : MF_UNCHECKED );
	CheckMenuItem( hSubMenu , IDM_SPACE_VIEW_TOGGLE,  gdSpaceView  ? MF_CHECKED : MF_UNCHECKED );
	CheckMenuItem( hSubMenu , IDM_GRID_VIEW_TOGGLE,   gbGridView   ? MF_CHECKED : MF_UNCHECKED );
	CheckMenuItem( hSubMenu , IDM_RIGHT_RULER_TOGGLE, gbRitRlrView ? MF_CHECKED : MF_UNCHECKED );

//	FrameNameModifyPopUp( hSubMenu, 1 );	//	枠の名前を挿入

	dRslt = TrackPopupMenu( hSubMenu, TPM_RETURNCMD, posX, posY, 0, hWnd, NULL );	//	TPM_CENTERALIGN | TPM_VCENTERALIGN | 
	//	選択せずで０か－１？、選択したらそのメニューのＩＤが戻るようにセット

	//	それぞれの処理に飛ばす
	FORWARD_WM_COMMAND( ghViewWnd, dRslt, hWndContext, 0, PostMessage );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	キーボードフォーカスを編集ビューへ
	@return		HRESULT	終了状態コード
*/
HRESULT ViewFocusSet( VOID )
{
//	ViewShowCaret(  );

	SetFocus( ghViewWnd );

//	SetForegroundWindow( ghPrntWnd );
	SetWindowPos( ghPrntWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE );

	TRACE( TEXT("キーボードフォーカスセット") );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	今のドット値と行番号をステータスバーに送信する
	@return		HRESULT	終了状態コード
*/
HRESULT ViewNowPosStatus( VOID )
{
	static INT	cdPreDot;	//	直前のドット位置かな
	TCHAR	atString[SUB_STRING];

	StringCchPrintf( atString, SUB_STRING, TEXT("%d[dot] %d[char] %d[line]"), gdDocXdot, gdDocMozi, gdDocLine + 1 );

	MainStatusBarSetText( SB_CURSOR, atString );

	//	ルーラの、直前のドット位置と今のドット位置のあたりで、再描画発生させる

	cdPreDot = gdDocXdot;

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定文字の幅を首都苦
	@param[in]	ch	幅を計りたい文字
	@return		幅ドット数
*/
INT ViewLetterWidthGet( TCHAR ch )
{
	SIZE	stSize;
	HDC		hdc= GetDC( ghViewWnd );
	HFONT	hFtOld;

	hFtOld = SelectFont( hdc, ghAaFont );

	GetTextExtentPoint32( hdc, &ch, 1, &stSize );

	SelectFont( hdc, hFtOld );

	ReleaseDC( ghViewWnd, hdc );

	return stSize.cx;
}
//-------------------------------------------------------------------------------------------------

/*!
	文字列のドット幅を数える
	@param[in]	ptStr	数えたい文字列
	@return		幅ドット数・０ならエラー
*/
INT ViewStringWidthGet( LPCTSTR ptStr )
{
	SIZE	stSize;
	UINT	cchSize;
	HDC		hdc= GetDC( ghViewWnd );
	HFONT	hFtOld;

	StringCchLength( ptStr, STRSAFE_MAX_CCH, &cchSize );

	if( 0 >= cchSize )	return 0;	//	異常事態

	hFtOld = SelectFont( hdc, ghAaFont );

	GetTextExtentPoint32( hdc, ptStr, cchSize, &stSize );

	SelectFont( hdc, hFtOld );

	ReleaseDC( ghViewWnd, hdc );

	return stSize.cx;
}
//-------------------------------------------------------------------------------------------------

/*!
	ビューの特定の領域を再描画対象領域にする
	@param[in]	pstRect	対象の文書位置の矩形を入れた構造体えのピンター
	@return		HRESULT	終了状態コード
*/
HRESULT ViewRedrawSetRect( LPRECT pstRect )
{
	RECT	rect;

	if( !(pstRect) )	return E_INVALIDARG;

	rect = *pstRect;
	rect.right++;
	rect.bottom++;	//	広げておく

	ViewPositionTransform( (PINT)&(rect.left),  (PINT)&(rect.top),    1 );
	ViewPositionTransform( (PINT)&(rect.right), (PINT)&(rect.bottom), 1 );

	InvalidateRect( ghViewWnd, &rect, TRUE );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定行の行番号再描画
	@param[in]	rdLine	対象の行番号・絶対０インデックス・マイナスなら画面全体再描画
	@return		HRESULT	終了状態コード
*/
HRESULT ViewRedrawSetVartRuler( INT rdLine )
{
	RECT	rect;
	INT	dDummy = 0;

	//	表示範囲外ならナニもする必要は無い
	if( gdViewTopLine > rdLine )	return S_FALSE;
	if( (gdViewTopLine + gdDispingLine + 1) < rdLine )	return S_FALSE;

	rect.top    = rdLine * LINE_HEIGHT;
	ViewPositionTransform( &dDummy, (PINT)&(rect.top), 1 );

	rect.bottom = rect.top + LINE_HEIGHT;
	rect.left   = 0;
	rect.right  = LINENUM_WID + 2;	//	ちゅっと余裕

	InvalidateRect( ghViewWnd, &rect, TRUE );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ビューの再描画領域を設定して描画指示
	@param[in]	rdLine	対象の行番号・絶対０インデックス・マイナスなら画面全体再描画
	@return		HRESULT	終了状態コード
*/
HRESULT ViewRedrawSetLine( INT rdLine )
{
	RECT	rect, clRect;
	INT	dDummy;

//InvalidateRectは、対象領域に対してWM_PAINTを発行する。通常だと
//ウインドウプロシージャに処理が廻って、WM_PAINTが処理されるが、
//UpdateWindowは、その場で即描画処理がはいる。

	ViewScrollBarAdjust( NULL );

	if( 0 > rdLine )
	{
		InvalidateRect( ghViewWnd, NULL, TRUE );
		return S_OK;
	}

	//	表示範囲外ならナニもする必要は無い
	if( gdViewTopLine > rdLine )	return S_FALSE;
	if( (gdViewTopLine + gdDispingLine + 1) < rdLine )	return S_FALSE;

	GetClientRect( ghViewWnd, &clRect );

	SetRect( &rect, 0, rdLine * LINE_HEIGHT, clRect.right, (rdLine+1) * LINE_HEIGHT );

	dDummy = 0;
	ViewPositionTransform( &dDummy, (PINT)&(rect.top), 1 );
	ViewPositionTransform( &dDummy, (PINT)&(rect.bottom), 1 );

	InvalidateRect( ghViewWnd, &rect, TRUE );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	編集ビューの表示を描く
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	hdc		描くデバイスコンテキスト
	@return		HRESULT	終了状態コード
*/
HRESULT ViewRedrawDo( HWND hWnd, HDC hdc )
{
	LPLETTER	pstTexts = NULL;
	INT		cchLen = 0, dot, iLines, i, vwLines;
	UINT	dFlag = 0;
	HFONT	hFtOld;

	UINT	bTrace = FALSE;	//	トレス中であるか

	//	画面に入ってない場合は？

	hFtOld = SelectFont( hdc, ghAaFont );	//	フォントをくっつける

	iLines = DocPageParamGet( NULL, NULL );	//	行数確保・頁情報再描画

	//	必要ないところの処理まではしなくていい
	vwLines = gdDispingLine + 2 + gdViewTopLine;	//	余裕持たせて

	//	トレスイメージ
	bTrace = TraceImageAppear( hdc, gdHideXdot, gdViewTopLine * LINE_HEIGHT );	//	左上位置を考慮セヨ
	if( bTrace )	SetBkMode( hdc, TRANSPARENT );

	//	トレス画像より上に来るように
	ViewDrawMetricLine( hdc, 0 );	//	ライン系

	for( i = 0; iLines > i; i++ )	//	文字列描画
	{
		//	必要な所から処理して
		if( gdViewTopLine > i ){	continue;	}
		if( vwLines <= i )	break;
		//	完全に画面外になったら終わっておｋ

		dot = DocLineDataGetAlloc( i, 0, &(pstTexts), &cchLen, &dFlag );
		if( 0 < cchLen )	//	cchLenにはヌルターミネータが入ってない
		{
			//	この中で位置や色を調整してきゅきゅっと
			ViewDrawTextOut( hdc, 0, i, pstTexts, cchLen );
		}
		FREE( pstTexts );

		if( dFlag & CT_RETURN )	//	改行描画
		{
			ViewDrawReturnMark( hdc, dot, i, dFlag );
		}

		if( dFlag & CT_EOF )	//	EOF描画
		{
			ViewDrawEOFMark( hdc, dot, i, dFlag );
		}
	}

	SelectFont( hdc, hFtOld );	//	フォントを外す

	ViewDrawRuler( hdc );	//	上ルーラー
	ViewDrawLineNumber( hdc );	//	左の行番号

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	文字列を、スタイル指定に従って描画していく
	@param[in]	hdc			デバイスコンテキスト
	@param[in]	dDot		描画開始するドット値
	@param[in]	rdLine		描画する行
	@param[in]	pstTexts	文字とスタイル情報
	@param[in]	cchLen		文字数
	@return		BOOLEAN		描画ＯＫかどうか
*/
BOOLEAN ViewDrawTextOut( HDC hdc, INT dDot, UINT rdLine, LPLETTER pstTexts, UINT_PTR cchLen )
{
	UINT_PTR	mz, cchMr;
	COLORREF	clrTextOld, clrBackOld, clrTrcMozi, clrMozi, clrRvsMozi;
	INT		dX, dY;	//	描画位置左上
	INT		width, rdStart;
	LPTSTR	ptText;
	UINT	bStyle, cbSize;
	BOOLEAN	bRslt, doDraw;
	RECT	rect;

	dX = dDot;
	dY = rdLine * LINE_HEIGHT;


	cbSize = (cchLen + 1) * sizeof(TCHAR);
	ptText = (LPTSTR)malloc( cbSize );
	if( !(ptText) ){	TRACE( TEXT("malloc error") );	return FALSE;	}
	ZeroMemory( ptText, cbSize );

	//	最初に基本モードセットして、標準設定を確保しておく
	if( TraceMoziColourGet( &clrTrcMozi ) ){	clrMozi = clrTrcMozi;	}
	else{					clrMozi =  gaColourTable[CLRT_BASICPEN];	}
	clrTextOld = SetTextColor( hdc, clrMozi );

	clrRvsMozi = ~clrMozi;	//	選択状態用に色を反転
	clrRvsMozi &= 0x00FFFFFF;

	clrBackOld = SetBkColor(   hdc, gaColourTable[CLRT_BASICBK] );

	bStyle  = pstTexts[0].mzStyle;
	cchMr   = 0;
	width   = 0;
	rdStart = 0;
	doDraw  = FALSE;

	ViewPositionTransform( &rdStart, &dY, 1 );

	SetBkMode( hdc, TRANSPARENT );

	for( mz = 0; cchLen >= mz; mz++ )
	{
		//	同じスタイルが続くなら
		if( bStyle == pstTexts[mz].mzStyle )
		{
			ptText[cchMr++] = pstTexts[mz].cchMozi;	//	壱繋がりの文字列として確保
			width += pstTexts[mz].rdWidth;
		}
		else{	doDraw = TRUE;	}

		//	末端まできちゃったら
		if( cchLen ==  mz ){	doDraw = TRUE;	}

		if( doDraw )	//	描画タイミングであるなら
		{
			//	スペースなら下線を描画する・ここで分ける
			if( bStyle & CT_SPACE )
			{
				ViewDrawSpace( hdc, rdStart, dY, ptText, cchMr, bStyle );
				//この中にも背景色の塗りとかある。整合性に注意セヨ
			}
			else
			{
				if( bStyle & CT_SELECT )	//	選択の場合背景色と枠塗り潰し
				{
					SetTextColor( hdc, clrRvsMozi );
					SetBkColor(   hdc, gaColourTable[CLRT_SELECTBK] );

					SetRect( &rect, rdStart, dY, rdStart + width, dY + LINE_HEIGHT );
					FillRect( hdc, &rect, gahBrush[BRHT_SELECTBK] );
				}
				else if( bStyle & CT_FINDED )	//	検索ヒット文字列の場合
				{
					SetTextColor( hdc, clrMozi );
					SetBkColor(   hdc, gaColourTable[CLRT_FINDBACK] );

					SetRect( &rect, rdStart, dY, rdStart + width, dY + LINE_HEIGHT );
					FillRect( hdc, &rect, gahBrush[BRHT_FINDBACK] );
				}
				else if( bStyle & CT_CANTSJIS )	//	SJIS不可（ユニコード文字）の場合
				{
					SetTextColor( hdc, clrMozi );
					SetBkColor(   hdc, gaColourTable[CLRT_CANTSJIS] );

					SetRect( &rect, rdStart, dY, rdStart + width, dY + LINE_HEIGHT );
					FillRect( hdc, &rect, gahBrush[BRHT_CANTSJISBK] );
				}
				else
				{
					SetTextColor( hdc, clrMozi );
					SetBkColor(   hdc, gaColourTable[CLRT_BASICBK] );
				}

				bRslt = ExtTextOut( hdc, rdStart, dY, 0, NULL, ptText, cchMr, NULL );
				if( !(bRslt)  ){	TRACE( TEXT("ExtTextOut error") );	return FALSE;	}
			}

			rdStart += width;
			//	描画したら、今の文字を新しいスタイルとして登録してループ再開
			bStyle = pstTexts[mz].mzStyle;
			ZeroMemory( ptText, cbSize );
			ptText[0] = pstTexts[mz].cchMozi;
			width  = pstTexts[mz].rdWidth;
			cchMr  = 1;

			doDraw = FALSE;
		}
	}

	FREE( ptText );	//	確保した領域は開放しないと死ぬ

	//	元に戻しておくと良いことがある
	SetTextColor( hdc, clrTextOld );
	SetBkColor(   hdc, clrBackOld );

	return TRUE;
}
//-------------------------------------------------------------------------------------------------

/*!
	スペースを、灰色下線で描画する
	@param[in]	hdc		デバイスコンテキスト
	@param[in]	dX		描画開始する横ドット位置
	@param[in]	dY		描画開始する縦ドット位置
	@param[in]	ptText	スペースの羅列、半角全角
	@param[in]	cchLen	文字数
	@param[in]	bStyle	警告付き・選択中
	@return		正否
*/
BOOLEAN ViewDrawSpace( HDC hdc, INT dX, UINT dY, LPTSTR ptText, UINT_PTR cchLen, UINT bStyle )
{
	HPEN	hPenOld;	//	描画用にペンを用意
	INT		width, xx, yy;
	UINT	cchPos;
	SIZE	stSize;
	RECT	stRect;

//	SetBkColor(   hdc, gaColourTable[CLRT_SPACELINE] );	//	背景色は不要
	//	描画位置はいろいろ調整済み

	xx = dX;
	yy = dY;

	dY += (LINE_HEIGHT - 2);	//	下線なので↓のほう

	if( bStyle & CT_WARNING )	//	ペンくっつける・警告と通常
	{
		hPenOld = SelectPen( hdc , gahPen[PENT_SPACEWARN] );
	}
	else
	{
		hPenOld = SelectPen( hdc , gahPen[PENT_SPACELINE] );
	}

	GetTextExtentPoint32( hdc, ptText, cchLen, &stSize );
	if( bStyle & CT_SELECT )	//	選択状態なら
	{
		SetRect( &stRect, xx, yy, xx + stSize.cx, yy + stSize.cy );
		FillRect( hdc, &stRect, gahBrush[BRHT_SELECTBK] );
	}
	else if( bStyle & CT_FINDED )	//	検索ヒット文字列の場合
	{
		SetRect( &stRect, xx, yy, xx + stSize.cx, yy + stSize.cy );
		FillRect( hdc, &stRect, gahBrush[BRHT_FINDBACK] );
	}
	else if( bStyle & CT_CANTSJIS )	//	SJIS不可（ユニコード文字）の場合
	{
		SetRect( &stRect, xx, yy, xx + stSize.cx, yy + stSize.cy );
		if( gdSpaceView )	FillRect( hdc, &stRect, gahBrush[BRHT_CANTSJISBK] );
		//	有効なら塗る
	}

	if( gdSpaceView || (bStyle & CT_WARNING) )	//	有効なら描画
	{
		for( cchPos = 0; cchLen > cchPos; cchPos++ )
		{
			if( TEXT(' ') == ptText[cchPos] )	//	半角
			{
				width = SPACE_HAN;
			}
			else if( TEXT('　') == ptText[cchPos] )	//	全角
			{
				width = SPACE_ZEN;
			}
			else	//	ユニコード空白の場合
			{
				width = ViewLetterWidthGet( ptText[cchPos] );
			}

			MoveToEx( hdc, dX, dY, NULL );	//	開始地点
			LineTo(   hdc, (dX + width - 1), dY  );	//	描画幅１ドット余裕持たせる

			dX += width;
		}
	}

	SelectPen( hdc, hPenOld );	//	元に戻しておく

	return TRUE;
}
//-------------------------------------------------------------------------------------------------

/*!
	800ドットの線やグリッドLineをかく
	@param[in]	hdc		デバイスコンテキスト
	@param[in]	bUpper	０文字の描画前　非０文字の描画後
	@return		HRESULT	終了状態コード
*/
HRESULT ViewDrawMetricLine( HDC hdc, UINT bUpper )
{
	HPEN	hPenOld;	//	描画用にペンを用意
	INT		dX, dY;	//	描画枠左上
	INT		aX, aY;	//	ペンの位置
	LONG	width, height;


	width = gstViewArea.cx + LINENUM_WID;	//	LineToは座標なので、最後まで指定する
	height = gstViewArea.cy + RULER_AREA;	//	LineToは座標なので、最後まで指定する

	//	下書き、上書き、ON/OFF自在になるように
	if( gbGridView )
	{

		//	グリッドライン・位置は設定から引っ張るように
		hPenOld = SelectPen( hdc , gahPen[PENT_GRID_LINE] );	//	あらかじめ確保っとく

		aX = gdGridXpos;
		aY = gdGridYpos;
		ViewPositionTransform( &aX, &aY, 1 );

		while( height > aY )	//	横線
		{
			MoveToEx( hdc , LINENUM_WID, aY, NULL );	//	開始地点
			LineTo(   hdc , width, aY );	//	境界線びゅー
			aY += gdGridYpos;
		}

		while( width  > aX )	//	縦線
		{
			MoveToEx( hdc, aX, RULER_AREA-1, NULL );	//	開始地点
			LineTo(   hdc, aX, height );	//	境界線びゅー
			aX += gdGridXpos;
		}

		SelectPen( hdc, hPenOld );	//	元に戻しておく
	}

	if( gbRitRlrView )
	{
		//	８００の線・グリッドより手前に書くようにする
		hPenOld = SelectPen( hdc , gahPen[PENT_SPACEWARN] );	//	あらかじめ確保っとく

		dX = gdRightRuler;	//	設定から引っ張る
		dY = 0;

		ViewPositionTransform( &dX, &dY, 1 );

		MoveToEx( hdc, dX, RULER_AREA-1, NULL  );	//	開始地点
		LineTo(   hdc, dX, height  );	//	境界線びゅー

		SelectPen( hdc, hPenOld );	//	元に戻しておく
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	改行マークを描く・下向き矢印でいいかな
	@param[in]	hdc		デバイスコンテキスト
	@param[in]	dDot	描画開始するドット値
	@param[in]	rdLine	描画する行
	@param[in]	dFlag	描画必要なフラグ
	@return		HRESULT	終了状態コード
*/
HRESULT ViewDrawReturnMark( HDC hdc, INT dDot, INT rdLine, UINT dFlag )
{
	HPEN	hPenOld;	//	描画用にペンを用意
	INT		dX, dY;	//	描画枠左上
	INT		aX, aY;	//	ペンの位置
	COLORREF	clrBackOld = 0;
	RECT	rect;

	dX = dDot;
	dY = rdLine * LINE_HEIGHT;

	ViewPositionTransform( &dX, &dY, 1 );

	//	画面に入ってない場合を考慮

	SetRect( &rect, dX, dY, dX + SPACE_ZEN, dY + LINE_HEIGHT );

	//	背景の色
	if( dFlag & CT_SELRTN )
	{
		clrBackOld = SetBkColor( hdc , gaColourTable[CLRT_SELECTBK] );
		FillRect( hdc, &rect, gahBrush[BRHT_SELECTBK] );
	}
	else if( dFlag & CT_FINDRTN )
	{
		clrBackOld = SetBkColor( hdc , gaColourTable[CLRT_FINDBACK] );
		FillRect( hdc, &rect, gahBrush[BRHT_FINDBACK] );
	}
	else if( dFlag & CT_LASTSP )
	{
		clrBackOld = SetBkColor( hdc , gaColourTable[CLRT_LASTSPWARN] );
		FillRect( hdc, &rect, gahBrush[BRHT_LASTSPWARN] );
	}
	else
	{
		clrBackOld = SetBkColor( hdc , gaColourTable[CLRT_BASICBK] );
		FillRect( hdc, &rect, gahBrush[BRHT_BASICBK] );
	}

//	ExtTextOut( hdc , dX, dY, 0, NULL, TEXT("　"), 1, NULL );	//	場所作って
//	20111216	いらない？

	SetBkColor( hdc, clrBackOld );

	hPenOld = SelectPen( hdc , gahPen[PENT_CRLF_MARK] );	//	ペンくっつけて

	aX = dX + 3;
	aY = dY + 3;	//	上マージン
	MoveToEx( hdc, aX, aY, NULL );	//	開始地点
	LineTo(   hdc, aX, aY + 12  );	//	上から下へ
	LineTo(   hdc, dX, aY + 9  );	//	そこから左上へ
	MoveToEx( hdc, aX, aY + 12, NULL );	//	矢印の先っぽへ
	LineTo(   hdc, aX + 3, aY + 9 );	//	そして右上へ
	//	もうちゅっとスマートにできないかこれ

	SelectPen( hdc, hPenOld );	//	元に戻しておく

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	EOF記号を描く
	@param[in]	hdc		デバイスコンテキスト
	@param[in]	dDot	描画開始するドット値
	@param[in]	rdLine	描画する行
	@param[in]	dFlag	文字列終端の情報
	@return		使ったドット数
*/
INT ViewDrawEOFMark( HDC hdc, INT dDot, INT rdLine, UINT dFlag )
{

	INT			dX, dY;	//	描画枠左上
	COLORREF	clrTextOld, clrBackOld = 0;
	RECT		stClip;
	SIZE		stSize;

	dX = dDot;
	dY = rdLine * LINE_HEIGHT;

	ViewPositionTransform( &dX, &dY, 1 );

	clrTextOld = SetTextColor( hdc , gaColourTable[CLRT_EOF_MARK] );	//	EOFの色
	if( dFlag & CT_LASTSP )
	{
		clrBackOld = SetBkColor(   hdc , gaColourTable[CLRT_LASTSPWARN] );	//	背景の色
		SetBkMode( hdc, OPAQUE );
	}


	GetTextExtentPoint32( hdc, gatEOF, EOF_SIZE, &stSize );

	//	画面の左端にめり込んでる場合を考慮

	//	表示場所確認
	stClip.left   = dX + 1;
	stClip.right  = dX + 1 + stSize.cx;
	stClip.top    = dY + 1;
	stClip.bottom = dY + LINE_HEIGHT;

	ExtTextOut( hdc, stClip.left, stClip.top, 0, &stClip, gatEOF, EOF_SIZE, NULL );

	SetTextColor( hdc, clrTextOld );
	if( dFlag & CT_LASTSP ){	SetBkColor( hdc, clrBackOld );	SetBkMode( hdc, TRANSPARENT );	}


	return stSize.cx;
}
//-------------------------------------------------------------------------------------------------

/*!
	ビュー領域の書換を発生させる
	@param[in]	iBgn	更新範囲Ｘ開始
	@param[in]	iEnd	更新範囲Ｘ終了
	@return		HRESULT	終了状態コード
*/
HRESULT ViewRulerRedraw( INT iBgn, INT iEnd )
{
	RECT	rect;

	GetClientRect( ghViewWnd, &rect );
	rect.bottom = RULER_AREA;

	if( 0 <= iBgn ){	rect.left  = iBgn;	}
	if( 0 <= iEnd ){	rect.right = iEnd;	}

	InvalidateRect( ghViewWnd, &rect, TRUE );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	枠上部のRulerを描く
	@param[in]	hdc		デバイスコンテキスト
	@return		HRESULT	終了状態コード
*/
HRESULT ViewDrawRuler( HDC hdc )
{
	HPEN	hPenOld;	//	描画用にペンを用意
	HFONT	hFtOld;		//	ルーラー用
	LONG	width, pos, ln, start, dif, sbn, hei;
	TCHAR	atStr[10];
	UINT_PTR	count;
	RECT		rect;

	hPenOld = SelectPen( hdc, gahPen[PENT_RULER] );	//	あらかじめ確保っとく

	width = gstViewArea.cx + LINENUM_WID;	//	LineToは座標なので、最後まで指定する

	SetBkMode( hdc, TRANSPARENT );

	SetRect( &rect, 0, 0, width, RULER_AREA );
	FillRect( hdc, &rect, gahBrush[BRHT_RULERBK] );

	MoveToEx( hdc, LINENUM_WID, RULER_AREA-1, NULL );	//	開始地点
	LineTo(   hdc, width, RULER_AREA-1 );	//	境界線びゅー

	start = gdHideXdot;	//	ここにスクロール量を考慮すればいい

	dif = start % 10;
	sbn = start / 10;
	if( dif ){	sbn++;	dif =  10 - dif;	}	//	ずれ量の値計算注意
	//	縦線
	for( pos = 0, ln = sbn; width > pos; pos+=10, ln++ )
	{
		hei = 6;
		if( !( ln % 5 ) )	hei = 3;
		if( !( ln % 10 ) )	hei = 0;
		MoveToEx( hdc, LINENUM_WID+pos+dif, hei, NULL );	//	開始地点
		LineTo(   hdc, LINENUM_WID+pos+dif, RULER_AREA-1 );	//	境界線びゅー
	}

	SelectPen( hdc, hPenOld );	//	元に戻しておく

	hFtOld = SelectFont( hdc, ghRulerFont );	//	フォントをくっつける

	//	数値
	dif = start % 100;	if( dif )	dif = 100 - dif;
	sbn = start / 100;	if( dif )	sbn++;
	sbn *= 100;
	for( pos = 0, ln = sbn; width > pos; pos+=100, ln+=100 )
	{
		StringCchPrintf( atStr, 10, TEXT("%d"), ln );
		StringCchLength( atStr, 10, &count );
		ExtTextOut( hdc, LINENUM_WID+pos+2+dif, 0, 0, NULL, atStr, count, NULL );
	}

	SelectFont( hdc, hFtOld );


	//	キャレット位置・再描画を？
	hPenOld = SelectPen( hdc, gahPen[PENT_CARET_POS] );	//	色

	MoveToEx( hdc, LINENUM_WID + gdDocXdot, 1, NULL );	//	開始地点
	LineTo(   hdc, LINENUM_WID + gdDocXdot, RULER_AREA-1 );	//	どぴゅっと引く

	SelectPen( hdc, hPenOld );	//	元に戻しておく


	if( 1 <= gdAutoDiffBase )	//	自動調整の基準
	{
		//	色、とりあえず空白警告で
		hPenOld = SelectPen( hdc, gahPen[PENT_SPACEWARN] );

		MoveToEx( hdc, LINENUM_WID + gdAutoDiffBase, 1, NULL );	//	開始地点
		LineTo(   hdc, LINENUM_WID + gdAutoDiffBase, RULER_AREA-1 );	//	どぴゅっと引く

		SelectPen( hdc, hPenOld );	//	元に戻しておく
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	行番号を描く
	@param[in]	hdc		デバイスコンテキスト
	@return		HRESULT	終了状態コード
*/
HRESULT ViewDrawLineNumber( HDC hdc )
{
	HPEN		hPenOld;	//	描画用にペンを用意
	HFONT		hFtOld;		//	ルーラー用
	LONG		height, num, hei;
	TCHAR		atStr[10];
	UINT_PTR	count;
	UINT		figure = 4;
	RECT		rect;

	hPenOld = SelectPen( hdc , gahPen[PENT_RULER] );	//	あらかじめ確保っとく

	height = gstViewArea.cy + RULER_AREA;	//	LineToは座標なので、最後まで指定する

	SetBkMode( hdc, TRANSPARENT );

	SetRect( &rect, 0, 0, LINENUM_WID-1, height );
	FillRect( hdc, &rect, gahBrush[BRHT_RULERBK] );

	//	線と描画Areaの間に１ドット余裕させるので－２
	MoveToEx( hdc, LINENUM_WID-2, RULER_AREA-1, NULL  );	//	開始地点
	LineTo(   hdc, LINENUM_WID-2, height  );	//	境界線びゅー

	SelectPen( hdc, hPenOld );	//	元に戻しておく

	num = gdViewTopLine;	//	開始数値
	if( 9999 > num )	//	num：０インデックス
	{
		figure =  1;
		hFtOld = SelectFont( hdc, ghNumFont4L );
	}
	else if( 9999 <= num && num < 99999 )
	{
		figure =  3;
		hFtOld = SelectFont( hdc, ghNumFont5L );
	}
	else
	{
		figure =  5;
		hFtOld = SelectFont( hdc, ghNumFont6L );
	}
	//	フォントをくっつける

	for( hei = 0; height > hei; hei+=LINE_HEIGHT, num++ )
	{
		if( 1 == figure && 9999 <= num )	//	４→５の切り替わり
		{	figure =  3;	SelectFont( hdc , ghNumFont5L );	}

		if( 3 == figure && 99999 <= num )	//	５→６の切り替わり
		{	figure =  5;	SelectFont( hdc , ghNumFont6L );	}

		if( DocBadSpaceIsExist( num )  )	//	空白警告あるなら赤くするとか
		{
			SetRect( &rect, 0, hei+RULER_AREA, LINENUM_WID-2, hei+RULER_AREA+LINE_HEIGHT );
			FillRect( hdc, &rect, gahBrush[BRHT_LASTSPWARN] );
		}

		switch( figure )
		{
			default:
			case  1:	StringCchPrintf( atStr, 10 , TEXT("%4d"), num + 1 );	break;
			case  3:	StringCchPrintf( atStr, 10 , TEXT("%5d"), num + 1 );	break;
			case  5:	StringCchPrintf( atStr, 10 , TEXT("%6d"), num + 1 );	break;
		}
		StringCchLength( atStr, 10, &count );
		ExtTextOut( hdc, 0, hei + RULER_AREA + figure, 0, NULL, atStr, count, NULL );
		//	４まで１　５で３　６で５
	}

	SelectFont( hdc, hFtOld );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	枠を追加する処理の入口
	@param[in]	dMode	枠番号０インデックス
	@return		HRESULT	終了状態コード
*/
HRESULT ViewFrameInsert( INT dMode )
{
	return DocFrameInsert( dMode , gbSqSelect );
}
//-------------------------------------------------------------------------------------------------

/*!
	MAA一覧からの使用モードをセット
	@param[in]	dMode		左クルック用・０通常挿入　１割込挿入　２レイヤ　３ユニコピー　４SJISコピー　５ドラフトボードへ
	@param[in]	dSubMode	中クルック用・０通常挿入　１割込挿入　２レイヤ　３ユニコピー　４SJISコピー　５ドラフトボードへ
	@return		HRESULT	終了状態コード
*/
HRESULT ViewMaaItemsModeSet( UINT dMode, UINT dSubMode )
{
	gdUseMode = dMode;
	gdUseSubMode = dSubMode;

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	MAA一覧からの使用モードを確保
	@return	使用モード　０通常挿入　１割込挿入　２レイヤ　３ユニコピー　４SJISコピー　５ドラフトボードへ
*/
UINT ViewMaaItemsModeGet( PUINT pdSubMode )
{
	if( pdSubMode ){	*pdSubMode = gdUseSubMode;	}

	return gdUseMode;
}
//-------------------------------------------------------------------------------------------------

/*!
	MAAからSJISを受け取って処理する
	@param[in]	pcCont	AAの文字列
	@param[in]	cbSize	バイト数・末端NULLは含まない
	@param[in]	dMode	使用モード・デフォもしくは個別指定
	@return		非０デフォ動作した　０指定モードだった
*/
UINT ViewMaaMaterialise( LPSTR pcCont, UINT cbSize, UINT dMode )
{
	LPTSTR		ptString;
	UINT_PTR	cchSize;
	UINT		uRslt = TRUE;	//	デフォ動作であるならTRUE・仕様変更により常にTRUE
	INT			xDot;

	//	デフォ動作であるかどうか
//	if( dMode == gdUseMode ){	uRslt = TRUE;	}
	if( MAA_DEFAULT ==  dMode ){	dMode = gdUseMode;	}
	if( MAA_SUBDEFAULT == dMode ){	dMode = gdUseSubMode;	}

	//	先にSJIS固定のイベントから済ませる
	if( MAA_SJISCLIP == dMode )
	{
		DocClipboardDataSet( pcCont, (cbSize + 1), D_SJIS );
		return uRslt;
	}

	if( MAA_DRAUGHT == dMode )	//	ドラフトボードに追加
	{
		DraughtItemAdding( pcCont );
		return uRslt;
	}


	xDot = 0;

	//	続きはユニコード処理なので・内容を諭煮小汚怒に変換する
	ptString = SjisDecodeAlloc( pcCont );
	StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSize );

	switch( dMode )
	{
		case MAA_UNICLIP:	//	ユニコード的にクリッペ
			DocClipboardDataSet( ptString, (cchSize + 1) * sizeof(TCHAR), D_UNI );
			break;

		case MAA_LAYERED:	//	レイヤする
			LayerBoxVisibalise( ghInst, ptString, 0x00 );
			break;

		case MAA_INTERRUPT:	//	割込挿入
			DocInsertString( &gdDocXdot, &gdDocLine, NULL, ptString, D_SQUARE, TRUE );
			DocPageInfoRenew( -1, 1 );
			ViewPosResetCaret( 0, gdDocLine );
			//	ズレなく挿入したいなら、レイヤ使えばいい
			break;

		case MAA_INSERT:	//	通常挿入
			DocInsertString( &xDot, &gdDocLine, NULL, ptString, 0, TRUE );
			DocPageInfoRenew( -1, 1 );
			ViewPosResetCaret( xDot, gdDocLine );
			break;

		default:	break;
	}

	FREE(ptString);

	return uRslt;
}
//-------------------------------------------------------------------------------------------------

/*!
	何らかの操作モードについて、ステータスバーの表示を変更する
	それぞれのモードが入れ替わったら呼ぶ
*/
HRESULT OperationOnStatusBar( VOID )
{
	CONST  TCHAR	*catTexts[] = { { TEXT("[矩形]") }, { TEXT("[塗潰]") },
		{ TEXT("[抽出]") }, { TEXT("[空白]") } };

	TCHAR	atString[SUB_STRING];

	ZeroMemory( atString, sizeof(atString) );

	if( gbSqSelect ){	StringCchCat( atString, SUB_STRING, catTexts[0] );	}
	if( gbBrushMode ){	StringCchCat( atString, SUB_STRING, catTexts[1] );	}
	if( gbExtract ){	StringCchCat( atString, SUB_STRING, catTexts[2] );	}
	if( gdSpaceView ){	StringCchCat( atString, SUB_STRING, catTexts[3] );	}

	MainStatusBarSetText( SB_OP_STYLE, atString );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	アンドゥとリドゥの面倒見る
	@param[in]	id		処理の識別子
	@param[in]	pxDot	カーソルドット位置・処理したら戻す
	@param[in]	pyLine	カーソル行・処理したら戻す
	@return		なし
*/
VOID OperationUndoRedo( INT id, PINT pxDot, PINT pyLine )
{
	INT		dCrLf;

	DocPageSelStateToggle( -1 );	//	操作前には選択範囲解除すべき

	if( IDM_UNDO == id ){		dCrLf = DocUndoExecute( pxDot, pyLine );	}
	else if( IDM_REDO == id ){	dCrLf = DocRedoExecute( pxDot, pyLine );	}
	else{	 return;	}	//	無関係ならナニもしない

	if( dCrLf ){	ViewRedrawSetLine( -1 );	}
	else{		ViewRedrawSetLine( *pyLine );	}

	ViewDrawCaret( *pxDot, *pyLine, TRUE );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	編集ビューとメインビューのCOMMANDメッセージの受け取り
	@param[in]	hWnd		メインウインドウハンドルであること
	@param[in]	id			メッセージを発生させた子ウインドウの識別子	LOWORD(wParam)
	@param[in]	hWndCtl		メッセージを発生させた子ウインドウのハンドル	lParam
	@param[in]	codeNotify	通知メッセージ	HIWORD(wParam)
	@return		なし
*/
VOID OperationOnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify )
{
	UINT	bMode;

	//	ユーザ定義メニュー
	if( IDM_USERINS_ITEM_FIRST <= id && id <= IDM_USERINS_ITEM_LAST )
	{
		UserDefItemInsert( hWnd, (id - IDM_USERINS_ITEM_FIRST) );
		return;
	}

	//	ファイルオーポン履歴
	if( IDM_OPEN_HIS_FIRST <= id && id <= IDM_OPEN_HIS_LAST )	//	開く
	{
		OpenHistoryLoad( hWnd, id );
		return;
	}
	else if( IDM_OPEN_HIS_CLEAR ==  id )	//	ファイルオーポン履歴クルヤー
	{
		OpenHistoryLogging( hWnd, NULL );
		return;
	}

#ifdef PLUGIN_ENABLE
	//---------------------------------------------------------------------
	//[_16in] Add - 2012/05/01
	//
	//-- プラグインメニューの選択
	if( id >= IDM_PLUGIN_ITEM_BASE )
	{
		if( plugin::RunPlugin( gPluginList, id - IDM_PLUGIN_ITEM_BASE ) )
		{
			return;
		}
	}

	//---------------------------------------------------------------------
#endif

	switch( id )
	{
		default:					TRACE( TEXT("未実装") );	break;

		//	ファイル閉じる
		case IDM_FILE_CLOSE:		MultiFileTabClose(  );	break;

		//	ユニコードパレットオーポン
		case  IDM_UNI_PALETTE:		UniDialogueEntry( ghInst, hWnd );	break;

		//	プレビューオーポン
		case  IDM_ON_PREVIEW:		PreviewVisibalise( gixFocusPage, TRUE );	break;

		//	頁一覧を前面に
		case  IDM_PAGELIST_VIEW:	ShowWindow( ghPgVwWnd , SW_SHOW );		SetForegroundWindow( ghPgVwWnd );	break;

		//	壱行テンプレを前面に
		case  IDM_LINE_TEMPLATE:	ShowWindow( ghLnTmplWnd , SW_SHOW );	SetForegroundWindow( ghLnTmplWnd  );	break;

		//	ブラシテンプレを前面に
		case  IDM_BRUSH_PALETTE:	ShowWindow( ghBrTmplWnd , SW_SHOW );	SetForegroundWindow( ghBrTmplWnd  );	break;

		//	枠設定ダイヤログ
		case  IDM_INSFRAME_EDIT:	FrameEditDialogue( ghInst, hWnd, 0 );	break;

		//	一般設定ダイヤログ
		case  IDM_GENERAL_OPTION:	OptionDialogueOpen(   );	break;

		//	トレス機能窓開く
		case  IDM_TRACE_MODE_ON:	TraceDialogueOpen( ghInst, hWnd );	break;

		//	文字スクリプト窓開く
		case  IDM_MOZI_SCR_OPEN:	MoziScripterCreate( ghInst , hWnd );	break;

		//	縦書きスクリプト開く
		case IDM_VERT_SCRIPT_OPEN:	VertScripterCreate( ghInst , hWnd );	break;

		//	色編集ダイヤログ開く
		case IDM_COLOUR_EDIT_OPEN:	ViewColourEditDlg( hWnd );	break;

		//	ツールバーのドロップダウンメニューの呼出
		case IDM_IN_UNI_SPACE:
		case IDM_INSTAG_COLOUR:
		case IDM_USERINS_NA:		ToolBarPseudoDropDown( hWnd , id );	break;

		//	頁挿入窓オーポン
		case IDM_PAGENUM_DLG_OPEN:	DocPageNumInsert( ghInst, hWnd );	DocLetterPosGetAdjust( &gdDocXdot, gdDocLine, 0 );	break;

		//	ブラシ機能をON/OFFする
		case IDM_BRUSH_STYLE:		BrushModeToggle(  );	break;

		//	ウインドウのフォーカスを変更する
		case  IDM_WINDOW_CHANGE:	WindowFocusChange( WND_MAIN,  1 );	break;
		case  IDM_WINDOW_CHG_RVRS:	WindowFocusChange( WND_MAIN, -1 );	break;

		//	トレス中に、画像の表示・非表示する
		case IDM_TRC_VIEWTOGGLE:	TraceImgViewTglExt(   );	break;

		//	ファイル新規作成
		case IDM_NEWFILE:			DocOpenFromNull( hWnd );	break;

		//	ファイル開く
		case IDM_OPEN:				DocFileOpen( hWnd );	break;

		//	上書き保存
		case IDM_OVERWRITESAVE:		DocFileSave( hWnd, D_SJIS );	PreviewVisibalise( gixFocusPage, FALSE );	break;

		//	名前を付けて保存
		case IDM_RENAMESAVE:		DocFileSave( hWnd , (D_SJIS|D_RENAME) );	PreviewVisibalise( gixFocusPage, FALSE );	break;

		//	画像として保存
		case IDM_IMAGE_SAVE:		DocImageSave( hWnd, 0, ghAaFont );	break;

		//	枠挿入しちゃったりして
		case IDM_INSFRAME_ALPHA:	ViewFrameInsert( 0 );	break;
		case IDM_INSFRAME_BRAVO:	ViewFrameInsert( 1 );	break;
		case IDM_INSFRAME_CHARLIE:	ViewFrameInsert( 2 );	break;
		case IDM_INSFRAME_DELTA:	ViewFrameInsert( 3 );	break;
		case IDM_INSFRAME_ECHO:		ViewFrameInsert( 4 );	break;
		case IDM_INSFRAME_FOXTROT:	ViewFrameInsert( 5 );	break;
		case IDM_INSFRAME_GOLF:		ViewFrameInsert( 6 );	break;
		case IDM_INSFRAME_HOTEL:	ViewFrameInsert( 7 );	break;
		case IDM_INSFRAME_INDIA:	ViewFrameInsert( 8 );	break;
		case IDM_INSFRAME_JULIETTE:	ViewFrameInsert( 9 );	break;

		case IDM_INSFRAME_KILO:		ViewFrameInsert( 10 );	break;
		case IDM_INSFRAME_LIMA:		ViewFrameInsert( 11 );	break;
		case IDM_INSFRAME_MIKE:		ViewFrameInsert( 12 );	break;
		case IDM_INSFRAME_NOVEMBER:	ViewFrameInsert( 13 );	break;
		case IDM_INSFRAME_OSCAR:	ViewFrameInsert( 14 );	break;
		case IDM_INSFRAME_PAPA:		ViewFrameInsert( 15 );	break;
		case IDM_INSFRAME_QUEBEC:	ViewFrameInsert( 16 );	break;
		case IDM_INSFRAME_ROMEO:	ViewFrameInsert( 17 );	break;
		case IDM_INSFRAME_SIERRA:	ViewFrameInsert( 18 );	break;
		case IDM_INSFRAME_TANGO:	ViewFrameInsert( 19 );	break;

		//	枠挿入窓オーポン
		case IDM_FRMINSBOX_OPEN:	FrameInsBoxCreate( ghInst, hWnd );	break;

		//	コンテキストメニュー編集
		case IDM_MENUEDIT_DLG_OPEN:	CntxEditDlgOpen( hWnd );	break;

#ifdef ACCELERATOR_EDIT
		//	アクセラレートキー編集
		case IDM_ACCELKEY_EDIT_DLG_OPEN:	AccelKeyDlgOpen( hWnd );	break;
#endif

#ifdef FIND_STRINGS
		//	文字列検索
		case  IDM_FIND_DLG_OPEN:		FindDialogueOpen( ghInst, hWnd );	break;
		case IDM_FIND_HIGHLIGHT_OFF:	FindHighlightOff(  );	break;

		case IDM_FIND_JUMP_NEXT:	FindStringJump( 0 );	break;
		case IDM_FIND_JUMP_PREV:	FindStringJump( 1 );	break;

#endif

		//	アンドゥする
		case IDM_UNDO:	OperationUndoRedo( IDM_UNDO, &gdDocXdot, &gdDocLine );	break;

		//	リドゥする
		case IDM_REDO:	OperationUndoRedo( IDM_REDO, &gdDocXdot, &gdDocLine );	break;

		//	切り取り
		case IDM_CUT:
			DocExClipSelect( D_UNI | gbSqSelect );	//	コピーして削除すればおｋ
			if( IsSelecting( NULL ) ){	Evw_OnKey( hWnd, VK_DELETE, TRUE, 0, 0 );	}
			break;

		//	コピー
		case IDM_COPY:
			if( gbExtract )	//	SJISでも出来るように？
			{
				DocExtractExecute( NULL );
				gbExtract = FALSE;		//	取り出したらモード終了
				ViewSelPageAll( -1 );	//	選択範囲無くなる
				ViewRedrawSetLine( -1 );	//	画面表示更新
				MenuItemCheckOnOff( IDM_EXTRACTION_MODE, 0 );
				OperationOnStatusBar(  );
			}
			else
			{
				DocExClipSelect( D_UNI | gbSqSelect );
			}
			break;

		//	SJISコピー
		case IDM_SJISCOPY:
			DocExClipSelect( D_SJIS | gbSqSelect  );
			break;

		//	全SJISコピー
		case IDM_SJISCOPY_ALL:	DocPageAllCopy( D_SJIS );	break;

		//	選択範囲をドラフトボードへ
		case IDM_COPY_TO_DRAUGHT:	DraughtItemAddFromSelect( gbSqSelect  );	break;

		//	カーソル位置の操作も必要・ポインタ渡しして中で弄る
		case IDM_PASTE:			DocInputFromClipboard( &gdDocXdot, &gdDocLine, &gdDocMozi , 0 );	break;

		//	矩形貼付・無理矢理矩形として貼り付ける
		case IDM_SQUARE_PASTE:	DocInputFromClipboard( &gdDocXdot, &gdDocLine, &gdDocMozi , 1 );	break;

		//	削除
		case IDM_DELETE:		Evw_OnKey( hWnd, VK_DELETE, TRUE, 0, 0 );	break;

		//	全選択
		case IDM_ALLSEL:		ViewSelPageAll( 1 );	break;

		//	矩形選択トグル
		case IDM_SQSELECT:		ViewSqSelModeToggle( 1 , NULL );	break;

		//	選択範囲を空白にする
		case IDM_FILL_SPACE:
			DocSelectedBrushFilling( NULL, &gdDocXdot , &gdDocLine );
			ViewDrawCaret( gdDocXdot, gdDocLine, 1 );	//	キャレット位置を決める
			DocPageInfoRenew( -1, 1 );
			break;

		//	画面の再描画
		case IDM_NOW_PAGE_REFRESH:
			ViewRedrawSetLine( -1 );
			PreviewVisibalise( gixFocusPage, FALSE );
			break;

		//	800Dｘ40Lくらいまでを全角スペースで埋めちゃう
		case IDM_FILL_ZENSP:	DocScreenFill( TEXT("　") );	break;
	
		//	抽出モードTOGGLE
		case IDM_EXTRACTION_MODE:
			if( gbExtract )
			{
				gbExtract = FALSE;
				ViewSelPageAll( -1 );	//	選択範囲無くなる
				ViewRedrawSetLine( -1 );	//	画面表示更新
			}
			else{	gbExtract = TRUE;	}
			MenuItemCheckOnOff( IDM_EXTRACTION_MODE, gbExtract );
			OperationOnStatusBar(  );
			break;

		//	レイヤボックス起動
		case IDM_LAYERBOX:
			if( gbExtract )
			{
				DocExtractExecute( ghInst );
				gbExtract = FALSE;		//	取り出したらモード終了
				ViewSelPageAll( -1 );	//	選択範囲無くなる
				ViewRedrawSetLine( -1 );	//	画面表示更新
				MenuItemCheckOnOff( IDM_EXTRACTION_MODE, 0 );
				OperationOnStatusBar(  );
			}
			else
			{
				LayerBoxVisibalise( ghInst, NULL, 0x00 );
			}
			break;

		//	ユニコード空白挿入
		case IDM_IN_01SPACE:
		case IDM_IN_02SPACE:
		case IDM_IN_03SPACE:
		case IDM_IN_04SPACE:
		case IDM_IN_05SPACE:
		case IDM_IN_08SPACE:
		case IDM_IN_10SPACE:
		case IDM_IN_16SPACE:		ViewInsertUniSpace( id );	break;

		//	色タグ挿入
		case IDM_INSTAG_WHITE:
		case IDM_INSTAG_BLUE:
		case IDM_INSTAG_BLACK:
		case IDM_INSTAG_RED:
		case IDM_INSTAG_GREEN:		ViewInsertColourTag( id );	break;

		//	右揃え線
		case IDM_RIGHT_GUIDE_SET:	DocRightGuideline( NULL );	break;

		//	右に寄せる
		case IDM_RIGHT_SLIDE:		DocRightSlide( &gdDocXdot , gdDocLine );	break;

		//	行末空白削除
		case  IDM_DEL_LASTSPACE:	DocLastSpaceErase( &gdDocXdot , gdDocLine );	break;

		//	行頭に全角空白追加
		case IDM_INS_TOPSPACE:		DocTopLetterInsert( TEXT('　'), &gdDocXdot, gdDocLine );	break;

		//	行頭空白削除
		case IDM_DEL_TOPSPACE:		DocTopSpaceErase( &gdDocXdot, gdDocLine );	break;

		//	行末文字削除
		case IDM_DEL_LASTLETTER:	DocLastLetterErase( &gdDocXdot, gdDocLine );	break;

//		case IDM_DEL_HANSPACE:	//	行頭及び連続半角空白削除
//			MENUITEM "行頭及び連続半角空白削除(&U)\tCtrl + L",	IDM_DEL_HANSPACE, GRAYED
//			break;	//	機能追加どうすべ・使うかこれ？

		//	１ドット増やす
		case  IDM_INCREMENT_DOT:	DocSpaceShiftProc( VK_RIGHT, &gdDocXdot, gdDocLine );	break;

		//	１ドット減らす
		case  IDM_DECREMENT_DOT:	DocSpaceShiftProc( VK_LEFT,  &gdDocXdot, gdDocLine );	break;

		//	全体１ドット右へ
		case IDM_INCR_DOT_LINES:	DocPositionShift( VK_RIGHT, &gdDocXdot, gdDocLine );	break;

		//	全体１ドット左へ
		case IDM_DECR_DOT_LINES:	DocPositionShift( VK_LEFT,  &gdDocXdot, gdDocLine );	break;

		//	自動調整位置決定
		case IDM_DOTDIFF_LOCK:
			gdAutoDiffBase = DocDiffAdjBaseSet( gdDocLine );
			ViewRulerRedraw( -1, -1 );
			break;

		//	自動調整する
		case IDM_DOTDIFF_ADJT:	DocDiffAdjExec( &gdDocXdot, gdDocLine );	break;

		//	行頭半角空白をユニコードに変換
		case IDM_HEADHALF_EXCHANGE:	DocHeadHalfSpaceExchange( hWnd );	break;

		//	複数行テンプレを見せたり消したり
		case  IDM_MAATMPLE_VIEW:
			bMode = MaaViewToggle( TRUE );
			InitParamValue( INIT_SAVE, VL_MAA_TOPMOST, bMode );
			MenuItemCheckOnOff( IDM_MAATMPLE_VIEW, bMode );
			break;

		//	空白の表示非表示切換
		case IDM_SPACE_VIEW_TOGGLE:
			gdSpaceView = !(gdSpaceView);
			InitParamValue( INIT_SAVE, VL_SPACE_VIEW, gdSpaceView );
			MenuItemCheckOnOff( IDM_SPACE_VIEW_TOGGLE, gdSpaceView );
			OperationOnStatusBar(  );
			ViewRedrawSetLine( -1 );	//	画面表示更新
			break;

		//	グリッド線TOGGLE
		case IDM_GRID_VIEW_TOGGLE:
			gbGridView = !(gbGridView);
			InitParamValue( INIT_SAVE, VL_GRID_VIEW, gbGridView );
			MenuItemCheckOnOff( IDM_GRID_VIEW_TOGGLE, gbGridView );
			ViewRedrawSetLine( -1 );	//	画面表示更新
			break;

		//	右端ガイドTOGGLE
		case IDM_RIGHT_RULER_TOGGLE:
			gbRitRlrView = !(gbRitRlrView);
			InitParamValue( INIT_SAVE, VL_R_RULER_VIEW, gbRitRlrView );
			MenuItemCheckOnOff( IDM_RIGHT_RULER_TOGGLE, gbRitRlrView );
			ViewRedrawSetLine( -1 );	//	画面表示更新
			break;

		//	ユニコードON/OFFトグル
		case IDM_UNICODE_TOGGLE:	UnicodeUseToggle( NULL );	break;

		//	頁分割
		case IDM_PAGEL_DIVIDE:	DocPageDivide( hWnd, ghInst, gdDocLine );	break;

		case IDM_REBER_DORESET:	ToolBarBandReset( hWnd );	break;

		//	PageCtrlへ飛ばす
		case IDM_PAGEL_ADD:
		case IDM_PAGEL_INSERT:
		case IDM_PAGEL_DELETE:
		case IDM_PAGEL_DUPLICATE:
		case IDM_PAGEL_COMBINE:
		case IDM_PAGEL_UPFLOW:
		case IDM_PAGEL_DOWNSINK:
		case IDM_PAGEL_RENAME:
		case IDM_PAGE_PREV:
		case IDM_PAGE_NEXT:
			FORWARD_WM_COMMAND( ghPgVwWnd, id, hWndCtl, codeNotify, SendMessage );
			break;

		case IDM_TMPLT_GROUP_PREV:
		case IDM_TMPLT_GROUP_NEXT:
		case IDM_TMPL_GRID_INCREASE:
		case IDM_TMPL_GRID_DECREASE:
			if( gbTmpltDock )
			{
				if( IsWindowVisible( ghLnTmplWnd ) ){		FORWARD_WM_COMMAND( ghLnTmplWnd, id, hWndCtl, codeNotify, SendMessage );	}
				else if( IsWindowVisible( ghBrTmplWnd ) ){	FORWARD_WM_COMMAND( ghBrTmplWnd, id, hWndCtl, codeNotify, SendMessage );	}
			}
			break;

		//	Ctrl+PageUpDownでファイルタブを移動
		case  IDM_FILE_PREV:	MultiFileTabSlide( -1 );	break;
		case  IDM_FILE_NEXT:	MultiFileTabSlide(  1 );	break;

		//	Ctrl+Spaceでドラフトボード
		case IDM_DRAUGHT_OPEN:	DraughtWindowCreate( GetModuleHandle(NULL), hWnd, 0 );	break;

		//	Ctrl+Tでサムネイル
		case IDM_MAA_THUMBNAIL_OPEN:	DraughtWindowCreate( GetModuleHandle(NULL), hWnd, 1 );	break;

		//	DOCKING時の、壱行・BRUSHテンプレを表示/非表示
		case IDM_LINE_BRUSH_TMPL_VIEW:	DockingTmplViewToggle(  0 );	break;

#ifdef AA_INVERSE
		case IDM_MIRROR_INVERSE:	DocInverseTransform( gbSqSelect, 1, &gdDocXdot, gdDocLine );	break;
		case IDM_UPSET_INVERSE:		DocInverseTransform( gbSqSelect, 0, &gdDocXdot, gdDocLine );	break;
#endif

		case IDM_TESTCODE:
			TRACE( TEXT("機能テスト") );
			break;
	}


	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	編集Viewの配色変更
	@param[in]	hWnd	ウインドウハンドル
	@return	HRESULT	終了状態コード
*/
HRESULT ViewColourEditDlg( HWND hWnd )
{
	INT_PTR	iRslt;

	COLORREF	cadColourSet[5];

	cadColourSet[0] =  gaColourTable[CLRT_BASICPEN];	//	BasicPen
	cadColourSet[1] =  gaColourTable[CLRT_BASICBK];		//	BasicBack
	cadColourSet[2] =  gaColourTable[CLRT_GRID_LINE];	//	GridLine
	cadColourSet[3] =  gaColourTable[CLRT_CRLF_MARK];	//	CrLfMark
	cadColourSet[4] =  gaColourTable[CLRT_CANTSJIS];	//	CantSjis

	iRslt = DialogBoxParam( ghInst, MAKEINTRESOURCE(IDD_COLOUR_DLG), hWnd, ColourEditDlgProc, (LPARAM)cadColourSet );
	if( IDOK == iRslt )
	{
		gaColourTable[CLRT_BASICPEN]  = cadColourSet[0];
		gaColourTable[CLRT_BASICBK]   = cadColourSet[1];
		gaColourTable[CLRT_GRID_LINE] = cadColourSet[2];
		gaColourTable[CLRT_CRLF_MARK] = cadColourSet[3];
		gaColourTable[CLRT_CANTSJIS]  = cadColourSet[4];

		DeleteBrush( gahBrush[BRHT_BASICBK] );
		DeletePen(   gahPen[PENT_CRLF_MARK] );
		DeletePen(   gahPen[PENT_GRID_LINE] );
		DeleteBrush( gahBrush[BRHT_CANTSJISBK] );

		gahBrush[BRHT_BASICBK]    = CreateSolidBrush( gaColourTable[CLRT_BASICBK] );
		gahPen[PENT_CRLF_MARK]    = CreatePen( PS_SOLID, 1, gaColourTable[CLRT_CRLF_MARK] );
		gahPen[PENT_GRID_LINE]    = CreatePen( PS_SOLID, 1, gaColourTable[CLRT_GRID_LINE] );
		gahBrush[BRHT_CANTSJISBK] = CreateSolidBrush( gaColourTable[CLRT_CANTSJIS] );

		InitColourValue( INIT_SAVE, CLRV_BASICPEN, gaColourTable[CLRT_BASICPEN] );
		InitColourValue( INIT_SAVE, CLRV_BASICBK,  gaColourTable[CLRT_BASICBK] );
		InitColourValue( INIT_SAVE, CLRV_GRIDLINE, gaColourTable[CLRT_GRID_LINE] );
		InitColourValue( INIT_SAVE, CLRV_CRLFMARK, gaColourTable[CLRT_CRLF_MARK] );
		InitColourValue( INIT_SAVE, CLRV_CANTSJIS, gaColourTable[CLRT_CANTSJIS] );

		SetClassLongPtr( ghViewWnd, GCL_HBRBACKGROUND, (LONG_PTR)(gahBrush[BRHT_BASICBK]) );

		InvalidateRect( ghViewWnd, NULL, TRUE );
	}
	
	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	バージョン情報ボックスのメッセージハンドラです。
	@param[in]	hDlg		ダイヤログハンドル
	@param[in]	message		ウインドウメッセージの識別番号
	@param[in]	wParam		追加の情報１
	@param[in]	lParam		追加の情報２
	@retval 0	メッセージは処理していない
	@retval no0	なんか処理された
*/
INT_PTR CALLBACK ColourEditDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static LPCOLORREF	pcadColour;	//	0:Pen　1:Back　2:Grid　3:CrLf　4:CantSjis
	static COLOUROBJECT	cstColours;	
//	COLORREF	dColourTmp;
//	UINT	dRslt;
	INT	id;

	switch( message )
	{
		case WM_INITDIALOG:
			pcadColour = (LPCOLORREF)lParam;
			cstColours.dTextColour = pcadColour[0];
			cstColours.hBackBrush  = CreateSolidBrush( pcadColour[1] );
			cstColours.hGridPen    = CreatePen( PS_SOLID, 1, pcadColour[2] );
			cstColours.hCrLfPen    = CreatePen( PS_SOLID, 1, pcadColour[3] );
			cstColours.hUniBackBrs = CreateSolidBrush( pcadColour[4] );
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			id = LOWORD(wParam);
			switch( id )
			{
				case IDOK:
				case IDCANCEL:
					DeleteBrush( cstColours.hBackBrush );
					DeletePen(   cstColours.hGridPen );
					DeletePen(   cstColours.hCrLfPen );
					DeleteBrush( cstColours.hUniBackBrs );
					EndDialog( hDlg, id );	return (INT_PTR)TRUE;

				case IDB_COLOUR_BASIC_PEN:
					if( ColourEditChoose( hDlg, &(pcadColour[0]) ) )
					{
						cstColours.dTextColour = pcadColour[0];
						InvalidateRect( GetDlgItem(hDlg,IDS_COLOUR_IMAGE), NULL, TRUE );
					}
					return (INT_PTR)TRUE;

				case IDB_COLOUR_BASIC_BACK:
					if( ColourEditChoose( hDlg, &(pcadColour[1]) ) )
					{
						DeleteBrush( cstColours.hBackBrush );
						cstColours.hBackBrush  = CreateSolidBrush( pcadColour[1] );
						InvalidateRect( GetDlgItem(hDlg,IDS_COLOUR_IMAGE), NULL, TRUE );
					}
					return (INT_PTR)TRUE;

				case IDB_COLOUR_GRID_LINE:
					if( ColourEditChoose( hDlg, &(pcadColour[2]) ) )
					{
						DeletePen(   cstColours.hGridPen );
						cstColours.hGridPen  = CreatePen( PS_SOLID, 1, pcadColour[2] );
						InvalidateRect( GetDlgItem(hDlg,IDS_COLOUR_IMAGE), NULL, TRUE );
					}
					return (INT_PTR)TRUE;

				case IDB_COLOUR_CRLF_MARK:
					if( ColourEditChoose( hDlg, &(pcadColour[3]) ) )
					{
						DeletePen(   cstColours.hCrLfPen );
						cstColours.hCrLfPen  = CreatePen( PS_SOLID, 1, pcadColour[3] );
						InvalidateRect( GetDlgItem(hDlg,IDS_COLOUR_IMAGE), NULL, TRUE );
					}
					return (INT_PTR)TRUE;

				case IDB_COLOUR_CANT_SJIS:
					if( ColourEditChoose( hDlg, &(pcadColour[4]) ) )
					{
						DeleteBrush( cstColours.hUniBackBrs );
						cstColours.hUniBackBrs  = CreateSolidBrush( pcadColour[4] );
						InvalidateRect( GetDlgItem(hDlg,IDS_COLOUR_IMAGE), NULL, TRUE );
					}
					return (INT_PTR)TRUE;

				default:	return (INT_PTR)FALSE;
			}
			break;

		case WM_DRAWITEM:	return ColourEditDrawItem( hDlg, ((CONST LPDRAWITEMSTRUCT)(lParam)), &cstColours );

	}
	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	色選択ダイヤログ使っていろいろする
	@param[in]		hWnd		オーナーウインドウハンドル
	@param[in,out]	pdTgtColour	元の色入れて、変更した色出てくる
	@return	UINT	非０変更した　０キャンセルした
*/
UINT ColourEditChoose( HWND hWnd, LPCOLORREF pdTgtColour )
{
	BOOL	bRslt;
	COLORREF	adColourTemp[16];
	CHOOSECOLOR	stChColour;

	ZeroMemory( adColourTemp, sizeof(adColourTemp) );
	adColourTemp[0] = *pdTgtColour;

	ZeroMemory( &stChColour, sizeof(CHOOSECOLOR) );
	stChColour.lStructSize  = sizeof(CHOOSECOLOR);
	stChColour.hwndOwner    = hWnd;
//	stChColour.hInstance    = GetModuleHandle( NULL );
	stChColour.rgbResult    = *pdTgtColour;
	stChColour.lpCustColors = adColourTemp;
	stChColour.Flags        = CC_RGBINIT;

	bRslt = ChooseColor( &stChColour );	//	色ダイヤログ使う
	if( bRslt ){	*pdTgtColour = stChColour.rgbResult;	}

	return bRslt;
}
//-------------------------------------------------------------------------------------------------

/*!
	オーナードローの処理・スタティックのアレ
	@param[in]	hDlg		ダイヤロゲハンドゥ
	@param[in]	pstDrawItem	ドロー情報へのポインター
	@param[in]	pstColours	色情報とGDI
	@return		処理したかせんかったか
*/
INT_PTR ColourEditDrawItem( HWND hDlg, CONST LPDRAWITEMSTRUCT pstDrawItem, LPCOLOUROBJECT pstColours )
{
	const  TCHAR	catMihon[] = { TEXT("フランちゃんウフフ") };
//	UINT_PTR	cchMr;

	const  TCHAR	catUniMhn[] = { 0x2600, 0x2006, 0x2665, 0x0000 };


	RECT	rect;
	INT		xpos;

	INT		dotlen;
	INT		dX = 6 , dY = 6;	//	改行描画枠左上
	INT		aX , aY;	//	改行ペンの位置

	HFONT	hFtOld;
	HPEN	hPenOld;

	if( IDS_COLOUR_IMAGE != pstDrawItem->CtlID ){	return (INT_PTR)FALSE;	}

	//atUniMihon[0] = 0x2600;
	//atUniMihon[1] = 0x2006;
	//atUniMihon[2] = 0x2665;
	//atUniMihon[3] = 0x0000;

	GetClientRect( GetDlgItem(hDlg,IDS_COLOUR_IMAGE), &rect );	//	見本エリアのサイズ

	hFtOld = SelectFont( pstDrawItem->hDC, ghAaFont );	//	フォントくっつける
	SetBkMode( pstDrawItem->hDC, TRANSPARENT );	//	背景透過

	SetTextColor( pstDrawItem->hDC, pstColours->dTextColour );	//	テキスト色

	FillRect( pstDrawItem->hDC, &(pstDrawItem->rcItem), pstColours->hBackBrush );	//	背景塗り

	hPenOld = SelectPen( pstDrawItem->hDC, pstColours->hGridPen );	//	グリッド
	for( xpos = 40; rect.right > xpos; xpos += 40 )
	{
		MoveToEx( pstDrawItem->hDC, xpos, 0, NULL );	//	開始地点
		LineTo(   pstDrawItem->hDC, xpos, rect.bottom );	//	上から下へ
	}

	dotlen = ViewStringWidthGet( catMihon );
	ExtTextOut( pstDrawItem->hDC, dX, dY, 0, NULL, catMihon, 9, NULL );//固定値注意
	dX += dotlen;

	SelectPen( pstDrawItem->hDC, pstColours->hCrLfPen );	//	改行マーク
	aX = dX + 3;
	aY = dY + 3;	//	上マージン
	MoveToEx( pstDrawItem->hDC, aX, aY, NULL );	//	開始地点
	LineTo(   pstDrawItem->hDC, aX, aY + 12  );	//	上から下へ
	LineTo(   pstDrawItem->hDC, dX, aY + 9  );	//	そこから左上へ
	MoveToEx( pstDrawItem->hDC, aX, aY + 12, NULL );	//	矢印の先っぽへ
	LineTo(   pstDrawItem->hDC, aX + 3, aY + 9 );	//	そして右上へ
	//	もうちゅっとスマートにできないかこれ

	//	ユニコードみほん
	dX  = 6;	//	固定値注意
	dY += LINE_HEIGHT;
	dotlen = ViewStringWidthGet( catUniMhn );
	SetRect( &rect, dX, dY, dX + dotlen, dY + LINE_HEIGHT );
	FillRect( pstDrawItem->hDC, &rect, pstColours->hUniBackBrs );	//	背景塗り
	ExtTextOut( pstDrawItem->hDC, dX, dY, 0, NULL, catUniMhn, 3, NULL );//固定値注意

	SelectPen( pstDrawItem->hDC, hPenOld );
	SelectFont( pstDrawItem->hDC, hFtOld );

	return (INT_PTR)TRUE;
}
//-------------------------------------------------------------------------------------------------

