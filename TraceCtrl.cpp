/*! @file
	@brief トレス機能の面倒見ます・画像保存の面倒も見ます
	このファイルは TraceCtrl.cpp です。
	@author	SikigamiHNQ
	@date	2011/07/19
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


//	画像保存機能にも使うから注意
#define IMGCTL_RUNTIME
#include "imgctl.h"
//-------------------------------------------------------------------------------------------------

/*
スライダとスクロールバーのダイヤログリソースの使い方
    CONTROL         "",IDC_SLIDER1,"msctls_trackbar32",TBS_BOTH | TBS_NOTICKS | WS_TABSTOP,14,66,100,15
    SCROLLBAR       IDC_SCROLLBAR1,134,68,82,11
*/
#define TRC_SCROLLBAR

#define TRC_POSITION_RANGE	2000	//	位置合わせ－１０００～１０００
#define TRC_POSITION_OFFSET	1000	//	これが基準位置

#define TRC_CONTRA_RANGE	 510	//	コントラストは－２５５～２５５
#define TRC_CONTRA_OFFSET	 255	//	これで０（操作無し）

#define TRC_GAMMA_RANGE		3000	//	ガンマ　０～３．０
#define TRC_GAMMA_OFFSET	1000	//	これで１．０（操作無し）・この値そのまま指定できる

#define TRC_GRAYMOPH_RANGE	 255	//	淡色化　０～２５５

#define TRC_ZOOM_RANGE		 250	//	拡大縮小は５０％～３００％
#define TRC_ZOOM_OFFSET		  50	//	これで１００％（操作無し）・足して１００で割る

#define TRC_TURN_RANGE		 359	//	一週すればいい
//-------------------------------------------------------------------------------------------------

static HMODULE		ghImgctl;		//!<	ImgCtl.dllのハンドル

TODIB		gpifToDIB;		//!<	ファイル名からイメージゲット
DELETEDIB	gpifDeleteDIB;	//!<	画像データを破棄する
HEADDIB		gpifHeadDIB;	//!<	画像のデータをゲッツ
COPYDIB		gpifCopyDIB;	//!<	
TURNDIBEX	gpifTurnDIBex;	//!<	画像を任意の角度に回転する
GRAYDIB		gpifGrayDIB;	//!<	画像を淡色化する
GAMMADIB	gpifGammaDIB;	//!<	ガンマ補正
CONTRASTDIB	gpifContrastDIB;//!<	コントラスト補正

DIBTODCEX	gpifDIBtoDCex;	//!<	画像データをディバイスコンテキスツにぺたっと

DCTODIB		gpifDCtoDIB;	//!<	デバイスコンテキストを画像でぇたに
DIBTOBMP	gpifDIBtoBMP;	//!<	
//DIBTOJPG	gpifDIBtoJPG;	//!<	
DIBTOPNG	gpifDIBtoPNG;	//!<	画像データをPNG形式でアウツ

static  HDIB	ghOutDib;				//!<	ファイル出力用
static  HDIB	ghImgDib, ghOrigDib;	//!<	トレス表示用
static  SIZE	gstImgSize;				//!<	読み込んだ画像のサイズ

static HBRUSH	gMoziClrBrush;	//!<	文字色ブラシ

static  HWND	ghTraceDlg;		//!<	トレス機能モーダレスダイヤログハンドル

static BOOLEAN	gbThumbUse;		//!<	画像オーポンダイヤローグでサムネールを有効に

//	全てスライダの数値を記録する
static TRACEPARAM	gstTrcPrm;

static BOOLEAN	gbOnView;		//!<	非０で表示
//-------------------------------------------------------------------------------------------------

INT_PTR	CALLBACK TraceCtrlDlgProc( HWND, UINT, WPARAM, LPARAM );

HRESULT	TraceImageFileOpen( HWND );
HRESULT	TraceMoziColourChoice( HWND );

HRESULT	TraceRedrawIamge( VOID );

INT_PTR	TraceOnScroll( HWND, HWND, UINT, INT );
//-------------------------------------------------------------------------------------------------

/*!
	ImgCtl.dllをロードする
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	bMode	非０作成　０破壊
*/
INT TraceInitialise( HWND hWnd, UINT bMode )
{
	if( bMode )
	{
		ghTraceDlg = NULL;
		ghImgDib = NULL;
		ghOrigDib = NULL;

		ghImgctl = LoadLibrary( TEXT("imgctl.dll") );

//トレスDIALOGUE、拡大縮小がない

		gpifToDIB       = (TODIB)GetProcAddress( ghImgctl, "ToDIB" );
		gpifDeleteDIB   = (DELETEDIB)GetProcAddress( ghImgctl, "DeleteDIB" );
		gpifHeadDIB     = (HEADDIB)GetProcAddress( ghImgctl, "HeadDIB" );

		gpifCopyDIB     = (COPYDIB)GetProcAddress( ghImgctl, "CopyDIB" );

		gpifTurnDIBex   = (TURNDIBEX)GetProcAddress( ghImgctl, "TurnDIBex" );
		gpifGrayDIB     = (GRAYDIB)GetProcAddress( ghImgctl, "GrayDIB" );
		gpifGammaDIB    = (GAMMADIB)GetProcAddress( ghImgctl, "GammaDIB" );
		gpifContrastDIB = (CONTRASTDIB)GetProcAddress( ghImgctl, "ContrastDIB" );

		gpifDIBtoDCex   = (DIBTODCEX)GetProcAddress( ghImgctl, "DIBtoDCex" );
		//	これexでないといかんか

		//	画像de保存用
		gpifDCtoDIB  = (DCTODIB)GetProcAddress( ghImgctl, "DCtoDIB" );
		gpifDIBtoBMP = (DIBTOBMP)GetProcAddress( ghImgctl, "DIBtoBMP" );
	//	gpifDIBtoJPG = (DIBTOJPG)GetProcAddress( ghImgctl, "DIBtoJPG" );
		gpifDIBtoPNG = (DIBTOPNG)GetProcAddress( ghImgctl, "DIBtoPNG" );



		gstTrcPrm.stOffsetPt.x = TRC_POSITION_OFFSET;
		gstTrcPrm.stOffsetPt.y = TRC_POSITION_OFFSET;

		gstTrcPrm.dContrast = TRC_CONTRA_OFFSET;
		gstTrcPrm.dGamma    = TRC_GAMMA_OFFSET;
		gstTrcPrm.dGrayMoph = 0;
		gstTrcPrm.dZooming  = TRC_ZOOM_OFFSET;
		gstTrcPrm.dTurning  = 0;

		gstTrcPrm.bUpset  = BST_UNCHECKED;
		gstTrcPrm.bMirror = BST_UNCHECKED;

		gstTrcPrm.dMoziColour = ViewMoziColourGet( NULL );

		InitTraceValue( INIT_LOAD, &gstTrcPrm );

		gbOnView = TRUE;

		gbThumbUse = TRUE;
	}
	else
	{
		if( ghImgDib  ){	gpifDeleteDIB( ghImgDib );	ghImgDib = NULL;	}
		if( ghOrigDib ){	gpifDeleteDIB( ghOrigDib );	ghOrigDib = NULL;	}

		if( ghOutDib  ){	gpifDeleteDIB( ghOutDib );	ghOutDib = NULL;	}

		if( ghImgctl )		FreeLibrary( ghImgctl );
		if( ghTraceDlg )	DestroyWindow( ghTraceDlg );
	}

	return 0;
}
//-------------------------------------------------------------------------------------------------

