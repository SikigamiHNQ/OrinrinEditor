/*! @file
	@brief ツリービューとお気に入り、タブの制御をします
	このファイルは MaaTreeView.cpp です。
	@author	SikigamiHNQ
	@date	2011/06/22
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

//	タブにお気に入りを登録するには

#include "stdafx.h"
#include "OrinrinEditor.h"
#include "MaaTemplate.h"
//-------------------------------------------------------------------------------------------------

//	複数ファイル保持のアレ
typedef struct tagMULTIPLEMAA
{
	INT		dTabNum;				//!<	タブの番号・２インデックス
	TCHAR	atFilePath[MAX_PATH];	//!<	ファイルパス・空なら使用から開いた
	TCHAR	atBase[MAX_PATH];		//!<	使用リストに入れる時のグループ名

	UINT	dLastTop;				//!<	見てたAAの番号

} MULTIPLEMAA, *LPMULTIPLEMAA;

//-------------------------------------------------------------------------------------------------

extern  HWND		ghSplitaWnd;	//	スプリットバーハンドル

extern HMENU		ghProfHisMenu;	//	履歴表示する部分・動的に内容作成せないかん

static HFONT		ghTabFont;

static  HWND		ghTabWnd;		//!<	選択タブのハンドル

static  HWND		ghFavLtWnd;		//!<	よく使う奴を登録するリストボックス

static  HWND		ghTreeWnd;		//!<	ツリーのハンドル
static HTREEITEM	ghTreeRoot;		//!<	ツリーのルートアイテム

static HIMAGELIST	ghImageList, ghOldIL;	//!<	ツリービューにくっつけるイメージリスト

static TCHAR		gatAARoot[MAX_PATH];	//!<	ＡＡディレクトリのカレント
static TCHAR		gatBaseName[MAX_PATH];	//!<	使用リストに入れる時のグループ名

static INT			gixUseTab;	//!<	今開いてるの・０ツリー　１お気に入り　２～複数ファイル
//	タブ番号であることに注意・複数ファイルリストの割当番号ではない

static WNDPROC	gpfOriginFavListProc;	
static WNDPROC	gpfOriginTreeViewProc;	
static WNDPROC	gpfOriginTabMultiProc;	


static list<MULTIPLEMAA>	gltMultiFiles;	//!<	複数ファイルの保持
typedef  list<MULTIPLEMAA>::iterator	MLTT_ITR;
//-------------------------------------------------------------------------------------------------

#ifdef MAA_VIRTUAL_TREE
HRESULT	TreeItemFromSqlII( HTREEITEM );
#else
UINT	TreeItemFromSql( LPCTSTR, HTREEITEM, UINT );
#endif

VOID	Mtv_OnMButtonUp( HWND, INT, INT, UINT );

HRESULT	TabMultipleRestore( HWND );
INT		TabMultipleSelect( HWND, INT, UINT );	//!<	
//INT	TabMultipleOpen( HWND , HTREEITEM );	//!<	
HRESULT	TabMultipleAppend( HWND );				//!<	
HRESULT	TabMultipleDelete( HWND, CONST INT );	//!<	

UINT	TabMultipleIsFavTab( INT, LPTSTR, UINT_PTR );

LRESULT	CALLBACK gpfFavListProc( HWND, UINT, WPARAM, LPARAM );	//	
LRESULT	CALLBACK gpfTreeViewProc( HWND, UINT, WPARAM, LPARAM );	//	
LRESULT	CALLBACK gpfTabMultiProc( HWND, UINT, WPARAM, LPARAM );	//	
//-------------------------------------------------------------------------------------------------

/*!
	ツリービューとかを作る
	@param[in]	hWnd	親ウインドウハンドル・NULLで破壊処理
	@param[in]	hInst	アプリの実存
	@param[in]	ptRect	メインウインドウの位置と大きさ
	@return		HRESULT	終了状態コード
*/
HRESULT TreeInitialise( HWND hWnd, HINSTANCE hInst, LPRECT ptRect )
{
	TCITEM		stTcItem;
	RECT		itRect, clRect;

	SHFILEINFO	stShFileInfo;

	//	破壊するとき
	if( !(hWnd) )
	{
		SetWindowFont( ghTabWnd, GetStockFont(DEFAULT_GUI_FONT), FALSE );
		DeleteFont( ghTabFont );

		//	開いてる副タブをINIに保存
		TabMultipleStore( hWnd );

		return S_OK;
	}

	//ghImageList = ImageList_Create( 16, 16, (ILC_COLOR4|ILC_MASK), 2, 1 );
	//ImageList_AddIcon( ghImageList, LoadIcon( NULL, IDI_QUESTION ) );
	//ImageList_AddIcon( ghImageList, LoadIcon( NULL, IDI_ASTERISK ) );

	ghTabFont = CreateFont( 14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, VARIABLE_PITCH, TEXT("MS UI Gothic") );


	gixUseTab = ACT_ALLTREE;

	ZeroMemory( gatAARoot, sizeof(gatAARoot) );

	GetClientRect( hWnd, &clRect );

//表示選択タブ
	ghTabWnd = CreateWindowEx( 0, WC_TABCONTROL, TEXT("treetab"),
		WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | TCS_RIGHTJUSTIFY | TCS_MULTILINE,
		0, 0, TREE_WIDTH, 0, hWnd, (HMENU)IDTB_TREESEL, hInst, NULL );	//	TCS_SINGLELINE
	SetWindowFont( ghTabWnd, ghTabFont, FALSE );

	ZeroMemory( &stTcItem, sizeof(stTcItem) );
	stTcItem.mask = TCIF_TEXT;
	stTcItem.pszText = TEXT("全て");	TabCtrl_InsertItem( ghTabWnd, 0, &stTcItem );
	stTcItem.pszText = TEXT("使用");	TabCtrl_InsertItem( ghTabWnd, 1, &stTcItem );

	//	選ばれしファイルをタブ的に追加？　タブ幅はウインドウ幅

	TabCtrl_GetItemRect( ghTabWnd, 1, &itRect );
	itRect.bottom -= itRect.top;

	itRect.right -= itRect.left;
	itRect.top = 0;
	itRect.left = 0;
	TabCtrl_AdjustRect( ghTabWnd, 0, &itRect );

	MoveWindow( ghTabWnd, 0, 0, clRect.right, itRect.top, TRUE );

	//	サブクラス化
	gpfOriginTabMultiProc =SubclassWindow( ghTabWnd, gpfTabMultiProc );

//お気に入り用リストボックス
	ghFavLtWnd = CreateWindowEx( WS_EX_CLIENTEDGE, WC_LISTBOX, TEXT("favlist"),
		WS_CHILD | WS_VSCROLL | LBS_NOTIFY | LBS_SORT | LBS_NOINTEGRALHEIGHT,
		0, itRect.bottom, TREE_WIDTH, ptRect->bottom-itRect.bottom-1, hWnd, (HMENU)IDLB_FAVLIST, hInst, NULL );

	//	サブクラス化
	gpfOriginFavListProc = SubclassWindow( ghFavLtWnd, gpfFavListProc );


//全ＡＡリストツリー
	ghImageList = (HIMAGELIST)SHGetFileInfo( TEXT(""), 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON) );

	ghTreeWnd = CreateWindowEx( WS_EX_CLIENTEDGE, WC_TREEVIEW, TEXT("itemtree"),
		WS_VISIBLE | WS_CHILD | TVS_DISABLEDRAGDROP | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS,
		0, itRect.bottom, TREE_WIDTH, ptRect->bottom-itRect.bottom-1, hWnd, (HMENU)IDTV_ITEMTREE, hInst, NULL );
	TreeView_SetImageList( ghTreeWnd, ghImageList, TVSIL_NORMAL );

	//	サブクラス化
	gpfOriginTreeViewProc = SubclassWindow( ghTreeWnd, gpfTreeViewProc );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	お気にリストのサブクラスプロシージャ
	@param[in]	hWnd	リストのハンドル
	@param[in]	msg		ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@return	処理結果とか
