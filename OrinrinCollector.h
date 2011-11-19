#pragma once

#include "resource.h"
//-------------------------------------------------------------------------------------------------

// このコード モジュールに含まれる関数の宣言
ATOM	InitWndwClass( HINSTANCE );
BOOL	InitInstance( HINSTANCE, INT );
LRESULT	CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

BOOLEAN	Cls_OnCreate( HWND, LPCREATESTRUCT );		//!<	本体の WM_CREATE の処理・固定Editとかつくる
VOID	Cls_OnCommand( HWND , INT, HWND, UINT );	//!<	本体の WM_COMMAND の処理
VOID	Cls_OnPaint( HWND );						//!<	本体の WM_PAINT の処理・枠線描画とか
VOID	Cls_OnDestroy( HWND );						//!<	本体の WM_DESTROY の処理・BRUSHとかのオブジェクトの破壊を忘れないように
HBRUSH	Cls_OnCtlColor( HWND , HDC, HWND, INT );	//!<	
VOID	Cls_OnHotKey( HWND, INT, UINT, UINT );		//!<	
VOID	Cls_OnDrawClipboard( HWND );				//!<	
VOID	Cls_OnChangeCBChain( HWND, HWND, HWND );	//!<	

VOID		WndTagSet( HWND, LONG_PTR );			//!<	
LONG_PTR	WndTagGet( HWND );						//!<	

BOOLEAN	SelectFileDlg( HWND, LPTSTR, UINT_PTR );	//!<	

HRESULT	ToolTipSetting( HWND, UINT, LPTSTR );		//!<	

HRESULT	TasktrayIconAdd( HWND );					//!<	
VOID	TaskTrayIconEvent( HWND, UINT, UINT );		//!<	
HRESULT	TaskTrayIconCaptionChange( HWND );			//!<	
HRESULT	TaskTrayIconBalloon( HWND, LPTSTR, LPTSTR, DWORD );	//!<	

INT		InitParamValue( UINT, UINT, INT );			//!<	
HRESULT	InitClipStealOpen( UINT, UINT, LPTSTR );	//!<	
HRESULT	InitSettingSave( HWND );					//!<	

HRESULT	FileListViewInit( HWND );				//!<	
INT		FileListViewAdd( HWND, LPTSTR );		//!<	
HRESULT	FileListViewGet( HWND, INT, LPTSTR );	//!<	
INT		FileListViewDelete( HWND  );			//!<	

BOOLEAN	FileTypeCheck( LPTSTR );			//!<	

HMENU	CreateFileSelMenu( HWND, UINT );	//!<	

HRESULT	ClipStealDoing( HWND  );		//!<	
LPTSTR	ClipboardDataGet( LPVOID  );	//!<	

LPTSTR	SjisDecodeAlloc( LPSTR );		//!<	
LPSTR	SjisEncodeAlloc( LPTSTR );		//!<	
LPSTR	SjisEntityExchange( LPCSTR );	//!<	
TCHAR	UniRefCheck( LPSTR );			//!<	