/*!
	モーダレストレスダイヤログを開けるを試みるですだよ
	@param[in]	hInst	アプリの実存
	@param[in]	hWnd	メインウインドウハンドルであるように
*/
HRESULT TraceDialogueOpen( HINSTANCE hInst, HWND hWnd )
{
	HWND	hDktpWnd;
	LONG	x;
	RECT	rect, dtRect, trRect;

	if( !(ghImgctl) )	return E_HANDLE;	//	DLL死んでたらアウツ！

	if( ghTraceDlg )
	{
		//SetForegroundWindow( ghTraceDlg );
		PostMessage( ghTraceDlg, WM_CLOSE, 0, 0 );
		return S_OK;
	}

	GetWindowRect( hWnd, &rect );	//	メイン窓のスクリーン座標
	
	//	スクロールバーにしてみるテスト
#ifdef TRC_SCROLLBAR
	ghTraceDlg = CreateDialogParam( hInst, MAKEINTRESOURCE(IDD_TRACEADJUST_DLG2), hWnd, TraceCtrlDlgProc, 0 );
#else
	ghTraceDlg = CreateDialogParam( hInst, MAKEINTRESOURCE(IDD_TRACEADJUST_DLG), hWnd, TraceCtrlDlgProc, 0 );
#endif
	GetClientRect( ghTraceDlg, &trRect );	//	トレス制御窓のサイズ

	//	ディスクトップからはみ出さないように
	hDktpWnd = GetDesktopWindow(  );
	GetWindowRect( hDktpWnd, &dtRect );

	x = dtRect.right - rect.right;	//	右の余裕確認
	if( trRect.right >  x ){	rect.right = dtRect.right - trRect.right;	}
	//	足りないようなら、表示位置をオフセットしておく

	SetWindowPos( ghTraceDlg, HWND_TOP, rect.right, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );

	if( ghOrigDib ){	ViewRedrawSetLine( -1 );	}

	MenuItemCheckOnOff( IDM_TRACE_MODE_ON, TRUE );

	AppTitleTrace( TRUE );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	トレスコントロールダイヤログのプロシージャ
	@param[in]	hDlg		ダイヤログハンドル
	@param[in]	message		ウインドウメッセージの識別番号
	@param[in]	wParam		追加の情報１
	@param[in]	lParam		追加の情報２
	@retval 0	メッセージは処理していない
	@retval no0	なんか処理された
*/
INT_PTR CALLBACK TraceCtrlDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	HWND	hWorkWnd;
	UINT	id;
	HDC		hdc;
	HWND	hWndChild;
	RECT	rect;
//	LONG	lRslt;
	COLORREF	caretColour;

#ifdef TRC_SCROLLBAR
	SCROLLINFO	stSclInfo;
#endif

	switch( message )
	{
		default:	break;

		case WM_INITDIALOG:
			gMoziClrBrush = CreateSolidBrush( gstTrcPrm.dMoziColour );

			hWorkWnd = GetDlgItem( hDlg, IDSL_TRC_HRIZ_POS );
#ifdef TRC_SCROLLBAR
			ZeroMemory( &stSclInfo, sizeof(SCROLLINFO) );
			stSclInfo.cbSize = sizeof(SCROLLINFO);
			stSclInfo.fMask  = SIF_RANGE;

			stSclInfo.nMax   = TRC_POSITION_RANGE;	//	nMin＝０
			SetScrollInfo( hWorkWnd, SB_CTL, &stSclInfo, TRUE );
#else
			SendMessage( hWorkWnd, TBM_SETRANGE, TRUE, (LPARAM)MAKELONG(0, TRC_POSITION_RANGE) );	//	
			SendMessage( hWorkWnd, TBM_SETPOS, TRUE , gstTrcPrm.stOffsetPt.x );	//	
			SendMessage( hWorkWnd, TBM_SETPAGESIZE, 0, 25 );
#endif
			TraceOnScroll( hDlg, hWorkWnd, TB_THUMBPOSITION, gstTrcPrm.stOffsetPt.x );

			hWorkWnd = GetDlgItem( hDlg, IDSL_TRC_VART_POS );
#ifdef TRC_SCROLLBAR
			stSclInfo.nMax   = TRC_POSITION_RANGE;	//	nMin＝０
			SetScrollInfo( hWorkWnd, SB_CTL, &stSclInfo, TRUE );
#else
			SendMessage( hWorkWnd, TBM_SETRANGE, TRUE, (LPARAM)MAKELONG(0, TRC_POSITION_RANGE) );	//	
			SendMessage( hWorkWnd, TBM_SETPOS, TRUE , gstTrcPrm.stOffsetPt.y );	//	
			SendMessage( hWorkWnd, TBM_SETPAGESIZE, 0, 25 );
#endif
			TraceOnScroll( hDlg, hWorkWnd, TB_THUMBPOSITION, gstTrcPrm.stOffsetPt.y );

			hWorkWnd = GetDlgItem( hDlg, IDSL_TRC_CONTRAST );
#ifdef TRC_SCROLLBAR
			stSclInfo.nMax   = TRC_CONTRA_RANGE;	//	nMin＝０
			SetScrollInfo( hWorkWnd, SB_CTL, &stSclInfo, TRUE );
#else
			SendMessage( hWorkWnd, TBM_SETRANGE, TRUE, (LPARAM)MAKELONG(0, TRC_CONTRA_RANGE) );	//	コントラスト
			SendMessage( hWorkWnd, TBM_SETPOS, TRUE , gstTrcPrm.dContrast );	//	
			SendMessage( hWorkWnd, TBM_SETPAGESIZE, 0, 10 );
#endif
			TraceOnScroll( hDlg, hWorkWnd, TB_THUMBPOSITION, gstTrcPrm.dContrast );

			hWorkWnd = GetDlgItem( hDlg, IDSL_TRC_GAMMA );
#ifdef TRC_SCROLLBAR
			stSclInfo.nMax   = TRC_GAMMA_RANGE;	//	nMin＝０
			SetScrollInfo( hWorkWnd, SB_CTL, &stSclInfo, TRUE );
#else
			SendMessage( hWorkWnd, TBM_SETRANGE, TRUE, (LPARAM)MAKELONG(0, TRC_GAMMA_RANGE) );	//	ガンマ
			SendMessage( hWorkWnd, TBM_SETPOS, TRUE , gstTrcPrm.dGamma );	//	
			SendMessage( hWorkWnd, TBM_SETPAGESIZE, 0, 100 );
#endif
			TraceOnScroll( hDlg, hWorkWnd, TB_THUMBPOSITION, gstTrcPrm.dGamma );

			hWorkWnd = GetDlgItem( hDlg, IDSL_TRC_GRAYMOPH );
#ifdef TRC_SCROLLBAR
			stSclInfo.nMax   = TRC_GRAYMOPH_RANGE;	//	nMin＝０
			SetScrollInfo( hWorkWnd, SB_CTL, &stSclInfo, TRUE );
#else
			SendMessage( hWorkWnd, TBM_SETRANGE, TRUE , (LPARAM)MAKELONG(0, TRC_GRAYMOPH_RANGE) );	//	淡色化
			SendMessage( hWorkWnd, TBM_SETPOS, TRUE , gstTrcPrm.dGrayMoph );	//	
			SendMessage( hWorkWnd, TBM_SETPAGESIZE, 0, 10 );
#endif
			TraceOnScroll( hDlg, hWorkWnd, TB_THUMBPOSITION, gstTrcPrm.dGrayMoph );

			hWorkWnd = GetDlgItem( hDlg, IDSL_TRC_ZOOM );
#ifdef TRC_SCROLLBAR
			stSclInfo.nMax   = TRC_ZOOM_RANGE;	//	nMin＝０
			SetScrollInfo( hWorkWnd, SB_CTL, &stSclInfo, TRUE );
#else
			SendMessage( hWorkWnd, TBM_SETRANGE, TRUE , (LPARAM)MAKELONG(0, TRC_ZOOM_RANGE) );	//	拡大縮小
			SendMessage( hWorkWnd, TBM_SETPOS, TRUE, gstTrcPrm.dZooming );	//	合ってるか？
			SendMessage( hWorkWnd, TBM_SETPAGESIZE, 0, 5 );
#endif
			TraceOnScroll( hDlg, hWorkWnd, TB_THUMBPOSITION, gstTrcPrm.dZooming );

			hWorkWnd = GetDlgItem( hDlg, IDSL_TRC_TURN );
#ifdef TRC_SCROLLBAR
			stSclInfo.nMax   = TRC_TURN_RANGE;	//	nMin＝０
			SetScrollInfo( hWorkWnd, SB_CTL, &stSclInfo, TRUE );
#else
			SendMessage( hWorkWnd, TBM_SETRANGE, TRUE , (LPARAM)MAKELONG(0, TRC_TURN_RANGE) );	//	回転
			SendMessage( hWorkWnd, TBM_SETPOS, TRUE, gstTrcPrm.dTurning );	//	合ってるか？
			SendMessage( hWorkWnd, TBM_SETPAGESIZE, 0, 5 );
#endif
			TraceOnScroll( hDlg, hWorkWnd, TB_THUMBPOSITION, gstTrcPrm.dTurning );

			CheckDlgButton( hDlg, IDCB_TRC_IMG_UPSET, gstTrcPrm.bUpset );

			CheckDlgButton( hDlg, IDCB_TRC_IMG_MIRROR, gstTrcPrm.bMirror );

			ViewRedrawSetLine( -1 );

			return (INT_PTR)TRUE;

		case WM_CTLCOLORSTATIC:
			hdc = (HDC)(wParam);
			hWndChild = (HWND)(lParam);
			id = GetDlgCtrlID( hWndChild );
			if( IDPL_TRC_MOZICOLOUR == id )	//	ターゲットは一つだけ
			{
				GetClientRect( hWndChild, &rect );
				FillRect( hdc, &rect, gMoziClrBrush );
				return (INT_PTR)TRUE;
			}
			break;


		case WM_VSCROLL:
		case WM_HSCROLL:
			return TraceOnScroll( hDlg, (HWND)(lParam), (UINT)(LOWORD(wParam)), (INT)(SHORT)HIWORD(wParam) );


		case WM_COMMAND:
			id = LOWORD(wParam);
			switch( id )
			{
				case IDB_TRC_IMAGEOPEN:		TraceImageFileOpen( hDlg );	return (INT_PTR)TRUE;
				case IDPL_TRC_MOZICOLOUR:	TraceMoziColourChoice( hDlg );	return (INT_PTR)TRUE;

				case IDCB_TRC_IMG_UPSET:	gstTrcPrm.bUpset  = IsDlgButtonChecked( hDlg, IDCB_TRC_IMG_UPSET  );	ViewRedrawSetLine( -1 );	break;
				case IDCB_TRC_IMG_MIRROR:	gstTrcPrm.bMirror = IsDlgButtonChecked( hDlg, IDCB_TRC_IMG_MIRROR );	ViewRedrawSetLine( -1 );	break;

				case IDB_TRC_VIEWTOGGLE:
					gbOnView = gbOnView ? FALSE : TRUE;
					SetDlgItemText( hDlg, IDB_TRC_VIEWTOGGLE, gbOnView ? TEXT("非表示") : TEXT("表示") );
					ViewRedrawSetLine( -1 );
					ViewFocusSet(  );	//	20110728	フォーカスを描画に戻す
					break;

				case IDB_TRC_RESET:
					if( ghImgDib  ){	gpifDeleteDIB( ghImgDib );	ghImgDib = NULL;	}
					if( ghOrigDib ){	ghImgDib = gpifCopyDIB( ghOrigDib );	}

					gstTrcPrm.stOffsetPt.x = TRC_POSITION_OFFSET;
					gstTrcPrm.stOffsetPt.y = TRC_POSITION_OFFSET;

					gstTrcPrm.dContrast = TRC_CONTRA_OFFSET;
					gstTrcPrm.dGamma    = TRC_GAMMA_OFFSET;
					gstTrcPrm.dGrayMoph = 0;
					gstTrcPrm.dZooming  = TRC_ZOOM_OFFSET;
					gstTrcPrm.dTurning  = 0;

					CheckDlgButton( hDlg, IDCB_TRC_IMG_UPSET, BST_UNCHECKED );
					gstTrcPrm.bUpset  = BST_UNCHECKED;
					CheckDlgButton( hDlg, IDCB_TRC_IMG_MIRROR, BST_UNCHECKED );
					gstTrcPrm.bMirror = BST_UNCHECKED;

#ifdef TRC_SCROLLBAR
					SendDlgItemMessage( hDlg, IDSL_TRC_HRIZ_POS, SBM_SETPOS, gstTrcPrm.stOffsetPt.x, TRUE );
					SendDlgItemMessage( hDlg, IDSL_TRC_VART_POS, SBM_SETPOS, gstTrcPrm.stOffsetPt.y, TRUE );	//	
					SendDlgItemMessage( hDlg, IDSL_TRC_CONTRAST, SBM_SETPOS, gstTrcPrm.dContrast, TRUE );	//	－２５５～２５５なので
					SendDlgItemMessage( hDlg, IDSL_TRC_GAMMA,    SBM_SETPOS, gstTrcPrm.dGamma, TRUE );		//	
					SendDlgItemMessage( hDlg, IDSL_TRC_GRAYMOPH, SBM_SETPOS, gstTrcPrm.dGrayMoph, TRUE );	//	
					SendDlgItemMessage( hDlg, IDSL_TRC_ZOOM,     SBM_SETPOS, gstTrcPrm.dZooming, TRUE  );	//	
					SendDlgItemMessage( hDlg, IDSL_TRC_TURN,     SBM_SETPOS, gstTrcPrm.dTurning, TRUE  );	//	
#else
					SendDlgItemMessage( hDlg, IDSL_TRC_HRIZ_POS, TBM_SETPOS, TRUE, gstTrcPrm.stOffsetPt.x );	//	
					SendDlgItemMessage( hDlg, IDSL_TRC_VART_POS, TBM_SETPOS, TRUE, gstTrcPrm.stOffsetPt.y );	//	
					SendDlgItemMessage( hDlg, IDSL_TRC_CONTRAST, TBM_SETPOS, TRUE, gstTrcPrm.dContrast );	//	－２５５～２５５なので
					SendDlgItemMessage( hDlg, IDSL_TRC_GAMMA,    TBM_SETPOS, TRUE, gstTrcPrm.dGamma );		//	
					SendDlgItemMessage( hDlg, IDSL_TRC_GRAYMOPH, TBM_SETPOS, TRUE, gstTrcPrm.dGrayMoph );	//	
					SendDlgItemMessage( hDlg, IDSL_TRC_ZOOM,     TBM_SETPOS, TRUE, gstTrcPrm.dZooming  );	//	
					SendDlgItemMessage( hDlg, IDSL_TRC_TURN,     TBM_SETPOS, TRUE, gstTrcPrm.dTurning  );	//	
#endif

					TraceOnScroll( hDlg, GetDlgItem( hDlg, IDSL_TRC_HRIZ_POS ), TB_THUMBPOSITION, gstTrcPrm.stOffsetPt.x );
					TraceOnScroll( hDlg, GetDlgItem( hDlg, IDSL_TRC_VART_POS ), TB_THUMBPOSITION, gstTrcPrm.stOffsetPt.y );
					TraceOnScroll( hDlg, GetDlgItem( hDlg, IDSL_TRC_CONTRAST ), TB_THUMBPOSITION, gstTrcPrm.dContrast );
					TraceOnScroll( hDlg, GetDlgItem( hDlg, IDSL_TRC_GAMMA ),    TB_THUMBPOSITION, gstTrcPrm.dGamma );
					TraceOnScroll( hDlg, GetDlgItem( hDlg, IDSL_TRC_GRAYMOPH ), TB_THUMBPOSITION, gstTrcPrm.dGrayMoph );
					TraceOnScroll( hDlg, GetDlgItem( hDlg, IDSL_TRC_ZOOM ),     TB_THUMBPOSITION, gstTrcPrm.dZooming );
					TraceOnScroll( hDlg, GetDlgItem( hDlg, IDSL_TRC_TURN ),     TB_THUMBPOSITION, gstTrcPrm.dTurning );

					gbOnView = TRUE;

					ViewRedrawSetLine( -1 );
					ViewFocusSet(  );	//	20110728	フォーカスを描画に戻す
					break;

				//case IDCANCEL:	//	要らないかな？
				//case IDOK:
				//	//	なんか処理
				//	DestroyWindow( hDlg );
				//	ghTraceDlg = NULL;
				//	return (INT_PTR)TRUE;

				default:	break;
			}
			break;


		case WM_CLOSE:
	//		if( ghImgDib  ){	gpifDeleteDIB( ghImgDib );	ghImgDib = NULL;	}
			DestroyWindow( hDlg );
			ghTraceDlg = NULL;
			ViewMoziColourGet( &caretColour );	//	キャレット色元に戻す
			ViewCaretReColour( caretColour );
			ViewRedrawSetLine( -1 );	//	描画領域再描画
			DeleteBrush( gMoziClrBrush );
			MenuItemCheckOnOff( IDM_TRACE_MODE_ON, FALSE );
			return (INT_PTR)TRUE;

		case WM_DESTROY:
			InitTraceValue( INIT_SAVE, &gstTrcPrm );
			ViewFocusSet(  );	//	20110728	フォーカスを描画に戻す
			AppTitleTrace( FALSE);	//	タイトルバー元に戻す
			return (INT_PTR)TRUE;
	}

	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------



/*!
	開くダイアログのフックプロシージャ
*/
UINT_PTR CALLBACK ImageOpenDlgHookProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static  HWND	chUseCbxWnd, chPanelWnd;
	static  HDIB	chThumbDib;		//	サムネ用

	HWND	hWndCtl, hWndChild, hWnd;
	HDC		hDC;
	INT		idCtrl, id, cx, cy;
	UINT	codeNotify, state;
	TCHAR	atFile[MAX_PATH], atSpec[MAX_PATH];
	CHAR	acName[MAX_PATH];
	LPOFNOTIFY	pstOfNty;
	RECT	rect, dlgRect;
	POINT	stPoint;
	SIZE	stSize;

	//	コモンダイヤログの位置は変更出来ない

	switch( message )
	{
		case WM_INITDIALOG:
			TRACE( TEXT("HOOK INIT") );
			chThumbDib = NULL;

			chUseCbxWnd = GetDlgItem( hDlg, IDCB_TRC_DLG_USETHUMB );
			Button_SetCheck( chUseCbxWnd, gbThumbUse ? BST_CHECKED : BST_UNCHECKED );

			chPanelWnd  = GetDlgItem( hDlg, IDS_TRC_DLG_THUMBFRAME );
			return (INT_PTR)TRUE;


		case WM_CTLCOLORSTATIC:
			hDC = (HDC)(wParam);
			hWndChild = (HWND)(lParam);
			if( hWndChild == chPanelWnd )
			{
				GetClientRect( chPanelWnd, &rect );
				if( chThumbDib && gbThumbUse )
				{
					gpifDIBtoDCex( hDC, 0, 0, rect.right, rect.bottom, 
						chThumbDib, 0, 0, 0, 0, SRCCOPY );
				}
				else
				{
					FillRect( hDC, &rect, GetStockBrush( WHITE_BRUSH ) );
				}
			}
			return (INT_PTR)FALSE;


		case WM_COMMAND:
			id = LOWORD(wParam);
			hWndCtl = (HWND)lParam;
			codeNotify = HIWORD(wParam);
			if( IDCB_TRC_DLG_USETHUMB == id )
			{
				if( BST_CHECKED == Button_GetCheck( chUseCbxWnd ) )	gbThumbUse = TRUE;
				else	gbThumbUse = FALSE;
			}
			return (INT_PTR)TRUE;


		case WM_SIZE:
			state = (UINT)(wParam);
			cx = (INT)(SHORT)LOWORD(lParam);
			cy = (INT)(SHORT)HIWORD(lParam);
			hWnd = GetParent(hDlg);
			GetClientRect( hWnd, &dlgRect );
			TRACE( TEXT("%dx%d, %dx%d"), hDlg, hWnd, cx, cy, dlgRect.right, dlgRect.bottom );
			GetWindowRect( chPanelWnd, &rect );
			stPoint.x = rect.left;	stPoint.y = rect.top;
			ScreenToClient( hDlg, &stPoint );
			stSize.cx = (dlgRect.right  - stPoint.x) -  8;
			stSize.cy = (dlgRect.bottom - stPoint.y) - 40;
			SetWindowPos( chPanelWnd, HWND_TOP, 0, 0, stSize.cx, stSize.cy, SWP_NOMOVE | SWP_NOZORDER );
			InvalidateRect( chPanelWnd, NULL, TRUE );
			return (INT_PTR)TRUE;


		case WM_NOTIFY:
			idCtrl = (int)wParam;
			pstOfNty = (LPOFNOTIFY)lParam;
			if( CDN_SELCHANGE == pstOfNty->hdr.code )
			{
				//	pstOfNty->hdr.hwndFrom は、ダイヤログ全体のSTATICみたいなやつ
				TRACE( TEXT("%X,%X,%d"), hDlg, pstOfNty->hdr.hwndFrom, pstOfNty->hdr.idFrom );

				CommDlg_OpenSave_GetSpec( pstOfNty->hdr.hwndFrom, atSpec, MAX_PATH );
				TRACE( TEXT("[SPEC]%s"), atSpec );	//	ファイル名のみ

				CommDlg_OpenSave_GetFilePath( pstOfNty->hdr.hwndFrom, atFile, MAX_PATH );
				TRACE( TEXT("[FILE]%s"), atFile );	//	フルパス

				ZeroMemory( acName, sizeof(acName) );
				WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, atFile, MAX_PATH, acName, MAX_PATH, NULL, NULL );
				if( chThumbDib ){	gpifDeleteDIB( chThumbDib );	chThumbDib = NULL;	}
				chThumbDib = gpifToDIB( acName );

				InvalidateRect( chPanelWnd, NULL, TRUE );
			}
			return (INT_PTR)TRUE;

		case WM_DESTROY:
			if( chThumbDib ){	gpifDeleteDIB( chThumbDib );	}
			return (INT_PTR)TRUE;


		default:	break;
	}

	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------