*/
LRESULT CALLBACK gpfFavListProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	UINT	ulRslt;

	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_CHAR,    Maa_OnChar  );	//	
		HANDLE_MSG( hWnd, WM_COMMAND, Maa_OnCommand );	//	アクセロリータ用

		HANDLE_MSG( hWnd, WM_KEYDOWN, Aai_OnKey );			//	20120221
		HANDLE_MSG( hWnd, WM_KEYUP,   Aai_OnKey );			//	

		case WM_MOUSEWHEEL:
			ulRslt = Maa_OnMouseWheel( hWnd, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (int)(short)HIWORD(wParam), (UINT)(short)LOWORD(wParam) );
			if( ulRslt )	return 0;
			break;

		default:	break;
	}

	return CallWindowProc( gpfOriginFavListProc, hWnd, msg, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------


/*!
	ツリービューのサブクラスプロシージャ
	@param[in]	hWnd	リストのハンドル
	@param[in]	msg		ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@return	処理結果とか
*/
LRESULT CALLBACK gpfTreeViewProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	UINT	ulRslt;

	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_CHAR,    Maa_OnChar  );	//	
		HANDLE_MSG( hWnd, WM_COMMAND, Maa_OnCommand );	//	アクセロリータ用

		HANDLE_MSG( hWnd, WM_KEYDOWN, Aai_OnKey );			//	20120221
		HANDLE_MSG( hWnd, WM_KEYUP,   Aai_OnKey );			//	

		HANDLE_MSG( hWnd, WM_MBUTTONUP, Mtv_OnMButtonUp );	

		case WM_MOUSEWHEEL:
			ulRslt = Maa_OnMouseWheel( hWnd, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (int)(short)HIWORD(wParam), (UINT)(short)LOWORD(wParam) );
			if( ulRslt )	return 0;
			break;

		default:	break;
	}

	return CallWindowProc( gpfOriginTreeViewProc, hWnd, msg, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------


/*!
	ツリービューでマウスの中バラァンがうｐされたら
	@param[in]	hWnd	ツリービューのハンドル
	@param[in]	x		クルックされたクライヤントＸ位置
	@param[in]	y		クルックされたクライヤントＹ位置
	@param[in]	flags	
*/
VOID Mtv_OnMButtonUp( HWND hWnd, INT x, INT y, UINT flags )
{
	INT	iRslt;
	HTREEITEM	hTreeItem;
	TVHITTESTINFO	stTvItemInfo;

	TRACE( TEXT("ツリービューで中クルック[%d x %d]"), x, y );

	ZeroMemory( &stTvItemInfo, sizeof(TVHITTESTINFO) );
	stTvItemInfo.pt.x = x;
	stTvItemInfo.pt.y = y;

	//	該当するアイテムを確保して
	hTreeItem = TreeView_HitTest( ghTreeWnd, &stTvItemInfo );

	//	操作する
	iRslt = TreeSelItemProc( GetParent( hWnd ), hTreeItem, 1 );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	マルチプルタブのサブクラスプロシージャ
	@param[in]	hWnd	リストのハンドル
	@param[in]	msg		ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@return	処理結果とか
*/
LRESULT	CALLBACK gpfTabMultiProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_CHAR,    Maa_OnChar  );	
		HANDLE_MSG( hWnd, WM_COMMAND, Maa_OnCommand );	//	アクセロリータ用

		HANDLE_MSG( hWnd, WM_KEYDOWN, Aai_OnKey );			//	20120221
		HANDLE_MSG( hWnd, WM_KEYUP,   Aai_OnKey );			//	

		default:	break;
	}

	return CallWindowProc( gpfOriginTabMultiProc, hWnd, msg, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------

/*!
	MAAのどっかで文字キーオサレが発生
	@param[in]	hWnd	ウインドウハンドル・発生源に注意
	@param[in]	ch		押された文字
	@param[in]	cRepeat	キーリピート回数・効いてない？
	@return		無し
*/
VOID Maa_OnChar( HWND hWnd, TCHAR ch, INT cRepeat )
{
	BOOLEAN	bShift;
	NMHDR	stNmHdr;
	INT		iTabs, iTarget;

	bShift = (0x8000 & GetKeyState(VK_SHIFT)) ? TRUE : FALSE;

	TRACE( TEXT("CHAR[%04X][%d]"), ch, bShift );

	if( VK_RETURN == ch )	//	エンター押されたら
	{
		AaItemsDoSelect( hWnd, MAA_DEFAULT, FALSE );
		return;
	}

	if( VK_TAB != ch ){	return;	}	//	タブ以外は何も無い

	iTabs = TabCtrl_GetItemCount( ghTabWnd );

	if( bShift )	//	逆回し
	{
		iTarget = gixUseTab - 1;
		if( 0 > iTarget ){	iTarget = iTabs - 1;	}
	}
	else
	{
		iTarget = gixUseTab + 1;
		if( iTabs <=  iTarget ){	iTarget = 0;	}
	}

	TabCtrl_SetCurSel( ghTabWnd, iTarget );

	stNmHdr.hwndFrom = ghTabWnd;
	stNmHdr.idFrom   = IDTB_TREESEL;
	stNmHdr.code     = TCN_SELCHANGE;

	TabBarNotify( hWnd, &stNmHdr );

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
VOID Maa_OnContextMenu( HWND hWnd, HWND hWndContext, UINT xPos, UINT yPos )
{
	HMENU	hMenu, hSubMenu;
	UINT	dRslt;
	INT		curSel, iRslt;
	TCHAR	atText[MAX_PATH], atName[MAX_PATH];
	LPARAM	lPrm;
	UINT_PTR	cchSize;
#ifndef _ORRVW
	LONG_PTR	rdExStyle;
#endif
	TCHAR	atSelName[MAX_PATH], atMenuStr[MAX_PATH], atMenuStr2[MAX_PATH];
	MULTIPLEMAA		stMulti;
	POINT			stPost;
	TVHITTESTINFO	stTvHitInfo;
	TCHITTESTINFO	stTcHitInfo;
	TCITEM			stTcItem;
	MENUITEMINFO	stMenuItemInfo;
	HTREEITEM		hTvHitItem;


	stPost.x = (SHORT)xPos;	//	画面座標はマイナスもありうる
	stPost.y = (SHORT)yPos;

	TRACE( TEXT("MAAコンテキストメニュー") );

	//	お気にリストボックスのコンテキスト
	if( ghFavLtWnd == hWndContext )
	{
		ZeroMemory( atSelName, sizeof(atSelName) );
		ZeroMemory( atMenuStr, sizeof(atMenuStr) );

		curSel = ListBox_GetCurSel( ghFavLtWnd );
		TRACE( TEXT("リストボックスコンテキスト %d"), curSel );
		if( 0 > curSel )	return;

		ListBox_GetText( ghFavLtWnd, curSel, atSelName );
		StringCchPrintf( atMenuStr,  MAX_PATH, TEXT("[ %s ]で副タブを追加"), atSelName );
		StringCchPrintf( atMenuStr2, MAX_PATH, TEXT("[ %s ]グループを削除"), atSelName );

		//	メニューは常に動的に作成する
		hMenu = CreatePopupMenu(  );
		//	お気にリストのセットを副タブに表示する機能
		AppendMenu( hMenu, MF_STRING, IDM_AATREE_SUBADD, atMenuStr );
		AppendMenu( hMenu, MF_SEPARATOR, 0, TEXT("----") );
		AppendMenu( hMenu, MF_STRING, IDM_MAA_FAVFLDR_DELETE, atMenuStr2 );

		dRslt = TrackPopupMenu( hMenu, TPM_RETURNCMD, stPost.x, stPost.y, 0, hWnd, NULL );
		switch( dRslt )
		{
			case IDM_AATREE_SUBADD:
				ZeroMemory( &stMulti, sizeof(MULTIPLEMAA) );
				StringCchCopy( stMulti.atBase, MAX_PATH, atSelName );
				//	atFilePathを空にすることで、使用リストからってことで
				stMulti.dTabNum = 0;	//	初期化・割当は２以降

				gltMultiFiles.push_back( stMulti );
				TabMultipleAppend( hWnd );
				break;

			case IDM_MAA_FAVFLDR_DELETE:	//	お気に入りリストを基点ごと削除
				SqlFavFolderDelete( atSelName );
				//	再描画
				while( ListBox_GetCount( ghFavLtWnd ) ){	ListBox_DeleteString( ghFavLtWnd, 0 );	}
				SqlFavFolderEnum( FavListFolderNameBack );
				break;

			default:	break;
		}
		DestroyMenu( hMenu );
		return;
	}

	//	ツリービューのコンテキスト
	if( ghTreeWnd == hWndContext )
	{
		hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_AATREE_POPUP) );
		hSubMenu = GetSubMenu( hMenu, 0 );

		stTvHitInfo.pt = stPost;
		ScreenToClient( ghTreeWnd, &(stTvHitInfo.pt) );
		hTvHitItem = TreeView_HitTest( ghTreeWnd, &stTvHitInfo );

		//	選択されたやつのファイル名、もしくはディレクトリ名確保
		lPrm = TreeItemInfoGet( hTvHitItem, atName, MAX_PATH );
		StringCchCat( atName, MAX_PATH, TEXT(" の操作") );
		//	名称を明示しておく
		ModifyMenu( hSubMenu, IDM_DUMMY, MF_BYCOMMAND | MF_STRING | MF_GRAYED, IDM_DUMMY, atName );
//		EnableMenuItem( hSubMenu, IDM_DUMMY, MF_GRAYED );
		if( lPrm )	//	Directoryなら
		{
			EnableMenuItem( hSubMenu, IDM_AATREE_MAINOPEN, MF_GRAYED );
			EnableMenuItem( hSubMenu, IDM_AATREE_SUBADD, MF_GRAYED );
			EnableMenuItem( hSubMenu, IDM_AATREE_GOEDIT, MF_GRAYED );
			EnableMenuItem( hSubMenu, IDM_MAA_IADD_OPEN, MF_GRAYED );
		}

		//	プロフ履歴入替
	//	ModifyMenu( hSubMenu, IDM_OPEN_HISTORY, MF_BYCOMMAND | MF_POPUP, (UINT_PTR)ghProfHisMenu, TEXT("ファイル使用履歴(&H)") );
		ModifyMenu( hSubMenu, 2, MF_BYPOSITION | MF_POPUP, (UINT_PTR)ghProfHisMenu, TEXT("ファイル使用履歴(&H)") );
		//	ポップアップへの変更は、Position指定でないと出来ない？

		//	右クリではノード選択されないようだ
		dRslt = TrackPopupMenu( hSubMenu, TPM_RETURNCMD, stPost.x, stPost.y, 0, hWnd, NULL );	//	TPM_CENTERALIGN | TPM_VCENTERALIGN | 
		DestroyMenu( hMenu );
		switch( dRslt )
		{
#ifndef _ORRVW
			//	プロフファイル開く
			case IDM_MAA_PROFILE_MAKE:	TreeProfileOpen( hWnd );	break;

			//	ディレクトリ系を再セット	ディレクトリ設定ダイヤログを開く
			case IDM_TREE_RECONSTRUCT:	TreeProfileRebuild( hWnd  );	break;

			case IDM_FINDMAA_DLG_OPEN:	TreeMaaFileFind( hWnd );	break;
#endif
			case IDM_AATREE_MAINOPEN:	TreeSelItemProc( hWnd, hTvHitItem , 0 );	break;
			case  IDM_AATREE_SUBADD:	TreeSelItemProc( hWnd, hTvHitItem , 1 );	break;
#ifndef _ORRVW
			case  IDM_AATREE_GOEDIT:	TreeSelItemProc( hWnd, hTvHitItem , 2 );	break;
#endif
			case  IDM_MAA_IADD_OPEN:	TreeSelItemProc( hWnd, hTvHitItem , 3 );	break;

			case IDM_OPEN_HIS_CLEAR:	OpenProfileLogging( hWnd, NULL );	break;

			default:
				if( IDM_OPEN_HIS_FIRST <= dRslt && dRslt <= IDM_OPEN_HIS_LAST )
				{
					OpenProfileLoad( hWnd, dRslt );
				}
				else if( IDM_OPEN_HIS_CLEAR == dRslt )	//	ファイルオーポン履歴クルヤー
				{
					OpenProfileLogging( hWnd, NULL );
				}
				break;
		}

		return;
	}

	//	タブバーのコンテキスト
	if( ghTabWnd == hWndContext )
	{
		stTcHitInfo.pt = stPost;
		ScreenToClient( ghTabWnd, &(stTcHitInfo.pt) );
		curSel = TabCtrl_HitTest( ghTabWnd, &stTcHitInfo );

		//	固定の二つの場合は無視
		if( 1 >= curSel )	return;
#pragma message ("MAAタブの固定タブの判別に注意")

		hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_AATABS_POPUP) );
		hSubMenu = GetSubMenu( hMenu, 0 );

		ZeroMemory( &stTcItem, sizeof(TCITEM) );
		stTcItem.mask       = TCIF_TEXT;
		stTcItem.pszText    = atText;
		stTcItem.cchTextMax = MAX_PATH;
		TabCtrl_GetItem( ghTabWnd, curSel, &stTcItem );

		StringCchCat( atText, MAX_PATH, TEXT(" を閉じる(&Q)") );
		StringCchLength( atText, MAX_PATH, &cchSize );

		ZeroMemory( &stMenuItemInfo, sizeof(MENUITEMINFO) );
		stMenuItemInfo.cbSize     = sizeof(MENUITEMINFO);
		stMenuItemInfo.fMask      = MIIM_TYPE;
		stMenuItemInfo.fType      = MFT_STRING;
		stMenuItemInfo.cch        = cchSize;
		stMenuItemInfo.dwTypeData = atText;
		SetMenuItemInfo( hSubMenu, IDM_AATABS_DELETE, FALSE, &stMenuItemInfo );

		//もし、お気にタブなら、編集で開くは無効にする
		if( TabMultipleIsFavTab( curSel, NULL, 0 ) )
		{
			EnableMenuItem( hSubMenu, IDM_AATREE_GOEDIT, MF_GRAYED );
		}

		dRslt = TrackPopupMenu( hSubMenu, TPM_RETURNCMD, stPost.x, stPost.y, 0, hWnd, NULL );
		DestroyMenu( hMenu );
		switch( dRslt )
		{
			case  IDM_AATABS_DELETE:	TabMultipleDelete( hWnd, curSel );	break;
			case  IDM_AATREE_GOEDIT:	TabMultipleSelect( hWnd, curSel, 1 );	break;
			//	ツリー側とはアプローチが違うから注意
			case  IDM_AATABS_ALLDELETE:	
				iRslt = MessageBox( hWnd, TEXT("全ての副タブを閉じようとしてるよ。\r\n本当に閉じちゃっていいかい？"), TEXT("お燐からの確認"), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 );
				if( IDYES == iRslt ){	TabMultipleDeleteAll( hWnd );	}
			default:	break;
		}

		return;
	}

