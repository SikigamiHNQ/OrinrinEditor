/*! @file
	@brief 縦書きの面倒みます
	このファイルは VerticalScript.cpp です。
	@author	SikigamiHNQ
	@date	2012/01/11
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
縦書き機能
文字以外は透過
右から、左から並べる切替
行間調整
句読点は右寄り

文字以外の透過はどうするか

壱文字毎に幅をもっておく。句読点は特殊処理しないと見栄えがよろしくない
入力された行毎に保持。文字数が行になる・表示の問題だけ
文字毎にＸドット位置を計算してもっておく。
確定したときは、各段毎に、パディング空白いれて行に変換してステルスレイヤする

プロシージャの処理共有できない？

ツールバー
確定・更新・透過・左から
チェキボキスで終わったら閉じる
*/
//-------------------------------------------------------------------------------------------------

#define VERTSCRIPT_CLASS	TEXT("VERTSCRIPT_CLASS")
#define VT_WIDTH	320
#define VT_HEIGHT	240

#define VT_PARAMHEI	25

#define LEFT_PADD	15	//	左余裕・中心線にたいして 

#define IDEO_COMMA	TEXT('、')	//	11dot
#define IDEO_FSTOP	TEXT('。')	//	11dot

#define VERTVIEW_CLASS	TEXT("VERTVIEW_CLASS")
//-------------------------------------------------------------------------------------------------

