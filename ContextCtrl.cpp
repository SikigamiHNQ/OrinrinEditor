/*! @file
	@brief メインのコンテキストメニューの管理します
	このファイルは ContextCtrl.cpp です。
	@author	SikigamiHNQ
	@date	2011/11/07
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

//	ユーザ定義とか枠とかは、サブメニュー用のダミーアイテムを用意する

typedef struct tagCONTEXTITEM
{
	TCHAR	atString[MIN_STRING];		//!<	表示する文字列
	TCHAR	atAccelerator[MIN_STRING];	//!<	アクセラレータ文字列・なかったら空文字列
	UINT	dCommandoID;				//!<	コマンドＩＤ・0でセパレータ

} CONTEXTITEM, *LPCONTEXTITEM;
//-------------------------------------------------------------------------------------------------




const static CONTEXTITEM	gstContextItem[] =
{
/* 0*/	{  TEXT("新規作成"),	/*ファイル*/	TEXT("Ctrl + N"),		IDM_NEWFILE			},
/* 1*/	{  TEXT("ファイル開く"),				TEXT("Ctrl + O"),		IDM_OPEN			},
		{  TEXT("上書き保存"),					TEXT("Ctrl + S"),		IDM_OVERWRITESAVE	},
		{  TEXT("名前を付けて保存"),			TEXT("Ctrl + Alt + S"),	IDM_RENAMESAVE		},
		{  TEXT("現在頁を画像にして保存"),		TEXT(""),				IDM_IMAGE_SAVE		},
/* 5*/	{  TEXT("一般設定"),					TEXT(""),				IDM_GENERAL_OPTION	},
		{  TEXT("（セパレータ）"),				TEXT(""),				0					},
		{  TEXT("元に戻す"),	/*編集*/		TEXT("Ctrl + Z"),			IDM_UNDO			},
		{  TEXT("やり直し"),					TEXT("Ctrl + Y"),			IDM_REDO			},
		{  TEXT("切り取り"),					TEXT("Ctrl + X"),			IDM_CUT				},
/*10*/	{  TEXT("Unicodeコピー"),				TEXT("Ctrl + C"),			IDM_COPY			},
		{  TEXT("SJISコピー"),					TEXT(""),					IDM_SJISCOPY		},
		{  TEXT("全体をSJISコピー"),			TEXT("Ctrl + Shift + C"),	IDM_SJISCOPY_ALL	},
		{  TEXT("貼付"),						TEXT("Ctrl + V"),			IDM_PASTE			},
		{  TEXT("削除"),						TEXT("Delete"),				IDM_DELETE			},
/*15*/	{  TEXT("全選択"),						TEXT("Ctrl + A"),			IDM_ALLSEL			},
		{  TEXT("矩形選択"),					TEXT("Ctrl + Alt + B"),		IDM_SQSELECT		},
		{  TEXT("レイヤボックス"),				TEXT("Alt + Space"),		IDM_LAYERBOX		},
		{  TEXT("部分抽出モード"),				TEXT(""),					IDM_EXTRACTION_MODE	},
		{  TEXT("次の行以降を新頁に分割"),		TEXT(""),					IDM_PAGEL_DIVIDE	},
/*20*/	{  TEXT("（セパレータ）"),				TEXT(""),					0					},
		{  TEXT("ユニコード空白"),	/*挿入*/	TEXT(""),	IDM_MN_UNISPACE		},//サブメニューダミー
		{  TEXT(" 1 dot SPACE"),				TEXT(""),	IDM_IN_01SPACE		},
		{  TEXT(" 2 dot SPACE"),				TEXT(""),	IDM_IN_02SPACE		},
		{  TEXT(" 3 dot SPACE"),				TEXT(""),	IDM_IN_03SPACE		},
/*25*/	{  TEXT(" 4 dot SPACE"),				TEXT(""),	IDM_IN_04SPACE		},
		{  TEXT(" 5 dot SPACE"),				TEXT(""),	IDM_IN_05SPACE		},
		{  TEXT(" 8 dot SPACE"),				TEXT(""),	IDM_IN_08SPACE		},
		{  TEXT("10 dot SPACE"),				TEXT(""),	IDM_IN_10SPACE		},
		{  TEXT("16 dot SPACE"),				TEXT(""),	IDM_IN_16SPACE		},
