/*! @file
	@brief 枠の面倒見る
	このファイルは FrameCtrl.cpp です。
	@author	SikigamiHNQ
	@date	2011/06/08
*/

/*
Orinrin Editor : AsciiArt Story Editor for Japanese Only
Copyright (C) 2011 Orinrin/SikigamiHNQ

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

#define FRAMEINSERTBOX_CLASS	TEXT("FRAMEINSBOX_CLASS")
#define FIB_WIDTH	400
#define FIB_HEIGHT	200

#define TRANCE_COLOUR	RGB(0x66,0x77,0x88)
//-------------------------------------------------------------------------------------------------

typedef struct tagFRAMEINSINFO
{
	UINT	dRfItem;	//	天井・上のパーツ数
	UINT	dLftRfDot;	//	左上＋上のドット数・中の右オフセット開始地点
	UINT	dMidLines;	//	間の行数
	UINT	dFlrItem;	//	床・下のパーツ数

	LPTSTR	ptRoof;		//	天井の文字列
	LPTSTR	ptMiddle;	//	間の文字列
	LPTSTR	ptFloor;	//	床の文字列

} FRMINSINFO, *LPFRMINSINFO;
//-------------------------------------------------------------------------------------------------

#define TB_ITEMS	14
static  TBBUTTON	gstFIBTBInfo[] = {
	{ 10,	IDM_FRAME_INS_DECIDE,	TBSTATE_ENABLED,	TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE,		0,	0  },	//	
	{  0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,							0,	0  },
	{  0,	IDM_INSFRAME_ALPHA   ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	0,	0  },	//	
	{  1,	IDM_INSFRAME_BRAVO   ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	0,	0  },	//	
	{  2,	IDM_INSFRAME_CHARLIE ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	0,	0  },	//	
	{  3,	IDM_INSFRAME_DELTA   ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	0,	0  },	//	
	{  4,	IDM_INSFRAME_ECHO    ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	0,	0  },	//	
	{  5,	IDM_INSFRAME_FOXTROT ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	0,	0  },	//	
	{  6,	IDM_INSFRAME_GOLF    ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	0,	0  },	//	
	{  7,	IDM_INSFRAME_HOTEL   ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	0,	0  },	//	
	{  8,	IDM_INSFRAME_INDIA   ,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	0,	0  },	//	
	{  9,	IDM_INSFRAME_JULIETTE,	TBSTATE_ENABLED,	TBSTYLE_CHECKGROUP | TBSTYLE_AUTOSIZE,	0,	0  },	//	
	{  0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,							0,	0  },
	{ 11,	IDM_FRMINSBOX_QCLOSE,	TBSTATE_ENABLED,	TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,		0,	0  }	//	
};	//	
//-------------------------------------------------------------------------------------------------


#ifdef MULTI_FILE
extern FILES_ITR	gitFileIt;	//!<	今見てるファイルの本体
#define gstFile	(*gitFileIt)	//!<	イテレータを構造体と見なす
#else
extern ONEFILE	gstFile;			//!<	ファイル単位・複数ファイルにはどうやって対応を？
#endif
		//!<	ファイル単位・複数ファイルにはどうやって対応を？
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

static HIMAGELIST	ghFrameImgLst;

static POINT		gstViewOrigin;	//!<	ビューの左上ウインドウ位置・
static POINT		gstOffset;		//!<	ビュー左上からの、ボックスの相対位置
static POINT		gstFrmSz;		//!<	ウインドウエッジから描画領域までのオフセット
static INT			gdToolBarHei;	//!<	ツールバー太さ

static  UINT		ghSelect;		//!<	選択した枠番号０～９
static BOOLEAN		gbQuickClose;	//!<	貼り付けたら直ぐ閉じる

static FRMINSINFO	gstFrmInsInfo;	//!<	表示中のヤツの情報

extern HFONT		ghAaFont;		//	AA用フォント


FRAMEINFO	gstFrameInfo[FRAME_MAX];	//!<	配列で必要数確保でいいか
//-------------------------------------------------------------------------------------------------

INT_PTR	CALLBACK FrameEditDlgProc( HWND, UINT, WPARAM, LPARAM );

INT_PTR	Frm_OnInitDialog( HWND, HWND, LPARAM );
INT_PTR	Frm_OnCommand( HWND, INT, HWND, UINT );
INT_PTR	Frm_OnDrawItem( HWND, CONST LPDRAWITEMSTRUCT );
INT_PTR	Frm_OnNotify( HWND, INT, LPNMHDR );

HRESULT	InitFrameItem( UINT, UINT, LPFRAMEINFO );

HRESULT	FrameNameLoad( UINT, LPTSTR, UINT_PTR );

HRESULT	FramePartsUpdate( HWND, HWND, LPFRAMEITEM );

HRESULT	FrameDataGet( UINT, LPFRAMEINFO );
HRESULT	FrameInfoDisp( HWND );


HRESULT	FrameInsBoxInfoCheck( VOID );
INT		FrameInsBoxSizeGet( LPRECT );
VOID	FrameInsBoxFrmDraw( HDC );
VOID	FrameDrawItem( HDC, INT, LPTSTR );
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

	ghSelect = 0;

	gbQuickClose = TRUE;

	ZeroMemory( &gstViewOrigin, sizeof(POINT) );
	ZeroMemory( &gstFrmSz, sizeof(POINT) );
	gdToolBarHei = 0;

	//	アイコン
	ghFrameImgLst = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 4, 1 );
	resnum = IDBMP_FRMINS_ALPHA;
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_FRMINS_SEL ) );
	for( ici = 0; 10 > ici; ici++ )
	{
		hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( (resnum++) ) );
		ImageList_Add( ghFrameImgLst, hImg, hMsq );	//	イメージリストにイメージを追加
		DeleteBitmap( hImg );
	}
	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMP_FRMINS_INSERT ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_FRMINS_INSERT ) );
	ImageList_Add( ghFrameImgLst, hImg, hMsq );	//	イメージリストにイメージを追加
	DeleteBitmap( hImg );	DeleteBitmap( hMsq );

	hImg = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMP_REFRESH ) );
	hMsq = LoadBitmap( hInstance, MAKEINTRESOURCE( IDBMQ_REFRESH ) );
	ImageList_Add( ghFrameImgLst, hImg, hMsq );	//	イメージリストにイメージを追加
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
	HMENU	hMenu, hSubMenu;//, hBoxMenu;
	UINT	i;
	TCHAR	atBuffer[MAX_PATH], atName[MAX_STRING];

	//	メニュー構造変わったらここも変更・どうにかならんのか
	hMenu = GetMenu( hWnd );
	hSubMenu = GetSubMenu( hMenu, 2 );

	//	メイン枠、０番のアレ
	//FrameNameLoad( 0, atName, MAX_STRING );
	//StringCchPrintf( atBuffer, MAX_PATH, TEXT("枠：(&A)%s"), atName );
	//ModifyMenu( hSubMenu, IDM_INSFRAME_ALPHA, MF_BYCOMMAND | MFT_STRING, IDM_INSFRAME_ALPHA, atBuffer );

	//	追加枠・１から０から
	for( i = 0; FRAME_MAX > i; i++ )
	{
		FrameNameLoad( i, atName, MAX_STRING );
		StringCchPrintf( atBuffer, MAX_PATH, TEXT("枠：(&%c)%s"), 'A'+i, atName );
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
	@param[in]	bMode		非０アクセラレータ付ける　０付けない
	@return		HRESULT	終了状態コード
*/
HRESULT FrameNameModifyPopUp( HMENU hPopMenu, UINT bMode )
{
	UINT	i, j;
	TCHAR	atBuffer[MAX_PATH], atName[MAX_STRING];

	for( i = 0, j = 1; FRAME_MAX > i; i++, j++ )
	{
		FrameNameLoad( i, atName, MAX_STRING );

		if( bMode )
		{
			if( 10 <= j )	j = 0;
			StringCchPrintf( atBuffer, MAX_PATH, TEXT("枠：(&%c)%s"), '0'+j, atName );
		}
		else
		{
			StringCchPrintf( atBuffer, MAX_PATH, TEXT("枠：%s"), atName );
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

	//	所定のAPP名を作る
	StringCchPrintf( atAppName, MIN_STRING, TEXT("Frame%u"), dNumber );

	GetPrivateProfileString( atAppName, TEXT("Name"), atAppName, ptNamed, cchSize, gatFrameIni );

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

	//	所定のAPP名を作る
	StringCchPrintf( atAppName, MIN_STRING, TEXT("Frame%u"), dNumber );

	if( dMode )	//	ロード
	{
		GetPrivateProfileString( atAppName, TEXT("Name"), atAppName, pstInfo->atFrameName, MAX_STRING, gatFrameIni );

		GetPrivateProfileString( atAppName, TEXT("Daybreak"), TEXT("│"), pstInfo->stDaybreak.atParts, PARTS_CCH, gatFrameIni );
		GetPrivateProfileString( atAppName, TEXT("Morning"), TEXT("┌"), pstInfo->stMorning.atParts, PARTS_CCH, gatFrameIni );
		GetPrivateProfileString( atAppName, TEXT("Noon"), TEXT("─"), pstInfo->stNoon.atParts, PARTS_CCH, gatFrameIni );
		GetPrivateProfileString( atAppName, TEXT("Afternoon"), TEXT("┐"), pstInfo->stAfternoon.atParts, PARTS_CCH, gatFrameIni );
		GetPrivateProfileString( atAppName, TEXT("Nightfall"), TEXT("│"), pstInfo->stNightfall.atParts, PARTS_CCH, gatFrameIni );
		GetPrivateProfileString( atAppName, TEXT("Twilight"), TEXT("┘"), pstInfo->stTwilight.atParts, PARTS_CCH, gatFrameIni );
		GetPrivateProfileString( atAppName, TEXT("Midnight"), TEXT("─"), pstInfo->stMidnight.atParts, PARTS_CCH, gatFrameIni );
		GetPrivateProfileString( atAppName, TEXT("Dawn"), TEXT("└"), pstInfo->stDawn.atParts, PARTS_CCH, gatFrameIni );

		GetPrivateProfileString( atAppName, TEXT("LEFTOFFSET"), TEXT("0"), atBuff, MIN_STRING, gatFrameIni );
		pstInfo->dLeftOffset  = StrToInt( atBuff );
		GetPrivateProfileString( atAppName, TEXT("RIGHTOFFSET"), TEXT("0"), atBuff, MIN_STRING, gatFrameIni );
		pstInfo->dRightOffset = StrToInt( atBuff );
	}
	else	//	セーブ
	{
		WritePrivateProfileString( atAppName, TEXT("Name"), pstInfo->atFrameName, gatFrameIni );

		WritePrivateProfileString( atAppName, TEXT("Daybreak"), pstInfo->stDaybreak.atParts, gatFrameIni );
		WritePrivateProfileString( atAppName, TEXT("Morning"), pstInfo->stMorning.atParts, gatFrameIni );
		WritePrivateProfileString( atAppName, TEXT("Noon"), pstInfo->stNoon.atParts, gatFrameIni );
		WritePrivateProfileString( atAppName, TEXT("Afternoon"), pstInfo->stAfternoon.atParts, gatFrameIni );
		WritePrivateProfileString( atAppName, TEXT("Nightfall"), pstInfo->stNightfall.atParts, gatFrameIni );
		WritePrivateProfileString( atAppName, TEXT("Twilight"), pstInfo->stTwilight.atParts, gatFrameIni );
		WritePrivateProfileString( atAppName, TEXT("Midnight"), pstInfo->stMidnight.atParts, gatFrameIni );
		WritePrivateProfileString( atAppName, TEXT("Dawn"), pstInfo->stDawn.atParts, gatFrameIni );

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstInfo->dLeftOffset );
		WritePrivateProfileString( atAppName, TEXT("LEFTOFFSET"), atBuff, gatFrameIni );
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstInfo->dRightOffset );
		WritePrivateProfileString( atAppName, TEXT("RIGHTOFFSET"), atBuff, gatFrameIni );
	}


	return S_OK;
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

	iRslt = DialogBoxParam( hInst, MAKEINTRESOURCE(IDD_FRAME_EDIT_DLG), hWnd, FrameEditDlgProc, 0 );
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
	UINT	i;

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
	HWND	hCmboxWnd;
//	TCHAR	atName[MAX_STRING];

	switch( id )
	{
		default:	break;

		case IDCANCEL:
		case IDB_CANCEL:
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
			if( IDB_OK ==  id ){	EndDialog( hDlg, cbNameMod ? IDYES : IDOK );	}
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

		case IDB_BOXP_NAME_APPLY:	//	名称を変更した
			Edit_GetText( GetDlgItem(hDlg,IDE_BOXP_NAME_EDIT), gstFrameInfo[gNowSel].atFrameName, MAX_STRING );
			cbNameMod = TRUE;	cbNameChg = TRUE;
			return (INT_PTR)TRUE;

		case IDCB_BOX_NAME_SEL:
			if( CBN_SELCHANGE == codeNotify )	//	選択が変更された
			{
				gNowSel = ComboBox_GetCurSel( hWndCtl );
				FrameInfoDisp( hDlg );
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
	TCHAR	atBuffer[MIN_STRING];

	if( Edit_GetTextLength( hWndCtl ) )
	{
		Edit_GetText( hWndCtl, atBuffer, MIN_STRING );
		atBuffer[PARTS_CCH-1] = 0;
		StringCchCopy( pstItem->atParts, PARTS_CCH, atBuffer );
	}
	else	//	文字がなかったら、全角空白にしちゃう
	{
		StringCchCopy( pstItem->atParts, PARTS_CCH, TEXT("　") );
	}

	//	ドット数確認して
	pstItem->dDot = ViewStringWidthGet( pstItem->atParts );

	//	ついでに再描画
	InvalidateRect( GetDlgItem(hDlg,IDS_FRAME_IMAGE), NULL, TRUE );

	return S_OK;
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
	LPFRAMEINFO	pstInfo;

	HFONT	hFtOld;
	TCHAR	atStr[BIG_STRING];	//	足りるか？

	UINT	cchSize;
	INT	xMaxDot, i, xNoonLen, mzNoonCnt, xNightPos, xMidLen, mzMidCnt;
	INT	yPos = (LINE_HEIGHT/2);

	if( IDS_FRAME_IMAGE != pstDrawItem->CtlID ){	return (INT_PTR)FALSE;	}

	pstInfo = &(gstFrameInfo[gNowSel]);

	hFtOld = SelectFont( pstDrawItem->hDC, ghAaFont );

	FillRect( pstDrawItem->hDC, &(pstDrawItem->rcItem), GetSysColorBrush( COLOR_WINDOW ) );
	SetBkMode( pstDrawItem->hDC, TRANSPARENT );

	//	枠描画領域を確保
	xMaxDot = pstDrawItem->rcItem.right - (SPACE_ZEN * 2);

	//	上線の文字数
	xNoonLen  = xMaxDot - ( pstInfo->stMorning.dDot + pstInfo->stAfternoon.dDot );
	mzNoonCnt = xNoonLen  / pstInfo->stNoon.dDot;

	//	下線の文字数
	xMidLen  = xMaxDot - ( pstInfo->stTwilight.dDot + pstInfo->stDawn.dDot );
	mzMidCnt = xMidLen  / pstInfo->stMidnight.dDot;

	//	天井の描画
	ZeroMemory( atStr, sizeof(atStr) );
	StringCchCopy( atStr, BIG_STRING, pstInfo->stMorning.atParts );
	for( i = 0; mzNoonCnt > i; i++ ){	StringCchCat( atStr, BIG_STRING, pstInfo->stNoon.atParts );	}
	//	ここまでのドット数・右上の描画開始位置が右の縦の開始位置
	xNightPos  = ViewStringWidthGet( atStr );
	xNightPos += pstInfo->dRightOffset;	//	オフセット分ずらしておく
	//	右上部分くっつけておｋ
	StringCchCat( atStr, BIG_STRING, pstInfo->stAfternoon.atParts );
	StringCchLength( atStr, BIG_STRING, &cchSize );
	ExtTextOut( pstDrawItem->hDC, SPACE_ZEN, yPos, 0, NULL, atStr, cchSize, NULL );

	yPos += LINE_HEIGHT;
	//	左柱の描画
	ZeroMemory( atStr, sizeof(atStr) );
	StringCchCopy( atStr, BIG_STRING, pstInfo->stDaybreak.atParts );
	StringCchLength( atStr, BIG_STRING, &cchSize );
	ExtTextOut( pstDrawItem->hDC, SPACE_ZEN, yPos, 0, NULL, atStr, cchSize, NULL );

	//	右柱の描画
	StringCchCopy( atStr, BIG_STRING, pstInfo->stNightfall.atParts );
	StringCchLength( atStr, BIG_STRING, &cchSize );
	ExtTextOut( pstDrawItem->hDC, SPACE_ZEN + xNightPos, yPos, 0, NULL, atStr, cchSize, NULL );

	yPos += LINE_HEIGHT;
	//	左柱の描画
	ZeroMemory( atStr, sizeof(atStr) );
	StringCchCopy( atStr, BIG_STRING, pstInfo->stDaybreak.atParts );
	StringCchLength( atStr, BIG_STRING, &cchSize );
	ExtTextOut( pstDrawItem->hDC, SPACE_ZEN, yPos, 0, NULL, atStr, cchSize, NULL );

	//	右柱の描画
	StringCchCopy( atStr, BIG_STRING, pstInfo->stNightfall.atParts );
	StringCchLength( atStr, BIG_STRING, &cchSize );
	ExtTextOut( pstDrawItem->hDC, SPACE_ZEN + xNightPos, yPos, 0, NULL, atStr, cchSize, NULL );


	yPos += LINE_HEIGHT;
	ZeroMemory( atStr, sizeof(atStr) );
	StringCchCopy( atStr, BIG_STRING, pstInfo->stDawn.atParts );
	for( i = 0; mzMidCnt > i; i++ ){	StringCchCat( atStr, BIG_STRING, pstInfo->stMidnight.atParts );	}
	StringCchCat( atStr, BIG_STRING, pstInfo->stTwilight.atParts );
	StringCchLength( atStr, BIG_STRING, &cchSize );
	ExtTextOut( pstDrawItem->hDC, SPACE_ZEN, yPos, 0, NULL, atStr, cchSize, NULL );

	return (INT_PTR)TRUE;
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

	nmCode = pstNmhdr->code;

	//	左押したらデルタが正、右で負
#if 0
	if( IDSP_LEFT_OFFSET == idFrom )
	{
		pstUpDown = (LPNMUPDOWN)pstNmhdr;
		if( UDN_DELTAPOS == nmCode )
		{
			TRACE( TEXT("%d %d"), pstUpDown->iPos, pstUpDown->iDelta );
			if( 0 < pstUpDown->iDelta ){		gstFrameInfo[gNowSel].dLeftOffset +=  1;	}
			else if( 0 >pstUpDown->iDelta ){	gstFrameInfo[gNowSel].dLeftOffset -=  1;	}
		}
		return (INT_PTR)TRUE;
	}
#endif
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

	pstFrame->stDaybreak.dDot  = ViewStringWidthGet( pstFrame->stDaybreak.atParts );
	pstFrame->stMorning.dDot   = ViewStringWidthGet( pstFrame->stMorning.atParts );
	pstFrame->stNoon.dDot      = ViewStringWidthGet( pstFrame->stNoon.atParts );
	pstFrame->stAfternoon.dDot = ViewStringWidthGet( pstFrame->stAfternoon.atParts );
	pstFrame->stNightfall.dDot = ViewStringWidthGet( pstFrame->stNightfall.atParts );
	pstFrame->stTwilight.dDot  = ViewStringWidthGet( pstFrame->stTwilight.atParts );
	pstFrame->stMidnight.dDot  = ViewStringWidthGet( pstFrame->stMidnight.atParts );
	pstFrame->stDawn.dDot      = ViewStringWidthGet( pstFrame->stDawn.atParts );

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
#if 0
	StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"),   gstFrameInfo[gNowSel].dLeftOffset );
	Edit_SetText( GetDlgItem(hDlg,IDE_LEFT_OFFSET),    atBuff );
#endif
	StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"),   gstFrameInfo[gNowSel].dRightOffset );
	Edit_SetText( GetDlgItem(hDlg,IDE_RIGHT_OFFSET),   atBuff );


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
	INT_PTR		iLns;
	INT			iTop, iBtm, iInX, iEndot, iPadding, i, baseDot;
	INT			xNoonLen, mzNoonCnt, xMidLen, mzMidCnt, xNightPos;
	UINT		cchSize;
	TCHAR		atStr[BIG_STRING];	//	足りるか？
	LPTSTR		ptPadding;

	FRAMEINFO	stInfo;

	FrameDataGet( dMode, &stInfo );


	//	ページ全体の行数
	iLines = gstFile.vcCont.at( gixFocusPage ).vcPage.size( );

	//	開始地点から開始	//	D_SQUARE
	iTop = gstFile.vcCont.at( gixFocusPage ).dSelLineTop;
	iBtm = gstFile.vcCont.at( gixFocusPage ).dSelLineBottom;
	if( 0 >  iTop ){	iTop = 0;	}
	if( 0 >  iBtm ){	iBtm = iLines - 1;	}

	iInX = DocLineParamGet( iBtm, NULL, NULL );
	if( 0 == iInX ){	 iBtm--;	}

	//	矩形選択無しとみなす・ていうか矩形に対応って蝶☆ややこしいＹＯ！

	ViewSelPageAll( -1 );	//	範囲とったので解除しておｋ

	//	選択範囲内でもっとも長いドット数を確認
	baseDot = DocPageMaxDotGet( iTop, iBtm );
	//	右余裕を足しておく。これは枠の左右を含まない量である
	baseDot += 16;	//	値は適当

	iBtm++;	//	天井の改行が入るので、この行の末端でまた改行すれば床の分確保

	DocCrLfAdd( 0 , iTop, TRUE );	//	まず天井を入れるために改行

	//天井追加
	//	上線の文字数
	xNoonLen  = baseDot  + stInfo.stNoon.dDot;	//	壱組分余裕を持たせる
	mzNoonCnt = xNoonLen / stInfo.stNoon.dDot;

	//	下線の文字数
	xMidLen  = xNoonLen;
	if( stInfo.stNoon.dDot == stInfo.stMidnight.dDot )	//	上下同じ計算できるなら
	{
		mzMidCnt = mzNoonCnt;
	}
	else
	{
		xMidLen += (stInfo.stMidnight.dDot / 2);
		mzMidCnt = xMidLen  / stInfo.stMidnight.dDot;
	}

	//	天井の描画
	ZeroMemory( atStr, sizeof(atStr) );
	StringCchCopy( atStr, BIG_STRING, stInfo.stMorning.atParts );
	for( i = 0; mzNoonCnt > i; i++ ){	StringCchCat( atStr, BIG_STRING, stInfo.stNoon.atParts );	}
	//	ここまでのドット数・右上の描画開始位置が右の縦の開始位置
	xNightPos  = ViewStringWidthGet( atStr );
	xNightPos += stInfo.dRightOffset;	//	オフセット分ずらしておく
	//	右上部分くっつけておｋ
	StringCchCat( atStr, BIG_STRING, stInfo.stAfternoon.atParts );
	//	天井書込
	iInX = 0;
	DocInsertString( &iInX, &iTop, NULL, atStr, 0, FALSE );

	//左右柱追加
	for( i = iTop+1; iBtm >= i; i++ )
	{
		iInX = 0;	//	左追加
		DocInsertString( &iInX, &i, NULL, stInfo.stDaybreak.atParts, 0, FALSE );
		iEndot = DocLineParamGet( i, NULL, NULL );
		iPadding  = xNightPos - iEndot;	//	右追加
		ptPadding = DocPaddingSpaceWithPeriod( iPadding, NULL, NULL, NULL, TRUE );
		iInX = iEndot;
		DocInsertString( &iInX, &i, NULL, ptPadding, 0, FALSE );
		FREE(ptPadding);
		DocInsertString( &iInX, &i, NULL, stInfo.stNightfall.atParts, 0, FALSE );
	}

	//床挿入改行
	iInX = DocLineParamGet( iBtm, NULL, NULL );
	DocCrLfAdd( iInX , iBtm, FALSE );
	iBtm++;

	//床追加
	ZeroMemory( atStr, sizeof(atStr) );
	StringCchCopy( atStr, BIG_STRING, stInfo.stDawn.atParts );
	for( i = 0; mzMidCnt > i; i++ ){	StringCchCat( atStr, BIG_STRING, stInfo.stMidnight.atParts );	}
	StringCchCat( atStr, BIG_STRING, stInfo.stTwilight.atParts );
	StringCchLength( atStr, BIG_STRING, &cchSize );
	iInX = 0;
	DocInsertString( &iInX, &iBtm, NULL, atStr, 0, FALSE );

	ViewRedrawSetLine( iTop );
	for( i = iTop+1; iBtm > i; i++ )
	{
		DocBadSpaceCheck( i );	//	バッド空白チェキ
		ViewRedrawSetLine( i );
	}
	//	改行してるから、これ以降全部再描画必要
	iLns = gstFile.vcCont.at( gixFocusPage ).vcPage.size( );	//	現在行数再認識
	for( ; iLns >  i; i++ ){	ViewRedrawSetLine(  i );	}
	ViewRedrawSetLine( ++i );	//	念のため


	//	最終的なキャレットの位置をリセット
	ViewPosResetCaret( iInX , iBtm );

	DocPageInfoRenew( -1, 1 );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------




/*!
	挿入用ウインドウ作る
	@param[in]	hPrWnd	メインのウインドウハンドル
*/
HWND FrameInsBoxCreate( HINSTANCE hInst, HWND hPrWnd )
{
	INT			x, y;
	UINT		d;
	TCHAR		atBuffer[MAX_STRING];
	RECT		rect, vwRect;
//	TBADDBITMAP	stToolBmp;



	if( ghFrInbxWnd )
	{
		SetForegroundWindow( ghFrInbxWnd );
		return ghFrInbxWnd;
	}

	ZeroMemory( &gstFrmInsInfo, sizeof(FRMINSINFO) );

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

	//	アイコン・とりあえず臨時で
	//stToolBmp.hInst = HINST_COMMCTRL;
	//stToolBmp.nID   = IDB_STD_SMALL_COLOR;
	//SendMessage( ghFIBtlbrWnd, TB_ADDBITMAP, 0, (LPARAM)&stToolBmp );

	SendMessage( ghFIBtlbrWnd, TB_SETIMAGELIST, 0, (LPARAM)ghFrameImgLst );

	SendMessage( ghFIBtlbrWnd, TB_SETBUTTONSIZE, 0, MAKELPARAM(16,16) );

	SendMessage( ghFIBtlbrWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );

	//	ツールチップ文字列を設定・ボタンテキストがツールチップになる
	StringCchCopy( atBuffer, MAX_STRING, TEXT("挿入する") );
	gstFIBTBInfo[0].iString = SendMessage( ghFIBtlbrWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	for( d = 0; 10 > d; d++ )
	{
		FrameNameLoad( d, atBuffer, MAX_STRING );
		gstFIBTBInfo[d+2].iString = SendMessage( ghFIBtlbrWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );
	}
	StringCchCopy( atBuffer, MAX_STRING, TEXT("挿入したら閉じる") );
	gstFIBTBInfo[13].iString = SendMessage( ghFIBtlbrWnd, TB_ADDSTRING, 0, (LPARAM)atBuffer );

	SendMessage( ghFIBtlbrWnd , TB_ADDBUTTONS, (WPARAM)TB_ITEMS, (LPARAM)&gstFIBTBInfo );	//	ツールバーにボタンを挿入

	SendMessage( ghFIBtlbrWnd , TB_AUTOSIZE, 0, 0 );	//	ボタンのサイズに合わせてツールバーをリサイズ
	InvalidateRect( ghFIBtlbrWnd , NULL, TRUE );		//	クライアント全体を再描画する命令

	//	初期状態としてアルファをチェキ状態に
	SendMessage( ghFIBtlbrWnd, TB_CHECKBUTTON, IDM_INSFRAME_ALPHA, TRUE );
	ghSelect = 0;

	//	直ぐ閉じるかどうか
	SendMessage( ghFIBtlbrWnd, TB_CHECKBUTTON, IDM_FRMINSBOX_QCLOSE, gbQuickClose );
	

	FrameInsBoxInfoCheck(  );	//	初期状態データ作成


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
	枠情報に合わせてデータ作る
*/
HRESULT FrameInsBoxInfoCheck( VOID )
{
/*
専用文字列
左上＋真ん中、ここまでの長さが真ん中の右オフセット開始地点
右上・幅からはみ出さないように、超えない最大を求める
左下＋真ん中＋右下、数数えて並べるだけ
*/
	LPTSTR		ptBuffer;
	INT			dWid, dLftTopWid, dHei, iMidDot;
	UINT_PTR	cchLeft, cchCentre, cchRight, dCnt, d;
	RECT		stFrmRct;

	FRAMEINFO	stInfo;


	FrameInsBoxSizeGet( &stFrmRct );

	FREE( gstFrmInsInfo.ptRoof );
	FREE( gstFrmInsInfo.ptMiddle );
	FREE( gstFrmInsInfo.ptFloor );

	//	枠パーツ情報確保
	FrameDataGet( ghSelect, &stInfo );

	//	上の幅
	dWid  = stFrmRct.right - stInfo.stMorning.dDot;	//	左上引いて
	dWid -= stInfo.stAfternoon.dDot;	//	右上も引くと、上のMAXドット数
	//	上のパーツ数・小数点切り捨てでおｋ
	gstFrmInsInfo.dRfItem = dWid / stInfo.stNoon.dDot;

	//	各パーツの文字数確認
	StringCchLength( stInfo.stMorning.atParts, PARTS_CCH, &cchLeft );
	StringCchLength( stInfo.stNoon.atParts, PARTS_CCH, &cchCentre );
	StringCchLength( stInfo.stAfternoon.atParts, PARTS_CCH, &cchRight );
	//	文字列作成
	dCnt = cchLeft + (cchCentre * gstFrmInsInfo.dRfItem) + cchRight + 2;
	gstFrmInsInfo.ptRoof = (LPTSTR)malloc( dCnt * sizeof(TCHAR) );
	ZeroMemory( gstFrmInsInfo.ptRoof, dCnt * sizeof(TCHAR) );
	StringCchCopy( gstFrmInsInfo.ptRoof, dCnt, stInfo.stMorning.atParts );
	for( d = 0; gstFrmInsInfo.dRfItem > d; d++ ){	StringCchCat( gstFrmInsInfo.ptRoof, dCnt, stInfo.stNoon.atParts );	}
	StringCchCat( gstFrmInsInfo.ptRoof, dCnt, stInfo.stAfternoon.atParts );


	//	左上＋上のドット数・真ん中の幅
	dLftTopWid  = gstFrmInsInfo.dRfItem * stInfo.stNoon.dDot;
	gstFrmInsInfo.dLftRfDot = dLftTopWid + stInfo.stMorning.dDot;
	//	真ん中の空白の幅
	iMidDot  = gstFrmInsInfo.dLftRfDot - stInfo.stDaybreak.dDot;	//	左ひいて
	iMidDot += stInfo.dRightOffset;	//	右オフセット足す
	ptBuffer = DocPaddingSpaceMake( iMidDot );	//	空白作成
	//	各パーツの文字数確認
	StringCchLength( stInfo.stDaybreak.atParts, PARTS_CCH, &cchLeft );
	StringCchLength( ptBuffer, STRSAFE_MAX_CCH, &cchCentre );	//	作った空白
	StringCchLength( stInfo.stNightfall.atParts, PARTS_CCH, &cchRight );
	//	文字列作成
	dCnt = cchLeft + cchCentre + cchRight + 2;
	gstFrmInsInfo.ptMiddle = (LPTSTR)malloc( dCnt * sizeof(TCHAR) );
	ZeroMemory( gstFrmInsInfo.ptMiddle, dCnt * sizeof(TCHAR) );
	StringCchCopy( gstFrmInsInfo.ptMiddle, dCnt, stInfo.stDaybreak.atParts );
	StringCchCat( gstFrmInsInfo.ptMiddle, dCnt, ptBuffer );
	StringCchCat( gstFrmInsInfo.ptMiddle, dCnt, stInfo.stNightfall.atParts );
	FREE(ptBuffer);


	//	下の幅
	dWid  = stFrmRct.right - stInfo.stDawn.dDot;	//	左下引いて
	dWid -= stInfo.stTwilight.dDot;	//	右下も引くと、下のMAXドット数
	//	下のパーツ数・小数点切り捨てでおｋ
	gstFrmInsInfo.dFlrItem = dWid / stInfo.stMidnight.dDot;

	//	各パーツの文字数確認
	StringCchLength( stInfo.stDawn.atParts, PARTS_CCH, &cchLeft );
	StringCchLength( stInfo.stMidnight.atParts, PARTS_CCH, &cchCentre );
	StringCchLength( stInfo.stTwilight.atParts, PARTS_CCH, &cchRight );
	//	文字列作成
	dCnt = cchLeft + (cchCentre * gstFrmInsInfo.dFlrItem) + cchRight + 2;
	gstFrmInsInfo.ptFloor = (LPTSTR)malloc( dCnt * sizeof(TCHAR) );
	ZeroMemory( gstFrmInsInfo.ptFloor, dCnt * sizeof(TCHAR) );
	StringCchCopy( gstFrmInsInfo.ptFloor, dCnt, stInfo.stDawn.atParts );
	for( d = 0; gstFrmInsInfo.dFlrItem > d; d++ ){	StringCchCat( gstFrmInsInfo.ptFloor, dCnt, stInfo.stMidnight.atParts );	}
	StringCchCat( gstFrmInsInfo.ptFloor, dCnt, stInfo.stTwilight.atParts );


	//	行数
	dHei =  stFrmRct.bottom - (LINE_HEIGHT * 2);	//	縦ドット数
	if( 0 >= dHei ){	gstFrmInsInfo.dMidLines = 0;	}
	else{	gstFrmInsInfo.dMidLines = dHei / LINE_HEIGHT;	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	挿入実行
	@param[in]	hWnd	ウインドウハンドル
*/
HRESULT FrameInsBoxDoInsert( HWND hWnd )
{
	LPTSTR		ptBuffer;
	UINT_PTR	cchRoof = 0, cchMidd = 0, cchFlor = 0, cchTotal;
	UINT		d;
	INT			iX, iY;
	HWND		hLyrWnd;
	RECT		rect;

	//	挿入用文字列を作成
	StringCchLength( gstFrmInsInfo.ptRoof,   STRSAFE_MAX_CCH, &cchRoof );
	StringCchLength( gstFrmInsInfo.ptMiddle, STRSAFE_MAX_CCH, &cchMidd );
	StringCchLength( gstFrmInsInfo.ptFloor,  STRSAFE_MAX_CCH, &cchFlor );

	//	各行に改行、最終行には不要、ヌルターミネータ用の余裕
	cchTotal = (cchRoof + 2) + (cchMidd + 2) * gstFrmInsInfo.dMidLines + cchFlor + 2;

	ptBuffer = (LPTSTR)malloc( cchTotal * sizeof(TCHAR) );
	ZeroMemory( ptBuffer, cchTotal * sizeof(TCHAR) );

	StringCchPrintf( ptBuffer, cchTotal, TEXT("%s\r\n"), gstFrmInsInfo.ptRoof );
	for( d = 0; gstFrmInsInfo.dMidLines > d; d++ )
	{
		StringCchCat( ptBuffer, cchTotal, gstFrmInsInfo.ptMiddle );
		StringCchCat( ptBuffer, cchTotal, TEXT("\r\n") );
	}
	StringCchCat( ptBuffer, cchTotal, gstFrmInsInfo.ptFloor );

	//	挿入処理には、レイヤボックスを非表示処理で使う
	hLyrWnd = LayerBoxVisibalise( GetModuleHandle(NULL), ptBuffer, 0x10 );
	FREE(ptBuffer);
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

	switch( id )
	{
		case IDM_FRAME_INS_DECIDE:	FrameInsBoxDoInsert( hWnd );	return;

		case IDM_INSFRAME_ALPHA:	ghSelect = 0;	break;
		case IDM_INSFRAME_BRAVO:	ghSelect = 1;	break;
		case IDM_INSFRAME_CHARLIE:	ghSelect = 2;	break;
		case IDM_INSFRAME_DELTA:	ghSelect = 3;	break;
		case IDM_INSFRAME_ECHO:		ghSelect = 4;	break;
		case IDM_INSFRAME_FOXTROT:	ghSelect = 5;	break;
		case IDM_INSFRAME_GOLF:		ghSelect = 6;	break;
		case IDM_INSFRAME_HOTEL:	ghSelect = 7;	break;
		case IDM_INSFRAME_INDIA:	ghSelect = 8;	break;
		case IDM_INSFRAME_JULIETTE:	ghSelect = 9;	break;

		case IDM_FRMINSBOX_QCLOSE:	gbQuickClose = SendMessage( ghFIBtlbrWnd, TB_ISBUTTONCHECKED, IDM_FRMINSBOX_QCLOSE, 0 );	break;

		default:	return;
	}

	FrameInsBoxInfoCheck(  );
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
	UINT	i;
	HFONT	hOldFnt;
	INT		topOst, iYpos;
	RECT	stFrmRct;

	FRAMEINFO	stInfo;


//	SetBkMode( hDC, OPAQUE );
	SetBkColor( hDC, ViewBackColourGet( NULL ) );	//	

	FrameDataGet( ghSelect, &stInfo );	//	枠パーツ情報確保

	hOldFnt = SelectFont( hDC, ghAaFont );	//	フォントくっつける

	topOst = FrameInsBoxSizeGet( &stFrmRct );	//	FRAME当てはめ枠のサイズ

	//	天井描画
	iYpos = topOst;
	FrameDrawItem( hDC, iYpos, gstFrmInsInfo.ptRoof );
	//StringCchLength( gstFrmInsInfo.ptRoof, STRSAFE_MAX_CCH, &cchSize );
	//ExtTextOut( hDC, 0, iYpos, 0, NULL, gstFrmInsInfo.ptRoof, cchSize, NULL );
	iYpos += LINE_HEIGHT;
	//	間描画
	//StringCchLength( gstFrmInsInfo.ptMiddle, STRSAFE_MAX_CCH, &cchSize );
	for( i = 0; gstFrmInsInfo.dMidLines > i; i++ )
	{
		FrameDrawItem( hDC, iYpos, gstFrmInsInfo.ptMiddle );
		//ExtTextOut( hDC, 0, iYpos, 0, NULL, gstFrmInsInfo.ptMiddle, cchSize, NULL );
		iYpos += LINE_HEIGHT;
	}
	//	床描画
	FrameDrawItem( hDC, iYpos, gstFrmInsInfo.ptFloor );
	//StringCchLength( gstFrmInsInfo.ptFloor, STRSAFE_MAX_CCH, &cchSize );
	//ExtTextOut( hDC, 0, iYpos, 0, NULL, gstFrmInsInfo.ptFloor, cchSize, NULL );



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

	FREE( gstFrmInsInfo.ptRoof );
	FREE( gstFrmInsInfo.ptMiddle );
	FREE( gstFrmInsInfo.ptFloor );

	StatusBarSetText( SB_LAYER, TEXT("") );

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
	StatusBarSetText( SB_LAYER, atBuffer );

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

	MoveWindow( ghFIBtlbrWnd, 0, 0, 0, 0, TRUE );	//	ツールバーは数値なくても勝手に合わせてくれる

	FrameInsBoxInfoCheck(  );	//	表示具合を変更して再描画する
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