/*!
	画像を開いてアッー！
	@param[in]	hDlg	ダイヤログハンドル
*/
HRESULT TraceImageFileOpen( HWND hDlg )
{
	BITMAPINFOHEADER	stBIH;

	OPENFILENAME	stOpenFile;
	BOOLEAN	bOpened;

	UINT_PTR	cchSize;
	TCHAR	atFilePath[MAX_PATH], atFileName[MAX_STRING];
	CHAR	acName[MAX_PATH];


	ZeroMemory( &stOpenFile, sizeof(OPENFILENAME) );

	ZeroMemory( atFilePath,  sizeof(atFilePath) );
	ZeroMemory( atFileName,  sizeof(atFileName) );

	stOpenFile.lStructSize     = sizeof(OPENFILENAME);
	stOpenFile.hInstance       = GetModuleHandle( NULL );
	stOpenFile.hwndOwner       = GetDesktopWindow(  );
	stOpenFile.lpstrFilter     = TEXT("画像ファイル ( bmp, png, jpg, gif )\0*.bmp;*.png;*.jpg;*.jpeg;*.jpe;*.gif\0\0");
//	stOpenFile.nFilterIndex    = 1;
	stOpenFile.lpstrFile       = atFilePath;
	stOpenFile.nMaxFile        = MAX_PATH;
	stOpenFile.lpstrFileTitle  = atFileName;
	stOpenFile.nMaxFileTitle   = MAX_STRING;
//	stOpenFile.lpstrInitialDir = 
	stOpenFile.lpstrTitle      = TEXT("画像ファイルを指定するのー");
	stOpenFile.Flags           = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK | OFN_ENABLESIZING;
//	stOpenFile.lpstrDefExt     = TEXT("");

	stOpenFile.lpfnHook        = ImageOpenDlgHookProc;
	stOpenFile.lpTemplateName  = MAKEINTRESOURCE(IDD_IMAGE_OPEN_DLG);

	bOpened = GetOpenFileName( &stOpenFile );

	if( !(bOpened) ){	return  E_ABORT;	}	//	キャンセルしてたら何もしない

	StringCchLength( atFilePath, MAX_PATH, &cchSize );

	ZeroMemory( acName, sizeof(acName) );
	WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, atFilePath, cchSize, acName, MAX_PATH, NULL, NULL );

	//	前の開きっぱなら破壊
	if( ghImgDib  ){	gpifDeleteDIB( ghImgDib );	ghImgDib = NULL;	}
	if( ghOrigDib ){	gpifDeleteDIB( ghOrigDib );	ghOrigDib = NULL;	}

	//	ズームだけは元に戻す
	gstTrcPrm.dZooming  = TRC_ZOOM_OFFSET;
	SendDlgItemMessage( hDlg, IDSL_TRC_ZOOM,     TBM_SETPOS, TRUE, gstTrcPrm.dZooming  );	//	
	TraceOnScroll( hDlg, GetDlgItem( hDlg, IDSL_TRC_ZOOM ),     TB_THUMBPOSITION, gstTrcPrm.dZooming );

	ghOrigDib = gpifToDIB( acName );

	if( !(ghOrigDib) )	return E_HANDLE;

	ghImgDib = gpifCopyDIB( ghOrigDib );

	ZeroMemory( &stBIH, sizeof(BITMAPINFOHEADER) );
	stBIH.biSize = sizeof(BITMAPINFOHEADER);
	gpifHeadDIB( ghImgDib, &stBIH );

	TRACE( TEXT("IMAGE [%dx%d]"), stBIH.biWidth, stBIH.biHeight );

	gstImgSize.cx = stBIH.biWidth;
	gstImgSize.cy = stBIH.biHeight;

	TraceRedrawIamge(  );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	文字用の、色選択ダイヤログーを開いてアッー！
	@param[in]	hDlg	ダイヤログハンドル
