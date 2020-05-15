/*! @file
	@brief メインのコンテキストメニューの管理します
	このファイルは ContextCtrl.cpp です。
	@author	SikigamiHNQ
	@date	2011/11/07
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

//	ユーザ定義とか枠とかは、サブメニュー用のダミーアイテムを用意する

typedef struct tagCONTEXTITEM
{
	TCHAR	atString[MIN_STRING];		//!<	表示する文字列
	UINT	dCommandoID;				//!<	コマンドＩＤ・0でセパレータ

} CONTEXTITEM, *LPCONTEXTITEM;
//-------------------------------------------------------------------------------------------------


//


CONST static CONTEXTITEM	gstContextItem[] =
{
/* 0*/	{  TEXT("新建文件"),	/*ファイル*/				IDM_NEWFILE				},
/* 1*/	{  TEXT("打开文件"),							IDM_OPEN				},
		{  TEXT("保存"),								IDM_OVERWRITESAVE		},
		{  TEXT("另存为"),							IDM_RENAMESAVE			},
		{  TEXT("保存为图片"),						IDM_IMAGE_SAVE			},
/* 5*/	{  TEXT("一般设定"),							IDM_GENERAL_OPTION		},
		{  TEXT("（セパレータ）"),						0						},
		{  TEXT("撤销动作"),	/*編集*/					IDM_UNDO				},
		{  TEXT("还原动作"),							IDM_REDO				},
		{  TEXT("剪切"),								IDM_CUT					},
/*10*/	{  TEXT("复制为Unicode"),						IDM_COPY				},
		{  TEXT("复制为SJIS"),						IDM_SJISCOPY			},
		{  TEXT("全部复制为SJIS"),					IDM_SJISCOPY_ALL		},
		{  TEXT("粘贴"),								IDM_PASTE				},
		{  TEXT("删除"),								IDM_DELETE				},
/*15*/	{  TEXT("全选"),								IDM_ALLSEL				},
		{  TEXT("矩阵选择"),							IDM_SQSELECT			},
		{  TEXT("矩阵粘贴"),							IDM_SQUARE_PASTE		},
		{  TEXT("AA嵌入工具"),						IDM_LAYERBOX			},
		{  TEXT("部分提取模式"),						IDM_EXTRACTION_MODE		},
/*20*/	{  TEXT("下一行开始另起一页"),					IDM_PAGEL_DIVIDE		},
		{  TEXT("将选择文字列设为页名"),				IDM_PAGENAME_SELASSIGN	},
		{  TEXT("（セパレータ）"),						0						},
		{  TEXT("UNICODE空格"),	/*挿入*/				IDM_MN_UNISPACE			},//サブメニューダミー
		{  TEXT("1像素空格"),							IDM_IN_01SPACE			},
/*25*/	{  TEXT("2像素空格"),							IDM_IN_02SPACE			},
		{  TEXT("3像素空格"),							IDM_IN_03SPACE			},
		{  TEXT("4像素空格"),							IDM_IN_04SPACE			},
		{  TEXT("5像素空格"),							IDM_IN_05SPACE			},
		{  TEXT("8像素空格"),							IDM_IN_08SPACE			},
/*30*/	{  TEXT("10像素空格"),						IDM_IN_10SPACE			},
		{  TEXT("16像素空格"),						IDM_IN_16SPACE			},
		{  TEXT("指定颜色"),							IDM_MN_COLOUR_SEL		},//サブメニューダミー
		{  TEXT("白"),								IDM_INSTAG_WHITE		},
		{  TEXT("蓝"),								IDM_INSTAG_BLUE			},
/*35*/	{  TEXT("黑"),								IDM_INSTAG_BLACK		},
		{  TEXT("红"),								IDM_INSTAG_RED			},
		{  TEXT("绿"),								IDM_INSTAG_GREEN		},
		{  TEXT("选择边框"),							IDM_MN_INSFRAME_SEL		},//サブメニューダミー
		{  TEXT("边框（１）"),						IDM_INSFRAME_ALPHA		},
/*40*/	{  TEXT("边框（２）"),						IDM_INSFRAME_BRAVO		},
		{  TEXT("边框（３）"),						IDM_INSFRAME_CHARLIE	},
		{  TEXT("边框（４）"),						IDM_INSFRAME_DELTA		},
		{  TEXT("边框（５）"),						IDM_INSFRAME_ECHO		},
		{  TEXT("边框（６）"),						IDM_INSFRAME_FOXTROT	},
/*45*/	{  TEXT("边框（７）"),						IDM_INSFRAME_GOLF		},
		{  TEXT("边框（８）"),						IDM_INSFRAME_HOTEL		},
		{  TEXT("边框（９）"),						IDM_INSFRAME_INDIA		},
		{  TEXT("边框（１０）"),						IDM_INSFRAME_JULIETTE	},
		{  TEXT("边框（１１）"),						IDM_INSFRAME_KILO		},
/*50*/	{  TEXT("边框（１２）"),						IDM_INSFRAME_LIMA		},
		{  TEXT("边框（１３）"),						IDM_INSFRAME_MIKE		},
		{  TEXT("边框（１４）"),						IDM_INSFRAME_NOVEMBER	},
		{  TEXT("边框（１５）"),						IDM_INSFRAME_OSCAR		},
		{  TEXT("边框（１６）"),						IDM_INSFRAME_PAPA		},
/*55*/	{  TEXT("边框（１７）"),						IDM_INSFRAME_QUEBEC		},
		{  TEXT("边框（１８）"),						IDM_INSFRAME_ROMEO		},
		{  TEXT("边框（１９）"),						IDM_INSFRAME_SIERRA		},
		{  TEXT("边框（２０）"),						IDM_INSFRAME_TANGO		},
		{  TEXT("编辑边框"),							IDM_INSFRAME_EDIT		},
/*60*/	{  TEXT("边框插入工具"),						IDM_FRMINSBOX_OPEN		},
		{  TEXT("文字AA化工具"),						IDM_MOZI_SCR_OPEN		},
		{  TEXT("纵排版工具"),						IDM_VERT_SCRIPT_OPEN	},
		{  TEXT("用户自定义原件"),						IDM_MN_USER_REFS		},//サブメニューダミー
		{  TEXT("用户自定义原件（１）"),				IDM_USER_ITEM_ALPHA		},
/*65*/	{  TEXT("用户自定义原件（２）"),				IDM_USER_ITEM_BRAVO		},
		{  TEXT("用户自定义原件（３）"),				IDM_USER_ITEM_CHARLIE	},
		{  TEXT("用户自定义原件（４）"),				IDM_USER_ITEM_DELTA		},
		{  TEXT("用户自定义原件（５）"),				IDM_USER_ITEM_ECHO		},
		{  TEXT("用户自定义原件（６）"),				IDM_USER_ITEM_FOXTROT	},
/*70*/	{  TEXT("用户自定义原件（７）"),				IDM_USER_ITEM_GOLF		},
		{  TEXT("用户自定义原件（８）"),				IDM_USER_ITEM_HOTEL		},
		{  TEXT("用户自定义原件（９）"),				IDM_USER_ITEM_INDIA		},
		{  TEXT("用户自定义原件（１０）"),				IDM_USER_ITEM_JULIETTE	},
		{  TEXT("用户自定义原件（１１）"),				IDM_USER_ITEM_KILO		},
/*75*/	{  TEXT("用户自定义原件（１２）"),				IDM_USER_ITEM_LIMA		},
		{  TEXT("用户自定义原件（１３）"),				IDM_USER_ITEM_MIKE		},
		{  TEXT("用户自定义原件（１４）"),				IDM_USER_ITEM_NOVEMBER	},
		{  TEXT("用户自定义原件（１５）"),				IDM_USER_ITEM_OSCAR		},
		{  TEXT("用户自定义原件（１６）"),				IDM_USER_ITEM_PAPA		},
