/*! @file
	@brief アプリケーションのエントリ ポイントを定義します。
	このファイルは OrinrinViewer.cpp です。
	@author	SikigamiHNQ
	@date	2011/08/18
*/
//-------------------------------------------------------------------------------------------------

/*
Orinrin Viewer : AsciiArt Viewer for Japanese Only
Copyright (C) 2011 SikigamiHNQ

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.
If not, see <http://www.gnu.org/licenses/>.

大日本帝国公用語は↓を見られたい
*/


#include "stdafx.h"
#include "OrinrinEditor.h"
#include "MaaTemplate.h"
//------------------------------------------------------------------------------------------------------------------------

static CONST TCHAR	gcatLicense[] = {
TEXT("このプログラムはフリーソフトウェアです。あなたはこれを、フリーソフトウェア財団によって発行されたGNU一般公衆利用許諾書(バージョン3か、それ以降のバージョンのうちどれか)が定める条件の下で再頒布または改変することができます。\r\n\r\n")
TEXT("このプログラムは有用であることを願って頒布されますが、*全くの無保証*です。商業可能性の保証や特定目的への適合性は、言外に示されたものも含め、全く存在しません。\r\n\r\n")
TEXT("詳しくはGNU一般公衆利用許諾書をご覧ください。\r\n\r\n")
TEXT("あなたはこのプログラムと共に、GNU一般公衆利用許諾書のコピーを一部受け取っているはずです。\r\n\r\n")
TEXT("もし受け取っていなければ、<http://www.gnu.org/licenses/> をご覧ください。\r\n\r\n")
};

//-------------------------------------------------------------------------------------------------

//	表示用フォントベーステーブル・これをコピーして使う
static LOGFONT	gstBaseFont = {
	FONTSZ_NORMAL,			//	フォントの高さ
	0,						//	平均幅
	0,						//	文字送りの方向とX軸との角度
	0,						//	ベースラインとX軸との角度
	FW_NORMAL,				//	文字の太さ(0~1000まで・400=nomal)
	FALSE,					//	イタリック体
	FALSE,					//	アンダーライン
	FALSE,					//	打ち消し線
	DEFAULT_CHARSET,		//	文字セット
	OUT_OUTLINE_PRECIS,		//	出力精度
	CLIP_DEFAULT_PRECIS,	//	クリッピング精度
	PROOF_QUALITY,			//	出力品質
	VARIABLE_PITCH,			//	固定幅か可変幅
	TEXT("ＭＳ Ｐゴシック")	//	フォント名
};
//-------------------------------------------------------------------------------------------------

static  UINT	gdUseMode;		//!<	挿入レイヤクリップ指示・設定に注意
static  UINT	gdUseSubMode;	//!<	

static  HWND	ghMaaWnd;		//!<	作られたウインドウハンドル
static TCHAR	gatIniPath[MAX_PATH];	//!<	ＩＮＩファイルの位置

extern  HWND	ghMaaFindDlg;	//!<	MAA検索ダイヤログハンドル

extern HFONT	ghAaFont;		//!<	表示用のフォント

extern  UINT	gdClickDrt;	//
//------------------------------------------------------------------------------------------------------------------------

BOOLEAN	SelectFolderDlg( HWND, LPTSTR, UINT_PTR );

HRESULT	ViewingFontNameLoad( VOID );
//------------------------------------------------------------------------------------------------------------------------


/*!
	アプリケーションのエントリポイント
	@param[in]	hInstance		このモジュールのインスタンスハンドル
	@param[in]	hPrevInstance	前のインスタンス。今は未使用
	@param[in]	lpCmdLine		コマンドライン。トークン分解はされてない、ただの文字列
	@param[in]	nCmdShow		起動時の表示状態が入ってる。表示とかそういうの
	@retval FALSE	途中終了
*/
INT APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow )
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	//	TODO: ここにコードを挿入してください。
	MSG		msg;
	HACCEL	hAccelTable;
	INT		msRslt;

