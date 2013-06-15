/*! @file
	@brief アプリケーションのエントリ ポイントを定義します。
	このファイルは OrinrinCollector.cpp です。
	@author	SikigamiHNQ
	@date	2011/10/06
*/

/*
Orinrin Collector : Clipboard Auto Stocker for Japanese Only
Copyright (C) 2011 - 2012 Orinrin/SikigamiHNQ

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.
If not, see <http://www.gnu.org/licenses/>.
*/
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "OrinrinCollector.h"
//-------------------------------------------------------------------------------------------------

// グローバル変数:
static HANDLE		ghMutex;					//!<	多重起動防止用Mutex

static HINSTANCE	ghInst;						//!<	現在のインターフェイス
static TCHAR		gatTitle[MAX_STRING];		//!<	タイトルバーのテキスト
static TCHAR		gatWindowClass[MAX_STRING];	//!<	メインウィンドウクラス名

static  HWND		ghToolTipWnd;				//!<	ツールチップのウインドウハンドル
static HBRUSH		ghBrush;					//!<	背景色ブラシ
static HICON		ghIcon;						//!<	メインアイコン
static  UINT		grdTaskbarResetID;			//!<	タスクバーが再起動するときのメッセージIDを保持
static  HWND		ghNextViewer;				//!<	クリップボードチェインの次のやつ
static BOOLEAN		gbClipSteal;				//!<	コピー頂戴する
static BOOLEAN		gGetMsgOn;					//!<	保存したらバルーンする
static BOOLEAN		gIsAST;						//!<	保存はAST形式で
static TCHAR		gatIniPath[MAX_PATH];		//!<	ＩＮＩファイルの位置
static TCHAR		gatClipFile[MAX_PATH];		//!<	クリップ内容を保存するファイル名
static NOTIFYICONDATA	gstNtfyIcon;			//!<	タスクトレイアイコン制御の構造体

EXTERNED UINT		gbUniRadixHex;				//!<	数値参照を１６進数型で保存する

static  UINT		gbHotMod;					//!<	ポップアップホットキー修飾子
static  UINT		gbHotVkey;					//!<	ポップアップホットキー仮想キーコード
//-------------------------------------------------------------------------------------------------

/*!
	アプリケーションのエントリポイント
	@param[in]	hInstance		このモジュールのインスタンスハンドル
	@param[in]	hPrevInstance	前のインスタンス。今は未使用
	@param[in]	lpCmdLine		コマンドライン。トークン分解はされてない、ただの文字列
	@param[in]	nCmdShow		起動時の表示状態が入ってる。表示とかそういうの
	@retval FALSE	途中終了
*/
INT APIENTRY _tWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow )
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	//	TODO: ここにコードを挿入してください。
	MSG	msg;
	INT	msRslt;

#ifdef _DEBUG
	//_CRTDBG_ALLOC_MEM_DF;		// 指定が必要なフラグ
	//_CRTDBG_CHECK_ALWAYS_DF;	//	メモリをチェック		_CRTDBG_CHECK_EVERY_128_DF
	//_CRTDBG_LEAK_CHECK_DF;		//	終了時にメモリリークをチェック
	//_CRTDBG_DELAY_FREE_MEM_DF;	//	
	//	ここで使用するフラグを指定
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	//	多重起動防止
	ghMutex = CreateMutex( NULL, TRUE, TEXT("OrinrinCollector") );	//	すでに起動しているか判定
	if( GetLastError() == ERROR_ALREADY_EXISTS )	//	すでに起動している
	{
		MessageBox( NULL, TEXT("已にアプリは起動してるよ！"), TEXT("お燐からのお知らせ"), MB_OK|MB_ICONINFORMATION );
		ReleaseMutex( ghMutex );
		CloseHandle( ghMutex );
		return 0;
	}


	INITCOMMONCONTROLSEX	iccex;
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	iccex.dwICC  = ICC_WIN95_CLASSES | ICC_USEREX_CLASSES;
	InitCommonControlsEx( &iccex );

	// グローバル文字列を初期化しています。
	LoadString( hInstance, IDS_APP_TITLE, gatTitle, MAX_STRING );
	LoadString( hInstance, IDC_ORINRINCOLLECTOR, gatWindowClass, MAX_STRING );
	InitWndwClass( hInstance );

	// アプリケーションの初期化を実行します:
	if( !InitInstance( hInstance, nCmdShow ) )
	{
		return FALSE;
	}


	//	メインメッセージループ
	for(;;)
	{
		msRslt = GetMessage( &msg, NULL, 0, 0 );
		if( 1 != msRslt )	break;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}


//-------------------------------------------------------------------------------------------------

/*!
	ウインドウクラス生成
	この関数および使い方は、'RegisterClassEx' 関数が追加された Windows 95 より前の Win32 システムと互換させる場合にのみ必要です。
	アプリケーションが、関連付けられた正しい形式の小さいアイコンを取得できるようにするには、この関数を呼び出してください。
	@param[in]	hInstance	このモジュールのインスタンスハンドル
	@return		登録したクラスアトム
*/
ATOM InitWndwClass( HINSTANCE hInstance )
{
	WNDCLASSEX	wcex;

	//これによる取得は破壊しなくてよい？
	ghIcon = LoadIcon( hInstance, MAKEINTRESOURCE(IDI_ORINRINCOLLECTOR) );
	ghBrush = CreateSolidBrush( BASIC_COLOUR );

	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= ghIcon;
	wcex.hCursor		= LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground	= ghBrush;//(HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= gatWindowClass;
	wcex.hIconSm		= ghIcon;

	return RegisterClassEx( &wcex );
}
//-------------------------------------------------------------------------------------------------