/*80*/	{  TEXT("在各页插入页码"),						IDM_PAGENUM_DLG_OPEN	},
		{  TEXT("（セパレータ）"),						0						},
		{  TEXT("右对齐线"),	/*整形*/					IDM_RIGHT_GUIDE_SET		},
		{  TEXT("在行头追加全角空格"),					IDM_INS_TOPSPACE		},
		{  TEXT("删除行头空格"),						IDM_DEL_TOPSPACE		},
/*85*/	{  TEXT("删除行尾空格"),						IDM_DEL_LASTSPACE		},
		{  TEXT("删除行尾文字"),						IDM_DEL_LASTLETTER		},
		{  TEXT("将选择范围替换为空格"),				IDM_FILL_SPACE			},
		{  TEXT("将全页用空格替换"),					IDM_FILL_ZENSP			},
		{  TEXT("将行头空格替换为Unicode"),			IDM_HEADHALF_EXCHANGE	},
/*90*/	{  TEXT("左右翻转"),							IDM_MIRROR_INVERSE		},
		{  TEXT("上下翻转"),							IDM_UPSET_INVERSE		},
		{  TEXT("向右对齐"),							IDM_RIGHT_SLIDE			},
		{  TEXT("增加1像素"),							IDM_INCREMENT_DOT		},
		{  TEXT("减少1像素"),							IDM_DECREMENT_DOT		},
/*95*/	{  TEXT("全体右移1像素"),						IDM_INCR_DOT_LINES		},
		{  TEXT("全体左移1像素"),						IDM_DECR_DOT_LINES		},
		{  TEXT("从指定位置向左挤压"),					IDM_DOT_SPLIT_RIGHT		},
		{  TEXT("从指定位置向右舒展"),					IDM_DOT_SPLIT_LEFT		},
		{  TEXT("锁定调整基准"),						IDM_DOTDIFF_LOCK		},
/*100*/	{  TEXT("调整为光标所在位置"),					IDM_DOTDIFF_ADJT		},
		{  TEXT("（セパレータ）"),						0						},
		{  TEXT("显示空格"),	/*表示*/					IDM_SPACE_VIEW_TOGGLE	},
		{  TEXT("显示辅助线"),						IDM_GRID_VIEW_TOGGLE	},
		{  TEXT("显示右边界线"),						IDM_RIGHT_RULER_TOGGLE	},
/*105*/	{  TEXT("显示下边界线"),						IDM_UNDER_RULER_TOGGLE	},
		{  TEXT("显示复数行模板"),						IDM_MAATMPLE_VIEW		},
		{  TEXT("页一览"),							IDM_PAGELIST_VIEW		},
		{  TEXT("显示单行模板"),						IDM_LINE_TEMPLATE		},
		{  TEXT("笔刷"),								IDM_BRUSH_PALETTE		},
/*110*/	{  TEXT("Unicode表"),						IDM_UNI_PALETTE			},
		{  TEXT("背景图控制器"),						IDM_TRACE_MODE_ON		},
		{  TEXT("预览"),								IDM_ON_PREVIEW			},
		{  TEXT("缓存作画版"),						IDM_DRAUGHT_OPEN		},
		{  TEXT("サムネイルを表示"),					IDM_MAA_THUMBNAIL_OPEN	},
/*115*/	{  TEXT("（セパレータ）"),						0						},
		{  TEXT("复制到缓存作画版"),					IDM_COPY_TO_DRAUGHT		},
		{  TEXT("（セパレータ）"),			0	/*以下、内容固定？の特殊コマンド*/	},
		{  TEXT("复制选择页"),						IDM_PAGEL_DUPLICATE		},
		{  TEXT("删除选择页"),						IDM_PAGEL_DELETE		},
/*120*/	{  TEXT("和下一页合并"),						IDM_PAGEL_COMBINE		},
		{  TEXT("在选择页之后新建页"),					IDM_PAGEL_INSERT		},
		{  TEXT("在末页之后新建页"),					IDM_PAGEL_ADD			},
		{  TEXT("将页往下移动"),						IDM_PAGEL_DOWNSINK		},
		{  TEXT("将页往上移动"),						IDM_PAGEL_UPFLOW		},
/*125*/	{  TEXT("修改页名称"),						IDM_PAGEL_RENAME		},
		{  TEXT("显示/隐藏 背景图"),					IDM_TRC_VIEWTOGGLE		},
		{  TEXT("切换模板组↑"),						IDM_TMPLT_GROUP_PREV	},
		{  TEXT("切换模板组↓"),						IDM_TMPLT_GROUP_NEXT	},
		{  TEXT("切换当前窗口↑"),						IDM_WINDOW_CHANGE		},
/*130*/	{  TEXT("切换当前窗口↓"),						IDM_WINDOW_CHG_RVRS		},
		{  TEXT("关闭文件"),							IDM_FILE_CLOSE			},
		{  TEXT("切换文件↑"),							IDM_FILE_PREV			},
		{  TEXT("切换文件↓"),							IDM_FILE_NEXT			},
		{  TEXT("移动至前一页"),						IDM_PAGE_PREV			},
/*135*/	{  TEXT("移动至后一页"),						IDM_PAGE_NEXT			},
		{  TEXT("テンプレグリッド増加"),				IDM_TMPL_GRID_INCREASE	},
		{  TEXT("テンプレグリッド減少"),				IDM_TMPL_GRID_DECREASE	},
		{  TEXT("刷新当前画面"),						IDM_NOW_PAGE_REFRESH	},
		{  TEXT("（未実装）文字列検索"),				IDM_FIND_DLG_OPEN		},
/*140*/	{  TEXT("（未実装）検索設定リセット"),			IDM_FIND_HIGHLIGHT_OFF	},
		{  TEXT("（未実装）次の検索位置"),				IDM_FIND_JUMP_NEXT		},
		{  TEXT("（未実装）前の検索位置"),				IDM_FIND_JUMP_PREV		},
		{  TEXT("（未実装）新しい文字列を検索"),			IDM_FIND_TARGET_SET		},
		{  TEXT("（セパレータ）"),						0						}
};

#define ALL_ITEMS	 117	//	右クリ用　０インデックス
#define FULL_ITEMS	 145	//	全アイテム

//	右クリ用サブアイテム・位置調整忘れないように
#define CTS_UNISPACE	24
#define CTS_COLOURINS	33
#define CTS_FRAMEINS	39
#define CTS_USERITEM	64

//	サブ展開するアイテムに注意セヨ・コンテキストメニューとアクセルキー

//	キーバインド・メニューに入ってない機能に注意

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


INT_PTR	CALLBACK AccelKeyDlgProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR	AccelKeyNotify( HWND, INT, LPNMHDR, list<ACCEL> * );