*/
HRESULT TraceMoziColourChoice( HWND hDlg )
{
	CHOOSECOLOR	stChsColour;
	COLORREF	adColours[16];

	TRACE( TEXT("色調整") );

	ZeroMemory( adColours, sizeof(adColours) );
	adColours[0] = gstTrcPrm.dMoziColour;

	ZeroMemory( &stChsColour, sizeof(CHOOSECOLOR) );

	stChsColour.lStructSize  = sizeof(CHOOSECOLOR);
	stChsColour.hwndOwner    = hDlg;
	stChsColour.rgbResult    = gstTrcPrm.dMoziColour;	//	ダイアログが最初に選択している初期カラーを指定します。ユーザーが色を選択してダイアログを閉じた後は、選択した色が格納されています。
	stChsColour.lpCustColors = adColours;	//	ダイアログが持っている基本色を格納する16個の COLORREF 配列へのポインタを指定します。ユーザーは、作成した色をダイアログのパレット領域に一時保存することができこの変数の配列には、そのダイアログの基本色が格納されます。
	stChsColour.Flags        = CC_RGBINIT;

	if( ChooseColor( &stChsColour ) )	//	色ダイヤログー出す
	{
		gstTrcPrm.dMoziColour = stChsColour.rgbResult;

		DeleteBrush( gMoziClrBrush );
		gMoziClrBrush = CreateSolidBrush( gstTrcPrm.dMoziColour );

		ViewCaretReColour( gstTrcPrm.dMoziColour );

		InvalidateRect( GetDlgItem(hDlg,IDPL_TRC_MOZICOLOUR), NULL, TRUE );
		ViewRedrawSetLine( -1 );
		return S_OK;
	}

	return E_ABORT;
}
//-------------------------------------------------------------------------------------------------