/*30*/	{  TEXT("色指定"),						TEXT(""),	IDM_MN_COLOUR_SEL	},//サブメニューダミー
		{  TEXT("白"),							TEXT(""),	IDM_INSTAG_WHITE	},
		{  TEXT("青"),							TEXT(""),	IDM_INSTAG_BLUE		},
		{  TEXT("黒"),							TEXT(""),	IDM_INSTAG_BLACK	},
		{  TEXT("赤"),							TEXT(""),	IDM_INSTAG_RED		},
/*35*/	{  TEXT("緑"),							TEXT(""),	IDM_INSTAG_GREEN	},
		{  TEXT("枠選択"),						TEXT(""),			IDM_MN_INSFRAME_SEL		},//サブメニューダミー
		{  TEXT("枠（１）"),					TEXT(""),			IDM_INSFRAME_ALPHA		},
		{  TEXT("枠（２）"),					TEXT(""),			IDM_INSFRAME_BRAVO		},
		{  TEXT("枠（３）"),					TEXT(""),			IDM_INSFRAME_CHARLIE	},
/*40*/	{  TEXT("枠（４）"),					TEXT(""),			IDM_INSFRAME_DELTA		},
		{  TEXT("枠（５）"),					TEXT(""),			IDM_INSFRAME_ECHO		},
		{  TEXT("枠（６）"),					TEXT(""),			IDM_INSFRAME_FOXTROT	},
		{  TEXT("枠（７）"),					TEXT(""),			IDM_INSFRAME_GOLF		},
		{  TEXT("枠（８）"),					TEXT(""),			IDM_INSFRAME_HOTEL		},
/*45*/	{  TEXT("枠（９）"),					TEXT(""),			IDM_INSFRAME_INDIA		},
		{  TEXT("枠（１０）"),					TEXT(""),			IDM_INSFRAME_JULIETTE	},
		{  TEXT("枠編集"),						TEXT(""),			IDM_INSFRAME_EDIT		},
		{  TEXT("枠挿入ボックス"),				TEXT("Ctrl + W"),	IDM_FRMINSBOX_OPEN		},
		{  TEXT("文字ＡＡ変換ボックス"),		TEXT(""),			IDM_MOZI_SCR_OPEN		},
/*50*/	{  TEXT("ユーザ定義"),					TEXT(""),			IDM_MN_USER_REFS		},//サブメニューダミー
		{  TEXT("各頁に通し番号を入れる"),		TEXT(""),			IDM_PAGENUM_DLG_OPEN	},
		{  TEXT("（セパレータ）"),				TEXT(""),			0						},
		{  TEXT("右揃え線"),	/*整形*/		TEXT("Ctrl + Alt + R"),		IDM_RIGHT_GUIDE_SET	},
		{  TEXT("行頭に全角空白追加"),			TEXT("Ctrl + I"),			IDM_INS_TOPSPACE	},
/*55*/	{  TEXT("行頭空白削除"),				TEXT("Ctrl + U"),			IDM_DEL_TOPSPACE	},
		{  TEXT("行末空白削除"),				TEXT("Ctrl + G"),			IDM_DEL_LASTSPACE	},
		{  TEXT("行末文字削除"),				TEXT("Ctrl + Shift + G"),	IDM_DEL_LASTLETTER	},
		{  TEXT("選択範囲を空白にする"),		TEXT("Ctrl + Alt + G"),		IDM_FILL_SPACE		},
		{  TEXT("頁全体を空白で埋める"),		TEXT(""),					IDM_FILL_ZENSP		},
/*60*/	{  TEXT("右に寄せる"),					TEXT(""),					IDM_RIGHT_SLIDE		},
		{  TEXT("１ドット増やす"),				TEXT("Alt + →"),			IDM_INCREMENT_DOT	},
		{  TEXT("１ドット減らす"),				TEXT("Alt + ←"),			IDM_DECREMENT_DOT	},
		{  TEXT("全体を１ドット右へ"),			TEXT("Alt + Shift + →"),	IDM_INCR_DOT_LINES	},
		{  TEXT("全体を１ドット左へ"),			TEXT("Alt + Shift + ←"),	IDM_DECR_DOT_LINES	},
