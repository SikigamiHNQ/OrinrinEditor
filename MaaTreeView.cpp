/*! @file
	@brief ツリービューとお気に入り、タブの制御をします
	このファイルは MaaTreeView.cpp です。
	@author	SikigamiHNQ
	@date	2011/06/22
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

//	タブにお気に入りを登録するには

/*
Ｄ＆Ｄによるツリー追加。
追加ディレクトリを用意。ファイルのフルパスの保持は？
別Table用意するとして、lParamに２や－１を？
lParamとるところでの整合性注意

簡易追加したとき、再構築したら？＜この時は消えても良いか？
簡易追加を纏めておくディレクトリを用意。テーブル増やすか？別個で持っておけば
構築時に無くならない・選択したときのファイルサーチどうする・ノードのlParamになにか？できたっけ？

ツリー右クリの 末尾にアイテム追加 を、エキストラファイル用の削除に変更する
IDM_MAA_IADD_OPEN
*/


//	TODO:	ディレクトリの閉じたり開いたりのアイコン調整

#ifdef EXTRA_NODE_STYLE
//エキストラファイルの使用追加の挙動がおかしい
//エキストラファイル追加したとき、ツリーへの追加が出来てない・描画？追加自体？
//エキストラファイルの削除 末尾にアイテム追加 を変更してやればいいだろう
#endif

#include "stdafx.h"
#include "OrinrinEditor.h"
#include "MaaTemplate.h"
//-------------------------------------------------------------------------------------------------

#define NODE_DIR	1		//!<	ノードで、ディレクトリを示す
#define NODE_FILE	0		//!<	ノードで、ファイルを示す
#define NODE_EXTRA	(-1)	//!<	ノードで、追加用ディレクトリを示す

#define TICO_DIR_CLOSE	0	//!<	ツリーアイコン：閉じたディレクトリ
#define TICO_DIR_OPEN	1	//!<	ツリーアイコン：開いたディレクトリ
#define TICO_DIR_EXTRA	2	//!<	ツリーアイコン：追加ディレクトリ
#define TICO_FILE_AST	3	//!<	ツリーアイコン：ＡＳＴファイル
#define TICO_FILE_MLT	4	//!<	ツリーアイコン：ＭＬＴファイル
#define TICO_FILE_ETC	5	//!<	ツリーアイコン：その他ファイル



//!	副タブの内容保持
typedef struct tagMULTIPLEMAA
{
	INT		dTabNum;				//!<	タブの番号・２インデックス
	TCHAR	atFilePath[MAX_PATH];	//!<	ファイルパス・空なら使用から開いた
	TCHAR	atBaseName[MAX_PATH];	//!<	使用リストに入れる時のグループ名
	TCHAR	atDispName[MAX_PATH];	//!<	タブ表示用名称

	UINT	dLastTop;				//!<	見てたAAの番号

} MULTIPLEMAA, *LPMULTIPLEMAA;

//-------------------------------------------------------------------------------------------------

extern  HWND		ghSplitaWnd;	//		スプリットバーハンドル

extern HMENU		ghProfHisMenu;	//		履歴表示する部分・動的に内容作成せないかん

static HFONT		ghTabFont;		//!<	タブ用のフォント・ちっちゃめの字

static  HWND		ghTabWnd;		//!<	選択タブのハンドル

static  HWND		ghFavLtWnd;		//!<	よく使う奴を登録するリストボックス

static  HWND		ghTreeWnd;		//!<	ツリーのハンドル
static HTREEITEM	ghTreeRoot;		//!<	ツリーのルートアイテム

//static HIMAGELIST	ghImageList;	//!<	ツリービューにくっつけるイメージリスト

static TCHAR		gatAARoot[MAX_PATH];	//!<	ＡＡディレクトリのカレント
static TCHAR		gatBaseName[MAX_PATH];	//!<	使用リストに入れる時のグループ名

static INT			gixUseTab;		//!<	今開いてるの・０ツリー　１お気に入り　２～複数ファイル
//	タブ番号であることに注意・複数ファイルリストの割当番号ではない

#ifdef HUKUTAB_DRAGMOVE
static POINT		gstMouseDown;		//!<	マウスボタンが押された位置
static INT			giDragSel;			//!<	動かそうとしたタブの番号
static BOOLEAN		gbTabDraging;		//!<	タブをドラッグしてる
#endif

static WNDPROC	gpfOriginFavListProc;	//!<	使用リストの元プロシージャ
static WNDPROC	gpfOriginTreeViewProc;	//!<	ツリービューの元プロシージャ
static WNDPROC	gpfOriginTabMultiProc;	//!<	タブの元プロシージャ


static list<MULTIPLEMAA>	gltMultiFiles;			//!<	副タブで開いているファイルの保持
typedef  list<MULTIPLEMAA>::iterator	MLTT_ITR;	//!<	副タブリストのイテレータ
//-------------------------------------------------------------------------------------------------

HRESULT	TreeItemFromSqlII( HTREEITEM  );			//!<	ディレクトリとファイルをＳＱＬからツリービューに展開

#ifdef EXTRA_NODE_STYLE
UINT	TreeNodeExtraAdding( LPCTSTR  );			//!<	エキストラファイルを追加する
HRESULT	TreeExtraItemFromSql( HTREEITEM, UINT );	//!<	エキストラファイルをＳＱＬからツリービューに展開
#endif

VOID	Mtv_OnMButtonUp( HWND, INT, INT, UINT );	//!<	ツリービューでマウスの中バラァンがうｐされた時の処理
VOID	Mtv_OnDropFiles( HWND , HDROP );			//!<	ツリービューにドラッグンドロップされたときの処理

HRESULT	TabMultipleRestore( HWND  );				//!<	複数ファイルをINIから読み込んで再展開する
INT		TabMultipleSelect( HWND, INT, UINT );		//!<	副タブから選択した場合
//INT	TabMultipleOpen( HWND , HTREEITEM );		//
HRESULT	TabMultipleDelete( HWND, CONST INT );		//!<	指定のタブを閉じる
INT		TabMultipleAppend( HWND );					//!<	タブを増やす

HRESULT	TabMultipleNameChange( HWND , INT );		//!<	タブ名前変更ダイヤログ開く

HRESULT	TabLineMultiSingleToggle( HWND );			//!<	タブの多段表示・一行表示を切り替える

UINT	TabMultipleIsFavTab( INT, LPTSTR, UINT_PTR );	//!<	副タブはお気にリストのであるか

LRESULT	CALLBACK gpfFavListProc(  HWND , UINT, WPARAM, LPARAM );	//!<	使用リストのサブクラスプロシージャ
LRESULT	CALLBACK gpfTreeViewProc( HWND , UINT, WPARAM, LPARAM );	//!<	ツリービューのサブクラスプロシージャ
LRESULT	CALLBACK gpfTabMultiProc( HWND , UINT, WPARAM, LPARAM );	//!<	タブのサブクラスプロシージャ