HRESULT	AccelKeyBindExistCheck( HWND, LPACCEL, list<ACCEL> * );
HRESULT	AccelKeyBindListMod( HWND, INT, LPACCEL, list<ACCEL> * );
HRESULT	AccelKeySettingReset( HWND, list<ACCEL> * );
HRESULT	AccelKeyListOutput( HWND );
HRESULT	AccelKeyBindString( LPACCEL, LPTSTR, UINT_PTR );
VOID	AccelKeyListInit( HWND, list<ACCEL> * );
BYTE	AccelHotModExchange( BYTE, BOOLEAN );
HRESULT	AccelKeyTableSave( list<ACCEL> * );



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
	UINT	d, e, num;
	TCHAR	atItem[MAX_STRING], atKey[MIN_STRING], atBuffer[SUB_STRING];
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
			ZeroMemory( atBuffer, sizeof(atBuffer) );
			StringCchCopy( atItem, MAX_STRING, itMnItm->atString );	//	先に記録

			if( IDM_INSFRAME_ALPHA <= itMnItm->dCommandoID && itMnItm->dCommandoID <= IDM_INSFRAME_TANGO )
			{	//	枠名称										//	IDM_INSFRAME_ZULU
				num = itMnItm->dCommandoID - IDM_INSFRAME_ALPHA;
				FrameNameLoad( num, atBuffer, SUB_STRING );
				StringCchPrintf( atItem, MAX_STRING, TEXT("边框：%s"), atBuffer );
			}
			else if( IDM_USER_ITEM_ALPHA <= itMnItm->dCommandoID && itMnItm->dCommandoID <= IDM_USER_ITEM_PAPA )
			{	//	ユーザアイテム名称
				num = itMnItm->dCommandoID - IDM_USER_ITEM_ALPHA;
				UserDefItemNameget( num, atBuffer, SUB_STRING );
				StringCchPrintf( atItem, MAX_STRING, TEXT("用户：%s"), atBuffer );
			}
			else
			{
				//	何も無い
			}

			if( 26 > e )
			{
				StringCchPrintf( atKey, MIN_STRING, TEXT("(&%c)"), 'A' + e );
				StringCchCat( atItem, MAX_STRING, atKey );
				e++;
			}

			switch( itMnItm->dCommandoID )
			{
				default:	AppendMenu( ghPopupMenu, MF_STRING, itMnItm->dCommandoID, atItem );	break;

				case IDM_MN_UNISPACE:
					ghUniSpMenu = CreatePopupMenu(  );
					for( d = 0; 8 > d; d++ ){	AppendMenu( ghUniSpMenu, MF_STRING, gstContextItem[CTS_UNISPACE+d].dCommandoID, gstContextItem[CTS_UNISPACE+d].atString );	}
					AppendMenu( ghPopupMenu, MF_POPUP, (UINT_PTR)ghUniSpMenu, atItem );
					break;

				case IDM_MN_COLOUR_SEL:
					ghColourMenu = CreatePopupMenu(  );
					for( d = 0; 5 > d; d++ ){	AppendMenu( ghColourMenu, MF_STRING, gstContextItem[CTS_COLOURINS+d].dCommandoID, gstContextItem[CTS_COLOURINS+d].atString );	}
					AppendMenu( ghPopupMenu, MF_POPUP, (UINT_PTR)ghColourMenu, atItem );
					break;

				case IDM_MN_INSFRAME_SEL:
					ghFrameMenu = CreatePopupMenu(  );
					for( d = 0; FRAME_MAX > d; d++ )
					{
					//	FrameNameLoad( d, atBuffer, SUB_STRING );
						AppendMenu( ghFrameMenu, MF_STRING, gstContextItem[CTS_FRAMEINS+d].dCommandoID, gstContextItem[CTS_FRAMEINS+d].atString );
					}
					FrameNameModifyPopUp( ghFrameMenu, 1 );
					AppendMenu( ghPopupMenu, MF_POPUP, (UINT_PTR)ghFrameMenu, atItem );
					break;

				case IDM_MN_USER_REFS:
					ghUsrDefMenu = CreatePopupMenu(  );
					UserDefMenuWrite( ghUsrDefMenu, 1 );
					AppendMenu( ghPopupMenu, MF_POPUP, (UINT_PTR)ghUsrDefMenu, atItem );
					break;
			}
		}
	}

	return;
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
			StringCchPrintf( atKeyName, MIN_STRING, TEXT("CmdID%d"), i );
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

	stLvColm.pszText  = TEXT("菜单按钮");
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
			StringCchCat( atItem, SUB_STRING, TEXT("（打开副菜单）") );
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
		std::advance( itMnItm, iIns+1 );
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
	std::advance( itMnItm, iSel );

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
	std::advance( itTgtItm, iSel );
	itSwpItm = itTgtItm;
	itSwpItm--;	//	入れる先は一つ前

	StringCchCopy( stItem.atString, MIN_STRING, itTgtItm->atString );
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
	std::advance( itSwpItm, iSel );
	itTgtItm = itSwpItm;
	itTgtItm++;	//	入れる先は一つ次

	StringCchCopy( stItem.atString, MIN_STRING, itTgtItm->atString );
	stItem.dCommandoID = itTgtItm->dCommandoID;

	gltCntxEdit.erase( itTgtItm );
	gltCntxEdit.insert( itSwpItm, stItem );

	CntxDlgBuildListUp( hDlg );

	ListView_SetItemState( hBuildWnd, ++iSel, LVIS_SELECTED, LVIS_SELECTED );

	return;
}
//-------------------------------------------------------------------------------------------------



#ifdef ACCELERATOR_EDIT
//-------------------------------------------------------------------------------------------------

//	ホットキーコントロールは、フォーカスあててキー入力すると認識する
//	登録済みのグローバルホットキーがあると、そっちが優先して動く