#ifndef _ORRVW
	//	それ以外の場所のポッパップメニュー・常に手前に表示のアレ
	hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_TEMPLATE_POPUP) );
	hSubMenu = GetSubMenu( hMenu, 0 );

	rdExStyle = GetWindowLongPtr( hWnd, GWL_EXSTYLE );
	if( WS_EX_TOPMOST & rdExStyle ){	CheckMenuItem( hSubMenu , IDM_TOPMOST_TOGGLE, MF_BYCOMMAND | MF_CHECKED );	}

	dRslt = TrackPopupMenu( hSubMenu, 0, stPost.x, stPost.y, 0, hWnd, NULL );
	//	選択せずで０か－１？、選択したらそのメニューのＩＤでWM_COMMANDが発行
	DestroyMenu( hMenu );
#endif
	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	画面サイズが変わったのでサイズ変更
	@param[in]	hWnd	親ウインドウハンドル
	@param[in]	ptRect	MAAウインドウの大きさ・高さはステータスバーとタブバーの考慮ずみ
	@return		HRESULT	終了状態コード
*/
HRESULT TreeResize( HWND hWnd, LPRECT ptRect )
{
	RECT	rect, sptRect;

	//	タブバーの幅を修正
	MaaTabBarSizeGet( &rect );
	//MoveWindow( ghTabWnd, 0, 0, ptRect->right, rect.bottom, TRUE );


	SplitBarPosGet( ghSplitaWnd, &sptRect );

	MoveWindow( ghFavLtWnd, 0, rect.bottom, sptRect.left, ptRect->bottom, TRUE );
	MoveWindow( ghTreeWnd,  0, rect.bottom, sptRect.left, ptRect->bottom, TRUE );
	//	TREE_WIDTH
	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	カレントダディレクトリを受け取って、ツリーをアッセンブリーする
	@param[in]	hWnd		親ウインドウハンドル
	@param[in]	ptCurrent	カレントディレクトリ名
	@param[in]	bSubTabReb	非０で副タブ復元
	@return		HRESULT		終了状態コード
*/
HRESULT TreeConstruct( HWND hWnd, LPCTSTR ptCurrent, BOOLEAN bSubTabReb )
{
	TVINSERTSTRUCT	stTreeIns;
	SHFILEINFO	stShFileInfo;
	TCHAR	atRoote[MAX_PATH];

	ZeroMemory( gatAARoot, sizeof(gatAARoot) );
	StringCchCopy( gatAARoot, MAX_PATH, ptCurrent );

	StringCchPrintf( atRoote, MAX_PATH, TEXT("ROOT[%s]"), gatAARoot );

	StatusBarMsgSet( 2, TEXT("ツリーを構築中です") );

	TreeView_DeleteAllItems( ghTreeWnd );	//	アイテム全破壊
	//	ルートアイテム作る
	ZeroMemory( &stTreeIns, sizeof(TVINSERTSTRUCT) );
	stTreeIns.hParent        = TVI_ROOT;
	stTreeIns.hInsertAfter   = TVI_SORT;
	stTreeIns.item.mask      = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
	stTreeIns.item.pszText   = atRoote;//TEXT("ROOT");
	stTreeIns.item.lParam    = 1;
	stTreeIns.item.cChildren = 1;

	SHGetFileInfo( TEXT(""), 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON) );
	stTreeIns.item.iImage = stShFileInfo.iIcon;
	SHGetFileInfo( TEXT(""), 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON) );
	stTreeIns.item.iSelectedImage = stShFileInfo.iIcon;

	ghTreeRoot = TreeView_InsertItem( ghTreeWnd, &stTreeIns );

	//	ディレクトリ指定が無かったら終わり
	if( 0 == ptCurrent[0] )
	{
		StatusBarMsgSet( 2, TEXT("") );
		return E_INVALIDARG;
	}

	//	カレントダディレクトリはフルパスのはず

	//	プロファイルモードなら、常にSQLからでおｋ

	//	SQLから展開
