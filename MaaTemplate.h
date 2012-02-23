/*! @file
	@brief 複数行テンプレで使う定数や函数です
	このファイルは MaaTemplate.h です。
	@author	SikigamiHNQ
	@date	2011/00/00
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


#pragma once


#include "resource.h"
//-------------------------------------------------------------------------------------------------

#define MAA_VIRTUAL_TREE

#define WMP_PROF_ACT	(WM_APP+100)	

#define TREE_WIDTH	170	//!<	MAAのツリービューの標準幅
#define LSSCL_WIDTH	15	//!<	MAAのAA一覧のスクロールバーの幅

//	開けてる
#define  ACT_ALLTREE	0
#define  ACT_FAVLIST	1
#define  ACT_SUBITEM	2	//	２以降に割当

typedef LRESULT (CALLBACK *BUFFERBACK)(UINT, UINT, UINT, LPCVOID);
//-------------------------------------------------------------------------------------------------


// このコード モジュールに含まれる関数の宣言
UINT		Maa_OnMouseWheel( HWND, INT, INT, INT, UINT );	//!<	
VOID		Maa_OnContextMenu( HWND, HWND, UINT, UINT );	//!<	
VOID		Maa_OnChar( HWND, TCHAR, INT );
VOID		Maa_OnSize( HWND , UINT, INT, INT );		//!<	
VOID		Maa_OnCommand( HWND , INT, HWND, UINT );		//!<	WM_COMMAND の処理

#ifdef _ORRVW
INT_PTR	CALLBACK OptionDlgProc( HWND, UINT, WPARAM, LPARAM );
#endif

VOID		StatusBarMsgSet( UINT, LPTSTR );	//!<	

LRESULT		TabBarNotify( HWND, LPNMHDR );		//!<	
VOID		TabBarResize( HWND, LPRECT );

HRESULT		TreeInitialise( HWND, HINSTANCE, LPRECT );	//!<	
HRESULT		TreeResize( HWND , LPRECT );		//!<	
HRESULT		TreeConstruct( HWND, LPCTSTR, BOOLEAN );	//!<	
LRESULT		TreeNotify( HWND, LPNMTREEVIEW );	//!<	
LPTSTR		TreeBaseNameGet( VOID );			//!<	
UINT		TreeFavIsUnderCursor( HWND, HWND, INT, INT, INT, UINT );	//!<	
INT			TreeFavWhichOpen( VOID );			//!<	
LPARAM		TreeItemInfoGet( HTREEITEM, LPTSTR, size_t );	//!<	
INT			TreeSelItemProc( HWND, HTREEITEM, UINT );	//!<	
UINT		TreeNodePathGet( HTREEITEM, LPTSTR );	//!<	

HRESULT		TreeMaaFileFind( HWND );

LRESULT		CALLBACK FavListFolderNameBack( UINT, UINT, UINT, LPCVOID );	//!<	
HRESULT		FavListSelected( HWND, UINT );		//!<	
HRESULT		FavContsRedrawRequest( HWND );		//!<	

HRESULT		TabMultipleDeleteAll( HWND );
HRESULT		TabMultipleStore( HWND );
INT			TabMultipleTopMemory( INT );

HRESULT		TabMultipleCtrlFromFind( HWND );


HRESULT		AaItemsInitialise( HWND, HINSTANCE, LPRECT );		//!<	
VOID		AaItemsResize( HWND, LPRECT );						//!<	
VOID		AaItemsDrawItem( HWND, CONST DRAWITEMSTRUCT * );	//!<	
//VOID		AaItemsMeasureItem( HWND, LPMEASUREITEMSTRUCT );	//!<	
HRESULT		AaItemsDoShow( HWND, LPTSTR, UINT );		//!<	
HRESULT		AaItemsFavUpload( LPSTR, UINT );	//!<	
UINT		AaItemsIsUnderCursor( HWND, HWND, INT );	//!<	
VOID		AaTitleClear( VOID );
INT			AaTitleAddString( UINT, LPSTR );
VOID		AaTitleSelect( HWND, UINT );

VOID		Aai_OnKey( HWND, UINT, BOOL, INT, UINT );
VOID		Aai_OnVScroll( HWND , HWND, UINT, INT );	//!<	

DWORD		AacAssembleFile( HWND, LPTSTR );	//!<	
HRESULT		AacMatrixClear( VOID  );			//!<	
DWORD		AacAssembleSql( HWND, LPCTSTR );	//!<	

#ifdef OPEN_PROFILE
HRESULT		OpenProfileLoad( HWND, INT );	//!<	
HRESULT		OpenProfileLogging( HWND, LPCTSTR );	//!<	
#endif

#ifndef _ORRVW
HRESULT		AacItemAdding( HWND, LPTSTR );
#endif

HRESULT		SqlDatabaseOpenClose( BYTE, LPCTSTR );	//!<	

HRESULT		SqlFavFolderEnum( BUFFERBACK );	//!<	
UINT		SqlFavCount( LPCTSTR, PUINT );	//!<	
HRESULT		SqlFavArtEnum( LPCTSTR, BUFFERBACK );	//!<	
HRESULT		SqlFavUpload( LPTSTR, DWORD, LPSTR, UINT );	//!<	
HRESULT		SqlFavDelete( LPTSTR, DWORD );	//!<	
HRESULT		SqlFavFolderDelete( LPTSTR );	//!<	

INT			TreeProfileMake( HWND, LPTSTR );
INT			TreeProfileOpen( HWND );
INT			TreeProfileRebuild( HWND );
HRESULT		TreeLoadDirCheck( HWND, HWND );

HTREEITEM	MaaSearchTreeItem( INT );
HTREEITEM	MaaSelectIDfile( HWND, INT );

INT			MaaSearchTreeID( HTREEITEM );

//HRESULT		SqlTreeOpenClose( BYTE, LPCTSTR );
HRESULT		SqlTransactionOnOff( BYTE );

HRESULT		SqlTreeTableCreate( LPTSTR );
UINT		SqlTreeCount( UINT, PUINT );
HRESULT		SqlTreeProfUpdate( LPCTSTR, LPCTSTR );
HRESULT		SqlTreeProfSelect( LPTSTR, UINT, LPTSTR, UINT );

UINT		SqlTreeNodeInsert( UINT, UINT, UINT, LPTSTR );
HRESULT		SqlTreeNodeDelete( UINT );
//HRESULT		SqlTreeNodeEnum( UINT, BUFFERBACK );
UINT		SqlTreeNodePickUpID( UINT, PUINT, PUINT, LPTSTR, UINT );
//UINT		SqlTreeNodeRootSearch( LPTSTR );
UINT		SqlChildNodePickUpID( UINT, UINT, PUINT, LPTSTR );

UINT		SqlTreeFileSearch( LPTSTR, UINT );
UINT		SqlTreeFileGetOnParent( LPTSTR, UINT );

HRESULT		SqlTreeCacheOpenClose( UINT );
HRESULT		SqlTreeCacheDelID( INT );
UINT		SqlTreeCacheInsert( UINT, UINT, LPTSTR );


UINT		SqlMultiTabInsert( LPTSTR, LPTSTR );
UINT		SqlMultiTabSelect( INT, LPTSTR, LPTSTR );
HRESULT		SqlMultiTabDelete( VOID );