/*65*/	{  TEXT("調整基準ロック"),				TEXT("Ctrl + Alt + W"),		IDM_DOTDIFF_LOCK	},
		{  TEXT("カーソル位置で調整"),			TEXT("Ctrl + Alt + E"),		IDM_DOTDIFF_ADJT	},
		{  TEXT("（セパレータ）"),				TEXT(""),					0					},
		{  TEXT("空白を表示"),	/*表示*/		TEXT(""),	IDM_SPACE_VIEW_TOGGLE	},
		{  TEXT("グリッド線を表示"),			TEXT(""),	IDM_GRID_VIEW_TOGGLE	},
/*70*/	{  TEXT("右ガイド線を表示"),			TEXT(""),	IDM_RIGHT_RULER_TOGGLE	},
		{  TEXT("複数行テンプレート"),			TEXT(""),	IDM_MAATMPLE_VIEW		},
		{  TEXT("頁一覧"),						TEXT(""),	IDM_PAGELIST_VIEW		},
		{  TEXT("壱行テンプレート"),			TEXT(""),	IDM_LINE_TEMPLATE		},
		{  TEXT("塗り潰しブラシ"),				TEXT(""),	IDM_BRUSH_PALETTE		},
/*75*/	{  TEXT("ユニコード表"),				TEXT(""),	IDM_UNI_PALETTE			},
		{  TEXT("トレスモード"),				TEXT(""),	IDM_TRACE_MODE_ON		},
		{  TEXT("プレビュー"),					TEXT(""),	IDM_ON_PREVIEW			},
		{  TEXT("ドラフトボードを開く"),		TEXT("Ctrl + Space"),	IDM_DRAUGHT_OPEN	},
		{  TEXT("サムネイルを表示"),			TEXT("Ctrl + T"),		IDM_MAA_THUMBNAIL_OPEN	},
/*80*/	{  TEXT("（セパレータ）"),				TEXT(""),	0						},
		{  TEXT("選択範囲をドラフトボードへ"),	TEXT(""),	IDM_COPY_TO_DRAUGHT		}
};
#define ALL_ITEMS	82



//初期設定
#define DEF_ITEMS	32
const static UINT	gadDefItem[] =
{
	IDM_CUT,				//	切り取り
	IDM_COPY,				//	Unicodeコピー
	IDM_PASTE,				//	貼付
	IDM_ALLSEL,				//	全選択
	0,
	IDM_COPY_TO_DRAUGHT,	//	選択範囲をドラフトボードへ
	0,
	IDM_SJISCOPY,			//	SJISコピー
	IDM_SJISCOPY_ALL,		//	全体をSJISコピー
	0,
	IDM_SQSELECT,			//	矩形選択
	0,
	IDM_LAYERBOX,			//	レイヤボックス
	IDM_FRMINSBOX_OPEN,		//	枠挿入ボックス
	IDM_MOZI_SCR_OPEN,		//	文字ＡＡ変換ボックス
	0,
	IDM_RIGHT_GUIDE_SET,	//	右揃え線
	IDM_INS_TOPSPACE,		//	行頭に全角空白追加
	IDM_DEL_TOPSPACE,		//	行頭空白削除
	IDM_DEL_LASTSPACE,		//	行末空白削除
	IDM_DEL_LASTLETTER,		//	行末文字削除
	IDM_FILL_SPACE,			//	選択範囲を空白にする
	IDM_RIGHT_SLIDE,		//	右に寄せる
	0,
	IDM_INCR_DOT_LINES,		//	全体を１ドット右へ
	IDM_DECR_DOT_LINES,		//	全体を１ドット左へ
	0,
	IDM_SPACE_VIEW_TOGGLE,	//	空白を表示	
	IDM_GRID_VIEW_TOGGLE,	//	グリッド線を表示
	IDM_RIGHT_RULER_TOGGLE,	//	右ガイド線を表示
	0,
	IDM_PAGEL_DIVIDE		//	次の行以降を新頁に分割
};
//-------------------------------------------------------------------------------------------------








extern  UINT	gbCpModSwap;	//!<	SJISとユニコードコピーを入れ替える

static HINSTANCE	ghInst;
static TCHAR		gatCntxIni[MAX_PATH];	//!<	

