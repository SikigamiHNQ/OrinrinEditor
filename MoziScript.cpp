/*! @file
	@brief 文字AAスクリプトの面倒みます
	このファイルは MoziScript.cpp です。
	@author	SikigamiHNQ
	@date	2011/10/26
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

制御窓は最前面固定
アイコンの説明分かりやすく
右クリにいれる
アイコンデザインかえる？


ＡＳＴ、ＡＳＤに対応出来るように
あらかじめ、文字に対応するAAを読み込んでおく・事前にファイルを複数登録出来る
内容はSQLオンメモリで・対応文字と、ＡＡイメージを保持しておく
ファイル組み合わせ変えて再構成できるとか
文字セット保持できるように？
ファイルはＩＮＩにまとめる。
[Mozi]
count=2
file0=....
use0=1
file1=....
use1=0


制御窓だす
入力された文字は、壱文字毎に覚えておく。左上位置を各文字毎に保持
入力内容が確定したら、レイヤボックスの書込ルーチンで処理する
表示枠・ボーダーＯＮＯＦＦ

字間調整できるように。連続改行は壱行づつ開ける

通常モード・入力ＥＤＩＴ、表示更新と確定バァラン・入力して更新、位置決めて確定
表示枠は、編集エリアにフローティングスタティックあたりで一枚板
スタティックはサブクラスでマウスダウンとかコンテキストぶんどる
ドラッグで移動・コンテキストには確定・更新でいいか
文字と文字の間を透過するかしないか選択
透過なら壱文字ずつ、しないなら一枚板に直してカキコすればいい


IDD_MOZI_SCRIPT_DLG

*/




#define MOZISCRIPT_CLASS	TEXT("MOZISCRIPT_CLASS")
#define MZ_WIDTH	320
#define MZ_HEIGHT	280

#define MZ_PARAMHEI	23

#define MOZIVIEW_CLASS	TEXT("MOZIVIEW_CLASS")
//-------------------------------------------------------------------------------------------------

#define TB_ITEMS	3	//	5
static  TBBUTTON	gstMztbInfo[] = {
	{  0,	IDM_MOZI_DECIDE,	TBSTATE_ENABLED,	TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{  1,	IDM_MOZI_REFRESH,	TBSTATE_ENABLED,	TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{  2,	IDM_MOZI_SETTING,	TBSTATE_ENABLED,	TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  } 	//	
//	{  0,	0,					TBSTATE_ENABLED,	TBSTYLE_SEP,						{0, 0}, 0, 0  },
//	{  3,	IDM_MOZI_ADVANCE,	0,					TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  } 	//	
};	//	
//-------------------------------------------------------------------------------------------------

typedef struct tagMOZIITEM
{
	TCHAR	cch;	//!<	文字
	LPTSTR	ptAA;	//!<	文字ＡＡ

	INT		iLeft;	//!<	左ドット
	INT		iTop;	//!<	上ドット
	INT		iWidth;	//!<	最大ドット幅
	INT		iLine;	//!<	使用行数

} MOZIITEM, *LPMOZIITEM;
//-------------------------------------------------------------------------------------------------


extern FILES_ITR	gitFileIt;		//!<	今見てるファイルの本体
//#define gstFile	(*gitFileIt)		//!<	イテレータを構造体と見なす

extern INT			gixFocusPage;	//	注目中のページ・０インデックス

extern HFONT		ghAaFont;		//	AA用フォント

extern  BYTE		gbAlpha;		//	透明度

extern  HWND		ghViewWnd;		//	編集ビューウインドウのハンドル
extern INT			gdHideXdot;		//	左の隠れ部分
extern INT			gdViewTopLine;	//	表示中の最上部行番号



static TCHAR		gatMoziIni[MAX_PATH];	//!<	

static  ATOM		gMoziAtom;		//!<	
EXTERNED HWND		ghMoziWnd;		//!<	
static  HWND		ghMoziToolBar;	//!<	
static HIMAGELIST	ghMoziImgLst;	//!<	

static  ATOM		gMoziViewAtom;
static  HWND		ghMoziViewWnd;	//!<	表示スタティック

static  HWND		ghTextWnd;		//!<	文字列入力枠
static  HWND		ghIsolaLvWnd;	//!<	アドバンズド文字選択リストビュー
static  HWND		ghSettiLvWnd;	//!<	設定リストビュー

static POINT		gstViewOrigin;	//!<	ビューの左上ウインドウ位置・
static POINT		gstOffset;		//!<	ビュー左上からの、ボックスの相対位置
static POINT		gstFrmSz;		//!<	ウインドウエッジから描画領域までのオフセット

static INT			gdToolBarHei;	//!<	ツールバー太さ

static INT			gdMoziInterval;	//!<	文字間隔・正：広がる　負：縮まる

static INT			gdNowMode;		//!<	０通常　１アドバンズド　0x10設定

static INT			gdAvrWidth;		//!<	平均占有幅
static INT			gdMaxLine;		//!<	最大占有行

static LPTSTR		gptMzBuff;		//!<	テキスト枠から文字確保枠・可変
static DWORD		gcchMzBuf;		//!<	確保枠の文字数・バイトじゃないぞ

static BOOLEAN		gbQuickClose;	//!<	貼り付けたらすぐ閉じる

static WNDPROC		gpfOrigMoziEditProc;	//!<	

static sqlite3		*gpMoziTable;	//!<	文字一覧のオンメモリデタベ

static vector<MOZIITEM>	gvcMoziItem;

typedef vector<MOZIITEM>::iterator	MZTM_ITR;
//-------------------------------------------------------------------------------------------------

static LRESULT	CALLBACK gpfMoziEditProc( HWND , UINT, WPARAM, LPARAM );	//!<	


LRESULT	CALLBACK MoziProc( HWND, UINT, WPARAM, LPARAM );	//!<	
VOID	Mzs_OnCommand( HWND , INT, HWND, UINT );			//!<	
VOID	Mzs_OnPaint( HWND );								//!<	
VOID	Mzs_OnDestroy( HWND );								//!<	
VOID	Mzs_OnContextMenu( HWND, HWND, UINT, UINT );		//!<	
VOID	Mzs_OnDropFiles( HWND , HDROP );					//!<	
LRESULT	Mzs_OnNotify( HWND , INT, LPNMHDR );				//!<	

HRESULT	MoziFileRefresh( HWND );							//!<	
HRESULT	MoziFileRebuild( HWND, UINT );						//!<	

HRESULT	MoziFileListAdd( LPTSTR );							//!<	
HRESULT MoziFileListDelete( HWND  );						//!<	

HRESULT	MoziFileStore( LPTSTR );							//!<	
UINT	CALLBACK MoziItemTablise( LPTSTR, LPCTSTR, INT );	//!<	
VOID	MoziItemRemovePeriod( LPTSTR  );					//!<	

HRESULT	MoziSpaceCreate( VOID );							//!<	

HRESULT	MoziScriptInsert( HWND );							//!<	

LRESULT	CALLBACK MoziViewProc( HWND, UINT, WPARAM, LPARAM );//!<	
VOID	Mzv_OnKey( HWND, UINT, BOOL, INT, UINT );			//!<	
VOID	Mzv_OnPaint( HWND );								//!<	
VOID	Mzv_OnMoving( HWND, LPRECT );						//!<	
BOOL	Mzv_OnWindowPosChanging( HWND, LPWINDOWPOS );		//!<	
VOID	Mzv_OnWindowPosChanged( HWND, const LPWINDOWPOS );	//!<	
VOID	MoziViewDraw( HDC );								//!<	

HRESULT	MoziSqlTableOpenClose( UINT );						//!<	
HRESULT	MoziSqlTransOnOff( BYTE );							//!<	
UINT	MoziSqlItemInsert( LPTSTR, LPTSTR, INT, INT );		//!<	
LPTSTR	MoziSqlItemSelect( TCHAR, LPINT, LPINT );			//!<	
UINT	MoziSqlItemCount( LPINT, LPINT );					//!<	
HRESULT	MoziSqlItemDeleteAll( VOID );						//!<	

//-------------------------------------------------------------------------------------------------

/*!
	文字スクリプトのINIファイル名確保・ウインドウクラス登録・アプリ起動後すぐ呼ばれる
	@param[in]	ptCurrent	基準ディレクトリ
	@param[in]	hInstance	インスタンスハンドル
	@return		HRESULT	終了状態コード
*/
INT MoziInitialise( LPTSTR ptCurrent, HINSTANCE hInstance )
{
	WNDCLASSEX	wcex;
	HBITMAP		hImg, hMsq;
	MZTM_ITR	itMzitm;

	if( !(ptCurrent) || !(hInstance) )
	{
		if( ghMoziWnd ){	DestroyWindow( ghMoziWnd  );	}

		if( ghMoziImgLst  ){	ImageList_Destroy( ghMoziImgLst );	}

		if( !( gvcMoziItem.empty( ) ) )
		{
			for( itMzitm = gvcMoziItem.begin( ); gvcMoziItem.end( ) != itMzitm; itMzitm++ )
			{
				FREE( itMzitm->ptAA );
			}
			gvcMoziItem.clear( );
		}

		MoziSqlTableOpenClose( M_DESTROY );

		FREE( gptMzBuff );

		return S_OK;
	}

	StringCchCopy( gatMoziIni, MAX_PATH, ptCurrent );
	PathAppend( gatMoziIni, MZCX_INI_FILE );

//文字スクリプト制御窓
	ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= MoziProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= MOZISCRIPT_CLASS;
	wcex.hIconSm		= NULL;

	gMoziAtom = RegisterClassEx( &wcex );

	ghMoziWnd = NULL;

	ZeroMemory( &gstViewOrigin, sizeof(POINT) );
	gdToolBarHei = 0;

	//	アイコン　確定・更新・設定・アドバンズド
	ghMoziImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 4, 1 );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMP_MOZI_WRITE ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_PAGENAMECHANGE ) );
	ImageList_Add( ghMoziImgLst, hImg, hMsq );	//	確定
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMP_REFRESH ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_REFRESH ) );
	ImageList_Add( ghMoziImgLst, hImg, hMsq );	//	更新
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMP_SETTING ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_SETTING ) );
	ImageList_Add( ghMoziImgLst, hImg, hMsq );	//	設定
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	//hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMP_MOZI_ADVANCE ) );
	//hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_MOZI_ADVANCE ) );
	//ImageList_Add( ghMoziImgLst, hImg, hMsq );	//	アドバンズド
	//DeleteBitmap( hImg );	DeleteBitmap( hMsq );