#ifdef MAA_VIRTUAL_TREE
	//ここでは展開しない
#else
	TreeItemFromSql( ptCurrent, ghTreeRoot, 0 );
#endif

	StatusBarMsgSet( 2, TEXT("") );
	TreeView_Expand( ghTreeWnd, ghTreeRoot, TVE_EXPAND );

	//	副タブもSQLから再構築
	if( bSubTabReb ){	TabMultipleRestore( hWnd  );	}	//	終了時の副タブを復帰する

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ツリーノードのフルパスを確保
	@param[in]	hNode	対象のツリーノード
	@param[out]	ptPath	フルパス入れるバッファ・MAX_PATHであること
*/
UINT TreeNodePathGet( HTREEITEM hNode, LPTSTR ptPath )
{
	UINT	i;
	TCHAR	atName[MAX_PATH], atPath[MAX_PATH];
	HTREEITEM	hParent;


	if( ghTreeRoot == hNode )	//	ルート自身であった場合はすぐ返しておｋ
	{
		StringCchCopy( ptPath, MAX_PATH, gatAARoot );
		return 1;
	}

	//	選択されたやつのファイル名、もしくはディレクトリ名確保
	TreeItemInfoGet( hNode, atName, MAX_PATH );

	//	上に辿って、ファイルパスを作る
	for( i = 0; 12 > i; i++ )	//	１２より腐海階層はたどれない
	{
		hParent = TreeView_GetParent( ghTreeWnd, hNode );
		if( !(hParent) )	return 0;	//	ルートの上はない
		if( ghTreeRoot == hParent ){	break;	}	//	検索がルートまでイッたら終わり

		TreeItemInfoGet( hParent, atPath, MAX_PATH );

		//	今のパスを記録していくと、最終的にルート直下のディレクトリ名になる
		PathAppend( atPath, atName );
		StringCchCopy( atName, MAX_PATH, atPath );

		hNode = hParent;
	}

	//	ルート位置をくっつけてフルパスにする
	StringCchCopy( atPath, MAX_PATH, gatAARoot );
	PathAppend( atPath, atName );

	StringCchCopy( ptPath, MAX_PATH, atPath );

	return 1;
}
//-------------------------------------------------------------------------------------------------

#ifdef MAA_VIRTUAL_TREE
/*!
	ディレクトリとファイルをＳＱＬからツリービューに展開・再帰するわけではない
	@param[in]	hTreeParent	対象ディレクトリのツリーアイテム・こいつにぶら下げていく
	@return		UINT		最後のID
*/
HRESULT TreeItemFromSqlII( HTREEITEM hTreeParent )
{
	TCHAR	atPath[MAX_PATH], atCurrent[MAX_PATH], atNodeName[MAX_PATH];
	UINT	dPrntID, tgtID, type;

	HTREEITEM	hNewParent, hLastDir = TVI_FIRST;
	TVINSERTSTRUCT	stTreeIns;
	SHFILEINFO	stShFileInfo;

	ZeroMemory( &stTreeIns, sizeof(TVINSERTSTRUCT) );
	stTreeIns.hParent      = hTreeParent;
	stTreeIns.hInsertAfter = TVI_LAST;
	stTreeIns.item.mask    = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;

	ZeroMemory( atCurrent, sizeof(atCurrent) );
	TreeNodePathGet( hTreeParent, atCurrent );

	dPrntID = MaaSearchTreeID( hTreeParent );	//	こいつのＩＤがパレントになる

	tgtID = 0;

	do{
		ZeroMemory( atNodeName, sizeof(atNodeName) );
		tgtID = SqlChildNodePickUpID( dPrntID, tgtID, &type, atNodeName );
		if( 0 == tgtID )	break;

		StringCchCopy( atPath, MAX_PATH, atCurrent );
		PathAppend( atPath, atNodeName );

		SHGetFileInfo( atPath, 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON) );
		stTreeIns.item.iImage = stShFileInfo.iIcon;
		SHGetFileInfo( atPath, 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON) );
		stTreeIns.item.iSelectedImage = stShFileInfo.iIcon;
		stTreeIns.item.pszText = atNodeName;

		if( FILE_ATTRIBUTE_DIRECTORY == type )	//	ディレクトリの場合
		{
			stTreeIns.item.lParam    = 1;
			stTreeIns.item.cChildren = 1;
			stTreeIns.hInsertAfter   = hLastDir;
			hNewParent = TreeView_InsertItem( ghTreeWnd, &stTreeIns );
			hLastDir = hNewParent;
		}
		else	//	ファイルの場合
		{
			stTreeIns.item.lParam    = 0;
			stTreeIns.item.cChildren = 0;
			stTreeIns.hInsertAfter   = TVI_LAST;
			hNewParent = TreeView_InsertItem( ghTreeWnd, &stTreeIns );
		}

	}while( tgtID );


	return S_OK;
}
//-------------------------------------------------------------------------------------------------

#else