static HMENU	ghPopupMenu;	
static HMENU	ghUniSpMenu;
static HMENU	ghColourMenu;
static HMENU	ghFrameMenu;
static HMENU	ghUsrDefMenu;

static vector<CONTEXTITEM>	gvcCntxItem;
typedef vector<CONTEXTITEM>::iterator	CTXI_VITR;

static list<CONTEXTITEM>	gltCntxEdit;
typedef list<CONTEXTITEM>::iterator	CTXI_LITR;
//-------------------------------------------------------------------------------------------------

VOID	CntxEditBuild( VOID );

INT_PTR	CALLBACK CntxEditDlgProc( HWND, UINT, WPARAM, LPARAM );

VOID	CntxDlgLvInit( HWND );
VOID	CntxDlgAllListUp( HWND );
VOID	CntxDlgBuildListUp( HWND );

VOID	CntxDlgItemAdd( HWND );
VOID	CntxDlgItemDel( HWND );
VOID	CntxDlgItemSpinUp( HWND );
VOID	CntxDlgItemSpinDown( HWND );
//-------------------------------------------------------------------------------------------------

/*!
	起動時初期化・INIディレクトリ格納とか初期生成とか
	@param[in]	ptCurrent	基準ディレクトリ
	@param[in]	hInstance	このアプリの実存
	@return		HRESULT		終了状態コード
*/
HRESULT CntxEditInitialise( LPTSTR ptCurrent, HINSTANCE hInstance )
{
	UINT	dCount, cid;
	UINT	ams, ims;
	TCHAR	atKeyName[MIN_STRING];



	if( !(ptCurrent) || !(hInstance) )
	{
		if( ghPopupMenu ){	DestroyMenu( ghPopupMenu  );	}
		if( ghUniSpMenu ){	DestroyMenu( ghUniSpMenu  );	}
		if( ghColourMenu ){	DestroyMenu( ghColourMenu );	}
		if( ghFrameMenu ){	DestroyMenu( ghFrameMenu  );	}
		if( ghUsrDefMenu ){	DestroyMenu( ghUsrDefMenu );	}

		return S_OK;
	}



	ghInst = hInstance;

	StringCchCopy( gatCntxIni, MAX_PATH, ptCurrent );
	PathAppend( gatCntxIni, MZCX_INI_FILE );


	gvcCntxItem.clear();

	//	初期状態
	dCount = GetPrivateProfileInt( TEXT("Context"), TEXT("Count"), 0, gatCntxIni );
	if( 1 <= dCount )	//	データ有り
	{
		for( ims = 0; dCount > ims; ims++ )
		{
			StringCchPrintf( atKeyName, MIN_STRING, TEXT("CmdID%u"), ims );
			cid = GetPrivateProfileInt( TEXT("Context"), atKeyName, 0, gatCntxIni );

			for( ams = 0; ALL_ITEMS > ams; ams++ )
			{
				if( cid == gstContextItem[ams].dCommandoID )
				{
					gvcCntxItem.push_back( gstContextItem[ams] );
					break;
				}
			}
		}
	}
	else	//	データ無しならデフォメニューを構築
	{
		for( ims = 0; DEF_ITEMS > ims; ims++ )
		{
			for( ams = 0; ALL_ITEMS > ams; ams++ )
			{
				if( gadDefItem[ims] == gstContextItem[ams].dCommandoID )
				{
					gvcCntxItem.push_back( gstContextItem[ams] );
					break;
				}
			}
		}
	}

	CntxEditBuild(  );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	できあがってるコンテキストメニューを外部で使う
*/
HMENU CntxMenuGet( VOID )
{
	return ghPopupMenu;
}
//-------------------------------------------------------------------------------------------------

/*!
	コンテキストメニューを組み立てる。
*/
VOID CntxEditBuild( VOID )
{
	UINT	d, e;
	TCHAR	atItem[SUB_STRING], atKey[MIN_STRING];
	CTXI_VITR	itMnItm;

	if( ghPopupMenu ){	DestroyMenu( ghPopupMenu  );	}	ghPopupMenu  = NULL;
	if( ghUniSpMenu ){	DestroyMenu( ghUniSpMenu  );	}	ghUniSpMenu  = NULL;
	if( ghColourMenu ){	DestroyMenu( ghColourMenu );	}	ghColourMenu = NULL;
	if( ghFrameMenu ){	DestroyMenu( ghFrameMenu  );	}	ghFrameMenu  = NULL;
	if( ghUsrDefMenu ){	DestroyMenu( ghUsrDefMenu );	}	ghUsrDefMenu = NULL;

	ghPopupMenu = CreatePopupMenu(  );

	e = 0;
	for( itMnItm = gvcCntxItem.begin(); gvcCntxItem.end() != itMnItm; itMnItm++ )
	{
		if( 0 >= itMnItm->dCommandoID )
		{
			AppendMenu( ghPopupMenu, MF_SEPARATOR, 0, NULL );
		}
		else
		{
			StringCchCopy( atItem, SUB_STRING, itMnItm->atString );

			if( gbCpModSwap )	//	コピーモード入替
			{
				if( IDM_COPY == itMnItm->dCommandoID )		StringCchCopy( atItem, SUB_STRING, TEXT("SJISコピ－") );
				if( IDM_SJISCOPY == itMnItm->dCommandoID )	StringCchCopy( atItem, SUB_STRING, TEXT("Unicodeコピ－") );
			}

			if( 26 > e )
			{
				StringCchPrintf( atKey, MIN_STRING, TEXT("(&%c)"), 'A' + e );
				StringCchCat( atItem, SUB_STRING, atKey );
				e++;
			}

			switch( itMnItm->dCommandoID )
			{
				default:	AppendMenu( ghPopupMenu, MF_STRING, itMnItm->dCommandoID, atItem );	break;

#pragma message ("サブメニューアイテムのサーチ、キメうちでやらないようにする")
				case IDM_MN_UNISPACE:
					ghUniSpMenu = CreatePopupMenu(  );
					for( d = 0; 8 > d; d++ )	AppendMenu( ghUniSpMenu, MF_STRING, gstContextItem[22+d].dCommandoID, gstContextItem[22+d].atString );
					AppendMenu( ghPopupMenu, MF_POPUP, (UINT_PTR)ghUniSpMenu, atItem );
					break;

				case IDM_MN_COLOUR_SEL:
					ghColourMenu = CreatePopupMenu(  );
					for( d = 0; 5 > d; d++ )	AppendMenu( ghColourMenu, MF_STRING, gstContextItem[31+d].dCommandoID, gstContextItem[31+d].atString );
					AppendMenu( ghPopupMenu, MF_POPUP, (UINT_PTR)ghColourMenu, atItem );
					break;

				case IDM_MN_INSFRAME_SEL:
					ghFrameMenu = CreatePopupMenu(  );
					for( d = 0; 10 > d; d++ )	AppendMenu( ghFrameMenu, MF_STRING, gstContextItem[37+d].dCommandoID, gstContextItem[37+d].atString );
					AppendMenu( ghPopupMenu, MF_POPUP, (UINT_PTR)ghFrameMenu, atItem );
					break;

				case IDM_MN_USER_REFS:
					ghUsrDefMenu = CreatePopupMenu(  );
					UserDefMenuWrite( ghUsrDefMenu );
					AppendMenu( ghPopupMenu, MF_POPUP, (UINT_PTR)ghUsrDefMenu, atItem );
					break;
			}
		}
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	コンテキストメニューのSJISとユニコードのアレを入れ替える
*/
HRESULT CntxMenuCopySwap( VOID )
{
	CntxEditBuild(  );	//	ていうかBuildし直しでいい

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	編集ダイヤログ開く
	@param[in]	hWnd	ウインドウハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT CntxEditDlgOpen( HWND hWnd )
{
	INT_PTR	iRslt, i;
	TCHAR	atKeyName[MIN_STRING], atBuff[MIN_STRING];
	CTXI_LITR	itEdit;

	iRslt = DialogBoxParam( ghInst, MAKEINTRESOURCE(IDD_CONTEXT_ITEM_DLG), hWnd, CntxEditDlgProc, 0 );
	if( IDOK == iRslt )
	{
		//	一旦セクションを空にする
		ZeroMemory( atBuff, sizeof(atBuff) );
		WritePrivateProfileSection( TEXT("Context"), atBuff, gatCntxIni );

		gvcCntxItem.clear();	i = 0;
		for( itEdit = gltCntxEdit.begin(); gltCntxEdit.end() != itEdit; itEdit++ )
		{
			StringCchPrintf( atKeyName, MIN_STRING, TEXT("CmdID%u"), i );
			StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), itEdit->dCommandoID );
			WritePrivateProfileString( TEXT("Context"), atKeyName, atBuff, gatCntxIni );

			gvcCntxItem.push_back( *itEdit );
			i++;
		}

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), i );
		WritePrivateProfileString( TEXT("Context"), TEXT("Count"), atBuff, gatCntxIni );

		CntxEditBuild(  );

		return S_OK;
	}

	return E_ABORT;
}
//-------------------------------------------------------------------------------------------------