/*!
	インスタンス ハンドルを保存して、メイン ウィンドウを作成します。
		この関数で、グローバル変数でインスタンス ハンドルを保存し、
		メイン プログラム ウィンドウを作成および表示します。
	@param[in]	hInstance	インスタンスハンドル
	@param[in]	nCmdShow	起動時の表示状態
*/
BOOL InitInstance( HINSTANCE hInstance, int nCmdShow )
{
	HWND	hWnd;
	INT		xxx, yyy;
	RECT	rect;

	BOOL	bHotRslt;

	ghInst = hInstance;	//	グローバル変数にインスタンス処理を格納します。


	//	設定ファイル位置確認
	GetModuleFileName( hInstance, gatIniPath, MAX_PATH );
	PathRemoveFileSpec( gatIniPath );
	PathAppend( gatIniPath, INI_FILE );

	//	ここらで初期設定確保
	gbClipSteal   = InitParamValue( INIT_LOAD, VL_COLLECT_AON,  0 );	//	コピペ保存・デフォは起動時ＯＦＦ
	gGetMsgOn     = InitParamValue( INIT_LOAD, VL_USE_BALLOON,  1 );
	gbUniRadixHex = InitParamValue( INIT_LOAD, VL_UNIRADIX_HEX, 0 );

	//	初期設定は Ctrl+Shift+C
	gbHotMod      = InitParamValue( INIT_LOAD, VL_COLHOT_MODY, (MOD_CONTROL | MOD_SHIFT) );
	gbHotVkey     = InitParamValue( INIT_LOAD, VL_COLHOT_VKEY, VK_C );


	hWnd = GetDesktopWindow(  );
	GetWindowRect( hWnd, &rect );
	xxx = ( rect.right  - WCL_WIDTH ) / 2;
	yyy = ( rect.bottom - WCL_HEIGHT ) / 2;

	hWnd = CreateWindowEx( WS_EX_TOOLWINDOW | WS_EX_APPWINDOW, gatWindowClass, gatTitle, WS_CAPTION | WS_POPUPWINDOW, xxx, yyy, WCL_WIDTH, WCL_HEIGHT, NULL, NULL, hInstance, NULL);

	if( !hWnd ){	return FALSE;	}


	//	タスクトレイがあぼ～んしたときの再起動メッセージ番号を確保
	grdTaskbarResetID = RegisterWindowMessage( TEXT("TaskbarCreated") );

	//	クリップボードチェーンに自分を登録
	ghNextViewer = SetClipboardViewer( hWnd );


	FileListViewInit( hWnd );	//	リスト初期化
	FileListViewGet( hWnd, 0, gatClipFile );	//	取り込みファイルを確保しておく
	FileTypeCheck( gatClipFile );	//	ASTかそうでないかを確認

	if( gGetMsgOn ){		Button_SetCheck( GetDlgItem(hWnd,IDB_CLIP_USE_BALLOON)  , BST_CHECKED );	}
	if( gbUniRadixHex ){	Button_SetCheck( GetDlgItem(hWnd,IDB_CLIP_UNIRADIX_HEX) , BST_CHECKED );	}
	if( gbClipSteal ){		Button_SetCheck( GetDlgItem(hWnd,IDB_CLIP_STEAL_ACT_ON) , BST_CHECKED );	}

	TasktrayIconAdd( hWnd );

	//	とりあえず、Ctrl+Shift+C
	bHotRslt = RegisterHotKey( hWnd, IDHK_CLIPSTEAL_FILECHANGE, gbHotMod, gbHotVkey );

	ShowWindow( hWnd, SW_HIDE );	//	SW_HIDE

	return TRUE;
}
//-------------------------------------------------------------------------------------------------

/*!
	レジスタホットキーとホットキーコントロールの修飾子を入替
	@param[in]	bSrc	元の修飾子コード
	@param[in]	bDrct	非０レジスタ→コントロール　０コントロール→レジスタ
	@return	変換したコード
*/
UINT RegHotModExchange( UINT bSrc, BOOLEAN bDrct )
{
	BYTE	bDest = 0;

	if( bDrct  )	//	レジスタ→コントロール
	{
		if( bSrc & MOD_SHIFT )		bDest |= HOTKEYF_SHIFT;		//	シフト
		if( bSrc & MOD_CONTROL )	bDest |= HOTKEYF_CONTROL;	//	コントロール
		if( bSrc & MOD_ALT )		bDest |= HOTKEYF_ALT;		//	アルタネート
	}
	else	//	コントロール→レジスタ
	{
		if( bSrc & HOTKEYF_SHIFT )		bDest |= MOD_SHIFT;		//	シフト
		if( bSrc & HOTKEYF_CONTROL )	bDest |= MOD_CONTROL;	//	コントロール
		if( bSrc & HOTKEYF_ALT )		bDest |= MOD_ALT;		//	アルタネート
	}

	return bDest;
}
//-------------------------------------------------------------------------------------------------