VOID	Mtb_OnMButtonUp( HWND, INT, INT, UINT );	//!<	

VOID	Mtb_OnLButtonDblclk( HWND, BOOL, INT, INT, UINT );	//!<	

#ifdef HUKUTAB_DRAGMOVE
VOID	TabMultipleOnLButtonDown( HWND, INT, INT, UINT );	//!<	
VOID	TabMultipleOnMouseMove(   HWND, INT, INT, UINT );	//!<	
VOID	TabMultipleOnLButtonUp(   HWND, INT, INT, UINT );	//!<	
#endif
//-------------------------------------------------------------------------------------------------

/*!
	ツリービュー・表示選択タグとかを作る
	@param[in]	hWnd	親ウインドウハンドル・NULLで破壊処理
	@param[in]	hInst	アプリの実存
	@param[in]	ptRect	メインウインドウの位置と大きさ
	@return		HRESULT	終了状態コード
*/
HRESULT TreeInitialise( HWND hWnd, HINSTANCE hInst, LPRECT ptRect )
{
	TCITEM		stTcItem;
	RECT		itRect, clRect;

//	SHFILEINFO	stShFileInfo;

	DWORD		dwStyles;

	HIMAGELIST	hTreeImgList;				//!<	
	HICON	hIcon;


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

#ifdef HUKUTAB_DRAGMOVE
	gbTabDraging = FALSE;
#endif

//表示選択タブ
	dwStyles = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | TCS_RIGHTJUSTIFY;
	//	多段モードなら、スタイルをくっつけておく
	if( !(InitParamValue( INIT_LOAD, VL_MAATAB_SNGL, 0 )) ){	dwStyles |= TCS_MULTILINE;	}

	ghTabWnd = CreateWindowEx( 0, WC_TABCONTROL, TEXT("treetab"), dwStyles, 0, 0, TREE_WIDTH, 0, hWnd, (HMENU)IDTB_TREESEL, hInst, NULL );
	SetWindowFont( ghTabWnd, ghTabFont, FALSE );

	ZeroMemory( &stTcItem, sizeof(stTcItem) );
	stTcItem.mask = TCIF_TEXT;
	stTcItem.pszText = TEXT("全て");	TabCtrl_InsertItem( ghTabWnd, 0, &stTcItem );
	stTcItem.pszText = TEXT("使用");	TabCtrl_InsertItem( ghTabWnd, 1, &stTcItem );

	//	選ばれしファイルをタブ的に追加？　タブ幅はウインドウ幅

	TabCtrl_GetItemRect( ghTabWnd, 1, &itRect );
	itRect.bottom -= itRect.top;

	itRect.right -= itRect.left;
	itRect.top  = 0;
	itRect.left = 0;
	TabCtrl_AdjustRect( ghTabWnd, 0, &itRect );

	MoveWindow( ghTabWnd, 0, 0, clRect.right, itRect.top, TRUE );

	//	サブクラス化
	gpfOriginTabMultiProc = SubclassWindow( ghTabWnd, gpfTabMultiProc );

//お気に入り用リストボックス
	ghFavLtWnd = CreateWindowEx( WS_EX_CLIENTEDGE, WC_LISTBOX, TEXT("favlist"),
		WS_CHILD | WS_VSCROLL | LBS_NOTIFY | LBS_SORT | LBS_NOINTEGRALHEIGHT,
		0, itRect.bottom, TREE_WIDTH, ptRect->bottom-itRect.bottom-1, hWnd, (HMENU)IDLB_FAVLIST, hInst, NULL );

	//	サブクラス化
	gpfOriginFavListProc = SubclassWindow( ghFavLtWnd, gpfFavListProc );


//全ＡＡリストツリー
//	ghImageList = (HIMAGELIST)SHGetFileInfo( TEXT(""), 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON) );

	ghTreeWnd = CreateWindowEx( WS_EX_CLIENTEDGE | WS_EX_ACCEPTFILES, WC_TREEVIEW, TEXT("itemtree"),
		WS_VISIBLE | WS_CHILD | TVS_DISABLEDRAGDROP | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS,
		0, itRect.bottom, TREE_WIDTH, ptRect->bottom-itRect.bottom-1, hWnd, (HMENU)IDTV_ITEMTREE, hInst, NULL );
//	TreeView_SetImageList( ghTreeWnd, ghImageList, TVSIL_NORMAL );

	//	サブクラス化
	gpfOriginTreeViewProc = SubclassWindow( ghTreeWnd, gpfTreeViewProc );

	//	アイコンくっつける
	hTreeImgList = ImageList_Create( 16, 16, ILC_COLOR24 | ILC_MASK, 6, 0 );
	hIcon = LoadIcon( hInst , MAKEINTRESOURCE(IDI_TREE_DIR_CLOSE) );	ImageList_AddIcon( hTreeImgList, hIcon );	DeleteObject( hIcon );
	hIcon = LoadIcon( hInst , MAKEINTRESOURCE(IDI_TREE_DIR_OPEN)  );	ImageList_AddIcon( hTreeImgList, hIcon );	DeleteObject( hIcon );
	hIcon = LoadIcon( hInst , MAKEINTRESOURCE(IDI_TREE_DIR_EXTRA) );	ImageList_AddIcon( hTreeImgList, hIcon );	DeleteObject( hIcon );
	hIcon = LoadIcon( hInst , MAKEINTRESOURCE(IDI_TREE_FILE_AST)  );	ImageList_AddIcon( hTreeImgList, hIcon );	DeleteObject( hIcon );
	hIcon = LoadIcon( hInst , MAKEINTRESOURCE(IDI_TREE_FILE_MLT)  );	ImageList_AddIcon( hTreeImgList, hIcon );	DeleteObject( hIcon );
	hIcon = LoadIcon( hInst , MAKEINTRESOURCE(IDI_TREE_FILE_ETC)  );	ImageList_AddIcon( hTreeImgList, hIcon );	DeleteObject( hIcon );
	TreeView_SetImageList( ghTreeWnd, hTreeImgList, TVSIL_NORMAL );


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
		HANDLE_MSG( hWnd, WM_CHAR,      Maa_OnChar  );		//	
		HANDLE_MSG( hWnd, WM_COMMAND,   Maa_OnCommand );	//	アクセロリータ用

		HANDLE_MSG( hWnd, WM_KEYDOWN,   Aai_OnKey );		//	20120221
		HANDLE_MSG( hWnd, WM_KEYUP,     Aai_OnKey );		//	

		HANDLE_MSG( hWnd, WM_MBUTTONUP, Mtv_OnMButtonUp );	//	
		HANDLE_MSG( hWnd, WM_DROPFILES, Mtv_OnDropFiles );	//	ドラグンドロップの受付

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
	ドラッグンドロップの受け入れ
	@param[in]	hWnd	親ウインドウのハンドル
	@param[in]	hDrop	ドロッピンオブジェクトハンドゥ
*/
VOID Mtv_OnDropFiles( HWND hWnd, HDROP hDrop )
{
	TCHAR	atFileName[MAX_PATH];
//	LPARAM	dNumber;
	BOOL	bRslt;

	ZeroMemory( atFileName, sizeof(atFileName) );

	DragQueryFile( hDrop, 0, atFileName, MAX_PATH );
	DragFinish( hDrop );

	bRslt = PathIsDirectory( atFileName );

	TRACE( TEXT("MTV DROP[%u][%s]"), bRslt, atFileName );

	if( bRslt ){	 return;	}	//	ファイルで無いのなら何もしない

#ifdef EXTRA_NODE_STYLE

	//	SQLに追加して、ツリーに追加する
	TreeNodeExtraAdding( atFileName );

#endif

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

		HANDLE_MSG( hWnd, WM_MBUTTONUP, Mtb_OnMButtonUp );

		HANDLE_MSG( hWnd, WM_LBUTTONDBLCLK, Mtb_OnLButtonDblclk );

#ifdef HUKUTAB_DRAGMOVE
		case WM_LBUTTONDOWN:	TabMultipleOnLButtonDown( hWnd, (INT)(SHORT)LOWORD(lParam), (INT)(SHORT)HIWORD(lParam), (UINT)(wParam) );	break;	//	
		case WM_MOUSEMOVE:		TabMultipleOnMouseMove(   hWnd, (INT)(SHORT)LOWORD(lParam), (INT)(SHORT)HIWORD(lParam), (UINT)(wParam) );	break;	//	
		case WM_LBUTTONUP:		TabMultipleOnLButtonUp(   hWnd, (INT)(SHORT)LOWORD(lParam), (INT)(SHORT)HIWORD(lParam), (UINT)(wParam) );	break;	//	
#endif

		default:	break;
	}

	return CallWindowProc( gpfOriginTabMultiProc, hWnd, msg, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------

/*!
	タブをダブルクルックしたとき
	@param[in]	hWnd			ウインドウハンドル
	@param[in]	fDoubleClick	非０ダブルクルック
	@param[in]	x				発生したクライヤントＸ座標値
	@param[in]	y				発生したクライヤントＹ座標値
	@param[in]	keyFlags		他に押されてるキーについて
*/
VOID Mtb_OnLButtonDblclk( HWND hWnd, BOOL fDoubleClick, INT x, INT y, UINT keyFlags )
{
	INT	curSel;
	//TCHITTESTINFO	stTcHitInfo;

	//stTcHitInfo.pt.x = x;
	//stTcHitInfo.pt.y = y;
	//curSel = TabCtrl_HitTest( ghTabWnd, &stTcHitInfo );
	//そのときアクティブになっているタブを選択する
	curSel = TabCtrl_GetCurSel( ghTabWnd );

	TRACE( TEXT("TAB DBLCLICK [%d] [%d x %d]"), curSel, x, y );

	if( 1 >= curSel ){	 return;	}	//	タブ　０ツリー、１使用のときは何もしない

	TabMultipleNameChange( hWnd, curSel );	//	名称変更

#pragma message ("ダブルクルックの機能を設定出来るようにするとおいしいかも")

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	副タブでマウスの中ボタンがうｐされたとき
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	x			発生したクライヤントＸ座標値
	@param[in]	y			発生したクライヤントＹ座標値
	@param[in]	keyFlags	他に押されてるキーについて
*/
VOID Mtb_OnMButtonUp( HWND hWnd, INT x, INT y, UINT flags )
{
	INT	curSel;
	TCHITTESTINFO	stTcHitInfo;

	stTcHitInfo.pt.x = x;
	stTcHitInfo.pt.y = y;
	curSel = TabCtrl_HitTest( ghTabWnd, &stTcHitInfo );
	//	タブ　０ツリー、１使用のときは何もしない

	TRACE( TEXT("MTAB start TAB [%d] [%d x %d]"), curSel, x, y );

	if( 1 >= curSel ){	 return;	}

	TabMultipleDelete( GetParent( ghTabWnd ), curSel );

	return;
}
//-------------------------------------------------------------------------------------------------

#ifdef HUKUTAB_DRAGMOVE

/*!
	副タブでマウスの左ボタンがダウンされたとき
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	x			発生したクライヤントＸ座標値
	@param[in]	y			発生したクライヤントＹ座標値
	@param[in]	keyFlags	他に押されてるキーについて
*/
VOID TabMultipleOnLButtonDown( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	TCHITTESTINFO	stTcHitInfo;

	TRACE( TEXT("MTAB LDOWN [%d x %d]"), x, y );

	gstMouseDown.x = x;	//	ダウンした位置を基点とする
	gstMouseDown.y = y;

	stTcHitInfo.pt = gstMouseDown;
	giDragSel = TabCtrl_HitTest( ghTabWnd, &stTcHitInfo );
	//	タブ　０ツリー、１使用のときは何もしない

	TRACE( TEXT("MTAB start TAB [%d]"), giDragSel );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	副タブでマウスを動かしたとき
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	x			発生したクライヤントＸ座標値
	@param[in]	y			発生したクライヤントＹ座標値
	@param[in]	keyFlags	他に押されてるキーについて
*/
VOID TabMultipleOnMouseMove( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	INT	mx, my, rx, ry;

	//	タブ　０ツリー、１使用のときは何もしない
	if( 1 >= giDragSel )	return;

	if( (keyFlags & MK_LBUTTON) && !(gbTabDraging) )
	{
		//	基点からの移動量の絶対値を確認
		mx = abs( gstMouseDown.x - x );
		my = abs( gstMouseDown.y - y );

		rx = GetSystemMetrics( SM_CXDRAG );
		ry = GetSystemMetrics( SM_CYDRAG );


		//	特定量移動したらドラッグ開始とする
		if( rx < mx || ry < my )
		{
			TRACE( TEXT("MTAB start DRAG [%d x %d] [%d x %d]"), rx, ry, mx, my );
			SetCapture( hWnd  );	//	マウスキャプチャ
			gbTabDraging = TRUE;
		}

	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	副タブでマウスの左ボタンがうっｐされたとき
	@param[in]	hWnd			ウインドウハンドル
	@param[in]	x				発生したクライヤントＸ座標値
	@param[in]	y				発生したクライヤントＹ座標値
	@param[in]	keyFlags		他に押されてるキーについて
*/
VOID TabMultipleOnLButtonUp( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	INT	iDragSel;
	POINT	point;
	TCHITTESTINFO	stTcHitInfo;

	TRACE( TEXT("MTAB LUP [%d x %d]"), x, y );
	point.x = x;
	point.y = y;

	if( gbTabDraging )
	{
		stTcHitInfo.pt = point;
		iDragSel = TabCtrl_HitTest( ghTabWnd, &stTcHitInfo );
		TRACE( TEXT("MTAB end TAB [%d]"), iDragSel );

		ReleaseCapture(  );
		gbTabDraging = FALSE;
	}

	return;
}
//-------------------------------------------------------------------------------------------------
#endif

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
	DWORD	dwStyles;
#ifdef EXTRA_NODE_STYLE
	LPARAM	iSelID = 0;
#endif
	UINT_PTR	cchSize;
	LONG_PTR	rdExStyle;
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
				StringCchCopy( stMulti.atBaseName, MAX_PATH, atSelName );
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

		if( hTvHitItem )
		{
			//	選択されたやつのファイル名、もしくはディレクトリ名確保
			lPrm = TreeItemInfoGet( hTvHitItem, atName, MAX_PATH );
			//＠＠	lParamの判断

#ifdef EXTRA_NODE_STYLE
			//	どれでもないのなら、エキストラファイルなので、ファイルにしておく・書換注意
			if( NODE_DIR != lPrm && NODE_FILE != lPrm && NODE_EXTRA != lPrm )
			{
				iSelID = lPrm;
				lPrm = NODE_FILE;
				EnableMenuItem( hSubMenu, IDM_MAA_ITEM_DELETE, MF_ENABLED );
				//	エキストラファイルの削除を有効にする
			}
#endif

			StringCchCat( atName, MAX_PATH, TEXT(" の操作") );
			//	名称を明示しておく
			ModifyMenu( hSubMenu, IDM_DUMMY, MF_BYCOMMAND | MF_STRING | MF_GRAYED, IDM_DUMMY, atName );
		}

		if( NODE_FILE != lPrm || !(hTvHitItem) )	//	ファイルでないか、未選択なら
		{
			EnableMenuItem( hSubMenu, IDM_AATREE_MAINOPEN, MF_GRAYED );
			EnableMenuItem( hSubMenu, IDM_AATREE_SUBADD,   MF_GRAYED );
			EnableMenuItem( hSubMenu, IDM_AATREE_GOEDIT,   MF_GRAYED );
		//	EnableMenuItem( hSubMenu, IDM_MAA_IADD_OPEN,   MF_GRAYED );	//	キャンセルされた
		}
		//	プロフ内のファイルも削除出来るようにしておくか？

		//	プロフ履歴入替
		ModifyMenu( hSubMenu, IDM_OPEN_HISTORY, MF_BYCOMMAND | MF_POPUP, (UINT_PTR)ghProfHisMenu, TEXT("ファイル使用履歴(&H)") );
	//	ModifyMenu( hSubMenu, 2, MF_BYPOSITION | MF_POPUP, (UINT_PTR)ghProfHisMenu, TEXT("ファイル使用履歴(&H)") );
		//	ポップアップへの変更は、Position指定でないと出来ない？

#ifdef _ORRVW
		rdExStyle = GetWindowLongPtr( hWnd, GWL_EXSTYLE );
		if( WS_EX_TOPMOST & rdExStyle ){	CheckMenuItem( hSubMenu , IDM_TOPMOST_TOGGLE, MF_BYCOMMAND | MF_CHECKED );	}
#endif


		//	右クリではノード選択されないようだ
		dRslt = TrackPopupMenu( hSubMenu, TPM_RETURNCMD, stPost.x, stPost.y, 0, hWnd, NULL );	//	TPM_CENTERALIGN | TPM_VCENTERALIGN | 
		RemoveMenu( hSubMenu, 2, MF_BYPOSITION );
		DestroyMenu( hMenu );	//このデストロイでポップアップまで破棄されるので、removeしておく
	
		switch( dRslt )
		{
			//	プロフファイル開く
			case IDM_MAA_PROFILE_MAKE:	TreeProfileOpen( hWnd );	break;

			//	ディレクトリ系を再セット	ディレクトリ設定ダイヤログを開く
			case IDM_TREE_RECONSTRUCT:	TreeProfileRebuild( hWnd  );	break;

			case IDM_FINDMAA_DLG_OPEN:	TreeMaaFileFind( hWnd );	break;

			case IDM_AATREE_MAINOPEN:	TreeSelItemProc( hWnd, hTvHitItem , 0 );	break;

			case  IDM_AATREE_SUBADD:	TreeSelItemProc( hWnd, hTvHitItem , 1 );	break;

#ifndef _ORRVW
			case  IDM_AATREE_GOEDIT:	TreeSelItemProc( hWnd, hTvHitItem , 2 );	break;
  #ifndef MAA_IADD_PLUS
		//	case  IDM_MAA_IADD_OPEN:	TreeSelItemProc( hWnd, hTvHitItem , 3 );	break;
			//キャンセルされた
  #endif
#endif
#ifdef EXTRA_NODE_STYLE
			case IDM_MAA_ITEM_DELETE:	TreeSelItemProc( hWnd, hTvHitItem , 4 );	break;
#endif
			//	ファイルオーポン履歴クルヤー
			case IDM_OPEN_HIS_CLEAR:	OpenProfileLogging( hWnd, NULL );	break;

			default:
				//	ファイルオーポン履歴
				if( IDM_OPEN_HIS_FIRST <= dRslt && dRslt <= IDM_OPEN_HIS_LAST )
				{
					OpenProfileLoad( hWnd, dRslt );
				}
#ifdef _ORRVW	//	その他はメインコマンドに回す
				else
				{
					Maa_OnCommand( hWnd, dRslt, hWndContext, 0 );
				}
#endif
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
		if( TabMultipleIsFavTab( curSel, NULL, 0 ) ){	EnableMenuItem( hSubMenu, IDM_AATREE_GOEDIT, MF_GRAYED );	}

		//	もし壱行表示スタイルなら、チェックマークいれとく
		dwStyles = GetWindowStyle( ghTabWnd );
		if( !(TCS_MULTILINE & dwStyles) ){	CheckMenuItem( hSubMenu, IDM_AATABS_SINGLETAB, MF_CHECKED );	}

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

			//	リネーム
			case IDM_AATABS_RENAME:	TabMultipleNameChange( hWnd, curSel );	break;

			//	副タブの、多段・シングル切替	20130521
			case IDM_AATABS_SINGLETAB:	TabLineMultiSingleToggle( hWnd );	break;

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
//	SHFILEINFO	stShFileInfo;
	TCHAR	atRoote[MAX_PATH];

	ZeroMemory( gatAARoot, sizeof(gatAARoot) );
	StringCchCopy( gatAARoot, MAX_PATH, ptCurrent );

	StringCchPrintf( atRoote, MAX_PATH, TEXT("ROOT[%s]"), gatAARoot );

	StatusBarMsgSet( SBMAA_FILENAME, TEXT("ツリーを構築中です") );

	TreeView_DeleteAllItems( ghTreeWnd );	//	アイテム全破壊
	//	ルートアイテム作る
	ZeroMemory( &stTreeIns, sizeof(TVINSERTSTRUCT) );
	stTreeIns.hParent        = TVI_ROOT;
	stTreeIns.hInsertAfter   = TVI_SORT;
	stTreeIns.item.mask      = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
	stTreeIns.item.pszText   = atRoote;//TEXT("ROOT");
	stTreeIns.item.lParam    = NODE_DIR;	//	１ディレクトリ　０ファイル
	stTreeIns.item.cChildren = 1;
	//	ルートのＩＤは０

	//SHGetFileInfo( TEXT(""), 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON) );
	stTreeIns.item.iImage = I_IMAGECALLBACK;//stShFileInfo.iIcon;
	//SHGetFileInfo( TEXT(""), 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON) );
	stTreeIns.item.iSelectedImage = I_IMAGECALLBACK;//stShFileInfo.iIcon;

	ghTreeRoot = TreeView_InsertItem( ghTreeWnd, &stTreeIns );

	//	ディレクトリ指定が無かったら終わり
	if( 0 == ptCurrent[0] )
	{
		StatusBarMsgSet( SBMAA_FILENAME, TEXT("") );
		return E_INVALIDARG;
	}

	//	カレントダディレクトリはフルパスのはず

	//	プロファイルモードなら、常にSQLからでおｋ

	//	SQLから展開　ここでは展開しない

	StatusBarMsgSet( SBMAA_FILENAME, TEXT("") );
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

#ifdef EXTRA_NODE_STYLE

/*!
	エキストラファイルを追加する
	@param[out]	ptPath	対象ファイルのフルパス
	@return	追加したファイルのSqlID・登録出来なかったら０
*/
UINT TreeNodeExtraAdding( LPCTSTR ptPath )
{
	UINT	id;
	LPARAM	lParam;
	HTREEITEM	hTreeRoot, hChildItem, hNextItem, hBuffItem;

	//	追加済ならメッセージ出して終了セヨ
	id = SqlTreeNodeExtraIsFileExist( ptPath );
	if( 0 < id )
	{
		MessageBox( GetDesktopWindow( ), TEXT("已に登録してあるみたいだよ。"), TEXT("お燐からのお知らせ"), MB_OK | MB_ICONINFORMATION );
		return id;
	}

	//	ルート確保
	hTreeRoot = TreeView_GetRoot( ghTreeWnd );

	//	その子ノードである追加用ノードを探す
	hChildItem = TreeView_GetChild( ghTreeWnd, hTreeRoot );
	hNextItem = NULL;

	do{
		//	ノードlParamをひっぱる・－１が該当ブツである
		lParam = TreeItemInfoGet( hChildItem, NULL, 0 );

		//	ヒットしたら終わり
		if( NODE_EXTRA == lParam  ){	break;	}

		//	なかったらその次を探す。
		hNextItem = TreeView_GetNextSibling( ghTreeWnd, hChildItem );
		if( hNextItem == hChildItem ){	hNextItem = NULL;	}	//	全部廻ったら同じ物が戻るらしい？
		hChildItem = hNextItem;

	}while( hChildItem );

	if( !(hChildItem) )	return 0;	//	ヒットしなかった
	//	ヒットしたのが追加用ノードである

	//	開けば、既存のブツも展開される
	TreeView_Expand( ghTreeWnd, hChildItem, TVE_EXPAND );
	//最初の１個目だった場合、展開されないので展開済フラグが立たない
	hBuffItem = TreeView_GetChild( ghTreeWnd, hChildItem );

	//	展開してから開かないと多重にツリーに出てくる
	id = SqlTreeNodeExtraInsert( 0, ptPath );	//	SQLに登録
	if( 0 >= id )	return 0;	//	失敗

	if( hBuffItem ){	TreeExtraItemFromSql( hChildItem, id-1 );	}	//	該当ＩＤの次から探すので注意
	else{	TreeView_Expand( ghTreeWnd, hChildItem, TVE_EXPAND );	}
	//	最初の一個の場合は、追加してから開くよろし


	return id;
}
//-------------------------------------------------------------------------------------------------


/*!
	エキストラファイルをＳＱＬからツリービューに展開
	@param[in]	hTreeParent	対象ディレクトリのツリーアイテム・こいつにぶら下げていく
	@param[in]	dFinID		このＩＤ以降のアイテムをツリーに追加する。通常０、追加時に注意セヨ
	@return	HRESULT	終了状態コード
*/
HRESULT TreeExtraItemFromSql( HTREEITEM hTreeParent, UINT dFinID )
{
	TCHAR	atPath[MAX_PATH], atNodeName[MAX_PATH];
	UINT	tgtID;
	INT		iFileType;

	HTREEITEM	hNewParent;
	TVINSERTSTRUCT	stTreeIns;
//	SHFILEINFO	stShFileInfo;

	ZeroMemory( &stTreeIns, sizeof(TVINSERTSTRUCT) );
	stTreeIns.hParent      = hTreeParent;
	stTreeIns.hInsertAfter = TVI_LAST;
	stTreeIns.item.mask    = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;

	tgtID = dFinID;

	do{
		ZeroMemory( atPath, sizeof(atPath) );
		ZeroMemory( atNodeName, sizeof(atNodeName) );

		tgtID = SqlTreeNodeExtraSelect( 0, tgtID, atPath );
		if( 0 == tgtID )	break;

		if( FileExtensionCheck( atPath, TEXT(".ast") ) ){	iFileType = TICO_FILE_AST;	}
		else if( FileExtensionCheck( atPath, TEXT(".mlt") ) ){	iFileType = TICO_FILE_MLT;	}
		else{	iFileType = TICO_FILE_ETC;	}
		//SHGetFileInfo( atPath, 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON) );
		stTreeIns.item.iImage = iFileType;//stShFileInfo.iIcon;
		//SHGetFileInfo( atPath, 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON) );
		stTreeIns.item.iSelectedImage = iFileType;//stShFileInfo.iIcon;
		stTreeIns.item.pszText = PathFindFileName( atPath );	//	ファイル名ブッコ抜く

		stTreeIns.item.lParam    = tgtID;	//	ここは特殊なので注意
		stTreeIns.item.cChildren = 0;	//	子ノードなし
		stTreeIns.hInsertAfter   = TVI_LAST;
		hNewParent = TreeView_InsertItem( ghTreeWnd, &stTreeIns );

	}while( tgtID );


	return S_OK;
}
//-------------------------------------------------------------------------------------------------

#endif

/*!
	ディレクトリとファイルをＳＱＬからツリービューに展開・再帰するわけではない
	@param[in]	hTreeParent	対象ディレクトリのツリーアイテム・こいつにぶら下げていく
	@return		HRESULT		終了状態コード
*/
HRESULT TreeItemFromSqlII( HTREEITEM hTreeParent )
{
	TCHAR	atPath[MAX_PATH], atCurrent[MAX_PATH], atNodeName[MAX_PATH];
	UINT	dPrntID, tgtID, type;
	INT		iFileType;

	HTREEITEM	hNewParent, hLastDir = TVI_FIRST;
	TVINSERTSTRUCT	stTreeIns;
//	SHFILEINFO	stShFileInfo;

	ZeroMemory( &stTreeIns, sizeof(TVINSERTSTRUCT) );
	stTreeIns.hParent      = hTreeParent;
	stTreeIns.hInsertAfter = TVI_LAST;
	stTreeIns.item.mask    = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;

	ZeroMemory( atCurrent, sizeof(atCurrent) );
	TreeNodePathGet( hTreeParent, atCurrent );

	dPrntID = MaaSearchTreeID( hTreeParent );	//	こいつのＩＤが、これから展開するノードの親になる
	//	ＩＤ０はルートノード
	tgtID = 0;

	//	ルートのときのみ、[*追加項目*]  みたいなのを付け加える。lParamは２とか？
#ifdef EXTRA_NODE_STYLE
	//最前列に追加
	if( 0 == dPrntID )
	{
		StringCchCopy( atNodeName, MAX_PATH, EXTRA_NODE );
		//	とりやえずカレントディレクトリのアイコンで良いはず	
		//SHGetFileInfo( atCurrent, 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON) );
		stTreeIns.item.iImage = TICO_DIR_EXTRA;//stShFileInfo.iIcon;
		//SHGetFileInfo( atCurrent, 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON) );
		stTreeIns.item.iSelectedImage = TICO_DIR_EXTRA;//stShFileInfo.iIcon;
		stTreeIns.item.pszText = atNodeName;
		stTreeIns.item.lParam    = NODE_EXTRA;
		stTreeIns.item.cChildren = 1;	//	子ノードアリ
		stTreeIns.hInsertAfter   = hLastDir;
		hNewParent = TreeView_InsertItem( ghTreeWnd, &stTreeIns );
		hLastDir = hNewParent;	//	ディレクトリ位置の最終・ファイルの手前	I_IMAGECALLBACK
	}
#endif

	do{
		ZeroMemory( atNodeName, sizeof(atNodeName) );
		tgtID = SqlChildNodePickUpID( dPrntID, tgtID, &type, atNodeName );
		if( 0 == tgtID )	break;

		StringCchCopy( atPath, MAX_PATH, atCurrent );
		PathAppend( atPath, atNodeName );

		//SHGetFileInfo( atPath, 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON) );
		//stTreeIns.item.iImage = stShFileInfo.iIcon;
		//SHGetFileInfo( atPath, 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON) );
		//stTreeIns.item.iSelectedImage = stShFileInfo.iIcon;
		stTreeIns.item.pszText = atNodeName;

		if( FILE_ATTRIBUTE_DIRECTORY == type )	//	ディレクトリの場合
		{
			stTreeIns.item.iImage         = I_IMAGECALLBACK;//TICO_DIR_CLOSE;
			stTreeIns.item.iSelectedImage = I_IMAGECALLBACK;//TICO_DIR_OPEN;
			stTreeIns.item.lParam    = NODE_DIR;
			stTreeIns.item.cChildren = 1;	//	子ノードあり
			stTreeIns.hInsertAfter   = hLastDir;
			hNewParent = TreeView_InsertItem( ghTreeWnd, &stTreeIns );
			hLastDir = hNewParent;	//	ディレクトリ位置の最終・ファイルの手前
		}
		else	//	ファイルの場合
		{
			if( FileExtensionCheck( atNodeName, TEXT(".ast") ) ){	iFileType = TICO_FILE_AST;	}
			else if( FileExtensionCheck( atNodeName, TEXT(".mlt") ) ){	iFileType = TICO_FILE_MLT;	}
			else{	iFileType = TICO_FILE_ETC;	}

			stTreeIns.item.iImage = iFileType;
			stTreeIns.item.iSelectedImage = iFileType;
			stTreeIns.item.lParam    = NODE_FILE;
			stTreeIns.item.cChildren = 0;	//	子ノードなし
			stTreeIns.hInsertAfter   = TVI_LAST;
			hNewParent = TreeView_InsertItem( ghTreeWnd, &stTreeIns );
		}

	}while( tgtID );


	return S_OK;
}
//-------------------------------------------------------------------------------------------------

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
	@return		LPARAM	１ディレクトリ　０ファイル
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

#ifdef EXTRA_NODE_STYLE
	LPARAM	lParam;
#endif

	HTREEITEM		hSelItem;
	LPTVITEM		pstTvItem;
	LPNMTVDISPINFO	pstDispInfo;

	nmCode = pstNmTrView->hdr.code;

	//	右クリックはコンテキストメニューへ

	if( TVN_SELCHANGED == nmCode )	//	選択した後
	{
		hSelItem = TreeView_GetSelection( ghTreeWnd );	//	選択されてるアイテム
		//	コンボックスクルヤー位置変更してみる
		TreeSelItemProc( hWnd, hSelItem, 0 );
	}


	if( TVN_ITEMEXPANDING == nmCode )	//	子ノードを展開または閉じるとき
	{
		TRACE( TEXT("TVN_ITEMEXPANDING[%X]"), pstNmTrView->action );

		if( pstNmTrView->action & TVE_EXPAND )	//	展開する
		{
			pstTvItem = &(pstNmTrView->itemNew);

			if( pstTvItem->state & TVIS_EXPANDEDONCE )	return 0;
			//	展開済なら何もしないでおｋ

#ifdef EXTRA_NODE_STYLE
			//	もしlPmaramが-1なら
			lParam = TreeItemInfoGet( pstTvItem->hItem, NULL, 0 );
			if( NODE_EXTRA == lParam )	//	追加分である
			{
				TRACE( TEXT("TREE EX DIR") );
				TreeExtraItemFromSql( pstTvItem->hItem, 0 );
			}
			else
			{
#endif
				TreeItemFromSqlII( pstTvItem->hItem );
#ifdef EXTRA_NODE_STYLE
			}
#endif
		}

		if( pstNmTrView->action & TVE_COLLAPSE )	//	閉じる
		{
			pstTvItem = &(pstNmTrView->itemNew);
			TRACE( TEXT("TVE_COLLAPSE[%d, %d]"), pstTvItem->iImage, pstTvItem->iSelectedImage );
		}
	}


	//	特異なアイコン追加するなら
	if( TVN_GETDISPINFO == nmCode )	//	なんか画像情報が居るとき
	{
		pstDispInfo = (LPNMTVDISPINFO)pstNmTrView;
		TRACE( TEXT("TVN_GETDISPINFO[%X]"), pstDispInfo->item.mask );

		if(pstDispInfo->item.mask & TVIF_IMAGE || pstDispInfo->item.mask & TVIF_SELECTEDIMAGE)
		{
			if( pstDispInfo->item.state & TVIS_EXPANDED )
			{
				pstDispInfo->item.iImage         = TICO_DIR_OPEN;
				pstDispInfo->item.iSelectedImage = TICO_DIR_OPEN;
			}
			else
			{
				pstDispInfo->item.iImage         = TICO_DIR_CLOSE;
				pstDispInfo->item.iSelectedImage = TICO_DIR_CLOSE;
			}
		}
	}


	return 0;
}
//-------------------------------------------------------------------------------------------------

/*!
	ツリーの選択したアイテムからの処理のチェイン・操作を統合
	@param[in]	hWnd		親ウインドウのハンドル
	@param[in]	hSelItem	選択してるノードのハンドル
	@param[in]	dMode		０主タブで開く　１副タブ追加　２編集ビューで開く　３アイテム追加　４ノード削除
	@return		非０処理した　０してない
*/
INT TreeSelItemProc( HWND hWnd, HTREEITEM hSelItem, UINT dMode )
{
	UINT	i;
	TCHAR	atName[MAX_PATH], atPath[MAX_PATH], atBaseName[MAX_PATH];
	LPARAM	lParam;
	HTREEITEM	hParentItem;
	MULTIPLEMAA	stMulti;

#ifdef EXTRA_NODE_STYLE
	UINT	id = 0;
#endif

	//	右クリ・ツリー選択から合流

	if( !(hSelItem) ){	return 0;	}	//	なんか無効なら何もしない

	//	選択されたのがエキストラファイルであれば？親ノードのlParam見ればわかる
	//	エキストラなら、フルパス確保せないかん　ベース名とか注意セヨ
#ifdef EXTRA_NODE_STYLE
	hParentItem = TreeView_GetParent( ghTreeWnd, hSelItem );
	lParam = TreeItemInfoGet( hParentItem, NULL, 0 );
	if( NODE_EXTRA == lParam )
	{
		//	選択されたやつには該当アイテムのsqlIDが入ってる
		lParam = TreeItemInfoGet( hSelItem, NULL, 0 );
		//	該当のアイテムを確保
		id = SqlTreeNodeExtraSelect( lParam, 0, atPath );
		if( 0 == id )	return 0;	//	なんかおかしいなら何もしない

		//	ファイル名をベース名にしておく
		if( 0 == dMode )
		{
			StringCchCopy( gatBaseName, MAX_PATH, PathFindFileName( atPath ) );
			StatusBarMsgSet( SBMAA_FILENAME , gatBaseName );	//	ステータスバーにファイル名表示
		}
		StringCchCopy( atBaseName, MAX_PATH, PathFindFileName( atPath ) );	//	いつでも記録で大丈夫か	20120530
	}
	else
	{
#endif
		//	選択されたやつのファイル名、もしくはディレクトリ名確保
		lParam = TreeItemInfoGet( hSelItem, atName, MAX_PATH );
		//＠＠	lParamの判断
		if( NODE_FILE != lParam ){	return 0;	}	//	ファイルで無いなら何もしない

		//	選択した名前を確保・ルートにある場合これで適用される
		if( 0 == dMode )
		{
			StringCchCopy( gatBaseName, MAX_PATH, atName );
			StatusBarMsgSet( SBMAA_FILENAME, atName );	//	ステータスバーにファイル名表示
		}
		StringCchCopy( atBaseName, MAX_PATH, atName );	//	いつでも記録で大丈夫か	20120530

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

#ifdef EXTRA_NODE_STYLE
	}
#endif

	switch( dMode )
	{
		default:	//	主タブで開く場合
		case  0:	AaItemsDoShow( hWnd , atPath, ACT_ALLTREE );	break;	//	そのMLTを開く

		case  1:	//	副タブに開く場合
			ZeroMemory( &stMulti, sizeof(MULTIPLEMAA) );
			StringCchCopy( stMulti.atFilePath, MAX_PATH, atPath );
			StringCchCopy( stMulti.atBaseName, MAX_PATH, atBaseName );
			stMulti.dTabNum = 0;	//	初期化・割当は２以降

			gltMultiFiles.push_back( stMulti );
			TabMultipleAppend( hWnd );
			break;
#ifndef _ORRVW
		case  2:	//	編集ビューで開く場合
			DocDoOpenFile( hWnd, atPath );	//	開いて中身展開
			break;

  #ifndef MAA_IADD_PLUS
		//	アイテム追加・キャンセル
		//case  3:	AacItemAdding( hWnd, atPath );	break;
  #endif
#endif	//	_ORRVW

#ifdef EXTRA_NODE_STYLE
		case  4:	//	ノード削除・とりやえずエキストラファイル
			if( 0 < id )	//	有効な場合
			{
				TreeView_DeleteItem( ghTreeWnd , hSelItem );	//	ツリーから削除して
				SqlTreeNodeExtraDelete( id );	//	リストからも削除
			}
			break;
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

/*!
	タブのサイズ変更
	@param[in]	hWnd	親ウインドウのハンドル
	@param[in]	pstRect	
*/
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
			AaTitleClear(  );	//	中身のある時に書換を
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
			if( ptBase ){	StringCchCopy( ptBase, cchSize, itNulti->atBaseName );	}

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
				StringCchCopy( gatBaseName, MAX_PATH, itNulti->atBaseName );

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

				StatusBarMsgSet( SBMAA_FILENAME, atName );	//	ステータスバーにファイル名表示
			}
#ifndef _ORRVW
			else	//	ファイル名を確保して、さらに編集ビュー側で開く処理をする
			{
				DocDoOpenFile( hWnd, itNulti->atFilePath );	//	ビューワ側の副タブから編集ウインドウに開くとき
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
	複タブの構成をプロファイルに保存する
	@param[in]	hWnd	ウインドウハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT TabMultipleStore( HWND hWnd )
{
	MLTT_ITR	itNulti;


	SqlMultiTabDelete(  );	//	一旦SQLの内容全消しして書き直ししてる

	for( itNulti = gltMultiFiles.begin( ); gltMultiFiles.end( ) != itNulti; itNulti++ )
	{
		//	記録しないでよろしいか？
		if( StrCmp( DROP_OBJ_NAME, itNulti->atBaseName ) )
		{
			SqlMultiTabInsert( itNulti->atFilePath, itNulti->atBaseName, itNulti->atDispName );
		}
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	複タブをプロファイルから読み込んで再展開する
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

		SqlMultiTabSelect( i+1, stMulti.atFilePath, stMulti.atBaseName, stMulti.atDispName );

		gltMultiFiles.push_back( stMulti );
		TabMultipleAppend( hWnd );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ドラッグンドロッペされたファイルを副タブにしちゃう
	@param[in]	hWnd	元ウインドウハンドル
	@param[in]	ptFile	ドロップされたファイルのパス
	@return		HRESULT	終了状態コード
*/
HRESULT TabMultipleDropAdd( HWND hWnd, LPCTSTR ptFile )
{
	MULTIPLEMAA		stMulti;
	INT		iTabNum;


	ZeroMemory( &stMulti, sizeof(MULTIPLEMAA) );
	StringCchCopy( stMulti.atFilePath, MAX_PATH, ptFile );
	StringCchCopy( stMulti.atBaseName, MAX_PATH, DROP_OBJ_NAME );	//	特殊名称・大丈夫か
	stMulti.dTabNum = 0;	//	初期化・割当は２以降

	gltMultiFiles.push_back( stMulti );
	iTabNum = TabMultipleAppend( hWnd );

	//	その他武を開いちゃう？
	TabCtrl_SetCurSel( ghTabWnd, iTabNum );
	TabMultipleSelect( hWnd, iTabNum, 0 );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	タブを増やす・保持リストにファイル名ぶち込んだら直ちに呼ぶべし
	@param[in]	hWnd	親ウインドウのハンドル
	@return	INT	開いたタブの番号
*/
INT TabMultipleAppend( HWND hWnd )
{
	TCHAR	atName[MAX_PATH];
	LONG	tCount;
//	RECT	itRect;
	TCITEM	stTcItem;

	MLTT_ITR	itNulti;


	itNulti = gltMultiFiles.end( );
	itNulti--;	//	新しく開くのは末端にあるはず
	StringCchCopy( atName, MAX_PATH, itNulti->atFilePath );
	if( NULL !=  atName[0] )	//	ツリーもしくはドラッグンドロッペ
	{
		PathStripPath( atName );	//	ファイル名だけにして
		PathRemoveExtension( atName );	//	拡張子を外す
	}
	else	//	お気にリストから追加する
	{
		StringCchCopy( atName, MAX_PATH, itNulti->atBaseName );
		StringCchCat(  atName, MAX_PATH, TEXT("[F]") );
	}

	if( NULL == itNulti->atDispName[0] )
	{	StringCchCopy( itNulti->atDispName , MAX_PATH, atName );	}	//	表示名デフォルト

	ZeroMemory( &stTcItem, sizeof(TCITEM) );
	stTcItem.mask = TCIF_TEXT | TCIF_PARAM;

	tCount = TabCtrl_GetItemCount( ghTabWnd );

	stTcItem.lParam  = 0;//tCount;ファイルなので０でいい・タブのは未使用
	stTcItem.pszText = itNulti->atDispName;
	TabCtrl_InsertItem( ghTabWnd, tCount, &stTcItem );

	itNulti->dTabNum = tCount;

	Maa_OnSize( hWnd, 0, 0, 0 );	//	引数は使ってなかったか

	return tCount;
}
//-------------------------------------------------------------------------------------------------

/*!
	開いてるヤツの番号を返す。主タブ・使用・副タブ
	@return	INT	開いてる奴の番号
*/
INT TabMultipleNowSel( VOID )
{
	return gixUseTab;	//	ACT_ALLTREE	ACT_FAVLIST
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






/*!
	名称変更ダイヤログボックスのメセージハンドラ・頁名の使い回しなので注意
	@param[in]	hDlg	ダイヤログハンドル
	@param[in]	message	ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@retval 0	メッセージは処理していない
	@retval no0	なんか処理された
*/
INT_PTR CALLBACK TabMultipleRenameDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static LPTSTR	cptName;
	TCHAR	atBuffer[MAX_PATH];

	switch( message )
	{
		case WM_INITDIALOG:
			cptName = (LPTSTR)lParam;	//	元文字列は MAX_PATH であること
			Edit_SetText( GetDlgItem(hDlg,IDE_PAGENAME), cptName );
			SetFocus( GetDlgItem(hDlg,IDE_PAGENAME) );
			return (INT_PTR)FALSE;

		case WM_COMMAND:
			if( IDOK == LOWORD(wParam) )
			{
				Edit_GetText( GetDlgItem(hDlg,IDE_PAGENAME), atBuffer, MAX_PATH );
				StringCchCopy( cptName, MAX_PATH, atBuffer );
				EndDialog( hDlg, IDOK );
				return (INT_PTR)TRUE;
			}

			if( IDCANCEL == LOWORD(wParam) )
			{
				EndDialog( hDlg, IDCANCEL );
				return (INT_PTR)TRUE;
			}

			break;
	}
	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	タブ名前変更の処理・ダイヤログ開いたり変更を記録したり
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	iTabSel	選択したタブ番号
	@return		HRESULT	終了状態コード
*/
HRESULT TabMultipleNameChange( HWND hWnd, INT iTabSel )
{
	INT_PTR	iRslt;
	TCHAR	atName[MAX_PATH];
	TCITEM	stTcItem;
	MLTT_ITR	itNulti;

	for( itNulti = gltMultiFiles.begin( ); gltMultiFiles.end( ) != itNulti; itNulti++ )
	{
		if( iTabSel == itNulti->dTabNum )	//	選択されてるやつをさがす
		{
			StringCchCopy( atName, MAX_PATH, itNulti->atDispName );

			iRslt = DialogBoxParam( GetModuleHandle( NULL ), MAKEINTRESOURCE(IDD_PAGE_NAME_DLG), hWnd, TabMultipleRenameDlgProc, (LPARAM)atName );
			if( IDOK == iRslt )	//	ＯＫしてたら、変更された名前がバッファに入ってるはず
			{
				StringCchCopy( itNulti->atDispName, MAX_PATH, atName );	//	記録変更

				ZeroMemory( &stTcItem, sizeof(TCITEM) );
				stTcItem.mask = TCIF_TEXT;
				stTcItem.pszText = atName;
				TabCtrl_SetItem( ghTabWnd, iTabSel, &stTcItem );

				return S_OK;
			}

			return E_ABORT;
		}
	}

	return E_OUTOFMEMORY;
}
//-------------------------------------------------------------------------------------------------

/*!
	タブの多段表示・一行表示を切り替える
	@param[in]	hWnd	ウインドウハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT TabLineMultiSingleToggle( HWND hWnd )
{
	 INT	doSingle;
	DWORD	dWndwStyle;

	dWndwStyle = GetWindowStyle( ghTabWnd );

	if( TCS_MULTILINE & dWndwStyle )	//	多段モード中・シングルスタイルにする
	{
		doSingle = 1;
		dWndwStyle &= ~TCS_MULTILINE;	//	スタイルを外す
	}
	else	//	シングルモード中・多段スタイルにする
	{
		doSingle = 0;
		dWndwStyle |= TCS_MULTILINE;	//	スタイルをくっつける
	}

	SetWindowLong( ghTabWnd, GWL_STYLE, dWndwStyle );	//	スタイル書き戻す
	//	再描画とか？なくてよさそう
	InitParamValue( INIT_SAVE, VL_MAATAB_SNGL, doSingle );	//	記録

	Maa_OnSize( hWnd, 0, 0, 0 );	//	引数は使ってなかったか

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

