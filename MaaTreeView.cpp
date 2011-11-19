/*! @file
	@brief �c���[�r���[�Ƃ��C�ɓ���A�^�u�̐�������܂�
	���̃t�@�C���� MaaTreeView.cpp �ł��B
	@author	SikigamiHNQ
	@date	2011/06/22
*/
//-------------------------------------------------------------------------------------------------



#include "stdafx.h"
#include "OrinrinEditor.h"
#include "MaaTemplate.h"
//-------------------------------------------------------------------------------------------------

//	�����t�@�C���ێ��̃A��
typedef struct tagMULTIPLEMAA
{
	INT		dTabNum;				//!<	�^�u�̔ԍ��E�Q�C���f�b�N�X
	TCHAR	atFilePath[MAX_PATH];	//!<	�t�@�C���p�X�E��Ȃ�g�p����J����
	TCHAR	atBase[MAX_PATH];		//!<	�g�p���X�g�ɓ���鎞�̃O���[�v��

	UINT	dLastTop;				//!<	���Ă�AA�̔ԍ�

} MULTIPLEMAA, *LPMULTIPLEMAA;

//-------------------------------------------------------------------------------------------------

extern  HWND		ghSplitaWnd;	//	�X�v���b�g�o�[�n���h��

static HFONT		ghTabFont;

static  HWND		ghTabWnd;		//!<	�I���^�u�̃n���h��

static  HWND		ghFavLtWnd;		//!<	�悭�g���z��o�^���郊�X�g�{�b�N�X

static  HWND		ghTreeWnd;		//!<	�c���[�̃n���h��
static HTREEITEM	ghTreeRoot;		//!<	�c���[�̃��[�g�A�C�e��

static HIMAGELIST	ghImageList, ghOldIL;	//!<	�c���[�r���[�ɂ�������C���[�W���X�g

static TCHAR		gatAARoot[MAX_PATH];	//!<	�`�`�f�B���N�g���̃J�����g
static TCHAR		gatBaseName[MAX_PATH];	//!<	�g�p���X�g�ɓ���鎞�̃O���[�v��

static INT			gixUseTab;	//!<	���J���Ă�́E�O�c���[�@�P���C�ɓ���@�Q�`�����t�@�C��
//	�^�u�ԍ��ł��邱�Ƃɒ��ӁE�����t�@�C�����X�g�̊����ԍ��ł͂Ȃ�

static WNDPROC	gpfOriginFavListProc;	
static WNDPROC	gpfOriginTreeViewProc;	
static WNDPROC	gpfOriginTabMultiProc;	


list<MULTIPLEMAA>	gltMultiFiles;	//!<	�����t�@�C���̕ێ�
typedef  list<MULTIPLEMAA>::iterator	MLTT_ITR;
//-------------------------------------------------------------------------------------------------

#ifdef MAA_PROFILE
UINT	TreeItemFromSql( LPCTSTR, HTREEITEM, UINT );
#else
HRESULT	TreeItemFind( LPCTSTR, HTREEITEM, UINT );	//!<	
#endif

