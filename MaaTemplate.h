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


LRESULT		CALLBACK FavListFolderNameBack( UINT, UINT, UINT, LPCVOID );	//!<	
HRESULT		FavListSelected( HWND, UINT );		//!<	
HRESULT		FavContsRedrawRequest( HWND );		//!<	

HRESULT		TabMultipleDeleteAll( HWND );
HRESULT		TabMultipleStore( HWND );
INT			TabMultipleTopMemory( INT );

HRESULT		AaItemsInitialise( HWND, HINSTANCE, LPRECT );		//!<	
VOID		AaItemsResize( HWND, LPRECT );						//!<	
VOID		AaItemsDrawItem( HWND, CONST DRAWITEMSTRUCT * );	//!<	
//VOID		AaItemsMeasureItem( HWND, LPMEASUREITEMSTRUCT );	//!<	
HRESULT		AaItemsDoShow( HWND, LPTSTR, UINT );		//!<	
UINT		AaItemsIsUnderCursor( HWND, HWND, INT );	//!<	
VOID		AaTitleClear( VOID );
INT			AaTitleAddString( UINT, LPSTR );
VOID		AaTitleSelect( HWND, UINT );

VOID		Aai_OnVScroll( HWND , HWND, UINT, INT );	//!<	

DWORD		AacAssembleFile( HWND, LPTSTR );	//!<	
HRESULT		AacMatrixClear( VOID  );			//!<	
LPSTR		AacAsciiArtGet( DWORD );			//!<	
DWORD		AacAssembleSql( HWND, LPCTSTR );	//!<	

#ifndef _ORRVW
HRESULT		AacItemAdding( HWND, LPTSTR );
#endif

HRESULT		SqlDatabaseOpenClose( BYTE, LPCTSTR );	//!<	
#ifndef MAA_PROFILE
HRESULT		SqlFavTableCreate( LPVOID );		//!<	
#endif

HRESULT		SqlFavFolderEnum( BUFFERBACK );	//!<	
UINT		SqlFavCount( LPCTSTR, PUINT );	//!<	
HRESULT		SqlFavArtEnum( LPCTSTR, BUFFERBACK );	//!<	
HRESULT		SqlFavUpload( LPTSTR, DWORD, LPSTR, UINT );	//!<	
HRESULT		SqlFavDelete( LPTSTR, DWORD );	//!<	

#ifdef MAA_PROFILE

INT			TreeProfileMake( HWND, LPTSTR );
INT			TreeProfileOpen( HWND );
INT			TreeProfileRebuild( HWND );
HRESULT		TreeLoadDirCheck( HWND, HWND );
UINT		TreeIsExistLoad( LPTSTR );

#ifdef FIND_MAA_FILE
HTREEITEM	MaaSearchTreeItem( INT );
HTREEITEM	MaaSelectIDfile( HWND, INT );
#endif

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

#endif


