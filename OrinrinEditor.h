/*! @file
	@brief アプリ全体で使う定数や函数です
	このファイルは OrinrinEditor.h です。
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

#define STRICT

#include "resource.h"
//-------------------------------------------------------------------------------------------------

#include "SplitBar.h"
//-------------------------------------------------------------------------------------------------

//	MLTの区切り文字列
#define MLT_SEPARATERW	TEXT("[SPLIT]")
#define MLT_SEPARATERA	("[SPLIT]")
#define MLT_SPRT_CCH	7

//	ASTの区切り文字列
#define AST_SEPARATERW	TEXT("[AA]")
#define AST_SEPARATERA	("[AA]")
#define AST_SPRT_CCH	4

//	テンプレのアレ
#define TMPLE_BEGINW	TEXT("[ListName=")
#define TMPLE_ENDW		TEXT("[end]")

//	改行
#define CH_CRLFW	TEXT("\r\n")
#define CH_CRLFA	("\r\n")
#define CH_CRLF_CCH	2

//	EOFマーク
#define EOF_SIZE	5
CONST  TCHAR	gatEOF[] = TEXT("[EOF]");
#define EOF_WIDTH	39

//	改行バイト数
#define YY2_CRLF	6
#define STRB_CRLF	4

#define PAGE_BYTE_MAX	4096

#define MODIFY_MSG	TEXT("[変更]")
//-------------------------------------------------------------------------------------------------

//	したらば用色指定タグ
#define COLOUR_TAG_WHITE	TEXT("<jbbs fontcolor=\"#ffffff\">")
#define COLOUR_TAG_BLUE		TEXT("<jbbs fontcolor=\"#0000ff\">")
#define COLOUR_TAG_BLACK	TEXT("<jbbs fontcolor=\"#000000\">")
#define COLOUR_TAG_RED		TEXT("<jbbs fontcolor=\"#ff0000\">")
#define COLOUR_TAG_GREEN	TEXT("<jbbs fontcolor=\"#00ff00\">")

//-------------------------------------------------------------------------------------------------



#define CC_TAB	0x09
#define CC_CR	0x0D
#define CC_LF	0x0A

//-----------------------------------------------------------------------------------------------------------------------------------------

//アンドゥ用COMMANDO
#define DO_INSERT	1	//	文字入力・ペーストとか
#define DO_DELETE	2	//	文字削除・切り取りとか

//-----------------------------------------------------------------------------------------------------------------------------------------


#define LINE_HEIGHT	18

#define RULER_AREA	13

#define LINENUM_WID	37
#define LINENUM_COLOUR		0xFF8000

#define RUL_LNNUM_COLOURBK	0xC0C0C0
//-------------------------------------------------------------------------------------------------

//	スペースの幅
#define SPACE_HAN	5
#define SPACE_ZEN	11
//-------------------------------------------------------------------------------------------------

#define CLR_BLACK	0x000000
#define CLR_MAROON	0x000080
#define CLR_GREEN	0x008000
#define CLR_OLIVE	0x008080
#define CLR_NAVY	0x800000
#define CLR_PURPLE	0x800080
#define CLR_TEAL	0x808000
#define CLR_GRAY	0x808080
#define CLR_SILVER	0xC0C0C0
#define CLR_RED		0x0000FF
#define CLR_LIME	0x00FF00
#define CLR_YELLOW	0x00FFFF
#define CLR_BLUE	0xFF0000
#define CLR_FUCHSIA	0xFF00FF
#define CLR_AQUA	0xFFFF00
#define CLR_WHITE	0xFFFFFF

//-------------------------------------------------------------------------------------------------

//	描画文字のアレ
#define CT_NORMAL	0x0000	//	普通の文字列
#define CT_WARNING	0x0001	//	連続半角空白のような警告
#define CT_SPACE	0x0002	//	空白
#define CT_SELECT	0x0004	//	選択状態である
#define CT_CANTSJIS	0x0008	//	シフトJISに変換できない文字
#define CT_LYR_TRNC	0x0010	//	レイヤボックスで透過範囲
#define CT_FINDED	0x0020	//	検索ヒット文字列

#define CT_SELRTN	0x0100	//	行末改行も選択状態
#define CT_LASTSP	0x0200	//	行末が空白である
#define CT_RETURN	0x0400	//	改行が必要
#define CT_EOF		0x0800	//	末端である
#define CT_FINDRTN	0x1000	//	行末改行が検索ヒット
//-------------------------------------------------------------------------------------------------

#ifndef _ORRVW

#ifdef OPEN_HISTORY
//開いた履歴用
#define OPENHIST_MAX	10
typedef struct tagOPENHISTORY
{
	TCHAR	atFile[MAX_PATH];

} OPENHIST, *LPOPENHIST;
typedef vector<OPENHIST>::iterator	OPHIS_ITR;
#endif
//----------------

//	枠パーツデータ
#define PARTS_CCH	10
typedef struct tagFRAMEITEM
{
	TCHAR	atParts[PARTS_CCH];	//!<	パーツ文字列・９字まで
	INT		dDot;	//!<	

} FRAMEITEM, *LPFRAMEITEM;
//----------------
//	枠処理用
typedef struct tagFRAMEINFO
{
	TCHAR	atFrameName[MAX_STRING];	//!<	名前・未使用

	FRAMEITEM	stDaybreak;	//!<	左		│
	FRAMEITEM	stMorning;	//!<	左上	┌
	FRAMEITEM	stNoon;		//!<	上		─
	FRAMEITEM	stAfternoon;//!<	右上	┐
	FRAMEITEM	stNightfall;//!<	右		│
	FRAMEITEM	stTwilight;	//!<	右下	┘
	FRAMEITEM	stMidnight;	//!<	下		─
	FRAMEITEM	stDawn;		//!<	左下	└

	INT		dLeftOffset;	//!<	左壁の配置オフセット・０は左上の始点
	INT		dRightOffset;	//!<	右壁の配置オフセット・０は右上の始点

} FRAMEINFO, *LPFRAMEINFO;
//-----------------------------

//	トレスモード用Parameter保持
typedef struct tagTRACEPARAM
{
	POINT	stOffsetPt;	//!<	位置合わせ
	INT		dContrast;	//!<	コントラスト
	INT		dGamma;		//!<	ガンマ
	INT		dGrayMoph;	//!<	淡色
	INT		dZooming;	//!<	拡大縮小
	INT		dTurning;	//!<	回転

	UINT	bUpset;		//!<	
	UINT	bMirror;	//!<	

	COLORREF	dMoziColour;	//!<	文字色について

} TRACEPARAM, *LPTRACEPARAM;
//----------------


//!	操作ログ本体
typedef struct tagOPERATELOG
{
	UINT	dCommando;	//!<	操作タイプ
	UINT	ixSequence;	//!<	操作番号・ふりきったらどうしよ
	UINT	ixGroup;	//!<	操作グループ・１回の処理・１インデックス

	INT		rdXdot;		//!<	操作のあったドット・文字位置とどっちが良い？
//	INT		rdXmozi;	//!<	操作のあったドキュメント文字位置
	INT		rdYline;	//!<	操作のあったドキュメント行

	LPTSTR	ptText;		//!<	操作された文字列
	UINT	cchSize;	//!<	文字列の文字数

} OPERATELOG, *LPOPERATELOG;
typedef vector<OPERATELOG>::iterator	OPSQ_ITR;

typedef struct tagUNDOBUFF
{
	UINT_PTR	dNowSqn;	//!<	参章中の操作位置？
	UINT		dTopSqn;	//!<	最新の操作番号１インデックス・追加はインクリしてから
	UINT		dGrpSqn;	//!<	操作グループ・１インデックス

	vector<OPERATELOG>	vcOpeSqn;	//!<	操作ログ本体

} UNDOBUFF, *LPUNDOBUFF;


//	壱文字の情報・受け渡しにも使う
typedef struct tagLETTER
{
	TCHAR	cchMozi;	//!<	文字
	INT		rdWidth;	//!<	この文字の幅
	UINT	mzStyle;	//!<	文字のタイプ・空白とかそういうの
	CHAR	acSjis[10];	//!<	シフトJISコード、もしくは「&#dddd;」形式をいれる
	INT_PTR	mzByte;		//!<	SJISバイトサイズ

} LETTER, *LPLETTER;
typedef vector<LETTER>::iterator	LTR_ITR;

//	壱行の管理
typedef struct tagONELINE
{
	INT		iDotCnt;	//!<	ドット数
	INT		iByteSz;	//!<	バイト数
//	INT		dCaret;		//!<	キャレットの現在位置０インデックス
	UINT	dStyle;		//!<	この行の特殊状態
	BOOLEAN	bBadSpace;	//!<	警告付き空白があるか

	vector<LETTER>	vcLine;	//!<	この行の内容・改行は含まない

	//	レイヤボックス用
	INT		dFrtSpDot;	//!<	前の空白ドット
	INT		dFrtSpMozi;	//!<	前の空白文字数
//	INT		dOffset;	//!<	矩形選択したときのズレ

} ONELINE, *LPONELINE;
typedef vector<ONELINE>::iterator	LINE_ITR;

//	SPLITページ壱分
typedef struct tagONEPAGE
{
	TCHAR	atPageName[SUB_STRING];	//!<	

	INT		dByteSz;		//!<	バイト数

	//	選択状態について
	INT		dSelLineTop;	//!<	一番上の選択がある行
	INT		dSelLineBottom;	//!<	一番下の選択がある行

	vector<ONELINE>	vcPage;	//!<	行全体

	UNDOBUFF	stUndoLog;	//!<	操作履歴・アンドゥに使う

} ONEPAGE, *LPONEPAGE;
typedef vector<ONEPAGE>::iterator	PAGE_ITR;

//	一つのファイル保持
typedef struct tagONEFILE
{
	TCHAR	atFileName[MAX_PATH];	//!<	ファイル名
	//	頁数はヴェクタルをカウンツすればＯＫ？
	UINT	dModify;	//!<	変更したかどうか

#ifdef MULTI_FILE
	LPARAM	dUnique;	//!<	通し番号・１インデックス
	TCHAR	atDummyName[MAX_PATH];	//!<	ファイル名ないときの仮名称
#endif

	INT		dNowPage;	//!<	見てる頁

	vector<ONEPAGE>	vcCont;	//!<	ページを保持する

} ONEFILE, *LPONEFILE;
//-----------------------------

#ifdef MULTI_FILE
typedef list<ONEFILE>::iterator	FILES_ITR;
#endif

//	複数ファイル扱うなら、さらにコレを包含すればいい？

//	壱行・ブラシテンプレ用
typedef struct tagAATEMPLATE
{
	TCHAR	atCtgryName[SUB_STRING];	//!<	セットの名前

	vector<wstring>	vcItems;	//!<	テンプレ文字列本体

} AATEMPLATE, *LPAATEMPLATE;

//-------------------------------------------------------------------------------------------------

typedef UINT (CALLBACK* PAGELOAD)(LPTSTR, LPTSTR, INT);

#endif	//	NOT _ORRVW

//-------------------------------------------------------------------------------------------------


//	このコード モジュールに含まれる関数の宣言
INT_PTR		CALLBACK About( HWND, UINT, WPARAM, LPARAM );

INT_PTR		MessageBoxCheckBox( HWND, HINSTANCE, UINT );

VOID		WndTagSet( HWND, LONG_PTR );
LONG_PTR	WndTagGet( HWND );

HRESULT		InitWindowPos( UINT, UINT, LPRECT );
INT			InitParamValue( UINT, UINT, INT );
HRESULT		InitParamString( UINT, UINT, LPTSTR );

BOOLEAN		SelectDirectoryDlg( HWND, LPTSTR, UINT_PTR );

UINT		ViewMaaMaterialise( LPSTR, UINT, UINT );

LPTSTR		SjisDecodeAlloc( LPSTR );
LPSTR		SjisEntityExchange( LPCSTR );
BOOLEAN		HtmlEntityCheck( TCHAR, LPSTR, UINT_PTR );

BOOLEAN		FileExtensionCheck( LPTSTR, LPTSTR );

HWND		MaaTmpltInitialise( HINSTANCE, HWND, LPRECT );
VOID		MaaTabBarSizeGet( LPRECT  );	//!<	

HRESULT		AaItemsTipSizeChange( INT, UINT );


#ifndef _ORRVW

LPSTR		SjisEncodeAlloc( LPTSTR );

ATOM		InitWndwClass( HINSTANCE );
BOOL		InitInstance( HINSTANCE, INT, LPTSTR );
LRESULT		CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

HRESULT		StatusBarSetText( INT, LPCTSTR );
HRESULT		StatusBarSetTextInt( INT, INT );

HRESULT		MenuItemCheckOnOff( UINT, UINT );
HRESULT		NotifyBalloonExist( HWND, LPTSTR, LPTSTR, DWORD );

HRESULT		BrushModeToggle( VOID );

HRESULT		WindowFocusChange( INT, INT );

HRESULT		OptionDialogueOpen( VOID );

COLORREF	InitColourValue( UINT, UINT, COLORREF );
INT			InitTraceValue( UINT, LPTRACEPARAM );
HRESULT		InitLastOpen( UINT, LPTSTR );
INT			InitWindowTopMost( UINT, UINT, INT );
#ifndef MAA_PROFILE
HRESULT		InitMaaFldrOpen( UINT, LPTSTR );
INT			InitMultipleFile( UINT, UINT, LPTSTR, LPTSTR );
#endif


VOID		ToolBarCreate( HWND, HINSTANCE );
VOID		ToolBarDestroy( VOID );
HRESULT		ToolBarSizeGet( LPRECT );
HRESULT		ToolBarCheckOnOff( UINT, UINT );
HRESULT		ToolBarOnSize( HWND, UINT, INT, INT );
LRESULT		ToolBarOnNotify( HWND, INT, LPNMHDR );
LRESULT		ToolBarOnContextMenu( HWND, HWND, LONG, LONG );
VOID		ToolBarPseudoDropDown( HWND, INT );

UINT		AppClientAreaCalc( LPRECT );

HRESULT		AppTitleChange( LPTSTR );
HRESULT		AppTitleTrace( UINT );

LPTSTR		ExePathGet( VOID );

HRESULT		UniDialogueEntry( HINSTANCE, HWND );

HRESULT		FrameInitialise( LPTSTR, HINSTANCE );
HRESULT		FrameNameModifyPopUp( HMENU, UINT );
INT_PTR		FrameEditDialogue( HINSTANCE, HWND, UINT );

HWND		FrameInsBoxCreate( HINSTANCE, HWND );
HRESULT		FrameMoveFromView( HWND, UINT );


#ifdef CONTEXT_EDIT

HRESULT		CntxEditInitialise( LPTSTR, HINSTANCE );
HRESULT		CntxEditDlgOpen( HWND );
HMENU		CntxMenuGet( VOID );
HRESULT		CntxMenuCopySwap( VOID );


#endif

#ifdef MULTI_FILE
HRESULT		MultiFileTabFirst( LPTSTR );
HRESULT		MultiFileTabAppend( LPARAM, LPTSTR );
HRESULT		MultiFileTabSelect( LPARAM );
HRESULT		MultiFileTabSlide( INT );
HRESULT		MultiFileTabRename( LPARAM, LPTSTR );
HRESULT		MultiFileTabClose( VOID );
INT			InitMultiFileTabOpen( UINT, INT, LPTSTR );
#endif

VOID		OperationOnCommand( HWND, INT, HWND, UINT );

VOID		AaFontCreate( UINT );

HWND		ViewInitialise( HINSTANCE, HWND, LPRECT, LPTSTR );
HRESULT		ViewSizeMove( HWND, LPRECT );
HRESULT		ViewFocusSet( VOID );

BOOL		ViewShowCaret( VOID );			//!<	
VOID		ViewHideCaret( VOID );

HRESULT		ViewFrameInsert( INT );
HRESULT		ViewMaaItemsModeSet( UINT );

INT			ViewLetterWidthGet( TCHAR );
INT			ViewStringWidthGet( LPTSTR );
HRESULT		ViewNowPosStatus( VOID );

HRESULT		ViewRedrawSetLine( INT );
HRESULT		ViewRedrawSetRect( LPRECT );
HRESULT		ViewRedrawSetVartRuler( INT );
HRESULT		ViewRulerRedraw( INT, INT );
HRESULT		ViewEditReset( VOID );

COLORREF	ViewMoziColourGet( LPCOLORREF );
COLORREF	ViewBackColourGet( LPVOID );

HRESULT		ViewCaretCreate( HWND, COLORREF, COLORREF );	//!<	
HRESULT		ViewCaretDelete( VOID );		//!<	
BOOLEAN		ViewDrawCaret( INT, INT , BOOLEAN );	//!<	本当はドローじゃなくてポジションチェンジだけ
BOOLEAN		ViewPosResetCaret( INT, INT );	//!<	
HRESULT		ViewCaretReColour( COLORREF );	//!<	

HRESULT		ViewPositionTransform( PINT, PINT, BOOLEAN );
BOOLEAN		ViewIsPosOnFrame( INT, INT );	//!<	
INT			ViewAreaSizeGet( PINT );

HRESULT		ViewSelPositionSet( LPVOID );	//!<	
HRESULT		ViewSelMoveCheck( UINT );		//!<	
HRESULT		ViewSelRangeCheck( UINT );		//!<	
INT			ViewSelPageAll( INT );			//!<	
UINT		ViewSqSelModeToggle( LPVOID );	//!<	

INT			ViewInsertUniSpace( UINT );
INT			ViewInsertColourTag( UINT );
INT			ViewInsertTmpleString( LPTSTR );

HRESULT		ViewBrushStyleSetting( UINT, LPTSTR );

VOID		Evw_OnMouseMove( HWND, INT, INT, UINT );
VOID		Evw_OnLButtonDown( HWND, BOOL, INT, INT, UINT );
VOID		Evw_OnLButtonUp( HWND, INT, INT, UINT );
VOID		Evw_OnRButtonDown( HWND, BOOL, INT, INT, UINT );

VOID		Evw_OnKey( HWND, UINT, BOOL, INT, UINT );	//!<	
VOID		Evw_OnChar( HWND, TCHAR, INT );				//!<	
VOID		Evw_OnMouseWheel( HWND, INT, INT, INT, UINT );

VOID		Evw_OnImeComposition( HWND, WPARAM, LPARAM );

BOOLEAN		IsSelecting( PUINT );

HRESULT		OperationOnStatusBar( VOID );

HWND		PageListInitialise( HINSTANCE, HWND, LPRECT );
VOID		PageListResize( HWND, LPRECT );
HRESULT		PageListClear( VOID );
HRESULT		PageListInsert( INT );
HRESULT		PageListDelete( INT );
HRESULT		PageListViewChange( INT );
HRESULT		PageListInfoSet( INT, INT, INT );
HRESULT		PageListNameSet( INT, LPTSTR );
#ifdef MULTI_FILE
INT			PageListIsNamed( FILES_ITR );
#else
INT			PageListIsNamed( VOID );
#endif

HRESULT		PageListBuild( LPVOID );


HRESULT		TemplateItemLoad( LPTSTR, PAGELOAD );
UINT		TemplateGridFluctuate( HWND, INT );

HWND		LineTmpleInitialise( HINSTANCE, HWND, LPRECT );
VOID		LineTmpleResize( HWND, LPRECT );

VOID		DockingTabSizeGet( LPRECT );

HWND		BrushTmpleInitialise( HINSTANCE, HWND, LPRECT, HWND );
LPTSTR		BrushStringMake( INT, LPTSTR );
VOID		BrushTmpleResize( HWND, LPRECT );

INT			UserDefInitialise( HWND, UINT );
HRESULT		UserDefItemInsert( HWND, UINT );
HRESULT		UserDefMenuWrite( HMENU );
HRESULT		UserDefSetString( vector<ONELINE> *, LPTSTR, UINT );	//!<	

HRESULT		FrameNameModifyMenu( HWND );

VOID		PreviewInitialise( HINSTANCE, HWND );
HRESULT		PreviewVisibalise( INT );

INT			TraceInitialise( HWND, UINT );
HRESULT		TraceDialogueOpen( HINSTANCE, HWND );
HRESULT		TraceImgViewTglExt( VOID );
UINT		TraceImageAppear( HDC, INT, INT );
UINT		TraceMoziColourGet( LPCOLORREF );

HRESULT		ImageFileSaveDC( HDC, LPTSTR, INT );


VOID		LayerBoxInitialise( HINSTANCE, LPRECT );
HRESULT		LayerBoxAlphaSet( UINT );
HRESULT		LayerMoveFromView( HWND, UINT );
HWND		LayerBoxVisibalise( HINSTANCE, LPTSTR, UINT );
INT			LayerHeadSpaceCheck( vector<LETTER> *, PINT );	//!<	
HRESULT		LayerTransparentToggle( HWND, UINT );			//!<	
HRESULT		LayerContentsImportable( HWND, UINT, LPINT, LPINT, UINT );	//!<	
HRESULT		LayerBoxPositionChange( HWND, LONG, LONG );
HRESULT		LayerStringReplace( HWND, LPTSTR );



HRESULT		DocInitialise( LPVOID );

BOOLEAN		DocRangeIsError( INT, INT );

INT_PTR		DocPageCount( VOID );

HRESULT		DocOpenFromNull( HWND );
UINT		DocPageParamGet( PINT, PINT );
INT			DocPageMaxDotGet( INT, INT );
INT			DocPageByteCount( INT, PINT );
HRESULT		DocPageInfoRenew( INT, UINT );

HRESULT		DocPageNameSet( LPTSTR );

UINT		DocPageCreate( INT );
HRESULT		DocPageDelete( INT );
HRESULT		DocPageChange( INT );

HRESULT		DocModifyContent( UINT );

#ifdef MULTI_FILE
LPARAM		DocMultiFileCreate( LPTSTR );
#else
HRESULT		DocContentsObliterate( VOID );
#endif
INT			DocLineParamGet( INT, PINT, PINT );

UINT		DocBadSpaceCheck( INT );
BOOLEAN		DocBadSpaceIsExist( INT );

HRESULT		DocPageDivide( HWND, HINSTANCE, INT );

BOOLEAN		DocIsSjisTrance( TCHAR, LPSTR );
INT_PTR		DocLetterByteCheck( LPLETTER );

INT			DocInputLetter( INT, INT, TCHAR );
INT			DocInputBkSpace( PINT, PINT );
INT			DocInputDelete( INT, INT );
INT			DocInputFromClipboard( PINT, PINT, PINT );

INT			DocAdditionalLine( INT, BOOLEAN );

INT			DocStringAdd( PINT, PINT, LPTSTR, INT );
HRESULT		DocCrLfAdd( INT, INT, BOOLEAN );
INT			DocSquareAdd( PINT, PINT, LPTSTR, INT, LPPOINT * );
INT			DocStringErase( INT, INT, LPTSTR, INT );

INT			DocInsertLetter( PINT, INT, TCHAR );
INT			DocInsertString( PINT, PINT, PINT, LPTSTR, UINT, BOOLEAN );

INT			DocIterateDelete( LTR_ITR, INT );
HRESULT		DocLineCombine( INT );

BOOLEAN		DocLineErase( INT, BOOLEAN );


HRESULT		DocFrameInsert( INT, INT );
HRESULT		DocScreenFill( LPTSTR );

HRESULT		DocPageNumInsert( HINSTANCE, HWND );

INT			DocExClipSelect( UINT );
HRESULT		DocPageAllCopy( UINT );

INT			DocLetterShiftPos( INT, INT, INT, PINT, PBOOLEAN );
INT			DocLetterPosGetAdjust( PINT, INT, INT );

HRESULT		DocReturnSelStateToggle( INT, INT );
INT			DocRangeSelStateToggle( INT, INT, INT, INT );
INT			DocPageSelStateToggle( INT );
HRESULT		DocSelRangeSet( INT, INT );
HRESULT		DocSelRangeGet( PINT, PINT );
//BOOLEAN		DocIsSelecting( VOID );

LPTSTR		DocClipboardDataGet( PUINT );
HRESULT		DocClipboardDataSet( LPVOID, INT, UINT );

INT			DocLineDataGetAlloc( INT, INT, LPLETTER *, PINT, PUINT );
INT			DocPageTextAllGetAlloc( UINT, LPVOID * );
LPSTR		DocPageTextPreviewAlloc( INT, PINT );

HRESULT		DocThreadDropCopy( VOID );

INT			DocSelectedDelete( PINT, PINT, UINT );
INT			DocSelectedBrushFilling( LPTSTR, PINT, PINT );
INT			DocSelectTextGetAlloc( UINT, LPVOID *, LPPOINT * );

HRESULT		DocExtractExecute( HINSTANCE );

HRESULT		DocFileSave( HWND, UINT );
HRESULT		DocFileOpen( HWND );
#ifdef MULTI_FILE
INT			DocAllTextGetAlloc( INT, UINT, LPVOID *, FILES_ITR );
#else
INT			DocAllTextGetAlloc( INT, UINT, LPVOID * );
#endif
HRESULT		DocImageSave( HWND, UINT, HFONT );


UINT		DocStringSplitAST( LPTSTR, INT, PAGELOAD );

INT			DocLineStateCheckWithDot( INT, INT, PINT, PINT, PINT, PINT, PBOOLEAN );
HRESULT		DocLeftGuideline( LPVOID );
INT			DocSpaceShiftProc( UINT, PINT, INT );
LPTSTR		DocPaddingSpaceMake( INT );
LPTSTR		DocPaddingSpaceUni( INT, PINT, PINT, PINT );
LPTSTR		DocPaddingSpaceWithGap( INT, PINT, PINT );
LPTSTR		DocPaddingSpaceWithPeriod( INT, PINT, PINT, PINT, BOOLEAN );
HRESULT		DocLastSpaceErase( PINT, INT );
HRESULT		DocTopLetterInsert( TCHAR, PINT, INT );
HRESULT		DocLastLetterErase( PINT, INT );
HRESULT		DocTopSpaceErase( PINT, INT );
HRESULT		DocRightSlide( PINT, INT );

HRESULT		DocPositionShift( UINT, PINT, INT );

LPTSTR		DocLastSpDel( vector<LETTER> * );

INT			DocDiffAdjBaseSet( INT );
INT			DocDiffAdjExec( PINT, INT );

VOID		ZeroONELINE( LPONELINE );
LPTSTR		NextLine( LPTSTR );
INT			DocLineCount( LPTSTR, UINT );

UINT		DocRangeDeleteByMozi( INT, INT, INT, INT, PBOOLEAN );

INT			DocUndoExecute( PINT, PINT );
INT			DocRedoExecute( PINT, PINT );

LPARAM		DocFileInflate( LPTSTR );
INT			DocFileCloseCheck( HWND, UINT );
HRESULT		DocClipLetter( TCHAR );
VOID		DocBackupDirectoryInit( LPTSTR );
HRESULT		DocFileBackup( HWND );
#ifdef MULTI_FILE
HRESULT		DocMultiFileDeleteAll( VOID );
LPARAM		DocMultiFileDelete( HWND, LPARAM );
HRESULT		DocMultiFileSelect( LPARAM );
HRESULT		DocMultiFileModify( UINT );
HRESULT		DocMultiFileStore( LPTSTR );
INT			DocMultiFileFetch( INT, LPTSTR, LPTSTR );
#endif

HRESULT		SqnInitialise( LPUNDOBUFF );
HRESULT		SqnFreeAll( LPUNDOBUFF );
HRESULT		SqnSetting( VOID );
UINT		SqnAppendLetter( LPUNDOBUFF, UINT, TCHAR, INT, INT, UINT );
UINT		SqnAppendString( LPUNDOBUFF, UINT, LPTSTR, INT, INT, UINT );
UINT		SqnAppendSquare( LPUNDOBUFF, UINT, LPTSTR, LPPOINT, INT, UINT );

HRESULT		UnicodeRadixExchange( LPVOID );

INT			MoziInitialise( LPTSTR, HINSTANCE );
HWND		MoziScripterCreate( HINSTANCE, HWND );
HRESULT		MoziMoveFromView( HWND, UINT );

#ifdef FIND_STRINGS
HRESULT		FindDialogueOpen( HINSTANCE, HWND );
HRESULT		FindHighlightOff( VOID );
#endif

#endif	//	NOT _ORRVW

