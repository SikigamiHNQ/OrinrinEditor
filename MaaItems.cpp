/*! @file
	@brief �`�`�ꗗ�̐�������܂�
	���̃t�@�C���� MaaItems.cpp �ł��B
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

#include "stdafx.h"
#include "OrinrinEditor.h"
#include "MaaTemplate.h"
//-------------------------------------------------------------------------------------------------

typedef struct tagVIEWORDER
{
	UINT	index;
	UINT	dHeight;
	UINT	dUpper;
	UINT	dDownr;

} VIEWORDER, *LPVIEWORDER;


typedef struct tagAATITLE
{
	UINT	number;
	TCHAR	atTitle[MAX_STRING];

} AATITLE, *LPAATITLE;
//-------------------------------------------------------------------------------------------------

//static LOGFONT	gstBaseFont = {
//	FONTSZ_NORMAL,			//	�t�H���g�̍���
//	0,						//	���ϕ�
//	0,						//	��������̕�����X���Ƃ̊p�x
//	0,						//	�x�[�X���C����X���Ƃ̊p�x
//	FW_NORMAL,				//	�����̑���(0~1000�܂ŁE400=nomal)
//	FALSE,					//	�C�^���b�N��
//	FALSE,					//	�A���_�[���C��
//	FALSE,					//	�ł�������
//	DEFAULT_CHARSET,		//	�����Z�b�g
//	OUT_OUTLINE_PRECIS,		//	�o�͐��x
//	CLIP_DEFAULT_PRECIS,	//	�N���b�s���O���x
//	PROOF_QUALITY,			//	�o�͕i��
//	VARIABLE_PITCH,			//	�Œ蕝���ϕ�
//	TEXT("�l�r �o�S�V�b�N")	//	�t�H���g��
//};
//-------------------------------------------------------------------------------------------------

#define TITLECBX_HEI	200

#define SBP_DIRECT		0xFF

static  HWND	ghItemsWnd;			//!<	���X�g�̃n���h��
#ifdef MAA_TOOLTIP
static  HWND	ghToolTipWnd;		//!<	�c�[���`�b�v
EXTERNED HFONT	ghTipFont;			//!<	�c�[���`�b�v�p
#endif

static  HWND	ghComboxWnd;		//!<	���o���p�R���{�b�N�X

static WNDPROC	gpfOrgAaItemsProc;	//!<	
static WNDPROC	gpfOrgAaTitleCbxProc;	//!<	

static LPTSTR	gptTipBuffer;		//!<	

static INT		gixTopItem;			//!<	�ꗗ�̍ŏ��
static INT		gixMaxItem;			//!<	�A�C�e����

static  LONG	gixNowSel;			//!<	�}�E�X�J�[�\��������Ƃ���̃C���f�b�N�X
#ifdef USE_HOVERTIP
static  LONG	gixNowToolTip;		//!<	�c�[���`�b�v�p�Z���N�g
#endif

static  HWND	ghScrollWnd;		//!<	�X�N���[���o�[

#ifdef _ORRVW
EXTERNED HFONT	ghAaFont;			//!<	�\���p�̃t�H���g
#else
static HFONT	ghAaFont;			//!<	�\���p�̃t�H���g
#endif

static HBRUSH	ghBkBrush;			//!<	�\���G�����̔w�i�F

static  HPEN	ghSepPen;			//!<	��؂���p�y��
static BOOLEAN	gbLineSep;			//!<	AA�̕����͐��ɂ���

#ifndef _ORRVW
static BOOLEAN	gbMaaRetFocus;		//!<	���ڂ�I��������ҏW���Ƀt�H�[�J�X�߂���
#endif

#ifdef MAA_TEXT_FIND
TCHAR	gatFindText[MAX_STRING];	//!<	�����p������̕ۑ�
#endif

extern  UINT	gbAAtipView;		//!<	��O�ŁA�`�`�c�[���`�b�v�\��

extern  HWND	ghSplitaWnd;		//!<	�X�v���b�g�o�[�n���h��

static vector<VIEWORDER>	gvcViewOrder;	//!<	�������Ă��̓��e
static vector<AATITLE>		gvcAaTitle;		//!<	
//-------------------------------------------------------------------------------------------------

#ifdef MAA_TOOLTIP
LRESULT	Aai_OnNotify( HWND , INT, LPNMHDR );			//!<	
#endif
VOID	Aai_OnMouseMove( HWND, INT, INT, UINT );		//!<	
VOID	Aai_OnLButtonUp( HWND, INT, INT, UINT );		//!<	
VOID	Aai_OnMButtonUp( HWND, INT, INT, UINT );		//!<	
VOID	Aai_OnContextMenu( HWND, HWND, UINT, UINT );	//!<	
VOID	Aai_OnDropFiles( HWND , HDROP );				//!<	

HRESULT	AaItemsFavDelete( LPSTR, UINT );	//!<	

#ifdef MAA_TEXT_FIND
UINT	AacItemFindOnePage( HWND, LPTSTR, INT );	//!<	
#endif

LRESULT	CALLBACK gpfAaItemsProc( HWND, UINT, WPARAM, LPARAM );		//!<	
LRESULT	CALLBACK gpfAaTitleCbxProc( HWND, UINT, WPARAM, LPARAM );	//!<	

#ifndef _ORRVW
INT_PTR	CALLBACK AaItemAddDlgProc( HWND, UINT, WPARAM, LPARAM );	//!<	
#endif

#ifdef USE_HOVERTIP
LPTSTR	CALLBACK AaItemsHoverTipInfo( LPVOID  );	//!<	
#endif
//-------------------------------------------------------------------------------------------------

/*!
	�S�c���[�₨�C�Ƀ��X�g�̓��e��\������X�^�e�B�b�N�Ƃ����
	@param[in]	hWnd	�e�E�C���h�E�n���h���ENULL�Ȃ�j��
	@param[in]	hInst	�A�v���̎���
	@param[in]	ptRect	�N���C�A���g�̈�̃T�C�Y
	@return		HRESULT	�I����ԃR�[�h
*/
HRESULT AaItemsInitialise( HWND hWnd, HINSTANCE hInst, LPRECT ptRect )
{
#ifdef MAA_TOOLTIP
	INT		ttSize;
	TTTOOLINFO	stToolInfo;
#endif
	SCROLLINFO	stScrollInfo;
	RECT	rect;
	LOGFONT	stFont;

	COLORREF	dBkColour;

	if( !(hWnd) )
	{
		free( gptTipBuffer );
		SetWindowFont( ghItemsWnd, GetStockFont(DEFAULT_GUI_FONT), FALSE );
#ifdef MAA_TOOLTIP
		SetWindowFont( ghToolTipWnd, GetStockFont(DEFAULT_GUI_FONT), FALSE );
		DeleteFont( ghTipFont );
#endif
		DeleteFont( ghAaFont );
		DeletePen( ghSepPen );
		DeleteBrush( ghBkBrush );
		return S_FALSE;
	}

//�E�C���h�E�̃T�C�Y�́A���ƂŕύX�����ł���̂ŁA�����ł͌Œ�l�ō���Ă���

	ghSepPen  = CreatePen( PS_SOLID, 1, RGB(0xAA,0xAA,0xAA) );

	//	�P�Ȃ��؂���X�^�C��
	gbLineSep = InitParamValue( INIT_LOAD, VL_MAASEP_STYLE, 0 );

	dBkColour = (COLORREF)InitParamValue( INIT_LOAD, VL_MAA_BKCOLOUR, 0x00FFFFFF );
	ghBkBrush = CreateSolidBrush( dBkColour );

#ifndef _ORRVW
	//	�I��������t�H�[�J�X��ҏW���ɖ߂��H
	gbMaaRetFocus = InitParamValue( INIT_LOAD, VL_MAA_RETFCS, 0 );
#endif

	gptTipBuffer = NULL;

	gixTopItem = 0;

	gixNowSel = -1;
#ifdef USE_HOVERTIP
	gixNowToolTip = -1;
#endif

#ifdef MAA_TEXT_FIND
	ZeroMemory( gatFindText, sizeof(gatFindText) );
#endif

#ifdef MAA_TOOLTIP
	//	�c�[���`�b�v���
	ghToolTipWnd = CreateWindowEx( WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWnd, NULL, hInst, NULL );
#endif
	//	���o���R���{�b�N�X
	ghComboxWnd = CreateWindowEx( 0, WC_COMBOBOX, TEXT(""), WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_NOINTEGRALHEIGHT, TREE_WIDTH + SPLITBAR_WIDTH, 0, ptRect->right - TREE_WIDTH - LSSCL_WIDTH, TITLECBX_HEI, hWnd, (HMENU)IDCB_AAITEMTITLE, hInst, NULL );
	GetClientRect( ghComboxWnd, &rect );
	//	�T�u�N���X��
	gpfOrgAaTitleCbxProc = SubclassWindow( ghComboxWnd, gpfAaTitleCbxProc );

	//	AA�ꗗ�̃X�^�e�B�b�N����E�I�[�i�[�h���[�ŕ`��
	ghItemsWnd = CreateWindowEx( WS_EX_CLIENTEDGE | WS_EX_ACCEPTFILES, WC_STATIC, TEXT(""), WS_VISIBLE | WS_CHILD | SS_OWNERDRAW | SS_NOTIFY, TREE_WIDTH + SPLITBAR_WIDTH, rect.bottom, ptRect->right - TREE_WIDTH - LSSCL_WIDTH, ptRect->bottom - rect.bottom, hWnd, (HMENU)IDSO_AAITEMS, hInst, NULL );
	//DragAcceptFiles( ghItemsWnd, TRUE );	WS_EX_ACCEPTFILES�ł���
	//	�T�u�N���X��
	gpfOrgAaItemsProc = SubclassWindow( ghItemsWnd, gpfAaItemsProc );

	//	�ꗗ�̃X�N���[���o�[
	ghScrollWnd = CreateWindowEx( 0, WC_SCROLLBAR, TEXT("scroll"), WS_VISIBLE | WS_CHILD | SBS_VERT, ptRect->right - LSSCL_WIDTH, rect.bottom, LSSCL_WIDTH, ptRect->bottom - rect.bottom, hWnd, (HMENU)IDSB_LISTSCROLL, hInst, NULL );

	ZeroMemory( &stScrollInfo, sizeof(SCROLLINFO) );
	stScrollInfo.cbSize = sizeof(SCROLLINFO);
	stScrollInfo.fMask = SIF_DISABLENOSCROLL;
	SetScrollInfo( ghScrollWnd, SB_CTL, &stScrollInfo, TRUE );

	//	�\���p���C���t�H���g
	ViewingFontGet( &stFont );
	ghAaFont = CreateFontIndirect( &stFont );	//	gstBaseFont
	SetWindowFont( ghItemsWnd, ghAaFont, TRUE );

#ifdef MAA_TOOLTIP
	//	�|�b�p�b�v�`�b�v�p�E12/9pt���p
	ttSize = InitParamValue( INIT_LOAD, VL_MAATIP_SIZE, FONTSZ_REDUCE );	//	�T�C�Y�m�F
	stFont.lfHeight = (FONTSZ_REDUCE == ttSize) ? FONTSZ_REDUCE : FONTSZ_NORMAL;
	ghTipFont = CreateFontIndirect( &stFont );
	SetWindowFont( ghToolTipWnd, ghTipFont, TRUE );

	//	�c�[���`�b�v���R�[���o�b�N�Ŋ���t��
	ZeroMemory( &stToolInfo, sizeof(TTTOOLINFO) );
	stToolInfo.cbSize   = sizeof(TTTOOLINFO);
	stToolInfo.uFlags   = TTF_SUBCLASS;
	stToolInfo.hinst    = NULL;	//	
	stToolInfo.hwnd     = ghItemsWnd;
	stToolInfo.uId      = IDSO_AAITEMS;
	GetClientRect( ghItemsWnd, &stToolInfo.rect );
	stToolInfo.lpszText = LPSTR_TEXTCALLBACK;	//	�R�����w�肷��ƃR�[���o�b�N�ɂȂ�
	SendMessage( ghToolTipWnd, TTM_ADDTOOL, 0, (LPARAM)&stToolInfo );
	SendMessage( ghToolTipWnd, TTM_SETMAXTIPWIDTH, 0, 0 );	//	�`�b�v�̕��B�O�ݒ�ł����B���ꂵ�Ƃ��Ȃ��Ɖ��s����Ȃ�
#endif

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	���o���R���{�b�N�X�̃T�u�N���X�v���V�[�W��
	@param[in]	hWnd	���X�g�̃n���h��
	@param[in]	msg		�E�C���h�E���b�Z�[�W�̎��ʔԍ�
	@param[in]	wParam	�ǉ��̏��P
	@param[in]	lParam	�ǉ��̏��Q
	@return	�������ʂƂ�
*/
LRESULT	CALLBACK gpfAaTitleCbxProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_CHAR, Maa_OnChar );
		default:	break;
	}

	return CallWindowProc( gpfOrgAaTitleCbxProc, hWnd, msg, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------


/*!
	���o���R���{�b�N�X����ɂ���
*/
VOID AaTitleClear( VOID )
{
	ComboBox_SetCurSel( ghComboxWnd, -1 );

	while( ComboBox_GetCount( ghComboxWnd ) ){	ComboBox_DeleteString( ghComboxWnd, 0 );	}
	gvcAaTitle.clear( );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	���o���R���{�b�N�X�ɓ��e�ǉ�
	@param[in]	number	�ʂ��ԍ�
	@param[in]	pcTitle	���ꍞ�ޕ�����
	@return	�p�����������Ƃ̍��ڐ�
*/
INT AaTitleAddString( UINT number, LPSTR pcTitle )
{
	AATITLE	stTitle;
	LPTSTR	ptTitle;

	ptTitle = SjisDecodeAlloc( pcTitle );

	ComboBox_AddString( ghComboxWnd, ptTitle );

	ZeroMemory( &stTitle, sizeof(AATITLE) );
	stTitle.number = number;
	StringCchCopy( stTitle.atTitle, MAX_STRING, ptTitle );

	gvcAaTitle.push_back( stTitle );

	FREE(ptTitle);

	return ComboBox_GetCount( ghComboxWnd );
}
//-------------------------------------------------------------------------------------------------

/*!
	�R���{�b�N�X�R�}���h����
	@param[in]	hWnd		�e�E�C���h�E�n���h��
	@param[in]	codeNotify	�ʒm���b�Z�[�W
*/
VOID AaTitleSelect( HWND hWnd, UINT codeNotify )
{
	INT	iSel;
	INT_PTR	iItems;

//	TRACE( TEXT("COMBOX[%u]"), codeNotify );

	if( CBN_SELCHANGE == codeNotify )	//	�I�����ύX���ꂽ
	{
		iSel = ComboBox_GetCurSel( ghComboxWnd );	//	�I�΂�Ă�̎����

		iItems = gvcAaTitle.size( );	//	�͂ݏo���m�F
		if( iItems <= iSel )	return;

		//	���o���̊Y������ʒu�փW�����v
		Aai_OnVScroll( hWnd, ghScrollWnd, SBP_DIRECT, gvcAaTitle.at( iSel ).number );
	}

	if( CBN_CLOSEUP == codeNotify )
	{
		SetFocus( ghItemsWnd );
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	��ʃT�C�Y���ς�����̂ŃT�C�Y�ύX
	@param[in]	hWnd	�e�E�C���h�E�n���h��
	@param[in]	ptRect	�N���C�A���g�̈�
*/
VOID AaItemsResize( HWND hWnd, LPRECT ptRect )
{
	INT		dWidth, dLeft;
	RECT	sptRect, rect;
#ifdef MAA_TOOLTIP
	TTTOOLINFO	stToolInfo;
#endif

	SplitBarPosGet( ghSplitaWnd, &sptRect );
	//	�g���^�u�o�[�̈ʒu�m��
//	MaaTabBarSizeGet( &tbRect );

	dWidth = ptRect->right - (sptRect.left + SPLITBAR_WIDTH) - LSSCL_WIDTH;
	dLeft  = ptRect->right - LSSCL_WIDTH;

	MoveWindow( ghComboxWnd, sptRect.left + SPLITBAR_WIDTH, ptRect->top, dWidth, TITLECBX_HEI, TRUE );
	GetClientRect( ghComboxWnd, &rect );

	MoveWindow( ghItemsWnd,  sptRect.left + SPLITBAR_WIDTH, ptRect->top + rect.bottom, dWidth, ptRect->bottom - rect.bottom, TRUE );
	MoveWindow( ghScrollWnd, dLeft, ptRect->top + rect.bottom, LSSCL_WIDTH, ptRect->bottom - rect.bottom, TRUE );

#ifdef MAA_TOOLTIP
	//	�K�v�ȏ����������΂���
	ZeroMemory( &stToolInfo, sizeof(TTTOOLINFO) );
	stToolInfo.cbSize = sizeof(TTTOOLINFO);
	stToolInfo.hwnd   = ghItemsWnd;
	stToolInfo.uId    = IDSO_AAITEMS;
	GetClientRect( ghItemsWnd, &stToolInfo.rect );
	SendMessage( ghToolTipWnd, TTM_NEWTOOLRECT, 0, (LPARAM)&stToolInfo );
#endif
	InvalidateRect( ghItemsWnd, NULL, TRUE );
	UpdateWindow( ghItemsWnd );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	AA�\���X�^�e�B�b�N�̃T�u�N���X�v���V�[�W���E�c�[���`�b�v�̏����ɕK�v
	@param[in]	hWnd	���X�g�̃n���h��
	@param[in]	msg		�E�C���h�E���b�Z�[�W�̎��ʔԍ�
	@param[in]	wParam	�ǉ��̏��P
	@param[in]	lParam	�ǉ��̏��Q
	@return		LRESULT	�������ʂƂ�
*/
LRESULT CALLBACK gpfAaItemsProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_CHAR,        Maa_OnChar );
		HANDLE_MSG( hWnd, WM_KEYDOWN,     Aai_OnKey );			//	20120221
		HANDLE_MSG( hWnd, WM_KEYUP,       Aai_OnKey );			//	

		HANDLE_MSG( hWnd, WM_MOUSEMOVE,   Aai_OnMouseMove );	//	�}�E�X��������
		HANDLE_MSG( hWnd, WM_LBUTTONUP,   Aai_OnLButtonUp );	//	�}�E�X���{�^������
		HANDLE_MSG( hWnd, WM_MBUTTONUP,   Aai_OnMButtonUp );	//	�}�E�X���{�^������
		HANDLE_MSG( hWnd, WM_CONTEXTMENU, Aai_OnContextMenu );	//	�R���e�L�X�g���j���[����
		HANDLE_MSG( hWnd, WM_DROPFILES,   Aai_OnDropFiles );	//	�h���O���h���b�v�̎�t
#ifdef MAA_TOOLTIP
		HANDLE_MSG( hWnd, WM_NOTIFY,      Aai_OnNotify );		//	�R�����R���g���[���̌ʃC�x���g
#endif


#ifdef USE_HOVERTIP
		case WM_MOUSEHOVER:
			HoverTipOnMouseHover( hWnd, wParam, lParam, AaItemsHoverTipInfo );
			return 0;

		case WM_MOUSELEAVE:
			HoverTipOnMouseLeave( hWnd );
			gixNowToolTip = -1;
			return 0;
#endif


		default:	break;
	}

	return CallWindowProc( gpfOrgAaItemsProc, hWnd, msg, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------

/*!
	AA�\���X�^�e�B�b�N�̃I�[�i�[�h���[�E�`��
	@param[in]	hWnd			�e�E�C���h�E�n���h��
	@param[in]	*pstDrawItem	�h���[���
	@return		�Ȃ�
*/
VOID AaItemsDrawItem( HWND hWnd, CONST DRAWITEMSTRUCT *pstDrawItem )
{
	UINT_PTR	rdLen;
	INT_PTR	rdLength;
	LPSTR	pcConts = NULL;
	LPTSTR	ptConStr = NULL;
	RECT	rect, drawRect;
	POINT	stPoint;

	VIEWORDER	stVwrder;

	INT		rdNextItem;
	LONG	rdDrawPxTop, rdBottom;	//	�`��̍ŏ�ʁA�`��̈�̍����l�`�w
	LONG	rdHeight, rdWidth;	//	�A�C�e���̍����E�`��̈�̕�

	HPEN	hOldPen;

	//	���ڂ̊O�ڎl�p�`�̎擾
	rect = pstDrawItem->rcItem;
	rdBottom = rect.bottom;
	rdWidth  = rect.right;
	rdDrawPxTop = 0;

	SetBkMode( pstDrawItem->hDC , TRANSPARENT );	//	�����`��͔w�i���߂Ŗ�I����

	FillRect( pstDrawItem->hDC, &rect, ghBkBrush );	//	��U�w�i�h��Ԃ���

	gvcViewOrder.clear();

	rdNextItem = gixTopItem;
	for( rdDrawPxTop = 0; rdBottom > rdDrawPxTop; rdNextItem++ )
	{
		pcConts = AacAsciiArtGet( rdNextItem );	//	�ꗗ�̃g�b�v���m�F
		if( !pcConts ){	break;	}

		stVwrder.index = rdNextItem;

		ptConStr = SjisDecodeAlloc( pcConts );	//	�\�����e�K���Ă���
		StringCchLength( ptConStr, STRSAFE_MAX_CCH, &rdLen );
		rdLength = rdLen;

		free( pcConts );
#pragma message ("MAA�̍s�ԁA�����Ő������v�Z����ׂ�")
		//	������ɍ��킹��RECT�m�ہE�v�Z�����ŁA�܂��������ĂȂ�
		DrawText( pstDrawItem->hDC, ptConStr, rdLength, &rect, DT_LEFT | DT_EDITCONTROL | DT_NOPREFIX | DT_CALCRECT );
		drawRect = rect;
		rdHeight = drawRect.bottom;
		drawRect.bottom += rdDrawPxTop;
		drawRect.top     = rdDrawPxTop;
		if( drawRect.right < rdWidth )	drawRect.right = rdWidth;

		stVwrder.dHeight = rdHeight;
		stVwrder.dUpper  = drawRect.top;
		stVwrder.dDownr  = drawRect.bottom;

		//	���ڂ��ƂɌ݂��Ⴂ�̐F�ɂ��邩�A��؂��������
		if( gbLineSep ){	FillRect( pstDrawItem->hDC, &drawRect, ghBkBrush );	}
		else
		{
			if( 1 & rdNextItem )	FillRect( pstDrawItem->hDC, &drawRect, GetStockBrush(LTGRAY_BRUSH) );
			else					FillRect( pstDrawItem->hDC, &drawRect, ghBkBrush );
		}

		//	�`�`�`��
		DrawText( pstDrawItem->hDC, ptConStr, rdLength, &drawRect, DT_LEFT | DT_EDITCONTROL | DT_NOPREFIX );

		if( gbLineSep )	//	��������
		{
			hOldPen = SelectPen( pstDrawItem->hDC, ghSepPen );
			MoveToEx( pstDrawItem->hDC, drawRect.left, drawRect.bottom-1, NULL );
			LineTo( pstDrawItem->hDC, drawRect.right, drawRect.bottom-1 );
			SelectPen( pstDrawItem->hDC, hOldPen );
		}

		gvcViewOrder.push_back( stVwrder );

		rdDrawPxTop += rdHeight;

		free( ptConStr );
	}
	InvalidateRect( ghScrollWnd, NULL, TRUE );
	UpdateWindow( ghScrollWnd );

	//	�J�[�\���ʒu�m�ۂ������E�|�b�p�b�v�\���p
	GetCursorPos( &stPoint );
	ScreenToClient( ghItemsWnd, &stPoint );
	Aai_OnMouseMove( hWnd, stPoint.x, stPoint.y, 0 );

	return;
}
//-------------------------------------------------------------------------------------------------

#if 0

/*!
	�I�[�i�[�h���[�E�T�C�Y�v��
	@param[in]	hWnd			�e�E�C���h�E�n���h��
	@param[in]	pstMeasureItem	�T�C�Y���
	@return		HRESULT			�I����ԃR�[�h
*/
VOID AaItemsMeasureItem( HWND hWnd, LPMEASUREITEMSTRUCT pstMeasureItem )
{
	HDC		hDC;
	INT		rdLength, rdHeight;
	LPSTR	pcConts;
	RECT	stRect;

	//	���ڂ̕�����擾
	pcConts = AacAsciiArtGet( pstMeasureItem->itemID );
	if( !pcConts )	return;

	rdLength = strlen( pcConts );

	ListBox_GetItemRect( ghItemsWnd, pstMeasureItem->itemID, &stRect );

	hDC = GetDC( ghItemsWnd );
	DrawTextExA( hDC, pcConts, rdLength, &stRect, DT_LEFT | DT_EDITCONTROL | DT_NOPREFIX | DT_CALCRECT );
	ReleaseDC( ghItemsWnd, hDC );

	pstMeasureItem->itemHeight = (stRect.bottom - stRect.top);
	if( 256 <= pstMeasureItem->itemHeight )	pstMeasureItem->itemHeight = 255;

	free( pcConts );

	return;
}
//-------------------------------------------------------------------------------------------------
#endif


/*!
	�L�[�_�E��������
	@param[in]	hWnd	�E�C���h�E�n���h���E�r���[�̂Ƃ͌���Ȃ��̂Œ��ӃZ��
	@param[in]	vk		�����ꂽ�L�[�����z�L�[�R�[�h�ŗ���
	@param[in]	fDown	��O�_�E���@�O�A�b�v
	@param[in]	cRepeat	�A���I�T���񐔁E���ĂȂ��H
	@param[in]	flags	�L�[�t���O���낢��
	@return		����
*/
VOID Aai_OnKey( HWND hWnd, UINT vk, BOOL fDown, INT cRepeat, UINT flags )
{
	TRACE( TEXT("KEY[%u][%u]"), vk, cRepeat );

	if( !(fDown) )	 return;	//	�Ƃ肠�����L�[�A�b�v�͂��邱�ƂȂ�

//	�������Ȃ��R���g���[���Ƃ��m�F�Z��

	switch( vk )
	{
		default:	return;

		case  VK_NEXT:	//	PageDown
			Aai_OnVScroll( hWnd, ghScrollWnd, SB_PAGEDOWN, 0 );
			break;

		case  VK_DOWN:	//	��
			Aai_OnVScroll( hWnd, ghScrollWnd, SB_LINEDOWN, 0 );
			break;

		case  VK_UP:	//	��
			Aai_OnVScroll( hWnd, ghScrollWnd, SB_LINEUP, 0 );
			break;

		case  VK_PRIOR:	//	PageUp
			Aai_OnVScroll( hWnd, ghScrollWnd, SB_PAGEUP, 0 );
			break;
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	�}�E�X���������Ƃ��̏���
	@param[in]	hWnd		�e�E�C���h�E�n���h��
	@param[in]	x			�N���C�A���g���W�w
	@param[in]	y			�N���C�A���g���W�x
	@param[in]	keyFlags	������Ă鑼�̃{�^��
	@return		�Ȃ�
*/
VOID Aai_OnMouseMove( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	TCHAR		atBuffer[MAX_STRING];
	UINT_PTR	i, max;
	LONG		iItem = -1, bottom;
	BOOLEAN		bReDraw = FALSE;

	INT		iDot = 0, iLine = 0, iByte = 0;

	//	���̂Ƃ��}�E�X�J�[�\�����ɂ���A�C�e����I�����Ă���

	if( !( gvcViewOrder.empty() ) )
	{
		max = gvcViewOrder.size();

		bottom = 0;
		for( i = 0; max > i; i++ )
		{
			bottom += gvcViewOrder.at( i ).dHeight;

			if( y < bottom ){	iItem = gvcViewOrder.at(  i ).index;	break;	}
		}
	}
	//	�J�[�\�����������Ȃ�AiItem�́|�P���

#ifdef USE_HOVERTIP
	if( gixNowToolTip != iItem ){	bReDraw =  TRUE;	}
	gixNowSel = iItem;
	gixNowToolTip = gixNowSel;

	if( bReDraw && gbAAtipView ){	HoverTipResist( ghItemsWnd );	}
#endif

#ifdef MAA_TOOLTIP
	if( gixNowSel != iItem ){	bReDraw =  TRUE;	}
	gixNowSel = iItem;

	if( bReDraw && gbAAtipView )	SendMessage( ghToolTipWnd, TTM_UPDATE, 0, 0 );
#endif

	if( bReDraw )
	{
		iDot = AacArtSizeGet( iItem, &iLine, &iByte );
		//pcConts = AacAsciiArtGet( iItem );
		//if( pcConts )
		//{
		//	ptString = SjisDecodeAlloc( pcConts );
		//	FREE( pcConts );
		//	if( ptString )
		//	{
		//		iDot = TextViewSizeGet( ptString, &iLine );
		//		FREE( ptString );
		//	}
		//}

		TRACE( TEXT("MAA MOUSE %3d[%dDOT x %dLINE]%dByte"), iItem+1, iDot, iLine, iByte );
#ifdef _ORRVW
		StringCchPrintf( atBuffer, MAX_STRING, TEXT("%3d[%dDOT x %dLINE]"), iItem+1, iDot, iLine );
#else
		StringCchPrintf( atBuffer, MAX_STRING, TEXT("%3d[%dDOT x %dLINE] %dByte"), iItem+1, iDot, iLine, iByte );
#endif
		StatusBarMsgSet( SBMAA_AXIS, atBuffer );
	}


	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	�}�E�X�̍��{�^�������������Ƃ��̏���
	@param[in]	hWnd			�e�E�C���h�E�n���h��
	@param[in]	x				�N���C�A���g���W�w
	@param[in]	y				�N���C�A���g���W�x
	@param[in]	keyFlags		������Ă鑼�̃{�^��
	@return		�Ȃ�
*/
VOID Aai_OnLButtonUp( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	AaItemsDoSelect( hWnd, MAA_DEFAULT, TRUE );

#ifndef _ORRVW
	if( gbMaaRetFocus ){	ViewFocusSet(  );	}
#endif
	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	�}�E�X�̒��{�^�������������Ƃ��̏���
	@param[in]	hWnd			�e�E�C���h�E�n���h��
	@param[in]	x				�N���C�A���g���W�w
	@param[in]	y				�N���C�A���g���W�x
	@param[in]	keyFlags		������Ă鑼�̃{�^��
	@return		�Ȃ�
*/
VOID Aai_OnMButtonUp( HWND hWnd, INT x, INT y, UINT keyFlags )
{
	AaItemsDoSelect( hWnd, MAA_SUBDEFAULT, TRUE );

#ifndef _ORRVW
	if( gbMaaRetFocus ){	ViewFocusSet(  );	}
#endif
	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	�X�N���[���̏���
	@param[in]	hWnd	�E�C���h�E�n���h��
	@param[in]	hwndCtl	�X�N���[���o�[�̃E�C���h�E�n���h��
	@param[in]	code	�����ԃR�[�h�E0xFF�����R�[�h�Ƃ��Ďg��
	@param[in]	pos		�܂݂̈ʒu
	@return		�����������e�Ƃ�
*/
VOID Aai_OnVScroll( HWND hWnd, HWND hwndCtl, UINT code, INT pos )
{
	INT	maePos;
	SCROLLINFO	stScrollInfo;

	if( ghScrollWnd != hwndCtl )	return;

	//	��Ԃ����₵��
	ZeroMemory( &stScrollInfo, sizeof(SCROLLINFO) );
	stScrollInfo.cbSize = sizeof(SCROLLINFO);
	stScrollInfo.fMask = SIF_ALL;
	GetScrollInfo( ghScrollWnd, SB_CTL, &stScrollInfo );

	maePos = gixTopItem;

	switch( code )	//	�X�N���[�������ɍ��킹�ē��e�����炷
	{
		case SB_TOP:
			gixTopItem = 0;
			break;

		case SB_LINEUP:
		case SB_PAGEUP:
			gixTopItem--;
			if( 0 > gixTopItem )	gixTopItem = 0;
			break;

		case SB_LINEDOWN:
		case SB_PAGEDOWN:
			gixTopItem++;
			if( gixMaxItem <=  gixTopItem ){	gixTopItem = gixMaxItem-1;	}
			break;

		case SB_BOTTOM:
			gixTopItem = gixMaxItem - 1;
			break;

		case SB_THUMBTRACK:
			gixTopItem = stScrollInfo.nTrackPos;
			break;

		case SBP_DIRECT:	//	���[�U�R�[�h����ɒǉ�
			gixTopItem = pos;
			break;

		default:	return;
	}
	//	�ω��Ȃ��Ȃ�Ȃɂ�����ł���
	if( maePos == gixTopItem )	return;

	InvalidateRect( ghItemsWnd, NULL, TRUE );
	UpdateWindow( ghItemsWnd  );	//	���A���^�C���`��ɕK�v

	stScrollInfo.fMask = SIF_POS;
	stScrollInfo.nPos  = gixTopItem;
	SetScrollInfo( ghScrollWnd, SB_CTL, &stScrollInfo, TRUE );

	TabMultipleTopMemory( gixTopItem );

	return;
}
//-------------------------------------------------------------------------------------------------

#ifdef MAA_TOOLTIP
/*!
	�m�[�e�B�t�@�C���b�Z�[�W�̏���
	@param[in]	hWnd		�E�C���h�E�n���h��
	@param[in]	idFrom		NOTIFY�𔭐��������R���g���[���̂h�c
	@param[in]	pstNmhdr	NOTIFY�̏ڍ�
	@return		�����������e�Ƃ�
*/
LRESULT Aai_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{
	UINT_PTR		rdLength;
	LPSTR			pcConts = NULL;
	LPNMTTDISPINFO	pstDispInfo;

	if( TTN_GETDISPINFO ==  pstNmhdr->code )	//	�c�[���`�b�v�̓��e�̖₢���킹��������
	{
		pstDispInfo = (LPNMTTDISPINFO)pstNmhdr;

		if( !(gbAAtipView) )	//	��\���Ȃ牽�����Ȃ��ł���
		{
			ZeroMemory( &(pstDispInfo->szText), sizeof(pstDispInfo->szText) );
			pstDispInfo->lpszText = NULL;
			return 0;
		}

		FREE( gptTipBuffer );

		pcConts = AacAsciiArtGet( gixNowSel );	//	�Y������C���f�b�N�XAA�����������Ă���
		if( !pcConts  ){	return 0;	}

		rdLength = strlen( pcConts );	//	������̒����擾

		//gptTipBuffer = (LPTSTR)malloc( sizeof(TCHAR) * (rdLength+1) );
		//ZeroMemory( gptTipBuffer, sizeof(TCHAR) * (rdLength+1) );
		//MultiByteToWideChar( CP_ACP, MB_COMPOSITE, pcConts, -1, gptTipBuffer, (rdLength+1) );

		gptTipBuffer = SjisDecodeAlloc( pcConts );

		pstDispInfo->lpszText = gptTipBuffer;

		free( pcConts );
	}

	return 0;
}
//-------------------------------------------------------------------------------------------------
#endif

/*!
	�R���e�L�X�g���j���[�Ăт����i�E�N���b�N�j
	@param[in]	hWnd		�E�C���h�E�n���h��
	@param[in]	hWndContext	�E�N���b�N�����q�E�C���h�E�n���h��
	@param[in]	xPos		�}�E�X�J�[�\���̃X�N���[���w���W
	@param[in]	yPos		�}�E�X�J�[�\���̃X�N���[���x���W
	@return		�Ȃ�
*/
VOID Aai_OnContextMenu( HWND hWnd, HWND hWndContext, UINT xPos, UINT yPos )
{
	HMENU	hMenu, hSubMenu;
	UINT	dRslt;
	INT		dOpen;	//	�S�c���[�Ƃ��C�Ƀ��X�g�J���Ă��
	LPSTR	pcConts = NULL;
	UINT_PTR	rdLength;
	INT		sx, sy;

	dOpen = TabMultipleNowSel(  );	//	�J���Ă�̂ŏ���������
	//	ACT_ALLTREE	ACT_FAVLIST

#pragma message ("Editor��viewer�́A���j���[���\�[�X�̐������ɒ��ӃZ��")
	hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_AALIST_POPUP) );
	hSubMenu = GetSubMenu( hMenu, 0 );

#ifdef _ORRVW
	//	�g�p���X�g�̂݁A�폜��L���ɁA�ύX���邱�ƁE�W���Ŗ����ɂ��Ă���
	if( ACT_FAVLIST ==  dOpen ){	EnableMenuItem( hSubMenu, IDM_MAA_FAV_DELETE , MF_ENABLED );	}
	//	��ʃA�C�e�����폜�o����悤�ɂ���
#endif

	//	�c�[���`�b�v�̕\���E��\���̃g�S�D
	if( gbAAtipView ){	CheckMenuItem( hSubMenu, IDM_MAA_AATIP_TOGGLE, MF_CHECKED );	}

#ifndef _ORRVW
	//	�t�H�[�J�X�߂����ǂ���
	if( gbMaaRetFocus ){	CheckMenuItem( hSubMenu, IDM_MAA_RETURN_FOCUS, MF_CHECKED );	}
#endif
	//	�}���`���j�^���Ă�ƁA���W�l���}�C�i�X�ɂȂ邱�Ƃ�����B
	sx = (SHORT)xPos;
	sy = (SHORT)yPos;

	//	gixNowSel�́A���������Ƃ��낾�Ɓ|�P�ɂȂ�

	//	�t���O��TPM_RETURNCMD���w�肷��ƁAWM_COMMAND����΂Ȃ�
	dRslt = TrackPopupMenu( hSubMenu, TPM_RETURNCMD, sx, sy, 0, hWnd, NULL );	//	TPM_CENTERALIGN | TPM_VCENTERALIGN | 
	DestroyMenu( hMenu );
	switch( dRslt )
	{
		case IDM_MAA_FAV_DELETE:
			if( ACT_FAVLIST == dOpen )	//	�g�p�̏ꍇ
			{
				pcConts = AacAsciiArtGet( gixNowSel );	//	�Y������C���f�b�N�XAA�����������Ă���
				if( !pcConts ){	return;	}

				rdLength = strlen( pcConts );	//	������̒����擾
				AaItemsFavDelete( pcConts, rdLength );	//	�폜Commando���s
				FavContsRedrawRequest( hWnd );	//	�ĕ`�悵�Ȃ��Ⴞ��
			}
#ifndef _ORRVW
			else	//	����ȊO�Ȃ�A��^�u�����^�u
			{
				AacItemDelete( hWnd, gixNowSel );
			}
#endif
			break;

#ifndef _ORRVW
		case IDM_MAA_INSERT_EDIT:		AaItemsDoSelect( hWnd, MAA_INSERT,   TRUE );	break;
		case IDM_MAA_INTERRUPT_EDIT:	AaItemsDoSelect( hWnd, MAA_INTERRUPT, TRUE );	break;
		case IDM_MAA_SET_LAYERBOX:		AaItemsDoSelect( hWnd, MAA_LAYERED,  TRUE );	break;
#endif
		case IDM_MAA_CLIP_UNICODE:		AaItemsDoSelect( hWnd, MAA_UNICLIP,  TRUE );	break;
		case IDM_MAA_CLIP_SHIFTJIS:		AaItemsDoSelect( hWnd, MAA_SJISCLIP, TRUE );	break;

		case IDM_DRAUGHT_ADDING:		AaItemsDoSelect( hWnd, MAA_DRAUGHT,  TRUE );	break;
#ifdef _ORRVW
		case IDM_DRAUGHT_OPEN:			Maa_OnCommand( hWnd, IDM_DRAUGHT_OPEN, NULL, 0 );	break;
#endif

		case IDM_MAA_AATIP_TOGGLE:
			gbAAtipView = gbAAtipView ? FALSE : TRUE;
			InitParamValue( INIT_SAVE, VL_MAATIP_VIEW, gbAAtipView );
			break;

		case IDM_MAA_SEP_STYLE_TOGGLE:
			gbLineSep = gbLineSep ? FALSE : TRUE;
			InitParamValue( INIT_SAVE, VL_MAASEP_STYLE, gbLineSep );
			InvalidateRect( ghItemsWnd, NULL, TRUE );
			break;

		case IDM_MAA_THUMBNAIL_OPEN:	Maa_OnCommand( hWnd , IDM_MAA_THUMBNAIL_OPEN, NULL, 0 );	break;

		case IDM_MAAITEM_BKCOLOUR:		MaaBackColourChoose( hWnd );	break;

#ifndef _ORRVW

		case IDM_MAA_RETURN_FOCUS:
			gbMaaRetFocus = gbMaaRetFocus ? FALSE : TRUE;
			InitParamValue( INIT_SAVE, VL_MAA_RETFCS, gbMaaRetFocus );
			break;

  #ifdef MAA_IADD_PLUS
		//	�r���ǉ�
		case IDM_MAA_ITEM_INSERT:		AacItemInsert( hWnd, gixNowSel );	break;
  #endif
#endif
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	�h���b�O���h���b�v�̎󂯓���
	@param[in]	hWnd	�\���X�^�e�B�b�N�̃n���h��
	@param[in]	hDrop	�h���b�s���I�u�W�F�N�g�n���h�D
*/
VOID Aai_OnDropFiles( HWND hWnd, HDROP hDrop )
{
	TCHAR	atFileName[MAX_PATH];
//	LPARAM	dNumber;

	ZeroMemory( atFileName, sizeof(atFileName) );

	DragQueryFile( hDrop, 0, atFileName, MAX_PATH );
	DragFinish( hDrop );

	TRACE( TEXT("AAI DROP[%s]"), atFileName );

	//	�h���b�y���ꂽ�t�@�C���𕛃^�u�ɓW�J�����Ⴄ
	TabMultipleDropAdd( GetParent( hWnd ), atFileName );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	MLT�t�@�C�����������͂��C�ɓ��薼���󂯎���āA���e���X�^�e�B�b�N�ɓ����
	@param[in]	hWnd		�e�E�C���h�E�n���h��
	@param[in]	ptFileName	�t�@�C����
	@param[in]	type		�O(ACT_ALLTREE)�F�t�@�C���W�J�@�P(ACT_FAVLIST)�FSQL�W�J�@�Q�F���^�u���
	@return		HRESULT		�I����ԃR�[�h
*/
HRESULT AaItemsDoShow( HWND hWnd, LPTSTR ptFileName, UINT type )
{
	SCROLLINFO	stScrollInfo;

	ZeroMemory( &stScrollInfo, sizeof(SCROLLINFO) );
	stScrollInfo.cbSize = sizeof(SCROLLINFO);

	//	�`��ʒu���ŏ�����ɂ��āA�ĕ`���Ԃɂ���
	gixTopItem = 0;

	AaTitleClear(  );	//	�����ŃN�����[���Ă݂�

	//	�J��
	switch( type )
	{
		case ACT_ALLTREE:	gixMaxItem = AacAssembleFile( hWnd, ptFileName );	break;
		case ACT_FAVLIST:	gixMaxItem = AacAssembleSql( hWnd, ptFileName );	break;
		default:	//	���^�u
			gixMaxItem = AacAssembleFile( hWnd, ptFileName );
			//	���X�g�������[�E�����ŕ`��ʒu�𕜌�����
			gixTopItem = TabMultipleTopMemory( -1 );
			break;
	}


	InvalidateRect( ghItemsWnd, NULL, TRUE );
	UpdateWindow( ghItemsWnd );

	if( 0 == gixMaxItem )
	{
		stScrollInfo.fMask = SIF_DISABLENOSCROLL;
		SetScrollInfo( ghScrollWnd, SB_CTL, &stScrollInfo, TRUE );
		return E_FAIL;
	}

	stScrollInfo.fMask = SIF_ALL;
	stScrollInfo.nMax  = gixMaxItem-1;
	stScrollInfo.nPos  = gixTopItem;
	stScrollInfo.nPage = 1;
	stScrollInfo.nTrackPos = 0;
	SetScrollInfo( ghScrollWnd, SB_CTL, &stScrollInfo, TRUE );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	�g�����`�`�����C�ɓ���r�p�k�ɂ����
	@param[in]	pcConts		AA�̕�����
	@param[in]	rdLength	�o�C�g��
	@return		HRESULT		�I����ԃR�[�h
*/
HRESULT AaItemsFavUpload( LPSTR pcConts, UINT rdLength )
{
	LPTSTR	ptBaseName;
	DWORD	dHash;

	ptBaseName = TreeBaseNameGet(  );	//	�ꗗ�x�[�X���擾����

	//	�h���b�s���I�u�W�F�N�g�Ȃ疳������̂ŁH
	if( !( StrCmp( DROP_OBJ_NAME, ptBaseName ) ) )	return E_ABORT;

	//	���̂`�`�̃n�b�V���l�����߂�
	HashData( (LPBYTE)pcConts, rdLength, (LPBYTE)(&(dHash)), 4 );

	//	���C�ɓ���ɋL�^����
	SqlFavUpload( ptBaseName, dHash, pcConts, rdLength );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	���C�ɓ��胊�X�g����폜����
	@param[in]	pcConts		AA�̕�����
	@param[in]	rdLength	�o�C�g��
	@return		HRESULT		�I����ԃR�[�h
*/
HRESULT AaItemsFavDelete( LPSTR pcConts, UINT rdLength )
{
	LPTSTR	ptBaseName;
	DWORD	dHash;

	ptBaseName = TreeBaseNameGet(  );	//	�ꗗ�x�[�X���擾����

	//	���̂`�`�̃n�b�V���l�����߂�
	HashData( (LPBYTE)pcConts, rdLength, (LPBYTE)(&(dHash)), 4 );

	//	���C�ɓ��肩��폜����
	SqlFavDelete( ptBaseName, dHash );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------


/*!
	MAA���̔w�i�F�I���_�C�����O�̖ʓ|����
	@param[in]	hWnd	�I�[�i�[�E�C���h�E�n���h��
	@return	HRESULT	�I����ԃR�[�h
*/
HRESULT MaaBackColourChoose( HWND hWnd )
{
	BOOL	bRslt;
	COLORREF	adColourTemp[16], dColour;
	CHOOSECOLOR	stChColour;

	ZeroMemory( adColourTemp, sizeof(adColourTemp) );

	dColour = (COLORREF)InitParamValue( INIT_LOAD, VL_MAA_BKCOLOUR, 0x00FFFFFF );

	adColourTemp[0] = dColour;

	ZeroMemory( &stChColour, sizeof(CHOOSECOLOR) );
	stChColour.lStructSize  = sizeof(CHOOSECOLOR);
	stChColour.hwndOwner    = hWnd;
//	stChColour.hInstance    = GetModuleHandle( NULL );
	stChColour.rgbResult    = dColour;
	stChColour.lpCustColors = adColourTemp;
	stChColour.Flags        = CC_RGBINIT;

	bRslt = ChooseColor( &stChColour  );	//	�F�_�C�����O�g��
	if( bRslt )
	{
		dColour =  stChColour.rgbResult;
		InitParamValue( INIT_SAVE, VL_MAA_BKCOLOUR, (INT)dColour );

		DeleteBrush( ghBkBrush );	//	�u���V��蒼��
		ghBkBrush = CreateSolidBrush( dColour );

		InvalidateRect( ghItemsWnd, NULL, TRUE );

		return S_OK;
	}

	return E_ABORT;
}
//-------------------------------------------------------------------------------------------------




/*!
	�z�E�B�[����]�������̏�Ŕ���������
	@param[in]	hWnd	�e�E�C���h�E�n���h��
	@param[in]	hChdWnd	�}�E�X�J�[�\���́��ɂ������q�E�C���h�E
	@param[in]	zDelta	��]�ʁE���Ȃ������A���Ȃ牺�����ւ̃X�N���[���ƌ��Ȃ�
	@return		��O�����������@�O�֌W�Ȃ���
*/
UINT AaItemsIsUnderCursor( HWND hWnd, HWND hChdWnd, INT zDelta )
{
	UINT	dCode;

	if( ghItemsWnd != hChdWnd )	return 0;

	if( 0 < zDelta )		dCode = SB_LINEUP;
	else if( 0 > zDelta )	dCode = SB_LINEDOWN;
	else					dCode = SB_ENDSCROLL;

	FORWARD_WM_VSCROLL( hWnd, ghScrollWnd, dCode, 0, PostMessage );

	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	�^�[�Q�b�g���Ă���AA���g������
	@param[in]	hWnd	����AA�ꗗ�̃E�C���h�E�n���h��
	@param[in]	dMode	�g�p���[�h�������̓f�t�H��
	@param[in]	dDirct	��O�}�E�X�����̂Ł@�O�\���g�b�v�̂�
	@return		UINT	��O�`�`�Ƃ����@�O�`�`��������
*/
UINT AaItemsDoSelect( HWND hWnd, UINT dMode, UINT dDirct )
{
	LPSTR		pcConts = NULL;
	UINT		uRslt;
	UINT_PTR	rdLength;

	//	�Y������C���f�b�N�XAA�����������Ă���
	if( dDirct ){	pcConts = AacAsciiArtGet( gixNowSel );	}
	else{			pcConts = AacAsciiArtGet( gixTopItem );	}

	if( !pcConts  ){	return 0;	}

	rdLength = strlen( pcConts );	//	������̒����擾

	uRslt = ViewMaaMaterialise( hWnd, pcConts, rdLength, dMode );	//	�{�̂ɔ�΂�
	//	Editor��Viewer�Ŗ{�̂��قȂ�̂Œ���

	//	�����ł��C�ɓ���ɓ����
	if( SUCCEEDED( AaItemsFavUpload( pcConts, rdLength ) ) )
	{
		FavContsRedrawRequest( hWnd );
	}

	free( pcConts );

	return 1;
}
//-------------------------------------------------------------------------------------------------


/*!
	�|�b�p�b�v�̕����T�C�Y�ύX�ƕ\����\����������
	@param[in]	ttSize	�����T�C�Y�E�P�Q���P�U�����Ȃ�
	@param[in]	bView	��O�\���@�O�\�����Ȃ�
	@return		HRESULT	�I����ԃR�[�h
*/
HRESULT AaItemsTipSizeChange( INT ttSize, UINT bView )
{
#ifdef MAA_TOOLTIP
	LOGFONT	stFont;
#endif
	gbAAtipView = bView ? TRUE : FALSE;

#ifdef MAA_TOOLTIP
	SetWindowFont( ghToolTipWnd, GetStockFont(DEFAULT_GUI_FONT), FALSE );
	DeleteFont( ghTipFont );

	//	���g���Ă郄�c�Ԃ��󂵂Ă���A�V�����̂����Ă�������
	ViewingFontGet( &stFont );	//	gstBaseFont
	stFont.lfHeight = (FONTSZ_REDUCE == ttSize) ? FONTSZ_REDUCE : FONTSZ_NORMAL;
	ghTipFont = CreateFontIndirect( &stFont );

	SetWindowFont( ghToolTipWnd, ghTipFont, TRUE );
#endif

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

#ifdef USE_HOVERTIP
/*!
	HoverTip�p�̃R�[���o�b�N���
	@param[in]	pVoid	����`
	@return	�m�ۂ���������E��������NULL
*/
LPTSTR CALLBACK AaItemsHoverTipInfo( LPVOID pVoid )
{
	UINT_PTR	rdLength;
	LPSTR		pcConts = NULL;
	LPTSTR		ptBuffer = NULL;


	if( !(gbAAtipView) ){		return NULL;	}	//	��\���Ȃ牽�����Ȃ��ł���
	if( 0 > gixNowToolTip ){	return NULL;	}

	pcConts = AacAsciiArtGet( gixNowToolTip );	//	�Y������C���f�b�N�X�̂`�`�����������Ă���
	if( !pcConts  ){	return 0;	}

	ptBuffer = SjisDecodeAlloc( pcConts );
	rdLength = lstrlen( ptBuffer  );	//	������̒����擾

	free( pcConts );

	TRACE( TEXT("MAA HOVER CALL %d, by[%d]"), gixNowToolTip, rdLength );

	return ptBuffer;
}
//-------------------------------------------------------------------------------------------------
#endif


#ifdef MAA_TEXT_FIND


/*!
	�����J�n�{�^��������E����Ɠ��ڈȍ~�̋�ʂɒ���
	@param[in]	hWnd	�e�E�C���h�E�̃n���h��
	@param[in]	bMode	��O�{�^���ɂ�錟���J�n�@�O�O�̏����Ŏ��̕ł��猟���J�n
	@return		HRESULT	�I����ԃR�[�h
*/
HRESULT AacFindTextEntry( HWND hWnd, UINT bMode )
{
	TCHAR	atString[MAX_STRING];
	UINT	isNowPage, dRslt;
	INT		iPage, i;

	TRACE( TEXT("MAA�F�����n��") );
	//	�e�R�Ȃ�A���̕ł���B�����̕������
	//	�{�^���Ȃ�A�����񂪈قȂ��Ă�����w��ɏ]���E�����Ȃ�Â�����

	ZeroMemory( atString, sizeof(atString) );

	SetDlgItemText( hWnd, IDS_MAA_TXTFIND_MSGBOX, TEXT("") );

	if( bMode )	//	�{�^������
	{
		GetDlgItemText( hWnd, IDE_MAA_TXTFIND_TEXT, atString, MAX_STRING );
		//	�G�f�B�b�g�{�b�L�X����Ȃ�Ȃ������
		if( NULL == atString[0] )	return E_NOTIMPL;

		//	�O�t�@�C���擪����@�P���̓V�ӕł���
		isNowPage = IsDlgButtonChecked( hWnd, IDB_MAA_TXTFIND_TOP_GO ) ? FALSE : TRUE;
	}
	else	//	�e�R��
	{
		//	���������������Ƃ��͂Ȃɂ����Ȃ�
		if( NULL == gatFindText[0] )	return E_NOTIMPL;

		//	�o�b�t�@����R�s�[���Ă����E����ɂ��O��Ɠ������������ɂȂ�
		StringCchCopy( atString, MAX_STRING, gatFindText );
	}

//����Ɠ��ڈȍ~�̋�ʂǂ����邩
	if( StrCmp( gatFindText, atString ) )	//	�قȂ邩��A���񏉂߂ĂƂ�����
	{
		//	�o�b�t�@�ɃR�s�[
		StringCchCopy( gatFindText, MAX_STRING, atString );

		//	�����J�n�ʒu
		if( !(isNowPage)  ){	iPage = 0;	}	//	�擪
		else{	iPage =  gixTopItem + 1;	}	//	���̎��̕ł���
		if( gixMaxItem <= iPage ){	iPage = 0;	}	//	�U��؂�����擪�ɖ߂�

	}
	else	//	�����Ȃ�P�x����������̂͂��E�������珈���𑱂���
	{
		iPage =  gixTopItem + 1;	//	���̎��̕ł���
		if( gixMaxItem <= iPage ){	iPage = 0;	}	//	�U��؂�����擪�ɖ߂�
	}

	dRslt = FALSE;
	//	�ł����Ԃɂ݂Ă���
	for( i = 0; gixMaxItem > i; i++ )	//	�S�ŉ񂵂���I���
	{
		dRslt = AacItemFindOnePage( hWnd, atString, iPage );
		if( dRslt ){	break;	}	//	�q�b�g

		//	�������łɂ͖��������ꍇ
		iPage++;
		if( gixMaxItem <= iPage ){	iPage = 0;	}	//	�U��؂�����擪�ɖ߂�
	}

	if( dRslt )
	{
		Aai_OnVScroll( hWnd, ghScrollWnd, SBP_DIRECT, iPage );	//	�Y������ʒu�փW�����v
	}
	else
	{
		SetDlgItemText( hWnd, IDS_MAA_TXTFIND_MSGBOX, TEXT("�Q�s���G") );
		//MessageBox( hWnd, TEXT("������Ȃ�������"), TEXT("���ӂ���̂��m�点"), MB_OK );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	�w��̕ł��A�w��������󂯂Č�������
	@param[in]	hWnd		�E�C���h�E�n���h��
	@param[in]	ptFindText	����������������
	@param[in]	iTargetPage	������
	@return	UINT	��O���̕łɕ����񂪂������@�O�Ȃ�����
*/
UINT AacItemFindOnePage( HWND hWnd, LPTSTR ptFindText, INT iTargetPage )
{
	LPSTR	pcItem;
	LPTSTR	ptTarget, ptFindPos;
	INT		iMoziPos;
	UINT	dFound = FALSE;

	//	���g�����Ă���
	pcItem = AacAsciiArtGet( iTargetPage );
	//	���j�R�[�h�ɒ����Ĕ�r����
	ptTarget = SjisDecodeAlloc( pcItem );
	FREE( pcItem );

	ptFindPos = FindStringProc( ptTarget, ptFindText, &iMoziPos );	//	Viewer��
	if( ptFindPos ){	dFound = TRUE;	}	//	���݂��Ă邩�H

	FREE( ptTarget );

	return dFound;
}
//-------------------------------------------------------------------------------------------------


#endif




#ifndef MAA_IADD_PLUS

#ifndef _ORRVW

typedef struct tagITEMADDINFO
{
	LPTSTR	ptContent;			//!<	�{�����e
	TCHAR	atSep[MAX_PATH];	//!<	�Z�p���[�^���e
	INT		bType;				//!<	��OMLT�@�OAST

} ITEMADDINFO, *LPITEMADDINFO;
//--------------------------------------

/*!
	�c���[�̃t�@�C���ɃA�C�e���ǉ��E�ǉ��������t�@�C���̃p�X���󂯎��
	@param[in]	hWnd	�E�C���h�E�n���h��
	@param[in]	ptFile	�ǉ��������t�@�C���̃t���p�X
*/
HRESULT AacItemAdding( HWND hWnd, LPTSTR ptFile )
{
	HANDLE	hFile;

	LPTSTR		ptBuffer;//, ptExten;
	LPSTR		pcOutput;
//	TCHAR		atExBuf[10];
	CHAR		acCheck[6];
	DWORD		readed, wrote;
	UINT_PTR	cchSize, cchSep, cbSize;
	ITEMADDINFO	stIaInfo;	

	ZeroMemory( &stIaInfo, sizeof(ITEMADDINFO) );

	ZeroMemory( acCheck, sizeof(acCheck) );

	//	�g���q�m�F
	if( FileExtensionCheck( ptFile, TEXT(".ast") ) ){	stIaInfo.bType =  0;	}
	else{	stIaInfo.bType =  1;	}

	if( DialogBoxParam( GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAA_IADD_DLG), hWnd, AaItemAddDlgProc, (LPARAM)(&stIaInfo) ) )
	{
		if( stIaInfo.ptContent )	//	���g���L���Ȃ珈������
		{
			hFile = CreateFile( ptFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
			if( INVALID_HANDLE_VALUE != hFile )
			{
				//	�t�@�C����SJIS�^�ł���ƌ��Ȃ�
				SetFilePointer( hFile, -2, NULL, FILE_END );	//	�����̏󋵂��m�F
				ReadFile( hFile, acCheck, 6, &readed, NULL );	//	���̎��_�Ńt�@�C�������ɂ���
				SetFilePointer( hFile,  0, NULL, FILE_END );	//	�O�̂��ߖ���
				if( acCheck[0] != '\r' || acCheck[1] != '\n' )	//	���������s����Ȃ�������
				{
					acCheck[0] = '\r';	acCheck[1] = '\n';	acCheck[2] = NULL;
					WriteFile( hFile, acCheck, 2, &wrote, NULL );	//	���s�����Ă���
				}

				StringCchLength( stIaInfo.atSep, MAX_PATH, &cchSep );
				StringCchLength( stIaInfo.ptContent, STRSAFE_MAX_CCH, &cchSize );
				cchSize += (cchSep+6);
				ptBuffer = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
				ZeroMemory( ptBuffer, cchSize * sizeof(TCHAR) );

				StringCchPrintf( ptBuffer, cchSize, TEXT("%s%s\r\n"), stIaInfo.atSep, stIaInfo.ptContent );
				pcOutput = SjisEncodeAlloc( ptBuffer );
				cbSize = strlen( pcOutput );

				WriteFile( hFile , pcOutput, cbSize, &wrote, NULL );	//	���e��������

				SetEndOfFile( hFile );
				CloseHandle( hFile );
			}

			FREE(stIaInfo.ptContent);
		}

		//	�������C���ŊJ���Ă����烍�[�h�E�Ă������c���[���Œǉ����Ă��������ˁH
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	�A�C�e���ǉ��̖ʓ|����_�C�����O�[
	@param[in]	hDlg		�_�C�����O�n���h��
	@param[in]	message		�E�C���h�E���b�Z�[�W�̎��ʔԍ�
	@param[in]	wParam		�ǉ��̏��P
	@param[in]	lParam		�ǉ��̏��Q
	@retval 0	���b�Z�[�W�͏������Ă��Ȃ�
	@retval no0	�Ȃ񂩏������ꂽ
*/
INT_PTR CALLBACK AaItemAddDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static LPITEMADDINFO	pstIaInfo;
	static LPTSTR	ptBuffer;
	UINT_PTR	cchSize;
	TCHAR	atName[MAX_PATH];
	INT		id;
	RECT	rect;

	switch( message )
	{
		case WM_INITDIALOG:
			pstIaInfo = (LPITEMADDINFO)(lParam);
			GetClientRect( hDlg, &rect );
			CreateWindowEx( 0, WC_BUTTON, TEXT("���̕�"),         WS_CHILD | WS_VISIBLE, 0, 0, 75, 23, hDlg, (HMENU)IDB_MAID_NOWPAGE, GetModuleHandle(NULL), NULL );
			CreateWindowEx( 0, WC_BUTTON, TEXT("�N���b�v�{�[�h"), WS_CHILD | WS_VISIBLE, 75, 0, 120, 23, hDlg, (HMENU)IDB_MAID_CLIPBOARD, GetModuleHandle(NULL), NULL ); 
			CreateWindowEx( 0, WC_EDIT,   TEXT(""),               WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 195, 0, rect.right-195-50, 23, hDlg, (HMENU)IDE_MAID_ITEMNAME, GetModuleHandle(NULL), NULL );
			CreateWindowEx( 0, WC_BUTTON, TEXT("�ǉ�"),           WS_CHILD | WS_VISIBLE, rect.right-50, 0, 50, 23, hDlg, (HMENU)IDB_MAID_ADDGO, GetModuleHandle(NULL), NULL );
			CreateWindowEx( 0, WC_EDIT,   TEXT(""),               WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY, 0, 23, rect.right, rect.bottom-23, hDlg, (HMENU)IDE_MAID_CONTENTS, GetModuleHandle(NULL), NULL );
			//	�Ȃ�ł킴�킴����������̂��v���o���Ȃ��B

			if( pstIaInfo->bType )
			{
				SetDlgItemText( hDlg, IDE_MAID_ITEMNAME, TEXT("���̂�AST�łȂ��Ǝg�p�ł��Ȃ��̂ł�") );
				EnableWindow( GetDlgItem(hDlg,IDE_MAID_ITEMNAME), FALSE );
				StringCchCopy( pstIaInfo->atSep, MAX_PATH, TEXT("[SPLIT]\r\n") );
			}

			//	�Ƃ肠�����N���b�v�{�[�h�̒��g���Ƃ�
			ptBuffer = DocClipboardDataGet( NULL );
			if( !(ptBuffer) ){	DocPageGetAlloc( D_UNI , (LPVOID *)(&ptBuffer) );	}
			//	�g���Ȃ��V�����m�Ȃ�A���̕ł̓��e�������Ă��ĕ\��
			SetDlgItemText( hDlg, IDE_MAID_CONTENTS, ptBuffer );
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			id = LOWORD(wParam);
			switch( id )
			{
				case IDCANCEL:
					FREE(ptBuffer);
					EndDialog(hDlg, 0 );
					return (INT_PTR)TRUE;

				case IDB_MAID_ADDGO:
					if( ptBuffer )
					{
						StringCchLength( ptBuffer, STRSAFE_MAX_CCH, &cchSize );
						cchSize += 2;
						pstIaInfo->ptContent = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
						StringCchCopy( pstIaInfo->ptContent, cchSize, ptBuffer );

						if( !(pstIaInfo->bType) )
						{
							GetDlgItemText( hDlg, IDE_MAID_ITEMNAME, atName, MAX_PATH );
							StringCchPrintf( pstIaInfo->atSep, MAX_PATH, TEXT("[AA][%s]\r\n"), atName );
						}
					}
					FREE(ptBuffer);
					EndDialog(hDlg, 1 );
					return (INT_PTR)TRUE;

				case IDB_MAID_CLIPBOARD:
					FREE(ptBuffer);
					ptBuffer = DocClipboardDataGet( NULL );
					SetDlgItemText( hDlg, IDE_MAID_CONTENTS, ptBuffer );
					return (INT_PTR)TRUE;

				case IDB_MAID_NOWPAGE:
					FREE(ptBuffer);
					DocPageGetAlloc( D_UNI , (LPVOID *)(&ptBuffer) );
					SetDlgItemText( hDlg, IDE_MAID_CONTENTS, ptBuffer );
					return (INT_PTR)TRUE;

				default:	break;
			}
			break;

		case WM_SIZE:
			GetClientRect( hDlg, &rect );
			MoveWindow( GetDlgItem(hDlg,IDE_MAID_ITEMNAME), 195, 0, rect.right-195-50, 23, TRUE );
			MoveWindow( GetDlgItem(hDlg,IDB_MAID_ADDGO),    rect.right-50, 0, 50, 23, TRUE );
			MoveWindow( GetDlgItem(hDlg,IDE_MAID_CONTENTS), 0, 23, rect.right, rect.bottom-23, TRUE );
			break;
	}

	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------
	
	
#endif

#endif	//	MAA_IADD_PLUS