/*!
	各すらいだ～が操作された
	@param[in]	hDlg	ダイヤローグハンドル
	@param[in]	hWndCtl	スライダのハンドル
	@param[in]	code	スクロールコード
	@param[in]	pos		スクロールボックス（つまみ）の位置
	@return		無し
*/
INT_PTR TraceOnScroll( HWND hDlg, HWND hWndCtl, UINT code, INT pos )
{
/*
#define TB_LINEUP               0
#define TB_LINEDOWN             1
#define TB_PAGEUP               2	すらいだエリアクルック・POS来ない
#define TB_PAGEDOWN             3	すらいだエリアクルック・POS来ない
#define TB_THUMBPOSITION        4	ツマミ動かし終わったらクル
#define TB_THUMBTRACK           5	ツマミ動かしてるときにクル
#define TB_TOP                  6
#define TB_BOTTOM               7
#define TB_ENDTRACK             8	操作終わったらクル・POS来ない

#define SB_LINEUP           0
#define SB_LINEDOWN         1
#define SB_PAGEUP           2
#define SB_PAGEDOWN         3
#define SB_THUMBPOSITION    4
#define SB_THUMBTRACK       5
#define SB_TOP              6
#define SB_BOTTOM           7
#define SB_ENDSCROLL        8
*/
//	飛んでくるposはスライダ値そのままになっているように注意セヨ


	static BOOLEAN	bThumPos = FALSE;
	TCHAR	atBuffer[SUB_STRING];
	UINT	ctlID;
	INT		dDigi, dSyou;


	ctlID = GetDlgCtrlID( hWndCtl );

	ZeroMemory( atBuffer, sizeof(atBuffer) );

	//	これらのときはposないので自前で位置確認
	if( TB_PAGEDOWN == code || TB_PAGEUP ==code || TB_ENDTRACK == code || TB_LINEUP == code || TB_LINEDOWN == code )
	{
#ifdef TRC_SCROLLBAR
		pos = SendMessage( hWndCtl, SBM_GETPOS, 0, 0 );
#else
		pos = SendMessage( hWndCtl, TBM_GETPOS, 0, 0 );
#endif
	}

	TRACE( TEXT("SLIDER[%d][%d]"), code, pos );

	if( TB_THUMBPOSITION ==  code ){	bThumPos = TRUE;	}


	if( (TB_ENDTRACK == code && !(bThumPos)) || TB_THUMBPOSITION ==  code )
	{
		if( ghImgDib  ){	gpifDeleteDIB( ghImgDib );	}
		if( ghOrigDib ){	ghImgDib = gpifCopyDIB( ghOrigDib );	}
	}

	switch( code )
	{
		default:	break;
		case SB_LINEDOWN:	pos++;		break;
		case SB_LINEUP:		pos--;		break;
	}

	switch( ctlID )
	{
		case IDSL_TRC_HRIZ_POS:
#ifdef TRC_SCROLLBAR
			switch( code )
			{
				default:	break;
				case SB_PAGEDOWN:	pos += 11;	break;
				case SB_PAGEUP:		pos -= 11;	break;
			}
			if( 0 > pos )	pos = 0;
			if( TRC_POSITION_RANGE < pos )	pos = TRC_POSITION_RANGE;
#endif
			gstTrcPrm.stOffsetPt.x = pos;
			StringCchPrintf( atBuffer, SUB_STRING, TEXT("%d"), pos - TRC_POSITION_OFFSET );
			Edit_SetText( GetDlgItem(hDlg,IDE_TRC_HRIZ_VALUE), atBuffer );
#ifdef TRC_SCROLLBAR
			SendDlgItemMessage( hDlg, IDSL_TRC_HRIZ_POS, SBM_SETPOS, pos, TRUE );
#endif
			break;


		case IDSL_TRC_VART_POS:
#ifdef TRC_SCROLLBAR
			switch( code )
			{
				default:	break;
				case SB_PAGEDOWN:	pos += 18;	break;
				case SB_PAGEUP:		pos -= 18;	break;
			}
			if( 0 > pos )	pos = 0;
			if( TRC_POSITION_RANGE < pos )	pos = TRC_POSITION_RANGE;
#endif
			gstTrcPrm.stOffsetPt.y = pos;
			StringCchPrintf( atBuffer, SUB_STRING, TEXT("%d"), pos - TRC_POSITION_OFFSET );
			Edit_SetText( GetDlgItem(hDlg,IDE_TRC_VART_VALUE), atBuffer );
#ifdef TRC_SCROLLBAR
			SendDlgItemMessage( hDlg, IDSL_TRC_VART_POS, SBM_SETPOS, pos, TRUE );
#endif
			break;


		case IDSL_TRC_CONTRAST:
#ifdef TRC_SCROLLBAR
			switch( code )
			{
				default:	break;
				case SB_PAGEDOWN:	pos += 10;	break;
				case SB_PAGEUP:		pos -= 10;	break;
			}
			if( 0 > pos )	pos = 0;
			if( TRC_CONTRA_RANGE < pos )	pos = TRC_CONTRA_RANGE;
#endif
			gstTrcPrm.dContrast = pos;
			StringCchPrintf( atBuffer, SUB_STRING, TEXT("%d"), pos - TRC_CONTRA_OFFSET );
			Edit_SetText( GetDlgItem(hDlg,IDE_TRC_CONTRAST_VALUE), atBuffer );
#ifdef TRC_SCROLLBAR
			SendDlgItemMessage( hDlg, IDSL_TRC_CONTRAST, SBM_SETPOS, pos, TRUE );
#endif
			break;


		case IDSL_TRC_GAMMA:
#ifdef TRC_SCROLLBAR
			switch( code )
			{
				default:	break;
				case SB_PAGEDOWN:	pos += 100;	break;
				case SB_PAGEUP:		pos -= 100;	break;
			}
			if( 0 > pos )	pos = 0;
			if( TRC_GAMMA_RANGE <  pos )	pos = TRC_GAMMA_RANGE;
#endif
			gstTrcPrm.dGamma = pos;
			dDigi = pos / 1000;
			dSyou = pos % 1000;
			StringCchPrintf( atBuffer, SUB_STRING, TEXT("%d.%03d"), dDigi, dSyou );
			Edit_SetText( GetDlgItem(hDlg,IDE_TRC_GAMMA_VALUE), atBuffer );
#ifdef TRC_SCROLLBAR
			SendDlgItemMessage( hDlg, IDSL_TRC_GAMMA, SBM_SETPOS, pos, TRUE );
#endif
			break;


		case IDSL_TRC_GRAYMOPH:
#ifdef TRC_SCROLLBAR
			switch( code )
			{
				default:	break;
				case SB_PAGEDOWN:	pos += 10;	break;
				case SB_PAGEUP:		pos -= 10;	break;
			}
			if( 0 > pos )	pos = 0;
			if( TRC_GRAYMOPH_RANGE < pos )	pos = TRC_GRAYMOPH_RANGE;
#endif
			gstTrcPrm.dGrayMoph = pos;
			StringCchPrintf( atBuffer, SUB_STRING, TEXT("%d"), gstTrcPrm.dGrayMoph );
			Edit_SetText( GetDlgItem(hDlg,IDE_TRC_GRAYMOPH_VALUE), atBuffer );
#ifdef TRC_SCROLLBAR
			SendDlgItemMessage( hDlg, IDSL_TRC_GRAYMOPH, SBM_SETPOS, pos, TRUE );
#endif
			break;


		case IDSL_TRC_ZOOM:
#ifdef TRC_SCROLLBAR
			switch( code )
			{
				default:	break;
				case SB_PAGEDOWN:	pos += 5;	break;
				case SB_PAGEUP:		pos -= 5;	break;
			}
			if( 0 > pos )	pos = 0;
			if( TRC_ZOOM_RANGE < pos )	pos = TRC_ZOOM_RANGE;
#endif
			gstTrcPrm.dZooming = pos;
			StringCchPrintf( atBuffer, SUB_STRING, TEXT("%d ％"), pos + TRC_ZOOM_OFFSET );
			Edit_SetText( GetDlgItem(hDlg,IDE_TRC_ZOOM_VALUE), atBuffer );
#ifdef TRC_SCROLLBAR
			SendDlgItemMessage( hDlg, IDSL_TRC_ZOOM, SBM_SETPOS, pos, TRUE );
#endif
			break;


		case IDSL_TRC_TURN:	//	回転すると画像サイズ変わる
#ifdef TRC_SCROLLBAR
			switch( code )
			{
				default:	break;
				case SB_PAGEDOWN:	pos += 5;	break;
				case SB_PAGEUP:		pos -= 5;	break;
			}
			if( 0 > pos )	pos = 0;
			if( TRC_TURN_RANGE < pos )	pos = TRC_TURN_RANGE;
#endif
			gstTrcPrm.dTurning = pos;
			StringCchPrintf( atBuffer, SUB_STRING, TEXT("%d"), gstTrcPrm.dTurning );
			Edit_SetText( GetDlgItem(hDlg,IDE_TRC_TURN_VALUE), atBuffer );
#ifdef TRC_SCROLLBAR
			SendDlgItemMessage( hDlg, IDSL_TRC_TURN, SBM_SETPOS, pos, TRUE );
#endif
			break;

		default:	return (INT_PTR)FALSE;
	}

	if( (TB_ENDTRACK == code || TB_THUMBPOSITION == code) &&  ghImgDib )	//	適用順を考慮できるか？
	{
		if( bThumPos && TB_ENDTRACK == code ){	bThumPos = FALSE;	}
		else{	TraceRedrawIamge(  );	}
	}

	//	20110728	フォーカスを描画に戻す
	if( TB_ENDTRACK == code ){	ViewFocusSet(  );	}

	return (INT_PTR)TRUE;
}
//-------------------------------------------------------------------------------------------------

