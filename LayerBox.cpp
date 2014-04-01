/*! @file
	@brief 合成用のレイヤボックスのビュー側の操作です
	このファイルは LayerBox.cpp です。
	@author	SikigamiHNQ
	@date	2011/05/31
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
輪郭白ヌキするには？
その行の、最初の空白以外の文字の手前と、末尾文字の後
透過エリアの開始位置と終端位置
しない・5dot・11dotくらいで

挿入上書き処理する前に、白ヌキエリアを元文字列に追加しておく
作業用文字列として、挿入処理函数内で単独で確保しておく・元文字列を破壊しないように
開始位置とか透過領域は、白ヌキに併せて弄っておく
*/

//-------------------------------------------------------------------------------------------------

typedef struct tagLAYERBOXSTRUCT
{
	LONG	id;					//!<	ボックスの認識番号

	POINT	stOffset;			//!<	ビュー左上からの、ボックスの相対位置

	HWND	hBoxWnd;			//!<	ボックスのウインドウハンドル

	HWND	hTextWnd;			//!<	テキストエリアのウインドウハンドル
//	WNDPROC	pfOrgTextProc;		//!<	サブクラス元プロシージャ・いらない？

	HWND	hToolWnd;			//!<	ツールバーのウインドウハンドル
//	WNDPROC	pfOrgToolProc;		//!<	サブクラス元プロシージャ・いらない？

	vector<ONELINE>	vcLyrImg;	//!<	表示するデータの保持・AA用

} LAYERBOXSTRUCT, *LPLAYERBOXSTRUCT;

typedef list<LAYERBOXSTRUCT>::iterator	LAYER_ITR;
typedef vector<ONELINE>::iterator		LYLINE_ITR;

//-------------------------------------------------------------------------------------------------

#define LAYERBOX_CLASS	TEXT("LAYER_BOX")
#define	LB_WIDTH	310
#define LB_HEIGHT	220

#define EDGE_BLANK_NARROW	16	//	最低限とる空白幅
#define EDGE_BLANK_WIDE		22	//	広い幅
//-------------------------------------------------------------------------------------------------

#define TB_ITEMS	8
static  TBBUTTON	gstTBInfo[] = {
	{ 0,	IDM_LYB_INSERT,		TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },	//	挿入
	{ 1,	IDM_LYB_OVERRIDE,	TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },	//	上書
	{ 0,	0,					TBSTATE_ENABLED,	TBSTYLE_SEP,						{0, 0}, 0, 0  },
	{ 2,	IDM_LYB_COPY,		TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  },	//	コピー
	{ 0,	0,					TBSTATE_ENABLED,	TBSTYLE_SEP,						{0, 0}, 0, 0  },
	{ 3,	IDM_LYB_DO_EDIT,	TBSTATE_ENABLED,	TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,	{0, 0}, 0, 0  },	//	編集ボックスON/OFF
	{ 0,	0,					TBSTATE_ENABLED,	TBSTYLE_SEP,						{0, 0}, 0, 0  },
	{ 4,	IDM_LYB_DELETE,		TBSTATE_ENABLED,	TBSTYLE_AUTOSIZE,					{0, 0}, 0, 0  } 	//	20120507	内容クルヤー

};	//	
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

extern INT		gdDocXdot;		//!<	キャレットのＸドット・ドキュメント位置
extern INT		gdDocLine;		//!<	キャレットのＹ行数・ドキュメント位置

extern INT		gdHideXdot;		//!<	左の隠れ部分
extern INT		gdViewTopLine;	//!<	表示中の最上部行番号

extern HFONT	ghAaFont;		//!<	AA用フォント

extern  HWND	ghViewWnd;		//!<	ビューウインドウハンドル

static POINT	gstViewOrigin;	//!<	ビューの左上ウインドウ位置・

static  ATOM	gLyrBoxAtom;	//!<	レイヤボックス窓のクラスアトム

static  LONG	gdBoxID;		//!<	通し番号・常にINCREMENT

static POINT	gstFrmSz;		//!<	ウインドウエッジからスタティックまでのオフセット
static INT		gdToolBarHei;	//!<	ツールバー太さ

EXTERNED BYTE	gbAlpha;		//!<	透明度

static BOOLEAN	gbQuickClose;	//!<	貼り付けたら直ぐ閉じる

static WNDPROC	gpfOrigLyrTBProc;	//!<	
static WNDPROC	gpfOrigLyrEditProc;	//!<	
//	元プロシージャは共通で問題無い？

static HIMAGELIST	ghLayerImgLst;	//!<	

static  list<LAYERBOXSTRUCT>	gltLayer;	//!<	複数のレイヤボックスを開いたとき
//-------------------------------------------------------------------------------------------------

static LRESULT	CALLBACK gpfLayerTBProc( HWND, UINT, WPARAM, LPARAM );	//!<	
static LRESULT	CALLBACK gpfLyrEditProc( HWND, UINT, WPARAM, LPARAM );	//!<	

LRESULT	CALLBACK LayerBoxProc( HWND, UINT, WPARAM, LPARAM );	//!<	

BOOLEAN	Lyb_OnCreate( HWND, LPCREATESTRUCT );				//!<	WM_CREATE の処理
VOID	Lyb_OnCommand( HWND , INT, HWND, UINT );			//!<	
//VOID	Lyb_OnSize( HWND , UINT, INT, INT );				//!<	
VOID	Lyb_OnKey( HWND, UINT, BOOL, INT, UINT );			//!<	
VOID	Lyb_OnPaint( HWND );								//!<	
VOID	Lyb_OnDestroy( HWND );								//!<	
VOID	Lyb_OnMoving( HWND, LPRECT );						//!<	
BOOL	Lyb_OnWindowPosChanging( HWND, LPWINDOWPOS );		//!<	
VOID	Lyb_OnWindowPosChanged( HWND, const LPWINDOWPOS );	//!<	
VOID	Lyb_OnLButtonDown( HWND, BOOL, INT, INT, UINT );	//!<	
VOID	Lyb_OnContextMenu( HWND, HWND, UINT, UINT );		//!<	

HRESULT	LayerEditOnOff( HWND, UINT );						//!<	


HRESULT	LayerStringObliterate( LAYER_ITR  );				//!<	
HRESULT	LayerFromString( LAYER_ITR, LPCTSTR );				//!<	
HRESULT	LayerFromSelectArea( LAYER_ITR , UINT );			//!<	
HRESULT	LayerFromClipboard( LAYER_ITR );					//!<	
HRESULT	LayerForClipboard( HWND, UINT );					//!<	
HRESULT	LayerOnDelete( HWND );								//!<	
INT		LayerInputLetter( LAYER_ITR, INT, INT, TCHAR );		//!<	
LPTSTR	LayerLineTextGetAlloc( LAYER_ITR, INT );			//!<	
HRESULT	LayerBoxSetString( LAYER_ITR, LPCTSTR, UINT, LPPOINT, UINT );	//!<	
HRESULT	LayerBoxSizeAdjust( LAYER_ITR );					//!<	

INT		LayerTransparentAdjust( LAYER_ITR, INT, INT );		//!<	

#ifdef EDGE_BLANK_STYLE
HRESULT	LayerEdgeBlankSizeCheck( HWND, INT );				//!<	
#endif
//-------------------------------------------------------------------------------------------------

/*!
	ボックスの作成・最初の1回のみ
	@param[in]	hInstance	アプリのインスタンス
	@param[in]	pstFrame	クライヤントサイズ
	@return		無し
*/
VOID LayerBoxInitialise( HINSTANCE hInstance, LPRECT pstFrame )
{
	WNDCLASSEX	wcex;
	HBITMAP	hImg, hMsq;

	if( !(hInstance) )	//	破壊命令
	{
		ImageList_Destroy( ghLayerImgLst );

		return;
	}

	ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	= LayerBoxProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_MENU+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= LAYERBOX_CLASS;
	wcex.hIconSm		= NULL;

	gLyrBoxAtom = RegisterClassEx( &wcex );

	gbQuickClose = TRUE;

	gdBoxID = 0;

	//ツールバー用イメージリスト作成
	ghLayerImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 5, 1 );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMP_LAYERINSERT) ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMQ_LAYERINSERT) ) );
	ImageList_Add( ghLayerImgLst, hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMP_LAAYEROVERWRITE) ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMQ_LAAYEROVERWRITE) ) );
	ImageList_Add( ghLayerImgLst, hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMP_COPY) ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMQ_COPY) ) );
	ImageList_Add( ghLayerImgLst, hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMP_LAYERTEXTEDIT) ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMQ_LAYERTEXTEDIT) ) );
	ImageList_Add( ghLayerImgLst, hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMP_DELETE) ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( (IDBMQ_DELETE) ) );
	ImageList_Add( ghLayerImgLst, hImg, hMsq );
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	れいやぼっくちゅのアルファを更新!?
	@param[in]	dParam	新しいアルファ値
	@return		HRESULT	終了状態コード
