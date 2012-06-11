/*! @file
	@brief ブラウザコンポーネント使ってPreviewをします
	このファイルは DocPreview.cpp です。
	@author	SikigamiHNQ
	@date	2011/09/01
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

/*
IEコンポつかうにはATLが要る
ttp://ysmt.blog21.fc2.com/blog-entry-244.html

VCExpressにはATL入ってないので、別口でゲットする

WindowsDriverKit710に入ってる
がいど
http://www.microsoft.com/japan/whdc/DevTools/WDK/WDKpkg.mspx
ダウソ
http://www.microsoft.com/download/en/details.aspx?displaylang=en&id=11800


インクルードファイルパスに、"(DDKフォルダ)\inc\atl71"
ライブラリファイルパスに、"(DDKフォルダ)\lib\atl\i386"を追加。

名前はATL71だけどバージョンは80

リンクでエラーが出るなら、
リンカの追加の依存ファイルに、atlthunk.libを付ける。

*/

#pragma warning( disable : 4995 )	//	ATL内のイケナイ函数警告を中止！

#include <atlbase.h>

#ifndef _ATL
#error IEコンポつかうにはATLが要るのです。あぅあぅ
#endif

//	DDKのATL使うなら必要なようだ
#pragma comment(lib, "atlthunk.lib")


//	ATLを使用するために必要なグローバル変数
//	この変数名は、必ず、この名前にすること。
extern CComModule	_Module;

#include <atlcom.h>
#include <atlhost.h>

#pragma warning( default : 4995 )	//	警告復帰

//#if _MSC_VER >=1500 // VC2008(VC9.0)以降

//	DDKなら、AtlAxWin80 っぽい


#if _ATL_VER == 0x0800
#define ATL_AX_WIN	TEXT("AtlAxWin80")
#else
#error ATLバージョンに合わせて定義する必要があるのです。あぅ。
#endif

//	ATLAXWIN_CLASS	定義されてるけどTEXTになってない

CComModule	_Module;

//	なんか要るみたい
BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP( )

//-------------------------------------------------------------------------------------------------

#define DOC_PREVIEW_CLASS	TEXT("PREVIEW_CLASS")	//!<	プレビュー窓のクラス名

#define PVW_WIDTH	 820	//!<	デフォルト画面幅
#define PVW_HEIGHT	 480	//!<	デフォルト画面高さ
//-------------------------------------------------------------------------------------------------

static CComQIPtr<IHTMLDocument2>	gpDocument;		//!<	ＩＥコンポーネントのなにか
static CComQIPtr<IWebBrowser2>		gpWebBrowser2;	//!<	ＩＥコンポーネントのナニカ
static  HWND	ghIEwnd;							//!<	ＩＥコンポーネントのハンドル



static  HWND	ghPrevWnd;			//!<	このウインドウのハンドル
static HINSTANCE	ghInst;			//!<	現在のインターフェイス

static  HWND	ghToolWnd;			//!<	ツールバー
static HIMAGELIST ghPrevwImgLst;	//!<	ツールバーアイコンのイメージリスト

extern  HWND	ghPrntWnd;				//	親ウインドウハンドル
extern list<ONEFILE>	gltMultiFiles;	//	複数ファイル保持
extern FILES_ITR	gitFileIt;			//	今見てるファイルの本体
extern INT		gixFocusPage;			//	注目中のページ・とりあえず０・０インデックス
//-------------------------------------------------------------------------------------------------

#define TB_ITEMS	1
static  TBBUTTON	gstTBInfo[] = {
	{ 0, IDM_PVW_ALLVW, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE | TBSTYLE_CHECK, {0, 0}, 0, 0 }
};	//	


#define PREV_HEADER_FILE	TEXT("Preview.htm")
#define SEPARATE_TAG	("[SEPARATE]")

static CONST CHAR	gcacWeek[7][4] = { ("日"), ("月"), ("火"), ("水"), ("木"), ("金"), ("土") };

//static  CHAR	gacResLineFmt[] = { ("<br>\r\n%d 名前：名無しの妖精さん[sage] 投稿日：%d/%02d/%02d(%s) %02d:%02d:%02d ID:OrinEdit99<br>\r\n") };

