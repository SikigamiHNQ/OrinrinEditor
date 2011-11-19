//	SplitBar.h

#pragma once

//-------------------------------------------------------------------------------------------------

#define SPLITBAR_CLASS		TEXT("CSplitBar")	//	スプリットバークラス名
#define SPLITBAR_WIDTH		4	//	スプリットバーの幅
#define SPLITBAR_LEFTLIMIT	120	//	スプリットバーの左位置リミット
//-------------------------------------------------------------------------------------------------

ATOM	SplitBarClass( HINSTANCE );
HWND	SplitBarCreate( HINSTANCE, HWND, INT, INT, INT );
VOID	SplitBarPosGet( HWND, LPRECT );
VOID	SplitBarResize( HWND, LPRECT );