/*!
	編集ダイヤログボックスのメセージハンドラだってばよ
	@param[in]	hDlg	ダイヤログハンドル
	@param[in]	message	ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@retval 0	メッセージは処理していない
	@retval no0	なんか処理された
*/
INT_PTR CALLBACK CntxEditDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	INT		id;
	HWND	hWndCtl;
	UINT	codeNotify;

	CTXI_VITR	itMnItm;


	switch( message )
	{
		case WM_INITDIALOG:
			gltCntxEdit.clear();
			for( itMnItm = gvcCntxItem.begin(); gvcCntxItem.end() != itMnItm; itMnItm++ ){	gltCntxEdit.push_back( *itMnItm );	}
			CntxDlgLvInit( hDlg );
			CntxDlgAllListUp( hDlg );
			CntxDlgBuildListUp( hDlg );
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			id         = LOWORD(wParam);	//	メッセージを発生させた子ウインドウの識別子
			hWndCtl    = (HWND)lParam;		//	メッセージを発生させた子ウインドウのハンドル
			codeNotify = HIWORD(wParam);	//	通知メッセージ
			switch( id )
			{
				case IDOK:		EndDialog( hDlg, IDOK );		return (INT_PTR)TRUE;
				case IDCANCEL:	EndDialog( hDlg, IDCANCEL );	return (INT_PTR)TRUE;

				case IDB_MENUITEM_ADD:		CntxDlgItemAdd( hDlg );	return (INT_PTR)TRUE;
				case IDB_MENUITEM_DEL:		CntxDlgItemDel( hDlg );	return (INT_PTR)TRUE;

				case IDB_MENUITEM_SPINUP:	CntxDlgItemSpinUp( hDlg );		return (INT_PTR)TRUE;
				case IDB_MENUITEM_SPINDOWN:	CntxDlgItemSpinDown( hDlg );	return (INT_PTR)TRUE;

				default:	break;
			}
			break;
	}

	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	アイテムのリストビュー作成
	@param[in]	hDlg	ダイヤログハンドル