/*!
	ディレクトリとファイルをＳＱＬからツリービューに展開・再帰に注意セヨ
	@param[in]	ptRootName	検索するディレクトリ名
	@param[in]	hTreeParent	対象ディレクトリのツリーアイテム・こいつにぶら下げていく
	@param[in]	dPrntID		SQLのID・ディレクトリ番号
	@return		UINT		最後のID
*/
UINT TreeItemFromSql( LPCTSTR ptRootName, HTREEITEM hTreeParent, UINT dPrntID )
{
	TCHAR	atPath[MAX_PATH], atNewTop[MAX_PATH], atTarget[MAX_PATH];
	BOOL	bLooping;
	UINT	dPnID, dTgtID, type, dPrvID;

	HTREEITEM	hNewParent, hLastDir = TVI_FIRST;
	TVINSERTSTRUCT	stTreeIns;
	SHFILEINFO	stShFileInfo;

	ZeroMemory( &stTreeIns, sizeof(TVINSERTSTRUCT) );
	stTreeIns.hParent      = hTreeParent;
	stTreeIns.hInsertAfter = TVI_LAST;
	stTreeIns.item.mask    = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;

	ZeroMemory( atTarget, sizeof(atTarget) );
	dTgtID = SqlTreeNodePickUpID( dPrntID, &type, &dPnID, atTarget, 0x01 );
	//	無くなったら終わり
	if( 0 == dTgtID )	return 0;
	//	この時点でＩＤが異なるのは、ディレクトリ指定だけがあって、中身が無い場合
	if( dPrntID != dPnID )	return dPrntID;

	bLooping = TRUE;
	do
	{
		StringCchCopy( atPath, MAX_PATH, ptRootName );
		PathAppend( atPath, atTarget );

		SHGetFileInfo( atPath, 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON) );
		stTreeIns.item.iImage = stShFileInfo.iIcon;
		SHGetFileInfo( atPath, 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON) );
		stTreeIns.item.iSelectedImage = stShFileInfo.iIcon;
		stTreeIns.item.pszText = atTarget;

		if( FILE_ATTRIBUTE_DIRECTORY == type )	//	ディレクトリの場合
		{
			stTreeIns.item.lParam  = 1;
			stTreeIns.hInsertAfter = hLastDir;
			hNewParent = TreeView_InsertItem( ghTreeWnd, &stTreeIns );
			hLastDir = hNewParent;

			StringCchCopy( atNewTop, MAX_PATH, ptRootName );
			PathAppend( atNewTop, atTarget );

			dTgtID = TreeItemFromSql( atNewTop, hNewParent, dTgtID );	//	該当ディレクトリ内を再帰検索
		}
		else	//	ファイルの場合
		{
			stTreeIns.item.lParam  = 0;
			stTreeIns.hInsertAfter = TVI_LAST;
			hNewParent = TreeView_InsertItem( ghTreeWnd, &stTreeIns );
		}

		//	この時点でdTgtIDが０になることはない？

		dPrvID = dTgtID;
		ZeroMemory( atTarget, sizeof(atTarget) );
//		SetLastError(0);
		dTgtID = SqlTreeNodePickUpID( dTgtID, &type, &dPnID, atTarget, 0x01 );
//中でエラー
		TRACE( TEXT("%4u\t%4u\t%4u\t%s"), dTgtID, type, dPnID, atTarget );

		if( 0 == dTgtID )	bLooping = FALSE;
		if( dPrntID != dPnID )	bLooping = FALSE;
	}
	while( bLooping );

	return dPrvID;
}
//-------------------------------------------------------------------------------------------------

#endif

/*!
	ツリーノードハンドルを渡して、該当するSqlIDを引っ張る・再帰
*/
INT MaaSearchTreeID( HTREEITEM hItem )
{
	TCHAR	atName[MAX_PATH];
	HTREEITEM	hPrntItem;
	UINT	dPrntID, ownID;

	TreeItemInfoGet( hItem, atName, MAX_PATH );	//	ノードの名前とって

	hPrntItem = TreeView_GetParent( ghTreeWnd, hItem );	//	親ノード確認

	//	親ノードのＩＤを確保・親がルートならhPrntItemはNULL
	if( !(hPrntItem)  ){	return 0;	}
	else{	dPrntID = MaaSearchTreeID( hPrntItem );	}

	//	該当する親ノードと名称をもつノードは一つしかない
	ownID = SqlTreeFileGetOnParent( atName, dPrntID );

	return ownID;
}
//-------------------------------------------------------------------------------------------------

/*!
	IDを渡して、該当アイテムのツリーノードハンドルをとる・再帰
	@param[in]	dOwnID	検索したいノードのSqlID
	@return		ヒットしたツリーノードハンドル・もしくはNULL
*/
HTREEITEM MaaSearchTreeItem( INT dOwnID )
{
	UINT	dType, dPrntID;
	TCHAR	atFileName[MAX_PATH], atCmprName[MAX_PATH];
	HTREEITEM	hPrntItem, hChildItem, hNextItem;

	ZeroMemory( atFileName, sizeof(atFileName) );
	dType   = 0;
	dPrntID = 0;

	//	IDで、SQLから該当アイテムの情報をひっぱる
	SqlTreeNodePickUpID( dOwnID, &dType, &dPrntID, atFileName, 0x11 );

	if( dPrntID )	//	上が有るようなら、再帰する
	{
		hPrntItem = MaaSearchTreeItem( dPrntID );
	}
	else	//	上がなかったら、ルートと見なして、ルートを種にしてCHILD検索
	{
		hPrntItem = ghTreeRoot;
	}

	if( !(hPrntItem) )	return NULL;	//	データ無かったら終わる

	//	ツリーノードハンドルが返ってきたら、それを種にしてCHILD検索
	//	ヒットしたら、該当ツリーノードハンドルを返す
	//	もし未展開なら、ここでNULLが帰る・そのときは、dPrntIDとatFileNameで該当アイテムを探して開ける必要がある
	TreeView_Expand( ghTreeWnd, hPrntItem, TVE_EXPAND );
	//	もしくは、先に該当パレントのアイテムを展開してからさがす
	hChildItem = TreeView_GetChild( ghTreeWnd, hPrntItem );

	do{
		//	名前引っ張って atFileName と照合
		TreeItemInfoGet( hChildItem, atCmprName, MAX_PATH );

		if( !( StrCmp( atFileName, atCmprName ) ) )
		{
			//	ヒットしたら
			break;
		}

		hNextItem = TreeView_GetNextSibling( ghTreeWnd, hChildItem );
		hChildItem = hNextItem;

	}while( hNextItem );


	return hChildItem;	//	ヒットしたら、該当ツリーノードハンドルを返す
}
//-------------------------------------------------------------------------------------------------

/*!
	対象のSqlIDを受け取って、該当のツリーアイテムをひらく
	@param[in]	hDlg	ダイヤログハンドル
	@param[in]	tgtID	対象のSqlID
	@return		ヒットしたツリーノードハンドル・もしくはNULL
*/
HTREEITEM MaaSelectIDfile( HWND hDlg, INT tgtID )
{
	HTREEITEM	hTgtItem;

	//	SqlIDを渡すと、該当するツリーノードハンドルが戻ってくるはず
	hTgtItem = MaaSearchTreeItem( tgtID );

	if( hTgtItem ){	TreeView_SelectItem( ghTreeWnd, hTgtItem );	}	//	選択状態にする

	return hTgtItem;
}
//-------------------------------------------------------------------------------------------------