*/
HRESULT LayerBoxAlphaSet( UINT dParam )
{
	gbAlpha = dParam & 0xFF;

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*！
	レイヤボックスを作成
	@param[in]	hInst	実存値
	@param[in]	ptStr	表示すべき文字列なら有効、違うならNULL
	@param[in]	bNormal	0x00普通に処理　0x10裏処理
	@return		HWND	作成されたレイヤボックスのウインドウハンドル
*/
HWND LayerBoxVisibalise( HINSTANCE hInst, LPCTSTR ptStr, UINT bNormal )
{
	INT		x, y;
	RECT	vwRect, rect;
	DWORD	dwStyle;

	BOOLEAN	bSelect = FALSE;
	UINT	bSqSel = 0;

	LAYERBOXSTRUCT	stLayer;
	LAYER_ITR	itLyr;


//	stLayer.pfOrgTextProc = NULL;	//	あとで
//	stLayer.pfOrgToolProc = NULL;	//	あとで
	stLayer.id = gdBoxID;	//	ボックスの認識番号

	bSelect = IsSelecting( &bSqSel );

	stLayer.vcLyrImg.clear( );	//	表示するデータの保持・AA用

	if( 0x10 & bNormal ){	dwStyle = WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU;	}
	else{		dwStyle = WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_VISIBLE | WS_SYSMENU;	}

	//	場所は０にしておけばクライヤント位置で計算出来る
	stLayer.hBoxWnd = CreateWindowEx( WS_EX_TOOLWINDOW | WS_EX_LAYERED, LAYERBOX_CLASS,
		TEXT("レイヤ"), dwStyle, 0, 0, LB_WIDTH, LB_HEIGHT, NULL, NULL, hInst, NULL);

	//	ＩＤをウインドウハンドルに保存しておく
	WndTagSet( stLayer.hBoxWnd, stLayer.id );

	SetLayeredWindowAttributes( stLayer.hBoxWnd, 0, gbAlpha, LWA_ALPHA );

	//	ツールバーのウインドウハンドル
	stLayer.hToolWnd = GetDlgItem( stLayer.hBoxWnd, IDW_LYB_TOOL_BAR );

	//	ウインドウ位置を確定させる
	GetWindowRect( ghViewWnd, &vwRect );
	gstViewOrigin.x = vwRect.left;//位置記録・そうそう変わるものじゃない
	gstViewOrigin.y = vwRect.top;
	//x = (vwRect.left + LINENUM_WID) - gstFrmSz.x;
	//y = (vwRect.top  + RULER_AREA)  - gstFrmSz.y;
	//この時点で、編集エリアの０，０を示している
	x = gdDocXdot;
	y = gdDocLine * LINE_HEIGHT;
	ViewPositionTransform( &x, &y, TRUE );
	x += (vwRect.left - gstFrmSz.x);
	y += (vwRect.top  - gstFrmSz.y);
	TRACE( TEXT("%d x %d"), x, y );

#ifdef _DEBUG
	SetWindowPos( stLayer.hBoxWnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE );
#else
	SetWindowPos( stLayer.hBoxWnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE );
#endif
	stLayer.stOffset.x = x - vwRect.left;
	stLayer.stOffset.y = y - vwRect.top;

	GetClientRect( stLayer.hBoxWnd, &rect );

	//	編集用エディット
	stLayer.hTextWnd = CreateWindowEx( 0, WC_EDIT, TEXT(""), 
		WS_CHILD | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
		0, gdToolBarHei, rect.right, rect.bottom - gdToolBarHei,
		stLayer.hBoxWnd, (HMENU)IDE_LYB_TEXTEDIT, hInst, NULL );
	SetWindowFont( stLayer.hTextWnd, ghAaFont, TRUE );

	//	サブクラス
	gpfOrigLyrEditProc = SubclassWindow( stLayer.hTextWnd, gpfLyrEditProc );

	//	レイヤリストに記録
	gltLayer.push_back( stLayer );
	itLyr = gltLayer.end();
	itLyr--;	//	追加したのは末端だからこれでいい

	//	優先順位に注意
	if( ptStr  )	//	有効文字列があるなら
	{
		TRACE( TEXT("LAYER from STRING") );
		LayerFromString( itLyr, ptStr );
	}
	else if( bSelect )	//	選択範囲が有効である	DocIsSelecting
	{
		TRACE( TEXT("LAYER from Select") );
		LayerFromSelectArea( itLyr, bSqSel );
	}
	else	//	どうでもないならクルップボードから
	{
		TRACE( TEXT("LAYER from ClipBoard") );
		LayerFromClipboard( itLyr );
	}

	if( !(0x10 & bNormal) )
	{
		ShowWindow( stLayer.hBoxWnd, SW_SHOW );
		UpdateWindow( stLayer.hBoxWnd );

		GetWindowRect( stLayer.hBoxWnd, &rect );
		Lyb_OnMoving( stLayer.hBoxWnd, &rect );
	}

	gdBoxID++;

	return stLayer.hBoxWnd;
}
//-------------------------------------------------------------------------------------------------

/*!
	レイヤボックスの位置を外部から変更
	@param[in]	hWnd	対象のレイヤボックスのハンドル
	@param[in]	x		描画位置のスクリーンＸ位置
	@param[in]	y		描画位置のスクリーンＹ位置
*/
HRESULT LayerBoxPositionChange( HWND hWnd, LONG x, LONG y )
{
	LAYER_ITR	itLyr;

	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hWnd )
		{
			x -= gstFrmSz.x;
			y -= gstFrmSz.y;
			SetWindowPos( hWnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
		}
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ツールバーサブクラス
	WindowsXPで、ツールバーのボタン上でマウスの左ボタンを押したまま右ボタンを押すと、
	それ以降のマウス操作を正常に受け付けなくなる。それの対策
	@param[in]	hWnd	ツールバーハンドル
	@param[in]	msg		ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@return	処理した結果とか
*/
LRESULT CALLBACK gpfLayerTBProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	INT		itemID;
	HDC		hdc;
	HWND	hWndChild;

	switch( msg )
	{
		case WM_CTLCOLORSTATIC:	//	チェックボックスの文字列部分の色変更
			hdc = (HDC)(wParam);
			hWndChild = (HWND)(lParam);

			itemID = GetDlgCtrlID( hWndChild );

			if( IDCB_LAYER_QUICKCLOSE == itemID || IDCB_LAYER_EDGE_BLANK == itemID )
			{
				SetBkColor( hdc, GetSysColor( COLOR_WINDOW ) );
				return (LRESULT)GetSysColorBrush( COLOR_WINDOW );
			}
			break;


		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			if( SendMessage(hWnd, TB_GETHOTITEM, 0, 0) >= 0 ){	ReleaseCapture(   );	}
			return 0;
	}

	return CallWindowProc( gpfOrigLyrTBProc, hWnd, msg, wParam, lParam );
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
LRESULT CALLBACK gpfLyrEditProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
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
			TRACE( TEXT("[%X]LyrEdit COMMAND %d"), hWnd, id );
			
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

	return CallWindowProc( gpfOrigLyrEditProc, hWnd, msg, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------


/*!
	レイヤボックスのウインドウプロシージャ
	@param[in]	hWnd	親ウインドウのハンドル
	@param[in]	message	ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@retval 0	メッセージ処理済み
	@retval no0	ここでは処理せず次に回す
*/
LRESULT CALLBACK LayerBoxProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_CREATE,			Lyb_OnCreate );		
		HANDLE_MSG( hWnd, WM_COMMAND,			Lyb_OnCommand );	
		HANDLE_MSG( hWnd, WM_PAINT,				Lyb_OnPaint );		
		HANDLE_MSG( hWnd, WM_DESTROY,			Lyb_OnDestroy );	
		HANDLE_MSG( hWnd, WM_KEYDOWN,			Lyb_OnKey );		
		HANDLE_MSG( hWnd, WM_LBUTTONDBLCLK,		Lyb_OnLButtonDown );	
		HANDLE_MSG( hWnd, WM_CONTEXTMENU,		Lyb_OnContextMenu );	
		HANDLE_MSG( hWnd, WM_WINDOWPOSCHANGING,	Lyb_OnWindowPosChanging );	
		HANDLE_MSG( hWnd, WM_WINDOWPOSCHANGED,	Lyb_OnWindowPosChanged );	
	//	WM_WINDOWPOSCHANGED を使った場合、WM_SIZEは発生しないようだ
	//	HANDLE_MSG( hWnd, WM_SIZE,				Lyb_OnSize );	

		case WM_MOVING:	Lyb_OnMoving( hWnd, (LPRECT)lParam );	return 0;

		default:	break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------

/*!
	レイヤボックスのクリエイト。
	@param[in]	hWnd			親ウインドウのハンドル
	@param[in]	lpCreateStruct	アプリケーションの初期化内容
	@return	TRUE	特になし
*/
BOOLEAN Lyb_OnCreate( HWND hWnd, LPCREATESTRUCT lpCreateStruct )
{
	HINSTANCE	lcInst  = lpCreateStruct->hInstance;	//	受け取った初期化情報から、インスタンスハンドルをひっぱる
	HWND	hToolWnd, hWorkWnd;
	TCHAR	atBuffer[MAX_STRING];
//	UINT	iIndex;
	RECT	tbRect;
//	TBADDBITMAP	stToolBmp;


	hToolWnd = CreateWindowEx( WS_EX_CLIENTEDGE, TOOLBARCLASSNAME, TEXT("toolbar"), WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS, 0, 0, 0, 0, hWnd, (HMENU)IDW_LYB_TOOL_BAR, lcInst, NULL );

	//	自動ツールチップスタイルを追加
	SendMessage( hToolWnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS );

	//stToolBmp.hInst = HINST_COMMCTRL;
	//stToolBmp.nID   = IDB_STD_SMALL_COLOR;
	//iIndex = SendMessage( hToolWnd, TB_ADDBITMAP, 0, (LPARAM)&stToolBmp );
	SendMessage( hToolWnd, TB_SETIMAGELIST, 0, (LPARAM)ghLayerImgLst );

	SendMessage( hToolWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );
	//	ツールチップ文字列を設定・ボタンテキストがツールチップになる
	StringCchCopy( atBuffer, MAX_STRING, TEXT("この辺に挿入") );	gstTBInfo[0].iString = SendMessage( hToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	StringCchCopy( atBuffer, MAX_STRING, TEXT("ここらに上書") );	gstTBInfo[1].iString = SendMessage( hToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	StringCchCopy( atBuffer, MAX_STRING, TEXT("コピーする") );		gstTBInfo[3].iString = SendMessage( hToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	StringCchCopy( atBuffer, MAX_STRING, TEXT("テキスト編集") );	gstTBInfo[5].iString = SendMessage( hToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	StringCchCopy( atBuffer, MAX_STRING, TEXT("内容を削除") );		gstTBInfo[7].iString = SendMessage( hToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );

	SendMessage( hToolWnd , TB_ADDBUTTONS, (WPARAM)TB_ITEMS, (LPARAM)&gstTBInfo );	//	ツールバーにボタンを挿入

	SendMessage( hToolWnd , TB_AUTOSIZE, 0, 0 );	//	ボタンのサイズに合わせてツールバーをリサイズ
	InvalidateRect( hToolWnd , NULL, TRUE );		//	クライアント全体を再描画する命令

	//	ツールバーサブクラス化
	gpfOrigLyrTBProc = SubclassWindow( hToolWnd, gpfLayerTBProc );

	//	貼り付けたら閉じるチェックボックスを付ける
	CreateWindowEx( 0, WC_BUTTON, TEXT("貼付たら閉じる"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 150, 2, 138, 23, hToolWnd, (HMENU)IDCB_LAYER_QUICKCLOSE, lcInst, NULL );
	CheckDlgButton( hToolWnd, IDCB_LAYER_QUICKCLOSE, gbQuickClose ? BST_CHECKED : BST_UNCHECKED );

#ifdef EDGE_BLANK_STYLE
	hWorkWnd = CreateWindowEx( 0, WC_COMBOBOX, TEXT(""), WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 290, 0, 123, 70, hToolWnd, (HMENU)IDCB_LAYER_EDGE_BLANK, lcInst, NULL );
	ComboBox_AddString( hWorkWnd, TEXT("白抜きしない") );
	ComboBox_AddString( hWorkWnd, TEXT("狭く白抜き") );
	ComboBox_AddString( hWorkWnd, TEXT("広く白抜き") );
	ComboBox_SetCurSel( hWorkWnd, 0 );
#endif


	if( 0 == gdBoxID )	//	最初の壱個目のときに計算
	{
		//GetClientRect( hToolWnd, &tbRect );
		//gdToolBarHei = tbRect.bottom + 5;
		GetWindowRect( hToolWnd, &tbRect );
		gdToolBarHei = tbRect.bottom - tbRect.top;


		//	スクリーン位置は００なのがポインヨ
		gstFrmSz.x = 0;
		gstFrmSz.y = gdToolBarHei;
		ClientToScreen( hWnd, &gstFrmSz );
		TRACE( TEXT("%d x %d"), gstFrmSz.x, gstFrmSz.y );
	}

	return TRUE;
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
VOID Lyb_OnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify )
{
	LRESULT	lRslt;
	INT		bEdgeBlank;
	INT		iXpos, iYln;

	switch( id )
	{
		case IDE_LYB_TEXTEDIT:	//	レイヤ非表示にしてもKILLFOCUS出る
			if( EN_SETFOCUS  == codeNotify ){	TRACE( TEXT("LYREDIT_SETFOCUS") );	}

			if( EN_KILLFOCUS == codeNotify )
			{
				TRACE( TEXT("LYREDIT_KILLFOCUS") );
				ViewFocusSet(  );
			}
			break;

		case IDM_LYB_INSERT:	//	貼り付ける
		case IDM_LYB_OVERRIDE:
			LayerContentsImportable( hWnd, id, &iXpos, &iYln, 0 );
			ViewPosResetCaret( iXpos, iYln );	
			DocPageInfoRenew( -1, 1 );
			if( gbQuickClose  ){	DestroyWindow( hWnd );	}	//	直ぐ閉じる？
			break;

		case IDM_LYB_COPY:	//	クルップボードへ
			LayerForClipboard( hWnd, D_UNI );
			break;

		case IDM_LYB_DO_EDIT:	//	文字列を編集
			lRslt = SendMessage( GetDlgItem(hWnd,IDW_LYB_TOOL_BAR), TB_GETSTATE, IDM_LYB_DO_EDIT, 0 );
			LayerEditOnOff( hWnd, (lRslt&TBSTATE_CHECKED) ? TRUE : FALSE );
			SendMessage( GetDlgItem(hWnd,IDW_LYB_TOOL_BAR), TB_SETSTATE, IDM_LYB_INSERT,   (lRslt&TBSTATE_CHECKED) ? 0 : TBSTATE_ENABLED );
			SendMessage( GetDlgItem(hWnd,IDW_LYB_TOOL_BAR), TB_SETSTATE, IDM_LYB_OVERRIDE, (lRslt&TBSTATE_CHECKED) ? 0 : TBSTATE_ENABLED );
			break;

		case IDM_LYB_DELETE:	//	内容を削除
			LayerOnDelete( hWnd );
			break;

		case IDCB_LAYER_QUICKCLOSE:	//	貼り付けたら閉じるか？
			gbQuickClose = IsDlgButtonChecked( GetDlgItem(hWnd,IDW_LYB_TOOL_BAR), IDCB_LAYER_QUICKCLOSE ) ? TRUE : FALSE;
			SetFocus( hWnd );
			break;

#ifdef EDGE_BLANK_STYLE
		case IDCB_LAYER_EDGE_BLANK:	//	白ヌキするか
			if( CBN_SELCHANGE == codeNotify )
			{
				bEdgeBlank = ComboBox_GetCurSel( hWndCtl );
				if( 1 == bEdgeBlank ){			LayerEdgeBlankSizeCheck( hWnd, EDGE_BLANK_NARROW );	}
				else if( 2 ==  bEdgeBlank ){	LayerEdgeBlankSizeCheck( hWnd, EDGE_BLANK_WIDE );	}
			}
			break;
#endif

		case IDM_LYB_TRANCE_RELEASE:	//	透過選択を解除
			LayerTransparentToggle( hWnd, 0 );
			InvalidateRect( hWnd, NULL, TRUE );
			break;

		case IDM_LYB_TRANCE_ALL:	//	空白を全部透過領域に設定
			LayerTransparentToggle( hWnd, 1 );
			InvalidateRect( hWnd, NULL, TRUE );
			break;

		default:	TRACE( TEXT("Layer未知のコマンド %d"), id );	break;
	}

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
VOID Lyb_OnKey( HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags )
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
	Lyb_OnMoving( hWnd, &rect );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	PAINT。無効領域が出来たときに発生。背景の扱いに注意。背景を塗りつぶしてから、オブジェクトを描画
	@param[in]	hWnd	親ウインドウのハンドル
	@return		無し
*/
VOID Lyb_OnPaint( HWND hWnd )
{
	PAINTSTRUCT	ps;
	HFONT		hFtOld;
	COLORREF	clrTextOld, clrBackOld;
	HDC			hdc;
	INT			height;
	UINT_PTR	iLines, i;
	LPTSTR		ptText;
	RECT		rect;
	LAYER_ITR	itLyr;

	UINT_PTR	mz, cchLen;
	UINT		bStyle, cchMr, cbSize;
	INT			width, rdStart;
	BOOLEAN		doDraw, bRslt;

	hdc = BeginPaint( hWnd, &ps );

#ifdef DO_TRY_CATCH
	try{
#endif

	height = gdToolBarHei;

	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hWnd )
		{
			clrTextOld = SetTextColor( hdc, CLR_BLACK );
			clrBackOld = SetBkColor(   hdc, CLR_WHITE );

			GetClientRect( hWnd, &rect );
			FillRect( hdc, &rect, GetStockBrush( WHITE_BRUSH ) );

			hFtOld = SelectFont( hdc, ghAaFont );

			iLines = itLyr->vcLyrImg.size( );

			for( i = 0; iLines > i; i++ )
			{
				cchLen = itLyr->vcLyrImg.at( i ).vcLine.size(  );	//	必要文字数
				if( 0 >= cchLen ){	height += LINE_HEIGHT;	continue;	}

				cbSize = (cchLen+1) * sizeof(TCHAR);
				ptText = (LPTSTR)malloc( cbSize );	//	ぬるたーみねーた分増やす
				ZeroMemory( ptText, cbSize );

				bStyle  = itLyr->vcLyrImg.at( i ).vcLine.at( 0 ).mzStyle;
				bStyle &= CT_LYR_TRNC;
				cchMr   = 0;
				width   = 0;
				rdStart = 0;//itLyr->vcLyrImg.at( i ).dOffset;
				doDraw  = FALSE;

				for( mz = 0; cchLen >= mz; mz++ )
				{
					if( cchLen ==  mz ){	doDraw = TRUE;	}	//	末端まできちゃったら
					else
					{
						//	同じスタイルが続くなら
						if( bStyle == (itLyr->vcLyrImg.at( i ).vcLine.at( mz ).mzStyle & CT_LYR_TRNC) )
						{
							ptText[cchMr++] = itLyr->vcLyrImg.at( i ).vcLine.at( mz ).cchMozi;	//	壱繋がりの文字列として確保
							width += itLyr->vcLyrImg.at( i ).vcLine.at( mz ).rdWidth;
						}
						else{	doDraw = TRUE;	}
					}

					if( doDraw )	//	描画タイミングであるなら
					{
						if( bStyle & CT_LYR_TRNC )	//	透過部分の場合背景色と枠塗り潰し
						{
							SetBkColor(   hdc, CLR_SILVER );	//	LTGRAY_BRUSH

							SetRect( &rect, rdStart, height, rdStart + width, height + LINE_HEIGHT );
							FillRect( hdc, &rect, GetStockBrush( LTGRAY_BRUSH ) );
						}
						else
						{
							SetBkColor(   hdc, CLR_WHITE );
						}

						bRslt = ExtTextOut( hdc, rdStart, height, 0, NULL, ptText, cchMr, NULL );
						if( !(bRslt)  ){	TRACE( TEXT("ExtTextOut error") );	}

						if( cchLen != mz )
						{
							rdStart += width;
							//	描画したら、今の文字を新しいスタイルとして登録してループ再開
							bStyle  = itLyr->vcLyrImg.at( i ).vcLine.at( mz ).mzStyle;
							bStyle &= CT_LYR_TRNC;
							ZeroMemory( ptText, cbSize );
							ptText[0] = itLyr->vcLyrImg.at( i ).vcLine.at( mz ).cchMozi;
							width  = itLyr->vcLyrImg.at( i ).vcLine.at( mz ).rdWidth;
							cchMr  = 1;
						}
						doDraw = FALSE;
					}
				}

				FREE( ptText );

				height += LINE_HEIGHT;
			}

			SelectFont( hdc, hFtOld );

			break;
		}
	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	ETC_MSG( err.what(), 0 );	 return;	}
	catch( ... ){	ETC_MSG( ("etc error"), 0 );	 return;	}
#endif

	EndPaint( hWnd, &ps );


	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ウインドウを閉じるときに発生。デバイスコンテキストとか確保した画面構造のメモリとかも終了。
	@param[in]	hWnd	ウインドウハンドル
	@return		無し
*/
VOID Lyb_OnDestroy( HWND hWnd )
{
	LAYER_ITR	itLyr;

	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hWnd )
		{
			LayerStringObliterate( itLyr );
			MainStatusBarSetText( SB_LAYER, TEXT("") );

			gltLayer.erase( itLyr );

			//SubclassWindow( itLyr->hTextWnd, gpfOrigLyrEditProc );	//	サブクラスを元に戻す

			break;
		}
	}


	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ウィンドウのサイズ変更が完了する前に送られてくる
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	pstWpos	新しい位置と大きさが入ってる
	@return		このMessageを処理したら０
*/
BOOL Lyb_OnWindowPosChanging( HWND hWnd, LPWINDOWPOS pstWpos )
{
	INT		clPosY, vwTopY, dSabun, dRem;
	BOOLEAN	bMinus = FALSE;
	RECT	vwRect;


	//TRACE( TEXT("WM_WINDOWPOSCHANGING POS[%d %d] SIZE[%d %d] FLAG[%d]"), 
	//	pstWpos->x, pstWpos->y, pstWpos->cx, pstWpos->cy, pstWpos->flags );

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

//	TRACE( TEXT("NH[%d]"), pstWpos->y );

	return FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	ウィンドウのサイズ変更が完了したら送られてくる
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	pstWpos	新しい位置と大きさが入ってる
*/
VOID Lyb_OnWindowPosChanged( HWND hWnd, const LPWINDOWPOS pstWpos )
{
	BOOLEAN	bHit = FALSE;
	LAYER_ITR	itLyr;
	RECT	vwRect, rect;

	//TRACE( TEXT("WM_WINDOWPOSCHANGED POS[%d %d] SIZE[%d %d] FLAG[%d]"), 
	//	pstWpos->x, pstWpos->y, pstWpos->cx, pstWpos->cy, pstWpos->flags );


	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hWnd ){	bHit = TRUE;	break;	}
	}
	if( !(bHit) )	return;

	GetClientRect( hWnd, &rect );
	MoveWindow( itLyr->hToolWnd, 0, 0, 0, 0, TRUE );	//	ツールバーは数値なくても勝手に合わせてくれる
	SetWindowPos( itLyr->hTextWnd, HWND_TOP, 0, 0, rect.right, rect.bottom - gdToolBarHei, SWP_NOMOVE | SWP_NOZORDER );

	//	移動がなかったときは何もしないでおｋ
	if( SWP_NOMOVE & pstWpos->flags )	return;

	GetWindowRect( ghViewWnd, &vwRect );
	gstViewOrigin.x = vwRect.left;//位置記録・そうそう変わるものじゃない
	gstViewOrigin.y = vwRect.top;

	itLyr->stOffset.x = pstWpos->x - vwRect.left;
	itLyr->stOffset.y = pstWpos->y - vwRect.top;

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	動かされているときに発生・マウスでウインドウドラッグ中とか
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	pstPos	その瞬間のスクリーン座標
*/
VOID Lyb_OnMoving( HWND hWnd, LPRECT pstPos )
{
	LONG	xEt, yEt, xLy, yLy, xSb, ySb;
	LONG	dLine, dRema;
	BOOLEAN	bMinus = FALSE;
	TCHAR	atBuffer[SUB_STRING];

	//	レイヤボックスウインドウのスクリーン座標左上と右下
//	TRACE( TEXT("WM_MOVING [L%d T%d R%d B%d]"), pstPos->left, pstPos->top, pstPos->right, pstPos->bottom );
	//	レイヤコンテンツの左上スクリーン座標
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

	StringCchPrintf( atBuffer, SUB_STRING, TEXT("Layer %d[dot] %d[line]"), xSb, dLine );
	MainStatusBarSetText( SB_LAYER, atBuffer );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	マウスの左ボタンがダウンされたとき・ダブルクルック用・クラススタイルにCS_DBLCLKSを付けないとメッセージ来ない
	@param[in]	hWnd			ウインドウハンドル・ビューのとは限らないので注意セヨ
	@param[in]	fDoubleClick	非０ダブルクルックされた場合
	@param[in]	x				発生したＸ座標値
	@param[in]	y				発生したＹ座標値
	@param[in]	keyFlags		他に押されてるキーについて
*/
VOID Lyb_OnLButtonDown( HWND hWnd, BOOL fDoubleClick, INT x, INT y, UINT keyFlags )
{
	INT			sy, iDot, iLine;
	LAYER_ITR	itLyr;
	RECT		rect;
	BOOLEAN		bGet = FALSE;


	iDot = x;	//	位置合わせ
	sy = y - gdToolBarHei;	if( 0 > sy )	sy = 0;
	iLine = sy / LINE_HEIGHT;

	TRACE( TEXT("マウスボタンダウン[%d][%dx%d(%d)]"), fDoubleClick, iDot, sy, iLine );

	if( !(fDoubleClick) )	 return;	//	ダブウクルックでないと用はない

	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hWnd )
		{
			bGet = TRUE;	break;
		}
	}
	if( !(bGet) )	 return;	//	ヒットしなかった・あり得ないはずだけど

	//	カーソルヒット位置が、連続空白の部分ならばマークを反転させる
	if( LayerTransparentAdjust( itLyr, iDot, iLine ) )
	{
		GetClientRect( hWnd, &rect );
		rect.top    = (iLine * LINE_HEIGHT) + gdToolBarHei;
		rect.bottom = rect.top + LINE_HEIGHT;
		InvalidateRect( hWnd, &rect, TRUE );
	}

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
VOID Lyb_OnContextMenu( HWND hWnd, HWND hWndContext, UINT xPos, UINT yPos )
{
	INT		posX, posY;
	HMENU	hMenu, hSubMenu;
	UINT	dRslt;

	posX = (SHORT)xPos;	//	画面座標はマイナスもありうる
	posY = (SHORT)yPos;

	TRACE( TEXT("LAYER_WM_CONTEXTMENU %d x %d"), posX, posY );

	hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_LAYERBOX_POPUP) );
	hSubMenu = GetSubMenu( hMenu, 0 );

	dRslt = TrackPopupMenu( hSubMenu, 0, posX, posY, 0, hWnd, NULL );	//	TPM_CENTERALIGN | TPM_VCENTERALIGN | 
	DestroyMenu( hMenu );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	透過合成エリアを全選択したり全解除したり
	@param[in]	hWnd	本体ウインドウハンドル・あまり意味はない
	@param[in]	bMode	非０全選択　０全解除
	@return		HRESULT	終了状態コード
*/
HRESULT LayerTransparentToggle( HWND hWnd, UINT bMode )
{
	TCHAR	chb;
	INT_PTR	iLines, iL;
	LETR_ITR	itMozi;
	LAYER_ITR	itLyr;

#ifdef DO_TRY_CATCH
	try{
#endif

	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hWnd ){	break;	}
	}
	if( itLyr == gltLayer.end( ) )	return E_OUTOFMEMORY;

	TRACE( TEXT("透過選択を解除か選択 %u"), bMode );

	//	行数確認
	iLines = itLyr->vcLyrImg.size(  );

	for( iL = 0; iLines > iL; iL++ )
	{
		//	文字をイテレータで確保
		for( itMozi = itLyr->vcLyrImg.at( iL ).vcLine.begin( );
		itMozi != itLyr->vcLyrImg.at( iL ).vcLine.end( ); itMozi++ )
		{
			if( bMode )
			{
				chb = itMozi->cchMozi;
				if( iswspace( chb ) ){	itMozi->mzStyle |=  CT_LYR_TRNC;	}
			}
			else
			{
				itMozi->mzStyle &= ~CT_LYR_TRNC;
			}
		}
		//	全部解除
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
	行数とドット値を受け取って、その場所の
	@param[in]	itLyr	対象レイヤボックスのイテレータ
	@param[in]	dNowDot	今のキャレット・
	@param[in]	rdLine	対象の行番号・絶対０インデックスか
	@return		文字数
*/
INT LayerTransparentAdjust( LAYER_ITR itLyr, INT dNowDot, INT rdLine )
{
	INT_PTR	i, iCount, iLines, iLetter;
	INT		dDotCnt = 0, dPrvCnt = 0, rdWidth = 0;
	TCHAR	ch, chb;
	LETR_ITR	itMozi, itHead, itTail, itTemp;

#ifdef DO_TRY_CATCH
	try{
#endif

	//	行のはみ出しを？
	iLines = itLyr->vcLyrImg.size(  );
	if( 0 >= iLines )	return 0;
	if( iLines <= rdLine )	return 0;

	//	文字数確認
	iCount = itLyr->vcLyrImg.at( rdLine ).vcLine.size(  );
	if( 0 >= iCount )	return 0;

	//	文字をイテレータで確保
	itMozi = itLyr->vcLyrImg.at( rdLine ).vcLine.begin( );

	for( i = 0, iLetter = 0; iCount > i; i++, iLetter++ )
	{
		if( dNowDot <= dDotCnt ){	break;	}

		dPrvCnt = dDotCnt;
		rdWidth = itLyr->vcLyrImg.at( rdLine ).vcLine.at( i ).rdWidth;
		dDotCnt += rdWidth;
	}	//	振り切るようなら末端

	if( iCount <= iLetter )	return 0;

	if(  1 <= iLetter )	//	左文字で判定
	{
		iLetter--;
		itMozi += iLetter;
	}

	ch = itLyr->vcLyrImg.at( rdLine ).vcLine.at( iLetter ).cchMozi;
	//	該当箇所の文字を確認して
	if( !( iswspace( ch ) ) )	return 0;
	//	空白でないなら何もしないでおｋ

	//	その場所から頭方向に辿って、途切れ目を探す
	itHead = itLyr->vcLyrImg.at( rdLine ).vcLine.begin( );
	for( ; itHead != itMozi; itMozi-- )
	{
		chb = itMozi->cchMozi;
		if(  !( iswspace( chb ) ) ){	itMozi++;	break;	}
	}
	if( itHead == itMozi )	//	先頭を確認
	{
		chb = itMozi->cchMozi;
		if(  !( iswspace( chb ) ) ){	itMozi++;	}
	}
	//	非空白文字にヒットしたか、先頭位置である


	//	その場所から、同じグループの所まで確認
	itTail = itLyr->vcLyrImg.at( rdLine ).vcLine.end( );
	for( itTemp = itMozi; itTemp != itTail; itTemp++ )
	{
		chb = itTemp->cchMozi;	//	空白である間は
		if(  !( iswspace( chb ) ) ){	break;	}

		itTemp->mzStyle ^= CT_LYR_TRNC;
	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return  ETC_MSG( ("etc error"), 0 );	}
#endif

	return iLetter;
}
//-------------------------------------------------------------------------------------------------

/*!
	ビューが移動した
	@param[in]	hWnd	本体ウインドウハンドル・あまり意味はない
	@param[in]	state	窓状態・最小化なら違うコトする
	@return		HRESULT	終了状態コード
*/
HRESULT LayerMoveFromView( HWND hWnd, UINT state )
{
	LAYER_ITR	itLyr;
	RECT	vwRect = {0,0,0,0};
	POINT	lyPoint;

	//	最小化時は非表示にするとか	SIZE_MINIMIZED

	if( SIZE_MINIMIZED != state )
	{
		GetWindowRect( ghViewWnd, &vwRect );
		gstViewOrigin.x = vwRect.left;//位置記録
		gstViewOrigin.y = vwRect.top;
	}

	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( SIZE_MINIMIZED == state )
		{
			ShowWindow( itLyr->hBoxWnd, SW_HIDE );
		}
		else
		{
	//		ShowWindow( itLyr->hBoxWnd, SW_SHOW );

			lyPoint.x = itLyr->stOffset.x + vwRect.left;
			lyPoint.y = itLyr->stOffset.y + vwRect.top;
	#ifdef _DEBUG
			SetWindowPos( itLyr->hBoxWnd, HWND_TOP, lyPoint.x, lyPoint.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );
	#else
			SetWindowPos( itLyr->hBoxWnd, HWND_TOPMOST, lyPoint.x, lyPoint.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );
	#endif
		}
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	行の内容を文字列で確保・ポインタ開放は呼んだほうでやる
	@param[in]	itLyr	保持してるイテレータ
	@param[in]	il		行番号
	@return		LPTSTR	文字列のぽいんた～
*/
LPTSTR LayerLineTextGetAlloc( LAYER_ITR itLyr, INT il )
{
	UINT_PTR	cchSize, i = 0;
	LPTSTR	ptText;

	cchSize = itLyr->vcLyrImg.at( il ).vcLine.size( );
	if( 0 >= cchSize )	return NULL;

	ptText = (LPTSTR)malloc( (cchSize+1) * sizeof(TCHAR) );	//	ぬるたーみねーた分増やす
	ZeroMemory( ptText, (cchSize+1) * sizeof(TCHAR) );

	for( i = 0; cchSize > i; i++ )
	{
		ptText[i] = itLyr->vcLyrImg.at( il ).vcLine.at( i ).cchMozi;
	}

	return ptText;
}
//-------------------------------------------------------------------------------------------------

/*!
	対象のレイヤボックスの保持してる文字列を破壊する
	@param[in]	itLyr	対象のレイヤボックスを示すイテレータ
	@return		HRESULT	終了状態コード
*/
HRESULT LayerStringObliterate( LAYER_ITR itLyr )
{
	UINT_PTR	j, iLine;

	iLine = itLyr->vcLyrImg.size( );
	for( j = 0; iLine > j; j++ )
	{
		itLyr->vcLyrImg.at( j ).vcLine.clear( );	//	各行の中身全消し
	}
	itLyr->vcLyrImg.clear(  );	//	行を全消し

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	レイヤボックスの中身の編集をON/OFF
	@param[in]	hWnd	レイヤボックスのウインドウハンドル
	@param[in]	dStyle	非０編集する　０終了
	@return		HRESULT	終了状態コード
*/
HRESULT LayerEditOnOff( HWND hWnd, UINT dStyle )
{
	UINT_PTR	i, iLines;
	INT			ndx;
	UINT		cchSize;
	LPTSTR		ptStr;
	ONELINE		stLine;
	LAYER_ITR	itLyr;


	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hWnd )
		{
			if( dStyle )	//	編集する
			{
		//		SetLayeredWindowAttributes( hWnd, 0, 0xFF, LWA_ALPHA );
				Edit_SetText( itLyr->hTextWnd, TEXT("") );

				SetFocus( itLyr->hTextWnd );

				iLines = itLyr->vcLyrImg.size( );
				for( i = 0; iLines > i; i++ )
				{
					if( 0 != i )	//	次の行に進むようなら改行いれとく
					{
						ndx = GetWindowTextLength( itLyr->hTextWnd );
						SendMessage( itLyr->hTextWnd, EM_SETSEL, ndx, ndx );
						SendMessage( itLyr->hTextWnd, EM_REPLACESEL, 0, (LPARAM)(CH_CRLFW) );
					}

					//	壱行ずつ中身を取って
					ptStr = LayerLineTextGetAlloc( itLyr, i );
					if( ptStr )
					{
						ndx = GetWindowTextLength( itLyr->hTextWnd );
						SendMessage( itLyr->hTextWnd, EM_SETSEL, ndx, ndx );
						SendMessage( itLyr->hTextWnd, EM_REPLACESEL, 0, (LPARAM)ptStr );

						FREE(ptStr);
					}
				}

				ShowWindow( itLyr->hTextWnd, SW_SHOW );
			}
			else	//	終了
			{
				ndx = Edit_GetTextLength( itLyr->hTextWnd );
				ndx += 2;	//	ぬるたみねた分
				ptStr = (LPTSTR)malloc( ndx * sizeof(TCHAR) );
				ZeroMemory( ptStr, ndx * sizeof(TCHAR) );
				Edit_GetText( itLyr->hTextWnd, ptStr, ndx );
				ShowWindow( itLyr->hTextWnd, SW_HIDE );

				StringCchLength( ptStr, STRSAFE_MAX_CCH, &cchSize );

				LayerStringObliterate( itLyr );
				ZeroONELINE( &stLine );
				itLyr->vcLyrImg.push_back( stLine );	//	壱発目

				LayerBoxSetString( itLyr, ptStr, cchSize, NULL, 0x00 );

				FREE(ptStr);

				InvalidateRect( hWnd, NULL, TRUE );
		//		SetLayeredWindowAttributes( hWnd, 0, gbAlpha, LWA_ALPHA );
			}

			break;
		}
	}

	UpdateWindow( hWnd );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	レイヤボックスの内容を入れ替える・外部から？
	@param[in]	hLyrWnd	レイヤボッキスウインドウのハンドル
	@param[in]	ptStr	表示する文字列
	@return		HRESULT	終了状態コード
*/
HRESULT LayerStringReplace( HWND hLyrWnd, LPTSTR ptStr )
{
	UINT		cchSize;
	ONELINE		stLine;
	LAYER_ITR	itLyr;

	//	イテレータで探して処理する
	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hLyrWnd )
		{
			StringCchLength( ptStr, STRSAFE_MAX_CCH, &cchSize );

			LayerStringObliterate( itLyr );
			ZeroONELINE( &stLine );
			itLyr->vcLyrImg.push_back( stLine );	//	壱発目

			LayerBoxSetString( itLyr, ptStr, cchSize, NULL, 0x00 );

			break;
		}
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	文字列を表示内容にする
	@param[in]	itLyr	レイヤボッキスのイテレータ
	@param[in]	ptStr	表示する文字列
	@return		HRESULT	終了状態コード
*/
HRESULT LayerFromString( LAYER_ITR itLyr, LPCTSTR ptStr )
{
	UINT	cchSize;
	ONELINE	stLine;

	ZeroONELINE( &stLine );

	itLyr->vcLyrImg.push_back( stLine );	//	壱発目

	StringCchLength( ptStr, STRSAFE_MAX_CCH, &cchSize );

	LayerBoxSetString( itLyr, ptStr, cchSize, NULL, 0x01 );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	表示内容を選択範囲から頂戴する
	@param[in]	itLyr	レイヤボッキスのイテレータ
	@param[in]	bSqSel	矩形選択中であるか
	@return		HRESULT	終了状態コード
*/
HRESULT LayerFromSelectArea( LAYER_ITR itLyr, UINT bSqSel )
{
	LPTSTR	ptString = NULL;
	UINT	cchSize, cbSize;
	LPPOINT	pstPos;
	ONELINE	stLine;

	TRACE( TEXT("選択範囲から取得") );
#ifdef DO_TRY_CATCH
	try{
#endif

	ZeroONELINE( &stLine );

	itLyr->vcLyrImg.push_back( stLine );	//	壱発目

	cbSize = DocSelectTextGetAlloc( D_UNI | bSqSel, (LPVOID *)(&ptString), &pstPos );

	StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSize );

	LayerBoxSetString( itLyr, ptString, cchSize, (bSqSel & D_SQUARE) ? pstPos : NULL, 0x01 );

	FREE(ptString);
	FREE(pstPos);

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif
	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	表示内容をくるっぷぼーどから頂戴する
	@param[in]	itLyr	保持してるイテレータ
	@return		HRESULT	終了状態コード
*/
HRESULT LayerFromClipboard( LAYER_ITR itLyr )
{
	LPTSTR	ptString = NULL;
	UINT	cchSize, dStyle;//, i;
//	INT		insDot, yLine;
	ONELINE	stLine;

	ZeroONELINE( &stLine );

	itLyr->vcLyrImg.push_back( stLine );	//	壱発目

	//	くるっぺぼーどからの場合は、矩形でも関係ない
	ptString = DocClipboardDataGet( &dStyle );

	StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSize );

	LayerBoxSetString( itLyr, ptString, cchSize, NULL, 0x01 );

	FREE( ptString );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ボックス内容に合わせてサイズ広げる
	@param[in]	itLyr	保持してるイテレータ
	@return		HRESULT	終了状態コード
*/
HRESULT LayerBoxSizeAdjust( LAYER_ITR itLyr )
{
	INT	dViewXdot, dYline, dViewYdot;
	INT	iMaxDot = 0, iYdot;
	INT_PTR	iLine, i;
	SIZE	wdSize, tgtSize;//clSize

#ifdef DO_TRY_CATCH
	try{
#endif
	//	最小サイズってことで
	//clSize.cx = LB_WIDTH  - gstFrmSz.x;
	//clSize.cy = LB_HEIGHT - gstFrmSz.y;

	//	今の画面の行数とドット数確認
	dYline = ViewAreaSizeGet( &dViewXdot );
	dViewYdot = dYline * LINE_HEIGHT;

	//	使っている内容からサイズを確認
	iLine = itLyr->vcLyrImg.size(  );
	iYdot = iLine * LINE_HEIGHT;
	for( i = 0; iLine > i; i++ )	//	最大ドット数を確認
	{
		if( iMaxDot < itLyr->vcLyrImg.at( i ).iDotCnt ){	iMaxDot = itLyr->vcLyrImg.at( i ).iDotCnt;	}
	}
	//	多分ウインドウサイズになるはず
	wdSize.cx = gstFrmSz.x + iMaxDot + gstFrmSz.x;
	wdSize.cy = gstFrmSz.y + iYdot + gstFrmSz.x;

	if( LB_WIDTH >  wdSize.cx ){	tgtSize.cx = LB_WIDTH;	}
	else if( dViewXdot < wdSize.cx ){	tgtSize.cx =  dViewXdot;	}
	else{	tgtSize.cx =  wdSize.cx;	}

	if( LB_HEIGHT > wdSize.cy ){	tgtSize.cy =  LB_HEIGHT;	}
	else if( dViewYdot < wdSize.cy ){	tgtSize.cy =  dViewYdot;	}
	else{	tgtSize.cy =  wdSize.cy;	}

#ifdef _DEBUG
	SetWindowPos( itLyr->hBoxWnd, HWND_TOP, 0, 0, tgtSize.cx, tgtSize.cy, SWP_NOMOVE | SWP_NOZORDER );
#else
	SetWindowPos( itLyr->hBoxWnd, HWND_TOPMOST, 0, 0, tgtSize.cx, tgtSize.cy, SWP_NOMOVE | SWP_NOZORDER );
#endif

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	レイヤボックスに文字列を記録する
	@param[in]	itLyr	対象レイヤボッキスのイテレータ
	@param[in]	ptText	記録する文字列
	@param[in]	cchSize	文字列の文字数
	@param[in]	pstPt	矩形のときのオフセット量
	@param[in]	bStyle	非０内容に合わせてサイズ変更　０ナニもしない
	@return		HRESULT	終了状態コード
*/
HRESULT LayerBoxSetString( LAYER_ITR itLyr, LPCTSTR ptText, UINT cchSize, LPPOINT pstPt, UINT bStyle )
{
	UINT_PTR	i, j, iLine, iTexts;
	LONG	dMin = 0;
	INT		insDot, yLine, dSpDot, dSpMozi, iLines = 0, dOffset;
	LPTSTR	ptBuff, ptSpace = NULL;
	ONELINE	stLine;

#ifdef DO_TRY_CATCH
	try{
#endif
	ZeroONELINE( &stLine );

	//	オフセット設定が有る場合、その分を埋める空白が必要
	if( pstPt )	//	最小オフセット値を探して、そこを左端にする
	{
		dMin = pstPt[0].x;

		yLine = 0;
		for( i = 0; cchSize > i; i++ )
		{
			if( CC_CR == ptText[i] && CC_LF == ptText[i+1] )	//	改行であったら
			{
				//	オフセット最小をさがす
				if( dMin > pstPt[yLine].x ){	dMin = pstPt[yLine].x;	}

				i++;		//	0x0D,0x0Aだから、壱文字飛ばすのがポイント
				yLine++;	//	改行したからFocusは次の行へ
			}
		}
		//	この時点で、yLineは行数になってる
		iLines = yLine;

		//	壱行目の空白を作って閃光入力しておく
		insDot = 0;
		dOffset = pstPt[0].x - dMin;
		ptSpace = DocPaddingSpaceUni( dOffset, NULL, NULL, NULL );
		//	前方空白は無視されるのでユニコード使って問題無い
		StringCchLength( ptSpace, STRSAFE_MAX_CCH, &iTexts );
		for( j = 0; iTexts > j; j++ )
		{
			insDot += LayerInputLetter( itLyr, insDot, 0, ptSpace[j] );
		}
		FREE(ptSpace);
	}

	yLine = 0;	insDot = 0;
	for( i = 0; cchSize > i; i++ )
	{
		if( CC_CR == ptText[i] && CC_LF == ptText[i+1] )	//	改行であったら
		{
			itLyr->vcLyrImg.push_back( stLine );	//	次の行を作る

			i++;		//	0x0D,0x0Aだから、壱文字飛ばすのがポイント
			yLine++;	//	改行したからFocusは次の行へ
			insDot = 0;	//	そして行の先頭である

			//	オフセット分の空白を作る
			if( pstPt && (iLines > yLine) )
			{
				dOffset = pstPt[yLine].x - dMin;
				ptSpace = DocPaddingSpaceUni( dOffset, NULL, NULL, NULL );
				//	前方空白は無視されるのでユニコード使って問題無い
				StringCchLength( ptSpace, STRSAFE_MAX_CCH, &iTexts );
				for( j = 0; iTexts > j; j++ )
				{
					insDot += LayerInputLetter( itLyr, insDot, yLine, ptSpace[j] );
				}
				FREE(ptSpace);
			}

		}
		else if( CC_TAB == ptText[i] )
		{
			//	タブは挿入しない
		}
		else
		{
			insDot += LayerInputLetter( itLyr, insDot, yLine, ptText[i] );
		}
	}

	//	末尾整形と前方空白確認
	iLine = itLyr->vcLyrImg.size( );
	for( i = 0; iLine > i; i++ )
	{
		//	末端空白削除
		ptBuff = DocLastSpDel( &(itLyr->vcLyrImg.at( i ).vcLine) );
		FREE(ptBuff);	//	使わないが、受けて開放しないとイケない

		//	先頭空白確認
		dSpMozi = 0;
		dSpDot = LayerHeadSpaceCheck( &(itLyr->vcLyrImg.at( i ).vcLine), &dSpMozi );

		itLyr->vcLyrImg.at( i ).dFrtSpDot  = dSpDot;
		itLyr->vcLyrImg.at( i ).dFrtSpMozi = dSpMozi;

		//	矩形オフセット
		//if( pstPt ){	itLyr->vcLyrImg.at( i ).dOffset = pstPt[i].x - dMin;	}
	}

	//	サイズ調整
	if( bStyle ){	LayerBoxSizeAdjust( itLyr );	}

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	行頭空白確認・ピリオドは空白と見なす・先頭のみ、とかオプソンできるかも
	@param[in]	*vcTgLine	該当する行のベクターへのポインタ～
	@param[out]	pdMozi		文字数を入れるポインタ～
	@return	非空白に至るまでのドット数
*/
INT LayerHeadSpaceCheck( vector<LETTER> *vcTgLine, PINT pdMozi )
{
	TCHAR		ch;
	INT			cchSp, dDot;	//	文字数とドット数
	UINT_PTR	i, iMozi;

#ifdef DO_TRY_CATCH
	try{
#endif
	iMozi = vcTgLine->size(  );

	dDot = 0;	cchSp = 0;
	for( i = 0; iMozi > i; i++ )
	{
		ch = vcTgLine->at( i ).cchMozi;

		//	字がスペースでもピリオドでもないなら、余白はそこまで
		if( !( iswspace(ch) ) && TEXT('.') != ch )
		{
			if( pdMozi ){	*pdMozi = cchSp;	}
			return dDot;
		}

		dDot += vcTgLine->at( i ).rdWidth;	//	ドット数
		cchSp++;	//	文字数
	}

	if( pdMozi ){	*pdMozi = cchSp;	}
#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return  ETC_MSG( ("etc error"), 0 );	}
#endif

	return dDot;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定行のドット位置(キャレット位置)に壱文字追加する
	@param[in]	itLyr	対象レイヤボッキスのイテレータ
	@param[in]	nowDot	挿入するドット位置・使ってない
	@param[in]	rdLine	対象の行番号・絶対０インデックスか
	@param[in]	ch		追加したい文字
	@return		INT		追加した文字のドット数
*/
INT LayerInputLetter( LAYER_ITR itLyr, INT nowDot, INT rdLine, TCHAR ch )
{
	LETTER	stLetter;
//	INT		iRslt;

#ifdef DO_TRY_CATCH
	try{
#endif
	//	データ作成
	DocLetterDataCheck( &stLetter, ch );	//	指定行のドット位置(キャレット位置)に壱文字追加する・レイヤボックス

	itLyr->vcLyrImg.at( rdLine ).vcLine.push_back( stLetter );

	itLyr->vcLyrImg.at( rdLine ).iDotCnt += stLetter.rdWidth;
	itLyr->vcLyrImg.at( rdLine ).iByteSz += stLetter.mzByte;

//	iRslt = DocBadSpaceCheck( rdLine );

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return ETC_MSG( err.what(), 0 );	}
	catch( ... ){	return  ETC_MSG( ("etc error"), 0 );	}
#endif

	return stLetter.rdWidth;
}
//-------------------------------------------------------------------------------------------------

/*!
	該当エリアに上書きしたり挿入したり
	@param[in]	hWnd	ボックスのウインドウハンドル
	@param[in]	cmdID	挿入か上書きか
	@param[out]	pXdot	入れ込み位置Ｘドット・NULLでも可
	@param[out]	pYline	入れ込み位置Ｙライン・NULLでも可
	@param[in]	dStyle	不可視特別処理を？
	@return		HRESULT	終了状態コード
*/
HRESULT LayerContentsImportable( HWND hWnd, UINT cmdID, LPINT pXdot, LPINT pYline, UINT dStyle )
{
	RECT		vwRect, lyRect;
	POINT		conPoint;
	INT			xTgDot, xDot, iSrcDot, iSabun, iDivid, iSpDot;
	INT			dGap, dInLen, dInBgn, dInEnd, dEndot;
	INT			dLeft, dRight;
	INT			iPageLine, yTgLine, dWkLine, dLyLine;
	INT			iMinus, iMozi, iStMozi, iEdMozi;
	INT			dBkLeft, dBkRight, dBkStMozi, dBkEdMozi;
	INT_PTR		dNeedLine;
	UINT_PTR	cchSize;
	LPTSTR		ptStr, ptBuffer;
	BOOLEAN		bFirst = TRUE;	//	なんか処理したらFALSE
	BOOLEAN		bSpace, bBkSpase;

#ifdef EDGE_BLANK_STYLE
	INT		bEdgeBlank;
	INT		xDotEx, iMoziEx;
#endif
	LAYER_ITR	itLyr;

	LETR_ITR	itLtr, itDel;
	wstring	wsBuff;

#ifdef DO_TRY_CATCH
	try{
#endif

	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hWnd ){	break;	}
	}
	if( itLyr == gltLayer.end( ) )	return E_OUTOFMEMORY;


	//	まず場所を確認
	GetWindowRect( ghViewWnd, &vwRect );
	vwRect.left += LINENUM_WID;
	vwRect.top  += RULER_AREA;

	GetWindowRect( itLyr->hBoxWnd, &lyRect );
	conPoint.x = lyRect.left + gstFrmSz.x;
	conPoint.y = lyRect.top  + gstFrmSz.y;
	//	左や上にはみ出してたら、ここはマイナスになっている
	xTgDot   =  conPoint.x - vwRect.left;
	yTgLine  =  conPoint.y - vwRect.top;

	yTgLine /= LINE_HEIGHT;

	//	20110704	この時点では、スクロールによるズレが考慮されてない
	xTgDot  += gdHideXdot;
	yTgLine += gdViewTopLine;
	//	多分これで大丈夫

	xDot = xTgDot;

	TRACE( TEXT("LAYER IMPORT[%d:%d]"), xTgDot, yTgLine );

	if( pXdot  )	*pXdot  = xTgDot;
	if( pYline )	*pYline = yTgLine;


	//	使う行数確認
	dNeedLine = itLyr->vcLyrImg.size( );
	//	最終行の空白確認
	ptStr = LayerLineTextGetAlloc( itLyr, dNeedLine - 1 );
	if( !(ptStr) )	dNeedLine--;	//	最後空白なら使わない
	FREE(ptStr);

	iPageLine = DocPageParamGet( NULL, NULL );	//	この頁の行数確認・入れ替えていけるか

	//	全体行数より、追加行数が多かったら、改行増やす
	if( iPageLine < (dNeedLine + yTgLine) )
	{
		iMinus = ( dNeedLine + yTgLine ) - iPageLine;	//	追加する行数
		DocAdditionalLine( iMinus, &bFirst );//	bFirst = FALSE;
		TRACE( TEXT("ADD LINE[%d]"), iMinus );
	}

	//	白ヌキするには、前後の空白文字量を増やせばいい
	//	透過領域が狭い場合は、非透過とする・先にスキャンするか。
	bEdgeBlank = ComboBox_GetCurSel( GetDlgItem( GetDlgItem(hWnd,IDW_LYB_TOOL_BAR), IDCB_LAYER_EDGE_BLANK ) );
	if( 1 == bEdgeBlank ){			LayerEdgeBlankSizeCheck( hWnd, EDGE_BLANK_NARROW );	}
	else if( 2 ==  bEdgeBlank ){	LayerEdgeBlankSizeCheck( hWnd, EDGE_BLANK_WIDE );	}


	//白ヌキするには狭い透過領域を消す


//各行毎に挿入位置をみて、字をよけて、スキマを埋める
//そこまでに足りないならパディング・レイヤ側のOffsetと行頭ピィリヲド～も考慮
//上書きの場合は、ドット数に合わせてスキマを入れて、前後ズレないように
//挿入・上書きいずれの場合も、レイヤ内の前空白を考慮してパディングする
	for( dWkLine = yTgLine, dLyLine = 0; (yTgLine+dNeedLine) > dWkLine; dWkLine++, dLyLine++ )
	{
		if( 0 > dWkLine )	continue;	//	上にめり込んでるのは処理しちゃいかん

		TRACE( TEXT("Check Line V[%d] L[%d]"), dWkLine, dLyLine );

		//	挿入内容の位置の確認・ここで、各部分毎にばらせばいい。
		//	行単位ではなく、透過領域で区切られた文字領域毎に判定する

		//	dLyLine：レイヤ内の行番号　dWkLine：ビューの行番号
		iSpDot  = itLyr->vcLyrImg.at( dLyLine ).dFrtSpDot;	//	レイヤ内ドットオフセット
		//	行頭から、初めて非空白が出てくるドット

		xDot   = xTgDot + iSpDot;	//	レイヤ内オブジェクトを挿入位置
		//	マイナスだった場合レイヤ内文字列の開始位置をずらす

		//	必要な所を抽出・使用バイト数も確認しておく
		itLtr  = itLyr->vcLyrImg.at( dLyLine ).vcLine.begin( );
		itLtr += itLyr->vcLyrImg.at( dLyLine ).dFrtSpMozi;	//	空白以外の開始位置
		//ここで開始位置までずらしている

		//	壱行ずつ状況をみながら挿入していく
		while( itLtr != itLyr->vcLyrImg.at( dLyLine ).vcLine.end( ) )
		{
			while( 0 > xDot )	//	マイナスだったら＋になるまでずらしていく
			{
				//	該当行の末尾までイッたら終了
				if( itLtr == itLyr->vcLyrImg.at( dLyLine ).vcLine.end( ) )	break;

				//透過フラグがあれば終了
				if( itLtr->mzStyle & CT_LYR_TRNC )	break;

				xDot   += itLtr->rdWidth;
				iSpDot += itLtr->rdWidth;

				itLtr++;
			}

			//	挿入内容の確保
			wsBuff.clear( );	dInLen = 0;
			for(  ; itLtr != itLyr->vcLyrImg.at( dLyLine ).vcLine.end( ); itLtr++ )
			{
				//透過フラグがあれば終了
				if( itLtr->mzStyle & CT_LYR_TRNC )	break;

				wsBuff += itLtr->cchMozi;	dInLen += itLtr->rdWidth;
			}	//	そこから終わりまで

			if( 0 != dInLen )	//	挿入できる内容がなかったらなにもせんでいい
			{
				cchSize = wsBuff.size( ) + 1;	//	dInLen：挿入内容のドット幅
				ptStr = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
				StringCchCopy( ptStr, cchSize, wsBuff.c_str( ) );

				dGap = 0;

				//	挿入位置の調整
				iSrcDot = DocLineParamGet( dWkLine, NULL, NULL );	//	挿入行の末端ドット位置
				iSabun  = xTgDot - iSrcDot;	//	＋なら足りてない
				iDivid  = iSabun + iSpDot;	//	レイヤ内も考慮
				if( 0 < iDivid )	//	行末端より後にきてる
				{
					xDot = iSrcDot;
					ptBuffer = DocPaddingSpaceWithPeriod( iDivid, NULL, NULL, NULL, TRUE );
					//	行末端からレイヤ内オブジェクトまでを埋める空白
					if( ptBuffer )
					{
						DocInsertString( &xDot, &dWkLine, NULL, ptBuffer, dStyle, bFirst );	bFirst = FALSE;
						FREE(ptBuffer);
					}

					//余裕があるなら、白ヌキはあまり関係ないか？
				}
				else if( 0 > iDivid )	//	既存の文字列のほうが長い場合
				{
					//	その地点の状況を確認して、空白エリアなら埋めに使う
					//	文字エリアなら、直近からパディングできるところまでを埋め直す
					iMozi = DocLetterPosGetAdjust( &xDot, dWkLine, -1 );	//	今の文字位置を確認
				//	iMozi：挿入位置文字数			xDot：文字列挿入位置ドット

#ifndef EDGE_BLANK_STYLE
					//	そこの文字が空白か、空白ならどこまで続いてるか確認
					DocLineStateCheckWithDot( xDot, dWkLine, &dLeft, &dRight, &iStMozi, NULL, &bSpace );
					//	dRight 使ってない
#endif
					//	先に上書きエリアの処理しないと、パディング直したらずれる
					//	上書きの場合ここから先をさらに削除してギャップパディング
					if( IDM_LYB_OVERRIDE == cmdID )
					{
						dInBgn  = xTgDot + iSpDot;	//	ボックス左端＋内部オフセット＝文字列開始位置
						dInEnd  = dInBgn + dInLen;	//	開始位置＋文字列幅＝文字列終端位置

//白抜くには、ここで範囲を広げればいい？
//透過領域の幅と巻き込む範囲によっては、前部分を巻き込む可能性？
//先にピタリの位置に合わせてあるので、問題は無い？

						//	20110817	時々ズレが出るのを修正＜処理手順間違い
						dEndot  = dInEnd;
#ifdef EDGE_BLANK_STYLE
						//	ここで dEndot をオフセットする？
						if( 1 == bEdgeBlank ){		dEndot += EDGE_BLANK_NARROW;	}
						else if( 2 == bEdgeBlank ){	dEndot += EDGE_BLANK_WIDE;	}
#endif
						iEdMozi = DocLetterPosGetAdjust( &dEndot , dWkLine, 1 );	//	上書きされる領域
						//	キャレット位置修正

						//	後半の、あまってるSpaceも考慮してパディング調整
						DocLineStateCheckWithDot( dEndot, dWkLine, &dBkLeft, &dBkRight, &dBkStMozi, &dBkEdMozi, &bBkSpase );
						if( bBkSpase )	//	後半の空白再編
						{
							dEndot  = dBkRight;
							iEdMozi = DocLetterPosGetAdjust( &dEndot , dWkLine, 1 );	//	上書きされる領域
							//↑は要らないかもだ
							dGap    = dBkRight - dInEnd;	//	元の部分の維持のためのギャップ量
							//	dBkRight：空白位置末端　dInEnd：挿入文字列リアル末端
						}
						else
						{
							//dEndot  = dInEnd;
							//iEdMozi = DocLetterPosGetAdjust( &dEndot , dWkLine, 1 );	//	上書きされる領域
							dGap    = dEndot - dInEnd;	//	元の部分の維持のためのギャップ量
							//	dEndot：挿入文字列キャレット末端　dInEnd：挿入文字列リアル末端
						}

						//	該当部分を削除
						DocRangeDeleteByMozi( xDot, dWkLine, iMozi, iEdMozi, &bFirst );

						if( 0 < dGap )
						{
							dInBgn  = xDot;
							//	レイヤ内オブジェクト末端から元絵の開始位置までを埋める数ドットの空白
							ptBuffer = DocPaddingSpaceWithPeriod( dGap, NULL, NULL, NULL, TRUE );
							if( ptBuffer )
							{
								DocInsertString( &dInBgn, &dWkLine, NULL, ptBuffer, dStyle, bFirst );	bFirst = FALSE;
								FREE(ptBuffer);
							}
						}
					}

#ifdef EDGE_BLANK_STYLE
					if( bEdgeBlank )
					{
						//	オフセット位置確認
						xDotEx  = (xTgDot + iSpDot);

						if( 1 == bEdgeBlank ){		xDotEx -= EDGE_BLANK_NARROW;	}
						else if( 2 == bEdgeBlank ){	xDotEx -= EDGE_BLANK_WIDE;	}
						else{	;	}

						if( 0 > xDotEx ){	xDotEx =  0;	}

						iMoziEx = DocLetterPosGetAdjust( &xDotEx, dWkLine, -1 );	//	今の文字位置を確認
					//	iMoziEx：挿入位置文字数			xDotEx：挿入位置オフセット

						//	そこの文字が空白か、空白ならどこまで続いてるか確認
						DocLineStateCheckWithDot( xDotEx, dWkLine, &dLeft, &dRight, &iStMozi, NULL, &bSpace );

						dGap = (xTgDot + iSpDot) - xDotEx;	//	前側の埋め処理
						xDot = xDotEx;
					}
					else
					{
						//	そこの文字が空白か、空白ならどこまで続いてるか確認
						DocLineStateCheckWithDot( xDot, dWkLine, &dLeft, &dRight, &iStMozi, NULL, &bSpace );
						iMoziEx = iStMozi;	//	特に意味はない
#endif
						dGap = (xTgDot + iSpDot) - xDot;	//	前側の埋め処理
#ifdef EDGE_BLANK_STYLE
					}
#endif
					if( bSpace )	//	空白なら、ギャップ分と合わせて入れ直す
					{
						dGap  += (xDot - dLeft);	//	パディングドット数
						//	既存の空白を一旦削除して
						DocRangeDeleteByMozi( dLeft, dWkLine, iStMozi, iMozi, &bFirst );
					}
					else	//	文字であるなら、なにもしない
					{
#ifdef EDGE_BLANK_STYLE
						//	必要なら、既存の文字列を一旦削除して
						if( bEdgeBlank )	DocRangeDeleteByMozi( xDot, dWkLine, iMoziEx, iMozi, &bFirst );
#endif
						dLeft = xDot;	//	ギャップ開始位置
					}

					ptBuffer = DocPaddingSpaceWithPeriod( dGap, NULL, NULL, NULL, TRUE );
					if( ptBuffer )
					{
						DocInsertString( &dLeft, &dWkLine, NULL, ptBuffer, dStyle, bFirst );	bFirst = FALSE;
						FREE(ptBuffer);
					}

					xDot = dLeft;	//	挿入位置であるように
				}
				else	//	末端位置にピタリである
				{
					//	特にすることない？
				}


				//	該当の場所へ文字列挿入
				DocInsertString( &xDot, &dWkLine, NULL, ptStr, dStyle, bFirst );	bFirst = FALSE;
				//	xDotには挿入終端ドットが入る

				FREE(ptStr);
			}

			//	この時点で、itLtrはendか透過開始位置である・xDotは透過開始ドットである
			if( itLtr == itLyr->vcLyrImg.at( dLyLine ).vcLine.end( ) )	break;
			//	終わってたらこの行の処理終わり

			//	透過しないところまで進める
			for(  ; itLtr != itLyr->vcLyrImg.at( dLyLine ).vcLine.end( ); itLtr++ )
			{
				//透過フラグが無くなれば終了
				if( !(itLtr->mzStyle & CT_LYR_TRNC) )	break;
				xDot += itLtr->rdWidth;
			}	//	そこから終わりまで

			iSpDot = xDot;	//	挿入開始位置を調整
			iSpDot -= xTgDot;
		}

		//DocBadSpaceCheck( dWkLine );	//	バッド空白チェキ
		//	DocInsertStringの中の方でやってるので、ここでは不要だと思われ
	}

	TRACE( TEXT("Layer Insert OK！") );

	//	最終的なキャレットの位置をリセット
	//	ViewPosResetCaret( xDot , dWkLine-1 );
	//	ここでは処理しない
#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	レイヤボックスの内容をクルップする
	@param[in]	hWnd	ボックスのウインドウハンドル
	@param[in]	bStyle	ユニコードかシフトJISで
	@return		HRESULT	終了状態コード
*/
HRESULT LayerForClipboard( HWND hWnd, UINT bStyle )
{
	INT_PTR	iLines, iL, cchSize, cbSize;
	LETR_ITR	itMozi;

	LAYER_ITR	itLyr;

	string	srString;
	wstring	wsString;

	//	該当のレイヤーボックスを確認
	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hWnd ){	break;	}
	}
	if( itLyr == gltLayer.end( ) )	return E_OUTOFMEMORY;


	iLines = itLyr->vcLyrImg.size( );	//	行数確認

	srString.clear( );
	wsString.clear( );

	for( iL = 0; iLines > iL; iL++ )
	{
		//	文字をイテレータで確保
		for( itMozi = itLyr->vcLyrImg.at( iL ).vcLine.begin( ); itMozi != itLyr->vcLyrImg.at( iL ).vcLine.end( ); itMozi++ )
		{
			srString += string( itMozi->acSjis );
			wsString += itMozi->cchMozi;
		}

		srString +=  string( CH_CRLFA );
		wsString += wstring( CH_CRLFW );
	}

	if( bStyle & D_UNI )	//	ユニコードである
	{
		cchSize = wsString.size( ) + 1;
		DocClipboardDataSet( (LPTSTR)(wsString.c_str()), cchSize * sizeof(TCHAR), bStyle );
	}
	else
	{
		cbSize = srString.size( ) + 1;
		DocClipboardDataSet( (LPSTR)(srString.c_str()), cbSize, bStyle );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	レイヤボックスの内容を削除する
	@param[in]	hWnd	ボックスのウインドウハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT LayerOnDelete( HWND hWnd )
{
	ONELINE		stLine;

	LAYER_ITR	itLyr;

#ifdef DO_TRY_CATCH
	try{
#endif

	//	該当のレイヤーボックスを確認
	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hWnd ){	break;	}
	}
	if( itLyr == gltLayer.end( ) )	return E_OUTOFMEMORY;

	LayerStringObliterate( itLyr );	//	中身破壊
	ZeroONELINE( &stLine );
	itLyr->vcLyrImg.push_back( stLine );	//	空データを作成しておく

	InvalidateRect( hWnd, NULL, TRUE );

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

#ifdef EDGE_BLANK_STYLE

/*!
	白抜き指定したときに、幅の狭い透過領域をキャンセルする
	@param[in]	hWnd	ボックスのウインドウハンドル
	@param[in]	iCanWid	キャンセルする最大幅
	@return		HRESULT	終了状態コード
*/
HRESULT LayerEdgeBlankSizeCheck( HWND hWnd, INT iCanWid )
{
	INT_PTR	iLines;
	INT		iWidth;

	LAYER_ITR	itLyr;

	LYLINE_ITR	itLine;
	LETR_ITR	itMozi, itMzx;

#ifdef DO_TRY_CATCH
	try{
#endif

	//	該当のレイヤーボックスを確認
	for( itLyr = gltLayer.begin(); itLyr != gltLayer.end(); itLyr++ )
	{
		if( itLyr->hBoxWnd == hWnd ){	break;	}
	}
	if( itLyr == gltLayer.end( ) )	return E_OUTOFMEMORY;

	iLines = itLyr->vcLyrImg.size( );	//	行数確認


	//	壱行ずつ見ていく
	for( itLine = itLyr->vcLyrImg.begin( ); itLine != itLyr->vcLyrImg.end( ); itLine++ )
	{
		//	文字をイテレータで確保
		for( itMozi = itLine->vcLine.begin( ); itMozi != itLine->vcLine.end( ); itMozi++ )
		{
			if(  itMozi->mzStyle & CT_LYR_TRNC )	//	透過領域にヒットしたら
			{
				//	その領域の幅をゲッツする
				iWidth = 0;
				for( itMzx = itMozi; itMzx != itLine->vcLine.end( ); itMzx++ )
				{
					if( !(itMzx->mzStyle & CT_LYR_TRNC) )	break;	//	外れたら終わり
					iWidth += itMzx->rdWidth;
				}

				if( iCanWid >=  iWidth )	//	もしちっちゃいなら
				{
					for( ; itMzx != itMozi; itMozi++ )
					{
						itMozi->mzStyle &= ~CT_LYR_TRNC;
					}
				}
				else
				{
					itMozi = itMzx;
				}
				itMozi--;	//	ループ先頭でインクリするため、一旦戻る
			}
		}
	}

	InvalidateRect( hWnd , NULL, TRUE );	//	再描画

#ifdef DO_TRY_CATCH
	}
	catch( exception &err ){	return (HRESULT)ETC_MSG( err.what(), E_UNEXPECTED );	}
	catch( ... ){	return (HRESULT)ETC_MSG( ("etc error") , E_UNEXPECTED );	}
#endif
	return S_OK;
}
//-------------------------------------------------------------------------------------------------

#endif