INT		TreeSelItemProc( HWND, HTREEITEM, UINT );	//!<	

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
	�c���[�r���[�Ƃ������
	@param[in]	hWnd	�e�E�C���h�E�n���h���ENULL�Ŕj�󏈗�
	@param[in]	hInst	�A�v���̎���
	@param[in]	ptRect	���C���E�C���h�E�̈ʒu�Ƒ傫��
	@return		HRESULT	�I����ԃR�[�h
*/
HRESULT TreeInitialise( HWND hWnd, HINSTANCE hInst, LPRECT ptRect )
{
	TCITEM		stTcItem;
	RECT		itRect, clRect;

	SHFILEINFO	stShFileInfo;

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

//�\���I���^�u
	ghTabWnd = CreateWindowEx( 0, WC_TABCONTROL, TEXT("treetab"),
		WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | TCS_RIGHTJUSTIFY | TCS_MULTILINE,
		0, 0, TREE_WIDTH, 0, hWnd, (HMENU)IDTB_TREESEL, hInst, NULL );	//	TCS_SINGLELINE
	SetWindowFont( ghTabWnd, ghTabFont, FALSE );

	ZeroMemory( &stTcItem, sizeof(stTcItem) );
	stTcItem.mask = TCIF_TEXT;
	stTcItem.pszText = TEXT("�S��");	TabCtrl_InsertItem( ghTabWnd, 0, &stTcItem );
	stTcItem.pszText = TEXT("�g�p");	TabCtrl_InsertItem( ghTabWnd, 1, &stTcItem );

	//	�I�΂ꂵ�t�@�C�����^�u�I�ɒǉ��H�@�^�u���̓E�C���h�E��

	TabCtrl_GetItemRect( ghTabWnd, 1, &itRect );
	itRect.bottom -= itRect.top;

	itRect.right -= itRect.left;
	itRect.top = 0;
	itRect.left = 0;
	TabCtrl_AdjustRect( ghTabWnd, 0, &itRect );

	MoveWindow( ghTabWnd, 0, 0, clRect.right, itRect.top, TRUE );

	//	�T�u�N���X��
	gpfOriginTabMultiProc =SubclassWindow( ghTabWnd, gpfTabMultiProc );

//���C�ɓ���p���X�g�{�b�N�X
	ghFavLtWnd = CreateWindowEx( WS_EX_CLIENTEDGE, WC_LISTBOX, TEXT("favlist"),
		WS_CHILD | WS_VSCROLL | LBS_NOTIFY | LBS_SORT | LBS_NOINTEGRALHEIGHT,
		0, itRect.bottom, TREE_WIDTH, ptRect->bottom-itRect.bottom-1, hWnd, (HMENU)IDLB_FAVLIST, hInst, NULL );

	//	�T�u�N���X��
	gpfOriginFavListProc = SubclassWindow( ghFavLtWnd, gpfFavListProc );


//�S�`�`���X�g�c���[
	ghImageList = (HIMAGELIST)SHGetFileInfo( TEXT(""), 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON) );

	ghTreeWnd = CreateWindowEx( WS_EX_CLIENTEDGE, WC_TREEVIEW, TEXT("itemtree"),
		WS_VISIBLE | WS_CHILD | TVS_DISABLEDRAGDROP | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS,
		0, itRect.bottom, TREE_WIDTH, ptRect->bottom-itRect.bottom-1, hWnd, (HMENU)IDTV_ITEMTREE, hInst, NULL );
	TreeView_SetImageList( ghTreeWnd, ghImageList, TVSIL_NORMAL );

	//	�T�u�N���X��
	gpfOriginTreeViewProc = SubclassWindow( ghTreeWnd, gpfTreeViewProc );

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
		HANDLE_MSG( hWnd, WM_CHAR, Maa_OnChar );		//	

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
		HANDLE_MSG( hWnd, WM_CHAR, Maa_OnChar );		//	

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
		HANDLE_MSG( hWnd, WM_CHAR, Maa_OnChar );
		default:	break;
	}

	return CallWindowProc( gpfOriginTabMultiProc, hWnd, msg, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------

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

	if( VK_TAB != ch )	return;

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
	INT		curSel;
	TCHAR	atText[MAX_PATH], atName[MAX_PATH];
	LPARAM	lPrm;
	UINT_PTR	cchSize;
//#ifndef _ORRVW
	LONG_PTR	rdExStyle;
	TCHAR	atSelName[MAX_PATH], atMenuStr[MAX_PATH];
	MULTIPLEMAA	stMulti;
//#endif
	POINT			stPost;
	TVHITTESTINFO	stTvHitInfo;
	TCHITTESTINFO	stTcHitInfo;
	TCITEM			stTcItem;
	MENUITEMINFO	stMenuItemInfo;
	HTREEITEM		hTvHitItem;


	stPost.x = (SHORT)xPos;	//	��ʍ��W�̓}�C�i�X�����肤��
	stPost.y = (SHORT)yPos;

	TRACE( TEXT("MAA�R���e�L�X�g���j���[") );

//#ifndef _ORRVW
	//	���C�Ƀ��X�g�{�b�N�X�̃R���e�L�X�g
	if( ghFavLtWnd == hWndContext )
	{
		ZeroMemory( atSelName, sizeof(atSelName) );
		ZeroMemory( atMenuStr, sizeof(atMenuStr) );

		curSel = ListBox_GetCurSel( ghFavLtWnd );
		TRACE( TEXT("���X�g�{�b�N�X�R���e�L�X�g %d"), curSel );
		if( 0 > curSel )	return;

		ListBox_GetText( ghFavLtWnd, curSel, atSelName );
		StringCchPrintf( atMenuStr, MAX_PATH, TEXT("[ %s ]�ŕ��^�u��ǉ�"), atSelName );

		hMenu = CreatePopupMenu(  );
		//	���C�Ƀ��X�g�̃Z�b�g�𕛃^�u�ɕ\������@�\
		AppendMenu( hMenu, MF_STRING, IDM_AATREE_SUBADD, atMenuStr );

		dRslt = TrackPopupMenu( hMenu, TPM_RETURNCMD, stPost.x, stPost.y, 0, hWnd, NULL );
		if( IDM_AATREE_SUBADD == dRslt )
		{
			ZeroMemory( &stMulti, sizeof(MULTIPLEMAA) );
			StringCchCopy( stMulti.atBase, MAX_PATH, atSelName );
			//	atFilePath����ɂ��邱�ƂŁA�g�p���X�g������Ă��Ƃ�
			stMulti.dTabNum = 0;	//	�������E�����͂Q�ȍ~

			gltMultiFiles.push_back( stMulti );
			TabMultipleAppend( hWnd );
		}
		DestroyMenu( hMenu );
		return;
	}
//#endif

	//	�c���[�r���[�̃R���e�L�X�g
	if( ghTreeWnd == hWndContext )
	{
		hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_AATREE_POPUP) );
		hSubMenu = GetSubMenu( hMenu, 0 );

#ifndef MAA_PROFILE
		DeleteMenu( hSubMenu, 2, MF_BYPOSITION );
		DeleteMenu( hSubMenu, 1, MF_BYPOSITION );
		DeleteMenu( hSubMenu, 0, MF_BYPOSITION );
#endif
		stTvHitInfo.pt = stPost;
		ScreenToClient( ghTreeWnd, &(stTvHitInfo.pt) );
		hTvHitItem = TreeView_HitTest( ghTreeWnd, &stTvHitInfo );

		//	�I�����ꂽ��̃t�@�C�����A�������̓f�B���N�g�����m��
		lPrm = TreeItemInfoGet( hTvHitItem, atName, MAX_PATH );
		StringCchCat( atName, MAX_PATH, TEXT(" �̑���") );
		//	���̂𖾎����Ă���
		ModifyMenu( hSubMenu, IDM_DUMMY, MF_BYCOMMAND | MF_STRING | MF_GRAYED, IDM_DUMMY, atName );
//		EnableMenuItem( hSubMenu, IDM_DUMMY, MF_GRAYED );
		if( lPrm )	//	Directory�Ȃ�
		{
			EnableMenuItem( hSubMenu, IDM_AATREE_MAINOPEN, MF_GRAYED );
			EnableMenuItem( hSubMenu, IDM_AATREE_SUBADD, MF_GRAYED );
			EnableMenuItem( hSubMenu, IDM_AATREE_GOEDIT, MF_GRAYED );
			EnableMenuItem( hSubMenu, IDM_MAA_IADD_OPEN, MF_GRAYED );
		}

		//	�E�N���ł̓m�[�h�I������Ȃ��悤��
		dRslt = TrackPopupMenu( hSubMenu, TPM_RETURNCMD, stPost.x, stPost.y, 0, hWnd, NULL );	//	TPM_CENTERALIGN | TPM_VCENTERALIGN | 
		DestroyMenu( hMenu );
		switch( dRslt )
		{
#if defined(MAA_PROFILE) && !defined(_ORRVW)
			//	�v���t�t�@�C���J��
			case IDM_MAA_PROFILE_MAKE:	TreeProfileOpen( hWnd );	break;

			//	�f�B���N�g���n���ăZ�b�g	�f�B���N�g���ݒ�_�C�����O���J��
			case IDM_TREE_RECONSTRUCT:	TreeProfileRebuild( hWnd  );	break;
#endif
			case IDM_AATREE_MAINOPEN:	TreeSelItemProc( hWnd, hTvHitItem , 0 );	break;
			case  IDM_AATREE_SUBADD:	TreeSelItemProc( hWnd, hTvHitItem , 1 );	break;
#ifndef _ORRVW
			case  IDM_AATREE_GOEDIT:	TreeSelItemProc( hWnd, hTvHitItem , 2 );	break;
#endif
			case  IDM_MAA_IADD_OPEN:	TreeSelItemProc( hWnd, hTvHitItem , 3 );	break;

			default:	break;
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
				//	�c���[���Ƃ̓A�v���[�`���Ⴄ���璍��
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

#if 0//def MAA_PROFILE

LRESULT CALLBACK TreeTest( UINT id, UINT type, UINT prnt, LPCVOID pVoida )
{
//	TCHAR	atBuff[MAX_STRING];

	TRACE( TEXT("%4u\t%4u\t%4u\t%s"), id, type, prnt, (LPCTSTR)pVoida );
	
	return 0;
}
//-------------------------------------------------------------------------------------------------

#endif

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
	SHFILEINFO	stShFileInfo;

	ZeroMemory( gatAARoot, sizeof(gatAARoot) );
	StringCchCopy( gatAARoot, MAX_PATH, ptCurrent );

	StatusBarMsgSet( 2, TEXT("�c���[���\�z���ł�") );

	TreeView_DeleteAllItems( ghTreeWnd );
	//	���[�g�A�C�e�����
	ZeroMemory( &stTreeIns, sizeof(TVINSERTSTRUCT) );
	stTreeIns.hParent      = TVI_ROOT;
	stTreeIns.hInsertAfter = TVI_SORT;
	stTreeIns.item.mask    = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	stTreeIns.item.pszText = TEXT("ROOT");
	stTreeIns.item.lParam  = 1;

	SHGetFileInfo( TEXT(""), 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON) );
	stTreeIns.item.iImage = stShFileInfo.iIcon;
	SHGetFileInfo( TEXT(""), 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON) );
	stTreeIns.item.iSelectedImage = stShFileInfo.iIcon;

	ghTreeRoot = TreeView_InsertItem( ghTreeWnd, &stTreeIns );

	//	�f�B���N�g���w�肪����������I���
	if( 0 == ptCurrent[0] )
	{
		StatusBarMsgSet( 2, TEXT("") );
		return E_INVALIDARG;
	}

	//	�J�����g�_�f�B���N�g���̓t���p�X�̂͂�

	//	�v���t�@�C�����[�h�Ȃ�A���SQL����ł���

#ifdef MAA_PROFILE
	//	SQL����W�J
	TreeItemFromSql( ptCurrent, ghTreeRoot, 0 );
#else
	//	�t�@�C������\�z
	TreeItemFind( ptCurrent, ghTreeRoot, 0 );	//	�J�����g�ȉ����ċA����
#endif

	StatusBarMsgSet( 2, TEXT("") );
	TreeView_Expand( ghTreeWnd, ghTreeRoot, TVE_EXPAND );

	//	���^�u��SQL����č\�z
	if( bSubTabReb ){	TabMultipleRestore( hWnd  );	}	//	�I�����̕��^�u�𕜋A����

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

#ifdef MAA_PROFILE

/*!
	�f�B���N�g���ƃt�@�C�����r�p�k����c���[�r���[�ɓW�J�E�ċA�ɒ��ӃZ��
	@param[in]	ptRootName	��������f�B���N�g����
	@param[in]	hTreeParent	�Ώۃf�B���N�g���̃c���[�A�C�e���E�����ɂԂ牺���Ă���
	@param[in]	dPrntID		SQL��ID�E�f�B���N�g���ԍ�
	@return		UINT		�Ō��ID
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
	dTgtID = SqlTreeNodeSelectID( dPrntID, &type, &dPnID, atTarget, 1 );
	//	�����Ȃ�����I���
	if( 0 == dTgtID )	return 0;
	//	���̎��_�łh�c���قȂ�̂́A�f�B���N�g���w�肾���������āA���g�������ꍇ
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

		if( FILE_ATTRIBUTE_DIRECTORY == type )	//	�f�B���N�g���̏ꍇ
		{
			stTreeIns.item.lParam  = 1;
			stTreeIns.hInsertAfter = hLastDir;
			hNewParent = TreeView_InsertItem( ghTreeWnd, &stTreeIns );
			hLastDir = hNewParent;

			StringCchCopy( atNewTop, MAX_PATH, ptRootName );
			PathAppend( atNewTop, atTarget );

			dTgtID = TreeItemFromSql( atNewTop, hNewParent, dTgtID );	//	�Y���f�B���N�g�������ċA����
		}
		else	//	�t�@�C���̏ꍇ
		{
			stTreeIns.item.lParam  = 0;
			stTreeIns.hInsertAfter = TVI_LAST;
			hNewParent = TreeView_InsertItem( ghTreeWnd, &stTreeIns );
		}

		//	���̎��_��dTgtID���O�ɂȂ邱�Ƃ͂Ȃ��H

		dPrvID = dTgtID;
		ZeroMemory( atTarget, sizeof(atTarget) );
//		SetLastError(0);
		dTgtID = SqlTreeNodeSelectID( dTgtID, &type, &dPnID, atTarget, 1 );
//���ŃG���[
		TRACE( TEXT("%4u\t%4u\t%4u\t%s"), dTgtID, type, dPnID, atTarget );

		if( 0 == dTgtID )	bLooping = FALSE;
		if( dPrntID != dPnID )	bLooping = FALSE;
	}
	while( bLooping );

	return dPrvID;
}
//-------------------------------------------------------------------------------------------------

#else

//�t�@�C��������������Ă���E�v���t�@�C�����[�h�Ȃ�g��Ȃ����H
/*!
	�f�B���N�g���ƃt�@�C�����c���[�r���[�ɓW�J�E�ċA�ɒ��ӃZ��
	@param[in]	ptRootName	��������f�B���N�g����
	@param[in]	hTreeParent	�Ώۃf�B���N�g���̃c���[�A�C�e���E�����ɂԂ牺���Ă���
	@param[in]	dPrntID		SQL��ID�E�f�B���N�g���ԍ�
	@return		HRESULT		�I����ԃR�[�h
*/
HRESULT TreeItemFind( LPCTSTR ptRootName, HTREEITEM hTreeParent, UINT dPrntID )
{
	HANDLE	hFind;
	TCHAR	atPath[MAX_PATH], atNewTop[MAX_PATH], atTarget[MAX_PATH];
	BOOL	bRslt;
	UINT	dPnID = 0;

	WIN32_FIND_DATA	stFindData;

	HTREEITEM	hNewParent, hLastDir = TVI_FIRST;
	TVINSERTSTRUCT	stTreeIns;
	SHFILEINFO	stShFileInfo;

	ZeroMemory( &stTreeIns, sizeof(TVINSERTSTRUCT) );
	stTreeIns.hParent      = hTreeParent;
	stTreeIns.hInsertAfter = TVI_LAST;
	stTreeIns.item.mask    = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;

	ZeroMemory( atTarget, sizeof(atTarget) );
	StringCchCopy( atTarget, MAX_PATH, ptRootName );
	PathAppend( atTarget, TEXT("*") );


	hFind = FindFirstFile( atTarget, &stFindData );	//	TEXT("*")
	do{
		if( lstrcmp( stFindData.cFileName, TEXT("..") ) && lstrcmp( stFindData.cFileName, TEXT(".") ) )
		{
			StringCchCopy( atPath, MAX_PATH, ptRootName );
			PathAppend( atPath, stFindData.cFileName );

			if( stFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{	//	�f�B���N�g���̏ꍇ
				SHGetFileInfo( atPath, 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON) );
				stTreeIns.item.iImage = stShFileInfo.iIcon;
				SHGetFileInfo( atPath, 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON) );
				stTreeIns.item.iSelectedImage = stShFileInfo.iIcon;

				stTreeIns.item.pszText = stFindData.cFileName;
				stTreeIns.item.lParam  = 1;
				stTreeIns.hInsertAfter = hLastDir;
				hNewParent = TreeView_InsertItem( ghTreeWnd, &stTreeIns );
				hLastDir = hNewParent;

				StringCchCopy( atNewTop, MAX_PATH, ptRootName );
				PathAppend( atNewTop, stFindData.cFileName );

				TreeItemFind( atNewTop, hNewParent, dPnID );	//	�Y���f�B���N�g�������ċA����
			}
			else
			{	//	�t�@�C���̏ꍇ
				bRslt  = PathMatchSpec( stFindData.cFileName, TEXT("*.mlt") );	//	�q�b�g������TRUE
				bRslt |= PathMatchSpec( stFindData.cFileName, TEXT("*.ast") );	//	�q�b�g������TRUE
				if( bRslt )	//	20110720	AST���C�P��悤�ɂ���
				{
					SHGetFileInfo( atPath, 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON) );
					stTreeIns.item.iImage = stShFileInfo.iIcon;
					SHGetFileInfo( atPath, 0, &stShFileInfo, sizeof(SHFILEINFO), (SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON) );
					stTreeIns.item.iSelectedImage = stShFileInfo.iIcon;

					stTreeIns.hInsertAfter = TVI_LAST;
					stTreeIns.item.pszText = stFindData.cFileName;
					stTreeIns.item.lParam  = 0;
					hNewParent = TreeView_InsertItem( ghTreeWnd, &stTreeIns );
				}
			}
		}

	}while( FindNextFile( hFind, &stFindData ) );

	FindClose( hFind );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