/*!
	ツリーのアイテムの名前とＰＡＲＡＭ情報を確保
	@param[in]	hTrItem	アイテムハンドル
	@param[out]	ptName	名前を入れるバッファへのポインタ・NULLでも良い
	@param[in]	cchName	バッファサイズ
	@return		LPARAM	ＰＡＲＡＭ情報
*/
LPARAM TreeItemInfoGet( HTREEITEM hTrItem, LPTSTR ptName, size_t cchName )
{
	TCHAR	atBuffer[MAX_PATH];
	TVITEM	stTvItem;

	ZeroMemory( &stTvItem, sizeof(TVITEM) );
	ZeroMemory( atBuffer, sizeof(atBuffer) );

	//	名前とディレクトリかファイルかを引っ張り出す
	stTvItem.hItem      = hTrItem;
	stTvItem.mask       = TVIF_TEXT | TVIF_PARAM;
	stTvItem.pszText    = atBuffer;
	stTvItem.cchTextMax = MAX_PATH;
	TreeView_GetItem( ghTreeWnd, &stTvItem );

	if( ptName )	//	バッファが有効なら
	{
		ZeroMemory( ptName, sizeof(TCHAR) * cchName );
		StringCchCopy( ptName, cchName, atBuffer );
	}

	return stTvItem.lParam;
}
//-------------------------------------------------------------------------------------------------

/*!
	ツリーのノーティファイメッセージの処理
	@param[in]	hWnd		親ウインドウのハンドル
	@param[in]	pstNmTrView	NOTIFYの詳細
	@return		処理した内容とか
*/
LRESULT TreeNotify( HWND hWnd, LPNMTREEVIEW pstNmTrView )
{
	INT		nmCode;

	HTREEITEM	hSelItem;
	LPTVITEM	pstTvItem;

	nmCode = pstNmTrView->hdr.code;

	//	右クリックはコンテキストメニューへ

	if( TVN_SELCHANGED == nmCode )	//	選択した後
	{
		hSelItem = TreeView_GetSelection( ghTreeWnd );	//	選択されてるアイテム

		AaTitleClear(  );	//	ここでクルヤーせないかん

		TreeSelItemProc( hWnd, hSelItem, 0 );
	}


#ifdef MAA_VIRTUAL_TREE
	if( TVN_ITEMEXPANDING == nmCode )
	{
		TRACE( TEXT("TVN_ITEMEXPANDING[%u]"), pstNmTrView->action );

		if( pstNmTrView->action & TVE_EXPAND )
		{
			pstTvItem = &(pstNmTrView->itemNew);
			if( pstTvItem->state & TVIS_EXPANDEDONCE )	return 0;

			TreeItemFromSqlII( pstTvItem->hItem );
		}
	}
#endif

	return 0;
}
//-------------------------------------------------------------------------------------------------