//	オンメモリSQLを、ガワだけ作っておく
	MoziSqlTableOpenClose( M_CREATE );


//文字表示窓
	ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= MoziViewProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= MOZIVIEW_CLASS;
	wcex.hIconSm		= NULL;

	gMoziViewAtom = RegisterClassEx( &wcex );

	//	適当に作っておく
	gptMzBuff = (LPTSTR)malloc( MAX_PATH * sizeof(TCHAR) );
	ZeroMemory( gptMzBuff, MAX_PATH * sizeof(TCHAR) );
	gcchMzBuf = MAX_PATH;


	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	文字スクリプトウインドウ作る
	@param[in]	hInst	インスタンスハンドル
	@param[in]	hPrWnd	メインのウインドウハンドル
*/
HWND MoziScripterCreate( HINSTANCE hInst, HWND hPrWnd )
{
	LONG	x, y;
	HWND	hDktpWnd;
	UINT	dCount, height;
	TCHAR	atBuffer[MAX_STRING];
	RECT	rect, vwRect, dtRect;

	LVCOLUMN	stLvColm;

	hDktpWnd = GetDesktopWindow(  );
	GetWindowRect( hDktpWnd, &dtRect );

	GetWindowRect( hPrWnd, &rect );
	x = dtRect.right - rect.right;
	if( MZ_WIDTH >  x ){	rect.right = dtRect.right - MZ_WIDTH;	}

	if( ghMoziWnd )
	{
		SetForegroundWindow( ghMoziViewWnd );
		SetWindowPos( ghMoziWnd, HWND_TOP, rect.right, rect.top, 0, 0, SWP_NOSIZE );
		SetForegroundWindow( ghMoziWnd );

		return ghMoziWnd;
	}

	gdMoziInterval = 0;

	//	本体ウインドウ
	ghMoziWnd = CreateWindowEx( WS_EX_TOOLWINDOW | WS_EX_TOPMOST, MOZISCRIPT_CLASS,
		TEXT("文字AA変換"), WS_POPUP | WS_CAPTION | WS_SYSMENU,
		rect.right, rect.top, MZ_WIDTH, MZ_HEIGHT, NULL, NULL, hInst, NULL );

//	DragAcceptFiles( ghMoziWnd, FALSE );

	gdNowMode = 0;

	gbQuickClose = TRUE;

	//	ツールバー
	ghMoziToolBar = CreateWindowEx( WS_EX_CLIENTEDGE, TOOLBARCLASSNAME, TEXT("mozitoolbar"), WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS, 0, 0, 0, 0, ghMoziWnd, (HMENU)IDTB_MZSCR_TOOLBAR, hInst, NULL );

	if( 0 == gdToolBarHei )	//	数値未取得なら
	{
		GetWindowRect( ghMoziToolBar, &rect );
		gdToolBarHei = rect.bottom - rect.top;
	}

	//	自動ツールチップスタイルを追加
	SendMessage( ghMoziToolBar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS );

	SendMessage( ghMoziToolBar, TB_SETIMAGELIST, 0, (LPARAM)ghMoziImgLst );
	SendMessage( ghMoziToolBar, TB_SETBUTTONSIZE, 0, MAKELPARAM(16,16) );

	SendMessage( ghMoziToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );

	//	ツールチップ文字列を設定・ボタンテキストがツールチップになる
	StringCchCopy( atBuffer, MAX_STRING, TEXT("文字ＡＡ挿入") );					gstMztbInfo[0].iString = SendMessage( ghMoziToolBar, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	StringCchCopy( atBuffer, MAX_STRING, TEXT("文字列更新 / 使用ファイル更新") );	gstMztbInfo[1].iString = SendMessage( ghMoziToolBar, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	StringCchCopy( atBuffer, MAX_STRING, TEXT("使用ファイル設定") );				gstMztbInfo[2].iString = SendMessage( ghMoziToolBar, TB_ADDSTRING, 0, (LPARAM)atBuffer );
//	StringCchCopy( atBuffer, MAX_STRING, TEXT("アドバンズド") );					gstMztbInfo[4].iString = SendMessage( ghMoziToolBar, TB_ADDSTRING, 0, (LPARAM)atBuffer );

	SendMessage( ghMoziToolBar , TB_ADDBUTTONS, (WPARAM)TB_ITEMS, (LPARAM)&gstMztbInfo );	//	ツールバーにボタンを挿入

	SendMessage( ghMoziToolBar , TB_AUTOSIZE, 0, 0 );	//	ボタンのサイズに合わせてツールバーをリサイズ

	//	貼り付けたら閉じるチェックボックスを付ける
	CreateWindowEx( 0, WC_BUTTON, TEXT("挿入したら閉じる"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 128, 2, 170, 23, ghMoziToolBar, (HMENU)IDCB_MZSCR_QUICKCLOSE, hInst, NULL );
	CheckDlgButton( ghMoziToolBar, IDCB_MZSCR_QUICKCLOSE, gbQuickClose ? BST_CHECKED : BST_UNCHECKED );

	InvalidateRect( ghMoziToolBar , NULL, TRUE );		//	クライアント全体を再描画する命令

	GetClientRect( ghMoziWnd, &rect );


	//	文字間STATIC
	CreateWindowEx( 0, WC_STATIC, TEXT("文字間"), WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, 0, gdToolBarHei, 52, MZ_PARAMHEI, ghMoziWnd, (HMENU)IDS_MZSCR_INTERVAL, hInst, NULL );

	//	文字間EDIT
	CreateWindowEx( 0, WC_EDIT, TEXT("0"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, 54, gdToolBarHei, 50, MZ_PARAMHEI, ghMoziWnd, (HMENU)IDE_MZSCR_INTERVAL, hInst, NULL );

	//	文字間SPIN
	CreateWindowEx( 0, UPDOWN_CLASS, TEXT("intervalspin"), WS_CHILD | WS_VISIBLE | UDS_AUTOBUDDY, 104, gdToolBarHei, 10, MZ_PARAMHEI, ghMoziWnd, (HMENU)IDUD_MZSCR_INTERVAL, hInst, NULL );

	//	文字間透過CBX
	CreateWindowEx( 0, WC_BUTTON, TEXT("文字間は透過"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 130, gdToolBarHei, 120, MZ_PARAMHEI, ghMoziWnd, (HMENU)IDCB_MZSCR_TRANSPARENT, hInst, NULL );


	height = gdToolBarHei + MZ_PARAMHEI;

	//文字列入力枠
	ghTextWnd = CreateWindowEx( 0, WC_EDIT, TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE,
		0, height, rect.right, rect.bottom - height, ghMoziWnd, (HMENU)IDE_MZSCR_TEXT, hInst, NULL );
	SetWindowFont( ghTextWnd, ghAaFont, TRUE );

	//	サブクラス
	gpfOrigMoziEditProc = SubclassWindow( ghTextWnd, gpfMoziEditProc );

	//設定枠
	ghSettiLvWnd = CreateWindowEx( 0, WC_LISTVIEW, TEXT("mozisetting"),
		WS_CHILD | WS_BORDER | WS_VSCROLL | LVS_REPORT | LVS_SINGLESEL | LVS_NOSORTHEADER,
		0, gdToolBarHei, rect.right, rect.bottom - gdToolBarHei, ghMoziWnd, (HMENU)IDLV_MZSCR_SETTING, hInst, NULL );
	ListView_SetExtendedListViewStyle( ghSettiLvWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES );

	//	サブクラス化	
//	gpfOrgSettiLvProc = SubclassWindow( ghSettiLvWnd, gpfSettiLvProc );

	ZeroMemory( &stLvColm, sizeof(LVCOLUMN) );
	stLvColm.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	stLvColm.fmt = LVCFMT_LEFT;
	stLvColm.pszText = TEXT("ファイル名");	stLvColm.cx = 200;	stLvColm.iSubItem = 0x00;	ListView_InsertColumn( ghSettiLvWnd, 0, &stLvColm );
	stLvColm.pszText = TEXT("フルパス");	stLvColm.cx = 500;	stLvColm.iSubItem = 0x01;	ListView_InsertColumn( ghSettiLvWnd, 1, &stLvColm );


	//	オンメモリSQL、中身が無効ならデータ読み込んで構成
	dCount = MoziSqlItemCount( NULL, NULL );
	MoziFileRebuild( ghMoziWnd, dCount ? FALSE : TRUE );

	ShowWindow( ghMoziWnd, SW_SHOW );
	UpdateWindow( ghMoziWnd );


//表示・位置決め半透明フローティングウインドー
	ghMoziViewWnd = CreateWindowEx( WS_EX_TOOLWINDOW | WS_EX_LAYERED, MOZIVIEW_CLASS,
		TEXT("配置"), WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_VISIBLE, 0, 0, 160, 120, NULL, NULL, hInst, NULL);
	SetLayeredWindowAttributes( ghMoziViewWnd, 0, gbAlpha, LWA_ALPHA );
	//	透明度はレイヤボックスの設定を使う

	ZeroMemory( &gstFrmSz, sizeof(POINT) );
	ClientToScreen( ghMoziViewWnd, &gstFrmSz );

	//	ウインドウ位置を確定させる
	GetWindowRect( ghViewWnd, &vwRect );
	gstViewOrigin.x = vwRect.left;	//	ビューウインドウの位置記録
	gstViewOrigin.y = vwRect.top;
	x = (vwRect.left + LINENUM_WID) - gstFrmSz.x;
	y = (vwRect.top  + RULER_AREA)  - gstFrmSz.y;
	TRACE( TEXT("MOZI %d x %d"), x, y );
	//	この時点で0dot,0lineの位置にクライヤント左上がアッー！

#ifdef _DEBUG
	SetWindowPos( ghMoziViewWnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW );
#else
	SetWindowPos( ghMoziViewWnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW );
#endif
	gstOffset.x = x - vwRect.left;
	gstOffset.y = y - vwRect.top;

	return ghMoziWnd;
}
//-------------------------------------------------------------------------------------------------

/*!
	テキストボックスの内容をスクリプトする
*/
HRESULT MoziEditAssemble( HWND hWnd )
{
	UINT_PTR	cchSz;
	INT			ileng, i, iNdot, iNlnDot, iMxLine, wid;
	INT			iMaxDot, iLastLine, iViewXdot, iYline, iViewYdot, cx, cy;
	HWND		hWorkWnd;
	LPTSTR		ptScript;
	RECT		rect;
	MOZIITEM	stMzitm;

	MZTM_ITR	itMzitm;

	hWorkWnd = GetDlgItem( hWnd, IDE_MZSCR_TEXT );

	//	文字数確認してバッファ作成
	ileng = Edit_GetTextLength( hWorkWnd );
	cchSz = ileng + 2;

	if( gcchMzBuf <  cchSz )	//	容量足りないなら拡張する
	{
		ptScript = (LPTSTR)realloc( gptMzBuff, cchSz * sizeof(TCHAR) );
		gptMzBuff = ptScript;
		gcchMzBuf = cchSz;
	}

	ZeroMemory( gptMzBuff, gcchMzBuf * sizeof(TCHAR) );
	Edit_GetText( hWorkWnd, gptMzBuff, cchSz );


	//	先のデータ破壊
	if( !( gvcMoziItem.empty( ) ) )
	{
		for( itMzitm = gvcMoziItem.begin( ); gvcMoziItem.end( ) != itMzitm; itMzitm++ ){	FREE( itMzitm->ptAA );	}
		gvcMoziItem.clear( );
	}

	iNdot   = 0;
	iNlnDot = 0;
	iMxLine = 1;
	iMaxDot   = 0;
	iLastLine = 0;
	for( i = 0; ileng > i; i++ )	//	壱文字ずつバラして位置確認
	{
		ZeroMemory( &stMzitm, sizeof(MOZIITEM) );

		if( TEXT('\r') ==  gptMzBuff[i] )	//	改行
		{
			iLastLine += iMxLine;

			iNdot = 0;
			iNlnDot += (iMxLine * LINE_HEIGHT);
			iMxLine = 1;
			i++;
		}
		else
		{
			stMzitm.ptAA  = MoziSqlItemSelect( gptMzBuff[i], &(stMzitm.iLine), &(stMzitm.iWidth) );
			if( !(stMzitm.ptAA) )	//	もし対応文字がなかったら、平均幅でパディング
			{
				wid = ViewLetterWidthGet( gptMzBuff[i] );
				//	文字幅確認して、適当に全角半角と見なす。
				if( 10 <= wid ){	stMzitm.iWidth = gdAvrWidth;	}
				else{	stMzitm.iWidth = gdAvrWidth / 2;	}
				stMzitm.iLine = gdMaxLine;
			}
			stMzitm.cch   = gptMzBuff[i];
			stMzitm.iLeft = iNdot;
			stMzitm.iTop  = iNlnDot;
			if( iMxLine < stMzitm.iLine ){	iMxLine = stMzitm.iLine;	}

			gvcMoziItem.push_back( stMzitm );
			iNdot += stMzitm.iWidth;
		}

		if( iMaxDot < iNdot )	iMaxDot = iNdot;
	}
	iLastLine += iMxLine;	//	最終行
	iLastLine++;	//	余裕付き

	//gdNowMode

	//	今の画面の行数とドット数確認
	iYline = ViewAreaSizeGet( &iViewXdot );
	iViewYdot = iYline * LINE_HEIGHT;

	GetWindowRect( ghMoziViewWnd, &rect );
	cx = rect.right  - rect.left;
	cy = rect.bottom - rect.top;
	GetClientRect( ghMoziViewWnd, &rect );
	cx -= rect.right;
	cy -= rect.bottom;

	//	多分ウインドウサイズになるはず
	cx += iMaxDot;
	cy += (iLastLine * LINE_HEIGHT);

	//	やたらデカいなら自重
	if( iViewXdot < cx ){	cx =  iViewXdot;	}
	if( iViewYdot < cy ){	cy =  iViewYdot;	}
	//	小さくても自重・最小サイズは適当
	if( 66 > cx ){	cx = 66;	}
	if( 66 > cy ){	cy = 66;	}

#ifdef _DEBUG
	SetWindowPos( ghMoziViewWnd, HWND_TOP, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
#else
	SetWindowPos( ghMoziViewWnd, HWND_TOPMOST, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
#endif

	InvalidateRect( ghMoziViewWnd, NULL, TRUE );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------


/*!
	エディットボックスサブクラス
	@param[in]	hWnd	ウインドウのハンドル
	@param[in]	msg		ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@return	処理した結果とか
*/
LRESULT CALLBACK gpfMoziEditProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	INT		len;
	INT		id;
	HWND	hWndCtl;
	UINT	codeNotify;

	switch( msg )
	{
		default:	break;

		case WM_COMMAND:
			id         = LOWORD(wParam);	//	発生したコマンドの識別子
			hWndCtl    = (HWND)lParam;		//	コマンドを発生させた子ウインドウのハンドル
			codeNotify = HIWORD(wParam);	//	追加の通知メッセージ
			TRACE( TEXT("[%X]MoziEdit COMMAND %d"), hWnd, id );
			
			switch( id )	//	キーボードショートカットをブッとばす
			{
				case IDM_PASTE:	SendMessage( hWnd, WM_PASTE, 0, 0 );	return 0;
				case IDM_COPY:	SendMessage( hWnd, WM_COPY,  0, 0 );	return 0;
				case IDM_CUT:	SendMessage( hWnd, WM_CUT,   0, 0 );	return 0;
				case IDM_UNDO:	SendMessage( hWnd, WM_UNDO,  0, 0 );	return 0;
				case IDM_ALLSEL:
					len = GetWindowTextLength( hWnd );
					SendMessage( hWnd, EM_SETSEL, 0, len );
					break;
				default:	break;
			}

			break;
	}

	return CallWindowProc( gpfOrigMoziEditProc, hWnd, msg, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------


/*!
	スクリプトビューのウインドウプロシージャ
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	message	ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@retval 0	メッセージ処理済み
	@retval no0	ここでは処理せず次に回す
*/
LRESULT CALLBACK MoziViewProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_KEYDOWN,			Mzv_OnKey );
		HANDLE_MSG( hWnd, WM_PAINT,				Mzv_OnPaint );		//	画面の更新とか
		HANDLE_MSG( hWnd, WM_WINDOWPOSCHANGING,	Mzv_OnWindowPosChanging );
		HANDLE_MSG( hWnd, WM_WINDOWPOSCHANGED,	Mzv_OnWindowPosChanged );

		case  WM_MOVING:	Mzv_OnMoving( hWnd, (LPRECT)lParam );	return 0;

		default:	break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------

/*!
	動かされているときに発生・マウスでウインドウドラッグ中とか
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	pstPos	その瞬間のスクリーン座標
*/
VOID Mzv_OnMoving( HWND hWnd, LPRECT pstPos )
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

	StringCchPrintf( atBuffer, SUB_STRING, TEXT("MOZI %d[dot] %d[line]"), xSb, dLine );
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
BOOL Mzv_OnWindowPosChanging( HWND hWnd, LPWINDOWPOS pstWpos )
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
VOID Mzv_OnWindowPosChanged( HWND hWnd, const LPWINDOWPOS pstWpos )
{
	RECT	vwRect;

	InvalidateRect( hWnd, NULL, TRUE );

	//	移動がなかったときは何もしないでおｋ
	if( SWP_NOMOVE & pstWpos->flags )	return;

	GetWindowRect( ghViewWnd, &vwRect );
	gstViewOrigin.x = vwRect.left;	//	位置記録
	gstViewOrigin.y = vwRect.top;

	gstOffset.x = pstWpos->x - vwRect.left;
	gstOffset.y = pstWpos->y - vwRect.top;

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
VOID Mzv_OnKey( HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags )
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

#ifdef _DEBUG
	SetWindowPos( hWnd, HWND_TOP, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
#else
	SetWindowPos( hWnd, HWND_TOPMOST, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
#endif
	Mzv_OnMoving( hWnd, &rect );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	PAINT。無効領域が出来たときに発生。背景の扱いに注意。背景を塗りつぶしてから、オブジェクトを描画
	@param[in]	hWnd	ウインドウハンドル
*/
VOID Mzv_OnPaint( HWND hWnd )
{
	PAINTSTRUCT	ps;
	HDC			hdc;

	hdc = BeginPaint( hWnd, &ps );

	MoziViewDraw( hdc );

	//リアルタイム更新するとフォーカスがおかしくなる？

	EndPaint( hWnd, &ps );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	表示枠の描画処理
	@param[in]	hDC	描画するデバイスコンテキスト
*/
VOID MoziViewDraw( HDC hDC )
{
	UINT_PTR	cchSize, dLeng, dPos;
	INT_PTR	iItems, iLn;
	INT		x, y, cmr;
	LPTSTR	ptHead;
	HFONT	hFtOld;

	MZTM_ITR	itMzitm;

	hFtOld = SelectFont( hDC, ghAaFont );	//	フォントをくっつける
	SetBkMode( hDC, TRANSPARENT );

	iItems = gvcMoziItem.size( );

	cmr = 0;
	//	各文字毎に
	for( itMzitm = gvcMoziItem.begin( ); gvcMoziItem.end( ) != itMzitm; itMzitm++ )
	{
		if( !(itMzitm->ptAA) )	continue;	//	データが無いなら飛ばせばいいはず

		StringCchLength( itMzitm->ptAA, STRSAFE_MAX_CCH, &cchSize );	//	全体長さ確認して

		x      = itMzitm->iLeft;

		if( 0 >= x ){	cmr = 0;	}
		x += (gdMoziInterval * cmr);
		cmr++;

		y      = itMzitm->iTop;
		ptHead = itMzitm->ptAA;
		dLeng  = 0;
		dPos   = 0;
		for( iLn = 0; itMzitm->iLine >  iLn; iLn++ )	//	文字の行毎に
		{
			while( cchSize >= dLeng )	//	改行まで・もしくはNULLまで
			{
				if( TEXT('\r') == itMzitm->ptAA[dLeng] )
				{
					ExtTextOut( hDC, x, y, 0, NULL, ptHead, dPos, NULL );

					y      += LINE_HEIGHT;	//	次の行位置へ
					dLeng  += 2;	//	'\n'こえて次の行の先頭
					dPos    = 0;
					ptHead  = &(itMzitm->ptAA[dLeng]);
					break;
				}

				if( 0 == itMzitm->ptAA[dLeng] )
				{
					ExtTextOut( hDC, x, y, 0, NULL, ptHead, dPos, NULL );
					break;
				}

				dLeng++;	dPos++;
			}
		}
	}

	SelectFont( hDC, hFtOld );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ビューが移動した
	@param[in]	hWnd	本体ウインドウハンドル・あまり意味はない
	@param[in]	state	窓状態・最小化なら違うコトする
	@return		HRESULT	終了状態コード
*/
HRESULT MoziMoveFromView( HWND hWnd, UINT state )
{
	RECT	vwRect = {0,0,0,0};
	POINT	lyPoint;

	if( !(ghMoziViewWnd) )	return S_FALSE;

	//	最小化時は非表示にするとか	SIZE_MINIMIZED

	if( SIZE_MINIMIZED != state )
	{
		GetWindowRect( ghViewWnd, &vwRect );
		gstViewOrigin.x = vwRect.left;//位置記録
		gstViewOrigin.y = vwRect.top;
	}

	if( SIZE_MINIMIZED == state )
	{
		ShowWindow( ghMoziViewWnd, SW_HIDE );
	}
	else
	{
		lyPoint.x = gstOffset.x + vwRect.left;
		lyPoint.y = gstOffset.y + vwRect.top;
#ifdef _DEBUG
		SetWindowPos( ghMoziViewWnd, HWND_TOP, lyPoint.x, lyPoint.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );
#else
		SetWindowPos( ghMoziViewWnd, HWND_TOPMOST, lyPoint.x, lyPoint.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );
#endif
	}

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
LRESULT CALLBACK MoziProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_PAINT,       Mzs_OnPaint );		//	画面の更新とか
		HANDLE_MSG( hWnd, WM_NOTIFY,      Mzs_OnNotify );		//	コモンコントロールの個別イベント
		HANDLE_MSG( hWnd, WM_COMMAND,     Mzs_OnCommand );	
		HANDLE_MSG( hWnd, WM_DESTROY,     Mzs_OnDestroy );		//	終了時の処理
		HANDLE_MSG( hWnd, WM_DROPFILES,   Mzs_OnDropFiles );	//	D&D
		HANDLE_MSG( hWnd, WM_CONTEXTMENU, Mzs_OnContextMenu );

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
VOID Mzs_OnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify )
{
	LRESULT	lRslt;

	switch( id )
	{
		case IDM_MOZI_DECIDE:
			if( !(0x10 & gdNowMode) )
			{
				MoziScriptInsert( hWnd );
				if( gbQuickClose  ){	DestroyWindow( hWnd );	}	//	直ぐ閉じる？
			}
			break;

		case IDM_MOZI_REFRESH:
			if( 0x10 & gdNowMode  ){	MoziFileRefresh( hWnd );	 return;	}	//	設定
			else{	MoziEditAssemble( hWnd );	}
			break;

		case IDE_MZSCR_TEXT:	//	文字入力枠・リヤルタイムでビューを書換
			if( EN_UPDATE == codeNotify ){	MoziEditAssemble( hWnd );	}
			break;

		case IDM_MOZI_SETTING:
			lRslt = SendMessage( ghMoziToolBar, TB_ISBUTTONCHECKED, IDM_MOZI_SETTING, 0 );
			if( lRslt  )	//	設定モード
			{
				gdNowMode |=  0x10;
				DragAcceptFiles( ghMoziWnd, TRUE );

				SetWindowPos( ghSettiLvWnd, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE );

				ShowWindow( ghTextWnd, SW_HIDE );
				ShowWindow( GetDlgItem(hWnd,IDS_MZSCR_INTERVAL),	SW_HIDE );
				ShowWindow( GetDlgItem(hWnd,IDE_MZSCR_INTERVAL),	SW_HIDE );
				ShowWindow( GetDlgItem(hWnd,IDUD_MZSCR_INTERVAL),	SW_HIDE );
				ShowWindow( GetDlgItem(hWnd,IDCB_MZSCR_TRANSPARENT),SW_HIDE );
			}
			else
			{
				gdNowMode &= ~0x10;
				DragAcceptFiles( ghMoziWnd, FALSE );

				SetWindowPos( ghTextWnd,    HWND_TOP,    0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE );
				ShowWindow( GetDlgItem(hWnd,IDS_MZSCR_INTERVAL),	SW_SHOW );
				ShowWindow( GetDlgItem(hWnd,IDE_MZSCR_INTERVAL),	SW_SHOW );
				ShowWindow( GetDlgItem(hWnd,IDUD_MZSCR_INTERVAL),	SW_SHOW );
				ShowWindow( GetDlgItem(hWnd,IDCB_MZSCR_TRANSPARENT),SW_SHOW );

				ShowWindow( ghSettiLvWnd, SW_HIDE );
			}
			break;

//		case IDM_MOZI_ADVANCE:	break;

		case IDCB_MZSCR_QUICKCLOSE:
			gbQuickClose = IsDlgButtonChecked( GetDlgItem(hWnd,IDTB_MZSCR_TOOLBAR), IDCB_MZSCR_QUICKCLOSE ) ? TRUE : FALSE;
			SetFocus( hWnd );
			break;

		case IDM_MOZI_LISTDEL:	MoziFileListDelete( hWnd  );	break;

		case IDM_PASTE:	TRACE( TEXT("MZ PASTE") );	SendMessage( ghTextWnd, WM_PASTE, 0, 0 );	return;
		case IDM_COPY:	SendMessage( ghTextWnd, WM_COPY,  0, 0 );	return;
		case IDM_CUT:	SendMessage( ghTextWnd, WM_CUT,   0, 0 );	return;

		default:	return;
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	PAINT。無効領域が出来たときに発生。背景の扱いに注意。背景を塗りつぶしてから、オブジェクトを描画
	@param[in]	hWnd	親ウインドウのハンドル
*/
VOID Mzs_OnPaint( HWND hWnd )
{
	PAINTSTRUCT	ps;
	HDC			hdc;


	hdc = BeginPaint( hWnd, &ps );

	EndPaint( hWnd, &ps );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ウインドウを閉じるときに発生。デバイスコンテキストとか確保した画面構造のメモリとかも終了。
	@param[in]	hWnd	親ウインドウのハンドル
	@return		無し
*/
VOID Mzs_OnDestroy( HWND hWnd )
{
	MZTM_ITR	itMzitm;

	MainStatusBarSetText( SB_LAYER, TEXT("") );

	if( ghMoziViewWnd ){	DestroyWindow( ghMoziViewWnd  );	}

	//	先のデータ破壊
	if( !( gvcMoziItem.empty( ) ) )
	{
		for( itMzitm = gvcMoziItem.begin( ); gvcMoziItem.end( ) != itMzitm; itMzitm++ ){	FREE( itMzitm->ptAA );	}
		gvcMoziItem.clear( );
	}

	ghMoziWnd = NULL;

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	コンテキストメニュー呼びだしアクション(要は右クルック）
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	hWndContext	コンテキストが発生したウインドウのハンドル
	@param[in]	xPos		スクリーンＸ座標
	@param[in]	yPos		スクリーンＹ座業
	@return		無し
*/
VOID Mzs_OnContextMenu( HWND hWnd, HWND hWndContext, UINT xPos, UINT yPos )
{
	HMENU	hMenu;
	POINT	stPoint;


	stPoint.x = (SHORT)xPos;	//	画面座標はマイナスもありうる
	stPoint.y = (SHORT)yPos;

	if( ghSettiLvWnd == hWndContext )
	{
		hMenu = CreatePopupMenu(  );

		AppendMenu( hMenu, MF_STRING, IDM_MOZI_LISTDEL, TEXT("リストから削除") );

		TrackPopupMenu( hMenu, 0, stPoint.x, stPoint.y, 0, hWnd, NULL );
		DestroyMenu( hMenu );

		return;
	}


	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ノーティファイメッセージの処理
	@param[in]	hWnd		親ウインドウのハンドル
	@param[in]	idFrom		NOTIFYを発生させたコントロールのＩＤ
	@param[in]	pstNmhdr	NOTIFYの詳細
	@return		処理した内容とか
*/
LRESULT Mzs_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{
	LPNMUPDOWN	pstNmUpDown;

	if( IDUD_MZSCR_INTERVAL == idFrom )
	{
		pstNmUpDown = (LPNMUPDOWN)pstNmhdr;

		if( UDN_DELTAPOS == pstNmUpDown->hdr.code )
		{
			//	iDelta　上で－１、下で１
			TRACE( TEXT("UPDOWN %d"), pstNmUpDown->iDelta );
			gdMoziInterval += (pstNmUpDown->iDelta);
			SetDlgItemInt( hWnd, IDE_MZSCR_INTERVAL, gdMoziInterval, TRUE );

			InvalidateRect( ghMoziViewWnd, NULL, TRUE );	//	リヤルタイム？
		}
	}

	return 0;	//	何もないなら０を戻す
}
//-------------------------------------------------------------------------------------------------

/*!
	ドラッグンドロップの受け入れ
	@param[in]	hWnd	親ウインドウのハンドル
	@param[in]	hDrop	ドロッピンオブジェクツハンドゥ
*/
VOID Mzs_OnDropFiles( HWND hWnd, HDROP hDrop )
{
	TCHAR	atFileName[MAX_PATH];//, atExBuf[10];
//	LPTSTR	ptExten;

	ZeroMemory( atFileName, sizeof(atFileName) );

	DragQueryFile( hDrop, 0, atFileName, MAX_PATH );
	DragFinish( hDrop );

	TRACE( TEXT("MOZI DROP[%s]"), atFileName );

	//	拡張子を確認
	if( FileExtensionCheck( atFileName, TEXT(".ast") ) )
	{
		MoziFileListAdd( atFileName );	//	登録処理に進む
	}
	//ptExten = PathFindExtension( atFileName );	//	拡張子が無いならNULL、というか末端になる
	//if( 0 == *ptExten ){	 return;	}
	////	拡張子指定がないならなにもできない
	//StringCchCopy( atExBuf, 10, ptExten );
	//CharLower( atExBuf );	//	比較のために小文字にしちゃう
	//if( StrCmp( atExBuf, TEXT(".ast") ) ){	 return;	}
	////	ASTでないならナニもしない


	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	設定リストビューにファイル名を増やす
*/
HRESULT MoziFileListAdd( LPTSTR ptFilePath )
{
	TCHAR	atFileName[MAX_PATH];

	UINT	iItem;
	LVITEM	stLvi;

	StringCchCopy( atFileName, MAX_PATH, ptFilePath );
	PathStripPath( atFileName );	//	ファイル名のところ切り出し


	iItem = ListView_GetItemCount( ghSettiLvWnd );

	ZeroMemory( &stLvi, sizeof(stLvi) );
	stLvi.mask  = LVIF_TEXT;
	stLvi.iItem = iItem;

	stLvi.pszText  = atFileName;
	stLvi.iSubItem = 0;
	ListView_InsertItem( ghSettiLvWnd, &stLvi );

	stLvi.pszText  = ptFilePath;
	stLvi.iSubItem = 1;
	ListView_SetItem( ghSettiLvWnd, &stLvi );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!

*/
HRESULT MoziFileListDelete( HWND hWnd )
{
	INT	iItem;

	iItem = ListView_GetNextItem( ghSettiLvWnd, -1, LVNI_ALL | LVNI_SELECTED );

	//	選択されてるモノがないと無意味
	if( 0 >  iItem ){	return  E_ABORT;	}

	ListView_DeleteItem( ghSettiLvWnd, iItem );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	リストビューの内容を保存して使用文字を再構成する
	@param[in]	hWnd	ウインドウハンドル
*/
HRESULT MoziFileRefresh( HWND hWnd )
{
	TCHAR	atKeyName[MIN_STRING], atBuff[MIN_STRING];
	TCHAR	atFilePath[MAX_PATH];
	INT		iItem, i;
	UINT	bCheck;
	LVITEM	stLvi;

	iItem = ListView_GetItemCount( ghSettiLvWnd );

	//	一旦セクションを空にする
	ZeroMemory( atBuff, sizeof(atBuff) );
	WritePrivateProfileSection( TEXT("MoziScript"), atBuff, gatMoziIni );

	StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), iItem );
	WritePrivateProfileString( TEXT("MoziScript"), TEXT("Count"), atBuff, gatMoziIni );

	MoziSqlItemDeleteAll(  );	//	ここでオンメモリSQLを空にして

	ZeroMemory( &stLvi, sizeof(stLvi) );
	stLvi.mask       = LVIF_TEXT;
	stLvi.pszText    = atFilePath;
	stLvi.iSubItem   = 1;
	stLvi.cchTextMax = MAX_PATH;
	for( i = 0; iItem > i; i++ )
	{
		stLvi.iItem   = i;
		ListView_GetItem( ghSettiLvWnd, &stLvi );
		bCheck = ListView_GetCheckState( ghSettiLvWnd, i );

		StringCchPrintf( atKeyName, MIN_STRING, TEXT("File%d"), i );
		WritePrivateProfileString( TEXT("MoziScript"), atKeyName, atFilePath, gatMoziIni );

		StringCchPrintf( atKeyName, MIN_STRING, TEXT("Use%d"), i );
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), bCheck );
		WritePrivateProfileString( TEXT("MoziScript"), atKeyName, atBuff, gatMoziIni );

		//チェック有効なら、中身を記録していく
		if( bCheck ){	MoziFileStore( atFilePath );	}
	}

	if( bCheck ){	MoziSpaceCreate(  );	}	//	平均幅と最大占有行数

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	使用文字ファイルをロードして使用内容を再構成する
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	bMode	非０SQL構築　０ロードのみ
*/
HRESULT MoziFileRebuild( HWND hWnd, UINT bMode )
{
	UINT	dCount, d;
	UINT	bCheck;
	TCHAR	atFileName[MAX_PATH], atFilePath[MAX_PATH];
	TCHAR	atKeyName[MIN_STRING];
	LVITEM	stLvi;

	dCount = GetPrivateProfileInt( TEXT("MoziScript"), TEXT("Count"), 0, gatMoziIni );

	if( bMode ){	MoziSqlItemDeleteAll(  );	}	//	ここでオンメモリSQLを空にして
	ListView_DeleteAllItems( ghSettiLvWnd );

	ZeroMemory( &stLvi, sizeof(stLvi) );
	stLvi.mask       = LVIF_TEXT;

	for( d = 0; dCount > d; d++ )
	{
		StringCchPrintf( atKeyName, MIN_STRING, TEXT("File%u"), d );

		ZeroMemory( atFilePath, sizeof(atFilePath) );
		GetPrivateProfileString( TEXT("MoziScript"), atKeyName, TEXT(""), atFilePath, MAX_PATH, gatMoziIni );

		StringCchPrintf( atKeyName, MIN_STRING, TEXT("Use%u"), d );
		bCheck = GetPrivateProfileInt( TEXT("MoziScript"), atKeyName, 0, gatMoziIni );


		StringCchCopy( atFileName, MAX_PATH, atFilePath );
		PathStripPath( atFileName );	//	ファイル名のところ切り出し

		stLvi.iItem    = d;

		stLvi.pszText  = atFileName;
		stLvi.iSubItem = 0;
		ListView_InsertItem( ghSettiLvWnd, &stLvi );

		stLvi.pszText  = atFilePath;
		stLvi.iSubItem = 1;
		ListView_SetItem( ghSettiLvWnd, &stLvi );

		ListView_SetCheckState( ghSettiLvWnd, d, bCheck );

		//チェック有効なら、中身を記録していく
		if( bCheck && bMode ){	MoziFileStore( atFilePath );	}
	}

	if( bCheck && bMode ){	MoziSpaceCreate(  );	}	//	平均幅と最大占有行数

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	文字幅の平均とる。非対応文字があった場合のパディングに使う
*/
HRESULT MoziSpaceCreate( VOID )
{
	INT	iLine, iAvDot;

	//	占有最大行数と平均幅を確保
	MoziSqlItemCount( &iLine, &iAvDot );

	gdAvrWidth = iAvDot;
	gdMaxLine  = iLine;

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	内容を書き込む
	@param[in]	hWnd	ウインドウハンドル・あまり意味はない
	@return		HRESULT	終了状態コード
*/
HRESULT MoziScriptInsert( HWND hWnd )
{
	UINT	bTranst;	//	透過するか？
	INT		ixNowPage;	//	今の頁を覚えておく
	INT		ixTmpPage;	//	作業用につくる
	INT		iXhideBuf = 0, iYhideBuf = 0;

	LPVOID		pBuffer;
	INT			x, y, iByteSize, cmr;
	INT			iX = 0, iY = 0;
	HWND		hLyrWnd;
	RECT		rect;

	MZTM_ITR	itMzitm;

	bTranst = IsDlgButtonChecked( hWnd, IDCB_MZSCR_TRANSPARENT );
	//透過なら、壱文字ずつカキコすればいい。統合なら、ワークページを非表示で作って、終わったら消す

	ixNowPage = gixFocusPage;	//	頁番号バックアップ
	ixTmpPage = gixFocusPage;

#ifdef DO_TRY_CATCH
	try{
#endif

	if( !(bTranst) )	//	一時頁に作成
	{
		ixTmpPage = DocPageCreate( -1 );	//	作業用一時頁作成
		gixFocusPage = ixTmpPage;			//	注目頁変更

		iXhideBuf = gdHideXdot;		//	スクロール位置バックアップ
		iYhideBuf = gdViewTopLine;
		gdHideXdot = 0;
		gdViewTopLine = 0;
	}
	//	透過なら本頁に直で、非透過ならダミー頁に書いてから本頁に転送する

	//	挿入処理には、レイヤボックスを非表示処理で使う
	hLyrWnd = LayerBoxVisibalise( GetModuleHandle(NULL), TEXT(" "), 0x10 );
	//	ダミー文字列渡して、とりあえず枠を作成

	GetWindowRect( ghMoziViewWnd, &rect );

	cmr = 0;
	//	各文字毎に
	for( itMzitm = gvcMoziItem.begin( ); gvcMoziItem.end( ) != itMzitm; itMzitm++ )
	{
		if( !(itMzitm->ptAA) )	continue;	//	データが無いなら飛ばせばいいはず

		//	内容を転送
		LayerStringReplace( hLyrWnd, itMzitm->ptAA );

		//	レイヤの位置を変更
		x = (rect.left + gstFrmSz.x) + itMzitm->iLeft;

		if( 0 >= itMzitm->iLeft ){	cmr = 0;	}
		x += (gdMoziInterval * cmr);
		cmr++;

		y = (rect.top + gstFrmSz.y)  + itMzitm->iTop;

		LayerBoxPositionChange( hLyrWnd, x, y );

		//	上書きする
		LayerContentsImportable( hLyrWnd, IDM_LYB_OVERRIDE, &iX, &iY, D_INVISI );
	}

	if( bTranst )	//	キャレットの位置を適当にあわせる
	{
		//	透過なら本頁に、非透過なら一時頁にレイヤしてる
	}
	else	//	一時頁からぶんどって改めて書込
	{
		iByteSize = DocPageTextGetAlloc( gitFileIt, ixTmpPage, D_UNI, &pBuffer, TRUE );

	//	gixFocusPage = ixNowPage;	//	元に戻す
		DocPageDelete( ixTmpPage, ixNowPage );	//	一時頁削除して移動
//		DocPageChange( ixNowPage  );	//	削除したら頁移動

		//	レイヤの位置を変更
		GetWindowRect( ghViewWnd, &rect );	//	編集ビューが基準位置である
		x = rect.left;// + gstFrmSz.x;
		y = rect.top;//  + gstFrmSz.y;
		ViewPositionTransform( &x, &y, 1 );
		LayerBoxPositionChange( hLyrWnd, x, y );

		gdHideXdot = iXhideBuf;
		gdViewTopLine = iYhideBuf;

		//	内容を転送
		LayerStringReplace( hLyrWnd, (LPTSTR)pBuffer );

		//	上書きする
		LayerContentsImportable( hLyrWnd, IDM_LYB_OVERRIDE, &iX, &iY, D_INVISI );

		FREE(pBuffer);
	}

	//キャレット位置修正
	//	まず場所を確認
	GetWindowRect( ghViewWnd, &rect );
	x = rect.left + LINENUM_WID;
	y = rect.top  + RULER_AREA;

	GetWindowRect( ghMoziViewWnd, &rect );
	iX = (rect.left + gstFrmSz.x) - x;
	iY = (rect.top  + gstFrmSz.y) - y;
	//	左や上にはみ出してたら、ここはマイナスになっている

	iY /= LINE_HEIGHT;

	//	この時点では、スクロールによるズレが考慮されてない
	iX += gdHideXdot;
	iY += gdViewTopLine;	//	多分これで大丈夫

	if( gdHideXdot >    iX )	iX = gdHideXdot + 11;	//	念のため
	if( gdViewTopLine > iY )	iY = gdViewTopLine;

	ViewPosResetCaret( iX, iY );	

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return ETC_MSG( err.what(), E_FAIL );	}
	catch( ... ){	return  ETC_MSG( ("etc error"), E_FAIL );	}
#endif

	//	終わったら閉じる
	DestroyWindow( hLyrWnd );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	頁毎の内容をぶち込む
	@param[in]	ptLter	項目の名前・無い時はNULL
	@param[in]	ptCont	項目の内容
	@param[in]	cchSize	内容の文字数
	@return		UINT	特に意味なし
*/
UINT CALLBACK MoziItemTablise( LPTSTR ptLter, LPCTSTR ptCont, INT cchSize )
{
	UINT_PTR	cchSz;
	LPTSTR		ptItem;

	LPTSTR	ptCaret, ptNext;
//	TCHAR	ch;
	INT		cl, dot, maxd;

	StringCchLength( ptLter, STRSAFE_MAX_CCH, &cchSz );
	if( 1 != cchSz )	return 0;
	//	ガイドが１字でないと意味が無い

	ptItem = (LPTSTR)malloc( (cchSize+1) * sizeof(TCHAR) );
	ZeroMemory( ptItem, (cchSize+1) * sizeof(TCHAR) );
	StringCchCopyN( ptItem, (cchSize+1), ptCont, cchSize );

	//	行数、最大ドット幅の解析もするか
	cl = 0;
	maxd = 0;
	ptCaret = ptItem;
	ptNext  = ptCaret;
	while( *ptCaret )
	{
		if( 0x000D == *ptCaret )
		{
			*ptCaret = 0x0000;	//	いったん終点にする
			MoziItemRemovePeriod( ptNext  );	//	先頭と終末のピリヲドをユニコードスペツナズに置き換え
			dot = ViewStringWidthGet( ptNext );
			*ptCaret = 0x000D;

			if( maxd <= dot )	maxd =  dot;	//	ドットカウント
			cl++;	//	行カウント

			ptCaret++;	//	0x000Aに移動
			ptCaret++;	//	次の行の先頭に移動
			ptNext  = ptCaret;	//	先頭を確保
		}
		else
		{
			ptCaret++;	//	次の文字へ移動
		}
	}
	//	最終行について
	MoziItemRemovePeriod( ptNext  );	//	先頭と終末のピリヲドをユニコードスペツナズに置き換え
	dot = ViewStringWidthGet( ptNext );
	if( maxd <= dot )	maxd = dot;
	cl++;	//	行カウント


	MoziSqlItemInsert( ptLter, ptItem, cl, maxd );	//	データベースにガンガン登録

	FREE(ptItem);

	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	壱行のデータを受け取って、先頭と終末のピリヲドを３ドットのユニコード空白に置き換える
	@param[in]	ptText	ターゲット行・NULLで終わっていること
*/
VOID MoziItemRemovePeriod( LPTSTR ptText )
{
	UINT_PTR	cchSize;
	UINT		d;

	StringCchLength( ptText, STRSAFE_MAX_CCH, &cchSize );
	if( 0 == cchSize )	return;	//	文字列ないならナニもしない

	//	先頭ピリヲド検索
	for( d = 0; cchSize > d; d++ )
	{
		if( TEXT('.') != ptText[d] )	break;
		//	ピリヲドじゃ無くなったら終わり
		ptText[d] = (TCHAR)0x2006;
	}

	//	終末ピリヲド検索
	for( d = (cchSize-1); 0 < d; d-- )
	{
		if( TEXT('.') != ptText[d] )	break;
		//	ピリヲドじゃ無くなったら終わり
		ptText[d] = (TCHAR)0x2006;
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	文字ファイルを読み込んでＳＱＬに貯めていく
	@param[in]	ptFilePath	読みこむファイル名
*/
HRESULT MoziFileStore( LPTSTR ptFilePath )
{
	HANDLE		hFile;
	DWORD		readed, dByteSz;
	UINT_PTR	cchSize;
	LPSTR		pcStr;
	LPTSTR		ptText;

	//	レッツオーポン
	hFile = CreateFile( ptFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile ){	return E_INVALIDARG;	}

	dByteSz = GetFileSize( hFile, NULL );
	pcStr = (LPSTR)malloc( dByteSz + 2 );
	ZeroMemory( pcStr, dByteSz + 2 );

	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	ReadFile( hFile, pcStr, dByteSz, &readed, NULL );
	CloseHandle( hFile );	//	内容全部取り込んだから開放

	//	拡張子を確認
	if( FileExtensionCheck( ptFilePath, TEXT(".ast") ) )
	{
		ptText = SjisDecodeAlloc( pcStr );	//	SJISの内容をユニコードにする
		FREE(pcStr);	//	元文字列は終わり・次はユニコード

		StringCchLength( ptText, STRSAFE_MAX_CCH, &cchSize );

		MoziSqlTransOnOff( M_CREATE );
		DocStringSplitAST( ptText, cchSize, MoziItemTablise );
		MoziSqlTransOnOff( M_DESTROY );

		FREE(ptText);
	}
	else
	{
		FREE(pcStr);	//	元文字列は終わり
		return E_NOTIMPL;
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------



/*!
	文字キャッシュ用オンメモリＤＢ
*/
HRESULT MoziSqlTableOpenClose( UINT bMode )
{
	//	ツリー情報
	CONST CHAR	cacMoziTable[] = { ("CREATE TABLE MoziScr ( id INTEGER PRIMARY KEY, letter TEXT, aacont TEXT, line INTEGER, dot INTEGER )") };
	INT		rslt;
	sqlite3_stmt	*statement;

	if( bMode )
	{
		rslt = sqlite3_open( (":memory:"), &gpMoziTable );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpMoziTable );	return E_FAIL;	}

		//ツリーテーブルを生成
		rslt = sqlite3_prepare( gpMoziTable, cacMoziTable, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpMoziTable );	return E_ACCESSDENIED;	}
		rslt = sqlite3_step( statement );	//	実行
		if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpMoziTable );	return E_ACCESSDENIED;	}
		rslt = sqlite3_finalize(statement);

	}
	else
	{

		if( gpMoziTable ){	rslt = sqlite3_close( gpMoziTable );	}
		gpMoziTable = NULL;
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	SQLiteのTransactionをしたりやめたり
	@param[in]	mode	非０開始　０終了
	@return		HRESULT	終了状態コード
*/
HRESULT MoziSqlTransOnOff( BYTE mode )
{
	if( mode )
	{
		sqlite3_exec( gpMoziTable, "BEGIN TRANSACTION",  NULL, NULL, NULL );	//	トランザクション開始
	}
	else
	{
		sqlite3_exec( gpMoziTable, "COMMIT TRANSACTION", NULL, NULL, NULL );	//	トランザクション終了
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------


/*!
	ディレクトリかファイルのデータ一時バッファにドピュッ
	@param[in]	ptLter	文字コード
	@param[in]	ptCont	文字のＡＡデータ
	@param[in]	iLine	行数
	@param[in]	iDot	最大ドット数
	@return	UINT	いま登録したID番号
*/
UINT MoziSqlItemInsert( LPTSTR ptLter, LPTSTR ptCont, INT iLine, INT iDot )
{
	CONST CHAR	acMoziItemInsert[] = { ("INSERT INTO MoziScr ( letter, aacont, line, dot ) VALUES ( ?, ?, ?, ? )") };
	CONST CHAR	acAddNumCheck[] = { ("SELECT LAST_INSERT_ROWID( ) FROM MoziScr") };

	INT		rslt;
	UINT	iRast = 0;
	sqlite3_stmt	*statement;


	if( !(gpMoziTable) ){	TRACE( TEXT("MOZI NoDatabase") );	return 0;	}

	rslt = sqlite3_prepare( gpMoziTable, acMoziItemInsert, -1, &statement, NULL);
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpMoziTable );	return 0;	}

	sqlite3_reset( statement );
	rslt = sqlite3_bind_text16( statement, 1, ptLter, -1, SQLITE_STATIC );	//	letter
	rslt = sqlite3_bind_text16( statement, 2, ptCont, -1, SQLITE_STATIC );	//	aacont
	rslt = sqlite3_bind_int(    statement, 3, iLine );	//	line
	rslt = sqlite3_bind_int(    statement, 4, iDot );	//	dot

	rslt = sqlite3_step( statement );
	if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpMoziTable );	}

	sqlite3_finalize( statement );

	//	今追加したやつのアレを取得
	rslt = sqlite3_prepare( gpMoziTable, acAddNumCheck, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpMoziTable );	return 0;	}

	rslt = sqlite3_step( statement );

	iRast = sqlite3_column_int( statement, 0 );

	sqlite3_finalize( statement );

	return iRast;
}
//-------------------------------------------------------------------------------------------------


/*!
	文字を指定してＡＡゲット
	@param[in]	ch		文字
	@param[in]	piLine	行数いれるバッファ
	@param[in]	piDot	最大ドット数いれるバッファ
	@return	LPTSTR	ＡＡ本文をAllocateして戻す・開放は受けたガワでやること・無かったらNULL
*/
LPTSTR MoziSqlItemSelect( TCHAR ch, LPINT piLine, LPINT piDot )
{
	CONST CHAR	acSelect[] = { ("SELECT * FROM MoziScr WHERE letter == ?") };
	INT		rslt,iLine, iDot;
	UINT	index = 0;
	TCHAR	atMozi[3];
	LPTSTR	ptAac = NULL;
	sqlite3_stmt*	statement;


	if( !(gpMoziTable) ){	return NULL;	}

	if( !(piLine) ){	return NULL;	}	*piLine = 0;
	if( !(piDot)  ){	return NULL;	}	*piDot  = 0;


	rslt = sqlite3_prepare( gpMoziTable, acSelect, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpMoziTable );	return 0;	}

	atMozi[ 0] = ch;	atMozi[1] = NULL;	
	sqlite3_reset( statement );
	rslt = sqlite3_bind_text16( statement, 1, atMozi, -1, SQLITE_STATIC );	//	letter

	rslt = sqlite3_step( statement );
	if( SQLITE_ROW == rslt )
	{
		index = sqlite3_column_int( statement , 0 );	//	id
		StringCchCopy( atMozi, 3, (LPCTSTR)sqlite3_column_text16( statement, 1 ) );	//	letter
		rslt = sqlite3_column_bytes16( statement, 2 );	//	サイズ確認
		rslt += 2;
		ptAac = (LPTSTR)malloc( rslt );
		ZeroMemory( ptAac, rslt );
		StringCchCopy( ptAac, (rslt / sizeof(TCHAR)), (LPCTSTR)sqlite3_column_text16( statement, 2 ) );	//	aacont
		iLine = sqlite3_column_int( statement , 3 );	//	line
		iDot  = sqlite3_column_int( statement , 4 );	//	dot
		*piLine = iLine;
		*piDot  = iDot;
	}

	sqlite3_finalize( statement );

	return ptAac;
}
//-------------------------------------------------------------------------------------------------


/*!
	登録されている個数確保・ドット幅平均値、行数最大値も確保
	@param[in]	piLine	行数いれるバッファ
	@param[in]	piAvDot	平均ドット幅いれるバッファ
	@return		登録されている個数
*/
UINT MoziSqlItemCount( LPINT piLine, LPINT piAvDot )
{
	INT		iLine, iAvDot;
	UINT	rslt, iCount;
	sqlite3_stmt*	statement;

	rslt = sqlite3_prepare( gpMoziTable, ("SELECT COUNT(id), MAX(line), AVG(dot) FROM MoziScr"), -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpMoziTable );	return 0;	}

	sqlite3_step( statement );
	iCount = sqlite3_column_int( statement, 0 );
	iLine  = sqlite3_column_int( statement, 1 );
	iAvDot = sqlite3_column_int( statement, 2 );
	sqlite3_finalize(statement);

	if( piLine )	*piLine  = iLine;
	if( piAvDot )	*piAvDot = iAvDot;

	return iCount;
}
//-------------------------------------------------------------------------------------------------

/*!
	全データ破壊
*/
HRESULT MoziSqlItemDeleteAll( VOID )
{
	INT		rslt;
	sqlite3_stmt	*statement;

	if( !(gpMoziTable) ){	return E_NOTIMPL;	}

	rslt = sqlite3_prepare( gpMoziTable, ("DELETE FROM MoziScr"), -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpMoziTable );	return E_OUTOFMEMORY;	}
	rslt = sqlite3_step( statement );
	sqlite3_finalize( statement );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------