/*!
	アクセラレートキー編集DIALOGUEを開く
	@param[in]	hWnd		ウインドウハンドル
	@return		HRESULT		終了状態コード
*/
HRESULT AccelKeyDlgOpen( HWND hWnd )
{
	INT_PTR	iRslt;

	LPACCEL	pstAccel;
	INT	iEntry;

	iRslt = DialogBoxParam( ghInst, MAKEINTRESOURCE(IDD_ACCEL_KEY_DLG), hWnd, AccelKeyDlgProc, 0 );
	if( IDOK == iRslt )
	{
		//	アクセラハンドル書換が必要
		//	アクセラテーブル確保してメモリ構造体で確保
		pstAccel = AccelKeyTableLoadAlloc( &iEntry );
		AccelKeyTableCreate( pstAccel, iEntry );

		ToolBarInfoChange( pstAccel, iEntry );

		FREE( pstAccel );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	アクセルキー編集ダイヤログボックスのメセージハンドラだってばよ
	@param[in]	hDlg	ダイヤログハンドル
	@param[in]	message	ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@retval 0	メッセージは処理していない
	@retval no0	なんか処理された
*/
INT_PTR CALLBACK AccelKeyDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static list<ACCEL>	cltAccel;

	INT		iAccEntry;
	LPACCEL	pstAccel;

	static  HWND	hHokyWnd;
	HWND	hLvWnd;
	LRESULT	lRslt;
	ACCEL	stAcce;
//	TCHAR	atBuffer[SUB_STRING];

	INT		i;
	INT		iItem;

	INT		id;
	HWND	hWndCtl;
	UINT	codeNotify;

//ホットキーCONTROLは、スペースが使えない

	switch( message )
	{
		case WM_INITDIALOG:
			hLvWnd = GetDlgItem( hDlg, IDLV_FUNCKEY_LIST );
			ListView_SetExtendedListViewStyle( hLvWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_LABELTIP );

			cltAccel.clear();
			pstAccel = AccelKeyTableGetAlloc( &iAccEntry );
			//	ここで確保したアクセラはリストにして保持しておく
			for( i = 0; iAccEntry > i; i++ ){	cltAccel.push_back( pstAccel[i] );	}
			FREE( pstAccel );

			AccelKeyListInit( hDlg, &cltAccel );

			hHokyWnd = GetDlgItem( hDlg, IDHKC_FUNCKEY_INPUT );
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			id         = LOWORD(wParam);	//	メッセージを発生させた子ウインドウの識別子
			hWndCtl    = (HWND)lParam;		//	メッセージを発生させた子ウインドウのハンドル
			codeNotify = HIWORD(wParam);	//	通知メッセージ
			switch( id )
			{
				case IDOK:
					AccelKeyTableSave( &cltAccel );	//	セーブ
				case IDCANCEL:
					cltAccel.clear();
					EndDialog( hDlg, id );
					return (INT_PTR)TRUE;


				case IDB_FUNCKEY_CLEAR:	//	解除
					SendMessage( hHokyWnd, HKM_SETHOTKEY, 0, 0 );
					iItem = WndTagGet( hHokyWnd );
					AccelKeyBindListMod( hDlg, iItem, NULL, &cltAccel );
					return (INT_PTR)TRUE;


				case IDB_FUNCKEY_SET:	//	セット
					lRslt = SendMessage( hHokyWnd, HKM_GETHOTKEY, 0, 0 );
					stAcce.key   = LOBYTE( lRslt );
					if( BST_CHECKED == IsDlgButtonChecked( hDlg, IDCB_FUNCKEY_SPACE ) )
					{	stAcce.key = VK_SPACE;	}
					stAcce.fVirt  = AccelHotModExchange( HIBYTE( lRslt ), 0 );
					stAcce.cmd    = 0;
					if( SUCCEEDED( AccelKeyBindExistCheck( hDlg, &stAcce, &cltAccel ) ) )
					{
						//	かぶってなかったら登録する
						iItem = WndTagGet( hHokyWnd );
						AccelKeyBindListMod( hDlg, iItem, &stAcce, &cltAccel );
					}
					return (INT_PTR)TRUE;


				case IDB_FUNCKEY_INIT:	//	設定を初期化する
					if( IDOK == MessageBox( hDlg, TEXT("要将快捷键设置初始化了哦"), TEXT("已经把设定初始化了哦"), MB_OKCANCEL | MB_ICONQUESTION ) )
					{
						AccelKeySettingReset( hDlg, &cltAccel );
						AccelKeyListInit( hDlg, &cltAccel );
					}
					return (INT_PTR)TRUE;


				case IDB_FUNCKEY_FILEOUT:	//	設定をファイルに出力
					AccelKeyListOutput( hDlg );
					return (INT_PTR)TRUE;


				default:	break;
			}
			break;

		case WM_NOTIFY:
			return AccelKeyNotify( hDlg, (INT)(wParam), (LPNMHDR)(lParam), &cltAccel );

		default:	break;
	}

	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	アクセルキー編集のノーティファイメッセージ処理
	@param[in]	hDlg		ダイヤログハンドル
	@param[in]	idFrom		発生したコモンのID番号
	@param[in]	pstNmhdr	イベント内容
	@param[in]	*pltAccel	アクセラキーテーブル
	@retval 0	メッセージは処理していない
	@retval no0	なんか処理された
*/
INT_PTR AccelKeyNotify( HWND hDlg, INT idFrom, LPNMHDR pstNmhdr, list<ACCEL> *pltAccel )
{
	LPNMLISTVIEW	pstLv;
	LVITEM			stLvi;
	BYTE	bMod;
	HWND	hHokyWnd;

	list<ACCEL>::iterator	itAccel;

	if( IDLV_FUNCKEY_LIST == idFrom )	//	一覧リストビューで
	{
		if( NM_CLICK == pstNmhdr->code )	//	クリッケされたら
		{
			pstLv = (LPNMLISTVIEW)pstNmhdr;
			pstLv->iItem;	//	そのアイテムを確認して

			ZeroMemory( &stLvi, sizeof(stLvi) );
			stLvi.mask     = LVIF_PARAM;	//	コマンドコードを確保する
			stLvi.iItem    = pstLv->iItem;
			ListView_GetItem( pstNmhdr->hwndFrom, &stLvi );

			hHokyWnd = GetDlgItem( hDlg, IDHKC_FUNCKEY_INPUT );
			WndTagSet( hHokyWnd , stLvi.iItem );	//	選択した行を確保

			for( itAccel = (*pltAccel).begin(); itAccel != (*pltAccel).end(); itAccel++ )
			{
				if( stLvi.lParam == itAccel->cmd )
				{
					bMod = AccelHotModExchange( itAccel->fVirt, 1 );
					if( 0x20 == itAccel->key )
					{	SendMessage( hHokyWnd , HKM_SETHOTKEY, MAKEWORD(itAccel->key, bMod), 0 );	}
					else{	SendMessage( hHokyWnd, HKM_SETHOTKEY, MAKEWORD(itAccel->key, (bMod|HOTKEYF_EXT)), 0 );	}

					break;			//		常にHOTKEYF_EXTいれると、Spaceが本当に空白になる
				}
			}
			if( itAccel == (*pltAccel).end() )	SendMessage( hHokyWnd, HKM_SETHOTKEY, 0, 0 );

			SetFocus( GetDlgItem( hDlg, IDHKC_FUNCKEY_INPUT ) );
		}
		return (INT_PTR)TRUE;
	}

	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	アクセラテーブルを初期ファイルから確保する・メモリ確保するので返り値の扱いに注意
	@param[out]	piEntry	確保したエントリ数を戻す
	@return		確保したテーブル構造体のポインター・freeに注意
*/
LPACCEL AccelKeyTableLoadAlloc( LPINT piEntry )
{
	UINT	dCount, dValue;
	UINT	i, aim = 0;
	TCHAR	atKeyName[MIN_STRING];
	LPACCEL	pstAccel = NULL;

	//	初期状態
	dCount = GetPrivateProfileInt( TEXT("Accelerator"), TEXT("Count"), 0, gatCntxIni );
	if( 1 <= dCount )	//	データ有り
	{
		pstAccel = (LPACCEL)malloc( dCount * sizeof(ACCEL) );

		for( i = 0; dCount > i; i++ )
		{
			StringCchPrintf( atKeyName, MIN_STRING, TEXT("AcCMD%d"), i );
			dValue = GetPrivateProfileInt( TEXT("Accelerator"), atKeyName, 0, gatCntxIni );
			if( 0 == dValue )	continue;	//	設定が０なら何もしない
			pstAccel[aim].cmd = dValue;

			StringCchPrintf( atKeyName, MIN_STRING, TEXT("AcVirt%d"), i );
			dValue = GetPrivateProfileInt( TEXT("Accelerator"), atKeyName, 0, gatCntxIni );
			pstAccel[aim].fVirt = dValue;

			StringCchPrintf( atKeyName, MIN_STRING, TEXT("AcKey%d"), i );
			dValue = GetPrivateProfileInt( TEXT("Accelerator"), atKeyName, 0, gatCntxIni );
			pstAccel[aim].key = dValue;

			aim++;
		}
	}

	if( piEntry )	*piEntry = aim;

	return pstAccel;
}
//-------------------------------------------------------------------------------------------------

/*!
	キーバインドを参照して、文字列を作る
	@param[in]	pstAccel	アクセラキーテーブル
	@param[in]	ptBuffer	文字列入れるバッファのポインター
	@param[in]	cchSize		バッファの文字数
	@return		HRESULT		終了状態コード
*/
HRESULT AccelKeyBindString( LPACCEL pstAccel, LPTSTR ptBuffer, UINT_PTR cchSize )
{
	TCHAR	atKey[MIN_STRING];

	ZeroMemory( ptBuffer, cchSize * sizeof(TCHAR) );

//	FVIRTKEY  FNOINVERT  は常に含める

	if( FCONTROL & pstAccel->fVirt )	StringCchCat( ptBuffer, cchSize, TEXT("Ctrl + ") );
	if( FSHIFT   & pstAccel->fVirt )	StringCchCat( ptBuffer, cchSize, TEXT("Shift + ") );
	if( FALT     & pstAccel->fVirt )	StringCchCat( ptBuffer, cchSize, TEXT("Alt + ") );

	if( '0' <= pstAccel->key && pstAccel->key <= '9' )	//	VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
	{
		StringCchPrintf( atKey, MIN_STRING, TEXT("%c"), pstAccel->key );
	}
	else if( 'A' <= pstAccel->key && pstAccel->key <= 'Z' )	//	VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
	{
		StringCchPrintf( atKey, MIN_STRING, TEXT("%c"), pstAccel->key );
	}
	else if( 0x60 <= pstAccel->key && pstAccel->key <= 0x69 )	//	NUMPAD0 - NUMPAD9
	{
		StringCchPrintf( atKey, MIN_STRING, TEXT("NUMPAD%u"), pstAccel->key - 0x60 );
	}
	else if( 0x70 <= pstAccel->key && pstAccel->key <= 0x87 )	//	F1 - F24
	{
		StringCchPrintf( atKey, MIN_STRING, TEXT("F%u"), pstAccel->key - 0x70 + 1 );
	}
	else
	{
		switch( pstAccel->key )
		{
			case VK_CANCEL:		StringCchCopy( atKey, MIN_STRING, TEXT("Break") );	break;
			case VK_BACK:		StringCchCopy( atKey, MIN_STRING, TEXT("BackSpace") );	break;
			case VK_TAB:		StringCchCopy( atKey, MIN_STRING, TEXT("TAB") );	break;
			case VK_CLEAR:		StringCchCopy( atKey, MIN_STRING, TEXT("CLEAR") );	break;
			case VK_RETURN:		StringCchCopy( atKey, MIN_STRING, TEXT("Enter") );	break;
			case VK_PAUSE:		StringCchCopy( atKey, MIN_STRING, TEXT("Pause") );	break;
			case VK_CAPITAL:	StringCchCopy( atKey, MIN_STRING, TEXT("CAPITAL") );	break;
			case VK_KANA:		StringCchCopy( atKey, MIN_STRING, TEXT("KANA") );	break;
			case VK_ESCAPE:		StringCchCopy( atKey, MIN_STRING, TEXT("Esc") );	break;
			case VK_CONVERT:	StringCchCopy( atKey, MIN_STRING, TEXT("变换") );	break;
			case VK_NONCONVERT:	StringCchCopy( atKey, MIN_STRING, TEXT("无変換") );	break;
			case VK_SPACE:		StringCchCopy( atKey, MIN_STRING, TEXT("Space") );	break;
			case VK_PRIOR:		StringCchCopy( atKey, MIN_STRING, TEXT("PageUp") );	break;
			case VK_NEXT:		StringCchCopy( atKey, MIN_STRING, TEXT("PageDown") );	break;
			case VK_END:		StringCchCopy( atKey, MIN_STRING, TEXT("End") );	break;
			case VK_HOME:		StringCchCopy( atKey, MIN_STRING, TEXT("Home") );	break;
			case VK_LEFT:		StringCchCopy( atKey, MIN_STRING, TEXT("←") );	break;
			case VK_UP:			StringCchCopy( atKey, MIN_STRING, TEXT("↑") );	break;
			case VK_RIGHT:		StringCchCopy( atKey, MIN_STRING, TEXT("→") );	break;
			case VK_DOWN:		StringCchCopy( atKey, MIN_STRING, TEXT("↓") );	break;
			case VK_SELECT:		StringCchCopy( atKey, MIN_STRING, TEXT("SELECT") );	break;
			case VK_PRINT:		StringCchCopy( atKey, MIN_STRING, TEXT("PRINT") );	break;
			case VK_EXECUTE:	StringCchCopy( atKey, MIN_STRING, TEXT("EXECUTE") );	break;
			case VK_SNAPSHOT:	StringCchCopy( atKey, MIN_STRING, TEXT("PrintScr") );	break;
			case VK_INSERT:		StringCchCopy( atKey, MIN_STRING, TEXT("Insert") );	break;
			case VK_DELETE:		StringCchCopy( atKey, MIN_STRING, TEXT("Delete") );	break;
			case VK_HELP:		StringCchCopy( atKey, MIN_STRING, TEXT("Help") );	break;
			case VK_LWIN:		StringCchCopy( atKey, MIN_STRING, TEXT("左Win") );	break;
			case VK_RWIN:		StringCchCopy( atKey, MIN_STRING, TEXT("右Win") );	break;
			case VK_APPS:		StringCchCopy( atKey, MIN_STRING, TEXT("APPZ") );	break;
			case VK_SLEEP:		StringCchCopy( atKey, MIN_STRING, TEXT("SLEEP") );	break;
			case VK_MULTIPLY:	StringCchCopy( atKey, MIN_STRING, TEXT("NUM *") );	break;
			case VK_ADD:		StringCchCopy( atKey, MIN_STRING, TEXT("NUM +") );	break;
			case VK_SEPARATOR:	StringCchCopy( atKey, MIN_STRING, TEXT("NUM ,") );	break;
			case VK_SUBTRACT:	StringCchCopy( atKey, MIN_STRING, TEXT("NUM -") );	break;
			case VK_DECIMAL:	StringCchCopy( atKey, MIN_STRING, TEXT("NUM .") );	break;
			case VK_DIVIDE:		StringCchCopy( atKey, MIN_STRING, TEXT("NUM /") );	break;
			case VK_NUMLOCK:	StringCchCopy( atKey, MIN_STRING, TEXT("NumLock") );	break;
			case VK_SCROLL:		StringCchCopy( atKey, MIN_STRING, TEXT("ScrollLock") );	break;
			case VK_OEM_NEC_EQUAL:	StringCchCopy( atKey, MIN_STRING, TEXT("NUM =") );	break;
			case VK_BROWSER_BACK:		StringCchCopy( atKey, MIN_STRING, TEXT("返回") );	break;
			case VK_BROWSER_FORWARD:	StringCchCopy( atKey, MIN_STRING, TEXT("前进") );	break;
			case VK_BROWSER_REFRESH:	StringCchCopy( atKey, MIN_STRING, TEXT("更新") );	break;
			case VK_BROWSER_STOP:		StringCchCopy( atKey, MIN_STRING, TEXT("停止") );	break;
			case VK_BROWSER_SEARCH:		StringCchCopy( atKey, MIN_STRING, TEXT("检索") );	break;
			case VK_BROWSER_FAVORITES:	StringCchCopy( atKey, MIN_STRING, TEXT("收藏") );	break;
			case VK_BROWSER_HOME:		StringCchCopy( atKey, MIN_STRING, TEXT("主页") );	break;
			case VK_VOLUME_MUTE:		StringCchCopy( atKey, MIN_STRING, TEXT("静音") );	break;
			case VK_VOLUME_DOWN:		StringCchCopy( atKey, MIN_STRING, TEXT("降音") );	break;
			case VK_VOLUME_UP:			StringCchCopy( atKey, MIN_STRING, TEXT("升音") );	break;
			case VK_MEDIA_NEXT_TRACK:	StringCchCopy( atKey, MIN_STRING, TEXT("后一项") );	break;
			case VK_MEDIA_PREV_TRACK:	StringCchCopy( atKey, MIN_STRING, TEXT("前一项") );	break;
			case VK_MEDIA_STOP:			StringCchCopy( atKey, MIN_STRING, TEXT("停止") );	break;
			case VK_MEDIA_PLAY_PAUSE:	StringCchCopy( atKey, MIN_STRING, TEXT("重播") );	break;
			case VK_LAUNCH_MAIL:		StringCchCopy( atKey, MIN_STRING, TEXT("邮件") );	break;
			case VK_LAUNCH_MEDIA_SELECT:StringCchCopy( atKey, MIN_STRING, TEXT("选择") );	break;
			case VK_LAUNCH_APP1:		StringCchCopy( atKey, MIN_STRING, TEXT("APP1") );	break;
			case VK_LAUNCH_APP2:		StringCchCopy( atKey, MIN_STRING, TEXT("APP2") );	break;
			case VK_OEM_1:		StringCchCopy( atKey, MIN_STRING, TEXT(":") );	break;
			case VK_OEM_PLUS:	StringCchCopy( atKey, MIN_STRING, TEXT(";") );	break;
			case VK_OEM_COMMA:	StringCchCopy( atKey, MIN_STRING, TEXT(",") );	break;
			case VK_OEM_MINUS:	StringCchCopy( atKey, MIN_STRING, TEXT("-") );	break;
			case VK_OEM_PERIOD:	StringCchCopy( atKey, MIN_STRING, TEXT(".") );	break;
			case VK_OEM_2:		StringCchCopy( atKey, MIN_STRING, TEXT("/") );	break;
			case VK_OEM_3:		StringCchCopy( atKey, MIN_STRING, TEXT("@") );	break;
			case VK_OEM_4:		StringCchCopy( atKey, MIN_STRING, TEXT("[") );	break;
			case VK_OEM_5:		StringCchCopy( atKey, MIN_STRING, TEXT("\\") );	break;	//	￥
			case VK_OEM_6:		StringCchCopy( atKey, MIN_STRING, TEXT("]") );	break;
			case VK_OEM_7:		StringCchCopy( atKey, MIN_STRING, TEXT("^") );	break;
			case VK_OEM_8:		StringCchCopy( atKey, MIN_STRING, TEXT("_") );	break;
			case VK_OEM_102:	StringCchCopy( atKey, MIN_STRING, TEXT("ろ") );	break;	//	ろ
			case VK_OEM_ATTN:	StringCchCopy( atKey, MIN_STRING, TEXT("大小写切换") );	break;
			case VK_OEM_COPY:	StringCchCopy( atKey, MIN_STRING, TEXT("片假平假转换") );	break;
			case VK_OEM_AUTO:	StringCchCopy( atKey, MIN_STRING, TEXT("半全角汉字1") );	break;
			case VK_OEM_ENLW:	StringCchCopy( atKey, MIN_STRING, TEXT("半全角汉字2") );	break;

			//	足りない分はＳＤＫから追加する
			default:	StringCchPrintf( atKey, MIN_STRING, TEXT("0x%02X"), pstAccel->key );	break;
		}
	}
//0x40 : unassigned
//0x07 : unassigned
//0x0A - 0x0B : reserved
//0x5E : reserved
//0x88 - 0x8F : unassigned
//0x97 - 0x9F : unassigned
//0xB8 - 0xB9 : reserved
//0xC1 - 0xD7 : reserved
//0xD8 - 0xDA : unassigned
//0xE0 : reserved
//0xE8 : unassigned
//0xFF : reserved

	StringCchCat( ptBuffer, cchSize, atKey );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	コマンド番号を参照して、ヒットしたらアクセル文字列を作ってくっつける
	@param[in]	ptText		処理結果をくっつける文字列ポインター
	@param[in]	cchSize		バッファの文字数
	@param[in]	dCommand	コマンド番号
	@param[in]	pstAccel	アクセラキーテーブル
	@param[in]	iEntry		テーブルのデータ数
	@return		HRESULT		終了状態コード
*/
HRESULT AccelKeyTextBuild( LPTSTR ptText, UINT_PTR cchSize, DWORD dCommand, CONST LPACCEL pstAccel, INT iEntry )
{
	TCHAR	atKeystr[SUB_STRING];
	INT		i;

	for( i = 0; iEntry > i; i++ )
	{
		if( pstAccel[i].cmd == dCommand )
		{
			AccelKeyBindString( &(pstAccel[i]), atKeystr, SUB_STRING );

			StringCchCat( ptText, cchSize, TEXT("\r\n") );
			StringCchCat( ptText, cchSize, atKeystr );

			return S_OK;
		}
	}

	return E_OUTOFMEMORY;
}
//-------------------------------------------------------------------------------------------------

/*!
	アクセルキー編集のリストビュー初期化
	@param[in]	hDlg		ダイヤログハンドル
	@param[in]	*pltAccel	アクセラキーテーブル
*/
VOID AccelKeyListInit( HWND hDlg, list<ACCEL> *pltAccel )
{
	HWND		hLvWnd;
	LVCOLUMN	stLvColm;
	LVITEM		stItem;
	RECT		rect;
	LONG		width, i, j;
	TCHAR		atBuffer[SUB_STRING];

	list<ACCEL>::iterator	itAccel;

	hLvWnd = GetDlgItem( hDlg, IDLV_FUNCKEY_LIST );
	GetClientRect( hLvWnd, &rect );
	width = rect.right - 23;
	width /= 2;

	ZeroMemory( &stLvColm, sizeof(LVCOLUMN) );
	stLvColm.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	stLvColm.fmt      = LVCFMT_LEFT;

	stLvColm.iSubItem = 0;
	stLvColm.pszText  = TEXT("功能");
	stLvColm.cx       = width;
	ListView_InsertColumn( hLvWnd, 0, &stLvColm );

	stLvColm.iSubItem = 1;
	stLvColm.pszText  = TEXT("组合键");
	stLvColm.cx       = width;
	ListView_InsertColumn( hLvWnd, 1, &stLvColm );


	ZeroMemory( &stItem, sizeof(LVITEM) );

	for( i = 0, j = 0; ALL_ITEMS > i; i++ )
	{
		stItem.iItem = j;

		//	関係無いやつは飛ばす
		if( 0 == gstContextItem[i].dCommandoID || 
		IDM_MN_UNISPACE     == gstContextItem[i].dCommandoID || 
		IDM_MN_COLOUR_SEL   == gstContextItem[i].dCommandoID || 
		IDM_MN_INSFRAME_SEL == gstContextItem[i].dCommandoID || 
		IDM_MN_USER_REFS    == gstContextItem[i].dCommandoID )
		{	continue;	}

		StringCchCopy( atBuffer, SUB_STRING, gstContextItem[i].atString );
		stItem.mask     = LVIF_TEXT | LVIF_PARAM;
		stItem.pszText  = atBuffer;
		stItem.lParam   = gstContextItem[i].dCommandoID;
		stItem.iSubItem = 0;
		ListView_InsertItem( hLvWnd, &stItem );

		ZeroMemory( atBuffer, sizeof(atBuffer) );

		for( itAccel = (*pltAccel).begin(); itAccel != (*pltAccel).end(); itAccel++ )
		{
			if( gstContextItem[i].dCommandoID == itAccel->cmd )
			{
				AccelKeyBindString( &(*itAccel), atBuffer, SUB_STRING );
				break;
			}
		}

		stItem.mask     = LVIF_TEXT;
		stItem.pszText  = atBuffer;
		stItem.iSubItem = 1;
		ListView_SetItem( hLvWnd, &stItem );

		j++;
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	アクセルキーとホットキーの修飾子を入替
	@param[in]	bSrc	元の修飾子コード
	@param[in]	bDrct	非０アクセル→ホット　０ホット→アクセル
	@return	変換したコード
*/
BYTE AccelHotModExchange( BYTE bSrc, BOOLEAN bDrct )
{
	BYTE	bDest = 0;

	if( bDrct )	//	アクセル→ホット
	{
		if( bSrc & FSHIFT )		bDest |= HOTKEYF_SHIFT;		//	シフト
		if( bSrc & FCONTROL )	bDest |= HOTKEYF_CONTROL;	//	コントロール
		if( bSrc & FALT )		bDest |= HOTKEYF_ALT;		//	アルタネート
	}
	else	//	ホット→アクセル
	{
		if( bSrc & HOTKEYF_SHIFT )		bDest |= FSHIFT;	//	シフト
		if( bSrc & HOTKEYF_CONTROL )	bDest |= FCONTROL;	//	コントロール
		if( bSrc & HOTKEYF_ALT )		bDest |= FALT;		//	アルタネート

		bDest |= (FVIRTKEY|FNOINVERT);	//	常にある
	}

	return bDest;
}
//-------------------------------------------------------------------------------------------------

/*!
	アクセルテーブルを保存
	@param[in]	*pltAccel	アクセラキーテーブル
	@return		HRESULT		終了状態コード
*/
HRESULT AccelKeyTableSave( list<ACCEL> *pltAccel )
{
	INT_PTR	i;
	TCHAR	atKeyName[MIN_STRING], atBuff[MIN_STRING];
	list<ACCEL>::iterator	itAccel;

	//	一旦セクションを空にする
	ZeroMemory( atBuff, sizeof(atBuff) );
	WritePrivateProfileSection( TEXT("Accelerator"), atBuff, gatCntxIni );

	i = 0;
	for( itAccel = (*pltAccel).begin(); itAccel != (*pltAccel).end(); itAccel++ )
	{
		StringCchPrintf( atKeyName, MIN_STRING, TEXT("AcCMD%d"), i );
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), itAccel->cmd );
		WritePrivateProfileString( TEXT("Accelerator"), atKeyName, atBuff, gatCntxIni );

		StringCchPrintf( atKeyName, MIN_STRING, TEXT("AcVirt%d"), i );
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), itAccel->fVirt );
		WritePrivateProfileString( TEXT("Accelerator"), atKeyName, atBuff, gatCntxIni );

		StringCchPrintf( atKeyName, MIN_STRING, TEXT("AcKey%d"), i );
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), itAccel->key );
		WritePrivateProfileString( TEXT("Accelerator"), atKeyName, atBuff, gatCntxIni );

		i++;
	}

	StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), i );
	WritePrivateProfileString( TEXT("Accelerator"), TEXT("Count"), atBuff, gatCntxIni );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	アクセル設定を初期状態にアッー
	@param[in]	hDlg		ダイヤログハンドル
	@param[in]	*pltAccel	アクセラキーテーブル
	@return		HRESULT		終了状態コード
