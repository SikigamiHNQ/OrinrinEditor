/*! @file
	@brief ユニコード一覧のダイヤログの面倒見ます
	このファイルは UnicodePalette.cpp です。
	@author	SikigamiHNQ
	@date	2011/05/20
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

#include "UnicodePalette.h"
//-------------------------------------------------------------------------------------------------

//	TODO:	使ったヤツの履歴ほしいか




#ifdef UNICODE
#define LPNMLVDISPINFO	LPNMLVDISPINFOW
#else
#define LPNMLVDISPINFO	LPNMLVDISPINFOA
#endif
//-------------------------------------------------------------------------------------------------


static LOGFONT	gstUniFont = {
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

static  HWND	ghMainWnd;		//!<	
static  HWND	ghUniPltWnd;	//!<	
static  HWND	ghGroupSelWnd;	//!<	
static  HWND	ghUniLvWnd;		//!<	

static WNDPROC	gpfOrgUniListProc;	//!<	

static HFONT	ghLvFont, ghPanelFont;	//!<	

static TCHAR	gtSelMozi;	//!<	

static UINT		gSelRow;	//!<	
static INT		gSelClm;	//!<	
//-------------------------------------------------------------------------------------------------

INT_PTR	CALLBACK UniPaletteDlgProc( HWND, UINT, WPARAM, LPARAM );

INT_PTR	Uni_OnInitDialog( HWND, HWND, LPARAM );
INT_PTR	Uni_OnCommand( HWND, INT, HWND, UINT );
INT_PTR	Uni_OnClose( HWND );
INT_PTR	Uni_OnNotify( HWND, INT, LPNMHDR );

LRESULT	CALLBACK gpfUniListProc( HWND, UINT, WPARAM, LPARAM );	//	
//-------------------------------------------------------------------------------------------------

/*!
	ユニコードパレットダイヤログーを開く処理
	@param[in]	hInst	インスタンスハンドル
	@param[in]	hWnd	親ウインドウハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT UniDialogueEntry( HINSTANCE hInst, HWND hWnd )
{
	ghMainWnd = hWnd;

	gSelRow   = 0xFFFFFFFF;
	gSelClm   = -1;

	if( ghUniPltWnd ){	PostMessage( ghUniPltWnd, WM_CLOSE, 0 , 0 );	return S_OK;	}

	gtSelMozi = NULL;
	ghUniPltWnd = CreateDialogParam( hInst, MAKEINTRESOURCE(IDD_UNICODE_PALETTE_DLG), hWnd, UniPaletteDlgProc, 0 );
	//	ダイヤログーはモーダレスでありんす

	if( ghUniPltWnd )
	{
		MenuItemCheckOnOff( IDM_UNI_PALETTE, TRUE );
		ShowWindow( ghUniPltWnd, SW_SHOW );
		//UpdateWindow( ghUniPltWnd );
	}

	return ghUniPltWnd ? S_OK : E_HANDLE;
}
//-------------------------------------------------------------------------------------------------

/*!
	ユニコードパレットダイヤログーのウインドウプロシージャ
	@param[in]	hDlg	ダイヤログハンドル
	@param[in]	message	ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@retval 0	メッセージに対して何もしなかった
	@retval no0	なんか処理した
*/
INT_PTR CALLBACK UniPaletteDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		default:	break;

		case WM_INITDIALOG:	return Uni_OnInitDialog( hDlg, (HWND)(wParam), lParam );
		case WM_COMMAND:	return Uni_OnCommand( hDlg, (INT)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam) );
		case WM_CLOSE:		return Uni_OnClose( hDlg  );
		case WM_NOTIFY:		return Uni_OnNotify( hDlg, (INT)(wParam), (LPNMHDR)(lParam) );
	}

	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	ダイヤログ起動時の初期化
	@param[in]	hDlg		ダイヤログハンドル
	@param[in]	hWndFocus	なんだっけ
	@param[in]	lParam		ダイヤログオーポンするときに呼び出し側が渡した値
	@return		FALSEを返すと、初期フォーカスコントロールの変更が有効になる
