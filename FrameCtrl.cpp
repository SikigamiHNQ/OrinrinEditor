/*! @file
	@brief 枠の面倒見る
	このファイルは FrameCtrl.cpp です。
	@author	SikigamiHNQ
	@date	2011/06/08
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
複数行断片を使うには
・区切りは￥ｎとし、読み書のタイミングで適当に変更する
・各パーツの幅と高さを持っておく
・配置用ダミーレイヤボックスみたいなのが居る
・目標幅に合わせて、上パーツを並べる・右上は、上パーツの右側を削る計算がいるか？
・左下に向かって配置していく
・右下に向かっていくのは同じ理屈のはず
・配置出来たら、枠ボックスならそのままレイヤ処理、挿入なら、元文字列をずらす処理して挿入か

ＡＡ文字配置と似てるか？

INIファイルは、文字列先頭の半角は無視するらしい？
￥ｓ＝半角空白とする
*/

#define FRAMEINSERTBOX_CLASS	TEXT("FRAMEINSBOX_CLASS")
#define FIB_WIDTH	600
#define FIB_HEIGHT	200

#define TRANCE_COLOUR	RGB(0x66,0x77,0x88)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------

#define TB_ITEMS	26
static  TBBUTTON	gstFIBTBInfo[] = {
	{ 20,	IDM_FRAME_INS_DECIDE,	TBSTATE_ENABLED,	TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE,		{0, 0}, 0, 0  },	//	
	{  0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,							{0, 0}, 0, 0  },
	{  0,	IDM_INSFRAME_ALPHA   ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{  1,	IDM_INSFRAME_BRAVO   ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{  2,	IDM_INSFRAME_CHARLIE ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{  3,	IDM_INSFRAME_DELTA   ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{  4,	IDM_INSFRAME_ECHO    ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{  5,	IDM_INSFRAME_FOXTROT ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{  6,	IDM_INSFRAME_GOLF    ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{  7,	IDM_INSFRAME_HOTEL   ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{  8,	IDM_INSFRAME_INDIA   ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{  9,	IDM_INSFRAME_JULIETTE,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{ 10,	IDM_INSFRAME_KILO,		TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{ 11,	IDM_INSFRAME_LIMA,		TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{ 12,	IDM_INSFRAME_MIKE,		TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{ 13,	IDM_INSFRAME_NOVEMBER,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{ 14,	IDM_INSFRAME_OSCAR,		TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{ 15,	IDM_INSFRAME_PAPA,		TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{ 16,	IDM_INSFRAME_QUEBEC,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{ 17,	IDM_INSFRAME_ROMEO,		TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{ 18,	IDM_INSFRAME_SIERRA,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{ 19,	IDM_INSFRAME_TANGO,		TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{  0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,							{0, 0}, 0, 0  },
	{ 21,	IDM_FRMINSBOX_QCLOSE,	TBSTATE_ENABLED,	TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,		{0, 0}, 0, 0  },	//	
	{  0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,							{0, 0}, 0, 0  },
	{ 22,	IDM_FRMINSBOX_PADDING,	TBSTATE_ENABLED,	TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,		{0, 0}, 0, 0  } 	//	
};	//	

CONST  TCHAR	*gatDefaultName[20] = {
	{ TEXT("ALPHA") },		{ TEXT("BRAVO") },	{ TEXT("CHARLIE") },	{ TEXT("DELTA") },		{ TEXT("ECHO") },
	{ TEXT("FOXTROT") },	{ TEXT("GOLF") },	{ TEXT("HOTEL") },		{ TEXT("INDIA") },		{ TEXT("JULIETTE") },
	{ TEXT("KILO") },		{ TEXT("LIMA") },	{ TEXT("MIKE") },		{ TEXT("NOVEMBER") },	{ TEXT("OSCAR") },
	{ TEXT("POPPA") },		{ TEXT("QUEBEC") },	{ TEXT("ROMEO") },		{ TEXT("SIERRA") },		{ TEXT("TANGO") }
};

//-------------------------------------------------------------------------------------------------


extern FILES_ITR	gitFileIt;	//!<	今見てるファイルの本体
extern INT		gixFocusPage;	//!<	注目中のページ・とりあえず０・０インデックス


extern  HWND	ghViewWnd;		//!<	ビューウインドウハンドル

extern INT		gdHideXdot;		//!<	左の隠れ部分
extern INT		gdViewTopLine;	//!<	表示中の最上部行番号

extern HFONT	ghAaFont;		//!<	AA用フォント

static INT		gNowSel;		//!<	選択中の枠０インデックス

static TCHAR	gatFrameIni[MAX_PATH];	//!<	20110707	枠用のINIいれる・増やした


static  ATOM		gFrmInsAtom;	//!<	
static  HWND		ghFrInbxWnd;
static  HWND		ghFIBtlbrWnd;
static HBRUSH		ghBgBrush;

static HIMAGELIST	ghFrameImgLst;	//!<	挿入BOX用のツールバー

static POINT		gstViewOrigin;	//!<	ビューの左上ウインドウ位置・
static POINT		gstOffset;		//!<	ビュー左上からの、ボックスの相対位置
static POINT		gstFrmSz;		//!<	ウインドウエッジから描画領域までのオフセット
static INT			gdToolBarHei;	//!<	ツールバー太さ

static  UINT		gdSelect;		//!<	選択した枠番号０～９
static BOOLEAN		gbQuickClose;	//!<	貼り付けたら直ぐ閉じる

extern HFONT		ghAaFont;		//	AA用フォント

static  RECT		gstOrigRect;	//!<	ダイヤログ起動時、つまり最小ウインドウサイズ

static LPTSTR		gptFrmSample;	//!<	枠設定ダイヤログの見本用
static  RECT		gstSamplePos;	//!<	見本窓の左上位置と、幅高さのオフセット量
static FRAMEINFO	gstNowFrameInfo;

static LPTSTR		gptFrmBox;		//!<	挿入枠用の文字列

static  UINT		gbMultiPaddTemp;	//!<	外周に沿うようにパディングするか・挿入BOX用

static FRAMEINFO	gstFrameInfo[FRAME_MAX];	//!<	配列で必要数確保でいいか
//-------------------------------------------------------------------------------------------------

INT_PTR	CALLBACK FrameEditDlgProc( HWND, UINT, WPARAM, LPARAM );	//!<	

INT_PTR	Frm_OnInitDialog( HWND , HWND, LPARAM );	//!<	
INT_PTR	Frm_OnCommand( HWND , INT, HWND, UINT );	//!<	
INT_PTR	Frm_OnDrawItem( HWND , CONST LPDRAWITEMSTRUCT );	//!<	
INT_PTR	Frm_OnNotify( HWND , INT, LPNMHDR );	//!<	

HRESULT	InitFrameItem( UINT, UINT, LPFRAMEINFO );	//!<	

HRESULT	FramePartsUpdate( HWND, HWND, LPFRAMEITEM );	//!<	

HRESULT	FrameDataGet( UINT, LPFRAMEINFO );	//!<	
HRESULT	FrameInfoDisp( HWND );	//!<	

VOID	FrameDataTranslate( LPTSTR, UINT );	//!<	
//INT		FramePartsSizeCalc( LPTSTR, PINT );

UINT	FrameMultiSubstring( LPCTSTR, CONST UINT, LPTSTR, CONST UINT_PTR, CONST INT );	//!<	

INT		FrameMultiSizeGet( LPFRAMEINFO, PINT, PINT );	//!<	
LPTSTR	FrameMakeOutsideBoundary( CONST INT, CONST INT, LPFRAMEINFO );	//!<	
LPTSTR	FrameMakeInsideBoundary( UINT , PINT, LPFRAMEINFO );	//!<	


INT		FrameInsBoxSizeGet( LPRECT );
VOID	FrameInsBoxFrmDraw( HDC );
VOID	FrameDrawItem( HDC, INT, LPTSTR );
INT_PTR	Frm_OnSize( HWND, UINT, INT, INT );
INT_PTR	Frm_OnWindowPosChanging( HWND, LPWINDOWPOS );

HRESULT	FrameInsBoxDoInsert( HWND );

LRESULT	CALLBACK FrameInsProc( HWND, UINT, WPARAM, LPARAM );
VOID	Fib_OnPaint( HWND );
VOID	Fib_OnCommand( HWND, INT, HWND, UINT );
VOID	Fib_OnDestroy( HWND );
VOID	Fib_OnMoving( HWND, LPRECT );			//!<	
VOID	Fib_OnKey( HWND, UINT, BOOL, INT, UINT );	//!<	
BOOL	Fib_OnWindowPosChanging( HWND, LPWINDOWPOS );		//!<	
VOID	Fib_OnWindowPosChanged( HWND, const LPWINDOWPOS );	//!<	
//-------------------------------------------------------------------------------------------------

/*!
	枠設定のINIファイル名確保・アプリ起動後すぐ呼ばれる
	@param[in]	ptCurrent	基準ディレクトリ
	@param[in]	hInstance	インスタンスハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT FrameInitialise( LPTSTR ptCurrent, HINSTANCE hInstance )
{
	WNDCLASSEX	wcex;
	UINT		resnum, ici;
	HBITMAP		hImg, hMsq;
	INT	iRslt;

	if( !(ptCurrent) || !(hInstance) )
	{
		if( ghFrInbxWnd ){	DestroyWindow( ghFrInbxWnd );	}
		if( ghBgBrush ){	DeleteBrush( ghBgBrush );	}

		if( ghFrameImgLst ){	ImageList_Destroy( ghFrameImgLst  );	}

		return S_OK;
	}

	StringCchCopy( gatFrameIni, MAX_PATH, ptCurrent );
	PathAppend( gatFrameIni, FRAME_INI_FILE );

//枠挿入窓
	ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= FrameInsProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= FRAMEINSERTBOX_CLASS;
	wcex.hIconSm		= NULL;

	gFrmInsAtom = RegisterClassEx( &wcex );

	ghBgBrush = CreateSolidBrush( TRANCE_COLOUR );

	ghFrInbxWnd = NULL;

	gptFrmBox = NULL;

	gdSelect = 0;

	gbQuickClose = TRUE;

	ZeroMemory( &gstViewOrigin, sizeof(POINT) );
	ZeroMemory( &gstFrmSz, sizeof(POINT) );
	gdToolBarHei = 0;

	//	アイコン
	ghFrameImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 23, 1 );
	resnum = IDBMP_FRMINS_ALPHA;
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_FRMINS_SEL ) );
	for( ici = 0; FRAME_MAX > ici; ici++ )
	{
		hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( (resnum++) ) );
		iRslt = ImageList_Add( ghFrameImgLst, hImg, hMsq );	//	イメージリストにイメージを追加・０～１９
		DeleteBitmap( hImg );
	}
	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMP_FRMINS_INSERT ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_FRMINS_INSERT ) );
	iRslt = ImageList_Add( ghFrameImgLst, hImg, hMsq );	//	イメージリストにイメージを追加・２０
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMP_REFRESH ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_REFRESH ) );
	iRslt = ImageList_Add( ghFrameImgLst, hImg, hMsq );	//	イメージリストにイメージを追加・２１
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMP_FRMINS_PADD ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_FRMINS_PADD ) );
	iRslt = ImageList_Add( ghFrameImgLst, hImg, hMsq );	//	イメージリストにイメージを追加・２２
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );


	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	枠名称の内容でメニューを変更
	@param[in]	hWnd	メインウインドウハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT FrameNameModifyMenu( HWND hWnd )
{
	HMENU	hMenu, hSubMenu;
	UINT	i, j, k;
	TCHAR	atBuffer[MAX_PATH], atName[MAX_STRING];

	//	メニュー構造変わったらここも変更・どうにかならんのか
	hMenu = GetMenu( hWnd );
	hSubMenu = GetSubMenu( hMenu, 2 );

	//	枠
	for( i = 0, j = 1; FRAME_MAX > i; i++, j++ )
	{
		FrameNameLoad( i, atName, MAX_STRING );

		if(  9 >= j ){		k = j + '0';	}
		else if( 10 == j ){	k = '0';	}
		else{		k = 'A' + j - 11;	}
		StringCchPrintf( atBuffer, MAX_PATH, TEXT("%s(&%c)"), atName, k );
		ModifyMenu( hSubMenu, IDM_INSFRAME_ALPHA+i, MF_BYCOMMAND | MFT_STRING, IDM_INSFRAME_ALPHA+i, atBuffer );
		//	メニューリソース番号の連番に注意
	}

	DrawMenuBar( hWnd );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ポッパップメニュー用に名前をずっこんばっこん
	@param[in]	hPopMenu	対象のポッパップメニューハンドル
	@param[in]	bMode		非０メニューキー付ける　０付けない
	@return		HRESULT	終了状態コード
*/
HRESULT FrameNameModifyPopUp( HMENU hPopMenu, UINT bMode )
{
	UINT	i, j, k;
	TCHAR	atBuffer[MAX_PATH], atName[MAX_STRING];

	for( i = 0, j = 1; FRAME_MAX > i; i++, j++ )
	{
		FrameNameLoad( i, atName, MAX_STRING );

		if( bMode )
		{
			if(  9 >= j ){		k = j + '0';	}
			else if( 10 == j ){	k = '0';	}
			else{		k = 'A' + j - 11;	}
			StringCchPrintf( atBuffer, MAX_PATH, TEXT("%s(&%c)"), atName, k );
		}
		else
		{
			StringCchPrintf( atBuffer, MAX_PATH, TEXT("%s"), atName );
		}
		ModifyMenu( hPopMenu, IDM_INSFRAME_ALPHA+i, MF_BYCOMMAND | MFT_STRING, IDM_INSFRAME_ALPHA+i, atBuffer );
		//	メニューリソース番号の連番に注意
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	枠の名前を引っ張ってくる
	@param[in]	dNumber	枠番号０インデックス
	@param[out]	ptNamed	名前入れるバッファへのポインター
	@param[in]	cchSize	バッファの文字数・バイトじゃないぞ
	@return		HRESULT	終了状態コード
*/
HRESULT FrameNameLoad( UINT dNumber, LPTSTR ptNamed, UINT_PTR cchSize )
{
	TCHAR	atAppName[MIN_STRING];

	if( !(ptNamed) || 0 >= cchSize )	return E_INVALIDARG;

	if( FRAME_MAX <= dNumber )	return E_OUTOFMEMORY;

	//	所定のAPP名を作る
	StringCchPrintf( atAppName, MIN_STRING, TEXT("Frame%u"), dNumber );

	GetPrivateProfileString( atAppName, TEXT("Name"), gatDefaultName[dNumber], ptNamed, cchSize, gatFrameIni );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

//	20110707	場所変えた
/*!
	枠情報のセーブロード
	@param[in]		dMode	非０ロード　０セーブ
	@param[in]		dNumber	枠番号０インデックス
	@param[in,out]	pstInfo	情報入れるバッファだったり保存内容だったりする構造体ぽいんた～
	@return			HRESULT	終了状態コード
*/
HRESULT InitFrameItem( UINT dMode, UINT dNumber, LPFRAMEINFO pstInfo )
{
	TCHAR	atAppName[MIN_STRING], atBuff[MIN_STRING];
	TCHAR	atBuffer[PARTS_CCH];

	//	所定のAPP名を作る
	StringCchPrintf( atAppName, MIN_STRING, TEXT("Frame%u"), dNumber );

//20120105	複数行を扱う、￥￥と￥ｎによる相互変換函数を用意

	if( dMode )	//	ロード
	{
		GetPrivateProfileString( atAppName, TEXT("Name"), gatDefaultName[dNumber], pstInfo->atFrameName, MAX_STRING, gatFrameIni );

		GetPrivateProfileString( atAppName, TEXT("Daybreak"),  TEXT("│"), pstInfo->stDaybreak.atParts, PARTS_CCH, gatFrameIni );
		GetPrivateProfileString( atAppName, TEXT("Morning"),   TEXT("┌"), pstInfo->stMorning.atParts, PARTS_CCH, gatFrameIni );
		GetPrivateProfileString( atAppName, TEXT("Noon"),      TEXT("─"), pstInfo->stNoon.atParts, PARTS_CCH, gatFrameIni );
		GetPrivateProfileString( atAppName, TEXT("Afternoon"), TEXT("┐"), pstInfo->stAfternoon.atParts, PARTS_CCH, gatFrameIni );
		GetPrivateProfileString( atAppName, TEXT("Nightfall"), TEXT("│"), pstInfo->stNightfall.atParts, PARTS_CCH, gatFrameIni );
		GetPrivateProfileString( atAppName, TEXT("Twilight"),  TEXT("┘"), pstInfo->stTwilight.atParts, PARTS_CCH, gatFrameIni );
		GetPrivateProfileString( atAppName, TEXT("Midnight"),  TEXT("─"), pstInfo->stMidnight.atParts, PARTS_CCH, gatFrameIni );
		GetPrivateProfileString( atAppName, TEXT("Dawn"),      TEXT("└"), pstInfo->stDawn.atParts, PARTS_CCH, gatFrameIni );

		FrameDataTranslate( pstInfo->stDaybreak.atParts , 1 );
		FrameDataTranslate( pstInfo->stMorning.atParts , 1 );
		FrameDataTranslate( pstInfo->stNoon.atParts , 1 );
		FrameDataTranslate( pstInfo->stAfternoon.atParts , 1 );
		FrameDataTranslate( pstInfo->stNightfall.atParts , 1 );
		FrameDataTranslate( pstInfo->stTwilight.atParts , 1 );
		FrameDataTranslate( pstInfo->stMidnight.atParts , 1 );
		FrameDataTranslate( pstInfo->stDawn.atParts , 1 );

		GetPrivateProfileString( atAppName, TEXT("LEFTOFFSET"),  TEXT("0"), atBuff, MIN_STRING, gatFrameIni );
		pstInfo->dLeftOffset  = StrToInt( atBuff );
		GetPrivateProfileString( atAppName, TEXT("RIGHTOFFSET"), TEXT("0"), atBuff, MIN_STRING, gatFrameIni );
		pstInfo->dRightOffset = StrToInt( atBuff );

		//追加
		GetPrivateProfileString( atAppName, TEXT("RestPadding"),  TEXT("1"), atBuff, MIN_STRING, gatFrameIni );
		pstInfo->dRestPadd = StrToInt( atBuff );

	}
	else	//	セーブ
	{
		WritePrivateProfileString( atAppName, TEXT("Name"), pstInfo->atFrameName, gatFrameIni );

		StringCchCopy( atBuffer, PARTS_CCH, pstInfo->stDaybreak.atParts );	FrameDataTranslate( atBuffer, 0 );
		WritePrivateProfileString( atAppName, TEXT("Daybreak"),  atBuffer, gatFrameIni );
		StringCchCopy( atBuffer, PARTS_CCH, pstInfo->stMorning.atParts );	FrameDataTranslate( atBuffer, 0 );
		WritePrivateProfileString( atAppName, TEXT("Morning"),   atBuffer, gatFrameIni );
		StringCchCopy( atBuffer, PARTS_CCH, pstInfo->stNoon.atParts );		FrameDataTranslate( atBuffer, 0 );
		WritePrivateProfileString( atAppName, TEXT("Noon"),      atBuffer, gatFrameIni );
		StringCchCopy( atBuffer, PARTS_CCH, pstInfo->stAfternoon.atParts );	FrameDataTranslate( atBuffer, 0 );
		WritePrivateProfileString( atAppName, TEXT("Afternoon"), atBuffer, gatFrameIni );
		StringCchCopy( atBuffer, PARTS_CCH, pstInfo->stNightfall.atParts );	FrameDataTranslate( atBuffer, 0 );
		WritePrivateProfileString( atAppName, TEXT("Nightfall"), atBuffer, gatFrameIni );
		StringCchCopy( atBuffer, PARTS_CCH, pstInfo->stTwilight.atParts );	FrameDataTranslate( atBuffer, 0 );
		WritePrivateProfileString( atAppName, TEXT("Twilight"),  atBuffer, gatFrameIni );
		StringCchCopy( atBuffer, PARTS_CCH, pstInfo->stMidnight.atParts );	FrameDataTranslate( atBuffer, 0 );
		WritePrivateProfileString( atAppName, TEXT("Midnight"),  atBuffer, gatFrameIni );
		StringCchCopy( atBuffer, PARTS_CCH, pstInfo->stDawn.atParts );		FrameDataTranslate( atBuffer, 0 );
		WritePrivateProfileString( atAppName, TEXT("Dawn"),      atBuffer, gatFrameIni );

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstInfo->dLeftOffset );
		WritePrivateProfileString( atAppName, TEXT("LEFTOFFSET"), atBuff, gatFrameIni );
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstInfo->dRightOffset );
		WritePrivateProfileString( atAppName, TEXT("RIGHTOFFSET"), atBuff, gatFrameIni );

		//	追加
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstInfo->dRestPadd );
		WritePrivateProfileString( atAppName, TEXT("RestPadding"), atBuff, gatFrameIni );
	}


	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	割り算する。０除算なら０を返す
	@param[in]	iLeft	割られる数
	@param[in]	iRight	割る数
	@return	INT	計算結果
*/
INT Divinus( INT iLeft, INT iRight )
{
	 INT	iAnswer;

	if( 0 == iRight )	return 0;

	iAnswer = iLeft / iRight;

	return iAnswer;
}
//-------------------------------------------------------------------------------------------------


/*!
	枠設定のダイヤログを開く
	@param[in]	hInst	アプリの実存
	@param[in]	hWnd	本体のウインドウハンドルであるようにすること
	@param[in]	dRsv	未使用
	@return 終了コード
*/
INT_PTR FrameEditDialogue( HINSTANCE hInst, HWND hWnd, UINT dRsv )
{
	INT_PTR	iRslt;

	gNowSel = 0;

	iRslt = DialogBoxParam( hInst, MAKEINTRESOURCE(IDD_FRAME_EDIT_DLG_2), hWnd, FrameEditDlgProc, 0 );

	//	処理結果によっては、ここでメニューの内容書換
	if( IDYES == iRslt ){	FrameNameModifyMenu( hWnd );	}

	return iRslt;
}
//-------------------------------------------------------------------------------------------------

/*!
	枠設定ダイヤログプロシージャ
	@param[in]	hDlg	ダイヤログハンドル
	@param[in]	message	ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@retval 0	メッセージは処理していない
	@retval no0	なんか処理された
*/
INT_PTR CALLBACK FrameEditDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		default:	break;

		case WM_INITDIALOG:	return Frm_OnInitDialog( hDlg, (HWND)(wParam), lParam );
		case WM_COMMAND:	return Frm_OnCommand( hDlg, (INT)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam) );
		case WM_DRAWITEM:	return Frm_OnDrawItem( hDlg, ((CONST LPDRAWITEMSTRUCT)(lParam)) );
		case WM_NOTIFY:		return Frm_OnNotify( hDlg, (INT)(wParam), (LPNMHDR)(lParam) );
		case WM_SIZE:		return Frm_OnSize( hDlg, (UINT)(wParam), (INT)(SHORT)LOWORD(lParam), (INT)(SHORT)HIWORD(lParam) );

		case WM_WINDOWPOSCHANGING:	return Frm_OnWindowPosChanging( hDlg, (LPWINDOWPOS)(lParam) );

	}
	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	ダイヤログ起動時の初期化
	@param[in]	hDlg		ダイヤログハンドル
	@param[in]	hWndFocus	なんだっけ
	@param[in]	lParam		ダイヤログオーポンするときに呼び出し側が渡した値
	@return		特に意味はない
*/
INT_PTR Frm_OnInitDialog( HWND hDlg, HWND hWndFocus, LPARAM lParam )
{
	HWND	hWorkWnd;
	UINT	i, ofs;
	RECT	rect;
	POINT	point;

	GetWindowRect( hDlg, &gstOrigRect );
	gstOrigRect.bottom -= gstOrigRect.top;
	gstOrigRect.right  -= gstOrigRect.left;
	gstOrigRect.top  = 0;
	gstOrigRect.left = 0;

	TRACE( TEXT("FRM DLG SIZE [%d x %d]"), gstOrigRect.right, gstOrigRect.bottom );



	//	コンボックスに名前いれとく
	hWorkWnd = GetDlgItem( hDlg, IDCB_BOX_NAME_SEL );

	for( i = 0; FRAME_MAX > i; i++ )	//	バッファに確保
	{
		FrameDataGet( i, &(gstFrameInfo[i]) );
		//	INIファイルから引っ張って、コンボックスに名前をいれちゃう
		ComboBox_AddString( hWorkWnd, gstFrameInfo[i].atFrameName );
	}

	ComboBox_SetCurSel( hWorkWnd, gNowSel );

	//	パーツ情報をいれる
	FrameInfoDisp( hDlg );

	SetWindowFont( GetDlgItem(hDlg,IDS_FRAME_IMAGE), ghAaFont, FALSE );

	//	見本ウインドウの位置を確定しておく
	GetWindowRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), &gstSamplePos );
	//	幅高さ確定
	gstSamplePos.right -= gstSamplePos.left;
	gstSamplePos.bottom -= gstSamplePos.top;

	//	クライヤント領域とのズレを調整
	GetClientRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), &rect );
	ofs = gstSamplePos.right - rect.right;	gstSamplePos.right += ofs;
	ofs = gstSamplePos.bottom - rect.bottom;	gstSamplePos.bottom += ofs;

	//	クライアント上での位置を確定
	point.x = gstSamplePos.left;	point.y = gstSamplePos.top;
	ScreenToClient( hDlg, &point );
	gstSamplePos.left = point.x;	gstSamplePos.top  = point.y;

	//	クライアントの幅高さのオフセット量を確定
	GetClientRect( hDlg, &rect );
	gstSamplePos.right  = rect.right  - gstSamplePos.right;
	gstSamplePos.bottom = rect.bottom - gstSamplePos.bottom;

	//xx = cx - gstSamplePos.right;
	//yy = cy - gstSamplePos.bottom;

	//SetWindowPos( hSmpWnd, HWND_TOP, 0, 0, xx, yy, SWP_NOMOVE | SWP_NOZORDER );



	GetClientRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), &rect );
	//	初期状態で確保
	gptFrmSample = FrameMakeOutsideBoundary( rect.right, rect.bottom, &(gstFrameInfo[gNowSel]) );


	return (INT_PTR)TRUE;
}
//-------------------------------------------------------------------------------------------------

/*!
	ダイヤログのCOMMANDメッセージの受け取り
	@param[in]	hDlg		ダイヤログーハンドル
	@param[in]	id			メッセージを発生させた子ウインドウの識別子	LOWORD(wParam)
	@param[in]	hWndCtl		メッセージを発生させた子ウインドウのハンドル	lParam
	@param[in]	codeNotify	通知メッセージ	HIWORD(wParam)
	@return		処理したかせんかったか
*/
INT_PTR Frm_OnCommand( HWND hDlg, INT id, HWND hWndCtl, UINT codeNotify )
{
	static BOOLEAN	cbNameMod = FALSE;	//	ダイヤログ終わり用の恒久的なもの
	static BOOLEAN	cbNameChg = FALSE;	//	APPLY用
	UINT	i;
	INT		iRslt;
	HWND	hCmboxWnd;
	RECT	rect;

	switch( id )
	{
		default:	break;

		case IDCANCEL:
		case IDB_CANCEL:
			FREE( gptFrmSample );
			EndDialog( hDlg, IDCANCEL );
			return (INT_PTR)TRUE;

		case IDB_APPLY:
		case IDB_OK:
			hCmboxWnd = GetDlgItem( hDlg, IDCB_BOX_NAME_SEL );
			for( i = 0; FRAME_MAX > i; i++ )
			{
				InitFrameItem( INIT_SAVE, i, &(gstFrameInfo[i]) );
				if( cbNameChg )
				{
					ComboBox_DeleteString( hCmboxWnd, 0 );//先頭消して
					ComboBox_AddString( hCmboxWnd, gstFrameInfo[i].atFrameName );//末尾に付け足す
				}
			}
			ComboBox_SetCurSel( hCmboxWnd, gNowSel );
			cbNameChg = FALSE;
			if( IDB_OK ==  id )
			{
				FREE( gptFrmSample );
				EndDialog( hDlg, cbNameMod ? IDYES : IDOK );
			}
			return (INT_PTR)TRUE;

		case IDE_BOXP_MORNING:		if( EN_UPDATE == codeNotify ){	FramePartsUpdate( hDlg , hWndCtl, &(gstFrameInfo[gNowSel].stMorning) );	}		return (INT_PTR)TRUE;
		case IDE_BOXP_NOON:			if( EN_UPDATE == codeNotify ){	FramePartsUpdate( hDlg , hWndCtl, &(gstFrameInfo[gNowSel].stNoon) );	}		return (INT_PTR)TRUE;
		case IDE_BOXP_AFTERNOON:	if( EN_UPDATE == codeNotify ){	FramePartsUpdate( hDlg , hWndCtl, &(gstFrameInfo[gNowSel].stAfternoon) );	}	return (INT_PTR)TRUE;
		case IDE_BOXP_DAYBREAK:		if( EN_UPDATE == codeNotify ){	FramePartsUpdate( hDlg , hWndCtl, &(gstFrameInfo[gNowSel].stDaybreak) );	}	return (INT_PTR)TRUE;
		case IDE_BOXP_NIGHTFALL:	if( EN_UPDATE == codeNotify ){	FramePartsUpdate( hDlg , hWndCtl, &(gstFrameInfo[gNowSel].stNightfall) );	}	return (INT_PTR)TRUE;
		case IDE_BOXP_TWILIGHT:		if( EN_UPDATE == codeNotify ){	FramePartsUpdate( hDlg , hWndCtl, &(gstFrameInfo[gNowSel].stTwilight) );	}	return (INT_PTR)TRUE;
		case IDE_BOXP_MIDNIGHT:		if( EN_UPDATE == codeNotify ){	FramePartsUpdate( hDlg , hWndCtl, &(gstFrameInfo[gNowSel].stMidnight) );	}	return (INT_PTR)TRUE;
		case IDE_BOXP_DAWN:			if( EN_UPDATE == codeNotify ){	FramePartsUpdate( hDlg , hWndCtl, &(gstFrameInfo[gNowSel].stDawn) );	}		return (INT_PTR)TRUE;

		case IDS_FRAME_IMAGE:
			if( STN_DBLCLK == codeNotify )	//	ダボークルックされた
			{
				TRACE( TEXT("だぼーくるっく") );
				InvalidateRect( hWndCtl, NULL, TRUE );
			}
			return (INT_PTR)TRUE;

		case IDB_FRM_PADDING:
			iRslt = Button_GetCheck( hWndCtl );
			gstFrameInfo[gNowSel].dRestPadd = (BST_CHECKED == iRslt) ? TRUE : FALSE;

			GetClientRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), &rect );
			FREE( gptFrmSample );
			gptFrmSample = FrameMakeOutsideBoundary( rect.right, rect.bottom, &(gstFrameInfo[gNowSel]) );

			InvalidateRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), NULL, TRUE );
			return (INT_PTR)TRUE;

		case IDB_BOXP_NAME_APPLY:	//	名称を変更した
			Edit_GetText( GetDlgItem(hDlg,IDE_BOXP_NAME_EDIT), gstFrameInfo[gNowSel].atFrameName, MAX_STRING );
			cbNameMod = TRUE;	cbNameChg = TRUE;
			return (INT_PTR)TRUE;

		case IDCB_BOX_NAME_SEL:
			if( CBN_SELCHANGE == codeNotify )	//	選択が変更された
			{
				gNowSel = ComboBox_GetCurSel( hWndCtl );
				FrameInfoDisp( hDlg );

				GetClientRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), &rect );
				FREE( gptFrmSample );
				gptFrmSample = FrameMakeOutsideBoundary( rect.right, rect.bottom, &(gstFrameInfo[gNowSel]) );

				InvalidateRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), NULL, TRUE );
			}
			return (INT_PTR)TRUE;
	}

	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	各パーツが更新されたら
	@param[in]	hDlg	ダイヤロゲハンドゥ
	@param[in]	hWndCtl	対象のパーツEDITBOX
	@param[in]	pstItem	対象パーツのデータ
	@return		HRESULT	終了状態コード
*/
HRESULT FramePartsUpdate( HWND hDlg, HWND hWndCtl, LPFRAMEITEM pstItem )
{
	TCHAR	atBuffer[PARTS_CCH];

	if( Edit_GetTextLength( hWndCtl ) )
	{
		Edit_GetText( hWndCtl, atBuffer, PARTS_CCH );
		atBuffer[PARTS_CCH-1] = 0;
		StringCchCopy( pstItem->atParts, PARTS_CCH, atBuffer );
	}
	else	//	文字がなかったら、全角空白にしちゃう
	{
		StringCchCopy( pstItem->atParts, PARTS_CCH, TEXT("　") );
	}

	//	ドット数確認して
	//pstItem->dDot = FramePartsSizeCalc( pstItem->atParts, &(pstItem->iLine) );
	pstItem->iLine = DocStringInfoCount( pstItem->atParts, 0, &(pstItem->dDot), NULL );

	//	ついでに再描画
	InvalidateRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), NULL, TRUE );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ダイヤログのサイズ変更が完了する前に送られてくる
	@param[in]	hDlg	ダイヤログのハンドル
	@param[in]	pstWpos	新しい位置と大きさが入ってる
	@return		処理したかせんかったかでいい？
*/
INT_PTR Frm_OnWindowPosChanging( HWND hDlg, LPWINDOWPOS pstWpos )
{
	//	移動がなかったときは何もしないでおｋ
	if( SWP_NOSIZE & pstWpos->flags )	return FALSE;

	//	x,y：ウインドウ左上座標　cx,cy：ウインドウの幅高さ
	TRACE( TEXT("FRM CHANGING [%d x %d][%d x %d]"), pstWpos->x, pstWpos->y, pstWpos->cx, pstWpos->cy );

	if( gstOrigRect.right > pstWpos->cx )	pstWpos->cx = gstOrigRect.right;
	if( gstOrigRect.bottom > pstWpos->cy )	pstWpos->cy = gstOrigRect.bottom;

	return (INT_PTR)TRUE;
}
//-------------------------------------------------------------------------------------------------

/*!
	ダイヤログがサイズ変更されたとき
	@param[in]	hDlg	ダイヤログのハンドル
	@param[in]	state	変更の状態・SIZE_MINIMIZED とか
	@param[in]	cx		クライヤントＸサイズ
	@param[in]	cy		クライヤントＹサイズ
	@return	処理したかせんかったか
*/
INT_PTR Frm_OnSize( HWND hDlg, UINT state, INT cx, INT cy )
{
	HWND	hSmpWnd;
	INT		xx, yy;
	RECT	rect;

	TRACE( TEXT("FRM SIZE [%d x %d]"), cx, cy );

	hSmpWnd = GetDlgItem( hDlg, IDS_FRAME_IMAGE );

	//	下半分に常に全開
	xx = cx - gstSamplePos.right;
	yy = cy - gstSamplePos.bottom;

	SetWindowPos( hSmpWnd, HWND_TOP, 0, 0, xx, yy, SWP_NOMOVE | SWP_NOZORDER );

	GetClientRect( hSmpWnd, &rect );
	FREE( gptFrmSample );
	gptFrmSample = FrameMakeOutsideBoundary( rect.right, rect.bottom, &(gstFrameInfo[gNowSel]) );

	InvalidateRect( GetDlgItem( hDlg, IDS_FRAME_IMAGE ), NULL, TRUE );

	return (INT_PTR)TRUE;
}
//-------------------------------------------------------------------------------------------------


/*!
	オーナードローの処理・スタティックのアレ
	@param[in]	hDlg		ダイヤロゲハンドゥ
	@param[in]	pstDrawItem	ドロー情報へのポインター
	@return		処理したかせんかったか
*/
INT_PTR Frm_OnDrawItem( HWND hDlg, CONST LPDRAWITEMSTRUCT pstDrawItem )
{
//	HFONT	hFtOld;

//	LPTSTR	ptMultiStr;

	if( IDS_FRAME_IMAGE != pstDrawItem->CtlID ){	return (INT_PTR)FALSE;	}

//	hFtOld = SelectFont( pstDrawItem->hDC, ghAaFont );	//	フォント設定

	FillRect( pstDrawItem->hDC, &(pstDrawItem->rcItem), GetSysColorBrush( COLOR_WINDOW ) );
	SetBkMode( pstDrawItem->hDC, TRANSPARENT );
//ここから複数行処理すればいいか

//	ptMultiStr = FrameMakeOutsideBoundary( pstDrawItem->rcItem.right, pstDrawItem->rcItem.bottom, &(gstFrameInfo[gNowSel]) );

	DrawText( pstDrawItem->hDC, gptFrmSample, -1, &(pstDrawItem->rcItem), DT_LEFT | DT_NOPREFIX | DT_NOCLIP | DT_WORDBREAK );
							//	ptMultiStr
//	FREE( ptMultiStr );

//	SelectFont( pstDrawItem->hDC, hFtOld );

	return (INT_PTR)TRUE;
}
//-------------------------------------------------------------------------------------------------

/*
描画幅は決まっている。枠指定なら外から、範囲指定なら、文字列＋左右の幅がＭＡＸ幅

左上パーツと右上パーツの幅を確認して、残りが上パーツ使う。使う個数は幅から算出
占有行数が異なるなら、下側を合わせる。右側は途中で切る事も考慮


床部分も処理は同じ。占有行数異なるなら、上側を合わせる。

柱は、必要行数確認する。複数行になるなら、途中で切る。
左柱は０基点、右柱は、右上右下パーツの左に合わせる＋オフセット

*/

/*!
	渡されたパーツから、必要なところを抜き出して文字列作る
	@param[in]	bEnable	行が有効範囲であるかどうかの判断
	@param[in]	pstItem	パーツ情報入ったやつ
	@param[out]	ptDest	作った文字列を入れるバッファへのポインター
	@param[in]	cchSz	バッファの文字数・バイトに非ず
	@return	
*/
UINT FrameMakeMultiSubLine( CONST BOOLEAN bEnable, LPFRAMEITEM pstItem, LPTSTR ptDest, CONST UINT_PTR cchSz )
{
	LPTSTR	ptBufStr;

	if( bEnable )	//	有効であるか
	{
		//	マルチ行の一部をブッコ抜く
		FrameMultiSubstring( pstItem->atParts, pstItem->iNowLn, ptDest, cchSz, pstItem->dDot );
		//最大幅に満たない行は、Paddingする
		pstItem->iNowLn++;
	}
	else	//	空白である
	{
		ptBufStr = DocPaddingSpaceWithPeriod( pstItem->dDot, NULL, NULL, NULL, TRUE );//DocPaddingSpaceMake( pstItem->dDot );
		StringCchCopy( ptDest, cchSz, ptBufStr );
		FREE( ptBufStr );
	}

	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	枠パーツの、天井と床の占有行数と、左柱のドット数を確保する
*/
INT FrameMultiSizeGet( LPFRAMEINFO pstInfo, PINT piUpLine, PINT piDnLine )
{
	INT	iUpLine, iDnLine;

	//	行数の確認
	iUpLine = pstInfo->stMorning.iLine;	//	左上
	if( iUpLine < pstInfo->stNoon.iLine )	iUpLine = pstInfo->stNoon.iLine;	//	上
	if( iUpLine < pstInfo->stAfternoon.iLine )	iUpLine = pstInfo->stAfternoon.iLine;	//	右上

	iDnLine = pstInfo->stDawn.iLine;	//	左下
	if( iDnLine < pstInfo->stMidnight.iLine )	iDnLine = pstInfo->stMidnight.iLine;	//	下
	if( iDnLine < pstInfo->stTwilight.iLine )	iDnLine = pstInfo->stTwilight.iLine;	//	右下

	if( piUpLine )	*piUpLine = iUpLine;
	if( piDnLine )	*piDnLine = iDnLine;

	return pstInfo->stDaybreak.dDot;
}
//-------------------------------------------------------------------------------------------------

/*!
	文字列を受けて、前パディングして、幅ぴったりになるようにパディングしたり切ったりする
	@param[in]		iFwOffs	前追加幅・０で無視
	@param[in,out]	ptStr	文字列・加工して戻す
	@param[in]		cchSz	文字列の文字数
	@param[in]		iMaxDot	全体のドット数・０で無視
	@return	
*/
UINT StringWidthAdjust( CONST UINT iFwOffs, LPTSTR ptStr, CONST UINT_PTR cchSz, CONST INT iMaxDot )
{
	INT			iDot, iPadd;
	INT			iDotCnt, iBuf;
	UINT_PTR	dm, dMozi;
	TCHAR		atWork[MAX_PATH];
	LPTSTR		ptBufStr;

	ZeroMemory( atWork, sizeof(atWork) );

	if( 1 <= iFwOffs )
	{
		ptBufStr = DocPaddingSpaceWithPeriod( iFwOffs, NULL, NULL, NULL, TRUE );//DocPaddingSpaceMake( iFwOffs );
		if( ptBufStr )
		{
			StringCchCopy( atWork, MAX_PATH, ptBufStr );
			FREE( ptBufStr );
		}
	}
	StringCchCat( atWork, MAX_PATH, ptStr );

	iDot = ViewStringWidthGet( atWork );

	if( (0 != iMaxDot) && (iDot != iMaxDot) )	//	０ではなく、丁度でも無い場合
	{
		if( iDot < iMaxDot )	//	指定幅のほうが広いならパディングー
		{
			iPadd = iMaxDot - iDot;
			ptBufStr = DocPaddingSpaceWithPeriod( iPadd, NULL, NULL, NULL, TRUE );//DocPaddingSpaceMake( iPadd );
			if( ptBufStr )
			{
				StringCchCat( atWork, MAX_PATH, ptBufStr );
				FREE( ptBufStr );
			}
		}
		else	//	そうでないならぶった切る
		{
			StringCchLength( atWork, MAX_PATH, &dMozi );
			iDotCnt = 0;	//	長さ確認していく
			for( dm = 0; dMozi > dm; dm++ )
			{
				iBuf = ViewLetterWidthGet( atWork[dm] );
				if( iMaxDot < (iDotCnt+iBuf) )
				{
					atWork[dm] = NULL;	//	一旦文字列閉じる
					iBuf = iMaxDot - iDotCnt;
					if( 0 < iBuf )
					{
						ptBufStr = DocPaddingSpaceWithPeriod( iBuf, NULL, NULL, NULL, TRUE );//DocPaddingSpaceMake( iBuf );
						if( ptBufStr )
						{
							StringCchCat( atWork, MAX_PATH, ptBufStr );
							FREE( ptBufStr );
						}
					}
					break;
				}
				iDotCnt += iBuf;
			}
		}

		iDot = ViewStringWidthGet( atWork );
	}

	StringCchCopy( ptStr, cchSz, atWork );

	return iDot;
}
//-------------------------------------------------------------------------------------------------

/*!
	外枠に合わせて、複数行枠を作る
	@param[in]	iWidth	外枠幅ドット数
	@param[in]	iHeight	外枠高ドット数
	@param[in]	pstInfo	使う枠の情報
	@return	確保した文字列を返す・freeに注意・失敗ならNULL
*/
LPTSTR FrameMakeOutsideBoundary( CONST INT iWidth, CONST INT iHeight, LPFRAMEINFO pstInfo )
{
	LPFRAMEITEM	pstItem;

	TCHAR		atSubStr[MAX_PATH];
	LPTSTR		ptBufStr;
	INT	iLines, i;	//	全体行数
	INT	iUpLine, iMdLine, iDnLine;	//	天井、柱、床の占有行

	INT	iRitOccup;
	INT	iOfsLine, iRight;
	INT	iRoofDot, iFloorDot;
	INT	iRoofCnt, iFloorCnt;	//	上と下のパーツの個数
	INT	iRfRstDot, iFlRstDot;
	INT	iRitOff;//, iPillarDot, iFloodDot;
	INT	iRitBuf;
	INT	iRealWid;
	INT	ic;
	INT	iTgtLn;	//	各部のフォーカス行数
	UINT	bMultiPadd;
	BOOLEAN	bEnable;

	UINT_PTR	cchTotal, dCount, d;

	HRESULT	hRslt;

	wstring	wsWorkStr;
	vector<wstring>	vcString;	//	作業用

#ifdef DO_TRY_CATCH
	try{
#endif

	wsWorkStr.assign( TEXT("") );

	iLines =  iHeight / LINE_HEIGHT;	//	切り捨てでおｋ
	TRACE( TEXT("MF LINE %d"), iLines );

	bMultiPadd = pstInfo->dRestPadd;	//	パディングするかどうか

	//	行数の確認
	FrameMultiSizeGet( pstInfo, &iUpLine, &iDnLine );

	iMdLine = iLines - (iUpLine + iDnLine);	//	柱
	TRACE( TEXT("MF R[%d] P[%d] F[%d]"), iUpLine, iMdLine, iDnLine );
	//	もし iMdLine がマイナスになったら？　中が作られなくなるだけ
	if( 0 > iMdLine ){	iLines -= iMdLine;	iMdLine = 0;	}	//	符号に注意セヨ
	//	はみ出すとバグるので、はみ出した分は無かったことにする

	for( i = 0; iLines > i; i++ )
	{
		vcString.push_back( wsWorkStr );	//	先に確保
	}



	iRealWid = iWidth;

	//	右パーツの占有幅、一番長いのを確認
	iRitOccup = pstInfo->stAfternoon.dDot;	//	右上
	if( iRitOccup <  pstInfo->stTwilight.dDot ){	iRitOccup = pstInfo->stTwilight.dDot;	}	//	右下
	iRitBuf = pstInfo->dRightOffset + pstInfo->stNightfall.dDot;	//	右とオフセット
	if( iRitOccup < iRitBuf ){	iRitOccup = iRitBuf;	}	//	右
	//	iRitOccupは、右パーツの最大占有幅である
	iRitOff = iWidth - iRitOccup;	//	右パーツ開始位置を確定

	//	天井に使えるドット数をゲット
		//iRitOff = iWidth - pstInfo->stAfternoon.dDot;	//	右上の占有分
	iRoofDot  = iRitOff - pstInfo->stMorning.dDot;	//	天井に使えるドット幅
	if( 1 <= pstInfo->dLeftOffset ){	iRoofDot -= pstInfo->dLeftOffset;	}
	iRoofCnt  = Divinus( iRoofDot, pstInfo->stNoon.dDot );	//	占有ドットを確認して、個数出す。切り捨てでおｋ
	iRfRstDot = iRoofDot - (iRoofCnt * pstInfo->stNoon.dDot);

//零除算発生

	//	左下と右下と床
		//iRitOff = iWidth - pstInfo->stTwilight.dDot;	//	右下の占有分
	iFloorDot = iRitOff - pstInfo->stDawn.dDot;	//	床に使えるドット幅
	if( 1 <= pstInfo->dLeftOffset ){	iFloorDot -= pstInfo->dLeftOffset;	}
	iFloorCnt = Divinus( iFloorDot , pstInfo->stMidnight.dDot );	//	占有ドットを確認して、個数出す。
	iFlRstDot = iFloorDot - (iFloorCnt * pstInfo->stMidnight.dDot);

	//	右柱開始位置・パディングを考慮セヨ
	if( bMultiPadd )
	{
		iRight = iRitOff + pstInfo->dRightOffset;
		//	負のオフセットなら、内側にめり込むので、その分端までの長さを縮める
		if( -1 >= pstInfo->dLeftOffset ){	iRight +=  pstInfo->dLeftOffset;	}
		//	負数の扱いに注意
	}
	else
	{
		iRight = (iRoofCnt * pstInfo->stNoon.dDot) + pstInfo->stMorning.dDot + pstInfo->dRightOffset;
		iRitBuf = (iFloorCnt * pstInfo->stMidnight.dDot) + pstInfo->stDawn.dDot + pstInfo->dRightOffset;
		if( iRight < iRitBuf ){	iRight = iRitBuf;	};	//	長い方に合わせる

		iRight +=  pstInfo->dLeftOffset;
	}

	//	枠合わせ拡大モードのときはオフセットは考慮しない

	TRACE( TEXT("MF RD[%d]C[%d][%d] FD[%d]C[%d][%d]"), iRoofDot, iRoofCnt, iRfRstDot, iFloorDot, iFloorCnt, iFlRstDot );

	//	作業に向けてクルヤー
	pstInfo->stDaybreak.iNowLn  = 0;
	pstInfo->stMorning.iNowLn   = 0;
	pstInfo->stNoon.iNowLn      = 0;
	pstInfo->stAfternoon.iNowLn = 0;
	pstInfo->stNightfall.iNowLn = 0;
	pstInfo->stTwilight.iNowLn  = 0;
	pstInfo->stMidnight.iNowLn  = 0;
	pstInfo->stDawn.iNowLn      = 0;

	//	天井から・下端を合わせる
	for( iTgtLn = 0, iOfsLine = 0; iUpLine > iTgtLn; iTgtLn++, iOfsLine++ )
	{
		//	左上
		if( 0 >= (iUpLine - iTgtLn) - pstInfo->stMorning.iLine )	bEnable = TRUE;
		else	bEnable = FALSE;
		FrameMakeMultiSubLine( bEnable, &(pstInfo->stMorning), atSubStr, MAX_PATH );
		if( 1 <= pstInfo->dLeftOffset ){	StringWidthAdjust( pstInfo->dLeftOffset, atSubStr, MAX_PATH, 0 );	}
		vcString.at( iOfsLine ).append( atSubStr );

		//	上
		if( 0 >= (iUpLine - iTgtLn) - pstInfo->stNoon.iLine )	bEnable = TRUE;
		else	bEnable = FALSE;
		FrameMakeMultiSubLine( bEnable, &(pstInfo->stNoon), atSubStr, MAX_PATH );
		//	必要個数繰り返す
		for( ic = 0; iRoofCnt >  ic; ic++ ){	vcString.at( iOfsLine ).append( atSubStr  );	}
		//	余ってるドット埋める
		if( (1 <= iRfRstDot) && bMultiPadd )
		{
			StringWidthAdjust( 0, atSubStr, MAX_PATH, iRfRstDot );
			vcString.at( iOfsLine ).append( atSubStr );
		}

		//	右上
		if( 0 >= (iUpLine - iTgtLn) - pstInfo->stAfternoon.iLine )	bEnable = TRUE;
		else	bEnable = FALSE;
		FrameMakeMultiSubLine( bEnable, &(pstInfo->stAfternoon), atSubStr, MAX_PATH );
		vcString.at( iOfsLine ).append( atSubStr );
	}

	//	柱おいていく
	for( iTgtLn = 0; iMdLine > iTgtLn; iTgtLn++, iOfsLine++ )
	{
		//	左
		pstItem = &(pstInfo->stDaybreak);		//	右開始位置までパディングしておく
		FrameMultiSubstring( pstItem->atParts, pstItem->iNowLn, atSubStr, MAX_PATH, iRight );
		//	負のパディングをセット
		if( -1 >= pstInfo->dLeftOffset ){	StringWidthAdjust( -(pstInfo->dLeftOffset), atSubStr, MAX_PATH, 0 );	}
		pstItem->iNowLn++;	//	壱行ずつループさせていく
		if( pstItem->iLine <= pstItem->iNowLn ){	pstItem->iNowLn = 0;	}
		vcString.at( iOfsLine ).append( atSubStr );

		//	右
		pstItem = &(pstInfo->stNightfall);		//	末端なのでパディングは不要
		FrameMultiSubstring( pstItem->atParts, pstItem->iNowLn, atSubStr, MAX_PATH, 0 );
		pstItem->iNowLn++;	//	壱行ずつループさせていく
		if( pstItem->iLine <= pstItem->iNowLn ){	pstItem->iNowLn = 0;	}
		vcString.at( iOfsLine ).append( atSubStr );
	}

	//	床・上端を合わせる
	for( iTgtLn = 0; iDnLine > iTgtLn; iTgtLn++, iOfsLine++ )
	{
		//	左下
		if( iTgtLn < pstInfo->stDawn.iLine )	bEnable = TRUE;
		else	bEnable = FALSE;
		FrameMakeMultiSubLine( bEnable, &(pstInfo->stDawn), atSubStr, MAX_PATH );
		if( 1 <= pstInfo->dLeftOffset ){	StringWidthAdjust( pstInfo->dLeftOffset, atSubStr, MAX_PATH, 0 );	}
		vcString.at( iOfsLine ).append( atSubStr );

		//	下
		if( iTgtLn < pstInfo->stMidnight.iLine )	bEnable = TRUE;
		else	bEnable = FALSE;
		FrameMakeMultiSubLine( bEnable, &(pstInfo->stMidnight), atSubStr, MAX_PATH );
		//	必要個数繰り返す
		for( ic = 0; iFloorCnt > ic; ic++ ){	vcString.at( iOfsLine ).append( atSubStr );	}
		//	余ってるドット埋める
		if( (1 <= iFlRstDot) && bMultiPadd )
		{
			StringWidthAdjust( 0, atSubStr, MAX_PATH, iFlRstDot );
			vcString.at( iOfsLine ).append( atSubStr );
		}

		//	右下
		if( iTgtLn < pstInfo->stTwilight.iLine )
		{
			FrameMultiSubstring( pstInfo->stTwilight.atParts, pstInfo->stTwilight.iNowLn, atSubStr, MAX_PATH, 0 );
			pstInfo->stTwilight.iNowLn++;	//	壱行ずつループさせていく
			vcString.at( iOfsLine ).append( atSubStr );
		}
	}

	cchTotal = 0;
	dCount = vcString.size();	//	有効行数
	//	全体の文字数を確保
	for( d = 0; dCount > d; d++ ){	cchTotal += vcString.at( d ).size();	}
	cchTotal += (dCount * sizeof(TCHAR));	//改行分＋余裕
	ptBufStr = (LPTSTR)malloc( cchTotal * sizeof(TCHAR) );	//	サイズ作って
	if( ptBufStr )	//	チェック
	{
		ZeroMemory( ptBufStr, cchTotal * sizeof(TCHAR) );
		for( d = 0; dCount > d; d++ )	//	全部コピー
		{
			if( 0 != d )	hRslt = StringCchCat( ptBufStr, cchTotal, TEXT("\r\n") );
			hRslt = StringCchCat( ptBufStr, cchTotal, vcString.at( d ).c_str() );
		}
	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (LPTSTR)ETC_MSG( err.what( ), NULL );	}
	catch( ... ){	return (LPTSTR)ETC_MSG( ("etc error"), NULL );	}
#endif

	return ptBufStr;
}
//-------------------------------------------------------------------------------------------------

/*!
	内枠に合わせて、複数行枠を作る
	@param[in]		dType	０初期化　１天井　２床
	@param[in,out]	piValue	処理によって受け渡しする
	@param[in]		pstInfo	使う枠の情報
	@return	確保した文字列を返す・freeに注意・失敗ならNULL
*/
LPTSTR FrameMakeInsideBoundary( UINT dType, PINT piValue, LPFRAMEINFO pstInfo )
{
	static INT	iRoofCnt, iFloorCnt;	//	上と下のパーツの個数
	static INT	iRfRstDot, iFlRstDot;

	TCHAR		atSubStr[MAX_PATH];
	LPTSTR		ptBufStr;

	UINT	bMultiPadd;
	INT	iUpLine, iDnLine;
	INT	i, ic;
	INT	iRitOff;
	INT	iRitBuf, iRitVle;
	INT	iRoofDot, iFloorDot;
	INT	iTgtLn;	//	各部のフォーカス行数
	INT	iOfsLine;
	BOOLEAN	bEnable;

	UINT_PTR	cchTotal, dCount, d;

	HRESULT	hRslt;

	wstring	wsWorkStr;
	vector<wstring>	vcString;	//	作業用

#ifdef DO_TRY_CATCH
	try{
#endif

	wsWorkStr.assign( TEXT("") );

	bMultiPadd = pstInfo->dRestPadd;	//	パディングするかどうか

	if( 0 == dType )	//	初期化してstaticで持っておくか
	{
		//	piValue[in]内部のドット数　[out]右柱開始位置

		//	右柱開始位置を確定
		iRitOff = pstInfo->stDaybreak.dDot + *piValue;
		//	左オフセットマイナスなら、左柱が内側にめり込む・マイナス計算に注意
		if( -1 >= pstInfo->dLeftOffset ){	iRitOff +=  -(pstInfo->dLeftOffset);	}
		//	右オフセットマイナスなら、右柱が内側にめり込む
		if( -1 >= pstInfo->dRightOffset ){	iRitOff +=  -(pstInfo->dRightOffset);	}
		//	すなわち、右開始位置がより右に移動する

		//	天井に使えるドット数をゲット
		iRoofDot  = iRitOff - pstInfo->stMorning.dDot;	//	天井に使えるドット幅
		if( 1 <= pstInfo->dLeftOffset ){	iRoofDot -= pstInfo->dLeftOffset;	}
		iRoofCnt  = Divinus( iRoofDot, pstInfo->stNoon.dDot );	//	占有ドットを確認して、個数出す。切り捨てでおｋ
		iRfRstDot = iRoofDot - (iRoofCnt * pstInfo->stNoon.dDot);	//	パーツを入れていったら余るドット数

		//	左下と右下と床
		iFloorDot = iRitOff - pstInfo->stDawn.dDot;	//	床に使えるドット幅
		if( 1 <= pstInfo->dLeftOffset ){	iFloorDot -= pstInfo->dLeftOffset;	}
		iFloorCnt = Divinus( iFloorDot , pstInfo->stMidnight.dDot );	//	占有ドットを確認して、個数出す。
		iFlRstDot = iFloorDot - (iFloorCnt * pstInfo->stMidnight.dDot);	//	パーツを入れていったら余るドット数

		if( !(bMultiPadd) )
		{
			//	パディングしないのなら、余り分はフルに使い、右柱開始位置は、より長い方に合わせる
			if( 0 != iRfRstDot ){	iRoofCnt++;		}
			if( 0 != iFlRstDot ){	iFloorCnt++;	}

			//	はみ出し分確保
			iRitVle = pstInfo->stNoon.dDot - iFlRstDot;
			iRitBuf = pstInfo->stMidnight.dDot - iFlRstDot;
			if( iRitVle < iRitBuf ){	iRitVle = iRitBuf;	}

			iRitOff += iRitVle;
		}

		//	右オフセットブラスなら、右柱開始位置はより右に移動
		if( 1 <= pstInfo->dRightOffset ){	iRitOff += pstInfo->dRightOffset;	}

		*piValue = iRitOff;	//	右開始位置を戻す

		return NULL;
	}
	else if( 1 == dType )	//	天井全体
	{
		FrameMultiSizeGet( pstInfo, &iUpLine, NULL );	//	天井の行数

		//	作業に向けてクルヤー
		pstInfo->stMorning.iNowLn   = 0;
		pstInfo->stNoon.iNowLn      = 0;
		pstInfo->stAfternoon.iNowLn = 0;

		//	先に確保
		for( i = 0; iUpLine > i; i++ ){	vcString.push_back( wsWorkStr );	}

		//	天井から・下端を合わせる
		for( iTgtLn = 0, iOfsLine = 0; iUpLine > iTgtLn; iTgtLn++, iOfsLine++ )
		{
			//	左上
			if( 0 >= (iUpLine - iTgtLn) - pstInfo->stMorning.iLine )	bEnable = TRUE;
			else	bEnable = FALSE;
			FrameMakeMultiSubLine( bEnable, &(pstInfo->stMorning), atSubStr, MAX_PATH );
			if( 1 <= pstInfo->dLeftOffset ){	StringWidthAdjust( pstInfo->dLeftOffset, atSubStr, MAX_PATH, 0 );	}
			vcString.at( iOfsLine ).append( atSubStr );

			//	上
			if( 0 >= (iUpLine - iTgtLn) - pstInfo->stNoon.iLine )	bEnable = TRUE;
			else	bEnable = FALSE;
			FrameMakeMultiSubLine( bEnable, &(pstInfo->stNoon), atSubStr, MAX_PATH );
			//	必要個数繰り返す
			for( ic = 0; iRoofCnt > ic; ic++ ){	vcString.at( iOfsLine ).append( atSubStr );	}
			//	余ってるドット埋める
			if( (1 <= iRfRstDot) && bMultiPadd )
			{
				StringWidthAdjust( 0, atSubStr, MAX_PATH, iRfRstDot );
				vcString.at( iOfsLine ).append( atSubStr );
			}

			//	右上
			if( 0 >= (iUpLine - iTgtLn) - pstInfo->stAfternoon.iLine )	bEnable = TRUE;
			else	bEnable = FALSE;
			FrameMakeMultiSubLine( bEnable, &(pstInfo->stAfternoon), atSubStr, MAX_PATH );
			vcString.at( iOfsLine ).append( atSubStr );
		}
	}
	else if( 2 == dType )	//	床全体
	{
		FrameMultiSizeGet( pstInfo, NULL, &iDnLine );	//	床の行数

		//	作業に向けてクルヤー
		pstInfo->stTwilight.iNowLn  = 0;
		pstInfo->stMidnight.iNowLn  = 0;
		pstInfo->stDawn.iNowLn      = 0;

		//	先に確保
		for( i = 0; iDnLine > i; i++ ){	vcString.push_back( wsWorkStr );	}

		//	床・上端を合わせる
		for( iTgtLn = 0, iOfsLine = 0; iDnLine > iTgtLn; iTgtLn++, iOfsLine++ )
		{
			//	左下
			if( iTgtLn < pstInfo->stDawn.iLine )	bEnable = TRUE;
			else	bEnable = FALSE;
			FrameMakeMultiSubLine( bEnable, &(pstInfo->stDawn), atSubStr, MAX_PATH );
			if( 1 <= pstInfo->dLeftOffset ){	StringWidthAdjust( pstInfo->dLeftOffset, atSubStr, MAX_PATH, 0 );	}
			vcString.at( iOfsLine ).append( atSubStr );

			//	下
			if( iTgtLn < pstInfo->stMidnight.iLine )	bEnable = TRUE;
			else	bEnable = FALSE;
			FrameMakeMultiSubLine( bEnable, &(pstInfo->stMidnight), atSubStr, MAX_PATH );
			//	必要個数繰り返す
			for( ic = 0; iFloorCnt > ic; ic++ ){	vcString.at( iOfsLine ).append( atSubStr );	}
			//	余ってるドット埋める
			if( (1 <= iFlRstDot) && bMultiPadd )
			{
				StringWidthAdjust( 0, atSubStr, MAX_PATH, iFlRstDot );
				vcString.at( iOfsLine ).append( atSubStr );
			}

			//	右下
			if( iTgtLn < pstInfo->stTwilight.iLine )
			{
				FrameMultiSubstring( pstInfo->stTwilight.atParts, pstInfo->stTwilight.iNowLn, atSubStr, MAX_PATH, 0 );
				pstInfo->stTwilight.iNowLn++;	//	壱行ずつループさせていく
				vcString.at( iOfsLine ).append( atSubStr );
			}
		}

	}
	else{	return NULL;	}

	cchTotal = 0;
	dCount = vcString.size();	//	有効行数
	//	全体の文字数を確保
	for( d = 0; dCount > d; d++ ){	cchTotal += vcString.at( d ).size();	}
	cchTotal += ((dCount+1) * sizeof(TCHAR));	//改行分＋余裕
	ptBufStr = (LPTSTR)malloc( cchTotal * sizeof(TCHAR) );	//	サイズ作って
	if( ptBufStr )	//	チェック
	{
		ZeroMemory( ptBufStr, cchTotal * sizeof(TCHAR) );
		for( d = 0; dCount > d; d++ )	//	全部コピー
		{
			hRslt = StringCchCat( ptBufStr, cchTotal, vcString.at( d ).c_str() );
			hRslt = StringCchCat( ptBufStr, cchTotal, TEXT("\r\n") );
			//	改行は常に必要である
		}
	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (LPTSTR)ETC_MSG( err.what( ), NULL );	}
	catch( ... ){	return (LPTSTR)ETC_MSG( ("etc error"), NULL );	}
#endif

	return ptBufStr;
}
//-------------------------------------------------------------------------------------------------

/*!
	ノーティファイメッセージの処理
	@param[in]	hDlg		ダイヤロゲハンドゥ
	@param[in]	idFrom		NOTIFYを発生させたコントロールのＩＤ
	@param[in]	pstNmhdr	NOTIFYの詳細
	@return		処理した内容とか
*/
INT_PTR Frm_OnNotify( HWND hDlg, INT idFrom, LPNMHDR pstNmhdr )
{
	INT	nmCode;
	TCHAR	atBuff[MIN_STRING];
	LPNMUPDOWN	pstUpDown;
	RECT	rect;

	nmCode = pstNmhdr->code;

	//	左押したらデルタが正、右で負

	if( IDSP_LEFT_OFFSET == idFrom )
	{
		pstUpDown = (LPNMUPDOWN)pstNmhdr;
		if( UDN_DELTAPOS == nmCode )
		{
			TRACE( TEXT("%d %d"), pstUpDown->iPos, pstUpDown->iDelta );
			if( 0 < pstUpDown->iDelta ){		gstFrameInfo[gNowSel].dLeftOffset +=  1;	}
			else if( 0 >pstUpDown->iDelta ){	gstFrameInfo[gNowSel].dLeftOffset -=  1;	}

			StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), gstFrameInfo[gNowSel].dLeftOffset );
			Edit_SetText( GetDlgItem(hDlg,IDE_LEFT_OFFSET), atBuff );

			GetClientRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), &rect );
			FREE( gptFrmSample );
			gptFrmSample = FrameMakeOutsideBoundary( rect.right, rect.bottom, &(gstFrameInfo[gNowSel]) );

			InvalidateRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), NULL, TRUE );
		}
		return (INT_PTR)TRUE;
	}

	if( IDSP_RIGHT_OFFSET == idFrom )
	{
		pstUpDown = (LPNMUPDOWN)pstNmhdr;
		if( UDN_DELTAPOS == nmCode )
		{
			TRACE( TEXT("%d %d"), pstUpDown->iPos, pstUpDown->iDelta );
			if( 0 < pstUpDown->iDelta ){		gstFrameInfo[gNowSel].dRightOffset -= 1;	}
			else if( 0 > pstUpDown->iDelta ){	gstFrameInfo[gNowSel].dRightOffset += 1;	}

			StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), gstFrameInfo[gNowSel].dRightOffset );
			Edit_SetText( GetDlgItem(hDlg,IDE_RIGHT_OFFSET), atBuff );

			GetClientRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), &rect );
			FREE( gptFrmSample );
			gptFrmSample = FrameMakeOutsideBoundary( rect.right, rect.bottom, &(gstFrameInfo[gNowSel]) );

			InvalidateRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), NULL, TRUE );
		}
		return (INT_PTR)TRUE;
	}

	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	所定の枠データをINIファイルからロードしてメモリに確保・ドット数も計算
	@param[in]	dNumber		枠番号
	@param[in]	pstFrame	枠データ入れる構造体へのポインタ～
	@return		HRESULT	終了状態コード