*/
VOID CntxDlgLvInit( HWND hDlg )
{
	HWND	hLvWnd;
	LVCOLUMN	stLvColm;
	RECT	rect;


	hLvWnd = GetDlgItem( hDlg, IDLV_MENU_ALLITEM );
	GetClientRect( hLvWnd, &rect );

	ListView_SetExtendedListViewStyle( hLvWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_LABELTIP );

	ZeroMemory( &stLvColm, sizeof(LVCOLUMN) );
	stLvColm.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	stLvColm.fmt      = LVCFMT_LEFT;
	stLvColm.iSubItem = 0;

	stLvColm.pszText  = TEXT("メニューアイテム");
	stLvColm.cx       = rect.right - 23;
	ListView_InsertColumn( hLvWnd, 0, &stLvColm );


	hLvWnd = GetDlgItem( hDlg, IDLV_MENU_BUILDX );
	GetClientRect( hLvWnd, &rect );

	ListView_SetExtendedListViewStyle( hLvWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_LABELTIP );

	stLvColm.cx       = rect.right - 23;
	ListView_InsertColumn( hLvWnd, 0, &stLvColm );

}
//-------------------------------------------------------------------------------------------------

/*!
	全アイテムをリストビューにブッこむ
	@param[in]	hDlg	ダイヤログハンドル
*/
VOID CntxDlgAllListUp( HWND hDlg )
{
	HWND	hLvWnd;
	UINT	d;
	LVITEM	stLvi;
	TCHAR	atItem[SUB_STRING];


	hLvWnd = GetDlgItem( hDlg, IDLV_MENU_ALLITEM );

	ListView_DeleteAllItems( hLvWnd );

	ZeroMemory( &stLvi, sizeof(stLvi) );
	stLvi.mask    = LVIF_TEXT;
	stLvi.pszText = atItem;

	for( d = 0; ALL_ITEMS > d; d++ )
	{
		StringCchCopy( atItem, SUB_STRING, gstContextItem[d].atString );

		if( IDM_MN_UNISPACE == gstContextItem[d].dCommandoID || 
		IDM_MN_COLOUR_SEL   == gstContextItem[d].dCommandoID || 
		IDM_MN_INSFRAME_SEL == gstContextItem[d].dCommandoID || 
		IDM_MN_USER_REFS    == gstContextItem[d].dCommandoID )
		{
			StringCchCat( atItem, SUB_STRING, TEXT("（サブメニュー展開）") );
		}

		if( gbCpModSwap )	//	コピーモード入替
		{
			if( IDM_COPY     ==  gstContextItem[d].dCommandoID )	StringCchCopy( atItem, SUB_STRING, TEXT("SJISコピ－") );
			if( IDM_SJISCOPY ==  gstContextItem[d].dCommandoID )	StringCchCopy( atItem, SUB_STRING, TEXT("Unicodeコピ－") );
		}

		stLvi.iItem = d;
		ListView_InsertItem( hLvWnd, &stLvi );
	}
}
//-------------------------------------------------------------------------------------------------