#ifdef _DEBUG
	//_CRTDBG_ALLOC_MEM_DF;		// 指定が必要なフラグ
	//_CRTDBG_CHECK_ALWAYS_DF;	//	メモリをチェック		_CRTDBG_CHECK_EVERY_128_DF
	//_CRTDBG_LEAK_CHECK_DF;		//	終了時にメモリリークをチェック
	//_CRTDBG_DELAY_FREE_MEM_DF;	//	
	//	ここで使用するフラグを指定
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	INITCOMMONCONTROLSEX	iccex;
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	iccex.dwICC  = ICC_WIN95_CLASSES;
	InitCommonControlsEx( &iccex );

	//	設定ファイル位置確認
	GetCurrentDirectory( MAX_PATH, gatIniPath );
	PathAppend( gatIniPath, INI_FILE );

	SplitBarClass( hInstance );	//	スプリットバーの準備

	gdUseMode    = InitParamValue( INIT_LOAD, VL_MAA_LCLICK, MAA_SJISCLIP );
	gdUseSubMode = InitParamValue( INIT_LOAD, VL_MAA_MCLICK, MAA_SJISCLIP );

	ViewingFontNameLoad(  );	//	フォント名確保

	//	アプリケーションの初期化を実行します:
	ghMaaWnd = MaaTmpltInitialise( hInstance, GetDesktopWindow(), NULL );
	if( !(ghMaaWnd) )	return (-1);

	DraughtInitialise( hInstance, ghMaaWnd );
	gdClickDrt = gdUseMode;

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ORINRINVIEWER));

	//	メインメッセージループ
	for(;;)
	{
		msRslt = GetMessage( &msg, NULL, 0, 0 );
		if( 1 != msRslt )	break;

		//	MAA検索ダイヤログ
		if( ghMaaFindDlg )
		{	//トップに来てるかどうか判断する
			if( ghMaaFindDlg == GetForegroundWindow(  ) )
			{
				if( TranslateAccelerator( ghMaaFindDlg, hAccelTable, &msg ) )	continue;
				if( IsDialogMessage( ghMaaFindDlg, &msg ) )	continue;
			}
		}

		if( !TranslateAccelerator( msg.hwnd, hAccelTable, &msg ) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}


//-------------------------------------------------------------------------------------------------

//	バージョン情報ボックスのメッセージ ハンドラです。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch( message )
	{
		case WM_INITDIALOG:
			SetDlgItemText( hDlg, IDE_ABOUT_DISP, gcatLicense );
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------


#pragma region ("設定内容読書")

#ifndef MAA_PROFILE

/*!
	MAA複数ファイルのセーブロード・Editor側にもある
	@param[in]	dMode	非０ロード　０セーブ
	@param[in]	dCount	データの個数[NULL==ptFile]／通し番号
	@param[out]	ptFile	フルパス・MAX_PATHであること
	@param[out]	ptBase	基点ディレクトリー名・MAX_PATHであること
	@return		INT	ロード：データの個数　セーブ：０
*/
INT InitMultipleFile( UINT dMode, UINT dCount, LPTSTR ptFile, LPTSTR ptBase )
{
	TCHAR	atKeyName[MIN_STRING], atBuff[MIN_STRING];
	INT	dBuff = 0;

	if( ptFile )
	{
		if( dMode  )	//	ロード
		{
			StringCchPrintf( atKeyName, MIN_STRING, TEXT("Item%d"), dCount );
			GetPrivateProfileString( TEXT("MaaSubOpen"), atKeyName, TEXT(""), ptFile, MAX_PATH, gatIniPath );

			StringCchPrintf( atKeyName, MIN_STRING, TEXT("Base%d"), dCount );
			GetPrivateProfileString( TEXT("MaaSubOpen"), atKeyName, TEXT(""), ptBase, MAX_PATH, gatIniPath );
		}
		else	//	セーブ
		{
			StringCchPrintf( atKeyName, MIN_STRING, TEXT("Item%d"), dCount );
			WritePrivateProfileString( TEXT("MaaSubOpen"), atKeyName, ptFile, gatIniPath );

			StringCchPrintf( atKeyName, MIN_STRING, TEXT("Base%d"), dCount );
			WritePrivateProfileString( TEXT("MaaSubOpen"), atKeyName, ptBase, gatIniPath );
		}
	}
	else	//	個数
	{
		if( dMode  )	//	ロード
		{
			GetPrivateProfileString( TEXT("MaaSubOpen"), TEXT("Count"), TEXT("0"), atBuff, MIN_STRING, gatIniPath );
			dBuff = StrToInt( atBuff );
		}
		else	//	セーブ
		{
			//	一旦セクションを空にする
			ZeroMemory( atBuff, sizeof(atBuff) );
			WritePrivateProfileSection( TEXT("MaaSubOpen"), atBuff, gatIniPath );

			StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), dCount );
			WritePrivateProfileString( TEXT("MaaSubOpen"), TEXT("Count"), atBuff, gatIniPath );
		}

		return dBuff;
	}


	return 0;
}
//-------------------------------------------------------------------------------------------------
#endif