/*!
	コントロールにツールチップを設定・ツールチップハンドルは大域変数で確保
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	itemID	コントロールのＩＤ番号
	@param[in]	ptText	チップしたい文字列
	@return	HRESULT	終了状態コード
*/
HRESULT ToolTipSetting( HWND hWnd, UINT itemID, LPTSTR ptText )
{
	TTTOOLINFO	stToolInfo;

	ZeroMemory( &stToolInfo, sizeof(TTTOOLINFO) );

	stToolInfo.cbSize   = sizeof(TTTOOLINFO);
	stToolInfo.uFlags   = TTF_SUBCLASS;
	stToolInfo.hinst    = NULL;	//	
	stToolInfo.hwnd     = GetDlgItem( hWnd, itemID );
	stToolInfo.uId      = itemID;
	GetClientRect( stToolInfo.hwnd, &stToolInfo.rect );
	stToolInfo.lpszText = ptText;

	SendMessage( ghToolTipWnd, TTM_ADDTOOL, 0, (LPARAM)&stToolInfo );
	SendMessage( ghToolTipWnd, TTM_SETMAXTIPWIDTH, 0, 0 );	//	チップの幅。０設定でいい。これしとかないと改行されない

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	リストビュー書き直し
	@param[in]	hWnd	親ウインドウのハンドル
*/
HRESULT FileListViewInit( HWND hWnd )
{
	UINT	dItems, d;
	TCHAR	atFilePath[MAX_PATH];
	HWND	hLvWnd = GetDlgItem( hWnd, IDLV_CLIPSTEAL_FILELISTVW );


	ListView_DeleteAllItems( hLvWnd );

	dItems = InitParamValue( INIT_LOAD, VL_CLIPFILECNT, 1 );
	for( d = 0; dItems > d; d++ )
	{
		InitClipStealOpen( INIT_LOAD, d, atFilePath );
		FileListViewAdd( hWnd, atFilePath );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	リストビューにファイル名を追加する
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	ptPath	ファイルフルパス
	@return		追加後のアイテム数
*/
INT FileListViewAdd( HWND hWnd, LPTSTR ptPath )
{
	INT	iCount;
	TCHAR	atName[MAX_PATH];
	LVITEM	stLvi;
	HWND	hLvWnd = GetDlgItem( hWnd, IDLV_CLIPSTEAL_FILELISTVW );

	iCount = ListView_GetItemCount( hLvWnd );

	if( !(ptPath) ){	return iCount;	}
	if( NULL == ptPath[0] ){	return iCount;	}

	StringCchCopy( atName, MAX_PATH, ptPath );
	PathStripPath( atName );

	ZeroMemory( &stLvi, sizeof(LVITEM) );
	stLvi.mask     = LVIF_TEXT;
	stLvi.iItem    = iCount;

	stLvi.iSubItem = 0;
	stLvi.pszText  = atName;
	ListView_InsertItem( hLvWnd, &stLvi );

	stLvi.iSubItem = 1;
	stLvi.pszText  = ptPath;
	ListView_SetItem( hLvWnd, &stLvi );

	iCount = ListView_GetItemCount( hLvWnd );

	return iCount;
}
//-------------------------------------------------------------------------------------------------

/*!
	リストビューからファイル名を確保
	@param[in]	hWnd	親ウインドウのハンドル
	@param[in]	iNumber	リスト上の番号
	@param[in]	ptFile	ファイルフルパス・MAX_PATHであること
*/
HRESULT FileListViewGet( HWND hWnd, INT iNumber, LPTSTR ptFile )
{
	LVITEM	stLvi;
	TCHAR	atBuff[MAX_PATH];
	INT		iCount;
	HWND	hLvWnd = GetDlgItem( hWnd, IDLV_CLIPSTEAL_FILELISTVW );

	iCount = ListView_GetItemCount( hLvWnd );
	if( iCount <= iNumber ){	return E_OUTOFMEMORY;	}

	ZeroMemory( &stLvi, sizeof(LVITEM) );
	stLvi.mask       = LVIF_TEXT;
	stLvi.iItem      = iNumber;
	stLvi.iSubItem   = 1;
	stLvi.pszText    = atBuff;
	stLvi.cchTextMax = MAX_PATH;
	ListView_GetItem( hLvWnd, &stLvi );

	StringCchCopy( ptFile, MAX_PATH, atBuff );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	リストビューのアイテムを削除する
	@param[in]	hWnd	親ウインドウのハンドル
	@return		削除後のアイテム数
*/
INT FileListViewDelete( HWND hWnd )
{
	HWND	hLvWnd = GetDlgItem( hWnd, IDLV_CLIPSTEAL_FILELISTVW );
	INT		iItem, iCount;
	TCHAR	atPath[MAX_PATH];

	//	選択されてる項目を確保
	iItem = ListView_GetNextItem( hLvWnd, -1, LVNI_ALL | LVNI_SELECTED );

	//	選択されてるモノがないと無意味
	if( 0 <= iItem )
	{
		//	もし選択中のアレなら削除しない
		FileListViewGet( hWnd, iItem, atPath );
		if( 0 == StrCmp( gatClipFile, atPath ) )
		{
			MessageBox( hWnd, TEXT("そのファイルは使用中だよ。\r\n削除できないよ。"), TEXT("お燐からのお知らせ"), MB_OK | MB_ICONERROR );
		}
		else	//	問題無いなら削除
		{
			ListView_DeleteItem( hLvWnd, iItem );
		}
	}

	iCount = ListView_GetItemCount( hLvWnd );

	return iCount;
}
//-------------------------------------------------------------------------------------------------

/*!
	全設定をセーブする
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	bActOn	起動時コピペ保存機能ＯＮにしておくか
*/
HRESULT InitSettingSave( HWND hWnd, UINT bActOn )
{
	HWND	hLvWnd = GetDlgItem( hWnd, IDLV_CLIPSTEAL_FILELISTVW );
	INT		iCount, i;
	TCHAR	atBuff[MAX_PATH];


	iCount = ListView_GetItemCount( hLvWnd );

	//	一旦セクションを空にする
	ZeroMemory( atBuff, sizeof(atBuff) );
	WritePrivateProfileSection( TEXT("Collector"), atBuff, gatIniPath );

	InitParamValue( INIT_SAVE, VL_USE_BALLOON,  gGetMsgOn );
	InitParamValue( INIT_SAVE, VL_UNIRADIX_HEX, gbUniRadixHex );
	InitParamValue( INIT_SAVE, VL_CLIPFILECNT,  iCount );
	InitParamValue( INIT_SAVE, VL_COLLECT_AON,  bActOn );
	InitParamValue( INIT_SAVE, VL_COLHOT_MODY,  gbHotMod );
	InitParamValue( INIT_SAVE, VL_COLHOT_VKEY,  gbHotVkey );

	for( i = 0; iCount > i; i++ )
	{
		FileListViewGet( hWnd, i, atBuff );
		InitClipStealOpen( INIT_SAVE, i, atBuff );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	コピペ保存用ファイルのセーブロード
	@param[in]	dMode	非０ロード　０セーブ
	@param[in]	dNumber	保存ファイルの番号
	@param[out]	ptFile	フルパス・MAX_PATHであること
	@return		HRESULT	終了状態コード
*/
HRESULT InitClipStealOpen( UINT dMode, UINT dNumber, LPTSTR ptFile )
{
	TCHAR	atKey[MIN_STRING];

	if(  !(ptFile) )	return 0;

	ZeroMemory( atKey, sizeof(atKey) );
	if( 0 == dNumber )	StringCchCopy( atKey, MIN_STRING, TEXT("CopySaveFile") );
	else	StringCchPrintf( atKey, MIN_STRING, TEXT("CopySaveFile%d"), dNumber );

	if( dMode ){	GetPrivateProfileString( TEXT("Collector"), atKey, TEXT(""), ptFile, MAX_PATH, gatIniPath );	}
	else{			WritePrivateProfileString( TEXT("Collector"), atKey, ptFile, gatIniPath );	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

BOOLEAN FileTypeCheck( LPTSTR ptFile )
{
	LPTSTR		ptExten;	//	ファイル名の拡張子
	TCHAR		atExBuf[10];

	//	保存形式確認
	ptExten = PathFindExtension( ptFile );	//	拡張子が無いならNULL、というか末端になる
	StringCchCopy( atExBuf, 10, ptExten );	//	操作のためコピー
	CharLower( atExBuf );	//	比較のために小文字にしちゃう
	//	ASTであるか
	if( !( StrCmp( atExBuf , TEXT(".ast") ) ) ){	gIsAST = TRUE;	}
	else	gIsAST =  FALSE;	//	ASTじゃないならMLTとして扱う

	return gIsAST;
}
//-------------------------------------------------------------------------------------------------

/*!
	パラメータ値のセーブロード
	@param[in]	dMode	非０ロード　０セーブ
	@param[in]	dStyle	パラメータの種類
	@param[in]	nValue	ロード：デフォ値　セーブ：値
	@return		INT	ロード：値　セーブ：０
*/
INT InitParamValue( UINT dMode, UINT dStyle, INT nValue )
{
	TCHAR	atKeyName[MIN_STRING], atBuff[MIN_STRING];
	INT	dBuff = 0;

	switch( dStyle )
	{
		case VL_UNIRADIX_HEX:	StringCchCopy( atKeyName, SUB_STRING, TEXT("UniRadixHex") );	break;
		case VL_USE_BALLOON:	StringCchCopy( atKeyName, SUB_STRING, TEXT("UseBalloon")  );	break;
		case VL_CLIPFILECNT:	StringCchCopy( atKeyName, SUB_STRING, TEXT("FileCount") );		break;
		case VL_COLLECT_AON:	StringCchCopy( atKeyName, SUB_STRING, TEXT("CollectActOn") );	break;
		case VL_COLHOT_MODY:	StringCchCopy( atKeyName, SUB_STRING, TEXT("CollectHotMod") );	break;
		case VL_COLHOT_VKEY:	StringCchCopy( atKeyName, SUB_STRING, TEXT("CollectHotVkey") );	break;

		default:	return nValue;
	}

	StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), nValue );

	if( dMode )	//	ロード
	{
		GetPrivateProfileString( TEXT("Collector"), atKeyName, atBuff, atBuff, MIN_STRING, gatIniPath );
		dBuff = StrToInt( atBuff );
	}
	else	//	セーブ
	{
		WritePrivateProfileString( TEXT("Collector"), atKeyName, atBuff, gatIniPath );
	}

	return dBuff;
}
//-------------------------------------------------------------------------------------------------


/*!
	メインのウインドウプロシージャ
	@param[in]	hWnd		親ウインドウのハンドル
	@param[in]	message		ウインドウメッセージの識別番号
	@param[in]	wParam		追加の情報１
	@param[in]	lParam		追加の情報２
	@retval 0	メッセージ処理済み
	@retval no0	ここでは処理せず次に回す
*/
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_CREATE,         Cls_OnCreate );	//	画面の構成パーツを作る。ボタンとか
		HANDLE_MSG( hWnd, WM_PAINT,          Cls_OnPaint  );	//	画面の更新とか
		HANDLE_MSG( hWnd, WM_COMMAND,        Cls_OnCommand );	//	ボタン押されたとかのコマンド処理
		HANDLE_MSG( hWnd, WM_DESTROY,        Cls_OnDestroy );	//	ソフト終了時の処理
		HANDLE_MSG( hWnd, WM_HOTKEY,         Cls_OnHotKey );	//	
		HANDLE_MSG( hWnd, WM_CTLCOLORSTATIC, Cls_OnCtlColor );
		HANDLE_MSG( hWnd, WM_DRAWCLIPBOARD,  Cls_OnDrawClipboard );	//	クリップボードに変更があったら
		HANDLE_MSG( hWnd, WM_CHANGECBCHAIN,  Cls_OnChangeCBChain );	//	クリップボードビューワチェインに変更があったら

		case WM_CLOSE:	//	設定閉じるとき
			RegisterHotKey( hWnd, IDHK_CLIPSTEAL_FILECHANGE, gbHotMod, gbHotVkey );	//	登録し直し
			ShowWindow( hWnd, SW_HIDE );
			return 0;

		case WMP_TRAYNOTIFYICON:	//	タスクトレイのアイコンがｸﾘｯｺされたら
			TaskTrayIconEvent( hWnd, (UINT)wParam, (UINT)lParam );
			return 0;

		default:
			if( grdTaskbarResetID == message )	//	タスクトレイがあぼ～んした時
			{
				TasktrayIconAdd( hWnd );	//	タスクトレイがあぼ～んしたときのメッセージ。
				return 0;					//	登録し直し
			}
			break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------

/*!
	メインのクリエイト。
	@param[in]	hWnd			親ウインドウのハンドル
	@param[in]	lpCreateStruct	アプリケーションの初期化内容
	@return	TRUE	特になし
*/
BOOLEAN Cls_OnCreate( HWND hWnd, LPCREATESTRUCT lpCreateStruct )
{
	HINSTANCE	lcInst = lpCreateStruct->hInstance;	//	受け取った初期化情報から、インスタンスハンドルをひっぱる
	HWND	hWorkWnd;
	RECT	rect;
	LVCOLUMN	stLvColm;

	UINT	bCtrlMod;

	GetClientRect( hWnd, &rect );

//ツールチップ
	ghToolTipWnd = CreateWindowEx( 0, TOOLTIPS_CLASS, NULL, TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWnd, NULL, lcInst, NULL );

//アイコン
	hWorkWnd = CreateWindowEx( 0, WC_STATIC, TEXT(""), WS_CHILD | WS_VISIBLE | SS_ICON, 8, 8, 32, 32, hWnd, (HMENU)IDC_MYICON, lcInst, NULL );
	SendMessage( hWorkWnd, STM_SETICON, (WPARAM)ghIcon, 0 );

	CreateWindowEx( 0, WC_STATIC, TEXT("OrinrinCollector, Version 1.2 (2012.510.2200.920)"), WS_CHILD | WS_VISIBLE, 44, 8, 370, 23, hWnd, (HMENU)IDC_STATIC, lcInst, NULL );

	CreateWindowEx( 0, WC_STATIC, TEXT("頂戴したクリップ内容を保存するファイル名"), WS_CHILD | WS_VISIBLE, 8, 48, 370, 23, hWnd, (HMENU)IDC_STATIC, lcInst, NULL );

	CreateWindowEx( 0, WC_EDIT, TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 8, 70, rect.right-16-160, 23, hWnd, (HMENU)IDE_CLIPSTEAL_FILE, lcInst, NULL );
	ToolTipSetting( hWnd, IDE_CLIPSTEAL_FILE, TEXT("ファイル名を入力してね。MLTとASTが使えるよ。") );

	CreateWindowEx( 0, WC_BUTTON, TEXT("参照"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rect.right-8-160, 70, 50, 23, hWnd, (HMENU)IDB_CLIPSTEAL_REF, lcInst, NULL );
	ToolTipSetting( hWnd, IDB_CLIPSTEAL_REF, TEXT("ファイル選択ダイヤログを開いて、ファイルを指定できるよ。") );

	CreateWindowEx( 0, WC_BUTTON, TEXT("追加"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rect.right-8-100, 70, 50, 23, hWnd, (HMENU)IDB_CLIPSTEAL_FILEADD, lcInst, NULL );
	ToolTipSetting( hWnd, IDB_CLIPSTEAL_FILEADD, TEXT("指定したファイルを、使用リストに追加するよ。") );

	CreateWindowEx( 0, WC_BUTTON, TEXT("削除"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rect.right-8-50, 70, 50, 23, hWnd, (HMENU)IDB_CLIPSTEAL_FILEDEL, lcInst, NULL );
	ToolTipSetting( hWnd, IDB_CLIPSTEAL_FILEDEL, TEXT("使用リストで選択したファイルを、リストから削除するよ。\r\nファイル自体を削除するわけじゃないよ。") );

	hWorkWnd = CreateWindowEx( 0, WC_LISTVIEW, TEXT("filelv"), WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LVS_REPORT | LVS_NOSORTHEADER | LVS_SINGLESEL, 8, 95, rect.right-16, 105, hWnd, (HMENU)IDLV_CLIPSTEAL_FILELISTVW, lcInst, NULL );
	ListView_SetExtendedListViewStyle( hWorkWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	ToolTipSetting( hWnd, IDLV_CLIPSTEAL_FILELISTVW, TEXT("使用するファイルの一覧だよ。") );

	ZeroMemory( &stLvColm, sizeof(LVCOLUMN) );
	stLvColm.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	stLvColm.fmt = LVCFMT_LEFT;
	stLvColm.pszText = TEXT("ファイル名");	stLvColm.cx = 120;	stLvColm.iSubItem = 0;	ListView_InsertColumn( hWorkWnd, 0, &stLvColm );
	stLvColm.pszText = TEXT("フルパス");	stLvColm.cx = 300;	stLvColm.iSubItem = 1;	ListView_InsertColumn( hWorkWnd, 1, &stLvColm );

	CreateWindowEx( 0, WC_BUTTON, TEXT("保存したらバルーンメッセージを表示する"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 8, 207, rect.right-16, 23, hWnd, (HMENU)IDB_CLIP_USE_BALLOON, lcInst, NULL );
	ToolTipSetting( hWnd, IDB_CLIP_USE_BALLOON, TEXT("ファイルに取り込んだら、バルーンメッセージでお知らせするよ。") );

	CreateWindowEx( 0, WC_BUTTON, TEXT("ユニコード数値参照は１６進数型（非チェックで１０進数）"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 8, 234, rect.right-16, 23, hWnd, (HMENU)IDB_CLIP_UNIRADIX_HEX, lcInst, NULL );
	ToolTipSetting( hWnd, IDB_CLIP_UNIRADIX_HEX, TEXT("頂戴した内容にユニコードが含まれていたら、どういう形式で保存するのかを決めてね。") );

	CreateWindowEx( 0, WC_BUTTON, TEXT("起動したときに、コピペ保存をＯＮにする"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 8, 261, rect.right-16, 23, hWnd, (HMENU)IDB_CLIP_STEAL_ACT_ON, lcInst, NULL );
	ToolTipSetting( hWnd, IDB_CLIP_STEAL_ACT_ON, TEXT("起動したときに、コピペ保存機能をＯＮにしておくよ。") );

	CreateWindowEx( 0, WC_STATIC, TEXT("メニューポップアップホットキー"), WS_CHILD | WS_VISIBLE, 8, 290, 260, 23, hWnd, (HMENU)IDC_STATIC, lcInst, NULL );
	hWorkWnd = CreateWindowEx( 0, HOTKEY_CLASS, TEXT(""), WS_CHILD | WS_VISIBLE, 270, 290, 200, 23, hWnd, (HMENU)IDHK_CLIP_POPUP_KEYBIND, lcInst, NULL );
	ToolTipSetting( hWnd, IDHK_CLIP_POPUP_KEYBIND, TEXT("ここをクリックして、キーの組み合わせを設定してね。") );

	bCtrlMod = RegHotModExchange( gbHotMod, 1 );
	SendMessage( hWorkWnd , HKM_SETHOTKEY, MAKEWORD(gbHotVkey, bCtrlMod), 0 );

	CreateWindowEx( 0, WC_BUTTON, TEXT("保存して閉じる"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rect.right-8-150, rect.bottom-30, 150, 23, hWnd, (HMENU)IDB_CLIP_SAVE_AND_EXIT, lcInst, NULL );
	ToolTipSetting( hWnd, IDB_CLIP_SAVE_AND_EXIT, TEXT("変更内容を保存して、この窓を閉じるよ。") );

	return TRUE;
}
//-------------------------------------------------------------------------------------------------

/*!
	メインのCOMMANDメッセージの受け取り。ボタン押されたとかで発生
	@param[in]	hWnd		親ウインドウのハンドル
	@param[in]	id			メッセージを発生させた子ウインドウの識別子	LOWORD(wParam)
	@param[in]	hwndCtl		メッセージを発生させた子ウインドウのハンドル	lParam
	@param[in]	codeNotify	通知メッセージ	HIWORD(wParam)
	@return		なし
*/
VOID Cls_OnCommand( HWND hWnd, INT id, HWND hwndCtl, UINT codeNotify )
{
	TCHAR	atPath[MAX_PATH], atBuff[MAX_PATH];
	INT		iRslt, iTgt;

	UINT	bActOn;
	BOOL	bHotRslt;

	LRESULT	lRslt;
	UINT	bMod, bVkey;

	switch( id )
	{
		case IDM_EXIT:	DestroyWindow( hWnd );	break;


		case IDM_CLIPSTEAL_OPTION:
			ShowWindow( hWnd, SW_SHOW );
			UnregisterHotKey( hWnd, IDHK_CLIPSTEAL_FILECHANGE );	//	キーバインド取得のために一時的に解除
			break;

		case IDM_CLIPSTEAL_TOGGLE:
			gbClipSteal = !(gbClipSteal);
			TaskTrayIconCaptionChange( hWnd );
			break;

		case IDB_CLIPSTEAL_REF:	//	ファイル開くダイヤログ呼出
			if( SelectFileDlg( hWnd, atPath, MAX_PATH ) )
			{
				Edit_SetText( GetDlgItem(hWnd,IDE_CLIPSTEAL_FILE), atPath );
			}
			break;

		case IDB_CLIPSTEAL_FILEADD:
			Edit_GetText( GetDlgItem(hWnd,IDE_CLIPSTEAL_FILE), atPath, MAX_PATH );
			iRslt = FileListViewAdd( hWnd, atPath );
			if( 1 == iRslt )	//	１発目なら
			{
				StringCchCopy( gatClipFile, MAX_PATH, atPath );
				FileTypeCheck( gatClipFile );
			}	//	取り込みファイルとして登録する
			break;

		case IDB_CLIPSTEAL_FILEDEL:
			FileListViewDelete( hWnd );
			break;

		case IDB_CLIP_SAVE_AND_EXIT:
			//	ホットキー設定を確保
			lRslt = SendDlgItemMessage( hWnd, IDHK_CLIP_POPUP_KEYBIND, HKM_GETHOTKEY, 0, 0 );
			bVkey = LOBYTE( lRslt );
			bMod  = RegHotModExchange( HIBYTE( lRslt ), 0 );
			//	もしレジスト失敗したら閉じない・成功で非０もどる
			bHotRslt = RegisterHotKey( hWnd, IDHK_CLIPSTEAL_FILECHANGE, bMod, bVkey );
			if( bHotRslt )
			{
				gbHotVkey = bVkey;
				gbHotMod  = bMod;
			}
			else
			{
				MessageBox( hWnd, TEXT("ホットキーが登録出来なかったよ。\r\n已に使われてるみたい。\r\n他の組み合わせを試してみて！"), TEXT("お燐からのお知らせ"), MB_OK | MB_ICONERROR );
				break;
			}

			if( BST_CHECKED == IsDlgButtonChecked( hWnd, IDB_CLIP_USE_BALLOON ) ){	gGetMsgOn = TRUE;	}
			else{	gGetMsgOn = FALSE;	}

			if( BST_CHECKED == IsDlgButtonChecked( hWnd, IDB_CLIP_UNIRADIX_HEX ) ){	gbUniRadixHex = TRUE;	}
			else{	gbUniRadixHex = FALSE;	}

			bActOn = IsDlgButtonChecked( hWnd, IDB_CLIP_STEAL_ACT_ON );

			InitSettingSave( hWnd, bActOn );

			ShowWindow( hWnd, SW_HIDE );
			break;

		default:	break;
	}

	if( IDM_CLIPSTEAL_SELECT <= id )	//	メニュー選んだ
	{
		iTgt = id - IDM_CLIPSTEAL_SELECT;	//	リスト上の位置
		FileListViewGet( hWnd, iTgt, atPath );
		StringCchCopy( gatClipFile, MAX_PATH, atPath );
		FileTypeCheck( gatClipFile );
		//	取り込みファイルとして登録する
		PathStripPath( atPath );
		StringCchPrintf( atBuff, MAX_PATH, TEXT("%s に保存していくよ"), atPath );
		TaskTrayIconBalloon( hWnd, atBuff, TEXT("保存先を変更したよ"), NIIF_INFO );
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	PAINT。無効領域が出来たときに発生。背景の扱いに注意。背景を塗りつぶしてから、オブジェクトを描画
	@param[in]	hWnd	親ウインドウのハンドル
	@return		無し
*/
VOID Cls_OnPaint( HWND hWnd )
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
VOID Cls_OnDestroy( HWND hWnd )
{
	UnregisterHotKey( hWnd, IDHK_CLIPSTEAL_FILECHANGE );

	ChangeClipboardChain( hWnd , ghNextViewer );	//	クリップボードチェインから離脱

	gstNtfyIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
	Shell_NotifyIcon( NIM_DELETE, &gstNtfyIcon );	//	トレイアイコン破壊

	DeleteBrush( ghBrush );

	PostQuitMessage( 0 );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	コントロールの色変え
	@param[in]	hWnd		親ウインドウのハンドル
	@param[in]	hdc			色を塗るデバイスコンテキスト
	@param[in]	hWndChild	発生したコントロールのハンドル
	@param[in]	type		発生したコントロールのタイプ
	@return		その色のブラシ
*/
HBRUSH Cls_OnCtlColor( HWND hWnd, HDC hdc, HWND hWndChild, INT type )
{
	SetBkColor( hdc, BASIC_COLOUR );

	return ghBrush;
}
//-------------------------------------------------------------------------------------------------

/*!
	ホットキー押されたら呼ばれる
	@param[in]	hWnd		親ウインドウのハンドル
	@param[in]	idHotKey	登録してあるＩＤ
	@param[in]	fuModifiers	CTRL、SHIFT、ALT の組み合わせ状態
	@param[in]	vk			押されている一般キー
	@return		無し
*/
VOID Cls_OnHotKey( HWND hWnd, INT idHotKey, UINT fuModifiers, UINT vk )
{
	POINT	stPoint;
	HMENU	hMenu;

	if( IDHK_CLIPSTEAL_FILECHANGE == idHotKey )
	{
		GetCursorPos( &stPoint );

		SetForegroundWindow( hWnd );//こうしないとポップアップが消えないらしい

		hMenu = CreateFileSelMenu( hWnd, 1 );
		TrackPopupMenu( hMenu, 0, stPoint.x, stPoint.y, 0, hWnd, NULL );
		DestroyMenu( hMenu );
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ファイル選択ダイヤログの表示
	@param[in]	hWnd		親ウインドウハンドル
	@param[in]	ptSelFile	ファイル名を入れるバッファへのポインター
	@param[in]	cchLen		バッファの文字数。バイト数じゃないぞ
	@return		非０：ファイルとった　０：キャンセルした
*/
BOOLEAN SelectFileDlg( HWND hWnd, LPTSTR ptSelFile, UINT_PTR cchLen )
{
	OPENFILENAME	stOpenFile;
	BOOLEAN	bOpened;
	TCHAR	atFilePath[MAX_PATH], atFileName[MAX_STRING];

	if( !(ptSelFile) )	return FALSE;

	ZeroMemory( &stOpenFile, sizeof(OPENFILENAME) );

	ZeroMemory( atFilePath,  sizeof(atFilePath) );
	ZeroMemory( atFileName,  sizeof(atFileName) );

	stOpenFile.lStructSize     = sizeof(OPENFILENAME);
	stOpenFile.hwndOwner       = hWnd;
	stOpenFile.lpstrFilter     = TEXT("アスキーアートファイル ( mlt, ast )\0*.mlt;*.ast\0全ての形式(*.*)\0*.*\0\0");
	stOpenFile.nFilterIndex    = 1;
	stOpenFile.lpstrFile       = atFilePath;
	stOpenFile.nMaxFile        = MAX_PATH;
	stOpenFile.lpstrFileTitle  = atFileName;
	stOpenFile.nMaxFileTitle   = MAX_STRING;
//	stOpenFile.lpstrInitialDir = 
	stOpenFile.lpstrTitle      = TEXT("ため込むファイルを指定するのです");
	stOpenFile.Flags           = OFN_EXPLORER | OFN_HIDEREADONLY;
	stOpenFile.lpstrDefExt     = TEXT("mlt");

	//ここで FileSaveDialogue を出す
	bOpened = GetOpenFileName( &stOpenFile );

	if( !(bOpened) ){	return FALSE;	}	//	キャンセルしてたら何もしない

	StringCchCopy( ptSelFile, cchLen, atFilePath );

	return TRUE;
}
//-------------------------------------------------------------------------------------------------

/*!
	タスクトレイにアイコンを表示させる
	@param[in]	hWnd	ウインドウハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT TasktrayIconAdd( HWND hWnd )
{
	INT	crdCount = 0;


	//	タスクトレイアイコン接続
	ZeroMemory( &gstNtfyIcon, sizeof(gstNtfyIcon) );
	gstNtfyIcon.cbSize = sizeof(NOTIFYICONDATA);
	gstNtfyIcon.hWnd   = hWnd;
	gstNtfyIcon.uID    = IDC_ORINRINCOLLECTOR;
	gstNtfyIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	gstNtfyIcon.uCallbackMessage = WMP_TRAYNOTIFYICON;
	gstNtfyIcon.hIcon  = ghIcon;
	StringCchPrintf( gstNtfyIcon.szTip, 128, TEXT("OrinrinCollector ー %s"), gbClipSteal ? TEXT("＜●>　<●＞") : TEXT("ＺＺＺ．．．") );

	// アイコンを登録できるかエラーになるまで繰り返す
	for( ; ; )
	{
		if( Shell_NotifyIcon(NIM_ADD, &gstNtfyIcon) )
		{
			break;	//	登録できたら終わり
		}
		else
		{
			if ( ERROR_TIMEOUT != GetLastError() || 10 <= crdCount )
			{	//	タイムアウト以外で失敗、もしくは１０回リトライしてもダメぽ
				MessageBox( hWnd, TEXT("タスクトレイにアイコンを登録できなかったのです。\r\n終了するのです。"), NULL, MB_OK | MB_ICONERROR );
				SendMessage( hWnd, WM_DESTROY, 0, 0 );
				break;
			}

			//	登録できていないことを確認する
			if( Shell_NotifyIcon( NIM_MODIFY, &gstNtfyIcon) )
			{
				return S_OK;	//	実は登録できてた
			}
			else
			{
				Sleep( 500 );	//	登録できていなかった
			}

			crdCount++;
		}
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	タスクトレイアイコンでイベントが発生したら
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	uID		イベントが発生したトレイアイコンのＩＤ
	@param[in]	message	マウス、キーボードの操作のWMmessage
	@return 特になし
*/
VOID TaskTrayIconEvent( HWND hWnd, UINT uID, UINT message )
{
	POINT	stPoint;
	HMENU	hMenu, hSubMenu;

	//	マウスの右か左ボタン押しが有効

	GetCursorPos( &stPoint );

	if( WM_RBUTTONUP == message )	//	メニュー呼出
	{
		SetForegroundWindow( hWnd );//こうしないとポップアップが消えないらしい

		//	ポップアップメニューを開く
		hMenu = LoadMenu( ghInst, MAKEINTRESOURCE(IDC_ORINRINCOLLECTOR) );
		hSubMenu = GetSubMenu( hMenu, 0 );

		if( gbClipSteal )	CheckMenuItem( hSubMenu, IDM_CLIPSTEAL_TOGGLE, MF_CHECKED );

		TrackPopupMenu( hSubMenu, 0, stPoint.x, stPoint.y, 0, hWnd, NULL );
		DestroyMenu( hMenu );
	}

	if( WM_LBUTTONUP == message )	//	選択のアレ
	{
		SetForegroundWindow( hWnd );//こうしないとポップアップが消えないらしい

		hMenu = CreateFileSelMenu( hWnd, 0 );
		TrackPopupMenu( hMenu, 0, stPoint.x, stPoint.y, 0, hWnd, NULL );
		DestroyMenu( hMenu );
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ファイル選択ポッパー作る
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	bStyle	非０横取りON/OFF入れる　０ファイル選択のみ
	@return		作ったメニューのハンドル・呼んだ側で消すこと
*/
HMENU CreateFileSelMenu( HWND hWnd, UINT bStyle )
{
	HWND	hLvWnd = GetDlgItem( hWnd, IDLV_CLIPSTEAL_FILELISTVW );
	INT		iCount, i;
	HMENU	hMenu;
	TCHAR	atName[MAX_PATH], atPath[MAX_PATH];

	iCount = ListView_GetItemCount( hLvWnd );

	hMenu = CreatePopupMenu(  );
	if( 0 == iCount )
	{
		AppendMenu( hMenu, MF_STRING, IDM_CLIPSTEAL_SELECT, TEXT("（无）") );
		EnableMenuItem( hMenu, IDM_CLIPSTEAL_SELECT, MF_GRAYED );
	}
	else
	{
		if( bStyle )
		{
			AppendMenu( hMenu, MF_STRING, IDM_CLIPSTEAL_TOGGLE, TEXT("コピー内容を頂戴する") );
			AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );

			if( gbClipSteal )	CheckMenuItem( hMenu, IDM_CLIPSTEAL_TOGGLE, MF_CHECKED );
		}

		for( i = 0; iCount > i; i++ )
		{
			FileListViewGet( hWnd, i, atPath );
			StringCchCopy( atName, MAX_PATH, atPath );
			PathStripPath( atName );
			AppendMenu( hMenu, MF_STRING, IDM_CLIPSTEAL_SELECT+i, atName );

			//	使用中のやつを検出してチェックつける
			if( 0 == StrCmp( gatClipFile, atPath ) )
			{
				CheckMenuItem( hMenu, IDM_CLIPSTEAL_SELECT+i, MF_CHECKED );
			}
		}
	}

	return hMenu;
}
//-------------------------------------------------------------------------------------------------

/*!
	状況に合わせてアイコンの表示メッセージを変更する
	@param[in]	hWnd	ウインドウハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT TaskTrayIconCaptionChange( HWND hWnd )
{
	gstNtfyIcon.uFlags = NIF_TIP;

	StringCchPrintf( gstNtfyIcon.szTip, 128, TEXT("OrinrinCollector ー %s"), gbClipSteal ? TEXT("＜●>　<●＞") : TEXT("ＺＺＺ．．．") );

	Shell_NotifyIcon( NIM_MODIFY, &gstNtfyIcon );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	タスクトレイアイコンにばりゅ～んめせーじを載せる
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	ptInfo	バルーンの本文
	@param[in]	ptTitle	バルーンのタイトル
	@param[in]	dIconTy	くっつくアイコン、１情報、２警告、３エラー
	@return		HRESULT	終了状態コード
*/
HRESULT TaskTrayIconBalloon( HWND hWnd, LPTSTR ptInfo, LPTSTR ptTitle, DWORD dIconTy )
{
	gstNtfyIcon.uFlags       = NIF_INFO;
	StringCchCopy( gstNtfyIcon.szInfoTitle, 64, ptTitle );
	StringCchCopy( gstNtfyIcon.szInfo, 256, ptInfo );
	gstNtfyIcon.uTimeout     = 11111;	//	ｍｓで１１秒くらい表示させてみる・効いてない
	gstNtfyIcon.dwInfoFlags  = dIconTy;	//	用いるアイコンタイプ
	Shell_NotifyIcon( NIM_MODIFY, &gstNtfyIcon );

//	NIIF_INFO       0x00000001
//	NIIF_WARNING    0x00000002
//	NIIF_ERROR      0x00000003

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	クリップボードビューワチェインから誰かが離れたときの繋ぎ換え処理
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	hWndRemove	チェインから離れるウインドウハンドル
	@param[in]	hWndNext	離れるHWNDの、次のやつのウインドウハンドル
*/
VOID Cls_OnChangeCBChain( HWND hWnd, HWND hWndRemove, HWND hWndNext )
{
	//	離れるハンドルが、自分の次のヤツだったら、次の行き先変更して完了。
	if( hWndRemove == ghNextViewer ){	ghNextViewer = hWndNext;	}
	else if( hWndNext ){	SendMessage( hWndNext, WM_CHANGECBCHAIN, (WPARAM)hWndRemove, (LPARAM)hWndNext );	}
	//	関係ないなら、同じ内容を次に渡す。
	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	クリップボードの内容が変更された場合の確保処理。自分で書き込んだ場合も発生。
	@param[in]	hWnd	親ウインドウのハンドル
	@return		無し
*/
VOID Cls_OnDrawClipboard( HWND hWnd )
{
	OutputDebugString( TEXT("クリップボード内容確認\r\n") );

	//	変更があったことを、次のビューワに知らせておく
	if( ghNextViewer )	SendMessage( ghNextViewer, WM_DRAWCLIPBOARD, 0, 0 );

	ClipStealDoing( hWnd );

	return;
}
//-------------------------------------------------------------------------------------------------


/*!
	クリップボードに変更があった場合の処理・自分からの変更に反応しないようにせにゃ
	@param[in]	hWnd	ウインドウハンドル・使わないか
	@return	HRESULT	終了状態コード
*/
HRESULT ClipStealDoing( HWND hWnd )
{
	LPTSTR		ptTexts;
	LPSTR		pcStrs;
	CHAR		acBuffer[MAX_STRING];
	TCHAR		atMsg[MAX_STRING];

	UINT_PTR	cbSize, cbSplSz;
	DWORD		wrote;

	HANDLE	hFile;

	SYSTEMTIME	stTime;


	//	保存するファイル指定が無いならナニもしない
	if( NULL == gatClipFile[0] ){	return E_NOTIMPL;	}

	//	クリップスティール機能有効？
	if( !(gbClipSteal) ){	return  S_FALSE;	}


	ptTexts = ClipboardDataGet( NULL );
	if( !(ptTexts) ){	return E_ACCESSDENIED;	}
	//	クリップされたのが文字列ではなかったら直ぐ終わる

	//	SJIS型に変換
	pcStrs = SjisEncodeAlloc( ptTexts );
	StringCchLengthA( pcStrs, STRSAFE_MAX_CCH, &cbSize );

	//	ゲットしたコピペ文字列の保存処理
	hFile = CreateFile( gatClipFile, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE != hFile )
	{
		SetFilePointer( hFile, 0, NULL, FILE_END );

		if( gIsAST )
		{
			GetLocalTime( &stTime );
			StringCchPrintfA( acBuffer, MAX_STRING, ("[AA][%04u/%02u/%02u %02u:%02u:%02u]\r\n"),
				stTime.wYear, stTime.wMonth, stTime.wDay, stTime.wHour, stTime.wMinute, stTime.wSecond );
			StringCchLengthA( acBuffer, MAX_STRING, &cbSplSz );
			WriteFile( hFile, acBuffer, cbSplSz, &wrote, NULL );
		}

		WriteFile( hFile, pcStrs, cbSize, &wrote, NULL );

		if( gIsAST ){	StringCchCopyA( acBuffer, MAX_STRING, ("\r\n") );	}
		else{	StringCchCopyA( acBuffer, MAX_STRING, ("\r\n[SPLIT]\r\n") );	}
		StringCchLengthA( acBuffer, MAX_STRING, &cbSplSz );
		WriteFile( hFile, acBuffer, cbSplSz, &wrote, NULL );

		SetEndOfFile( hFile );
		CloseHandle( hFile );
	}

	FREE( ptTexts );
	FREE( pcStrs );

	//	保存したメッセージが必要
	if( gGetMsgOn )
	{
		StringCchPrintf( atMsg, MAX_STRING, TEXT("%u Byte 取得"), cbSize );
		TaskTrayIconBalloon( hWnd, TEXT("コピーされた文字列を保存したよ。"), atMsg, 1 );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	クリップボードのデータをいただく・同じモノが、DocInsDelCtrl.cpp にある
	@param[in]	pVoid	特になし
	@return		確保した文字列・mallocしてるので、函数呼んだ方でfree忘れないように
*/
LPTSTR ClipboardDataGet( LPVOID pVoid )
{
	LPTSTR	ptString = NULL, ptClipTxt;
	LPSTR	pcStr, pcClipTp;	//	変換用臨時
	DWORD	cbSize;
	UINT	dEnumFmt;
	INT		ixCount, iC;
	HANDLE	hClipData;

	//	クリップボードの中身をチェキ・どっちにしてもユニコードテキストフラグはある
	if( IsClipboardFormatAvailable( CF_UNICODETEXT ) )
	{
		OpenClipboard( NULL );	//	クリップボードをオーポンする
		//	開けっ放しだと他のアプリに迷惑なのですぐ閉めるように

		dEnumFmt = 0;	//	初期値は０
		ixCount = CountClipboardFormats(  );
		for( iC = 0; ixCount > iC; iC++ )
		{	//	順番に列挙して、先にヒットしたフォーマットで扱う
			dEnumFmt = EnumClipboardFormats( dEnumFmt );
			if( CF_UNICODETEXT == dEnumFmt || CF_TEXT == dEnumFmt ){	break;	}
		}
		if( 0 == dEnumFmt ){	return NULL;	}
		//	それ以上列挙が無いか、函数失敗なら０になる

		//	クリップボードのデータをゲッツ！
		//	ハンドルのオーナーはクリップボードなので、こちらからは操作しないように
		//	中身の変更などもってのほかである
		hClipData = GetClipboardData( dEnumFmt );

		if( CF_UNICODETEXT == dEnumFmt )
		{
			//	取得データを処理。TEXTなら、ハンドルはグローバルメモリハンドル
			//	新たにコピーされたらハンドルは無効になるので、中身をコピーせよ
			ptClipTxt = (LPTSTR)GlobalLock( hClipData );
			cbSize    = GlobalSize( (HGLOBAL)hClipData );
			//	確保出来るのはバイトサイズ・テキストだと末尾のNULLターミネータ含む

			if( 0 < cbSize )
			{
				ptString = (LPTSTR)malloc( cbSize );
				StringCchCopy( ptString, (cbSize / 2), ptClipTxt );
			}
		}
		else	//	非ユニコードが優先されている場合
		{
			pcClipTp = (LPSTR)GlobalLock( hClipData );
			cbSize   = GlobalSize( (HGLOBAL)hClipData );

			if( 0 < cbSize )
			{
				pcStr = (LPSTR)malloc( cbSize );
				StringCchCopyA( pcStr, cbSize, pcClipTp );

				ptString = SjisDecodeAlloc( pcStr );	//	SJISの内容をユニコードにする
				free( pcStr );
			}
		}


		//	使い終わったら閉じておく
		GlobalUnlock( hClipData );
		CloseClipboard(  );
	}

	return ptString;
}
//-------------------------------------------------------------------------------------------------

