/*! @file
	@brief �c���[�r���[�Ƃ��C�ɓ���A�^�u�̐�������܂�
	���̃t�@�C���� MaaTreeView.cpp �ł��B
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

//	�^�u�ɂ��C�ɓ����o�^����ɂ�

/*
�c���c�ɂ��c���[�ǉ��B
�ǉ��f�B���N�g����p�ӁB�t�@�C���̃t���p�X�̕ێ��́H
��Table�p�ӂ���Ƃ��āAlParam�ɂQ��|�P���H
lParam�Ƃ�Ƃ���ł̐���������

�ȈՒǉ������Ƃ��A�č\�z������H�����̎��͏����Ă��ǂ����H
�ȈՒǉ���Z�߂Ă����f�B���N�g����p�ӁB�e�[�u�����₷���H�ʌŎ����Ă�����
�\�z���ɖ����Ȃ�Ȃ��E�I�������Ƃ��̃t�@�C���T�[�`�ǂ�����E�m�[�h��lParam�ɂȂɂ��H�ł��������H

�c���[�E�N���� �����ɃA�C�e���ǉ� ���A�G�L�X�g���t�@�C���p�̍폜�ɕύX����
IDM_MAA_IADD_OPEN
*/


//	TODO:	�f�B���N�g���̕�����J������̃A�C�R������

#ifdef EXTRA_NODE_STYLE
//�G�L�X�g���t�@�C���̎g�p�ǉ��̋�������������
//�G�L�X�g���t�@�C���ǉ������Ƃ��A�c���[�ւ̒ǉ����o���ĂȂ��E�`��H�ǉ����́H
//�G�L�X�g���t�@�C���̍폜 �����ɃA�C�e���ǉ� ��ύX���Ă��΂������낤
#endif

#include "stdafx.h"
#include "OrinrinEditor.h"
#include "MaaTemplate.h"
//-------------------------------------------------------------------------------------------------

#define NODE_DIR	1		//!<	�m�[�h�ŁA�f�B���N�g��������
#define NODE_FILE	0		//!<	�m�[�h�ŁA�t�@�C��������
#define NODE_EXTRA	(-1)	//!<	�m�[�h�ŁA�ǉ��p�f�B���N�g��������

#define TICO_DIR_CLOSE	0	//!<	�c���[�A�C�R���F�����f�B���N�g��
#define TICO_DIR_OPEN	1	//!<	�c���[�A�C�R���F�J�����f�B���N�g��
#define TICO_DIR_EXTRA	2	//!<	�c���[�A�C�R���F�ǉ��f�B���N�g��
#define TICO_FILE_AST	3	//!<	�c���[�A�C�R���F�`�r�s�t�@�C��
#define TICO_FILE_MLT	4	//!<	�c���[�A�C�R���F�l�k�s�t�@�C��
#define TICO_FILE_ETC	5	//!<	�c���[�A�C�R���F���̑��t�@�C��



//!	���^�u�̓��e�ێ�
typedef struct tagMULTIPLEMAA
{
	INT		dTabNum;				//!<	�^�u�̔ԍ��E�Q�C���f�b�N�X
	TCHAR	atFilePath[MAX_PATH];	//!<	�t�@�C���p�X�E��Ȃ�g�p����J����
	TCHAR	atBaseName[MAX_PATH];	//!<	�g�p���X�g�ɓ���鎞�̃O���[�v��
	TCHAR	atDispName[MAX_PATH];	//!<	�^�u�\���p����

	UINT	dLastTop;				//!<	���Ă�AA�̔ԍ�

} MULTIPLEMAA, *LPMULTIPLEMAA;

//-------------------------------------------------------------------------------------------------

extern  HWND		ghSplitaWnd;	//		�X�v���b�g�o�[�n���h��

extern HMENU		ghProfHisMenu;	//		����\�����镔���E���I�ɓ��e�쐬���Ȃ�����

static HFONT		ghTabFont;		//!<	�^�u�p�̃t�H���g�E��������߂̎�

static  HWND		ghTabWnd;		//!<	�I���^�u�̃n���h��

static  HWND		ghFavLtWnd;		//!<	�悭�g���z��o�^���郊�X�g�{�b�N�X

static  HWND		ghTreeWnd;		//!<	�c���[�̃n���h��
static HTREEITEM	ghTreeRoot;		//!<	�c���[�̃��[�g�A�C�e��

//static HIMAGELIST	ghImageList;	//!<	�c���[�r���[�ɂ�������C���[�W���X�g

static TCHAR		gatAARoot[MAX_PATH];	//!<	�`�`�f�B���N�g���̃J�����g
static TCHAR		gatBaseName[MAX_PATH];	//!<	�g�p���X�g�ɓ���鎞�̃O���[�v��

static INT			gixUseTab;		//!<	���J���Ă�́E�O�c���[�@�P���C�ɓ���@�Q�`�����t�@�C��
//	�^�u�ԍ��ł��邱�Ƃɒ��ӁE�����t�@�C�����X�g�̊����ԍ��ł͂Ȃ�

#ifdef HUKUTAB_DRAGMOVE
static POINT		gstMouseDown;		//!<	�}�E�X�{�^���������ꂽ�ʒu
static INT			giDragSel;			//!<	���������Ƃ����^�u�̔ԍ�
static BOOLEAN		gbTabDraging;		//!<	�^�u���h���b�O���Ă�
#endif

static WNDPROC	gpfOriginFavListProc;	//!<	�g�p���X�g�̌��v���V�[�W��
static WNDPROC	gpfOriginTreeViewProc;	//!<	�c���[�r���[�̌��v���V�[�W��
static WNDPROC	gpfOriginTabMultiProc;	//!<	�^�u�̌��v���V�[�W��


static list<MULTIPLEMAA>	gltMultiFiles;			//!<	���^�u�ŊJ���Ă���t�@�C���̕ێ�
typedef  list<MULTIPLEMAA>::iterator	MLTT_ITR;	//!<	���^�u���X�g�̃C�e���[�^
//-------------------------------------------------------------------------------------------------

HRESULT	TreeItemFromSqlII( HTREEITEM  );			//!<	�f�B���N�g���ƃt�@�C�����r�p�k����c���[�r���[�ɓW�J

#ifdef EXTRA_NODE_STYLE
UINT	TreeNodeExtraAdding( LPCTSTR  );			//!<	�G�L�X�g���t�@�C����ǉ�����
HRESULT	TreeExtraItemFromSql( HTREEITEM, UINT );	//!<	�G�L�X�g���t�@�C�����r�p�k����c���[�r���[�ɓW�J
#endif

VOID	Mtv_OnMButtonUp( HWND, INT, INT, UINT );	//!<	�c���[�r���[�Ń}�E�X�̒��o���@�����������ꂽ���̏���
VOID	Mtv_OnDropFiles( HWND , HDROP );			//!<	�c���[�r���[�Ƀh���b�O���h���b�v���ꂽ�Ƃ��̏���

HRESULT	TabMultipleRestore( HWND  );				//!<	�����t�@�C����INI����ǂݍ���ōēW�J����
INT		TabMultipleSelect( HWND, INT, UINT );		//!<	���^�u����I�������ꍇ
//INT	TabMultipleOpen( HWND , HTREEITEM );		//
HRESULT	TabMultipleDelete( HWND, CONST INT );		//!<	�w��̃^�u�����
INT		TabMultipleAppend( HWND );					//!<	�^�u�𑝂₷

HRESULT	TabMultipleNameChange( HWND , INT );		//!<	�^�u���O�ύX�_�C�����O�J��

HRESULT	TabLineMultiSingleToggle( HWND );			//!<	�^�u�̑��i�\���E��s�\����؂�ւ���

UINT	TabMultipleIsFavTab( INT, LPTSTR, UINT_PTR );	//!<	���^�u�͂��C�Ƀ��X�g�̂ł��邩

LRESULT	CALLBACK gpfFavListProc(  HWND , UINT, WPARAM, LPARAM );	//!<	�g�p���X�g�̃T�u�N���X�v���V�[�W��
LRESULT	CALLBACK gpfTreeViewProc( HWND , UINT, WPARAM, LPARAM );	//!<	�c���[�r���[�̃T�u�N���X�v���V�[�W��
LRESULT	CALLBACK gpfTabMultiProc( HWND , UINT, WPARAM, LPARAM );	//!<	�^�u�̃T�u�N���X�v���V�[�W��

VOID	Mtb_OnMButtonUp( HWND, INT, INT, UINT );	//!<	

VOID	Mtb_OnLButtonDblclk( HWND, BOOL, INT, INT, UINT );	//!<	

#ifdef HUKUTAB_DRAGMOVE
VOID	TabMultipleOnLButtonDown( HWND, INT, INT, UINT );	//!<	
VOID	TabMultipleOnMouseMove(   HWND, INT, INT, UINT );	//!<	
VOID	TabMultipleOnLButtonUp(   HWND, INT, INT, UINT );	//!<	
#endif
//-------------------------------------------------------------------------------------------------