#define TB_ITEMS	5
static  TBBUTTON	gstVttbInfo[] = {
	{  0,	IDM_VLINE_DECIDE,		TBSTATE_ENABLED,	TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{  0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,						{0, 0}, 0, 0  },
	{  1,	IDM_VLINE_REFRESH,		TBSTATE_ENABLED,	TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{  2,	IDCB_VLINE_LEFT_GO,		TBSTATE_ENABLED,	TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	
	{  3,	IDM_VLINE_TRANSPARENT,	TBSTATE_ENABLED,	TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  } 	//	
};	//	
//-------------------------------------------------------------------------------------------------


typedef struct tagVERTITEM
{
	TCHAR	cch;	//!<	文字
	INT		iWidth;	//!<	文字幅・中心になるように揃える

	INT		iOffset;//!<	最左からの右オフセットドット・相対位置ではない

	INT		iRow;	//!<	縦にした時のＹ位置
	INT		iColumn;//!<	縦にした時の文字列番号・改行に注意

} VERTITEM, *LPVERTITEM;
//-------------------------------------------------------------------------------------------------

extern FILES_ITR	gitFileIt;		//!<	今見てるファイルの本体
//#define gstFile	(*gitFileIt)		//!<	イテレータを構造体と見なす

extern INT			gixFocusPage;	//	注目中のページ・０インデックス

extern HFONT		ghAaFont;		//	AA用フォント

extern  BYTE		gbAlpha;		//	透明度

extern  HWND		ghViewWnd;		//	編集ビューウインドウのハンドル
extern INT			gdHideXdot;		//	左の隠れ部分
extern INT			gdViewTopLine;	//	表示中の最上部行番号

static  HWND		ghVertToolBar;	//!<	
static HIMAGELIST	ghVertImgLst;	//!<	

static  ATOM		gVertAtom;		//!<	
EXTERNED HWND		ghVertWnd;		//!<	

static  HWND		ghTextWnd;		//!<	文字列入力枠
static INT			gdToolBarHei;	//!<	ツールバー太さ

static  ATOM		gVertViewAtom;
static  HWND		ghVertViewWnd;	//!<	表示スタティック

static POINT		gstViewOrigin;	//!<	ビューの左上ウインドウ位置・
static POINT		gstOffset;		//!<	ビュー左上からの、ボックスの相対位置
static POINT		gstFrmSz;		//!<	ウインドウエッジから描画領域までのオフセット

static INT			gdVertInterval;	//!<	行間隔ドット数・デフォを２２で
static  UINT		gbLeftGo;		//!<	非０左から　０右から配置する

static  UINT		gbSpTrans;		//!<	空白を　非０透過　０透過しない

static LPTSTR		gptVtBuff;		//!<	テキスト枠から文字確保枠・可変
static DWORD		gcchVtBuf;		//!<	確保枠の文字数・バイトじゃないぞ

static BOOLEAN		gbQuickClose;	//!<	貼り付けたら直ぐ閉じる

static WNDPROC		gpfOrigVertEditProc;	//!<	


static  vector<VERTITEM>	gvcVertItem;
typedef vector<VERTITEM>::iterator	VTIM_ITR;
typedef vector<VERTITEM>::reverse_iterator	VTIM_RITR;
//-------------------------------------------------------------------------------------------------

static LRESULT	CALLBACK gpfVertEditProc( HWND , UINT, WPARAM, LPARAM );	//!<	

LRESULT	CALLBACK VertProc( HWND, UINT, WPARAM, LPARAM );	//!<	
VOID	Vrt_OnCommand( HWND , INT, HWND, UINT );	//!<	
VOID	Vrt_OnPaint( HWND );	//!<	
VOID	Vrt_OnDestroy( HWND );	//!<	
LRESULT	Vrt_OnNotify( HWND , INT, LPNMHDR );	//!<	

LRESULT	CALLBACK VertViewProc( HWND, UINT, WPARAM, LPARAM );	//!<	
VOID	Vvw_OnKey( HWND, UINT, BOOL, INT, UINT );			//!<	
VOID	Vvw_OnPaint( HWND );								//!<	
VOID	Vvw_OnMoving( HWND, LPRECT );						//!<	
BOOL	Vvw_OnWindowPosChanging( HWND, LPWINDOWPOS );		//!<	
VOID	Vvw_OnWindowPosChanged( HWND, const LPWINDOWPOS );	//!<	

HRESULT	VertTextAssemble( HWND );	//!<	
VOID	VertViewDraw( HDC );		//!<	
HRESULT	VertScriptInsert( HWND );	//!<	
//-------------------------------------------------------------------------------------------------


/*!
	ウインドウクラス登録とか・アプリ起動後すぐ呼ばれる
	@param[in]	ptCurrent	基準ディレクトリ・使わない？
	@param[in]	hInstance	インスタンスハンドル
	@return		HRESULT	終了状態コード
*/
INT VertInitialise( LPTSTR ptCurrent, HINSTANCE hInstance )
{
	WNDCLASSEX	wcex;
	HBITMAP		hImg, hMsq;

	if( !(ptCurrent) || !(hInstance) )
	{
		if( ghVertWnd ){	DestroyWindow( ghVertWnd  );	}

		FREE( gptVtBuff );

		return S_OK;
	}


//縦書き制御窓
	ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= VertProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= VERTSCRIPT_CLASS;
	wcex.hIconSm		= NULL;

	gVertAtom = RegisterClassEx( &wcex );

	ghVertWnd = NULL;

	ZeroMemory( &gstViewOrigin, sizeof(POINT) );


//文字表示窓
	ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= VertViewProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= VERTVIEW_CLASS;
	wcex.hIconSm		= NULL;

	gVertViewAtom = RegisterClassEx( &wcex );

	//	適当に作っておく
	gptVtBuff = (LPTSTR)malloc( MAX_PATH * sizeof(TCHAR) );
	ZeroMemory( gptVtBuff, MAX_PATH * sizeof(TCHAR) );
	gcchVtBuf = MAX_PATH;

	//	アイコン　確定・更新・左から・透過
	ghVertImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 4, 1 );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMP_MOZI_WRITE ) );	//	対象名前注意
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_PAGENAMECHANGE ) );
	ImageList_Add( ghVertImgLst, hImg, hMsq );	//	確定
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMP_REFRESH ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_REFRESH ) );
	ImageList_Add( ghVertImgLst, hImg, hMsq );	//	更新
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMP_VERT_LEFT ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_VERT_LEFT ) );
	ImageList_Add( ghVertImgLst, hImg, hMsq );	//	左から
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMP_VERT_TRANS ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_VERT_TRANS ) );
	ImageList_Add( ghVertImgLst, hImg, hMsq );	//	透過
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );


	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	縦書きスクリプトウインドウ作る
	@param[in]	hInst	インスタンスハンドル
	@param[in]	hPrWnd	メインのウインドウハンドル