/*!
	ツリーの選択したアイテムからの処理のチェイン・操作を統合
	@param[in]	hWnd		親ウインドウのハンドル
	@param[in]	hSelItem	選択してるアイテム
	@param[in]	dMode		０主タブで　１副タブで　２編集ビューで　開く　３アイテム追加
	@return		非０処理した　０してない
*/
INT TreeSelItemProc( HWND hWnd, HTREEITEM hSelItem, UINT dMode )
{
	UINT	i;
	TCHAR	atName[MAX_PATH], atPath[MAX_PATH], atBaseName[MAX_PATH];
	LPARAM	lParam;
#ifndef _ORRVW
//	LPARAM	dNumber;
#endif
	HTREEITEM	hParentItem;

	MULTIPLEMAA	stMulti;

	if( !(hSelItem) ){	return 0;	}	//	なんか無効なら何もしない

	//	選択されたやつのファイル名、もしくはディレクトリ名確保
	lParam = TreeItemInfoGet( hSelItem, atName, MAX_PATH );

	if( lParam ){	return 0;	}	//	ディレクトリなら何もしない

	//	選択した名前を確保・ルートにある場合これで適用される
	if( 0 == dMode )
	{
		StringCchCopy( gatBaseName, MAX_PATH, atName );
		StatusBarMsgSet( 2, atName );	//	ステータスバーにファイル名表示
	}
	else
	{
		StringCchCopy( atBaseName, MAX_PATH, atName );
	}

	//	ベース名を、所属するディレクトリ名にする。ルートのファイルならそのまま

	//	上に辿って、ファイルパスを作る
	for( i = 0; 12 > i; i++ )	//	１２より腐海階層はたどれない
	{
		hParentItem = TreeView_GetParent( ghTreeWnd, hSelItem );
		if( !(hParentItem) )	return 0;	//	ルートの上はない・そして選択もされない
		if( ghTreeRoot == hParentItem ){	break;	}	//	検索がルートまでイッたら終わり

		TreeItemInfoGet( hParentItem, atPath, MAX_PATH );

		if( 0 == i )	//	初回のみなら、所属するディレクトリ名になる	20110928
		{
			if( 0 == dMode ){	StringCchCopy( gatBaseName, MAX_PATH, atPath );	}
			else{				StringCchCopy( atBaseName, MAX_PATH, atPath );	}
		}

		//	今のパスを記録していくと、最終的にルート直下のディレクトリ名になる
		PathAppend( atPath, atName );
		StringCchCopy( atName, MAX_PATH, atPath );

		hSelItem = hParentItem;
	}

	//	ルート位置をくっつけてフルパスにする
	StringCchCopy( atPath, MAX_PATH, gatAARoot );
	PathAppend( atPath, atName );

	switch( dMode )
	{
		default:	//	主タブで開く場合
		case  0:	AaItemsDoShow( hWnd , atPath, ACT_ALLTREE );	break;	//	そのMLTを開く

		case  1:	//	副タブに開く場合
			ZeroMemory( &stMulti, sizeof(MULTIPLEMAA) );
			StringCchCopy( stMulti.atFilePath, MAX_PATH, atPath );
			StringCchCopy( stMulti.atBase, MAX_PATH, atBaseName );
			stMulti.dTabNum = 0;	//	初期化・割当は２以降

			gltMultiFiles.push_back( stMulti );
			TabMultipleAppend( hWnd );
			break;
#ifndef _ORRVW
		case  2:	//	編集ビューで開く場合
			DocDoOpenFile( hWnd , atPath );	//	開いて中身展開
			break;

		//	アイテム追加
		case  3:	AacItemAdding( hWnd, atPath );	break;
#endif
	}

	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	選択しているリストの主体ディレクトリ名もしくはファイル名のバッファポインタを確保
	@return	ポインタ
*/
LPTSTR TreeBaseNameGet( VOID )
{
	return gatBaseName;
}
//-------------------------------------------------------------------------------------------------




/*!
	タブバーのサイズを確保する
	@param[in]	pstRect	サイズ入れるアレ
*/
VOID MaaTabBarSizeGet( LPRECT pstRect )
{
//	RECT	rect;
	RECT	itRect;
	LONG	height;

	assert( pstRect );

	pstRect->left   = 0;
	pstRect->top    = 0;

	TabCtrl_GetItemRect( ghTabWnd, 1, &itRect );
	itRect.bottom -= itRect.top;
	itRect.right -= itRect.left;
	itRect.top = 0;
	itRect.left = 0;
	TabCtrl_AdjustRect( ghTabWnd, 0, &itRect );
	height = itRect.top;
	GetWindowRect( ghTabWnd, &itRect );
	itRect.right -= itRect.left;

	pstRect->right  = itRect.right;
	pstRect->bottom = height;

	//GetWindowRect( ghTabWnd, &rect );
	//pstRect->right  = rect.right  - rect.left;
	//pstRect->bottom = rect.bottom - rect.top;

	return;
}
//-------------------------------------------------------------------------------------------------

VOID TabBarResize( HWND hWnd, LPRECT pstRect )
{
	RECT	tbRect;

	MoveWindow( ghTabWnd, 0, 0, pstRect->right, pstRect->bottom, TRUE );
	MaaTabBarSizeGet( &tbRect );
	MoveWindow( ghTabWnd, 0, 0, tbRect.right, tbRect.bottom, TRUE );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	タブバーのノーティファイメッセージの処理
	@param[in]	hWnd		親ウインドウのハンドル
	@param[in]	pstNmhdr	NOTIFYの詳細
	@return		処理した内容とか
*/
LRESULT TabBarNotify( HWND hWnd, LPNMHDR pstNmhdr )
{
	INT		nmCode;
	INT		curSel;

	NMTREEVIEW	stNmTrView;

	//pstNmhdr->hwndFrom;
	nmCode   = pstNmhdr->code;

	//	右クリックはコンテキストメニューへ

	if( TCN_SELCHANGE == nmCode )	//	タブをチェンジしたあと
	{
		curSel = TabCtrl_GetCurSel( ghTabWnd );

		TRACE( TEXT("TAB sel [%d]"), curSel );

		ShowWindow( ghTreeWnd,  SW_HIDE );
		ShowWindow( ghFavLtWnd, SW_HIDE );

		AaTitleClear(  );	//	ここで変換して問題ないはず

		if( ACT_ALLTREE == curSel )
		{
	//		AacMatrixClear(  );
			ShowWindow( ghTreeWnd, SW_SHOW );
			gixUseTab = ACT_ALLTREE;
			//	選択を発生させる
			ZeroMemory( &stNmTrView, sizeof(NMTREEVIEW) );
			stNmTrView.hdr.hwndFrom = ghTreeWnd;
			stNmTrView.hdr.idFrom   = IDTV_ITEMTREE;
			stNmTrView.hdr.code     = TVN_SELCHANGED;
			//	他は使ってないから０でおｋ
			TreeNotify( hWnd, &stNmTrView );
		}
		else if( ACT_FAVLIST == curSel )
		{
			//	オーポンされたときに、全部書換
			while( ListBox_GetCount( ghFavLtWnd ) ){	ListBox_DeleteString( ghFavLtWnd, 0 );	}
			SqlFavFolderEnum( FavListFolderNameBack );

	//		AacMatrixClear(  );
			ShowWindow( ghFavLtWnd, SW_SHOW );
			gixUseTab = ACT_FAVLIST;
		}
		else
		{
			TabMultipleSelect( hWnd, curSel, 0 );
		}
	}

	return 0;
}
//-------------------------------------------------------------------------------------------------

/*!
	副タブはお気にリストのであるか
	@param[in]	tabSel	選択されたタブ番号
	@param[out]	ptBase	ベース名を入れるバッファへのポインター・NULL可
	@param[in]	cchSize	バッファの文字数
	@return	非０お気にである　０違う
*/
UINT TabMultipleIsFavTab( INT tabSel, LPTSTR ptBase, UINT_PTR cchSize )
{
	MLTT_ITR	itNulti;

	for( itNulti = gltMultiFiles.begin( ); gltMultiFiles.end( ) != itNulti; itNulti++ )
	{
		if( tabSel == itNulti->dTabNum )	//	選択されてるやつをさがす
		{
			//	とりあえずコピー
			if( ptBase ){	StringCchCopy( ptBase, cchSize, itNulti->atBase );	}

			if( NULL == itNulti->atFilePath[0] )	return 1;	//	お気にである
			else	return 0;	//	戻っておｋ
		}
	}

	return 0;	//	ヒットしなかったらとりあえず違うことにする
}
//-------------------------------------------------------------------------------------------------

/*!
	副タブから選択した場合
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	tabSel	選択されたタブ番号
	@param[in]	dMode	０タブ選択した　１編集ビューで開く
	@return	非０MLT開いた　０なかった
*/
INT TabMultipleSelect( HWND hWnd, INT tabSel, UINT dMode )
{
	MLTT_ITR	itNulti;
	TCHAR	atName[MAX_PATH];
#ifndef _ORRVW
//	LPARAM	dNumber;
#endif

	if( 0 == dMode )	gixUseTab = tabSel;


	for( itNulti = gltMultiFiles.begin( ); gltMultiFiles.end( ) != itNulti; itNulti++ )
	{
		if( tabSel == itNulti->dTabNum )	//	選択されてるやつをさがす
		{
			if( 0 == dMode )	//	ビューエリアに表示
			{
				//	基点ディレクトリをセット
				StringCchCopy( gatBaseName, MAX_PATH, itNulti->atBase );

				//	ここで、ファイルかお気にかを判断する・atFilePathが空であれば
				if( NULL == itNulti->atFilePath[0] )	//	お気にである
				{
					StringCchCopy( atName, MAX_PATH, gatBaseName );
					StringCchCat(  atName, MAX_PATH, TEXT("[F]") );

					AaItemsDoShow( hWnd, gatBaseName, ACT_FAVLIST );	//	引っ張ってくる元の指定である
				}
				else
				{
					//	ファイル名を確保・表示用
					StringCchCopy( atName, MAX_PATH, itNulti->atFilePath );
					PathStripPath( atName );

					//	そのMLTを開く・違いが重要
					AaItemsDoShow( hWnd, itNulti->atFilePath, ACT_SUBITEM );
				}

				StatusBarMsgSet( 2, atName );	//	ステータスバーにファイル名表示
			}
#ifndef _ORRVW
			else	//	ファイル名を確保して、さらに編集ビュー側で開く処理をする
			{
				DocDoOpenFile( hWnd, itNulti->atFilePath );
			}
#endif
			return 1;
		}
	}

	return 0;
}
//-------------------------------------------------------------------------------------------------

/*!
	表示した天辺位置を記憶する
	@param[in]	dTop	新しい位置・－１なら、記憶されている位置を返す
	@return	INT	呼出なら該当する値
*/
INT TabMultipleTopMemory( INT dTop )
{
	MLTT_ITR	itNulti;

	//	関係ないなら何もしない
	if( ACT_SUBITEM > gixUseTab )	return 0;

	for( itNulti = gltMultiFiles.begin( ); gltMultiFiles.end( ) != itNulti; itNulti++ )
	{
		if( gixUseTab == itNulti->dTabNum )	//	選択されてるやつをさがす
		{
			//	値を入れたり出したり
			if( 0 >  dTop ){	dTop = itNulti->dLastTop;	}
			else{	itNulti->dLastTop = dTop;	}
			break;
		}
	}

	if( 0 > dTop )	dTop = 0;	//	一応安全対策

	return dTop;
}
//-------------------------------------------------------------------------------------------------

/*!
	複数ファイルをINIに保存する
	@param[in]	hWnd	ウインドウハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT TabMultipleStore( HWND hWnd )
{
	MLTT_ITR	itNulti;

	//	一旦全消しして書き直ししてる
	SqlMultiTabDelete(  );
	for( itNulti = gltMultiFiles.begin( ); gltMultiFiles.end( ) != itNulti; itNulti++ )
	{
		SqlMultiTabInsert( itNulti->atFilePath, itNulti->atBase );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	複数ファイルをINIから読み込んで再展開する
	@param[in]	hWnd	ウインドウハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT TabMultipleRestore( HWND hWnd )
{
	INT	iCount, i;
	MULTIPLEMAA	stMulti;

	TabMultipleDeleteAll( hWnd );

	iCount = SqlTreeCount( 2, NULL );

	for( i = 0; iCount > i; i++ )
	{
		ZeroMemory( &stMulti, sizeof(MULTIPLEMAA) );

		SqlMultiTabSelect( i+1, stMulti.atFilePath, stMulti.atBase );

		gltMultiFiles.push_back( stMulti );
		TabMultipleAppend( hWnd );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	タブを増やす・保持リストにファイル名ぶち込んだら直ちに呼ぶべし
	@param[in]	hWnd	親ウインドウのハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT TabMultipleAppend( HWND hWnd )
{
	TCHAR	atName[MAX_PATH];
	LONG	tCount;
//	RECT	itRect;
	TCITEM	stTcItem;

	MLTT_ITR	itNulti;


	itNulti = gltMultiFiles.end( );
	itNulti--;	//	新しく開くのは末端にあるはず
	StringCchCopy( atName, MAX_PATH, itNulti->atFilePath );
	if( NULL !=  atName[0] )	//	ツリービューからなら
	{
		PathStripPath( atName );
		PathRemoveExtension( atName );	//	拡張子を外す
	}
	else	//	お気にリストから追加する
	{
		StringCchCopy( atName, MAX_PATH, itNulti->atBase );
		StringCchCat(  atName, MAX_PATH, TEXT("[F]") );
	}

	ZeroMemory( &stTcItem, sizeof(TCITEM) );
	stTcItem.mask = TCIF_TEXT | TCIF_PARAM;

	tCount = TabCtrl_GetItemCount( ghTabWnd );

	stTcItem.lParam  = 0;//tCount;ファイルなので０でいい
	stTcItem.pszText = atName;
	TabCtrl_InsertItem( ghTabWnd, tCount, &stTcItem );

	itNulti->dTabNum = tCount;

	Maa_OnSize( hWnd, 0, 0, 0 );	//	引数は使ってなかったか

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	副タブを全部閉じる
	@param[in]	hWnd	ウインドウハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT TabMultipleDeleteAll( HWND hWnd )
{
	INT	ttlSel, i;
	NMHDR	stNmHdr;

	ttlSel = TabCtrl_GetItemCount( ghTabWnd );

	//	全破壊
//	for( i = 2; ttlSel > i; i++ ){	TabCtrl_DeleteItem( ghTabWnd, i );	}間違い
	for( i = (ttlSel-1); 2 <= i; i-- ){	TabCtrl_DeleteItem( ghTabWnd, i );	}

	gltMultiFiles.clear();

	//	ツリーに選択を戻す
	TabCtrl_SetCurSel( ghTabWnd, ACT_ALLTREE );
	stNmHdr.hwndFrom = ghTabWnd;
	stNmHdr.idFrom   = IDTB_TREESEL;
	stNmHdr.code     = TCN_SELCHANGE;
	TabBarNotify( hWnd, &stNmHdr );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定のタブを閉じる
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	tabSel	タブ番号
	@return		HRESULT	終了状態コード
*/
HRESULT TabMultipleDelete( HWND hWnd, CONST INT tabSel )
{
	INT	nowSel, i;
	NMHDR	stNmHdr;
	MLTT_ITR	itNulti;

	nowSel = TabCtrl_GetCurSel( ghTabWnd );

	TRACE( TEXT("TAB del [%d][%d]"), nowSel, tabSel );

	TabCtrl_DeleteItem( ghTabWnd, tabSel );

	for( itNulti = gltMultiFiles.begin( ); gltMultiFiles.end( ) != itNulti; itNulti++ )
	{
		if( tabSel == itNulti->dTabNum )
		{
			gltMultiFiles.erase( itNulti );
			break;
		}
	}

	//	20110808	タブ番号振り直し
	i = 2;
	for( itNulti = gltMultiFiles.begin( ); gltMultiFiles.end( ) != itNulti; itNulti++ )
	{
		itNulti->dTabNum = i;
		i++;
	}

	//	もし、削除タブが開いてるタブだったら・ツリーに選択を戻す
	if( nowSel == tabSel )
	{
		TabCtrl_SetCurSel( ghTabWnd, ACT_ALLTREE );
		stNmHdr.hwndFrom = ghTabWnd;
		stNmHdr.idFrom   = IDTB_TREESEL;
		stNmHdr.code     = TCN_SELCHANGE;
		TabBarNotify( hWnd, &stNmHdr );
	}

	Maa_OnSize( hWnd, 0, 0, 0 );	//	引数は使ってなかったか

//この段階では、記録の書き直しはしない

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	今現在開いている副タブを閉じる
	@param[in]	hWnd	ウインドウハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT TabMultipleSelDelete( HWND hWnd )
{
	INT		curSel;

	curSel = TabCtrl_GetCurSel( ghTabWnd );

	TRACE( TEXT("VIEW FILE CLOSE [%d]"), curSel );

	//	固定の二つの場合は無視
	if( 1 >= curSel )	return E_ACCESSDENIED;

	return TabMultipleDelete( hWnd, curSel );	//	タブ削除に渡す
}
//-------------------------------------------------------------------------------------------------

/*!
	検索ウインドウからの選択の処理をする
	@param[in]	hWnd	MAAのウインドウハンドルであること
*/
HRESULT TabMultipleCtrlFromFind( HWND hWnd )
{
	NMHDR	stNmHdr;

	//ここに来る前に、ツリー内で該当ノードが選択済

	//	選択タブチェンジして、ツリーの選択を発生させる
	TabCtrl_SetCurSel( ghTabWnd, ACT_ALLTREE );
	stNmHdr.hwndFrom = ghTabWnd;
	stNmHdr.idFrom   = IDTB_TREESEL;
	stNmHdr.code     = TCN_SELCHANGE;
	TabBarNotify( hWnd, &stNmHdr );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	お気に入りのリストをコールバックで受け取る
	@param[in]	dNumber		通し番号かもだ
	@param[in]	dummy		未使用
	@param[in]	fake		未使用
	@param[in]	ptFdrName	文字列
	@return		処理した内容とか
*/
LRESULT CALLBACK FavListFolderNameBack( UINT dNumber, UINT dummy, UINT fake, LPCVOID ptFdrName )
{
	INT	iOrder;

	iOrder = ListBox_AddString( ghFavLtWnd, (LPCTSTR)ptFdrName );

	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	お気に入りのリストがクルックされたとき
	@param[in]	hWnd	親ウインドウのハンドル
	@param[in]	iCode	発生したイベント
	@return		HRESULT	終了状態コード
*/
HRESULT FavListSelected( HWND hWnd, UINT iCode )
{
	TCHAR	atFdrName[MAX_PATH];
	INT	selIndex;

	if( LBN_SELCHANGE == iCode )
	{
		selIndex = ListBox_GetCurSel( ghFavLtWnd );
		if( LB_ERR == selIndex )	return E_OUTOFMEMORY;

		ListBox_GetText( ghFavLtWnd, selIndex, atFdrName );

		StringCchCopy( gatBaseName, MAX_PATH, atFdrName );
		//	確保したディレクトリ名に該当するＡＡをＳＱＬから引っ張る
		AaItemsDoShow( hWnd, atFdrName, ACT_FAVLIST );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	お気に入りの場合はクルックされたときに再描画するか・選択出来るように
	@param[in]	hWnd	親ウインドウのハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT FavContsRedrawRequest( HWND hWnd )
{
	//	関係ないときは何もしない・関係ないときは呼ばないように注意セヨ
	//	お気にリストをタブに表示したとき、再描画指定を判定する必要が有る
	if( ACT_FAVLIST == gixUseTab || TabMultipleIsFavTab( gixUseTab, NULL, 0 ) )
	{
		AaItemsDoShow( hWnd, gatBaseName, ACT_FAVLIST );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	全体ツリー、お気に入りリスト、複数ファイル、開いてるヤツを返す
	@return	INT	開いてる奴の番号
*/
INT TreeFavWhichOpen( VOID )
{
	return gixUseTab;	//	ACT_ALLTREE	ACT_FAVLIST
}
//-------------------------------------------------------------------------------------------------

/*!
	ホウィール回転が自分の上で発生したか
	@param[in]	hWnd	親ウインドウハンドル
	@param[in]	hChdWnd	マウスカーソルの↓にあった子ウインドウ
	@param[in]	xPos	発生した座標Ｘ
	@param[in]	yPos	発生した座標Ｙ
	@param[in]	zDelta	回転量・WHEEL_DELTAの倍数・正の値は前(奥)、負の値は後ろ(手前)へ回された
	@param[in]	fwKeys	押されてるキー
	@return		非０自分だった　０関係ないね
*/
UINT TreeFavIsUnderCursor( HWND hWnd, HWND hChdWnd, INT xPos, INT yPos, INT zDelta, UINT fwKeys )
{
	if( ghTreeWnd == hChdWnd )
	{
		FORWARD_WM_MOUSEWHEEL( ghTreeWnd, xPos, yPos, zDelta, fwKeys, PostMessage );
		TRACE( TEXT("TreeUnderCursor[%d]"), zDelta );
		return 1;
	}

	if( ghFavLtWnd == hChdWnd )
	{
		FORWARD_WM_MOUSEWHEEL( ghFavLtWnd, xPos, yPos, zDelta, fwKeys, PostMessage );
		TRACE( TEXT("FavUnderCursor[%d]"), zDelta );
		return 1;
	}

	return 0;
}
//-------------------------------------------------------------------------------------------------