/*!
	パラメータ値のセーブロード・Editor側にもある
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
		case VL_MAA_SPLIT:		StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaSplit") );		break;
		case VL_MAA_LCLICK:		StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaMethod") );		break;
		case VL_MAATIP_VIEW:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaToolTip")  );	break;
		case VL_MAATIP_SIZE:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaToolTipSize") );	break;
		case VL_MAA_TOPMOST:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaTopMost")  );	break;
		case VL_MAASEP_STYLE:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaSepLine")  );	break;
		case VL_MAA_MCLICK:		StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaSubMethod") );	break;
		default:	return 0;
	}

	if( dMode )	//	ロード
	{
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), nValue );
		GetPrivateProfileString( TEXT("General"), atKeyName, atBuff, atBuff, MIN_STRING, gatIniPath );
		dBuff = StrToInt( atBuff );
	}
	else	//	セーブ
	{
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), nValue );
		WritePrivateProfileString( TEXT("General"), atKeyName, atBuff, gatIniPath );
	}

	return dBuff;
}
//-------------------------------------------------------------------------------------------------


/*!
	文字列の設定内容をセーブロード
	@param[in]		dMode	非０ロード　０セーブ
	@param[in]		dStyle	パラメータの種類
	@param[in,out]	ptFile	MAX_PATHであること
	@return			HRESULT	終了状態コード
*/
HRESULT InitParamString( UINT dMode, UINT dStyle, LPTSTR ptFile )
{
	TCHAR	atKeyName[MIN_STRING], atDefault[MAX_PATH];

	if(  !(ptFile) )	return E_INVALIDARG;

	switch( dStyle )
	{
		case VS_PROFILE_NAME:	StringCchCopy( atKeyName, SUB_STRING, TEXT("ProfilePath") );	break;
		case VS_PAGE_FORMAT:	StringCchCopy( atKeyName, SUB_STRING, TEXT("PageFormat")  );	break;
		case VS_FONT_NAME:		StringCchCopy( atKeyName, SUB_STRING, TEXT("FontName") );		break;
		default:	return E_INVALIDARG;
	}

	if( dMode )	//	ロード
	{
		StringCchCopy( atDefault, MAX_PATH, ptFile );
		GetPrivateProfileString( TEXT("General"), atKeyName, atDefault, ptFile, MAX_PATH, gatIniPath );
	}
	else
	{
		WritePrivateProfileString( TEXT("General"), atKeyName, ptFile, gatIniPath );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------


/*!
	ウインドウ位置のセーブロード・Editor側にもある
	@param[in]	dMode	非０ロード　０セーブ
	@param[in]	dStyle	１ビュー　２未定
	@param[in]	pstRect	ロード結果を入れるか、セーブ内容を入れる
	@return		HRESULT	終了状態コード
*/
HRESULT InitWindowPos( UINT dMode, UINT dStyle, LPRECT pstRect )
{
	TCHAR	atAppName[MIN_STRING], atBuff[MIN_STRING];

	if( !pstRect )	return E_INVALIDARG;

	switch( dStyle )
	{
		case  WDP_MAATPL:	StringCchCopy( atAppName, SUB_STRING, TEXT("MaaTmple") );	break;
		default:	SetRect( pstRect , 0, 0, 0, 0 );	return E_INVALIDARG;
	}

	if( dMode )	//	ロード
	{
		GetPrivateProfileString( atAppName, TEXT("LEFT"), TEXT("0"), atBuff, MIN_STRING, gatIniPath );
		pstRect->left   = StrToInt( atBuff );
		GetPrivateProfileString( atAppName, TEXT("TOP"), TEXT("0"), atBuff, MIN_STRING, gatIniPath );
		pstRect->top    = StrToInt( atBuff );
		GetPrivateProfileString( atAppName, TEXT("RIGHT"), TEXT("0"), atBuff, MIN_STRING, gatIniPath );
		pstRect->right  = StrToInt( atBuff );
		GetPrivateProfileString( atAppName, TEXT("BOTTOM"), TEXT("0"), atBuff, MIN_STRING, gatIniPath );
		pstRect->bottom = StrToInt( atBuff );
	}
	else	//	セーブ
	{
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstRect->left );
		WritePrivateProfileString( atAppName, TEXT("LEFT"), atBuff, gatIniPath );
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstRect->top );
		WritePrivateProfileString( atAppName, TEXT("TOP"), atBuff, gatIniPath );
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstRect->right );
		WritePrivateProfileString( atAppName, TEXT("RIGHT"), atBuff, gatIniPath );
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstRect->bottom );
		WritePrivateProfileString( atAppName, TEXT("BOTTOM"), atBuff, gatIniPath );
	}
	
	return S_OK;
}
//-------------------------------------------------------------------------------------------------