*/
HRESULT AccelKeySettingReset( HWND hDlg, list<ACCEL> *pltAccel )
{
	HWND	hLvWnd;
	HACCEL	hAccel;	//	
	LPACCEL	pstAccel = NULL;
	INT		iItems, i;

	//	元々のテーブルを確保
	hAccel = LoadAccelerators( ghInst, MAKEINTRESOURCE(IDC_ORINRINEDITOR) );

	//	まず個数確保
	iItems = CopyAcceleratorTable( hAccel, NULL, 0 );
	if( 0 >= iItems )	return E_POINTER;

	pstAccel = (LPACCEL)malloc( iItems * sizeof(ACCEL) );
	if( !(pstAccel) )	return NULL;

	//	本体確保
	iItems = CopyAcceleratorTable( hAccel, pstAccel, iItems );

	DestroyAcceleratorTable( hAccel );	//	全部汚倭ったらぶっ壊しておく

	(*pltAccel).clear();	//	クルヤー

	for( i = 0; iItems > i; i++ ){	(*pltAccel).push_back( pstAccel[i] );	}

	FREE( pstAccel );

	hLvWnd = GetDlgItem( hDlg, IDLV_FUNCKEY_LIST );
	ListView_DeleteAllItems( hLvWnd );	//	リストビューは壊しておく方が早い

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	設定をファイルにエクスポートする
	@param[in]	hDlg	ダイヤログハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT AccelKeyListOutput( HWND hDlg )
{
//	CONST  WCHAR	rtHead = 0xFEFF;	//	ユニコードテキストヘッダ
	//	ファイル形式は？　SJISかUTF8でいい

	HANDLE	hFile;
	DWORD	wrote;
	BOOLEAN	bOpened;

	OPENFILENAME	stSaveFile;

	INT		i, iAccEntry;
	INT		j;
	LPACCEL	pstAccel;

	UINT_PTR	cchSz;
	INT		cbSize;


	TCHAR	atFilePath[MAX_PATH], atFileName[MAX_STRING];
	TCHAR	atCmdName[MIN_STRING], atKeyBind[SUB_STRING];
	TCHAR	atBuffer[MAX_PATH];

	CHAR	acString[BIG_STRING];


	ZeroMemory( &stSaveFile, sizeof(OPENFILENAME) );

	ZeroMemory( atFilePath,  sizeof(atFilePath) );
	ZeroMemory( atFileName,  sizeof(atFileName) );
	ZeroMemory( atBuffer,  sizeof(atBuffer) );

	StringCchCopy( atFilePath, MAX_PATH, TEXT("Accelerator.txt") );
#if 1
	//ここで FileSaveDialogue を出す
	stSaveFile.lStructSize     = sizeof(OPENFILENAME);
	stSaveFile.hwndOwner       = hDlg;
	stSaveFile.lpstrFilter     = TEXT("文本文件 ( *.txt )\0*.txt\0所有文件类型 ( *.* )\0*.*\0\0");
	stSaveFile.nFilterIndex    = 1;	//	デフォのフィルタ選択肢
	stSaveFile.lpstrFile       = atFilePath;
	stSaveFile.nMaxFile        = MAX_PATH;
	stSaveFile.lpstrFileTitle  = atFileName;
	stSaveFile.nMaxFileTitle   = MAX_STRING;
	stSaveFile.lpstrDefExt     = TEXT("txt");
//	stSaveFile.lpstrInitialDir = 
	stSaveFile.lpstrTitle      = TEXT("请指定要保存的文件名呢");
	stSaveFile.Flags           = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

	bOpened = GetSaveFileName( &stSaveFile );
	if( !(bOpened) ){	return  E_ABORT;	}
	//	キャンセルしてたら何もしない
#endif
	hFile = CreateFile( atFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile )
	{
		MessageBox( hDlg, TEXT("打开文件失败了哦……"), NULL, MB_OK | MB_ICONERROR );
		return E_HANDLE;
	}
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );

	//WriteFile( hFile, &rtHead, 2, &wrote, NULL );

	//	設定されてるテーブルを確保
	pstAccel = AccelKeyTableGetAlloc( &iAccEntry );
	for( i = 0; iAccEntry > i; i++ )
	{
		AccelKeyBindString( &(pstAccel[i]), atKeyBind, SUB_STRING );

		StringCchCopy( atCmdName, MIN_STRING, TEXT("（未知名称）") );
		for( j = 0; FULL_ITEMS > j; j++ )
		{
			if( gstContextItem[j].dCommandoID == pstAccel[i].cmd )
			{
				StringCchCopy( atCmdName, MIN_STRING, gstContextItem[j].atString );
				break;
			}
		}

		StringCchPrintf( atBuffer, MAX_PATH, TEXT("%s\t%s\r\n"), atCmdName, atKeyBind );
		StringCchLength( atBuffer, MAX_PATH, &cchSz );

		cbSize = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, atBuffer, -1, acString, BIG_STRING, NULL, NULL );
									//	CP_UTF8, 0	
		//	変換文字数にはヌルターミネータが含まれているので注意
		WriteFile( hFile, acString, cbSize-1, &wrote, NULL );
	}
	FREE( pstAccel );

	SetEndOfFile( hFile );
	CloseHandle( hFile );

	MessageBox( hDlg, TEXT("已经保存了文件了哦。"), TEXT("（・∀・）ｂ"), MB_OK | MB_ICONINFORMATION );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定のコマンドが使われてるかどうか確認
	@param[in]	hDlg		ダイヤログハンドル
	@param[in]	pstAccel	アクセラキーテーブル
	@param[in]	*pltAccel	既存のやつの一覧
	@return		HRESULT		終了状態コード　S_OKかぶり無し　E_ACCESSDENIEDかぶり有った