/*!
	メニュー構成をリストビューに表示
	@param[in]	hDlg	ダイヤログハンドル
*/
VOID CntxDlgBuildListUp( HWND hDlg )
{
	HWND	hLvWnd;
	UINT	d;
	LVITEM	stLvi;
	TCHAR	atItem[SUB_STRING];

	CTXI_LITR	itMnItm;


	hLvWnd = GetDlgItem( hDlg, IDLV_MENU_BUILDX );

	ListView_DeleteAllItems( hLvWnd );

	ZeroMemory( &stLvi, sizeof(stLvi) );
	stLvi.mask    = LVIF_TEXT;
	stLvi.pszText = atItem;

	for( itMnItm = gltCntxEdit.begin(), d = 0; gltCntxEdit.end() != itMnItm; itMnItm++, d++ )
	{
		stLvi.iItem = d;
		if( 0 >= itMnItm->dCommandoID )
		{
			StringCchCopy( atItem, SUB_STRING, TEXT("---------------") );
		}
		else
		{
			StringCchCopy( atItem, SUB_STRING, itMnItm->atString );

			if( IDM_MN_UNISPACE == itMnItm->dCommandoID || 
			IDM_MN_COLOUR_SEL   == itMnItm->dCommandoID || 
			IDM_MN_INSFRAME_SEL == itMnItm->dCommandoID || 
			IDM_MN_USER_REFS    == itMnItm->dCommandoID )
			{
				StringCchCat( atItem, SUB_STRING, TEXT("　　[＞") );
			}

			if( gbCpModSwap )	//	コピーモード入替
			{
				if( IDM_COPY     == itMnItm->dCommandoID )	StringCchCopy( atItem, SUB_STRING, TEXT("SJISコピ－") );
				if( IDM_SJISCOPY == itMnItm->dCommandoID )	StringCchCopy( atItem, SUB_STRING, TEXT("Unicodeコピ－") );
			}
		}

		ListView_InsertItem( hLvWnd, &stLvi );
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	アイテムを追加
	@param[in]	hDlg	ダイヤログハンドル
*/
VOID CntxDlgItemAdd( HWND hDlg )
{
	HWND	hListWnd, hBuildWnd;
	INT		iSel, iIns, iCount;
	CTXI_LITR	itMnItm;

	hListWnd  = GetDlgItem( hDlg, IDLV_MENU_ALLITEM );
	hBuildWnd = GetDlgItem( hDlg, IDLV_MENU_BUILDX );

	//	元のほう
	iSel = ListView_GetNextItem( hListWnd, -1, LVNI_ALL | LVNI_SELECTED );
	if( 0 > iSel )	 return;	//	選択してなかったら終わり

	iCount = ListView_GetItemCount( hBuildWnd );


	//	選択したヤツの次にいれる・未選択か末尾なら末尾へ
	iIns = ListView_GetNextItem( hBuildWnd, -1, LVNI_ALL | LVNI_SELECTED );
	if( (0 > iIns) || ((iIns+1) >= iCount) ){	gltCntxEdit.push_back( gstContextItem[iSel] );	}
	else
	{
		itMnItm = gltCntxEdit.begin();
		advance( itMnItm, iIns+1 );
		//	イテレータの直前に入る
		gltCntxEdit.insert( itMnItm, gstContextItem[iSel] );
	}



	CntxDlgBuildListUp( hDlg );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	アイテムを削除
	@param[in]	hDlg	ダイヤログハンドル
*/
VOID CntxDlgItemDel( HWND hDlg )
{
	HWND	hListWnd, hBuildWnd;
	INT		iSel;
	CTXI_LITR	itMnItm;

	hListWnd  = GetDlgItem( hDlg, IDLV_MENU_ALLITEM );
	hBuildWnd = GetDlgItem( hDlg, IDLV_MENU_BUILDX );

	iSel = ListView_GetNextItem( hBuildWnd, -1, LVNI_ALL | LVNI_SELECTED );
	if( 0 > iSel )	return;	//	選択してなかったら終わり

	itMnItm = gltCntxEdit.begin();
	advance( itMnItm, iSel );

	gltCntxEdit.erase( itMnItm );

	CntxDlgBuildListUp( hDlg );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	アイテムを↑へ
	@param[in]	hDlg	ダイヤログハンドル
*/
VOID CntxDlgItemSpinUp( HWND hDlg )
{
	HWND	hBuildWnd;
	INT		iSel;
	CTXI_LITR	itTgtItm, itSwpItm;
	CONTEXTITEM	stItem;

	hBuildWnd = GetDlgItem( hDlg, IDLV_MENU_BUILDX );

	iSel = ListView_GetNextItem( hBuildWnd, -1, LVNI_ALL | LVNI_SELECTED );
	if( 0 >= iSel ){	 return;	}	//	選択してないか一番上なら終わり

	itTgtItm = gltCntxEdit.begin();
	advance( itTgtItm, iSel );
	itSwpItm = itTgtItm;
	itSwpItm--;	//	入れる先は一つ前

	StringCchCopy( stItem.atString, MIN_STRING, itTgtItm->atString );
	StringCchCopy( stItem.atAccelerator, MIN_STRING, itTgtItm->atAccelerator );
	stItem.dCommandoID = itTgtItm->dCommandoID;

	gltCntxEdit.erase( itTgtItm );
	gltCntxEdit.insert( itSwpItm, stItem );

	CntxDlgBuildListUp( hDlg );

	ListView_SetItemState( hBuildWnd, --iSel, LVIS_SELECTED, LVIS_SELECTED );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	アイテムを↓へ
	@param[in]	hDlg	ダイヤログハンドル
*/
VOID CntxDlgItemSpinDown( HWND hDlg )
{
	HWND	hBuildWnd;
	INT		iSel, iCount;
	CTXI_LITR	itTgtItm, itSwpItm;
	CONTEXTITEM	stItem;

	hBuildWnd = GetDlgItem( hDlg, IDLV_MENU_BUILDX );

	iSel = ListView_GetNextItem( hBuildWnd, -1, LVNI_ALL | LVNI_SELECTED );
	if( 0 > iSel )	return;	//	選択してなかったら終わり

	iCount = ListView_GetItemCount( hBuildWnd );
	if( iSel >= (iCount-1) )	 return;	//	末端なら終わり

	//	処理の位置関係に注意
	itSwpItm = gltCntxEdit.begin();
	advance( itSwpItm, iSel );
	itTgtItm = itSwpItm;
	itTgtItm++;	//	入れる先は一つ次

	StringCchCopy( stItem.atString, MIN_STRING, itTgtItm->atString );
	StringCchCopy( stItem.atAccelerator, MIN_STRING, itTgtItm->atAccelerator );
	stItem.dCommandoID = itTgtItm->dCommandoID;

	gltCntxEdit.erase( itTgtItm );
	gltCntxEdit.insert( itSwpItm, stItem );

	CntxDlgBuildListUp( hDlg );

	ListView_SetItemState( hBuildWnd, ++iSel, LVIS_SELECTED, LVIS_SELECTED );

	return;
}
//-------------------------------------------------------------------------------------------------