#ifndef MAA_PROFILE

/*!
	MLTディレクトリのセーブロード・Editor側にもある
	@param[in]	dMode	非０ロード　０セーブ
	@param[out]	ptFile	フルパス・MAX_PATHであること
	@return		HRESULT	終了状態コード
*/
HRESULT InitMaaFldrOpen( UINT dMode, LPTSTR ptFile )
{

	if(  !(ptFile) )	return 0;

	if( dMode )	//	ロード
	{
		GetPrivateProfileString( TEXT("General"), TEXT("MultiLinePath"), TEXT(""), ptFile, MAX_PATH, gatIniPath );
	}
	else
	{
		WritePrivateProfileString( TEXT("General"), TEXT("MultiLinePath"), ptFile, gatIniPath );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------
#endif

#pragma endregion	//	("設定内容読書")


#pragma region ("クリップする処理")
/*!
	クリップボードに文字列貼り付け・Editor側にもある
	@param[in]	pDatum	貼り付けたい文字列・ユニかSJIS
	@param[in]	cbSize	文字列の、NULLを含んだバイト数
	@param[in]	dStyle	矩形かとかそういう指定
	@return	HRESULT	終了状態コード
*/
HRESULT DocClipboardDataSet( LPVOID pDatum, INT cbSize, UINT dStyle )
{
	HGLOBAL	hGlobal;
	HANDLE	hClip;
	LPVOID	pBuffer;
	HRESULT	hRslt;
	UINT	ixFormat, ixSqrFmt;

	//	オリジナルフォーマット名を定義しておく
	ixFormat = RegisterClipboardFormat( CLIP_FORMAT );
	ixSqrFmt = RegisterClipboardFormat( CLIP_SQUARE );

	//	クリップするデータは共有メモリに入れる
	hGlobal = GlobalAlloc( GHND, cbSize );
	pBuffer = GlobalLock( hGlobal );
	CopyMemory( pBuffer, pDatum, cbSize );
	GlobalUnlock( hGlobal );

	//	クリップボードオーポン
	OpenClipboard( NULL );

	//	中身を消しちゃう
	EmptyClipboard(  );

	//	共有メモリにブッ込んだデータをクリッペする
	if( dStyle & D_UNI )	hClip = SetClipboardData( CF_UNICODETEXT, hGlobal );
	else					hClip = SetClipboardData( CF_TEXT, hGlobal );

	if( hClip )
	{
		//	クリッポが上手くいったら、オリジナル名でも記録しておく
		SetClipboardData( ixFormat, hGlobal );
		hRslt = S_OK;
	}
	else
	{
		//	登録失敗の場合は、自分で共有メモリを破壊せないかん
		GlobalFree( hGlobal );
		hRslt = E_OUTOFMEMORY;
	}

	//	クリップボード閉じる
	CloseClipboard(  );

	TRACE( TEXT("COPY DONE") );

	return hRslt;
}
//-------------------------------------------------------------------------------------------------

/*!
	MAAからSJISを受け取って処理する・Editor側にもある
	@param[in]	pcCont	AAの文字列
	@param[in]	cbSize	バイト数・末端NULLは含まない
	@param[in]	dMode	使用モード・デフォもしくは個別指定
	@return		非０デフォ動作した　０指定モードだった
*/
UINT ViewMaaMaterialise( LPSTR pcCont, UINT cbSize, UINT dMode )
{
	LPTSTR		ptString;
	UINT_PTR	cchSize;
	UINT		uRslt = TRUE;	//	デフォ動作であるならTRUE＜いつでもTRUEにした

//	FLASHWINFO	stFshWInfo;

	//	デフォ動作であるかどうか
//	if( dMode == gdUseMode ){		uRslt = TRUE;	}
	if( MAA_DEFAULT ==  dMode ){	dMode = gdUseMode;	}
	if( MAA_SUBDEFAULT== dMode ){	dMode = gdUseSubMode;	}

	if( MAA_UNICLIP == dMode )	//	ユニコード
	{
		ptString = SjisDecodeAlloc( pcCont );	//	ユニコードにしておく
		StringCchLength( ptString, STRSAFE_MAX_CCH, &cchSize );

		//	ユニコード的にクリッペ
		DocClipboardDataSet( ptString, (cchSize + 1) * 2, D_UNI );

		FREE(ptString);
	}
	else if( MAA_DRAUGHT == dMode ){	DraughtItemAdding( pcCont );	}	//	ドラフトボードに追加
	else{	DocClipboardDataSet( pcCont, (cbSize + 1), D_SJIS );	}	//	SJISコピー


	//ZeroMemory( &stFshWInfo, sizeof(FLASHWINFO) );
	//stFshWInfo.cbSize    = sizeof(FLASHWINFO);	//	この構造体のサイズ
	//stFshWInfo.hwnd      = ghMaaWnd;	//	ｗｋｔｋさせたいウインダウのハンドル
	//stFshWInfo.dwFlags   = FLASHW_ALL;	//	キャプションとタスクバーのボタン両方
	//stFshWInfo.uCount    = 2;			//	ｗｋｔｋさせる回数
	//stFshWInfo.dwTimeout = 0;			//	間隔。０でデフォルト的な間隔
	//FlashWindowEx( &stFshWInfo );		//	ｗｋｔｋさせる

	return uRslt;
}
//-------------------------------------------------------------------------------------------------
#pragma endregion	//	("クリップする処理")


#pragma region ("設定ダイヤログ")

/*!
	おぷしょんダイヤログのプロシージャ
	@param[in]	hDlg		ダイヤログハンドル
	@param[in]	message		ウインドウメッセージの識別番号
	@param[in]	wParam		追加の情報１
	@param[in]	lParam		追加の情報２
	@retval 0	メッセージは処理していない
	@retval no0	なんか処理された
*/
INT_PTR CALLBACK OptionDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	UINT	id;
	INT		dValue, dBuff;
#ifndef MAA_PROFILE
	TCHAR	atPath[MAX_PATH];//, atBuff[SUB_STRING];
#endif

	switch( message )
	{
		case WM_INITDIALOG:
			//	MAA一覧
#ifdef MAA_PROFILE
			Edit_SetText( GetDlgItem(hDlg,IDE_AA_DIRECTORY), TEXT("ＡＡディレクトリはプロファイルから設定してね") );
			EnableWindow( GetDlgItem(hDlg,IDE_AA_DIRECTORY), FALSE );
			ShowWindow( GetDlgItem(hDlg,IDB_AADIR_SEARCH), SW_HIDE );
#else
			InitMaaFldrOpen( INIT_LOAD, atPath );
			Edit_SetText( GetDlgItem(hDlg,IDE_AA_DIRECTORY), atPath );
#endif
			//	MAAポップアップについて
			dValue = InitParamValue( INIT_LOAD, VL_MAATIP_SIZE, 16 );	//	サイズ確認
			if( FONTSZ_REDUCE == dValue )	CheckRadioButton( hDlg, IDRB_POPUP_NOMAL, IDRB_POPUP_REDUCE, IDRB_POPUP_REDUCE );
			else							CheckRadioButton( hDlg, IDRB_POPUP_NOMAL, IDRB_POPUP_REDUCE, IDRB_POPUP_NOMAL );

			dValue = InitParamValue( INIT_LOAD, VL_MAATIP_VIEW, 1 );	//	ポッパップするか
			CheckDlgButton( hDlg, IDCB_POPUP_VISIBLE, dValue ? BST_CHECKED : BST_UNCHECKED );

			//	複数行テンプレをクルックしたときの動作
			dValue = InitParamValue( INIT_LOAD, VL_MAA_LCLICK, MAA_SJISCLIP );
			switch( dValue )
			{
				case MAA_UNICLIP:	id = IDRB_SEL_CLIP_UNI;		break;
				default:
				case MAA_SJISCLIP:	id = IDRB_SEL_CLIP_SJIS;	break;
				case MAA_DRAUGHT:	id = IDRB_SEL_DRAUGHT;		break;
			}
			CheckRadioButton( hDlg, IDRB_SEL_INS_EDIT, IDRB_SEL_DRAUGHT, id );

			dValue = InitParamValue( INIT_LOAD, VL_MAA_MCLICK, MAA_SJISCLIP );
			switch( dValue )
			{
				case MAA_UNICLIP:	id = IDRB_SELSUB_CLIP_UNI;	break;
				default:
				case MAA_SJISCLIP:	id = IDRB_SELSUB_CLIP_SJIS;	break;
				case MAA_DRAUGHT:	id = IDRB_SELSUB_DRAUGHT;	break;
			}
			CheckRadioButton( hDlg, IDRB_SELSUB_INS_EDIT, IDRB_SELSUB_DRAUGHT, id );

			return (INT_PTR)TRUE;

		case WM_COMMAND:
			id = LOWORD(wParam);
			switch( id )
			{
	#ifndef MAA_PROFILE
				case IDB_AADIR_SEARCH:	//	MAAディレクトリ探す
					if( SelectFolderDlg( hDlg, atPath, MAX_PATH ) )
					{
						Edit_SetText( GetDlgItem(hDlg,IDE_AA_DIRECTORY), atPath );
					}
					return (INT_PTR)TRUE;
#endif

				case IDB_APPLY://適用
				case IDOK:
	#ifndef MAA_PROFILE
					//	MAAのディレクトリー
					Edit_GetText( GetDlgItem(hDlg,IDE_AA_DIRECTORY), atPath, MAX_PATH );
					InitMaaFldrOpen( INIT_SAVE, atPath );
	#endif

					//	MAAポップアップについて
					dValue = FONTSZ_NORMAL;
					if( IsDlgButtonChecked( hDlg, IDRB_POPUP_REDUCE ) ){	dValue =  FONTSZ_REDUCE;	}
					InitParamValue( INIT_SAVE, VL_MAATIP_SIZE, dValue );
					dBuff = IsDlgButtonChecked( hDlg, IDCB_POPUP_VISIBLE );
					AaItemsTipSizeChange( dValue, dBuff );
					InitParamValue( INIT_SAVE, VL_MAATIP_VIEW, dBuff );

					//	MAAの操作
					if(      IsDlgButtonChecked( hDlg, IDRB_SEL_CLIP_UNI ) ){	dValue = MAA_UNICLIP;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_SEL_DRAUGHT ) ){	dValue = MAA_DRAUGHT;	}
					else{	dValue = MAA_SJISCLIP;	}
					InitParamValue( INIT_SAVE, VL_MAA_LCLICK, dValue );
					gdUseMode = dValue;
					gdClickDrt = gdUseMode;

					if(      IsDlgButtonChecked( hDlg, IDRB_SELSUB_CLIP_UNI ) ){	dValue = MAA_UNICLIP;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_SELSUB_DRAUGHT )  ){	dValue = MAA_DRAUGHT;	}
					else{	dValue = MAA_SJISCLIP;	}
					InitParamValue( INIT_SAVE, VL_MAA_MCLICK, dValue );
					gdUseSubMode = dValue;

					//	ＯＫなら閉じちゃう
					if( IDOK == id ){	EndDialog( hDlg, IDOK );	}
					return (INT_PTR)TRUE;

				case IDCANCEL:
					EndDialog( hDlg, IDCANCEL );
					return (INT_PTR)TRUE;

				default:	break;
			}
			break;

		default:	break;
	}

	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	ディレクトリ選択ダイアログの表示
	@param[in]	hWnd		親ウインドウのハンドル
	@param[in]	ptSelFolder	ディレクトリ名を入れるバッファへのポインター
	@param[in]	cchLen		バッファの文字数。バイト数じゃないぞ
	@return		非０：ディレクトリとった　０：キャンセルした