*/
HRESULT AccelKeyBindExistCheck( HWND hDlg, LPACCEL pstAccel, list<ACCEL> *pltAccel )
{
	INT	i;
	//BYTE	fVirt;
	WORD	dCommand;
	list<ACCEL>::iterator	itAccel;
	TCHAR	atFuncName[MIN_STRING], atMsg[MAX_STRING];

	for( itAccel = (*pltAccel).begin(); itAccel != (*pltAccel).end(); itAccel++ )
	{
		//	同じのがあるかどうか探す
		//fVirt= itAccel->fVirt;
		//fVirt &= ~(FVIRTKEY|FNOINVERT);変換時に付け足してるので要らない
		if( pstAccel->key == itAccel->key && pstAccel->fVirt == itAccel->fVirt )
		{
			//	同じのがあった
			dCommand = itAccel->cmd;
			StringCchCopy( atFuncName, MIN_STRING, TEXT("（未知名称）") );

			for( i = 0; FULL_ITEMS > i; i++ )
			{
				if( dCommand == gstContextItem[i].dCommandoID )
				{
					StringCchCopy( atFuncName, MIN_STRING, gstContextItem[i].atString );
					break;
				}
			}

			StringCchPrintf( atMsg, MAX_STRING, TEXT("这个组合键已经被「%s」占用了哦。"), atFuncName );
			MessageBox( hDlg, atMsg, TEXT("设置重复了哦"), MB_OK | MB_ICONWARNING );

			return E_ACCESSDENIED;
		}
	}
	
	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	アクセルキー編集のリストビューにデータ書込
	@param[in]	hDlg		ダイヤログハンドル
	@param[in]	iItem		操作する行番号
	@param[in]	pstAccel	アクセラキーテーブル
	@param[in]	*pltAccel	既存のやつの一覧
	@return		HRESULT		終了状態コード
*/
HRESULT AccelKeyBindListMod( HWND hDlg, INT iItem, LPACCEL pstAccel, list<ACCEL> *pltAccel )
{
	HWND	hLvWnd = GetDlgItem( hDlg, IDLV_FUNCKEY_LIST );
	HWND	hHkcWnd = GetDlgItem( hDlg, IDHKC_FUNCKEY_INPUT );
	LVITEM	stLvi;
	TCHAR	atBuffer[SUB_STRING];
	WORD	dCommand;
	list<ACCEL>::iterator	itAccel;

	//	該当行から、コマンド番号を持ってくる
	ZeroMemory( &stLvi, sizeof(stLvi) );
	stLvi.mask     = LVIF_PARAM;	//	コマンドコードを確保する
	stLvi.iItem    = iItem;
	ListView_GetItem( hLvWnd, &stLvi );
	dCommand = stLvi.lParam;

	for( itAccel = (*pltAccel).begin(); itAccel != (*pltAccel).end(); itAccel++ )
	{
		if( dCommand ==  itAccel->cmd ){	break;	}
		//	ヒットしたらソレでおｋ
	}

	if( pstAccel )	//	変更・追加
	{
		if( itAccel == (*pltAccel).end() )	//	追加
		{
			pstAccel->cmd = dCommand;
			(*pltAccel).push_back( *pstAccel );
		}
		else	//	変更
		{
			itAccel->key   = pstAccel->key;
			itAccel->fVirt = pstAccel->fVirt;
		}
		
		//	表示用文字列作って
		AccelKeyBindString( pstAccel, atBuffer, SUB_STRING );
		ZeroMemory( &stLvi, sizeof(stLvi) );
		stLvi.mask     = LVIF_TEXT;	//	コマンドコードを確保する
		stLvi.iItem    = iItem;
		stLvi.iSubItem = 1;
		stLvi.pszText  = atBuffer;
		ListView_SetItem( hLvWnd, &stLvi );
	}
	else	//	解除
	{
		//	ヒットしてないなんてことは無いはずだが
		if( itAccel == (*pltAccel).end( ) ){	return E_HANDLE;	}

		//	該当要素を削除
		(*pltAccel).erase( itAccel );
		//	ホットキーコントロールを空に
		SendMessage( hHkcWnd, HKM_SETHOTKEY, 0, 0 );
		//	リストビューの表示も空にする
		ZeroMemory( atBuffer, sizeof(atBuffer) );
		ZeroMemory( &stLvi, sizeof(stLvi) );
		stLvi.mask     = LVIF_TEXT;	//	コマンドコードを確保する
		stLvi.iItem    = iItem;
		stLvi.iSubItem = 1;
		stLvi.pszText  = atBuffer;
		ListView_SetItem( hLvWnd, &stLvi );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	メニュー文字列に、アクセラーキー名称を割り当てていく
	@param[in]	hWnd		メニューのあるウインドウハンドル
	@param[in]	pstAccel	内容テーブル・無い時はNULL
	@param[in]	iEntry		テーブルのエントリ数
	@return		HRESULT		終了状態コード
*/
HRESULT AccelKeyMenuRewrite( HWND hWnd, LPACCEL pstAccel, CONST INT iEntry )
{
	HMENU		hMenu;
	WORD		dCmd;
	INT			i, j, iRslt;
	UINT		mRslt;
	UINT_PTR	d, cchSz;
	BOOLEAN		bModify;
	TCHAR		atBuffer[MAX_STRING], atBind[SUB_STRING];

	hMenu = GetMenu( hWnd );	//	サブメニューまで全部イケる

	for( i = 0; FULL_ITEMS > i; i++ )
	{
		//	関係無いやつは飛ばす
		if( 0 == gstContextItem[i].dCommandoID )	continue;

		dCmd = gstContextItem[i].dCommandoID;	//	基本的にこれでヒットするはず
		ZeroMemory( atBuffer, sizeof(atBuffer) );
		iRslt = GetMenuString( hMenu, dCmd, atBuffer, MAX_STRING, MF_BYCOMMAND );
		if( !(iRslt) )	continue;

		bModify = FALSE;

		//	先の内容を破壊する
		StringCchLength( atBuffer, MAX_STRING, &cchSz );
		for( d = 0; cchSz > d; d++ )
		{
			if( TEXT('\t') == atBuffer[d] )
			{
				atBuffer[d] = 0;
				bModify = TRUE;
				break;
			}
		}

		//	このコマンドのエントリーはあるか
		for( j = 0; iEntry > j; j++ )
		{
			if( dCmd == pstAccel[j].cmd )	//	あったら作成
			{
				ZeroMemory( atBind, sizeof(atBind) );
				AccelKeyBindString( &(pstAccel[j]), atBind, SUB_STRING );

				StringCchCat( atBuffer, MAX_STRING, TEXT("\t") );
				StringCchCat( atBuffer, MAX_STRING, atBind );

				bModify = TRUE;
				break;
			}
		}

		if( bModify )
		{
			//	必要に応じてチェック状態を確保する
			mRslt = GetMenuState( hMenu, dCmd, MF_BYCOMMAND );

			ModifyMenu( hMenu, dCmd, (MF_CHECKED & mRslt), dCmd, atBuffer );
			//	MF_BYCOMMAND | MF_STRING は両方０なので、必要なのはチェックの是非だけ
		}
	}

	DrawMenuBar( hWnd );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

#endif