/*!
	イメージを再描画する
*/
HRESULT TraceRedrawIamge( VOID )
{
	SHORT	dBuff;

	BITMAPINFOHEADER	stBIH;

	dBuff = gstTrcPrm.dContrast - TRC_CONTRA_OFFSET;
	gpifContrastDIB( ghImgDib, dBuff, dBuff, dBuff );
	gpifGammaDIB( ghImgDib, gstTrcPrm.dGamma, gstTrcPrm.dGamma, gstTrcPrm.dGamma );
	gpifGrayDIB( ghImgDib, gstTrcPrm.dGrayMoph );

	gpifTurnDIBex( ghImgDib, gstTrcPrm.dTurning * 1000, 0x00EEEEEE );

	ZeroMemory( &stBIH, sizeof(BITMAPINFOHEADER) );
	stBIH.biSize = sizeof(BITMAPINFOHEADER);
	gpifHeadDIB( ghImgDib, &stBIH );

	gstImgSize.cx = stBIH.biWidth;
	gstImgSize.cy = stBIH.biHeight;

	ViewRedrawSetLine( -1 );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	画像の表示非表示のON/OFFする
*/
HRESULT TraceImgViewTglExt( VOID )
{
	if( !(ghTraceDlg) ){	return E_HANDLE;	}

	gbOnView = gbOnView ? FALSE : TRUE;
	SetDlgItemText( ghTraceDlg, IDB_TRC_VIEWTOGGLE, gbOnView ? TEXT("非表示") : TEXT("表示") );
	ViewRedrawSetLine( -1 );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	文字描画色をお知らせする
	@param[in]	pColour	選択されている色を入れる
	@retval	非０	トレス中
	@retval	０		トレスモードになっていない
*/
UINT TraceMoziColourGet( LPCOLORREF pColour )
{
	if( !(ghTraceDlg) || !(pColour) ){	return FALSE;	}

	if( !(gbOnView) ){	return FALSE;	}

	*pColour = gstTrcPrm.dMoziColour;

	return TRUE;
}
//-------------------------------------------------------------------------------------------------

/*!
	イメージ開いてて表示有効な・ビュー描画で呼ばれる
	@param[in]	hdc		描画するデバイスコンテキスト
	@param[in]	iScrlX	描画領域のＸスクロールDot数
	@param[in]	iScrlY	描画領域のＹスクロールDot数
	@retval	非０		画像イメージあった
	@retval	０			画像は開かれていない
*/
UINT TraceImageAppear( HDC hdc, INT iScrlX, INT iScrlY )
{
	POINT	stBegin;
	SIZE	stStretch, stReverse;

	if( !(ghTraceDlg && ghImgDib) ){	return FALSE;	}

	if( !(gbOnView) ){	return FALSE;	}

	//	左上のオフセットを考慮して描画セヨ

	SetStretchBltMode( hdc, COLORONCOLOR );

	//	位置合わせに使う
	stBegin.x = LINENUM_WID + (gstTrcPrm.stOffsetPt.x - TRC_POSITION_OFFSET);
	stBegin.y = RULER_AREA  + (gstTrcPrm.stOffsetPt.y - TRC_POSITION_OFFSET);
	//	スクロール分の考慮
	stBegin.x -= iScrlX;
	stBegin.y -= iScrlY;

	//	拡大縮小に使う
	stStretch = gstImgSize;
	stStretch.cx *= (gstTrcPrm.dZooming + TRC_ZOOM_OFFSET);	stStretch.cx /= 100;
	stStretch.cy *= (gstTrcPrm.dZooming + TRC_ZOOM_OFFSET);	stStretch.cy /= 100;

	//	反転に使う
	stReverse = gstImgSize;
	if( BST_CHECKED == gstTrcPrm.bUpset )	stReverse.cy *= -1;
	if( BST_CHECKED == gstTrcPrm.bMirror )	stReverse.cx *= -1;

	gpifDIBtoDCex( hdc, stBegin.x, stBegin.y, stStretch.cx, stStretch.cy, 
		ghImgDib, 0, 0, stReverse.cx, stReverse.cy, SRCCOPY );

	return TRUE;
}
//-------------------------------------------------------------------------------------------------





/*!
	デバイスコンテキストの内容を、各形式で保存する
	@param[in]	hDC		保存したい内容がはいってるディバイスコンティキスト
	@param[in]	ptName	保存したいフルパス・拡張子は指定に合わせて修正しちゃう・MAX_PATHであること
	@param[in]	iType	ファイルタイプ指定　１：ＢＭＰ　２：ＪＰＧ　３：ＰＮＧ
	@return		HRESULT	終了状態コード
*/
HRESULT ImageFileSaveDC( HDC hDC, LPTSTR ptName, INT iType )
{
	HDIB	hDIB;
	BOOL	bRslt;
	UINT_PTR	cchSize;

	CHAR	acOutName[MAX_PATH];


	ZeroMemory( acOutName, sizeof(acOutName) );

	hDIB = gpifDCtoDIB( hDC, 0, 0, 0, 0 );	//	これで画面全体

	PathRemoveExtension( ptName );	//	拡張子あぼ～ん

	switch( iType )
	{
		case  ISAVE_BMP:	//	BMP保存は、２値とか２５６色でおｋ
			PathAddExtension( ptName, TEXT(".bmp") );
			StringCchLength( ptName, MAX_PATH, &cchSize );
			WideCharToMultiByte( CP_ACP, 0, ptName, cchSize, acOutName, MAX_PATH, NULL, NULL );
			bRslt = gpifDIBtoBMP( acOutName, hDIB );
			break;

		//case ISAVE_JPEG:
		//	PathAddExtension( ptName, TEXT(".jpg") );
		//	break;

		case  ISAVE_PNG:
			PathAddExtension( ptName, TEXT(".png") );
			StringCchLength( ptName, MAX_PATH, &cchSize );
			WideCharToMultiByte( CP_ACP, 0, ptName, cchSize, acOutName, MAX_PATH, NULL, NULL );
			bRslt = gpifDIBtoPNG( acOutName, hDIB, TRUE );
			break;

		default:	bRslt = FALSE;	break;
	}

	gpifDeleteDIB( hDIB );

	return bRslt ? S_OK : E_DRAW;
}
//-------------------------------------------------------------------------------------------------