#endif

/*!
	�c���[�̃A�C�e���̖��O�Ƃo�`�q�`�l�����m��
	@param[in]	hTrItem	�A�C�e���n���h��
	@param[out]	ptName	���O������o�b�t�@�ւ̃|�C���^�ENULL�ł��ǂ�
	@param[in]	cchName	�o�b�t�@�T�C�Y
	@return		LPARAM	�o�`�q�`�l���
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

	HTREEITEM	hSelItem;

	nmCode = pstNmTrView->hdr.code;

	//	�E�N���b�N�̓R���e�L�X�g���j���[��

	if( TVN_SELCHANGED == nmCode )	//	�I��������
	{
		hSelItem = TreeView_GetSelection( ghTreeWnd );	//	�I������Ă�A�C�e��

		AaTitleClear(  );	//	�����ŃN�����[���Ȃ�����

		TreeSelItemProc( hWnd, hSelItem, 0 );
	}

	return 0;
}
//-------------------------------------------------------------------------------------------------

/*!
	�c���[�̑I�������A�C�e������̏����̃`�F�C���E����𓝍�
	@param[in]	hWnd		�e�E�C���h�E�̃n���h��
	@param[in]	hSelItem	�I�����Ă�A�C�e��
	@param[in]	dMode		�O��^�u�Ł@�P���^�u�Ł@�Q�ҏW�r���[�Ł@�J���@�R�A�C�e���ǉ�
	@return		�����������e�Ƃ�
*/
INT TreeSelItemProc( HWND hWnd, HTREEITEM hSelItem, UINT dMode )
{
	UINT	i;
	TCHAR	atName[MAX_PATH], atPath[MAX_PATH], atBaseName[MAX_PATH];
	LPARAM	lParam;
#ifndef _ORRVW
	LPARAM	dNumber;
#endif
	HTREEITEM	hParentItem;

	MULTIPLEMAA	stMulti;

	if( !(hSelItem) ){	return 0;	}	//	�Ȃ񂩖����Ȃ牽�����Ȃ�

	//	�I�����ꂽ��̃t�@�C�����A�������̓f�B���N�g�����m��
	lParam = TreeItemInfoGet( hSelItem, atName, MAX_PATH );

	if( lParam ){	return 0;	}	//	�f�B���N�g���Ȃ牽�����Ȃ�

	//	�I���������O���m�ہE���[�g�ɂ���ꍇ����œK�p�����
	if( 0 == dMode )
	{
		StringCchCopy( gatBaseName, MAX_PATH, atName );
		StatusBarMsgSet( 2, atName );	//	�X�e�[�^�X�o�[�Ƀt�@�C�����\��
	}
	else
	{
		StringCchCopy( atBaseName, MAX_PATH, atName );
	}

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

	switch( dMode )
	{
		default:	//	��^�u�ŊJ���ꍇ
		case  0:	AaItemsDoShow( hWnd , atPath, ACT_ALLTREE );	break;	//	����MLT���J��

		case  1:	//	���^�u�ɊJ���ꍇ
			ZeroMemory( &stMulti, sizeof(MULTIPLEMAA) );
			StringCchCopy( stMulti.atFilePath, MAX_PATH, atPath );
			StringCchCopy( stMulti.atBase, MAX_PATH, atBaseName );
			stMulti.dTabNum = 0;	//	�������E�����͂Q�ȍ~

			gltMultiFiles.push_back( stMulti );
			TabMultipleAppend( hWnd );
			break;
#ifndef _ORRVW
		case  2:	//	�ҏW�r���[�ŊJ���ꍇ
			dNumber = DocFileInflate( atPath  );	//	�J���Ē��g�W�J
	#ifdef MULTI_FILE
			if( dNumber ){	MultiFileTabAppend( dNumber, atPath );	}
	#endif
			break;

		//	�A�C�e���ǉ�
		case  3:	AacItemAdding( hWnd, atPath );	break;
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

		AaTitleClear(  );	//	�����ŕϊ����Ė��Ȃ��͂�

		if( ACT_ALLTREE == curSel )
		{
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
			if( ptBase ){	StringCchCopy( ptBase, cchSize, itNulti->atBase );	}

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
	LPARAM	dNumber;
#endif

	if( 0 == dMode )	gixUseTab = tabSel;


	for( itNulti = gltMultiFiles.begin( ); gltMultiFiles.end( ) != itNulti; itNulti++ )
	{
		if( tabSel == itNulti->dTabNum )	//	�I������Ă���������
		{
			if( 0 == dMode )	//	�r���[�G���A�ɕ\��
			{
				//	��_�f�B���N�g�����Z�b�g
				StringCchCopy( gatBaseName, MAX_PATH, itNulti->atBase );

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

				StatusBarMsgSet( 2, atName );	//	�X�e�[�^�X�o�[�Ƀt�@�C�����\��
			}
#ifndef _ORRVW
			else	//	�t�@�C�������m�ۂ��āA����ɕҏW�r���[���ŊJ������������
			{
				dNumber = DocFileInflate( itNulti->atFilePath );	//	�J���Ē��g�W�J
	#ifdef MULTI_FILE
				if( dNumber ){	MultiFileTabAppend( dNumber, itNulti->atFilePath );	}
	#endif
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
	�����t�@�C����INI�ɕۑ�����
	@param[in]	hWnd	�E�C���h�E�n���h��
	@return		HRESULT	�I����ԃR�[�h
*/
HRESULT TabMultipleStore( HWND hWnd )
{
	MLTT_ITR	itNulti;

#ifdef MAA_PROFILE
	//	��U�S�������ď����������Ă�
	SqlMultiTabDelete(  );
	for( itNulti = gltMultiFiles.begin( ); gltMultiFiles.end( ) != itNulti; itNulti++ )
	{
		SqlMultiTabInsert( itNulti->atFilePath, itNulti->atBase );
	}
#else
	UINT_PTR	iRslt;

	iRslt = gltMultiFiles.size( );
	InitMultipleFile( INIT_SAVE, iRslt, NULL, NULL );
	for( i = 0, itNulti = gltMultiFiles.begin( ); gltMultiFiles.end( ) != itNulti; i++, itNulti++ )
	{
		InitMultipleFile( INIT_SAVE, i, itNulti->atFilePath, itNulti->atBase );
	}
#endif
	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	�����t�@�C����INI����ǂݍ���ōēW�J����
	@param[in]	hWnd	�E�C���h�E�n���h��
	@return		HRESULT	�I����ԃR�[�h
*/
HRESULT TabMultipleRestore( HWND hWnd )
{
	INT	iCount, i;
	MULTIPLEMAA	stMulti;

	TabMultipleDeleteAll( hWnd );

#ifdef MAA_PROFILE
	iCount = SqlTreeCount( 2, NULL );
#else
	iCount = InitMultipleFile( INIT_LOAD, 0, NULL, NULL );
#endif
	for( i = 0; iCount > i; i++ )
	{
		ZeroMemory( &stMulti, sizeof(MULTIPLEMAA) );
#ifdef MAA_PROFILE
		SqlMultiTabSelect( i+1, stMulti.atFilePath, stMulti.atBase );
#else
		InitMultipleFile( INIT_LOAD, i, stMulti.atFilePath, stMulti.atBase );
#endif
		gltMultiFiles.push_back( stMulti );
		TabMultipleAppend( hWnd );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

#if 0
/*!
	���^�u�ɒǉ�����Ƃ�
	@param[in]	hWnd		�e�E�C���h�E�̃n���h��
	@param[in]	hSelItem	�I�����Ă�A�C�e��
	@return	��O�J�����@�O�J���Ȃ��������
*/
INT TabMultipleOpen( HWND hWnd, HTREEITEM hSelItem )
{
	UINT	i;
	TCHAR	atName[MAX_PATH], atPath[MAX_PATH], atBaseName[MAX_PATH];
	LPARAM	lParam;
	HTREEITEM	hParentItem;

	MULTIPLEMAA	stMulti;


	if( !(hSelItem) ){	return 0;	}	//	�Ȃ񂩖����Ȃ牽�����Ȃ�

	lParam = TreeItemInfoGet( hSelItem, atName, MAX_PATH );

	if( lParam ){	return 0;	}	//	�f�B���N�g���Ȃ牽�����Ȃ�

	//	�I���������O�E���[�g�ɂ���ꍇ����œK�p�����
	StringCchCopy( atBaseName, MAX_PATH, atName );

//	StatusBarMsgSet( 2, atName );	//	�X�e�[�^�X�o�[�Ƀt�@�C�����\��

	//	��ɒH���āA�t�@�C���p�X�����
	for( i = 0; 12 > i; i++ )	//	�P�Q��蕅�C�K�w�͂��ǂ�Ȃ�
	{
		hParentItem = TreeView_GetParent( ghTreeWnd, hSelItem );
		if( !(hParentItem) )	return 0;	//	���[�g�̏�͂Ȃ��E�����đI��������Ȃ�
		if( ghTreeRoot == hParentItem ){	break;	}	//	���������[�g�܂ŃC�b����I���

		TreeItemInfoGet( hParentItem, atPath, MAX_PATH );
		StringCchCopy( atBaseName, MAX_PATH, atPath );
		//	���̃p�X���L�^���Ă����ƁA�ŏI�I�Ƀ��[�g�����̃f�B���N�g�����ɂȂ�
		PathAppend( atPath, atName );
		StringCchCopy( atName, MAX_PATH, atPath );

		hSelItem = hParentItem;
	}

	StringCchCopy( atPath, MAX_PATH, gatAARoot );
	PathAppend( atPath, atName );



	ZeroMemory( &stMulti, sizeof(MULTIPLEMAA) );
	StringCchCopy( stMulti.atBase, MAX_PATH, atBaseName );
	StringCchCopy( stMulti.atFilePath, MAX_PATH, atPath );
	stMulti.dTabNum = 0;	//	�������E�����͂Q�ȍ~

	gltMultiFiles.push_back( stMulti );
	TabMultipleAppend( hWnd );

	return 1;
}
//-------------------------------------------------------------------------------------------------
#endif

/*!
	�^�u�𑝂₷�E�ێ����X�g�Ƀt�@�C�����Ԃ����񂾂璼���ɌĂԂׂ�
	@param[in]	hWnd	�e�E�C���h�E�̃n���h��
	@return		HRESULT	�I����ԃR�[�h
*/
HRESULT TabMultipleAppend( HWND hWnd )
{
	TCHAR	atName[MAX_PATH];
	LONG	tCount;
//	RECT	itRect;
	TCITEM	stTcItem;

	MLTT_ITR	itNulti;


	itNulti = gltMultiFiles.end( );
	itNulti--;	//	�V�����J���͖̂��[�ɂ���͂�
	StringCchCopy( atName, MAX_PATH, itNulti->atFilePath );
	if( NULL !=  atName[0] )	//	�c���[�r���[����Ȃ�
	{
		PathStripPath( atName );
		PathRemoveExtension( atName );	//	�g���q���O��
	}
	else	//	���C�Ƀ��X�g����ǉ�����
	{
		StringCchCopy( atName, MAX_PATH, itNulti->atBase );
		StringCchCat(  atName, MAX_PATH, TEXT("[F]") );
	}

	ZeroMemory( &stTcItem, sizeof(TCITEM) );
	stTcItem.mask = TCIF_TEXT | TCIF_PARAM;

	tCount = TabCtrl_GetItemCount( ghTabWnd );

	stTcItem.lParam  = 0;//tCount;�t�@�C���Ȃ̂łO�ł���
	stTcItem.pszText = atName;
	TabCtrl_InsertItem( ghTabWnd, tCount, &stTcItem );

	itNulti->dTabNum = tCount;

	//TabCtrl_GetItemRect( ghTabWnd, 1, &itRect );
	//itRect.bottom -= itRect.top;
	//itRect.right -= itRect.left;
	//itRect.top = 0;
	//itRect.left = 0;
	//TabCtrl_AdjustRect( ghTabWnd, 0, &itRect );
	//height = itRect.top;
	//GetWindowRect( ghTabWnd, &itRect );
	//itRect.right -= itRect.left;
	//MoveWindow( ghTabWnd, 0, 0, itRect.right, height, TRUE );

	Maa_OnSize( hWnd, 0, 0, 0 );	//	�����͎g���ĂȂ�������

	return S_OK;
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
	for( i = 2; ttlSel > i; i++ ){	TabCtrl_DeleteItem( ghTabWnd, i );	}

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
	�S�̃c���[�A���C�ɓ��胊�X�g�A�����t�@�C���A�J���Ă郄�c��Ԃ�
	@return	INT	�J���Ă�z�̔ԍ�
*/
INT TreeFavWhichOpen( VOID )
{
	return gixUseTab;	//	ACT_ALLTREE	ACT_FAVLIST
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