*/
HWND VertScripterCreate( HINSTANCE hInst, HWND hPrWnd )
{
	LONG	x, y;
	HWND	hDktpWnd;
	UINT	height;
	TCHAR	atBuffer[MAX_STRING];
	RECT	rect, vwRect, dtRect;

	hDktpWnd = GetDesktopWindow(  );
	GetWindowRect( hDktpWnd, &dtRect );

	GetWindowRect( hPrWnd, &rect );
	x = dtRect.right - rect.right;
	if( VT_WIDTH >  x ){	rect.right = dtRect.right - VT_WIDTH;	}

	if( ghVertWnd )
	{
		SetForegroundWindow( ghVertViewWnd );
		SetWindowPos( ghVertWnd, HWND_TOP, rect.right, rect.top, 0, 0, SWP_NOSIZE );
		SetForegroundWindow( ghVertWnd );


		return ghVertWnd;
	}

	gbLeftGo = 0;

	gbSpTrans = 0;

	gbQuickClose = 1;	//	初期状態で直ぐ閉じる

	//	本体ウインドウ
	ghVertWnd = CreateWindowEx( WS_EX_TOOLWINDOW | WS_EX_TOPMOST, VERTSCRIPT_CLASS,
		TEXT("縦書き"), WS_POPUP | WS_CAPTION | WS_SYSMENU,
		rect.right, rect.top, VT_WIDTH, VT_HEIGHT, NULL, NULL, hInst, NULL );

	//	ツールバー
	ghVertToolBar = CreateWindowEx( WS_EX_CLIENTEDGE, TOOLBARCLASSNAME, TEXT("verttoolbar"), WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS, 0, 0, 0, 0, ghVertWnd, (HMENU)IDTB_VLINE_TOOLBAR, hInst, NULL );

	if( 0 == gdToolBarHei )	//	数値未取得なら
	{
		GetWindowRect( ghVertToolBar, &rect );
		gdToolBarHei = rect.bottom - rect.top;
	}

	//	自動ツールチップスタイルを追加
	SendMessage( ghVertToolBar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS );

	SendMessage( ghVertToolBar, TB_SETIMAGELIST, 0, (LPARAM)ghVertImgLst );
	SendMessage( ghVertToolBar, TB_SETBUTTONSIZE, 0, MAKELPARAM(16,16) );

	SendMessage( ghVertToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );

	//	ツールチップ文字列を設定・ボタンテキストがツールチップになる
	StringCchCopy( atBuffer, MAX_STRING, TEXT("文字列挿入") );	gstVttbInfo[0].iString = SendMessage( ghVertToolBar, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	StringCchCopy( atBuffer, MAX_STRING, TEXT("文字列更新") );	gstVttbInfo[2].iString = SendMessage( ghVertToolBar, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	StringCchCopy( atBuffer, MAX_STRING, TEXT("左から配置") );	gstVttbInfo[3].iString = SendMessage( ghVertToolBar, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	StringCchCopy( atBuffer, MAX_STRING, TEXT("空白を透過") );	gstVttbInfo[4].iString = SendMessage( ghVertToolBar, TB_ADDSTRING, 0, (LPARAM)atBuffer );

	SendMessage( ghVertToolBar , TB_ADDBUTTONS, (WPARAM)TB_ITEMS, (LPARAM)&gstVttbInfo );	//	ツールバーにボタンを挿入

	SendMessage( ghVertToolBar , TB_AUTOSIZE, 0, 0 );	//	ボタンのサイズに合わせてツールバーをリサイズ
	InvalidateRect( ghVertToolBar , NULL, TRUE );		//	クライアント全体を再描画する命令

	GetClientRect( ghVertWnd, &rect );

	//	文字間STATIC
	CreateWindowEx( 0, WC_STATIC, TEXT("行間"), WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_RIGHT, 2, gdToolBarHei, 45, VT_PARAMHEI, ghVertWnd, (HMENU)IDS_VLINE_INTERVAL, hInst, NULL );
	//	文字間EDIT
	gdVertInterval = 22;
	CreateWindowEx( 0, WC_EDIT, TEXT("22"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, 49, gdToolBarHei, 50, VT_PARAMHEI, ghVertWnd, (HMENU)IDE_VLINE_INTERVAL, hInst, NULL );
	//	文字間SPIN
	CreateWindowEx( 0, UPDOWN_CLASS, TEXT("intervalspin"), WS_CHILD | WS_VISIBLE | UDS_AUTOBUDDY, 99, gdToolBarHei, 10, VT_PARAMHEI, ghVertWnd, (HMENU)IDUD_VLINE_INTERVAL, hInst, NULL );

	CreateWindowEx( 0, WC_BUTTON, TEXT("確定したら閉じる"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 120, gdToolBarHei, 180, VT_PARAMHEI, ghVertWnd, (HMENU)IDCB_VLINE_QUICKCLOSE, hInst, NULL );
	CheckDlgButton( ghVertWnd, IDCB_VLINE_QUICKCLOSE, gbQuickClose ? BST_CHECKED : BST_UNCHECKED );

	height = gdToolBarHei + VT_PARAMHEI;

	//文字列入力枠
	ghTextWnd = CreateWindowEx( 0, WC_EDIT, TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE,
		0, height, rect.right, rect.bottom - height, ghVertWnd, (HMENU)IDE_VLINE_TEXT, hInst, NULL );
	SetWindowFont( ghTextWnd, ghAaFont, TRUE );

	//	サブクラス
	gpfOrigVertEditProc = SubclassWindow( ghTextWnd, gpfVertEditProc );


	ShowWindow( ghVertWnd, SW_SHOW );
	UpdateWindow( ghVertWnd );


//表示・位置決め半透明フローティングウインドー
	ghVertViewWnd = CreateWindowEx( WS_EX_TOOLWINDOW | WS_EX_LAYERED, VERTVIEW_CLASS,
		TEXT("配置"), WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_VISIBLE, 0, 0, 160, 120, NULL, NULL, hInst, NULL);
	SetLayeredWindowAttributes( ghVertViewWnd, 0, gbAlpha, LWA_ALPHA );
	//	透明度はレイヤボックスの設定を使う

	ZeroMemory( &gstFrmSz, sizeof(POINT) );
	ClientToScreen( ghVertViewWnd, &gstFrmSz );

	//	ウインドウ位置を確定させる
	GetWindowRect( ghViewWnd, &vwRect );	//	編集窓
	gstViewOrigin.x = vwRect.left;	//	ビューウインドウの位置記録
	gstViewOrigin.y = vwRect.top;
	x = (vwRect.left + LINENUM_WID) - gstFrmSz.x;
	y = (vwRect.top  + RULER_AREA)  - gstFrmSz.y;
	TRACE( TEXT("VERT %d x %d"), x, y );
	//	この時点で0dot,0lineの位置にクライヤント左上がアッー！

#ifdef _DEBUG
	SetWindowPos( ghVertViewWnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW );
#else
	SetWindowPos( ghVertViewWnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW );
#endif
	gstOffset.x = x - vwRect.left;
	gstOffset.y = y - vwRect.top;


	return ghVertWnd;
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
LRESULT CALLBACK gpfVertEditProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
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
			TRACE( TEXT("[%X]VertEdit COMMAND %d"), hWnd, id );
			
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

	return CallWindowProc( gpfOrigVertEditProc, hWnd, msg, wParam, lParam );
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
LRESULT CALLBACK VertProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_PAINT,   Vrt_OnPaint );	//	画面の更新とか
		HANDLE_MSG( hWnd, WM_NOTIFY,  Vrt_OnNotify );	//	コモンコントロールの個別イベント
		HANDLE_MSG( hWnd, WM_COMMAND, Vrt_OnCommand );	
		HANDLE_MSG( hWnd, WM_DESTROY, Vrt_OnDestroy );	//	終了時の処理

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
VOID Vrt_OnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify )
{
	LRESULT	lRslt;

	switch( id )
	{
		case  IDM_VLINE_DECIDE:	//	確定
			VertScriptInsert( hWnd );
			if( gbQuickClose  ){	DestroyWindow( hWnd );	}	//	直ぐ閉じる？
			break;

		case  IDM_VLINE_REFRESH:	VertTextAssemble( hWnd );	break;

		case  IDE_VLINE_TEXT:	//	文字入力枠・リヤルタイムでビューを書換
			if( EN_UPDATE == codeNotify ){	VertTextAssemble( hWnd );	}
			break;

		case  IDCB_VLINE_LEFT_GO:	//	左から
			lRslt = SendMessage( ghVertToolBar, TB_ISBUTTONCHECKED, IDCB_VLINE_LEFT_GO, 0 );
			if( lRslt ){	gbLeftGo = 1;	}	//	左から
			else{	gbLeftGo = 0;	}	//	チェキ状態を確認して書き直す
			VertTextAssemble( hWnd );
			break;

		case  IDM_VLINE_TRANSPARENT:	//	空白透過
			lRslt = SendMessage( ghVertToolBar, TB_ISBUTTONCHECKED, IDM_VLINE_TRANSPARENT, 0 );
			if( lRslt ){	gbSpTrans = 1;	}	//	透過する
			else{	gbSpTrans = 0;	}	//	透過しない
			break;

		case  IDCB_VLINE_QUICKCLOSE:	//	直ぐ閉じる
			gbQuickClose = IsDlgButtonChecked( hWnd, IDCB_VLINE_QUICKCLOSE ) ? TRUE : FALSE;
			break;

		case  IDM_PASTE:	TRACE( TEXT("VT PASTE") );	SendMessage( ghTextWnd, WM_PASTE, 0, 0 );	return;
		case  IDM_COPY:		SendMessage( ghTextWnd, WM_COPY,  0, 0 );	return;
		case  IDM_CUT:		SendMessage( ghTextWnd, WM_CUT,   0, 0 );	return;

		default:	return;
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	PAINT。無効領域が出来たときに発生。背景の扱いに注意。背景を塗りつぶしてから、オブジェクトを描画
	@param[in]	hWnd	親ウインドウのハンドル
*/
VOID Vrt_OnPaint( HWND hWnd )
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
VOID Vrt_OnDestroy( HWND hWnd )
{
	MainStatusBarSetText( SB_LAYER, TEXT("") );

	if( ghVertViewWnd ){	DestroyWindow( ghVertViewWnd  );	}

	ghVertWnd = NULL;

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
LRESULT Vrt_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{
	LPNMUPDOWN	pstNmUpDown;

	if( IDUD_VLINE_INTERVAL == idFrom )
	{
		pstNmUpDown = (LPNMUPDOWN)pstNmhdr;

		if( UDN_DELTAPOS == pstNmUpDown->hdr.code )
		{
			//	iDelta　上で－１、下で１
			TRACE( TEXT("UPDOWN %d"), pstNmUpDown->iDelta );
			gdVertInterval -= (pstNmUpDown->iDelta);
			SetDlgItemInt( hWnd, IDE_VLINE_INTERVAL, gdVertInterval, TRUE );
			VertTextAssemble( hWnd );
			InvalidateRect( ghVertViewWnd, NULL, TRUE );	//	リヤルタイム？
		}
	}

	return 0;	//	何もないなら０を戻す
}
//-------------------------------------------------------------------------------------------------

/*!
	テキストボックスの内容をスクリプトする
*/
HRESULT VertTextAssemble( HWND hWnd )
{
	UINT_PTR	cchSz;
	INT			ileng, i, iRow, iClm, iLine, iMaxRow;
	INT			iMaxY, iMaxX, iViewXdot, iViewYdot, iYline, cx, cy;	//	配置窓のサイズ変更に使用する
	INT			iLnDot, iLnCnt, bkLine;
	UINT_PTR	iMozis;
	HWND		hWorkWnd;
	LPTSTR		ptScript;
	RECT		rect;
	VERTITEM	stVtitm;

	VTIM_ITR	itVtitm;


	hWorkWnd = GetDlgItem( hWnd, IDE_VLINE_TEXT );

	//	文字数確認してバッファ作成
	ileng = Edit_GetTextLength( hWorkWnd );
	cchSz = ileng + 2;

	if( gcchVtBuf <  cchSz )	//	容量足りないなら拡張する
	{
		ptScript = (LPTSTR)realloc( gptVtBuff, cchSz * sizeof(TCHAR) );
		gptVtBuff = ptScript;
		gcchVtBuf = cchSz;
	}

	ZeroMemory( gptVtBuff, gcchVtBuf * sizeof(TCHAR) );
	Edit_GetText( hWorkWnd, gptVtBuff, cchSz );

	//	先のデータ破壊
	gvcVertItem.clear( );	//	動的メモリは無いので普通にクルヤーでおｋ

	//	中身無いならここで終わる
	if( 0 >= ileng )	return S_FALSE;

	//	壱文字ずつバラして取込
	iRow  = 0;
	iClm  = 0;
	iLine = 1;	//	行数
	iMaxRow = 0;	//	壱行の最大文字数
	for( i = 0; ileng > i; i++ )
	{
		ZeroMemory( &stVtitm, sizeof(VERTITEM) );

		if( TEXT('\r') ==  gptVtBuff[i] )	//	改行
		{
			//	最大文字数確認
			if( iMaxRow < iRow )	iMaxRow = iRow;

			i++;
			iLine++;
			iClm++;
			iRow = 0;
		}
		else
		{
			stVtitm.cch      = gptVtBuff[i];	//	文字確保
			stVtitm.iWidth   = ViewLetterWidthGet( stVtitm.cch );	//	文字幅
			stVtitm.iOffset  = LEFT_PADD;	//	最左からの右オフセットドット
			stVtitm.iRow     = iRow++;	//	縦にした時のＹ位置
			stVtitm.iColumn  = iClm;	//	縦にした時の文字列番号・改行に注意

			gvcVertItem.push_back( stVtitm );
		}
	}
	//	最大文字数確認
	if( iMaxRow < iRow )	iMaxRow = iRow;

	iMaxY = (iMaxRow+1) * LINE_HEIGHT;	//	縦・余裕入れとく
	iMaxX  = LEFT_PADD + (gdVertInterval * iLine );	//	横幅余裕

	//	まず読み込んで内容を確認・位置調整はこの後
	if( gbLeftGo ){	iLnCnt = 0;	}
	else{	iLnCnt = iClm;	}
	//	右からか左からか
	iLnDot = LEFT_PADD + (gdVertInterval * iLnCnt );	//	基準位置に注意・行は０インデックスか
	bkLine = 0;
	iMozis = gvcVertItem.size( );

	for( itVtitm = gvcVertItem.begin(); itVtitm != gvcVertItem.end(); itVtitm++ )	//	壱文字ずつ確認していく
	{
		if( bkLine != itVtitm->iColumn )	//	改行はいった
		{
			if( gbLeftGo ){	iLnCnt++;	}
			else{	iLnCnt--;	}	//	行を移る
			if( 0 > iLnCnt ){	iLnCnt =  0;	}

			iLnDot = LEFT_PADD + (gdVertInterval * iLnCnt );	//	ドット位置修正
		}
		bkLine = itVtitm->iColumn;

		//	文字中心なので、幅の半分の位置が左開始位置
		itVtitm->iOffset = iLnDot - ((itVtitm->iWidth+1) / 2);	//	四捨五入
		//	句読点のバヤイは特別な位置にしておく
		if( IDEO_COMMA == itVtitm->cch || IDEO_FSTOP == itVtitm->cch )
		{
			itVtitm->iOffset = iLnDot - 3;	//	固定値注意
		}
		//	はみ出しに注意
		if( 0 >  itVtitm->iOffset ){	itVtitm->iOffset = 0;	}
	}

	//配置窓大きさ調整
	//	今の画面の行数とドット数確認
	iYline = ViewAreaSizeGet( &iViewXdot );
	iViewYdot = iYline * LINE_HEIGHT;

	GetWindowRect( ghVertViewWnd, &rect );
	cx = rect.right  - rect.left;
	cy = rect.bottom - rect.top;
	GetClientRect( ghVertViewWnd, &rect );
	cx -= rect.right;
	cy -= rect.bottom;

	//	多分ウインドウサイズになるはず
	cx += iMaxX;
	cy += iMaxY;

	//	やたらデカいなら自重
	if( iViewXdot < cx ){	cx =  iViewXdot;	}
	if( iViewYdot < cy ){	cy =  iViewYdot;	}
	//	小さくても自重・最小サイズは適当
	if( 66 > cx ){	cx = 66;	}
	if( 66 > cy ){	cy = 66;	}


#ifdef _DEBUG
	SetWindowPos( ghVertViewWnd, HWND_TOP, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
#else
	SetWindowPos( ghVertViewWnd, HWND_TOPMOST, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
#endif

	InvalidateRect( ghVertViewWnd, NULL, TRUE );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	配置窓のウインドウプロシージャ
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	message	ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@retval 0	メッセージ処理済み
	@retval no0	ここでは処理せず次に回す
*/
LRESULT CALLBACK VertViewProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_KEYDOWN,			Vvw_OnKey );
		HANDLE_MSG( hWnd, WM_PAINT,				Vvw_OnPaint );		//	画面の更新とか
		HANDLE_MSG( hWnd, WM_WINDOWPOSCHANGING,	Vvw_OnWindowPosChanging );
		HANDLE_MSG( hWnd, WM_WINDOWPOSCHANGED,	Vvw_OnWindowPosChanged );

		case  WM_MOVING:	Vvw_OnMoving( hWnd, (LPRECT)lParam );	return 0;

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
VOID Vvw_OnMoving( HWND hWnd, LPRECT pstPos )
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
	xSb   += gdHideXdot;
	dLine += gdViewTopLine;

	StringCchPrintf( atBuffer, SUB_STRING, TEXT("VLINE %d[dot] %d[line]"), xSb, dLine );
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
BOOL Vvw_OnWindowPosChanging( HWND hWnd, LPWINDOWPOS pstWpos )
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
VOID Vvw_OnWindowPosChanged( HWND hWnd, const LPWINDOWPOS pstWpos )
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
VOID Vvw_OnKey( HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags )
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

	Vvw_OnMoving( hWnd, &rect );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	PAINT。無効領域が出来たときに発生。背景の扱いに注意。背景を塗りつぶしてから、オブジェクトを描画
	@param[in]	hWnd	ウインドウハンドル
*/
VOID Vvw_OnPaint( HWND hWnd )
{
	PAINTSTRUCT	ps;
	HDC			hdc;

	hdc = BeginPaint( hWnd, &ps );

	VertViewDraw( hdc );

	//リアルタイム更新するとフォーカスがおかしくなる？

	EndPaint( hWnd, &ps );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	表示枠の描画処理
	@param[in]	hDC	描画するデバイスコンテキスト
*/
VOID VertViewDraw( HDC hDC )
{
	INT_PTR	iItems;
	INT		x, y;
	TCHAR	atMozi[2];
	HFONT	hFtOld;

	VTIM_ITR	itVtitm;

	hFtOld = SelectFont( hDC, ghAaFont );	//	フォントをくっつける
	SetBkMode( hDC, TRANSPARENT );

	iItems = gvcVertItem.size( );

	atMozi[1] = 0;

	for( itVtitm = gvcVertItem.begin(); itVtitm != gvcVertItem.end(); itVtitm++ )	//	壱文字ずつ確認していく
	{
		atMozi[0] = itVtitm->cch;
		x = itVtitm->iOffset;
		y = itVtitm->iRow * LINE_HEIGHT;
		ExtTextOut( hDC, x, y, 0, NULL, atMozi, 1, NULL );
	}

	SelectFont( hDC, hFtOld );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	内容を書き込む
	@param[in]	hWnd	ウインドウハンドル・あまり意味はない
	@return		HRESULT	終了状態コード
*/
HRESULT	VertScriptInsert( HWND hWnd )
{
	LPTSTR		ptText;
	INT			iTgtRow, iMaxRow, iMaxClm;
	INT			iRitDot, iNeedPadd;
	HWND		hLyrWnd;
	INT			iX, iY;
	RECT		rect;

	wstring		wsBuffer;	//	作った文字列を入れちゃう

	VTIM_ITR	itVtitm;
	VTIM_RITR	itRvsVtitm;

//右からなら逆にたどる。左からなら正順。Rowを基準値にして、直前位置とのスキマは空白埋め
//改行とか入れまくって一繋がりの文字列にして、ステルスレイヤで貼る。

	//	縦横の大きさを求める
	iMaxRow = 0;	iMaxClm = 0;
	for( itVtitm = gvcVertItem.begin(); itVtitm != gvcVertItem.end(); itVtitm++ )
	{
		if( iMaxRow < itVtitm->iRow )		iMaxRow = itVtitm->iRow;
		if( iMaxClm < itVtitm->iColumn )	iMaxClm = itVtitm->iColumn;
	}

	ptText = NULL;
	wsBuffer.clear();
	for( iTgtRow = 0; iMaxRow >= iTgtRow; iTgtRow++ )	//	文字位置を上から順番に見ていく
	{
		iRitDot = 0;

		if( gbLeftGo )	//	左から
		{
			for( itVtitm = gvcVertItem.begin(); itVtitm != gvcVertItem.end(); itVtitm++ )
			{
				if( iTgtRow == itVtitm->iRow )	//	注目ROWが一致したら
				{
					iNeedPadd = itVtitm->iOffset - iRitDot;	//	ここまでの右端からのドット数
					if( 0 > iNeedPadd ){	iNeedPadd = 0;	}	//	重なった場合とかあり得る

					ptText = DocPaddingSpaceMake( iNeedPadd );	//	０ならNULLが返る
					if( ptText )	//	埋め分を継ぎ足す
					{
						wsBuffer += wstring( ptText );
						FREE( ptText );
					}
					wsBuffer += itVtitm->cch;	//	本命の文字を入れる

					iRitDot += iNeedPadd;	//	ここまでの埋め量と
					iRitDot += itVtitm->iWidth;	//	使用幅を埋める
				}
			}
		}
		else
		{
			//	右から
			for( itRvsVtitm = gvcVertItem.rbegin(); itRvsVtitm != gvcVertItem.rend(); itRvsVtitm++ )
			{
				if( iTgtRow == itRvsVtitm->iRow )	//	注目ROWが一致したら
				{
					iNeedPadd = itRvsVtitm->iOffset - iRitDot;	//	ここまでの右端からのドット数
					if( 0 > iNeedPadd ){	iNeedPadd = 0;	}	//	重なった場合とかあり得る

					ptText = DocPaddingSpaceMake( iNeedPadd );	//	０ならNULLが返る
					if( ptText )	//	埋め分を継ぎ足す
					{
						wsBuffer += wstring( ptText );
						FREE( ptText );
					}
					wsBuffer += itRvsVtitm->cch;	//	本命の文字を入れる

					iRitDot += iNeedPadd;	//	ここまでの埋め量と
					iRitDot += itRvsVtitm->iWidth;	//	使用幅を埋める
				}
			}
		}

		wsBuffer += wstring( CH_CRLFW );	//	壱行終わったら改行
	}

	//	挿入処理には、レイヤボックスを非表示処理で使う
	hLyrWnd = LayerBoxVisibalise( GetModuleHandle(NULL), wsBuffer.c_str(), 0x10 );
	//	レイヤの位置を変更
	GetWindowRect( ghVertViewWnd, &rect );
	LayerBoxPositionChange( hLyrWnd, (rect.left + gstFrmSz.x), (rect.top + gstFrmSz.y) );
	//	設定によれば、空白を全部透過指定にする
	if( gbSpTrans ){	LayerTransparentToggle( hLyrWnd, 1 );	}
	//	上書きする
	LayerContentsImportable( hLyrWnd, IDM_LYB_OVERRIDE, &iX, &iY, D_INVISI );
	ViewPosResetCaret( iX, iY );	
	//	終わったら閉じる
	DestroyWindow( hLyrWnd );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ビューが移動した
	@param[in]	hWnd	本体ウインドウハンドル・あまり意味はない
	@param[in]	state	窓状態・最小化なら違うコトする
	@return		HRESULT	終了状態コード
*/
HRESULT VertMoveFromView( HWND hWnd, UINT state )
{
	RECT	vwRect = {0,0,0,0};
	POINT	lyPoint;

	if( !(ghVertViewWnd) )	return S_FALSE;

	//	最小化時は非表示にするとか	SIZE_MINIMIZED

	if( SIZE_MINIMIZED != state )
	{
		GetWindowRect( ghViewWnd, &vwRect );
		gstViewOrigin.x = vwRect.left;//位置記録
		gstViewOrigin.y = vwRect.top;
	}

	if( SIZE_MINIMIZED == state )
	{
		ShowWindow( ghVertViewWnd, SW_HIDE );
	}
	else
	{
		lyPoint.x = gstOffset.x + vwRect.left;
		lyPoint.y = gstOffset.y + vwRect.top;
#ifdef _DEBUG
		SetWindowPos( ghVertViewWnd, HWND_TOP, lyPoint.x, lyPoint.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );
#else
		SetWindowPos( ghVertViewWnd, HWND_TOPMOST, lyPoint.x, lyPoint.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );
#endif
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------