*/
HRESULT FrameDataGet( UINT dNumber, LPFRAMEINFO pstFrame )
{
	InitFrameItem( INIT_LOAD, dNumber, pstFrame );

	pstFrame->stDaybreak.iLine  = DocStringInfoCount( pstFrame->stDaybreak.atParts,  0, &(pstFrame->stDaybreak.dDot), NULL );
	pstFrame->stMorning.iLine   = DocStringInfoCount( pstFrame->stMorning.atParts,   0, &(pstFrame->stMorning.dDot), NULL );
	pstFrame->stNoon.iLine      = DocStringInfoCount( pstFrame->stNoon.atParts,      0, &(pstFrame->stNoon.dDot), NULL );
	pstFrame->stAfternoon.iLine = DocStringInfoCount( pstFrame->stAfternoon.atParts, 0, &(pstFrame->stAfternoon.dDot), NULL );
	pstFrame->stNightfall.iLine = DocStringInfoCount( pstFrame->stNightfall.atParts, 0, &(pstFrame->stNightfall.dDot), NULL );
	pstFrame->stTwilight.iLine  = DocStringInfoCount( pstFrame->stTwilight.atParts,  0, &(pstFrame->stTwilight.dDot), NULL );
	pstFrame->stMidnight.iLine  = DocStringInfoCount( pstFrame->stMidnight.atParts,  0, &(pstFrame->stMidnight.dDot), NULL );
	pstFrame->stDawn.iLine      = DocStringInfoCount( pstFrame->stDawn.atParts,      0, &(pstFrame->stDawn.dDot), NULL );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	エディットボックスに設定内容を入れる
	@param[in]	hDlg	ダイヤロゲハンドゥ
	@return		HRESULT	終了状態コード
*/
HRESULT FrameInfoDisp( HWND hDlg )
{
	TCHAR	atBuff[MIN_STRING];

	//	名前表示
	Edit_SetText( GetDlgItem(hDlg,IDE_BOXP_NAME_EDIT), gstFrameInfo[gNowSel].atFrameName );

	//	パーツ情報をいれる
	Edit_SetText( GetDlgItem(hDlg,IDE_BOXP_MORNING),   gstFrameInfo[gNowSel].stMorning.atParts );
	Edit_SetText( GetDlgItem(hDlg,IDE_BOXP_NOON),      gstFrameInfo[gNowSel].stNoon.atParts );
	Edit_SetText( GetDlgItem(hDlg,IDE_BOXP_AFTERNOON), gstFrameInfo[gNowSel].stAfternoon.atParts );
	Edit_SetText( GetDlgItem(hDlg,IDE_BOXP_DAYBREAK),  gstFrameInfo[gNowSel].stDaybreak.atParts );
	Edit_SetText( GetDlgItem(hDlg,IDE_BOXP_NIGHTFALL), gstFrameInfo[gNowSel].stNightfall.atParts );
	Edit_SetText( GetDlgItem(hDlg,IDE_BOXP_TWILIGHT),  gstFrameInfo[gNowSel].stTwilight.atParts );
	Edit_SetText( GetDlgItem(hDlg,IDE_BOXP_MIDNIGHT),  gstFrameInfo[gNowSel].stMidnight.atParts );
	Edit_SetText( GetDlgItem(hDlg,IDE_BOXP_DAWN),      gstFrameInfo[gNowSel].stDawn.atParts );

	StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"),   gstFrameInfo[gNowSel].dLeftOffset );
	Edit_SetText( GetDlgItem(hDlg,IDE_LEFT_OFFSET),    atBuff );

	StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"),   gstFrameInfo[gNowSel].dRightOffset );
	Edit_SetText( GetDlgItem(hDlg,IDE_RIGHT_OFFSET),   atBuff );

	//	天井と床の余り部分埋めるかどうか
	Button_SetCheck( GetDlgItem( hDlg, IDB_FRM_PADDING ), gstFrameInfo[gNowSel].dRestPadd ? BST_CHECKED : BST_UNCHECKED );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	枠を入れる
	@param[in]	dMode	入れたい枠の番号０インデックス
	@param[in]	dStyle	矩形かどうか
	@return		HRESULT	終了状態コード
*/
HRESULT DocFrameInsert( INT dMode, INT dStyle )
{
	UINT_PTR	iLines;
	INT_PTR		iLns, iLast;
	INT			iTop, iBtm, iInX, iEndot, iPadding, i, baseDot;
	INT			xMidLen;
	LPTSTR		ptPadding;
	LPTSTR		ptString;

	FRAMEINFO	stInfo;

	INT			iMidLine, iUpLine, iDnLine;
	LPFRAMEITEM	pstItem;
	TCHAR		atSubStr[MAX_PATH];	//	足りるか？

#ifdef DO_TRY_CATCH
	try{
#endif

	FrameDataGet( dMode, &stInfo );

	iLines = DocNowFilePageLineCount( );	//	ページ全体の行数

	//	開始地点から開始	//	D_SQUARE
	iTop = gitFileIt->vcCont.at( gixFocusPage ).dSelLineTop;
	iBtm = gitFileIt->vcCont.at( gixFocusPage ).dSelLineBottom;
	if( 0 >  iTop ){	iTop = 0;	}
	if( 0 >  iBtm ){	iBtm = iLines - 1;	}

	//	末端を確認・内容がないなら、使用行戻す
	iInX = DocLineParamGet( iBtm, NULL, NULL );
	if( 0 == iInX ){	 iBtm--;	}

	//	矩形選択無しとみなす

	ViewSelPageAll( -1 );	//	範囲とったので解除しておｋ

	//	選択範囲内でもっとも長いドット数を確認
	baseDot = DocPageMaxDotGet( iTop, iBtm );

//天井の行数、左の幅、床の行数を確保

	iMidLine = (iBtm - iTop) + 1;	//	間の行数確保

	xMidLen = baseDot;
	FrameMakeInsideBoundary( 0, &xMidLen, &stInfo );
	//	初期状態を確定する

	//	天井パーツ作成
	ptString = FrameMakeInsideBoundary( 1, &xMidLen, &stInfo );
	FrameMultiSizeGet( &stInfo, &iUpLine, NULL );	//	天井の行数
	iLns = iTop;	//	注目行
	iInX = 0;	//	天井追加
	DocInsertString( &iInX, &iLns, NULL, ptString, 0, TRUE );
	FREE( ptString );

	//	左と右つくる
	stInfo.stDaybreak.iNowLn  = 0;
	stInfo.stNightfall.iNowLn = 0;
	for( i = 0; iMidLine > i; i++, iLns++ )
	{
		//	左
		pstItem = &(stInfo.stDaybreak);
		//	埋めるのはパーツ最大位置まで
		FrameMultiSubstring( pstItem->atParts, pstItem->iNowLn, atSubStr, MAX_PATH, pstItem->dDot );
		//	負のパディングをセット
		if( -1 >= stInfo.dLeftOffset ){	StringWidthAdjust( -(stInfo.dLeftOffset), atSubStr, MAX_PATH, 0 );	}
		pstItem->iNowLn++;	//	壱行ずつループさせていく
		if( pstItem->iLine <= pstItem->iNowLn ){	pstItem->iNowLn = 0;	}
		iInX = 0;	//	左端からいれる
		DocInsertString( &iInX, &iLns, NULL, atSubStr, 0, FALSE );

		//	右
		iEndot = DocLineParamGet( iLns, NULL, NULL );	//	この行の末端
		iPadding = xMidLen - iEndot;	//	埋め量確認
		ptPadding = DocPaddingSpaceWithPeriod( iPadding, NULL, NULL, NULL, TRUE );//DocPaddingSpaceMake( iPadding );
		if( ptPadding )
		{
			DocInsertString( &iEndot, &iLns, NULL, ptPadding, 0, FALSE );
			FREE( ptPadding );
		}
		pstItem = &(stInfo.stNightfall);
		FrameMultiSubstring( pstItem->atParts, pstItem->iNowLn, atSubStr, MAX_PATH, 0 );
		pstItem->iNowLn++;	//	壱行ずつループさせていく
		if( pstItem->iLine <= pstItem->iNowLn ){	pstItem->iNowLn = 0;	}
		DocInsertString( &iEndot, &iLns, NULL, atSubStr, 0, FALSE );
	}

	//	行末がＥＯＦならここでおかしな事になる
	iLast = DocPageParamGet( NULL, NULL );
	if( iLast <= iLns )
	{
		iLns = iLast - 1;
		iInX = DocLineParamGet( iLns, NULL, NULL );
		DocCrLfAdd( iInX , iLns, FALSE );	//	床を入れるために改行
		iLns++;
	}

	//	床作る
	ptString = FrameMakeInsideBoundary( 2, &xMidLen, &stInfo );
	FrameMultiSizeGet( &stInfo, NULL, &iDnLine );	//	床の行数
	iInX = 0;	//	天井追加
	DocInsertString( &iInX, &iLns, NULL, ptString, 0, FALSE );
	FREE( ptString );


#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif
#ifdef DO_TRY_CATCH
	try{
#endif

	//	最終的なキャレットの位置をリセット
	ViewPosResetCaret( iInX , iLns );

	ViewRedrawSetLine( iTop );
	DocBadSpaceCheck( iTop );	//	バッド空白チェキ

	//	改行してるから、これ以降全部再描画必要
	iLns = DocNowFilePageLineCount( );	//	現在行数再認識
	for( i = iTop; iLns > i; i++ )
	{
		DocBadSpaceCheck( i );	//	バッド空白チェキ
		ViewRedrawSetLine(  i );
	}
	//ViewRedrawSetLine( i );	//	念のため
	//DocBadSpaceCheck( i );	//	バッド空白チェキ


	DocPageInfoRenew( -1, 1 );

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif
	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	複数行Frameの、￥￥・￥ｎ・￥ｓ＜＝＞￥・0x0D0A・半角空白の相互変換
	@param[in,out]	ptData	変換元バッファで、変換後文字列入れる。PARTS_CCHサイズであること
	@param[in]		bMode	１：￥ｎを改行にする　０：改行を￥ｎにする
*/
VOID FrameDataTranslate( LPTSTR ptData, UINT bMode )
{
	TCHAR	atBuffer[SUB_STRING];
	UINT_PTR	i, j, cchLen;

	ZeroMemory( atBuffer, sizeof(atBuffer) );

	StringCchLength( ptData, PARTS_CCH, &cchLen );	//	長さ確認

	for( i = 0, j = 0; cchLen > i; i++, j++ )
	{
		if( 0x0000 == ptData[i] )	break;	//	多分意味はないけど安全対策

		if( bMode  )	//	￥ｎを改行にする
		{
			if( 0x005C == ptData[i] )	//	エスケープシーケンス
			{
				i++;	//	次の文字が重要
				if( 'n' == ptData[i] )
				{
					atBuffer[j++] = 0x000D;
					atBuffer[j] = 0x000A;
				}
				else if( 's' == ptData[i] )
				{
					atBuffer[j] = 0x0020;
				}
				else	//	￥￥であった
				{
					atBuffer[j] = ptData[i];
				}
			}
			else	//	関係ないならそのままコピーしていく
			{
				atBuffer[j] = ptData[i];
			}
		}
		else	//	改行を￥ｎにする
		{
			if( 0x005C == ptData[i] )	//	￥記号
			{
				atBuffer[j++] = 0x005C;
				atBuffer[j] = 0x005C;	//	重ねる
			}
			else if( 0x000D == ptData[i] )	//	改行はいった
			{
				atBuffer[j++] = 0x005C;
				atBuffer[j] = TEXT('n');	//	エスケープシーケンス
				i++;	//	次に進める
			}
			else if( 0x0020 == ptData[i] )	//	半角空白はいった
			{
				atBuffer[j++] = 0x005C;
				atBuffer[j] = TEXT('s');	//	エスケープシーケンス
			}
			else	//	関係ないならそのままコピーしていく
			{
				atBuffer[j] = ptData[i];
			}
		}
	}

	StringCchCopy( ptData, PARTS_CCH, atBuffer );	//	書き戻す

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	改行を含む文字列を受け取って、指定行の内容をバッファに入れる
	@param[in]	ptSrc	元文字列
	@param[in]	dLine	切り出す行番号・０インデックス
	@param[out]	ptDest	切り出した文字列を入れるバッファへのポインター
	@param[in]	cchSz	バッファの文字数・バイトに非ず
	@param[in]	iUseDot	必要とするドット数・足りないならパディング・多いなら放置
	@return	UINT	全体の行数
*/
UINT FrameMultiSubstring( LPCTSTR ptSrc, CONST UINT dLine, LPTSTR ptDest, CONST UINT_PTR cchSz, CONST INT iUseDot )
{
	LPTSTR		ptPadding;
	INT			iPaDot, iStrDot;
	UINT_PTR	cchSrc, c, d;
	UINT		iLnCnt;

	StringCchLength( ptSrc, STRSAFE_MAX_CCH , &cchSrc );	//	元文字列の長さ確認

	ZeroMemory( ptDest, cchSz * sizeof(TCHAR) );	//	とりあえずウケを浄化

	iLnCnt = 0;	d = 0;
	for( c = 0; cchSrc > c; c++ )
	{
		if( 0x000D == ptSrc[c] )	//	かいぎょうはっけん
		{
			c++;	//	0x0Aを飛ばす
			iLnCnt++;	//	フォーカス行数
		}
		else	//	普通の文字
		{
			if( dLine == iLnCnt )	//	行が一致したら
			{
				if( cchSz > d ){	ptDest[d] = ptSrc[c];	d++;	}
			}
		}
	}
	ptDest[(cchSz-1)] = NULL;	//	ヌルターミネータ

//	StringCchLength( ptDest, cchSz, &cchSrc );	//	ブッコ抜いた文字列の長さ
	iStrDot = ViewStringWidthGet( ptDest );	//	ブッコ抜いた文字列のドット長
	//	パディングもしちゃう
	iPaDot = iUseDot - iStrDot;
	if( 1 <= iPaDot )
	{
		ptPadding = DocPaddingSpaceWithPeriod( iPaDot, NULL, NULL, NULL, TRUE );//DocPaddingSpaceMake( iPaDot );
		StringCchCat( ptDest, cchSz, ptPadding );
		FREE( ptPadding );
	}

	iLnCnt++;	//	０インデックスなので１増やすのが正解
	return iLnCnt;
}
//-------------------------------------------------------------------------------------------------

//挿入ウインドウについて

/*!
	挿入用ウインドウ作る
	@param[in]	hInst	実存ハンドル
	@param[in]	hPrWnd	メインのウインドウハンドル
*/
HWND FrameInsBoxCreate( HINSTANCE hInst, HWND hPrWnd )
{
	INT			x, y;
	UINT		d;
	TCHAR		atBuffer[MAX_STRING];
	RECT		rect, vwRect;
//	TBADDBITMAP	stToolBmp;

	RECT	stFrmRct;
	INT		topOst;

	if( ghFrInbxWnd )
	{
		SetForegroundWindow( ghFrInbxWnd );
		return ghFrInbxWnd;
	}

	//	本体ウインドウ
	ghFrInbxWnd = CreateWindowEx( WS_EX_LAYERED | WS_EX_TOOLWINDOW,
		FRAMEINSERTBOX_CLASS, TEXT("枠挿入ボックス"),
		WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU,
		0, 0, FIB_WIDTH, FIB_HEIGHT, NULL, NULL, hInst, NULL );
	SetLayeredWindowAttributes( ghFrInbxWnd, TRANCE_COLOUR, 0xFF, LWA_COLORKEY );
										//	TRANCE_COLOUR

	//	ツールバー
	ghFIBtlbrWnd = CreateWindowEx( WS_EX_CLIENTEDGE, TOOLBARCLASSNAME, TEXT("fibtoolbar"),
		WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS,
		0, 0, 0, 0, ghFrInbxWnd, (HMENU)IDTB_FRMINSBOX_TOOLBAR, hInst, NULL );

	if( 0 == gdToolBarHei )	//	数値未取得なら
	{
		GetWindowRect( ghFIBtlbrWnd, &rect );
		gdToolBarHei = rect.bottom - rect.top;

		gstFrmSz.x = 0;
		gstFrmSz.y = gdToolBarHei;
		ClientToScreen( ghFrInbxWnd, &gstFrmSz );
	}

	//	自動ツールチップスタイルを追加
	SendMessage( ghFIBtlbrWnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS );

	//	アイコン
	SendMessage( ghFIBtlbrWnd, TB_SETIMAGELIST, 0, (LPARAM)ghFrameImgLst );
	SendMessage( ghFIBtlbrWnd, TB_SETBUTTONSIZE, 0, MAKELPARAM(16,16) );
	SendMessage( ghFIBtlbrWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );

	//	ツールチップ文字列を設定・ボタンテキストがツールチップになる
	StringCchCopy( atBuffer, MAX_STRING, TEXT("挿入する") );
	gstFIBTBInfo[0].iString = SendMessage( ghFIBtlbrWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	for( d = 0; FRAME_MAX > d; d++ )
	{
		FrameNameLoad( d, atBuffer, MAX_STRING );
		gstFIBTBInfo[d+2].iString = SendMessage( ghFIBtlbrWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	}
	StringCchCopy( atBuffer, MAX_STRING, TEXT("挿入したら閉じる") );
	gstFIBTBInfo[23].iString = SendMessage( ghFIBtlbrWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	StringCchCopy( atBuffer, MAX_STRING, TEXT("上下の端数を埋める\r\n（ここでの変更は保存されません）") );
	gstFIBTBInfo[25].iString = SendMessage( ghFIBtlbrWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );


	SendMessage( ghFIBtlbrWnd , TB_ADDBUTTONS, (WPARAM)TB_ITEMS, (LPARAM)&gstFIBTBInfo );	//	ツールバーにボタンを挿入

	SendMessage( ghFIBtlbrWnd , TB_AUTOSIZE, 0, 0 );	//	ボタンのサイズに合わせてツールバーをリサイズ
	InvalidateRect( ghFIBtlbrWnd , NULL, TRUE );		//	クライアント全体を再描画する命令

	//	初期状態としてアルファをチェキ状態に
	SendMessage( ghFIBtlbrWnd, TB_CHECKBUTTON, IDM_INSFRAME_ALPHA, TRUE );
	gdSelect = 0;

	//	直ぐ閉じるかどうか
	SendMessage( ghFIBtlbrWnd, TB_CHECKBUTTON, IDM_FRMINSBOX_QCLOSE, gbQuickClose );

	FrameDataGet( gdSelect , &gstNowFrameInfo );	//	枠パーツ情報確保

	//	埋めるかどうか
	SendMessage( ghFIBtlbrWnd, TB_CHECKBUTTON, IDM_FRMINSBOX_PADDING, gstNowFrameInfo.dRestPadd );
	gbMultiPaddTemp = gstNowFrameInfo.dRestPadd;	//	

	topOst = FrameInsBoxSizeGet( &stFrmRct );	//	FRAME当てはめ枠のサイズ
	gptFrmBox = FrameMakeOutsideBoundary( stFrmRct.right, stFrmRct.bottom, &gstNowFrameInfo );

	//	ウインドウ位置を確定させる
	GetWindowRect( ghViewWnd, &vwRect );
	gstViewOrigin.x = vwRect.left;//位置記録・そうそう変わるものじゃない
	gstViewOrigin.y = vwRect.top;
	x = (vwRect.left + LINENUM_WID) - gstFrmSz.x;
	y = (vwRect.top  + RULER_AREA)  - gstFrmSz.y;
	TRACE( TEXT("Frame %d x %d"), x, y );

#ifdef _DEBUG
	SetWindowPos( ghFrInbxWnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW );
#else
	SetWindowPos( ghFrInbxWnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW );
#endif
	gstOffset.x = x - vwRect.left;
	gstOffset.y = y - vwRect.top;


	return ghFrInbxWnd;
}
//-------------------------------------------------------------------------------------------------

/*!
	フレームサイズを確保
	@param[out]	pstRect	フレームサイズを入れるバッファ
	@return		高さオフセット（ツールバーの高さ）
*/
INT FrameInsBoxSizeGet( LPRECT pstRect )
{
	RECT	rect;

	GetClientRect( ghFrInbxWnd, &rect );

	//	クライヤント高さから、ツールバー高さをヌく
	rect.bottom -= gdToolBarHei;

	*pstRect = rect;

	return gdToolBarHei;
}
//-------------------------------------------------------------------------------------------------

/*!
	挿入実行
	@param[in]	hWnd	ウインドウハンドル
*/
HRESULT FrameInsBoxDoInsert( HWND hWnd )
{
	INT			iX, iY;
	HWND		hLyrWnd;
	RECT		rect;

	//	挿入処理には、レイヤボックスを非表示処理で使う
	hLyrWnd = LayerBoxVisibalise( GetModuleHandle(NULL), gptFrmBox, 0x10 );

	//	レイヤの位置を変更
	GetWindowRect( hWnd, &rect );
	LayerBoxPositionChange( hLyrWnd, (rect.left + gstFrmSz.x), (rect.top + gstFrmSz.y) );
	//	空白を全部透過指定にする
	LayerTransparentToggle( hLyrWnd, 1 );
	//	上書きする
	LayerContentsImportable( hLyrWnd, IDM_LYB_OVERRIDE, &iX, &iY, D_INVISI );
	ViewPosResetCaret( iX, iY );	
	//	終わったら閉じる
	DestroyWindow( hLyrWnd );

	if( gbQuickClose )	DestroyWindow( hWnd );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ウインドウプロシージャ
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	message	ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@retval 0	メッセージ処理済み
	@retval no0	ここでは処理せず次に回す
*/
LRESULT CALLBACK FrameInsProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_PAINT,		Fib_OnPaint );		//	画面の更新とか
		HANDLE_MSG( hWnd, WM_KEYDOWN,	Fib_OnKey );
		HANDLE_MSG( hWnd, WM_COMMAND,	Fib_OnCommand );	
		HANDLE_MSG( hWnd, WM_DESTROY,	Fib_OnDestroy );	//	終了時の処理
		HANDLE_MSG( hWnd, WM_WINDOWPOSCHANGING, Fib_OnWindowPosChanging );
		HANDLE_MSG( hWnd, WM_WINDOWPOSCHANGED,  Fib_OnWindowPosChanged );

		case WM_MOVING:	Fib_OnMoving( hWnd, (LPRECT)lParam );	return 0;

		default:	break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------

/*!
	COMMANDメッセージの受け取り。ボタン押されたとかで発生
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	id			メッセージを発生させた子ウインドウの識別子	LOWORD(wParam)
	@param[in]	hWndCtl		メッセージを発生させた子ウインドウのハンドル	lParam
	@param[in]	codeNotify	通知メッセージ	HIWORD(wParam)
	@return		なし
*/
VOID Fib_OnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify )
{
	RECT	stFrmRct;
	INT		topOst, iRslt = -1;

	switch( id )
	{
		case IDM_FRAME_INS_DECIDE:	FrameInsBoxDoInsert( hWnd );	return;

		case IDM_INSFRAME_ALPHA:	gdSelect = 0;	break;
		case IDM_INSFRAME_BRAVO:	gdSelect = 1;	break;
		case IDM_INSFRAME_CHARLIE:	gdSelect = 2;	break;
		case IDM_INSFRAME_DELTA:	gdSelect = 3;	break;
		case IDM_INSFRAME_ECHO:		gdSelect = 4;	break;
		case IDM_INSFRAME_FOXTROT:	gdSelect = 5;	break;
		case IDM_INSFRAME_GOLF:		gdSelect = 6;	break;
		case IDM_INSFRAME_HOTEL:	gdSelect = 7;	break;
		case IDM_INSFRAME_INDIA:	gdSelect = 8;	break;
		case IDM_INSFRAME_JULIETTE:	gdSelect = 9;	break;

		case IDM_INSFRAME_KILO:		gdSelect = 10;	break;
		case IDM_INSFRAME_LIMA:		gdSelect = 11;	break;
		case IDM_INSFRAME_MIKE:		gdSelect = 12;	break;
		case IDM_INSFRAME_NOVEMBER:	gdSelect = 13;	break;
		case IDM_INSFRAME_OSCAR:	gdSelect = 14;	break;
		case IDM_INSFRAME_PAPA:		gdSelect = 15;	break;
		case IDM_INSFRAME_QUEBEC:	gdSelect = 16;	break;
		case IDM_INSFRAME_ROMEO:	gdSelect = 17;	break;
		case IDM_INSFRAME_SIERRA:	gdSelect = 18;	break;
		case IDM_INSFRAME_TANGO:	gdSelect = 19;	break;

		case IDM_FRMINSBOX_QCLOSE:	gbQuickClose = SendMessage( ghFIBtlbrWnd, TB_ISBUTTONCHECKED, IDM_FRMINSBOX_QCLOSE, 0 );	return;

		case IDM_FRMINSBOX_PADDING:	iRslt = SendMessage( ghFIBtlbrWnd, TB_ISBUTTONCHECKED, IDM_FRMINSBOX_PADDING, 0 );	break;

		default:	return;
	}

	FrameDataGet( gdSelect, &gstNowFrameInfo );	//	枠パーツ情報確保

	if( 0 <= iRslt ){	gstNowFrameInfo.dRestPadd = iRslt;	gbMultiPaddTemp = iRslt;	}
	else
	{
		gbMultiPaddTemp = gstNowFrameInfo.dRestPadd;
		SendMessage( ghFIBtlbrWnd, TB_CHECKBUTTON, IDM_FRMINSBOX_PADDING, gstNowFrameInfo.dRestPadd );
	}

	topOst = FrameInsBoxSizeGet( &stFrmRct );	//	FRAME当てはめ枠のサイズ
	FREE( gptFrmBox );
	gptFrmBox = FrameMakeOutsideBoundary( stFrmRct.right, stFrmRct.bottom, &gstNowFrameInfo );

	InvalidateRect( hWnd, NULL, TRUE );

	return;
}
//-------------------------------------------------------------------------------------------------


/*!
	キーダウンが発生・キーボードで移動用
	@param[in]	hWnd	ウインドウハンドル・ビューのとは限らないので注意セヨ
	@param[in]	vk		押されたキーが仮想キーコードで来る
	@param[in]	fDown	非０ダウン　０アップ
	@param[in]	cRepeat	連続オサレ回数・取れてない？
	@param[in]	flags	キーフラグいろいろ
	@return		無し
*/
VOID Fib_OnKey( HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags )
{
	RECT	rect;

	GetWindowRect( hWnd, &rect );

	if( fDown )
	{
		switch( vk )
		{
			case VK_RIGHT:	TRACE( TEXT("右") );	rect.left++;	break;
			case VK_LEFT:	TRACE( TEXT("左") );	rect.left--;	break;
			case VK_DOWN:	TRACE( TEXT("下") );	rect.top += LINE_HEIGHT;	break;
			case  VK_UP:	TRACE( TEXT("上") );	rect.top -= LINE_HEIGHT;	break;
			default:	return;
		}
	}

	SetWindowPos( hWnd, HWND_TOP, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
	Fib_OnMoving( hWnd, &rect );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	PAINT。無効領域が出来たときに発生。背景の扱いに注意。背景を塗りつぶしてから、オブジェクトを描画
	@param[in]	hWnd	親ウインドウのハンドル
*/
VOID Fib_OnPaint( HWND hWnd )
{
	PAINTSTRUCT	ps;
	HDC			hdc;

	RECT	rect;

	GetClientRect( hWnd, &rect );

	hdc = BeginPaint( hWnd, &ps );

	FillRect( hdc, &rect, ghBgBrush );

	//	文字列再描画
	FrameInsBoxFrmDraw( hdc );

	EndPaint( hWnd, &ps );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	描画
	@param[in]	hDC	描画するデバイスコンテキスト
*/
VOID FrameInsBoxFrmDraw( HDC hDC )
{
	//UINT_PTR	cchSize;
	HFONT	hOldFnt;
	INT		topOst, iYpos;
	RECT	stFrmRct;

	UINT	dLines, d;
//	LPTSTR	ptMultiStr;
	TCHAR	atBuffer[MAX_PATH];

//	SetBkMode( hDC, OPAQUE );
	SetBkColor( hDC, ViewBackColourGet( NULL ) );	//	

	hOldFnt = SelectFont( hDC, ghAaFont );	//	フォントくっつける

	topOst = FrameInsBoxSizeGet( &stFrmRct );	//	FRAME当てはめ枠のサイズ

//	ptMultiStr = FrameMakeOutsideBoundary( stFrmRct.right, stFrmRct.bottom, &gstNowFrameInfo );

	dLines = DocStringInfoCount( gptFrmBox, 0, NULL, NULL );	//	行数確保
							//	ptMultiStr

//	stFrmRct.top = topOst;
//	stFrmRct.bottom += topOst;
//	DrawText( hDC, ptMultiStr, -1, &stFrmRct, DT_LEFT | DT_NOPREFIX | DT_NOCLIP | DT_WORDBREAK );

	iYpos = topOst;	//
	for( d = 0; dLines > d; d++ )
	{					//	ptMultiStr
		FrameMultiSubstring( gptFrmBox, d, atBuffer, MAX_PATH, 0 );
		FrameDrawItem( hDC, iYpos, atBuffer );
		iYpos += LINE_HEIGHT;
	}

//	FREE( ptMultiStr );

	SelectFont( hDC , hOldFnt );	//	フォント戻す

	return;
}
//-This way-----------------------------------------------------------------------------------------------

/*!
	壱行分の描画
	@param[in]	hDC		描画するデバイスコンテキスト
	@param[in]	iY		描画する高さ（左は常に０でよろし）
	@param[in]	ptLine	壱行分のデータ
*/
VOID FrameDrawItem( HDC hDC, INT iY, LPTSTR ptLine )
{
	UINT_PTR	cchSize, cl;
	UINT		iX, caret, len;
	INT			mRslt, mBase;
	LPTSTR		ptBgn;
	SIZE		stSize;

	StringCchLength( ptLine, STRSAFE_MAX_CCH, &cchSize );

	caret = 0;
	iX = 0;
	for( cl = 0; cchSize > cl; )
	{
		mRslt = iswspace(  ptLine[cl] );	//	開始位置の文字タイプ確認
		ptBgn = &(ptLine[cl]);

		for( len= 0; cchSize > cl; len++, cl++ )
		{
			mBase = iswspace(  ptBgn[len] );	//	文字タイプを確認していく
			if( mRslt != mBase ){	break;	}	//	タイプが変わったら
		}
		GetTextExtentPoint32( hDC, ptBgn, len, &stSize );	//	ドット数確認

		if( mRslt ){	SetBkMode( hDC, TRANSPARENT );	}
		else{	SetBkMode( hDC, OPAQUE );	}

		ExtTextOut( hDC, iX, iY, 0, NULL, ptBgn, len, NULL );

		iX += stSize.cx;
	}

	return;
}
//-First Comes Rock-----------------------------------------------------------------------------------------------


/*!
	ウインドウを閉じるときに発生。デバイスコンテキストとか確保した画面構造のメモリとかも終了。
	@param[in]	hWnd	親ウインドウのハンドル
	@return		無し
*/
VOID Fib_OnDestroy( HWND hWnd )
{
	FREE( gptFrmBox );

	MainStatusBarSetText( SB_LAYER, TEXT("") );

	ghFrInbxWnd = NULL;

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	動かされているときに発生・マウスでウインドウドラッグ中とか
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	pstPos	その瞬間のスクリーン座標
*/
VOID Fib_OnMoving( HWND hWnd, LPRECT pstPos )
{
	LONG	xEt, yEt, xLy, yLy, xSb, ySb;
	LONG	dLine, dRema;
	BOOLEAN	bMinus = FALSE;
	TCHAR	atBuffer[SUB_STRING];

	//	フレーム窓の左上スクリーン座標
	xLy = pstPos->left + gstFrmSz.x;
	yLy = pstPos->top  + gstFrmSz.y;

	//	ビューの左上テキストエリア位置
	xEt = (gstViewOrigin.x + LINENUM_WID);
	yEt = (gstViewOrigin.y + RULER_AREA);
//	TRACE( TEXT("%d x %d"), xEt, yEt );

	//	オフセット量
	xSb = xLy - xEt;	//	Ｘはそのままドット数
	ySb = yLy - yEt;	//	Ｙもドットなので行数にしないといけない

	if( 0 > ySb ){	ySb *= -1;	bMinus = TRUE;	}	//	マイナス補正
	//	行数的なモノを求めるってばよ
	dLine = ySb / LINE_HEIGHT;
	dRema = ySb % LINE_HEIGHT;
	if( (LINE_HEIGHT/2) < dRema ){	dLine++;	}
	if( bMinus ){	dLine *= -1;	}else{	dLine++;	}

	//	20110704	ここでは、まだ位置はスクロールのズレが考慮されてない
	xSb += gdHideXdot;
	dLine += gdViewTopLine;

	StringCchPrintf( atBuffer, SUB_STRING, TEXT("Frame %d[dot] %d[line]"), xSb, dLine );
	MainStatusBarSetText( SB_LAYER, atBuffer );

	return;
}
//-------------------------------------------------------------------------------------------------


/*!
	ウィンドウのサイズ変更が完了する前に送られてくる
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	pstWpos	新しい位置と大きさが入ってる
	@return		このMessageを処理したら０
*/
BOOL Fib_OnWindowPosChanging( HWND hWnd, LPWINDOWPOS pstWpos )
{
	INT		clPosY, vwTopY, dSabun, dRem;
	BOOLEAN	bMinus = FALSE;
	RECT	vwRect;

	//	移動がなかったときは何もしないでおｋ
	if( SWP_NOMOVE & pstWpos->flags )	return TRUE;

	clPosY = pstWpos->y + gstFrmSz.y;	//	表示位置のTOP

	//	表示高さを壱行単位に合わせる
	GetWindowRect( ghViewWnd, &vwRect );
	gstViewOrigin.x = vwRect.left;//位置記録・そうそう変わるものじゃない
	gstViewOrigin.y = vwRect.top;
	vwTopY = (vwRect.top  + RULER_AREA);

	dSabun = vwTopY - clPosY;
	if( 0 > dSabun ){	dSabun *= -1;	bMinus = TRUE;	}

	dRem = dSabun % LINE_HEIGHT;

	if( 0 == dRem ){	return TRUE;	}

	if( (LINE_HEIGHT/2) < dRem ){	dRem = dRem - LINE_HEIGHT;	}

	if( bMinus ){	dRem *=  -1;	}

	pstWpos->y += dRem;

	return FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	ウィンドウのサイズ変更が完了したら送られてくる
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	pstWpos	新しい位置と大きさが入ってる
*/
VOID Fib_OnWindowPosChanged( HWND hWnd, const LPWINDOWPOS pstWpos )
{
	RECT	vwRect;
	RECT	stFrmRct;
	INT		topOst;

	MoveWindow( ghFIBtlbrWnd, 0, 0, 0, 0, TRUE );	//	ツールバーは数値なくても勝手に合わせてくれる

	FrameDataGet( gdSelect, &gstNowFrameInfo );	//	枠パーツ情報確保
	gstNowFrameInfo.dRestPadd = gbMultiPaddTemp;	//	一時設定

	topOst = FrameInsBoxSizeGet( &stFrmRct );	//	FRAME当てはめ枠のサイズ
	FREE( gptFrmBox );
	gptFrmBox = FrameMakeOutsideBoundary( stFrmRct.right, stFrmRct.bottom, &gstNowFrameInfo );

	InvalidateRect( hWnd, NULL, TRUE );

	//	移動がなかったときは何もしないでおｋ
	if( SWP_NOMOVE & pstWpos->flags )	return;

	GetWindowRect( ghViewWnd, &vwRect );
	gstViewOrigin.x = vwRect.left;//位置記録・そうそう変わるものじゃない
	gstViewOrigin.y = vwRect.top;

	gstOffset.x = pstWpos->x - vwRect.left;
	gstOffset.y = pstWpos->y - vwRect.top;

	return;
}
//-------------------------------------------------------------------------------------------------


/*!
	ビューが移動した
	@param[in]	hWnd	本体ウインドウハンドル・あまり意味はない
	@param[in]	state	窓状態・最小化なら違うコトする
	@return		HRESULT	終了状態コード
*/
HRESULT FrameMoveFromView( HWND hWnd, UINT state )
{
	RECT	vwRect = {0,0,0,0};
	POINT	lyPoint;

	if( !(ghFrInbxWnd) )	return S_FALSE;

	//	最小化時は非表示にするとか	SIZE_MINIMIZED

	if( SIZE_MINIMIZED != state )
	{
		GetWindowRect( ghViewWnd, &vwRect );
		gstViewOrigin.x = vwRect.left;//位置記録
		gstViewOrigin.y = vwRect.top;
	}

	if( SIZE_MINIMIZED == state )
	{
		ShowWindow( ghFrInbxWnd, SW_HIDE );
	}
	else
	{
		lyPoint.x = gstOffset.x + vwRect.left;
		lyPoint.y = gstOffset.y + vwRect.top;
#ifdef _DEBUG
		SetWindowPos( ghFrInbxWnd, HWND_TOP, lyPoint.x, lyPoint.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );
#else
		SetWindowPos( ghFrInbxWnd, HWND_TOPMOST, lyPoint.x, lyPoint.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );
#endif
	}


	return S_OK;
}
//-------------------------------------------------------------------------------------------------