static  CHAR	gacResHeaderFmt[] = { "<dt>%d 名前：名無しの妖精さん[sage] 投稿日：%d/%02d/%02d(%s) %02d:%02d:%02d ID:OrinEdit99</dt> <dd>" };	//	
static  CHAR	gacResFooterFmt[] = { "<br></dd>\r\n" };


static LPSTR	gpcHtmlHdr;
static LPSTR	gpcHtmlFtr;
//-------------------------------------------------------------------------------------------------


LRESULT	CALLBACK PreviewWndProc( HWND, UINT, WPARAM, LPARAM );	//!<	
VOID	Pvw_OnCommand( HWND , INT, HWND, UINT );	//!<	WM_COMMAND の処理
VOID	Pvw_OnSize( HWND , UINT, INT, INT );		//!<	
VOID	Pvw_OnPaint( HWND );						//!<	WM_PAINT の処理・枠線描画とか
VOID	Pvw_OnDestroy( HWND );						//!<	WM_DESTROY の処理・BRUSHとかのオブジェクトの破壊を忘れないように

HRESULT	PreviewHeaderGet( VOID );	//!<	

HRESULT	PreviewPageWrite( INT );	//!<	
//-------------------------------------------------------------------------------------------------

/*!
	プレビューウインドウクラスを作成
	@param[in]	hInstance	アプリのインスタンス
	@param[in]	hParentWnd	親ウインドウのハンドル
	@return	なし
*/
VOID PreviewInitialise( HINSTANCE hInstance, HWND hParentWnd )
{
	WNDCLASSEX	wcex;
	GUID	guid;

	HBITMAP	hImg, hMsq;

	if( hInstance )
	{
		ghInst = hInstance;

		_Module.Init( ObjectMap, hInstance, &guid );

		//	専用のウインドウクラス作成
		ZeroMemory( &wcex, sizeof(WNDCLASSEX) );
		wcex.cbSize			= sizeof(WNDCLASSEX);
		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= PreviewWndProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= hInstance;
		wcex.hIcon			= NULL;
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wcex.lpszMenuName	= NULL;
		wcex.lpszClassName	= DOC_PREVIEW_CLASS;
		wcex.hIconSm		= NULL;

		RegisterClassEx( &wcex );

		ghPrevWnd = NULL;

		PreviewHeaderGet(  );

		CoInitialize( NULL );

		ghPrevwImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 1, 1 );
		hImg = LoadBitmap( ghInst, MAKEINTRESOURCE( (IDBMP_PREVIEW_ALL) ) );
		hMsq = LoadBitmap( ghInst, MAKEINTRESOURCE( (IDBMQ_PREVIEW_ALL) ) );
		ImageList_Add( ghPrevwImgLst , hImg, hMsq );	//	イメージリストにイメージを追加
		DeleteBitmap( hImg );	DeleteBitmap( hMsq );
	}
	else
	{
		//	もし開けたままだったら綺麗に閉じないとメモリリーク
		if( ghPrevWnd ){	SendMessage( ghPrevWnd, WM_CLOSE, 0, 0 );	}

		CoUninitialize( );

		FREE(gpcHtmlHdr);

		ImageList_Destroy( ghPrevwImgLst );
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	テンプレディレクトリに入ってるHeaderFooterファイルの中身をゲットする
*/
HRESULT PreviewHeaderGet( VOID )
{
	TCHAR	atPrevFile[MAX_PATH];

	HANDLE	hFile;
	DWORD	readed;

	INT		iByteSize;
	LPSTR	pcText, pcNext;


	gpcHtmlHdr = NULL;
	gpcHtmlFtr = NULL;

	//	ヘッダ・フッタを確保
	StringCchCopy( atPrevFile, MAX_PATH, ExePathGet() );
	PathAppend( atPrevFile, TEMPLATE_DIR );
	PathAppend( atPrevFile, PREV_HEADER_FILE );

	hFile = CreateFile( atPrevFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile ){	return E_HANDLE;	}

	iByteSize = GetFileSize( hFile, NULL );
	pcText = (LPSTR)malloc( iByteSize + 2 );
	ZeroMemory( pcText, iByteSize + 2 );

	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	ReadFile( hFile, pcText, iByteSize, &readed, NULL );
	CloseHandle( hFile );	//	内容全部取り込んだから開放

	gpcHtmlHdr = pcText;	//	HEADER部分・freeにもつかう

	pcNext = StrStrA( pcText, SEPARATE_TAG );
	pcNext[0] = NULL;	//	HEADERとFOOTERの境界にヌルターミネータ
	pcNext++;
	gpcHtmlFtr = NextLineA( pcNext );	//	次の行からが本番



	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*！
	ウインドウを作成
	@param[in]	iNowPage	プレビュりたい頁番号
	@param[in]	bForeg		非０なら再プレビューのときにフォアグランドにする・０ならしない・プレビュー開いてないなら何もしない
	@return	HRESULT	終了状態コード
*/
HRESULT PreviewVisibalise( INT iNowPage, BOOLEAN bForeg )
{
	HWND	hWnd;
	TCHAR	atBuffer[MAX_STRING];
//	UINT	iIndex;
	RECT	rect;
	RECT	tbRect;

	HRESULT	hRslt;

	CComPtr<IUnknown>	comPunkIE;	
	CComPtr<IDispatch>	pDispatch;
	CComVariant	vEmpty;
	CComVariant	vUrl( TEXT("about:blank") );


	if( ghPrevWnd )	//	已にPreview窓有ったら
	{
		SendMessage( ghToolWnd, TB_CHECKBUTTON, IDM_PVW_ALLVW, FALSE );

		PreviewPageWrite( iNowPage );	//	内容書き換え
		InvalidateRect( ghPrevWnd, NULL, TRUE );

		if( bForeg )	SetForegroundWindow( ghPrevWnd );

		return S_FALSE;
	}

	//	プレビュー開いてないときに、非フォアグランドなら何もしない
	if( !(bForeg) ){	return  E_ABORT;	}


	InitWindowPos( INIT_LOAD, WDP_PREVIEW, &rect );
	if( 0 >= rect.right || 0 >= rect.bottom )	//	幅高さが０はデータ無し
	{
		hWnd = GetDesktopWindow( );
		GetWindowRect( hWnd, &rect );
		rect.left   = ( rect.right  - PVW_WIDTH ) / 2;
		rect.top    = ( rect.bottom - PVW_HEIGHT ) / 2;
		rect.right  = PVW_WIDTH;
		rect.bottom = PVW_HEIGHT;
		InitWindowPos( INIT_SAVE , WDP_PREVIEW, &rect );	//	起動時保存
	}


	ghPrevWnd = CreateWindowEx( WS_EX_TOOLWINDOW, DOC_PREVIEW_CLASS, TEXT("IEコンポーネントによるプレビュー"),
		WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_VISIBLE | WS_SYSMENU,
		rect.left, rect.top, rect.right, rect.bottom, NULL, NULL, ghInst, NULL );

	//ツールバー作る
	ghToolWnd = CreateWindowEx( 0, TOOLBARCLASSNAME, TEXT("toolbar"),
		WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS | CCS_NODIVIDER,
		0, 0, 0, 0, ghPrevWnd, (HMENU)IDW_PVW_TOOL_BAR, ghInst, NULL );

	//	自動ツールチップスタイルを追加
	SendMessage( ghToolWnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS );

	SendMessage( ghToolWnd, TB_SETIMAGELIST, 0, (LPARAM)ghPrevwImgLst );

	SendMessage( ghToolWnd, TB_SETBUTTONSIZE, 0, MAKELPARAM(16,16) );

	SendMessage( ghToolWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );
	//	ツールチップ文字列を設定・ボタンテキストがツールチップになる
	StringCchCopy( atBuffer, MAX_STRING, TEXT("全プレビュースタイル") );	gstTBInfo[0].iString = SendMessage( ghToolWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );

	SendMessage( ghToolWnd , TB_ADDBUTTONS, (WPARAM)TB_ITEMS, (LPARAM)&gstTBInfo );	//	ツールバーにボタンを挿入

	SendMessage( ghToolWnd , TB_AUTOSIZE, 0, 0 );	//	ボタンのサイズに合わせてツールバーをリサイズ
	InvalidateRect( ghToolWnd , NULL, TRUE );		//	クライアント全体を再描画する命令

	//	ツールバーサブクラス化が必要

	GetWindowRect( ghToolWnd, &tbRect );
	tbRect.right  -= tbRect.left;
	tbRect.bottom -= tbRect.top;
	tbRect.left = 0;
	tbRect.top  = 0;

	GetClientRect( ghPrevWnd, &rect );
	rect.top     = tbRect.bottom;
	rect.bottom -= tbRect.bottom;

	AtlAxWinInit(  );

	ghIEwnd = CreateWindowEx( 0, ATL_AX_WIN, TEXT("Shell.Explorer.2"),
		WS_CHILD | WS_VISIBLE, rect.left, rect.top, rect.right, rect.bottom,
		ghPrevWnd, (HMENU)IDW_PVW_VIEW_WNDW, ghInst, NULL );

	//	ActiveXコントロールのインターフェースを要求
	if( SUCCEEDED( AtlAxGetControl( ghIEwnd, &comPunkIE ) ) )
	{
		gpWebBrowser2 = comPunkIE;	//	ポインタに格納

		if( gpWebBrowser2 )
		{
			gpWebBrowser2->Navigate2( &vUrl, &vEmpty, &vEmpty, &vEmpty, &vEmpty );

			while( 1 )
			{
				hRslt = gpWebBrowser2->get_Document( &pDispatch );
				if( SUCCEEDED(hRslt) && pDispatch )
				{
					gpDocument = pDispatch;
					if( gpDocument ){	hRslt = S_OK;	break;	}
				}
				Sleep(100);
			}
		}
		else
		{
			NotifyBalloonExist( TEXT("あぅあぅ、IEコンポーネントを初期化出来なかったのです。あぅあぅ"), TEXT("あぅあぅ"), NIIF_ERROR );
			hRslt = E_ACCESSDENIED;
		}
	}

	if( SUCCEEDED(hRslt)  ){	PreviewPageWrite( iNowPage );	}

	UpdateWindow( ghPrevWnd );

	return hRslt;
}
//-------------------------------------------------------------------------------------------------


/*!
	ウインドウプロシージャ
	@param[in]	hWnd	親ウインドウのハンドル
	@param[in]	message	ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@retval 0	メッセージ処理済み
	@retval no0	ここでは処理せず次に回す
*/
LRESULT CALLBACK PreviewWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_SIZE,    Pvw_OnSize );	
		HANDLE_MSG( hWnd, WM_PAINT,   Pvw_OnPaint );	
		HANDLE_MSG( hWnd, WM_COMMAND, Pvw_OnCommand );	
		HANDLE_MSG( hWnd, WM_DESTROY, Pvw_OnDestroy );

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
VOID Pvw_OnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify )
{
	LRESULT	lRslt;

	switch( id )
	{
		case IDM_PVW_ALLVW:	//	全プレON/OFF
			lRslt = SendMessage( ghToolWnd, TB_ISBUTTONCHECKED, IDM_PVW_ALLVW, 0 );
			if( lRslt )	PreviewPageWrite( -1 );
			else		PreviewPageWrite( gixFocusPage );
			InvalidateRect( ghPrevWnd, NULL, TRUE );
			break;

		default:	break;
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	サイズ変更された
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	state	なにかの状態
	@param[in]	cx		変更されたクライヤント幅
	@param[in]	cy		変更されたクライヤント高さ
*/
VOID Pvw_OnSize( HWND hWnd, UINT state, INT cx, INT cy )
{
	HWND	hWorkWnd;
	RECT	tbRect, rect;

	hWorkWnd = GetDlgItem( hWnd, IDW_PVW_TOOL_BAR );

	MoveWindow( hWorkWnd, 0, 0, 0, 0, TRUE );	//	ツールバーは数値なくても勝手に合わせてくれる

	GetWindowRect( hWorkWnd, &tbRect );
	tbRect.right  -= tbRect.left;
	tbRect.bottom -= tbRect.top;
	tbRect.left = 0;
	tbRect.top  = 0;

	GetClientRect( ghPrevWnd, &rect );
	rect.top     = tbRect.bottom;
	rect.bottom -= tbRect.bottom;

	hWorkWnd = GetDlgItem( hWnd, IDW_PVW_VIEW_WNDW );

	MoveWindow( hWorkWnd, rect.left, rect.top, rect.right, rect.bottom, TRUE );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	PAINT。無効領域が出来たときに発生。背景の扱いに注意。背景を塗りつぶしてから、オブジェクトを描画
	@param[in]	hWnd	親ウインドウのハンドル
	@return		無し
*/
VOID Pvw_OnPaint( HWND hWnd )
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
VOID Pvw_OnDestroy( HWND hWnd )
{
	RECT	rect;

	//	ウインドウ位置も記録
	GetWindowRect( ghPrevWnd, &rect );
	rect.right  -= rect.left;
	rect.bottom -= rect.top;
	InitWindowPos( INIT_SAVE , WDP_PREVIEW, &rect );	//	終了時保存

	ghPrevWnd = NULL;

	gpDocument.Release( );

	gpWebBrowser2.Release( );

	if( hWnd )	PostMessage( ghPrntWnd, WMP_PREVIEW_CLOSE, 0, 0 );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	頁を表示する
	@param[in]	iViewPage	頁番号・－１なら全頁
*/
HRESULT PreviewPageWrite( INT iViewPage )
{
	HRESULT	hRslt;

	VARIANT		*param;
	SAFEARRAY	*sfArray;

	UINT_PTR	szSize;
	INT		szCont, bstrLen;
	INT_PTR	iPage, i;
	LPSTR	pcContent;

	CHAR	acSeper[MAX_STRING];
	BSTR	bstr;

	string	asString;

	SYSTEMTIME	stTime;

	hRslt = S_OK;

	if( !(gpcHtmlHdr) )
	{
		NotifyBalloonExist( TEXT("プレビュー用テンプレートファイルが見つからないのです。あぅあぅ"), TEXT("あぅあぅ"), NIIF_ERROR );
		return E_HANDLE;
	}

	//	表示内容をＳＪＩＳでつくる
	GetLocalTime( &stTime );

	if( 0 >  iViewPage )	//	全プレ
	{
		asString  = string( gpcHtmlHdr );

		iPage = DocNowFilePageCount(  );	//	頁数確認して
		for( i = 0; iPage > i; i++ )
		{
			StringCchPrintfA( acSeper, MAX_STRING, gacResHeaderFmt, (i+1),
				stTime.wYear, stTime.wMonth, stTime.wDay,
				gcacWeek[stTime.wDayOfWeek],
				stTime.wHour, stTime.wMinute, stTime.wSecond );

			pcContent = DocPageTextPreviewAlloc( i, &szCont );

			asString += string( acSeper );
			asString += string( pcContent );
			asString += string( gacResFooterFmt );

			FREE(pcContent);
		}

		asString += string( gpcHtmlFtr );
	}
	else
	{
		StringCchPrintfA( acSeper, MAX_STRING, gacResHeaderFmt, (iViewPage+1),
			stTime.wYear, stTime.wMonth, stTime.wDay,
			gcacWeek[stTime.wDayOfWeek],
			stTime.wHour, stTime.wMinute, stTime.wSecond );

		pcContent = DocPageTextPreviewAlloc( iViewPage, &szCont );

		asString  = string( gpcHtmlHdr );
		asString += string( acSeper );
		asString += string( pcContent );
		asString += string( gacResFooterFmt );
		asString += string( gpcHtmlFtr );

		FREE(pcContent);
	}

	//	BSTRに必要なサイズ確認
	szSize = asString.size( );
	bstrLen = MultiByteToWideChar( CP_ACP, 0, asString.c_str( ), szSize, NULL, 0 );

	//	バッファを確保
	bstr = SysAllocStringLen( NULL, bstrLen );

	//	BSTRにブチこむ
	MultiByteToWideChar( CP_ACP, 0, asString.c_str( ), szSize, bstr, bstrLen );

	sfArray = SafeArrayCreateVector( VT_VARIANT, 0, 1 );
			
	if (sfArray == NULL || gpDocument == NULL)
	{
		goto cleanup;
	}

	hRslt = SafeArrayAccessData(sfArray, (LPVOID*)&param );
	param->vt = VT_BSTR;
	param->bstrVal = bstr;
	hRslt = SafeArrayUnaccessData(sfArray);
	hRslt = gpDocument->writeln(sfArray);

	hRslt = gpDocument->close( );

cleanup:
	if( bstr )
	{
		SysFreeString( bstr );
		hRslt = SafeArrayAccessData( sfArray, (LPVOID*)&param );
		param->vt = VT_BSTR;
		param->bstrVal = NULL;
		hRslt = SafeArrayUnaccessData( sfArray );
	}

	if( sfArray ){	SafeArrayDestroy( sfArray );	}

	return hRslt;
}
//-------------------------------------------------------------------------------------------------