*/
INT_PTR Uni_OnInitDialog( HWND hDlg, HWND hWndFocus, LPARAM lParam )
{
	INT		iTopIdx;
	UINT	i;
	LVCOLUMN	stLvColm;
	RECT		rect;


	ghGroupSelWnd = GetDlgItem( hDlg, IDCB_UNI_GROUPSEL );

	for( i = 0; UNIGROUP_MAX > i; i++ )
	{
		ComboBox_AddString( ghGroupSelWnd, gstUniGroupName[i].ptNameStr );
	}
	ComboBox_SetCurSel( ghGroupSelWnd, 0 );

//	ghPanelFont = CreateFont( 48, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, VARIABLE_PITCH, TEXT("ＭＳ Ｐゴシック") );
	gstUniFont.lfHeight = 48;
	ghPanelFont = CreateFontIndirect( &gstUniFont );
	SetWindowFont( GetDlgItem(hDlg,IDS_UNI_LETTER_IMG), ghPanelFont, TRUE );

//	ghLvFont = CreateFont( 28, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, VARIABLE_PITCH, TEXT("ＭＳ Ｐゴシック") );
	gstUniFont.lfHeight = 28;
	ghLvFont = CreateFontIndirect( &gstUniFont );

	ghUniLvWnd = GetDlgItem( hDlg, IDLV_UNI_CATALOGUE );
	ListView_SetExtendedListViewStyle( ghUniLvWnd, LVS_EX_GRIDLINES | LVS_EX_LABELTIP );
	SetWindowFont( ghUniLvWnd, ghLvFont, TRUE );

	//	サブクラス化	
	gpfOrgUniListProc = SubclassWindow( ghUniLvWnd, gpfUniListProc );

	ZeroMemory( &stLvColm, sizeof(LVCOLUMN) );
	stLvColm.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	stLvColm.fmt = LVCFMT_CENTER;
	stLvColm.pszText = TEXT("Z");	stLvColm.cx =  0;	stLvColm.iSubItem = 0x00;	ListView_InsertColumn( ghUniLvWnd, 0x00, &stLvColm );
	stLvColm.pszText = TEXT("0");	stLvColm.cx = 40;	stLvColm.iSubItem = 0x01;	ListView_InsertColumn( ghUniLvWnd, 0x01, &stLvColm );
	stLvColm.pszText = TEXT("1");	stLvColm.cx = 40;	stLvColm.iSubItem = 0x02;	ListView_InsertColumn( ghUniLvWnd, 0x02, &stLvColm );
	stLvColm.pszText = TEXT("2");	stLvColm.cx = 40;	stLvColm.iSubItem = 0x03;	ListView_InsertColumn( ghUniLvWnd, 0x03, &stLvColm );
	stLvColm.pszText = TEXT("3");	stLvColm.cx = 40;	stLvColm.iSubItem = 0x04;	ListView_InsertColumn( ghUniLvWnd, 0x04, &stLvColm );
	stLvColm.pszText = TEXT("4");	stLvColm.cx = 40;	stLvColm.iSubItem = 0x05;	ListView_InsertColumn( ghUniLvWnd, 0x05, &stLvColm );
	stLvColm.pszText = TEXT("5");	stLvColm.cx = 40;	stLvColm.iSubItem = 0x06;	ListView_InsertColumn( ghUniLvWnd, 0x06, &stLvColm );
	stLvColm.pszText = TEXT("6");	stLvColm.cx = 40;	stLvColm.iSubItem = 0x07;	ListView_InsertColumn( ghUniLvWnd, 0x07, &stLvColm );
	stLvColm.pszText = TEXT("7");	stLvColm.cx = 40;	stLvColm.iSubItem = 0x08;	ListView_InsertColumn( ghUniLvWnd, 0x08, &stLvColm );
	stLvColm.pszText = TEXT("8");	stLvColm.cx = 40;	stLvColm.iSubItem = 0x09;	ListView_InsertColumn( ghUniLvWnd, 0x09, &stLvColm );
	stLvColm.pszText = TEXT("9");	stLvColm.cx = 40;	stLvColm.iSubItem = 0x0A;	ListView_InsertColumn( ghUniLvWnd, 0x0A, &stLvColm );
	stLvColm.pszText = TEXT("A");	stLvColm.cx = 40;	stLvColm.iSubItem = 0x0B;	ListView_InsertColumn( ghUniLvWnd, 0x0B, &stLvColm );
	stLvColm.pszText = TEXT("B");	stLvColm.cx = 40;	stLvColm.iSubItem = 0x0C;	ListView_InsertColumn( ghUniLvWnd, 0x0C, &stLvColm );
	stLvColm.pszText = TEXT("C");	stLvColm.cx = 40;	stLvColm.iSubItem = 0x0D;	ListView_InsertColumn( ghUniLvWnd, 0x0D, &stLvColm );
	stLvColm.pszText = TEXT("D");	stLvColm.cx = 40;	stLvColm.iSubItem = 0x0E;	ListView_InsertColumn( ghUniLvWnd, 0x0E, &stLvColm );
	stLvColm.pszText = TEXT("E");	stLvColm.cx = 40;	stLvColm.iSubItem = 0x0F;	ListView_InsertColumn( ghUniLvWnd, 0x0F, &stLvColm );
	stLvColm.pszText = TEXT("F");	stLvColm.cx = 40;	stLvColm.iSubItem = 0x10;	ListView_InsertColumn( ghUniLvWnd, 0x10, &stLvColm );

	ListView_SetItemCountEx( ghUniLvWnd, 4096, LVSICF_NOINVALIDATEALL );

	iTopIdx = InitParamValue( INIT_LOAD, VL_UNILISTLAST, 0 );
	ListView_GetItemRect( ghUniLvWnd, 0, &rect, LVIR_BOUNDS );
	iTopIdx *= rect.bottom;
	ListView_Scroll( ghUniLvWnd, 0, iTopIdx );

	SetFocus( ghUniLvWnd );

	return (INT_PTR)FALSE;
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
INT_PTR Uni_OnCommand( HWND hDlg, INT id, HWND hWndCtl, UINT codeNotify )
{
	INT	dSel, tIdx, dPos;
	RECT	rect;

	switch( id )
	{
		default:	break;

		case IDCB_UNI_GROUPSEL:
			if( CBN_SELCHANGE == codeNotify )	//	コンボックスで選択がされた直後
			{
				tIdx = ListView_GetTopIndex( ghUniLvWnd );
				ListView_GetItemRect( ghUniLvWnd, tIdx, &rect, LVIR_BOUNDS );
				TRACE( TEXT("%d %d %d"), tIdx, rect.top, rect.bottom );

				dSel = ComboBox_GetCurSel( hWndCtl );
				dPos = gstUniGroupName[dSel].dCode;
				TRACE( TEXT("%s[%u]"), gstUniGroupName[dSel].ptNameStr, dPos );
				dPos /= 16;	//	壱行あたり１６文字なので
				dPos -= tIdx;
				dPos *= rect.bottom;
				TRACE( TEXT("%d"), dPos );
				ListView_Scroll( ghUniLvWnd, 0, dPos );
				SetFocus( ghUniLvWnd );
			}
			return (INT_PTR)TRUE;


		case IDB_UNI_DECIDE:
			if( gtSelMozi ){	Evw_OnChar( ghMainWnd, gtSelMozi, 0 );	}
			return (INT_PTR)TRUE;

		case IDB_UNI_COPY_CLIP:	
			if( gtSelMozi ){	DocClipLetter( gtSelMozi ) ;	}
			return (INT_PTR)TRUE;

	//	case IDOK:
		case IDCANCEL:
			return (INT_PTR)TRUE;
	}

	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	ウインドウ閉じるとき
	@param[in]	hDlg		ダイヤローグハンドル
	@return		処理したかせんかったか
*/
INT_PTR Uni_OnClose( HWND hDlg )
{
	INT	iTopIdx;

	iTopIdx = ListView_GetTopIndex( ghUniLvWnd );
	InitParamValue( INIT_SAVE, VL_UNILISTLAST, iTopIdx );

	DeleteFont( ghLvFont  );
	DeleteFont( ghPanelFont );
	DestroyWindow( hDlg );

	ghUniPltWnd = NULL;

	MenuItemCheckOnOff( IDM_UNI_PALETTE, FALSE );

	return (INT_PTR)TRUE;
}
//-------------------------------------------------------------------------------------------------

/*!
	ノーティファイメッセージの処理
	@param[in]	hDlg		ダイヤローグハンドル
	@param[in]	idFrom		NOTIFYを発生させたコントロールのＩＤ
	@param[in]	pstNmhdr	NOTIFYの詳細
	@return		処理したかせんかったか
*/
INT_PTR Uni_OnNotify( HWND hDlg, INT idFrom, LPNMHDR pstNmhdr )
{
	INT		iItem, iSubItem, i;//, tIdx;
	DWORD	iNumber;
	TCHAR	ch[2], atBuff[MIN_STRING];
	RECT	rect;

	LVHITTESTINFO		stHitTestInfo;

	LPNMLISTVIEW		pstListView;
	LPNMLVDISPINFO		pstDispInfo;
	LPNMLVCUSTOMDRAW	pstCustomDraw;

	if( LVN_GETDISPINFO == pstNmhdr->code )	//	仮想リストビューの処理
	{
		pstDispInfo = (LPNMLVDISPINFO)pstNmhdr;

		iItem = pstDispInfo->item.iItem;

		if( pstDispInfo->item.mask & LVIF_TEXT )
		{
			iSubItem = pstDispInfo->item.iSubItem;

			if( 0 != iSubItem )
			{
				iNumber = iItem * 16 + (iSubItem - 1);
				ch[0] = (TCHAR)iNumber;
				ch[1] = 0;

				StringCchPrintf( pstDispInfo->item.pszText, pstDispInfo->item.cchTextMax, TEXT("%c"), ch[0] );
			}
			else
			{
				ListView_GetItemRect( ghUniLvWnd, iItem, &rect, LVIR_BOUNDS );

			//	TRACE( TEXT("DISP[%d][%d]"), tIdx, iItem );

				StringCchCopy( pstDispInfo->item.pszText, pstDispInfo->item.cchTextMax, TEXT("") );
			}
		}

		return (INT_PTR)TRUE;
	}

	pstListView = (LPNMLISTVIEW)pstNmhdr;

	if( NM_CLICK == pstListView->hdr.code )	//	クリッケした奴の処理
	{
		//	FULLROWSELECTしてないと取得出来ないので、このようにしてゲットする
		stHitTestInfo.pt = pstListView->ptAction;
		ListView_SubItemHitTest( ghUniLvWnd, &stHitTestInfo );

		gSelRow = stHitTestInfo.iItem;
		gSelClm = stHitTestInfo.iSubItem;

		iNumber = gSelRow * 16 + (gSelClm - 1);
		ch[0] = (TCHAR)iNumber;
		ch[1] = 0;

		gtSelMozi = (TCHAR)iNumber;

		Static_SetText( GetDlgItem(hDlg,IDS_UNI_LETTER_IMG), ch );
		StringCchPrintf( atBuff, MIN_STRING, TEXT("0x%04X"), iNumber );
		Edit_SetText( GetDlgItem(hDlg,IDE_UNI_HEX_CODE), atBuff );
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), iNumber );
		Edit_SetText( GetDlgItem(hDlg,IDE_UNI_DEC_CODE), atBuff );
		Edit_SetText( GetDlgItem(hDlg,IDE_UNI_LETTER_NAME), TEXT("") );
		for( i = 0; UNINAME_MAX > i; i++ )
		{
			if( iNumber == gstUniMoziName[i].dCode )
			{
				Edit_SetText( GetDlgItem(hDlg,IDE_UNI_LETTER_NAME), gstUniMoziName[i].ptNameStr );
				break;
			}

			//	Sort済なのでこれでおｋ
			if( iNumber <  gstUniMoziName[i].dCode )	break;
		}

		InvalidateRect( ghUniLvWnd, NULL, TRUE );
		//UpdateWindow( ghUniLvWnd );
		return (INT_PTR)TRUE;
	}

	if( NM_DBLCLK == pstListView->hdr.code )	//	ダボルクリッケした奴の処理
	{
		//	確定操作として扱う
		if( gtSelMozi ){	Evw_OnChar( ghMainWnd, gtSelMozi, 0 );	}
	}

	//	選択壱弐色をつけるのはカスタムドロー
	if( NM_CUSTOMDRAW == pstListView->hdr.code )
	{
		pstCustomDraw = (LPNMLVCUSTOMDRAW)pstListView;


		if( CDDS_PREPAINT == pstCustomDraw->nmcd.dwDrawStage || 
			CDDS_ITEMPREPAINT == pstCustomDraw->nmcd.dwDrawStage )
		{
			SetWindowLong( hDlg, DWL_MSGRESULT, (long)CDRF_NOTIFYSUBITEMDRAW );
			return TRUE;
		}

		if( (CDDS_ITEMPREPAINT|CDDS_SUBITEM) == pstCustomDraw->nmcd.dwDrawStage )
		{
			//if( 0 == pstCustomDraw->iSubItem )
			//{
			//	TRACE( TEXT("CSTM [%d %d]"), pstCustomDraw->nmcd.rc.top, pstCustomDraw->nmcd.dwItemSpec );
			//}

			if( (gSelClm == pstCustomDraw->iSubItem) && (gSelRow == pstCustomDraw->nmcd.dwItemSpec) )
			{
				pstCustomDraw->clrTextBk = GetSysColor( COLOR_HIGHLIGHT );
				pstCustomDraw->clrText   = GetSysColor( COLOR_HIGHLIGHTTEXT );
				SetWindowLong( hDlg, DWL_MSGRESULT, (long)CDRF_NEWFONT);
			}
			else
			{
				pstCustomDraw->clrTextBk = GetSysColor( COLOR_WINDOW );
				pstCustomDraw->clrText   = GetSysColor( COLOR_WINDOWTEXT );
				SetWindowLong( hDlg, DWL_MSGRESULT, (long)CDRF_NEWFONT);
			}
			return TRUE;
		}

		return (INT_PTR)TRUE;	//	CDRF_DODEFAULT
	}
/*
リストビューで、指定行の背景色を変更するには、カスタムドローを行います。
カスタムドローは、ダブルクリックの取得で説明した時と同じで、
WM_NOTIFYメッセージ内でNM_CUSTOMDRAWを判別します。
更に、リストビュー用のカスタムドロー構造体で、ステージ毎の処理を行います。
・CDDS_PREPAINT    ・・・描画前通知
・CDDS_ITEMPREPAINT・・・項目毎の描画前通知
使用しないステージの場合は、CDRF_DODEFAULTを戻り値としてリターンします。
これは、コントロールに描画を任すをことを意味します。
このステージを処理した後は、処理したことを通知する必要あるのですが、
ここの例で使用しているのはダイアログである為、通知を返せません。
ですので、SetWindowLong関数を使用して返します。

以下に、NMLVCUSTOMDRAW構造体の説明を書きます。

＜NMLVCUSTOMDRAW構造体＞
・NMHDR  hdr         NMHDR構造体を表します。
・DWORD  dwDrawStage 現在の描画ステージを表します。
・HDC    hdc         コントロールのデバイスコンテキストハンドルを表します。
・RECT   rc          描画されようとしている領域のRECT構造体を表します。
・DWORD  dwItemSpec  アイテム番号を表します。
・UINT   uItemState  現在のアイテムの状態を表します。
・LPARAM lItemlParam アプリケーション定義のデータを表します。

HWND             hList;
LPNMHDR          lpnmhdr;
LPNMLVCUSTOMDRAW lplvcd;

case WM_NOTIFY:
	lpnmhdr = (LPNMHDR)lp;
	if( lpnmhdr->hwndFrom == GetDlgItem(hWnd, リストビューのID) ){
		switch( lpnmhdr->code ){
			case NM_CUSTOMDRAW:
				lplvcd = (LPNMLVCUSTOMDRAW)lp;

				if( lplvcd->nmcd.dwDrawStage == CDDS_PREPAINT ){
					SetWindowLong( hWnd, DWL_MSGRESULT, (long)CDRF_NOTIFYITEMDRAW );
				return( TRUE );
		}
		if( lplvcd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT ){
			lplvcd->clrTextBk = RGB(0xCC, 0xCC, 0xCC);
			SetWindowLong( hWnd, DWL_MSGRESULT, (long)CDRF_NEWFONT);
		return( TRUE );
	}
return( CDRF_DODEFAULT );
*/

	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	ユニコード一覧のサブクラスプロシージャ
	@param[in]	hWnd	リストのハンドル
	@param[in]	msg		ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@return		LRESULT	処理結果とか
*/
LRESULT CALLBACK gpfUniListProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	INT	tIdx;
	TCHAR	atBuffer[MIN_STRING];

	switch( msg )
	{
		case WM_PAINT:	//	リストの書換が発生すなわち表示内容が変更されたと思われ
			tIdx = ListView_GetTopIndex( hWnd );
		//	TRACE( TEXT("UPSC_WM_PAINT [%d]"), tIdx );
			StringCchPrintf( atBuffer, MIN_STRING, TEXT("0x%04X"),  tIdx    * 0x10 );	SetDlgItemText( ghUniPltWnd, IDS_UA_ALPHA,   atBuffer );
			StringCchPrintf( atBuffer, MIN_STRING, TEXT("0x%04X"), (tIdx+1) * 0x10 );	SetDlgItemText( ghUniPltWnd, IDS_UA_BRAVO,   atBuffer );
			StringCchPrintf( atBuffer, MIN_STRING, TEXT("0x%04X"), (tIdx+2) * 0x10 );	SetDlgItemText( ghUniPltWnd, IDS_UA_CHARLIE, atBuffer );
			StringCchPrintf( atBuffer, MIN_STRING, TEXT("0x%04X"), (tIdx+3) * 0x10 );	SetDlgItemText( ghUniPltWnd, IDS_UA_DELTA,   atBuffer );
			StringCchPrintf( atBuffer, MIN_STRING, TEXT("0x%04X"), (tIdx+4) * 0x10 );	SetDlgItemText( ghUniPltWnd, IDS_UA_ECHO,    atBuffer );
			StringCchPrintf( atBuffer, MIN_STRING, TEXT("0x%04X"), (tIdx+5) * 0x10 );	SetDlgItemText( ghUniPltWnd, IDS_UA_FOXTROT, atBuffer );
			StringCchPrintf( atBuffer, MIN_STRING, TEXT("0x%04X"), (tIdx+6) * 0x10 );	SetDlgItemText( ghUniPltWnd, IDS_UA_GOLF,    atBuffer );
			StringCchPrintf( atBuffer, MIN_STRING, TEXT("0x%04X"), (tIdx+7) * 0x10 );	SetDlgItemText( ghUniPltWnd, IDS_UA_HOTEL,   atBuffer );
			break;

		default:	break;
	}

	return CallWindowProc( gpfOrgUniListProc, hWnd, msg, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------