*/
BOOLEAN SelectDirectoryDlg( HWND hWnd, LPTSTR ptSelFolder, UINT_PTR cchLen )
{
	BROWSEINFO		stBrowseInfo;
	LPITEMIDLIST	pstItemIDList;
	TCHAR	atDisplayName[MAX_PATH];

	if( !(ptSelFolder) )	return FALSE;
	ZeroMemory( ptSelFolder, sizeof(TCHAR) * cchLen );

	//	BROWSEINFO構造体に値を設定
	stBrowseInfo.hwndOwner		 = hWnd;	//	ダイアログの親ウインドウのハンドル
	stBrowseInfo.pidlRoot		 = NULL;	//	ルートディレクトリを示すITEMIDLISTのポインタ・NULLの場合デスクトップ
	stBrowseInfo.pszDisplayName	 = atDisplayName;	//	選択されたディレクトリ名を受け取るバッファのポインタ
	stBrowseInfo.lpszTitle		 = TEXT("ＡＡの入ってるディレクトリを選択するのー！");	//	ツリービューの上部に表示される文字列
	stBrowseInfo.ulFlags		 = BIF_RETURNONLYFSDIRS;	//	表示されるディレクトリの種類を示すフラグ
	stBrowseInfo.lpfn			 = NULL;		//	BrowseCallbackProc関数のポインタ
	stBrowseInfo.lParam			 = (LPARAM)0;	//	コールバック関数に渡す値

	//	ディレクトリ選択ダイアログを表示
	pstItemIDList = SHBrowseForFolder( &stBrowseInfo );
	if( !(pstItemIDList) )
	{
		//	戻り値がNULLの場合、ディレクトリが選択されずにダイアログが閉じられたということ
		return FALSE;
	}
	else
	{
		//	ItemIDListをパス名に変換
		if( !SHGetPathFromIDList( pstItemIDList, atDisplayName ) )
		{
			//	エラー処理
			return FALSE;
		}
		//	atDisplayNameに選択されたディレクトリのパスが入ってる
		StringCchCopy( ptSelFolder, cchLen, atDisplayName );

		//	pstItemIDListを開放せしめる
		CoTaskMemFree( pstItemIDList );
	}

	return TRUE;
}
//-------------------------------------------------------------------------------------------------
#pragma endregion	//	("設定ダイヤログ")