/*!
	�c���[�r���[�E�\���I���^�O�Ƃ������
	@param[in]	hWnd	�e�E�C���h�E�n���h���ENULL�Ŕj�󏈗�
	@param[in]	hInst	�A�v���̎���
	@param[in]	ptRect	���C���E�C���h�E�̈ʒu�Ƒ傫��
	@return		HRESULT	�I����ԃR�[�h
*/
HRESULT TreeInitialise( HWND hWnd, HINSTANCE hInst, LPRECT ptRect )
{
	TCITEM		stTcItem;
	RECT		itRect, clRect;

//	SHFILEINFO	stShFileInfo;

	DWORD		dwStyles;

	HIMAGELIST	hTreeImgList;				//!<	
	HICON	hIcon;


	//	�j�󂷂�Ƃ�
	if( !(hWnd) )
	{
		SetWindowFont( ghTabWnd, GetStockFont(DEFAULT_GUI_FONT), FALSE );
		DeleteFont( ghTabFont );

		//	�J���Ă镛�^�u��INI�ɕۑ�
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

//�\���I���^�u
	dwStyles = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | TCS_RIGHTJUSTIFY;
	//	���i���[�h�Ȃ�A�X�^�C�����������Ă���
	if( !(InitParamValue( INIT_LOAD, VL_MAATAB_SNGL, 0 )) ){	dwStyles |= TCS_MULTILINE;	}

	ghTabWnd = CreateWindowEx( 0, WC_TABCONTROL, TEXT("treetab"), dwStyles, 0, 0, TREE_WIDTH, 0, hWnd, (HMENU)IDTB_TREESEL, hInst, NULL );
	SetWindowFont( ghTabWnd, ghTabFont, FALSE );

	ZeroMemory( &stTcItem, sizeof(stTcItem) );
	stTcItem.mask = TCIF_TEXT;
	stTcItem.pszText = TEXT("�S��");	TabCtrl_InsertItem( ghTabWnd, 0, &stTcItem );
	stTcItem.pszText = TEXT("�g�p");	TabCtrl_InsertItem( ghTabWnd, 1, &stTcItem );

	//	�I�΂ꂵ�t�@�C�����^�u�I�ɒǉ��H�@�^�u���̓E�C���h�E��

	TabCtrl_GetItemRect( ghTabWnd, 1, &itRect );
	itRect.bottom -= itRect.top;

	itRect.right -= itRect.left;
	itRect.top  = 0;
	itRect.left = 0;
	TabCtrl_AdjustRect( ghTabWnd, 0, &itRect );

	MoveWindow( ghTabWnd, 0, 0, clRect.right, itRect.top, TRUE );

	//	�T�u�N���X��
	gpfOriginTabMultiProc = SubclassWindow( ghTabWnd, gpfTabMultiProc );

//���C�ɓ���p���X�g�{�b�N�X
	ghFavLtWnd = CreateWindowEx( WS_EX_CLIENTEDGE, WC_LISTBOX, TEXT("favlist"),
		WS_CHILD | WS_VSCROLL | LBS_NOTIFY | LBS_SORT | LBS_NOINTEGRALHEIGHT,
		0, itRect.bottom, TREE_WIDTH, ptRect->bottom-itRect.bottom-1, hWnd, (HMENU)IDLB_FAVLIST, hInst, NULL );

	//	�T�u�N���X��
	gpfOriginFavListProc = SubclassWindow( ghFavLtWnd, gpfFavListProc );


//�S�`�`���X�g�c���[
//	ghImageList = (HIMAGELIST)SHGetFileInfo( TEXT(""), 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON) );

	ghTreeWnd = CreateWindowEx( WS_EX_CLIENTEDGE | WS_EX_ACCEPTFILES, WC_TREEVIEW, TEXT("itemtree"),
		WS_VISIBLE | WS_CHILD | TVS_DISABLEDRAGDROP | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS,
		0, itRect.bottom, TREE_WIDTH, ptRect->bottom-itRect.bottom-1, hWnd, (HMENU)IDTV_ITEMTREE, hInst, NULL );
//	TreeView_SetImageList( ghTreeWnd, ghImageList, TVSIL_NORMAL );

	//	�T�u�N���X��
	gpfOriginTreeViewProc = SubclassWindow( ghTreeWnd, gpfTreeViewProc );

	//	�A�C�R����������
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
	���C�Ƀ��X�g�̃T�u�N���X�v���V�[�W��
	@param[in]	hWnd	���X�g�̃n���h��
	@param[in]	msg		�E�C���h�E���b�Z�[�W�̎��ʔԍ�
	@param[in]	wParam	�ǉ��̏��P
	@param[in]	lParam	�ǉ��̏��Q
	@return	�������ʂƂ�
*/
LRESULT CALLBACK gpfFavListProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	UINT	ulRslt;

	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_CHAR,    Maa_OnChar  );	//	
		HANDLE_MSG( hWnd, WM_COMMAND, Maa_OnCommand );	//	�A�N�Z�����[�^�p

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
	�c���[�r���[�̃T�u�N���X�v���V�[�W��
	@param[in]	hWnd	���X�g�̃n���h��
	@param[in]	msg		�E�C���h�E���b�Z�[�W�̎��ʔԍ�
	@param[in]	wParam	�ǉ��̏��P
	@param[in]	lParam	�ǉ��̏��Q
	@return	�������ʂƂ�
*/
LRESULT CALLBACK gpfTreeViewProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	UINT	ulRslt;

	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_CHAR,      Maa_OnChar  );		//	
		HANDLE_MSG( hWnd, WM_COMMAND,   Maa_OnCommand );	//	�A�N�Z�����[�^�p

		HANDLE_MSG( hWnd, WM_KEYDOWN,   Aai_OnKey );		//	20120221
		HANDLE_MSG( hWnd, WM_KEYUP,     Aai_OnKey );		//	

		HANDLE_MSG( hWnd, WM_MBUTTONUP, Mtv_OnMButtonUp );	//	
		HANDLE_MSG( hWnd, WM_DROPFILES, Mtv_OnDropFiles );	//	�h���O���h���b�v�̎�t

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
	�c���[�r���[�Ń}�E�X�̒��o���@�����������ꂽ��
	@param[in]	hWnd	�c���[�r���[�̃n���h��
	@param[in]	x		�N���b�N���ꂽ�N���C�����g�w�ʒu
	@param[in]	y		�N���b�N���ꂽ�N���C�����g�x�ʒu
	@param[in]	flags	
*/
VOID Mtv_OnMButtonUp( HWND hWnd, INT x, INT y, UINT flags )
{
	INT	iRslt;
	HTREEITEM	hTreeItem;
	TVHITTESTINFO	stTvItemInfo;

	TRACE( TEXT("�c���[�r���[�Œ��N���b�N[%d x %d]"), x, y );

	ZeroMemory( &stTvItemInfo, sizeof(TVHITTESTINFO) );
	stTvItemInfo.pt.x = x;
	stTvItemInfo.pt.y = y;

	//	�Y������A�C�e�����m�ۂ���
	hTreeItem = TreeView_HitTest( ghTreeWnd, &stTvItemInfo );

	//	���삷��
	iRslt = TreeSelItemProc( GetParent( hWnd ), hTreeItem, 1 );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	�h���b�O���h���b�v�̎󂯓���
	@param[in]	hWnd	�e�E�C���h�E�̃n���h��
	@param[in]	hDrop	�h���b�s���I�u�W�F�N�g�n���h�D
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

	if( bRslt ){	 return;	}	//	�t�@�C���Ŗ����̂Ȃ牽�����Ȃ�

#ifdef EXTRA_NODE_STYLE

	//	SQL�ɒǉ����āA�c���[�ɒǉ�����
	TreeNodeExtraAdding( atFileName );

#endif

	return;
}
//-------------------------------------------------------------------------------------------------



/*!
	�}���`�v���^�u�̃T�u�N���X�v���V�[�W��
	@param[in]	hWnd	���X�g�̃n���h��
	@param[in]	msg		�E�C���h�E���b�Z�[�W�̎��ʔԍ�
	@param[in]	wParam	�ǉ��̏��P
	@param[in]	lParam	�ǉ��̏��Q
	@return	�������ʂƂ�
*/
LRESULT	CALLBACK gpfTabMultiProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_CHAR,    Maa_OnChar  );	
		HANDLE_MSG( hWnd, WM_COMMAND, Maa_OnCommand );	//	�A�N�Z�����[�^�p

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
	�^�u���_�u���N���b�N�����Ƃ�
	@param[in]	hWnd			�E�C���h�E�n���h��
	@param[in]	fDoubleClick	��O�_�u���N���b�N
	@param[in]	x				���������N���C�����g�w���W�l
	@param[in]	y				���������N���C�����g�x���W�l
	@param[in]	keyFlags		���ɉ�����Ă�L�[�ɂ���
*/
VOID Mtb_OnLButtonDblclk( HWND hWnd, BOOL fDoubleClick, INT x, INT y, UINT keyFlags )
{
	INT	curSel;
	//TCHITTESTINFO	stTcHitInfo;

	//stTcHitInfo.pt.x = x;
	//stTcHitInfo.pt.y = y;
	//curSel = TabCtrl_HitTest( ghTabWnd, &stTcHitInfo );
	//���̂Ƃ��A�N�e�B�u�ɂȂ��Ă���^�u��I������
	curSel = TabCtrl_GetCurSel( ghTabWnd );

	TRACE( TEXT("TAB DBLCLICK [%d] [%d x %d]"), curSel, x, y );

	if( 1 >= curSel ){	 return;	}	//	�^�u�@�O�c���[�A�P�g�p�̂Ƃ��͉������Ȃ�

	TabMultipleNameChange( hWnd, curSel );	//	���̕ύX

#pragma message ("�_�u���N���b�N�̋@�\��ݒ�o����悤�ɂ���Ƃ�����������")

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	���^�u�Ń}�E�X�̒��{�^�����������ꂽ�Ƃ�
	@param[in]	hWnd		�E�C���h�E�n���h��
	@param[in]	x			���������N���C�����g�w���W�l
	@param[in]	y			���������N���C�����g�x���W�l
	@param[in]	keyFlags	���ɉ�����Ă�L�[�ɂ���
*/
VOID Mtb_OnMButtonUp( HWND hWnd, INT x, INT y, UINT flags )
{
	INT	curSel;
	TCHITTESTINFO	stTcHitInfo;

	stTcHitInfo.pt.x = x;
	stTcHitInfo.pt.y = y;
	curSel = TabCtrl_HitTest( ghTabWnd, &stTcHitInfo );
	//	�^�u�@�O�c���[�A�P�g�p�̂Ƃ��͉������Ȃ�

	TRACE( TEXT("MTAB start TAB [%d] [%d x %d]"), curSel, x, y );

	if( 1 >= curSel ){	 return;	}

	TabMultipleDelete( GetParent( ghTabWnd ), curSel );

	return;
}
//-------------------------------------------------------------------------------------------------

#ifdef HUKUTAB_DRAGMOVE

/*!
	���^�u�Ń}�E�X�̍��{�^�����_�E�����ꂽ�Ƃ�
	@param[in]	hWnd		�E�C���h�E�n���h��
	@param[in]	x			���������N���C�����g�w���W�l
	@param[in]	y			���������N���C�����g�x���W�l
	@param[in]	keyFlags	���ɉ�����Ă�L�[�ɂ���
*/
VOID TabMultipleOnLButtonDown( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	TCHITTESTINFO	stTcHitInfo;

	TRACE( TEXT("MTAB LDOWN [%d x %d]"), x, y );

	gstMouseDown.x = x;	//	�_�E�������ʒu����_�Ƃ���
	gstMouseDown.y = y;

	stTcHitInfo.pt = gstMouseDown;
	giDragSel = TabCtrl_HitTest( ghTabWnd, &stTcHitInfo );
	//	�^�u�@�O�c���[�A�P�g�p�̂Ƃ��͉������Ȃ�

	TRACE( TEXT("MTAB start TAB [%d]"), giDragSel );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	���^�u�Ń}�E�X�𓮂������Ƃ�
	@param[in]	hWnd		�E�C���h�E�n���h��
	@param[in]	x			���������N���C�����g�w���W�l
	@param[in]	y			���������N���C�����g�x���W�l
	@param[in]	keyFlags	���ɉ�����Ă�L�[�ɂ���
*/
VOID TabMultipleOnMouseMove( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	INT	mx, my, rx, ry;

	//	�^�u�@�O�c���[�A�P�g�p�̂Ƃ��͉������Ȃ�
	if( 1 >= giDragSel )	return;

	if( (keyFlags & MK_LBUTTON) && !(gbTabDraging) )
	{
		//	��_����̈ړ��ʂ̐�Βl���m�F
		mx = abs( gstMouseDown.x - x );
		my = abs( gstMouseDown.y - y );

		rx = GetSystemMetrics( SM_CXDRAG );
		ry = GetSystemMetrics( SM_CYDRAG );


		//	����ʈړ�������h���b�O�J�n�Ƃ���
		if( rx < mx || ry < my )
		{
			TRACE( TEXT("MTAB start DRAG [%d x %d] [%d x %d]"), rx, ry, mx, my );
			SetCapture( hWnd  );	//	�}�E�X�L���v�`��
			gbTabDraging = TRUE;
		}

	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	���^�u�Ń}�E�X�̍��{�^�������������ꂽ�Ƃ�
	@param[in]	hWnd			�E�C���h�E�n���h��
	@param[in]	x				���������N���C�����g�w���W�l
	@param[in]	y				���������N���C�����g�x���W�l
	@param[in]	keyFlags		���ɉ�����Ă�L�[�ɂ���
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
	MAA�̂ǂ����ŕ����L�[�I�T��������
	@param[in]	hWnd	�E�C���h�E�n���h���E�������ɒ���
	@param[in]	ch		�����ꂽ����
	@param[in]	cRepeat	�L�[���s�[�g�񐔁E�����ĂȂ��H
	@return		����
*/
VOID Maa_OnChar( HWND hWnd, TCHAR ch, INT cRepeat )
{
	BOOLEAN	bShift;
	NMHDR	stNmHdr;
	INT		iTabs, iTarget;

	bShift = (0x8000 & GetKeyState(VK_SHIFT)) ? TRUE : FALSE;

	TRACE( TEXT("CHAR[%04X][%d]"), ch, bShift );

	if( VK_RETURN == ch )	//	�G���^�[�����ꂽ��
	{
		AaItemsDoSelect( hWnd, MAA_DEFAULT, FALSE );
		return;
	}

	if( VK_TAB != ch ){	return;	}	//	�^�u�ȊO�͉�������

	iTabs = TabCtrl_GetItemCount( ghTabWnd );

	if( bShift )	//	�t��
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
	�R���e�L�X�g���j���[�Ăт����A�N�V����(�v�͉E�N���b�N�j
	@param[in]	hWnd		�E�C���h�E�n���h��
	@param[in]	hWndContext	�R���e�L�X�g�����������E�C���h�E�̃n���h��
	@param[in]	xPos		�X�N���[���w���W
	@param[in]	yPos		�X�N���[���x����
	@return		����
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

	stPost.x = (SHORT)xPos;	//	��ʍ��W�̓}�C�i�X�����肤��
	stPost.y = (SHORT)yPos;

	TRACE( TEXT("MAA�R���e�L�X�g���j���[") );

	//	���C�Ƀ��X�g�{�b�N�X�̃R���e�L�X�g
	if( ghFavLtWnd == hWndContext )
	{
		ZeroMemory( atSelName, sizeof(atSelName) );
		ZeroMemory( atMenuStr, sizeof(atMenuStr) );

		curSel = ListBox_GetCurSel( ghFavLtWnd );
		TRACE( TEXT("���X�g�{�b�N�X�R���e�L�X�g %d"), curSel );
		if( 0 > curSel )	return;

		ListBox_GetText( ghFavLtWnd, curSel, atSelName );
		StringCchPrintf( atMenuStr,  MAX_PATH, TEXT("[ %s ]�ŕ��^�u��ǉ�"), atSelName );
		StringCchPrintf( atMenuStr2, MAX_PATH, TEXT("[ %s ]�O���[�v���폜"), atSelName );

		//	���j���[�͏�ɓ��I�ɍ쐬����
		hMenu = CreatePopupMenu(  );
		//	���C�Ƀ��X�g�̃Z�b�g�𕛃^�u�ɕ\������@�\
		AppendMenu( hMenu, MF_STRING, IDM_AATREE_SUBADD, atMenuStr );
		AppendMenu( hMenu, MF_SEPARATOR, 0, TEXT("----") );
		AppendMenu( hMenu, MF_STRING, IDM_MAA_FAVFLDR_DELETE, atMenuStr2 );

		dRslt = TrackPopupMenu( hMenu, TPM_RETURNCMD, stPost.x, stPost.y, 0, hWnd, NULL );
		switch( dRslt )
		{
			case IDM_AATREE_SUBADD:
				ZeroMemory( &stMulti, sizeof(MULTIPLEMAA) );
				StringCchCopy( stMulti.atBaseName, MAX_PATH, atSelName );
				//	atFilePath����ɂ��邱�ƂŁA�g�p���X�g������Ă��Ƃ�
				stMulti.dTabNum = 0;	//	�������E�����͂Q�ȍ~

				gltMultiFiles.push_back( stMulti );
				TabMultipleAppend( hWnd );
				break;

			case IDM_MAA_FAVFLDR_DELETE:	//	���C�ɓ��胊�X�g����_���ƍ폜
				SqlFavFolderDelete( atSelName );
				//	�ĕ`��
				while( ListBox_GetCount( ghFavLtWnd ) ){	ListBox_DeleteString( ghFavLtWnd, 0 );	}
				SqlFavFolderEnum( FavListFolderNameBack );
				break;

			default:	break;
		}
		DestroyMenu( hMenu );
		return;
	}

	//	�c���[�r���[�̃R���e�L�X�g
	if( ghTreeWnd == hWndContext )
	{
		hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_AATREE_POPUP) );
		hSubMenu = GetSubMenu( hMenu, 0 );

		stTvHitInfo.pt = stPost;
		ScreenToClient( ghTreeWnd, &(stTvHitInfo.pt) );
		hTvHitItem = TreeView_HitTest( ghTreeWnd, &stTvHitInfo );

		if( hTvHitItem )
		{
			//	�I�����ꂽ��̃t�@�C�����A�������̓f�B���N�g�����m��
			lPrm = TreeItemInfoGet( hTvHitItem, atName, MAX_PATH );
			//����	lParam�̔��f

#ifdef EXTRA_NODE_STYLE
			//	�ǂ�ł��Ȃ��̂Ȃ�A�G�L�X�g���t�@�C���Ȃ̂ŁA�t�@�C���ɂ��Ă����E��������
			if( NODE_DIR != lPrm && NODE_FILE != lPrm && NODE_EXTRA != lPrm )
			{
				iSelID = lPrm;
				lPrm = NODE_FILE;
				EnableMenuItem( hSubMenu, IDM_MAA_ITEM_DELETE, MF_ENABLED );
				//	�G�L�X�g���t�@�C���̍폜��L���ɂ���
			}
#endif

			StringCchCat( atName, MAX_PATH, TEXT(" �̑���") );
			//	���̂𖾎����Ă���
			ModifyMenu( hSubMenu, IDM_DUMMY, MF_BYCOMMAND | MF_STRING | MF_GRAYED, IDM_DUMMY, atName );
		}

		if( NODE_FILE != lPrm || !(hTvHitItem) )	//	�t�@�C���łȂ����A���I���Ȃ�
		{
			EnableMenuItem( hSubMenu, IDM_AATREE_MAINOPEN, MF_GRAYED );
			EnableMenuItem( hSubMenu, IDM_AATREE_SUBADD,   MF_GRAYED );
			EnableMenuItem( hSubMenu, IDM_AATREE_GOEDIT,   MF_GRAYED );
		//	EnableMenuItem( hSubMenu, IDM_MAA_IADD_OPEN,   MF_GRAYED );	//	�L�����Z�����ꂽ
		}
		//	�v���t���̃t�@�C�����폜�o����悤�ɂ��Ă������H

		//	�v���t�������
		ModifyMenu( hSubMenu, IDM_OPEN_HISTORY, MF_BYCOMMAND | MF_POPUP, (UINT_PTR)ghProfHisMenu, TEXT("�t�@�C���g�p����(&H)") );
	//	ModifyMenu( hSubMenu, 2, MF_BYPOSITION | MF_POPUP, (UINT_PTR)ghProfHisMenu, TEXT("�t�@�C���g�p����(&H)") );
		//	�|�b�v�A�b�v�ւ̕ύX�́APosition�w��łȂ��Əo���Ȃ��H

#ifdef _ORRVW
		rdExStyle = GetWindowLongPtr( hWnd, GWL_EXSTYLE );
		if( WS_EX_TOPMOST & rdExStyle ){	CheckMenuItem( hSubMenu , IDM_TOPMOST_TOGGLE, MF_BYCOMMAND | MF_CHECKED );	}
#endif


		//	�E�N���ł̓m�[�h�I������Ȃ��悤��
		dRslt = TrackPopupMenu( hSubMenu, TPM_RETURNCMD, stPost.x, stPost.y, 0, hWnd, NULL );	//	TPM_CENTERALIGN | TPM_VCENTERALIGN | 
		RemoveMenu( hSubMenu, 2, MF_BYPOSITION );
		DestroyMenu( hMenu );	//���̃f�X�g���C�Ń|�b�v�A�b�v�܂Ŕj�������̂ŁAremove���Ă���
	
		switch( dRslt )
		{
			//	�v���t�t�@�C���J��
			case IDM_MAA_PROFILE_MAKE:	TreeProfileOpen( hWnd );	break;

			//	�f�B���N�g���n���ăZ�b�g	�f�B���N�g���ݒ�_�C�����O���J��
			case IDM_TREE_RECONSTRUCT:	TreeProfileRebuild( hWnd  );	break;

			case IDM_FINDMAA_DLG_OPEN:	TreeMaaFileFind( hWnd );	break;

			case IDM_AATREE_MAINOPEN:	TreeSelItemProc( hWnd, hTvHitItem , 0 );	break;

			case  IDM_AATREE_SUBADD:	TreeSelItemProc( hWnd, hTvHitItem , 1 );	break;

#ifndef _ORRVW
			case  IDM_AATREE_GOEDIT:	TreeSelItemProc( hWnd, hTvHitItem , 2 );	break;
  #ifndef MAA_IADD_PLUS
		//	case  IDM_MAA_IADD_OPEN:	TreeSelItemProc( hWnd, hTvHitItem , 3 );	break;
			//�L�����Z�����ꂽ
  #endif
#endif
#ifdef EXTRA_NODE_STYLE
			case IDM_MAA_ITEM_DELETE:	TreeSelItemProc( hWnd, hTvHitItem , 4 );	break;
#endif
			//	�t�@�C���I�[�|�������N�����[
			case IDM_OPEN_HIS_CLEAR:	OpenProfileLogging( hWnd, NULL );	break;

			default:
				//	�t�@�C���I�[�|������
				if( IDM_OPEN_HIS_FIRST <= dRslt && dRslt <= IDM_OPEN_HIS_LAST )
				{
					OpenProfileLoad( hWnd, dRslt );
				}
#ifdef _ORRVW	//	���̑��̓��C���R�}���h�ɉ�
				else
				{
					Maa_OnCommand( hWnd, dRslt, hWndContext, 0 );
				}
#endif
				break;
		}

		return;
	}

	//	�^�u�o�[�̃R���e�L�X�g
	if( ghTabWnd == hWndContext )
	{
		stTcHitInfo.pt = stPost;
		ScreenToClient( ghTabWnd, &(stTcHitInfo.pt) );
		curSel = TabCtrl_HitTest( ghTabWnd, &stTcHitInfo );

		//	�Œ�̓�̏ꍇ�͖���
		if( 1 >= curSel )	return;
#pragma message ("MAA�^�u�̌Œ�^�u�̔��ʂɒ���")

		hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_AATABS_POPUP) );
		hSubMenu = GetSubMenu( hMenu, 0 );

		ZeroMemory( &stTcItem, sizeof(TCITEM) );
		stTcItem.mask       = TCIF_TEXT;
		stTcItem.pszText    = atText;
		stTcItem.cchTextMax = MAX_PATH;
		TabCtrl_GetItem( ghTabWnd, curSel, &stTcItem );

		StringCchCat( atText, MAX_PATH, TEXT(" �����(&Q)") );
		StringCchLength( atText, MAX_PATH, &cchSize );

		ZeroMemory( &stMenuItemInfo, sizeof(MENUITEMINFO) );
		stMenuItemInfo.cbSize     = sizeof(MENUITEMINFO);
		stMenuItemInfo.fMask      = MIIM_TYPE;
		stMenuItemInfo.fType      = MFT_STRING;
		stMenuItemInfo.cch        = cchSize;
		stMenuItemInfo.dwTypeData = atText;
		SetMenuItemInfo( hSubMenu, IDM_AATABS_DELETE, FALSE, &stMenuItemInfo );

		//�����A���C�Ƀ^�u�Ȃ�A�ҏW�ŊJ���͖����ɂ���
		if( TabMultipleIsFavTab( curSel, NULL, 0 ) ){	EnableMenuItem( hSubMenu, IDM_AATREE_GOEDIT, MF_GRAYED );	}

		//	������s�\���X�^�C���Ȃ�A�`�F�b�N�}�[�N����Ƃ�
		dwStyles = GetWindowStyle( ghTabWnd );
		if( !(TCS_MULTILINE & dwStyles) ){	CheckMenuItem( hSubMenu, IDM_AATABS_SINGLETAB, MF_CHECKED );	}

		dRslt = TrackPopupMenu( hSubMenu, TPM_RETURNCMD, stPost.x, stPost.y, 0, hWnd, NULL );
		DestroyMenu( hMenu );
		switch( dRslt )
		{
			case  IDM_AATABS_DELETE:	TabMultipleDelete( hWnd, curSel );	break;
			case  IDM_AATREE_GOEDIT:	TabMultipleSelect( hWnd, curSel, 1 );	break;
			//	�c���[���Ƃ̓A�v���[�`���Ⴄ���璍��
			case  IDM_AATABS_ALLDELETE:	
				iRslt = MessageBox( hWnd, TEXT("�S�Ă̕��^�u����悤�Ƃ��Ă��B\r\n�{���ɕ�������Ă��������H"), TEXT("���ӂ���̊m�F"), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 );
				if( IDYES == iRslt ){	TabMultipleDeleteAll( hWnd );	}

			//	���l�[��
			case IDM_AATABS_RENAME:	TabMultipleNameChange( hWnd, curSel );	break;

			//	���^�u�́A���i�E�V���O���ؑ�	20130521
			case IDM_AATABS_SINGLETAB:	TabLineMultiSingleToggle( hWnd );	break;

			default:	break;
		}

		return;
	}

#ifndef _ORRVW
	//	����ȊO�̏ꏊ�̃|�b�p�b�v���j���[�E��Ɏ�O�ɕ\���̃A��
	hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_TEMPLATE_POPUP) );
	hSubMenu = GetSubMenu( hMenu, 0 );

	rdExStyle = GetWindowLongPtr( hWnd, GWL_EXSTYLE );
	if( WS_EX_TOPMOST & rdExStyle ){	CheckMenuItem( hSubMenu , IDM_TOPMOST_TOGGLE, MF_BYCOMMAND | MF_CHECKED );	}

	dRslt = TrackPopupMenu( hSubMenu, 0, stPost.x, stPost.y, 0, hWnd, NULL );
	//	�I�������łO���|�P�H�A�I�������炻�̃��j���[�̂h�c��WM_COMMAND�����s
	DestroyMenu( hMenu );
#endif
	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	��ʃT�C�Y���ς�����̂ŃT�C�Y�ύX
	@param[in]	hWnd	�e�E�C���h�E�n���h��
	@param[in]	ptRect	MAA�E�C���h�E�̑傫���E�����̓X�e�[�^�X�o�[�ƃ^�u�o�[�̍l������
	@return		HRESULT	�I����ԃR�[�h
*/
HRESULT TreeResize( HWND hWnd, LPRECT ptRect )
{
	RECT	rect, sptRect;

	//	�^�u�o�[�̕����C��
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
	�J�����g�_�f�B���N�g�����󂯎���āA�c���[���A�b�Z���u���[����
	@param[in]	hWnd		�e�E�C���h�E�n���h��
	@param[in]	ptCurrent	�J�����g�f�B���N�g����
	@param[in]	bSubTabReb	��O�ŕ��^�u����
	@return		HRESULT		�I����ԃR�[�h
*/
HRESULT TreeConstruct( HWND hWnd, LPCTSTR ptCurrent, BOOLEAN bSubTabReb )
{
	TVINSERTSTRUCT	stTreeIns;
//	SHFILEINFO	stShFileInfo;
	TCHAR	atRoote[MAX_PATH];

	ZeroMemory( gatAARoot, sizeof(gatAARoot) );
	StringCchCopy( gatAARoot, MAX_PATH, ptCurrent );

	StringCchPrintf( atRoote, MAX_PATH, TEXT("ROOT[%s]"), gatAARoot );

	StatusBarMsgSet( SBMAA_FILENAME, TEXT("�c���[���\�z���ł�") );

	TreeView_DeleteAllItems( ghTreeWnd );	//	�A�C�e���S�j��
	//	���[�g�A�C�e�����
	ZeroMemory( &stTreeIns, sizeof(TVINSERTSTRUCT) );
	stTreeIns.hParent        = TVI_ROOT;
	stTreeIns.hInsertAfter   = TVI_SORT;
	stTreeIns.item.mask      = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
	stTreeIns.item.pszText   = atRoote;//TEXT("ROOT");
	stTreeIns.item.lParam    = NODE_DIR;	//	�P�f�B���N�g���@�O�t�@�C��
	stTreeIns.item.cChildren = 1;
	//	���[�g�̂h�c�͂O

	//SHGetFileInfo( TEXT(""), 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON) );
	stTreeIns.item.iImage = I_IMAGECALLBACK;//stShFileInfo.iIcon;
	//SHGetFileInfo( TEXT(""), 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON) );
	stTreeIns.item.iSelectedImage = I_IMAGECALLBACK;//stShFileInfo.iIcon;

	ghTreeRoot = TreeView_InsertItem( ghTreeWnd, &stTreeIns );

	//	�f�B���N�g���w�肪����������I���
	if( 0 == ptCurrent[0] )
	{
		StatusBarMsgSet( SBMAA_FILENAME, TEXT("") );
		return E_INVALIDARG;
	}

	//	�J�����g�_�f�B���N�g���̓t���p�X�̂͂�

	//	�v���t�@�C�����[�h�Ȃ�A���SQL����ł���

	//	SQL����W�J�@�����ł͓W�J���Ȃ�

	StatusBarMsgSet( SBMAA_FILENAME, TEXT("") );
	TreeView_Expand( ghTreeWnd, ghTreeRoot, TVE_EXPAND );

	//	���^�u��SQL����č\�z
	if( bSubTabReb ){	TabMultipleRestore( hWnd  );	}	//	�I�����̕��^�u�𕜋A����

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	�c���[�m�[�h�̃t���p�X���m��
	@param[in]	hNode	�Ώۂ̃c���[�m�[�h
	@param[out]	ptPath	�t���p�X�����o�b�t�@�EMAX_PATH�ł��邱��
*/
UINT TreeNodePathGet( HTREEITEM hNode, LPTSTR ptPath )
{
	UINT	i;
	TCHAR	atName[MAX_PATH], atPath[MAX_PATH];
	HTREEITEM	hParent;


	if( ghTreeRoot == hNode )	//	���[�g���g�ł������ꍇ�͂����Ԃ��Ă���
	{
		StringCchCopy( ptPath, MAX_PATH, gatAARoot );
		return 1;
	}

	//	�I�����ꂽ��̃t�@�C�����A�������̓f�B���N�g�����m��
	TreeItemInfoGet( hNode, atName, MAX_PATH );

	//	��ɒH���āA�t�@�C���p�X�����
	for( i = 0; 12 > i; i++ )	//	�P�Q��蕅�C�K�w�͂��ǂ�Ȃ�
	{
		hParent = TreeView_GetParent( ghTreeWnd, hNode );
		if( !(hParent) )	return 0;	//	���[�g�̏�͂Ȃ�
		if( ghTreeRoot == hParent ){	break;	}	//	���������[�g�܂ŃC�b����I���

		TreeItemInfoGet( hParent, atPath, MAX_PATH );

		//	���̃p�X���L�^���Ă����ƁA�ŏI�I�Ƀ��[�g�����̃f�B���N�g�����ɂȂ�
		PathAppend( atPath, atName );
		StringCchCopy( atName, MAX_PATH, atPath );

		hNode = hParent;
	}

	//	���[�g�ʒu���������ăt���p�X�ɂ���
	StringCchCopy( atPath, MAX_PATH, gatAARoot );
	PathAppend( atPath, atName );

	StringCchCopy( ptPath, MAX_PATH, atPath );

	return 1;
}
//-------------------------------------------------------------------------------------------------

#ifdef EXTRA_NODE_STYLE

/*!
	�G�L�X�g���t�@�C����ǉ�����
	@param[out]	ptPath	�Ώۃt�@�C���̃t���p�X
	@return	�ǉ������t�@�C����SqlID�E�o�^�o���Ȃ�������O
*/
UINT TreeNodeExtraAdding( LPCTSTR ptPath )
{
	UINT	id;
	LPARAM	lParam;
	HTREEITEM	hTreeRoot, hChildItem, hNextItem, hBuffItem;

	//	�ǉ��ςȂ烁�b�Z�[�W�o���ďI���Z��
	id = SqlTreeNodeExtraIsFileExist( ptPath );
	if( 0 < id )
	{
		MessageBox( GetDesktopWindow( ), TEXT("�߂ɓo�^���Ă���݂�������B"), TEXT("���ӂ���̂��m�点"), MB_OK | MB_ICONINFORMATION );
		return id;
	}

	//	���[�g�m��
	hTreeRoot = TreeView_GetRoot( ghTreeWnd );

	//	���̎q�m�[�h�ł���ǉ��p�m�[�h��T��
	hChildItem = TreeView_GetChild( ghTreeWnd, hTreeRoot );
	hNextItem = NULL;

	do{
		//	�m�[�hlParam���Ђ��ς�E�|�P���Y���u�c�ł���
		lParam = TreeItemInfoGet( hChildItem, NULL, 0 );

		//	�q�b�g������I���
		if( NODE_EXTRA == lParam  ){	break;	}

		//	�Ȃ������炻�̎���T���B
		hNextItem = TreeView_GetNextSibling( ghTreeWnd, hChildItem );
		if( hNextItem == hChildItem ){	hNextItem = NULL;	}	//	�S��������瓯�������߂�炵���H
		hChildItem = hNextItem;

	}while( hChildItem );

	if( !(hChildItem) )	return 0;	//	�q�b�g���Ȃ�����
	//	�q�b�g�����̂��ǉ��p�m�[�h�ł���

	//	�J���΁A�����̃u�c���W�J�����
	TreeView_Expand( ghTreeWnd, hChildItem, TVE_EXPAND );
	//�ŏ��̂P�ڂ������ꍇ�A�W�J����Ȃ��̂œW�J�σt���O�������Ȃ�
	hBuffItem = TreeView_GetChild( ghTreeWnd, hChildItem );

	//	�W�J���Ă���J���Ȃ��Ƒ��d�Ƀc���[�ɏo�Ă���
	id = SqlTreeNodeExtraInsert( 0, ptPath );	//	SQL�ɓo�^
	if( 0 >= id )	return 0;	//	���s

	if( hBuffItem ){	TreeExtraItemFromSql( hChildItem, id-1 );	}	//	�Y���h�c�̎�����T���̂Œ���
	else{	TreeView_Expand( ghTreeWnd, hChildItem, TVE_EXPAND );	}
	//	�ŏ��̈�̏ꍇ�́A�ǉ����Ă���J����낵


	return id;
}
//-------------------------------------------------------------------------------------------------


/*!
	�G�L�X�g���t�@�C�����r�p�k����c���[�r���[�ɓW�J
	@param[in]	hTreeParent	�Ώۃf�B���N�g���̃c���[�A�C�e���E�����ɂԂ牺���Ă���
	@param[in]	dFinID		���̂h�c�ȍ~�̃A�C�e�����c���[�ɒǉ�����B�ʏ�O�A�ǉ����ɒ��ӃZ��
	@return	HRESULT	�I����ԃR�[�h
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
		stTreeIns.item.pszText = PathFindFileName( atPath );	//	�t�@�C�����u�b�R����

		stTreeIns.item.lParam    = tgtID;	//	�����͓���Ȃ̂Œ���
		stTreeIns.item.cChildren = 0;	//	�q�m�[�h�Ȃ�
		stTreeIns.hInsertAfter   = TVI_LAST;
		hNewParent = TreeView_InsertItem( ghTreeWnd, &stTreeIns );

	}while( tgtID );


	return S_OK;
}
//-------------------------------------------------------------------------------------------------

#endif

/*!
	�f�B���N�g���ƃt�@�C�����r�p�k����c���[�r���[�ɓW�J�E�ċA����킯�ł͂Ȃ�
	@param[in]	hTreeParent	�Ώۃf�B���N�g���̃c���[�A�C�e���E�����ɂԂ牺���Ă���
	@return		HRESULT		�I����ԃR�[�h
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

	dPrntID = MaaSearchTreeID( hTreeParent );	//	�����̂h�c���A���ꂩ��W�J����m�[�h�̐e�ɂȂ�
	//	�h�c�O�̓��[�g�m�[�h
	tgtID = 0;

	//	���[�g�̂Ƃ��̂݁A[*�ǉ�����*]  �݂����Ȃ̂�t��������BlParam�͂Q�Ƃ��H
#ifdef EXTRA_NODE_STYLE
	//�őO��ɒǉ�
	if( 0 == dPrntID )
	{
		StringCchCopy( atNodeName, MAX_PATH, EXTRA_NODE );
		//	�Ƃ�₦���J�����g�f�B���N�g���̃A�C�R���ŗǂ��͂�	
		//SHGetFileInfo( atCurrent, 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON) );
		stTreeIns.item.iImage = TICO_DIR_EXTRA;//stShFileInfo.iIcon;
		//SHGetFileInfo( atCurrent, 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON) );
		stTreeIns.item.iSelectedImage = TICO_DIR_EXTRA;//stShFileInfo.iIcon;
		stTreeIns.item.pszText = atNodeName;
		stTreeIns.item.lParam    = NODE_EXTRA;
		stTreeIns.item.cChildren = 1;	//	�q�m�[�h�A��
		stTreeIns.hInsertAfter   = hLastDir;
		hNewParent = TreeView_InsertItem( ghTreeWnd, &stTreeIns );
		hLastDir = hNewParent;	//	�f�B���N�g���ʒu�̍ŏI�E�t�@�C���̎�O	I_IMAGECALLBACK
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

		if( FILE_ATTRIBUTE_DIRECTORY == type )	//	�f�B���N�g���̏ꍇ
		{
			stTreeIns.item.iImage         = I_IMAGECALLBACK;//TICO_DIR_CLOSE;
			stTreeIns.item.iSelectedImage = I_IMAGECALLBACK;//TICO_DIR_OPEN;
			stTreeIns.item.lParam    = NODE_DIR;
			stTreeIns.item.cChildren = 1;	//	�q�m�[�h����
			stTreeIns.hInsertAfter   = hLastDir;
			hNewParent = TreeView_InsertItem( ghTreeWnd, &stTreeIns );
			hLastDir = hNewParent;	//	�f�B���N�g���ʒu�̍ŏI�E�t�@�C���̎�O
		}
		else	//	�t�@�C���̏ꍇ
		{
			if( FileExtensionCheck( atNodeName, TEXT(".ast") ) ){	iFileType = TICO_FILE_AST;	}
			else if( FileExtensionCheck( atNodeName, TEXT(".mlt") ) ){	iFileType = TICO_FILE_MLT;	}
			else{	iFileType = TICO_FILE_ETC;	}

			stTreeIns.item.iImage = iFileType;
			stTreeIns.item.iSelectedImage = iFileType;
			stTreeIns.item.lParam    = NODE_FILE;
			stTreeIns.item.cChildren = 0;	//	�q�m�[�h�Ȃ�
			stTreeIns.hInsertAfter   = TVI_LAST;
			hNewParent = TreeView_InsertItem( ghTreeWnd, &stTreeIns );
		}

	}while( tgtID );


	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	�c���[�m�[�h�n���h����n���āA�Y������SqlID����������E�ċA
*/
INT MaaSearchTreeID( HTREEITEM hItem )
{
	TCHAR	atName[MAX_PATH];
	HTREEITEM	hPrntItem;
	UINT	dPrntID, ownID;

	TreeItemInfoGet( hItem, atName, MAX_PATH );	//	�m�[�h�̖��O�Ƃ���

	hPrntItem = TreeView_GetParent( ghTreeWnd, hItem );	//	�e�m�[�h�m�F

	//	�e�m�[�h�̂h�c���m�ہE�e�����[�g�Ȃ�hPrntItem��NULL
	if( !(hPrntItem)  ){	return 0;	}
	else{	dPrntID = MaaSearchTreeID( hPrntItem );	}

	//	�Y������e�m�[�h�Ɩ��̂����m�[�h�͈�����Ȃ�
	ownID = SqlTreeFileGetOnParent( atName, dPrntID );

	return ownID;
}
//-------------------------------------------------------------------------------------------------

/*!
	ID��n���āA�Y���A�C�e���̃c���[�m�[�h�n���h�����Ƃ�E�ċA
	@param[in]	dOwnID	�����������m�[�h��SqlID
	@return		�q�b�g�����c���[�m�[�h�n���h���E��������NULL
*/
HTREEITEM MaaSearchTreeItem( INT dOwnID )
{
	UINT	dType, dPrntID;
	TCHAR	atFileName[MAX_PATH], atCmprName[MAX_PATH];
	HTREEITEM	hPrntItem, hChildItem, hNextItem;

	ZeroMemory( atFileName, sizeof(atFileName) );
	dType   = 0;
	dPrntID = 0;

	//	ID�ŁASQL����Y���A�C�e���̏����Ђ��ς�
	SqlTreeNodePickUpID( dOwnID, &dType, &dPrntID, atFileName, 0x11 );

	if( dPrntID )	//	�オ�L��悤�Ȃ�A�ċA����
	{
		hPrntItem = MaaSearchTreeItem( dPrntID );
	}
	else	//	�オ�Ȃ�������A���[�g�ƌ��Ȃ��āA���[�g����ɂ���CHILD����
	{
		hPrntItem = ghTreeRoot;
	}

	if( !(hPrntItem) )	return NULL;	//	�f�[�^����������I���

	//	�c���[�m�[�h�n���h�����Ԃ��Ă�����A�������ɂ���CHILD����
	//	�q�b�g������A�Y���c���[�m�[�h�n���h����Ԃ�
	//	�������W�J�Ȃ�A������NULL���A��E���̂Ƃ��́AdPrntID��atFileName�ŊY���A�C�e����T���ĊJ����K�v������
	TreeView_Expand( ghTreeWnd, hPrntItem, TVE_EXPAND );
	//	�������́A��ɊY���p�����g�̃A�C�e����W�J���Ă��炳����
	hChildItem = TreeView_GetChild( ghTreeWnd, hPrntItem );

	do{
		//	���O���������� atFileName �Əƍ�
		TreeItemInfoGet( hChildItem, atCmprName, MAX_PATH );

		if( !( StrCmp( atFileName, atCmprName ) ) )
		{
			//	�q�b�g������
			break;
		}

		hNextItem = TreeView_GetNextSibling( ghTreeWnd, hChildItem );
		hChildItem = hNextItem;

	}while( hNextItem );


	return hChildItem;	//	�q�b�g������A�Y���c���[�m�[�h�n���h����Ԃ�
}
//-------------------------------------------------------------------------------------------------

/*!
	�Ώۂ�SqlID���󂯎���āA�Y���̃c���[�A�C�e�����Ђ炭
	@param[in]	hDlg	�_�C�����O�n���h��
	@param[in]	tgtID	�Ώۂ�SqlID
	@return		�q�b�g�����c���[�m�[�h�n���h���E��������NULL
*/
HTREEITEM MaaSelectIDfile( HWND hDlg, INT tgtID )
{
	HTREEITEM	hTgtItem;

	//	SqlID��n���ƁA�Y������c���[�m�[�h�n���h�����߂��Ă���͂�
	hTgtItem = MaaSearchTreeItem( tgtID );

	if( hTgtItem ){	TreeView_SelectItem( ghTreeWnd, hTgtItem );	}	//	�I����Ԃɂ���

	return hTgtItem;
}
//-------------------------------------------------------------------------------------------------

/*!
	�c���[�̃A�C�e���̖��O�Ƃo�`�q�`�l�����m��
	@param[in]	hTrItem	�A�C�e���n���h��
	@param[out]	ptName	���O������o�b�t�@�ւ̃|�C���^�ENULL�ł��ǂ�
	@param[in]	cchName	�o�b�t�@�T�C�Y
	@return		LPARAM	�P�f�B���N�g���@�O�t�@�C��
*/
LPARAM TreeItemInfoGet( HTREEITEM hTrItem, LPTSTR ptName, size_t cchName )
{
	TCHAR	atBuffer[MAX_PATH];
	TVITEM	stTvItem;

	ZeroMemory( &stTvItem, sizeof(TVITEM) );
	ZeroMemory( atBuffer, sizeof(atBuffer) );

	//	���O�ƃf�B���N�g�����t�@�C��������������o��
	stTvItem.hItem      = hTrItem;
	stTvItem.mask       = TVIF_TEXT | TVIF_PARAM;
	stTvItem.pszText    = atBuffer;
	stTvItem.cchTextMax = MAX_PATH;
	TreeView_GetItem( ghTreeWnd, &stTvItem );

	if( ptName )	//	�o�b�t�@���L���Ȃ�
	{
		ZeroMemory( ptName, sizeof(TCHAR) * cchName );
		StringCchCopy( ptName, cchName, atBuffer );
	}

	return stTvItem.lParam;
}
//-------------------------------------------------------------------------------------------------

/*!
	�c���[�̃m�[�e�B�t�@�C���b�Z�[�W�̏���
	@param[in]	hWnd		�e�E�C���h�E�̃n���h��
	@param[in]	pstNmTrView	NOTIFY�̏ڍ�
	@return		�����������e�Ƃ�
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

	//	�E�N���b�N�̓R���e�L�X�g���j���[��

	if( TVN_SELCHANGED == nmCode )	//	�I��������
	{
		hSelItem = TreeView_GetSelection( ghTreeWnd );	//	�I������Ă�A�C�e��
		//	�R���{�b�N�X�N�����[�ʒu�ύX���Ă݂�
		TreeSelItemProc( hWnd, hSelItem, 0 );
	}


	if( TVN_ITEMEXPANDING == nmCode )	//	�q�m�[�h��W�J�܂��͕���Ƃ�
	{
		TRACE( TEXT("TVN_ITEMEXPANDING[%X]"), pstNmTrView->action );

		if( pstNmTrView->action & TVE_EXPAND )	//	�W�J����
		{
			pstTvItem = &(pstNmTrView->itemNew);

			if( pstTvItem->state & TVIS_EXPANDEDONCE )	return 0;
			//	�W�J�ςȂ牽�����Ȃ��ł���

#ifdef EXTRA_NODE_STYLE
			//	����lPmaram��-1�Ȃ�
			lParam = TreeItemInfoGet( pstTvItem->hItem, NULL, 0 );
			if( NODE_EXTRA == lParam )	//	�ǉ����ł���
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

		if( pstNmTrView->action & TVE_COLLAPSE )	//	����
		{
			pstTvItem = &(pstNmTrView->itemNew);
			TRACE( TEXT("TVE_COLLAPSE[%d, %d]"), pstTvItem->iImage, pstTvItem->iSelectedImage );
		}
	}


	//	���قȃA�C�R���ǉ�����Ȃ�
	if( TVN_GETDISPINFO == nmCode )	//	�Ȃ񂩉摜��񂪋���Ƃ�
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
	�c���[�̑I�������A�C�e������̏����̃`�F�C���E����𓝍�
	@param[in]	hWnd		�e�E�C���h�E�̃n���h��
	@param[in]	hSelItem	�I�����Ă�m�[�h�̃n���h��
	@param[in]	dMode		�O��^�u�ŊJ���@�P���^�u�ǉ��@�Q�ҏW�r���[�ŊJ���@�R�A�C�e���ǉ��@�S�m�[�h�폜
	@return		��O���������@�O���ĂȂ�
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

	//	�E�N���E�c���[�I�����獇��

	if( !(hSelItem) ){	return 0;	}	//	�Ȃ񂩖����Ȃ牽�����Ȃ�

	//	�I�����ꂽ�̂��G�L�X�g���t�@�C���ł���΁H�e�m�[�h��lParam����΂킩��
	//	�G�L�X�g���Ȃ�A�t���p�X�m�ۂ��Ȃ�����@�x�[�X���Ƃ����ӃZ��
#ifdef EXTRA_NODE_STYLE
	hParentItem = TreeView_GetParent( ghTreeWnd, hSelItem );
	lParam = TreeItemInfoGet( hParentItem, NULL, 0 );
	if( NODE_EXTRA == lParam )
	{
		//	�I�����ꂽ��ɂ͊Y���A�C�e����sqlID�������Ă�
		lParam = TreeItemInfoGet( hSelItem, NULL, 0 );
		//	�Y���̃A�C�e�����m��
		id = SqlTreeNodeExtraSelect( lParam, 0, atPath );
		if( 0 == id )	return 0;	//	�Ȃ񂩂��������Ȃ牽�����Ȃ�

		//	�t�@�C�������x�[�X���ɂ��Ă���
		if( 0 == dMode )
		{
			StringCchCopy( gatBaseName, MAX_PATH, PathFindFileName( atPath ) );
			StatusBarMsgSet( SBMAA_FILENAME , gatBaseName );	//	�X�e�[�^�X�o�[�Ƀt�@�C�����\��
		}
		StringCchCopy( atBaseName, MAX_PATH, PathFindFileName( atPath ) );	//	���ł��L�^�ő��v��	20120530
	}
	else
	{
#endif
		//	�I�����ꂽ��̃t�@�C�����A�������̓f�B���N�g�����m��
		lParam = TreeItemInfoGet( hSelItem, atName, MAX_PATH );
		//����	lParam�̔��f
		if( NODE_FILE != lParam ){	return 0;	}	//	�t�@�C���Ŗ����Ȃ牽�����Ȃ�

		//	�I���������O���m�ہE���[�g�ɂ���ꍇ����œK�p�����
		if( 0 == dMode )
		{
			StringCchCopy( gatBaseName, MAX_PATH, atName );
			StatusBarMsgSet( SBMAA_FILENAME, atName );	//	�X�e�[�^�X�o�[�Ƀt�@�C�����\��
		}
		StringCchCopy( atBaseName, MAX_PATH, atName );	//	���ł��L�^�ő��v��	20120530

		//	�x�[�X�����A��������f�B���N�g�����ɂ���B���[�g�̃t�@�C���Ȃ炻�̂܂�

		//	��ɒH���āA�t�@�C���p�X�����
		for( i = 0; 12 > i; i++ )	//	�P�Q��蕅�C�K�w�͂��ǂ�Ȃ�
		{
			hParentItem = TreeView_GetParent( ghTreeWnd, hSelItem );
			if( !(hParentItem) )	return 0;	//	���[�g�̏�͂Ȃ��E�����đI��������Ȃ�
			if( ghTreeRoot == hParentItem ){	break;	}	//	���������[�g�܂ŃC�b����I���

			TreeItemInfoGet( hParentItem, atPath, MAX_PATH );

			if( 0 == i )	//	����݂̂Ȃ�A��������f�B���N�g�����ɂȂ�	20110928
			{
				if( 0 == dMode ){	StringCchCopy( gatBaseName, MAX_PATH, atPath );	}
				else{				StringCchCopy( atBaseName, MAX_PATH, atPath );	}
			}

			//	���̃p�X���L�^���Ă����ƁA�ŏI�I�Ƀ��[�g�����̃f�B���N�g�����ɂȂ�
			PathAppend( atPath, atName );
			StringCchCopy( atName, MAX_PATH, atPath );

			hSelItem = hParentItem;
		}

		//	���[�g�ʒu���������ăt���p�X�ɂ���
		StringCchCopy( atPath, MAX_PATH, gatAARoot );
		PathAppend( atPath, atName );

#ifdef EXTRA_NODE_STYLE
	}
#endif

	switch( dMode )
	{
		default:	//	��^�u�ŊJ���ꍇ
		case  0:	AaItemsDoShow( hWnd , atPath, ACT_ALLTREE );	break;	//	����MLT���J��

		case  1:	//	���^�u�ɊJ���ꍇ
			ZeroMemory( &stMulti, sizeof(MULTIPLEMAA) );
			StringCchCopy( stMulti.atFilePath, MAX_PATH, atPath );
			StringCchCopy( stMulti.atBaseName, MAX_PATH, atBaseName );
			stMulti.dTabNum = 0;	//	�������E�����͂Q�ȍ~

			gltMultiFiles.push_back( stMulti );
			TabMultipleAppend( hWnd );
			break;
#ifndef _ORRVW
		case  2:	//	�ҏW�r���[�ŊJ���ꍇ
			DocDoOpenFile( hWnd, atPath );	//	�J���Ē��g�W�J
			break;

  #ifndef MAA_IADD_PLUS
		//	�A�C�e���ǉ��E�L�����Z��
		//case  3:	AacItemAdding( hWnd, atPath );	break;
  #endif
#endif	//	_ORRVW

#ifdef EXTRA_NODE_STYLE
		case  4:	//	�m�[�h�폜�E�Ƃ�₦���G�L�X�g���t�@�C��
			if( 0 < id )	//	�L���ȏꍇ
			{
				TreeView_DeleteItem( ghTreeWnd , hSelItem );	//	�c���[����폜����
				SqlTreeNodeExtraDelete( id );	//	���X�g������폜
			}
			break;
#endif
	}

	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	�I�����Ă��郊�X�g�̎�̃f�B���N�g�����������̓t�@�C�����̃o�b�t�@�|�C���^���m��
	@return	�|�C���^
*/
LPTSTR TreeBaseNameGet( VOID )
{
	return gatBaseName;
}
//-------------------------------------------------------------------------------------------------




/*!
	�^�u�o�[�̃T�C�Y���m�ۂ���
	@param[in]	pstRect	�T�C�Y�����A��
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
	�^�u�̃T�C�Y�ύX
	@param[in]	hWnd	�e�E�C���h�E�̃n���h��
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
	�^�u�o�[�̃m�[�e�B�t�@�C���b�Z�[�W�̏���
	@param[in]	hWnd		�e�E�C���h�E�̃n���h��
	@param[in]	pstNmhdr	NOTIFY�̏ڍ�
	@return		�����������e�Ƃ�
*/
LRESULT TabBarNotify( HWND hWnd, LPNMHDR pstNmhdr )
{
	INT		nmCode;
	INT		curSel;

	NMTREEVIEW	stNmTrView;

	//pstNmhdr->hwndFrom;
	nmCode   = pstNmhdr->code;

	//	�E�N���b�N�̓R���e�L�X�g���j���[��

	if( TCN_SELCHANGE == nmCode )	//	�^�u���`�F���W��������
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
			//	�I���𔭐�������
			ZeroMemory( &stNmTrView, sizeof(NMTREEVIEW) );
			stNmTrView.hdr.hwndFrom = ghTreeWnd;
			stNmTrView.hdr.idFrom   = IDTV_ITEMTREE;
			stNmTrView.hdr.code     = TVN_SELCHANGED;
			//	���͎g���ĂȂ�����O�ł���
			TreeNotify( hWnd, &stNmTrView );
		}
		else if( ACT_FAVLIST == curSel )
		{
			//	�I�[�|�����ꂽ�Ƃ��ɁA�S������
			while( ListBox_GetCount( ghFavLtWnd ) ){	ListBox_DeleteString( ghFavLtWnd, 0 );	}
			SqlFavFolderEnum( FavListFolderNameBack );

	//		AacMatrixClear(  );
			ShowWindow( ghFavLtWnd, SW_SHOW );
			gixUseTab = ACT_FAVLIST;
		}
		else
		{
			AaTitleClear(  );	//	���g�̂��鎞�ɏ�����
			TabMultipleSelect( hWnd, curSel, 0 );
		}
	}

	return 0;
}
//-------------------------------------------------------------------------------------------------

/*!
	���^�u�͂��C�Ƀ��X�g�̂ł��邩
	@param[in]	tabSel	�I�����ꂽ�^�u�ԍ�
	@param[out]	ptBase	�x�[�X��������o�b�t�@�ւ̃|�C���^�[�ENULL��
	@param[in]	cchSize	�o�b�t�@�̕�����
	@return	��O���C�ɂł���@�O�Ⴄ
*/
UINT TabMultipleIsFavTab( INT tabSel, LPTSTR ptBase, UINT_PTR cchSize )
{
	MLTT_ITR	itNulti;

	for( itNulti = gltMultiFiles.begin( ); gltMultiFiles.end( ) != itNulti; itNulti++ )
	{
		if( tabSel == itNulti->dTabNum )	//	�I������Ă���������
		{
			//	�Ƃ肠�����R�s�[
			if( ptBase ){	StringCchCopy( ptBase, cchSize, itNulti->atBaseName );	}

			if( NULL == itNulti->atFilePath[0] )	return 1;	//	���C�ɂł���
			else	return 0;	//	�߂��Ă���
		}
	}

	return 0;	//	�q�b�g���Ȃ�������Ƃ肠�����Ⴄ���Ƃɂ���
}
//-------------------------------------------------------------------------------------------------

/*!
	���^�u����I�������ꍇ
	@param[in]	hWnd	�E�C���h�E�n���h��
	@param[in]	tabSel	�I�����ꂽ�^�u�ԍ�
	@param[in]	dMode	�O�^�u�I�������@�P�ҏW�r���[�ŊJ��
	@return	��OMLT�J�����@�O�Ȃ�����
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
		if( tabSel == itNulti->dTabNum )	//	�I������Ă���������
		{
			if( 0 == dMode )	//	�r���[�G���A�ɕ\��
			{
				//	��_�f�B���N�g�����Z�b�g
				StringCchCopy( gatBaseName, MAX_PATH, itNulti->atBaseName );

				//	�����ŁA�t�@�C�������C�ɂ��𔻒f����EatFilePath����ł����
				if( NULL == itNulti->atFilePath[0] )	//	���C�ɂł���
				{
					StringCchCopy( atName, MAX_PATH, gatBaseName );
					StringCchCat(  atName, MAX_PATH, TEXT("[F]") );

					AaItemsDoShow( hWnd, gatBaseName, ACT_FAVLIST );	//	���������Ă��錳�̎w��ł���
				}
				else
				{
					//	�t�@�C�������m�ہE�\���p
					StringCchCopy( atName, MAX_PATH, itNulti->atFilePath );
					PathStripPath( atName );

					//	����MLT���J���E�Ⴂ���d�v
					AaItemsDoShow( hWnd, itNulti->atFilePath, ACT_SUBITEM );
				}

				StatusBarMsgSet( SBMAA_FILENAME, atName );	//	�X�e�[�^�X�o�[�Ƀt�@�C�����\��
			}
#ifndef _ORRVW
			else	//	�t�@�C�������m�ۂ��āA����ɕҏW�r���[���ŊJ������������
			{
				DocDoOpenFile( hWnd, itNulti->atFilePath );	//	�r���[�����̕��^�u����ҏW�E�C���h�E�ɊJ���Ƃ�
			}
#endif
			return 1;
		}
	}

	return 0;
}
//-------------------------------------------------------------------------------------------------

/*!
	�\�������V�ӈʒu���L������
	@param[in]	dTop	�V�����ʒu�E�|�P�Ȃ�A�L������Ă���ʒu��Ԃ�
	@return	INT	�ďo�Ȃ�Y������l
*/
INT TabMultipleTopMemory( INT dTop )
{
	MLTT_ITR	itNulti;

	//	�֌W�Ȃ��Ȃ牽�����Ȃ�
	if( ACT_SUBITEM > gixUseTab )	return 0;

	for( itNulti = gltMultiFiles.begin( ); gltMultiFiles.end( ) != itNulti; itNulti++ )
	{
		if( gixUseTab == itNulti->dTabNum )	//	�I������Ă���������
		{
			//	�l����ꂽ��o������
			if( 0 >  dTop ){	dTop = itNulti->dLastTop;	}
			else{	itNulti->dLastTop = dTop;	}
			break;
		}
	}

	if( 0 > dTop )	dTop = 0;	//	�ꉞ���S�΍�

	return dTop;
}
//-------------------------------------------------------------------------------------------------

/*!
	���^�u�̍\�����v���t�@�C���ɕۑ�����
	@param[in]	hWnd	�E�C���h�E�n���h��
	@return		HRESULT	�I����ԃR�[�h
*/
HRESULT TabMultipleStore( HWND hWnd )
{
	MLTT_ITR	itNulti;


	SqlMultiTabDelete(  );	//	��USQL�̓��e�S�������ď����������Ă�

	for( itNulti = gltMultiFiles.begin( ); gltMultiFiles.end( ) != itNulti; itNulti++ )
	{
		//	�L�^���Ȃ��ł�낵�����H
		if( StrCmp( DROP_OBJ_NAME, itNulti->atBaseName ) )
		{
			SqlMultiTabInsert( itNulti->atFilePath, itNulti->atBaseName, itNulti->atDispName );
		}
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	���^�u���v���t�@�C������ǂݍ���ōēW�J����
	@param[in]	hWnd	�E�C���h�E�n���h��
	@return		HRESULT	�I����ԃR�[�h
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
	�h���b�O���h���b�y���ꂽ�t�@�C���𕛃^�u�ɂ����Ⴄ
	@param[in]	hWnd	���E�C���h�E�n���h��
	@param[in]	ptFile	�h���b�v���ꂽ�t�@�C���̃p�X
	@return		HRESULT	�I����ԃR�[�h
*/
HRESULT TabMultipleDropAdd( HWND hWnd, LPCTSTR ptFile )
{
	MULTIPLEMAA		stMulti;
	INT		iTabNum;


	ZeroMemory( &stMulti, sizeof(MULTIPLEMAA) );
	StringCchCopy( stMulti.atFilePath, MAX_PATH, ptFile );
	StringCchCopy( stMulti.atBaseName, MAX_PATH, DROP_OBJ_NAME );	//	���ꖼ�́E���v��
	stMulti.dTabNum = 0;	//	�������E�����͂Q�ȍ~

	gltMultiFiles.push_back( stMulti );
	iTabNum = TabMultipleAppend( hWnd );

	//	���̑������J�����Ⴄ�H
	TabCtrl_SetCurSel( ghTabWnd, iTabNum );
	TabMultipleSelect( hWnd, iTabNum, 0 );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	�^�u�𑝂₷�E�ێ����X�g�Ƀt�@�C�����Ԃ����񂾂璼���ɌĂԂׂ�
	@param[in]	hWnd	�e�E�C���h�E�̃n���h��
	@return	INT	�J�����^�u�̔ԍ�
*/
INT TabMultipleAppend( HWND hWnd )
{
	TCHAR	atName[MAX_PATH];
	LONG	tCount;
//	RECT	itRect;
	TCITEM	stTcItem;

	MLTT_ITR	itNulti;


	itNulti = gltMultiFiles.end( );
	itNulti--;	//	�V�����J���͖̂��[�ɂ���͂�
	StringCchCopy( atName, MAX_PATH, itNulti->atFilePath );
	if( NULL !=  atName[0] )	//	�c���[�������̓h���b�O���h���b�y
	{
		PathStripPath( atName );	//	�t�@�C���������ɂ���
		PathRemoveExtension( atName );	//	�g���q���O��
	}
	else	//	���C�Ƀ��X�g����ǉ�����
	{
		StringCchCopy( atName, MAX_PATH, itNulti->atBaseName );
		StringCchCat(  atName, MAX_PATH, TEXT("[F]") );
	}

	if( NULL == itNulti->atDispName[0] )
	{	StringCchCopy( itNulti->atDispName , MAX_PATH, atName );	}	//	�\�����f�t�H���g

	ZeroMemory( &stTcItem, sizeof(TCITEM) );
	stTcItem.mask = TCIF_TEXT | TCIF_PARAM;

	tCount = TabCtrl_GetItemCount( ghTabWnd );

	stTcItem.lParam  = 0;//tCount;�t�@�C���Ȃ̂łO�ł����E�^�u�͖̂��g�p
	stTcItem.pszText = itNulti->atDispName;
	TabCtrl_InsertItem( ghTabWnd, tCount, &stTcItem );

	itNulti->dTabNum = tCount;

	Maa_OnSize( hWnd, 0, 0, 0 );	//	�����͎g���ĂȂ�������

	return tCount;
}
//-------------------------------------------------------------------------------------------------

/*!
	�J���Ă郄�c�̔ԍ���Ԃ��B��^�u�E�g�p�E���^�u
	@return	INT	�J���Ă�z�̔ԍ�
*/
INT TabMultipleNowSel( VOID )
{
	return gixUseTab;	//	ACT_ALLTREE	ACT_FAVLIST
}
//-------------------------------------------------------------------------------------------------

/*!
	���^�u��S������
	@param[in]	hWnd	�E�C���h�E�n���h��
	@return		HRESULT	�I����ԃR�[�h
*/
HRESULT TabMultipleDeleteAll( HWND hWnd )
{
	INT	ttlSel, i;
	NMHDR	stNmHdr;

	ttlSel = TabCtrl_GetItemCount( ghTabWnd );

	//	�S�j��
//	for( i = 2; ttlSel > i; i++ ){	TabCtrl_DeleteItem( ghTabWnd, i );	}�ԈႢ
	for( i = (ttlSel-1); 2 <= i; i-- ){	TabCtrl_DeleteItem( ghTabWnd, i );	}

	gltMultiFiles.clear();

	//	�c���[�ɑI����߂�
	TabCtrl_SetCurSel( ghTabWnd, ACT_ALLTREE );
	stNmHdr.hwndFrom = ghTabWnd;
	stNmHdr.idFrom   = IDTB_TREESEL;
	stNmHdr.code     = TCN_SELCHANGE;
	TabBarNotify( hWnd, &stNmHdr );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	�w��̃^�u�����
	@param[in]	hWnd	�E�C���h�E�n���h��
	@param[in]	tabSel	�^�u�ԍ�
	@return		HRESULT	�I����ԃR�[�h
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

	//	20110808	�^�u�ԍ��U�蒼��
	i = 2;
	for( itNulti = gltMultiFiles.begin( ); gltMultiFiles.end( ) != itNulti; itNulti++ )
	{
		itNulti->dTabNum = i;
		i++;
	}

	//	�����A�폜�^�u���J���Ă�^�u��������E�c���[�ɑI����߂�
	if( nowSel == tabSel )
	{
		TabCtrl_SetCurSel( ghTabWnd, ACT_ALLTREE );
		stNmHdr.hwndFrom = ghTabWnd;
		stNmHdr.idFrom   = IDTB_TREESEL;
		stNmHdr.code     = TCN_SELCHANGE;
		TabBarNotify( hWnd, &stNmHdr );
	}

	Maa_OnSize( hWnd, 0, 0, 0 );	//	�����͎g���ĂȂ�������

//���̒i�K�ł́A�L�^�̏��������͂��Ȃ�

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	�����݊J���Ă��镛�^�u�����
	@param[in]	hWnd	�E�C���h�E�n���h��
	@return		HRESULT	�I����ԃR�[�h
*/
HRESULT TabMultipleSelDelete( HWND hWnd )
{
	INT		curSel;

	curSel = TabCtrl_GetCurSel( ghTabWnd );

	TRACE( TEXT("VIEW FILE CLOSE [%d]"), curSel );

	//	�Œ�̓�̏ꍇ�͖���
	if( 1 >= curSel )	return E_ACCESSDENIED;

	return TabMultipleDelete( hWnd, curSel );	//	�^�u�폜�ɓn��
}
//-------------------------------------------------------------------------------------------------

/*!
	�����E�C���h�E����̑I���̏���������
	@param[in]	hWnd	MAA�̃E�C���h�E�n���h���ł��邱��
*/
HRESULT TabMultipleCtrlFromFind( HWND hWnd )
{
	NMHDR	stNmHdr;

	//�����ɗ���O�ɁA�c���[���ŊY���m�[�h���I����

	//	�I���^�u�`�F���W���āA�c���[�̑I���𔭐�������
	TabCtrl_SetCurSel( ghTabWnd, ACT_ALLTREE );
	stNmHdr.hwndFrom = ghTabWnd;
	stNmHdr.idFrom   = IDTB_TREESEL;
	stNmHdr.code     = TCN_SELCHANGE;
	TabBarNotify( hWnd, &stNmHdr );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	���C�ɓ���̃��X�g���R�[���o�b�N�Ŏ󂯎��
	@param[in]	dNumber		�ʂ��ԍ�������
	@param[in]	dummy		���g�p
	@param[in]	fake		���g�p
	@param[in]	ptFdrName	������
	@return		�����������e�Ƃ�
*/
LRESULT CALLBACK FavListFolderNameBack( UINT dNumber, UINT dummy, UINT fake, LPCVOID ptFdrName )
{
	INT	iOrder;

	iOrder = ListBox_AddString( ghFavLtWnd, (LPCTSTR)ptFdrName );

	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	���C�ɓ���̃��X�g���N���b�N���ꂽ�Ƃ�
	@param[in]	hWnd	�e�E�C���h�E�̃n���h��
	@param[in]	iCode	���������C�x���g
	@return		HRESULT	�I����ԃR�[�h
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
		//	�m�ۂ����f�B���N�g�����ɊY������`�`���r�p�k�����������
		AaItemsDoShow( hWnd, atFdrName, ACT_FAVLIST );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	���C�ɓ���̏ꍇ�̓N���b�N���ꂽ�Ƃ��ɍĕ`�悷�邩�E�I���o����悤��
	@param[in]	hWnd	�e�E�C���h�E�̃n���h��
	@return		HRESULT	�I����ԃR�[�h
*/
HRESULT FavContsRedrawRequest( HWND hWnd )
{
	//	�֌W�Ȃ��Ƃ��͉������Ȃ��E�֌W�Ȃ��Ƃ��͌Ă΂Ȃ��悤�ɒ��ӃZ��
	//	���C�Ƀ��X�g���^�u�ɕ\�������Ƃ��A�ĕ`��w��𔻒肷��K�v���L��
	if( ACT_FAVLIST == gixUseTab || TabMultipleIsFavTab( gixUseTab, NULL, 0 ) )
	{
		AaItemsDoShow( hWnd, gatBaseName, ACT_FAVLIST );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	�z�E�B�[����]�������̏�Ŕ���������
	@param[in]	hWnd	�e�E�C���h�E�n���h��
	@param[in]	hChdWnd	�}�E�X�J�[�\���́��ɂ������q�E�C���h�E
	@param[in]	xPos	�����������W�w
	@param[in]	yPos	�����������W�x
	@param[in]	zDelta	��]�ʁEWHEEL_DELTA�̔{���E���̒l�͑O(��)�A���̒l�͌��(��O)�։񂳂ꂽ
	@param[in]	fwKeys	������Ă�L�[
	@return		��O�����������@�O�֌W�Ȃ���
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
	���̕ύX�_�C�����O�{�b�N�X�̃��Z�[�W�n���h���E�Ŗ��̎g���񂵂Ȃ̂Œ���
	@param[in]	hDlg	�_�C�����O�n���h��
	@param[in]	message	�E�C���h�E���b�Z�[�W�̎��ʔԍ�
	@param[in]	wParam	�ǉ��̏��P
	@param[in]	lParam	�ǉ��̏��Q
	@retval 0	���b�Z�[�W�͏������Ă��Ȃ�
	@retval no0	�Ȃ񂩏������ꂽ
*/
INT_PTR CALLBACK TabMultipleRenameDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static LPTSTR	cptName;
	TCHAR	atBuffer[MAX_PATH];

	switch( message )
	{
		case WM_INITDIALOG:
			cptName = (LPTSTR)lParam;	//	��������� MAX_PATH �ł��邱��
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
	�^�u���O�ύX�̏����E�_�C�����O�J������ύX���L�^������
	@param[in]	hWnd	�E�C���h�E�n���h��
	@param[in]	iTabSel	�I�������^�u�ԍ�
	@return		HRESULT	�I����ԃR�[�h
*/
HRESULT TabMultipleNameChange( HWND hWnd, INT iTabSel )
{
	INT_PTR	iRslt;
	TCHAR	atName[MAX_PATH];
	TCITEM	stTcItem;
	MLTT_ITR	itNulti;

	for( itNulti = gltMultiFiles.begin( ); gltMultiFiles.end( ) != itNulti; itNulti++ )
	{
		if( iTabSel == itNulti->dTabNum )	//	�I������Ă���������
		{
			StringCchCopy( atName, MAX_PATH, itNulti->atDispName );

			iRslt = DialogBoxParam( GetModuleHandle( NULL ), MAKEINTRESOURCE(IDD_PAGE_NAME_DLG), hWnd, TabMultipleRenameDlgProc, (LPARAM)atName );
			if( IDOK == iRslt )	//	�n�j���Ă���A�ύX���ꂽ���O���o�b�t�@�ɓ����Ă�͂�
			{
				StringCchCopy( itNulti->atDispName, MAX_PATH, atName );	//	�L�^�ύX

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
	�^�u�̑��i�\���E��s�\����؂�ւ���
	@param[in]	hWnd	�E�C���h�E�n���h��
	@return		HRESULT	�I����ԃR�[�h
*/
HRESULT TabLineMultiSingleToggle( HWND hWnd )
{
	 INT	doSingle;
	DWORD	dWndwStyle;

	dWndwStyle = GetWindowStyle( ghTabWnd );

	if( TCS_MULTILINE & dWndwStyle )	//	���i���[�h���E�V���O���X�^�C���ɂ���
	{
		doSingle = 1;
		dWndwStyle &= ~TCS_MULTILINE;	//	�X�^�C�����O��
	}
	else	//	�V���O�����[�h���E���i�X�^�C���ɂ���
	{
		doSingle = 0;
		dWndwStyle |= TCS_MULTILINE;	//	�X�^�C������������
	}

	SetWindowLong( ghTabWnd, GWL_STYLE, dWndwStyle );	//	�X�^�C�������߂�
	//	�ĕ`��Ƃ��H�Ȃ��Ă悳����
	InitParamValue( INIT_SAVE, VL_MAATAB_SNGL, doSingle );	//	�L�^

	Maa_OnSize( hWnd, 0, 0, 0 );	//	�����͎g���ĂȂ�������

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