/*!
	文字列のドット幅を数える
	@param[in]	ptStr	数えたい文字列
	@return		幅ドット数・０ならエラー
*/
INT ViewStringWidthGet( LPCTSTR ptStr )
{
	SIZE	stSize;
	UINT	cchSize;
	HDC		hdc= GetDC( ghMaaWnd );
	HFONT	hFtOld;

	StringCchLength( ptStr, STRSAFE_MAX_CCH, &cchSize );

	if( 0 >= cchSize )	return 0;	//	異常事態

	hFtOld = SelectFont( hdc, ghAaFont );

	GetTextExtentPoint32( hdc, ptStr, cchSize, &stSize );

	SelectFont( hdc, hFtOld );

	ReleaseDC( ghMaaWnd, hdc );

	return stSize.cx;
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
	表示用フォントの名前を頂く
*/
HRESULT ViewingFontNameLoad( VOID )
{
	TCHAR	atName[LF_FACESIZE];

	ZeroMemory( atName, sizeof(atName) );	//	デフォネーム
	StringCchCopy( atName, LF_FACESIZE, TEXT("ＭＳ Ｐゴシック") );

	InitParamString( INIT_LOAD, VS_FONT_NAME, atName );	//	ゲッツ！

	StringCchCopy( gstBaseFont.lfFaceName, LF_FACESIZE, atName );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	表示用フォントデータをコピーする
	@param[in]	pstLogFont	データコピる構造体へのポインター
*/
HRESULT ViewingFontGet( LPLOGFONT pstLogFont )
{
	ZeroMemory( pstLogFont, sizeof(LOGFONT) );	//	念のため空白にする

	*pstLogFont = gstBaseFont;
	//	構造体はコピーでおｋ
	return S_OK;
}
//-------------------------------------------------------------------------------------------------



#ifdef _DEBUG
VOID OutputDebugStringPlus( DWORD rixError, LPSTR pcFile, INT rdLine, LPSTR pcFunc, LPTSTR ptFormat, ... )
{
	va_list	argp;
	TCHAR	atBuf[MAX_PATH], atOut[MAX_PATH], atFiFu[MAX_PATH], atErrMsg[MAX_PATH];
	CHAR	acFile[MAX_PATH], acFiFu[MAX_PATH];
	UINT	length;

	StringCchCopyA( acFile, MAX_PATH, pcFile );
	PathStripPathA( acFile );

	StringCchPrintfA( acFiFu, MAX_PATH, ("%s %d %s"), acFile, rdLine, pcFunc );
	length = (UINT)strlen( acFiFu );

	ZeroMemory( atFiFu, sizeof(atFiFu) );
	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, acFiFu, length, atFiFu, MAX_PATH );
	//	コードページ,文字の種類を指定するフラグ,マップ元文字列のアドレス,マップ元文字列のバイト数,
	//	マップ先ワイド文字列を入れるバッファのアドレス,バッファのサイズ

	va_start(argp, ptFormat);
	StringCchVPrintf( atBuf, MAX_PATH, ptFormat, argp );
	va_end( argp );

	StringCchPrintf( atOut, MAX_PATH, TEXT("%s @ %s\r\n"), atBuf, atFiFu );//

	OutputDebugString( atOut );

	if( rixError )
	{
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, rixError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), atErrMsg, MAX_PATH, NULL );
		//	メッセージには改行が含まれているようだ
		StringCchPrintf( atBuf, MAX_PATH, TEXT("[%d]%s"), rixError, atErrMsg );//

		OutputDebugString( atBuf );
		SetLastError( 0 );
	}
}
//-------------------------------------------------------------------------------------------------
#endif

