/*! @file
	@brief アプリ全体で使う定数や函数です
	このファイルは OrinrinEditor.h です。
	@author	SikigamiHNQ
	@date	2011/00/00
*/

/*
Orinrin Editor : AsciiArt Story Editor for Japanese Only
Copyright (C) 2011 - 2014 Orinrin/SikigamiHNQ

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

//!	MLTの区切り文字列
#define MLT_SEPARATERW	TEXT("[SPLIT]")
#define MLT_SEPARATERA	("[SPLIT]")
#define MLT_SPRT_CCH	7

//!	ASTの区切り文字列
#define AST_SEPARATERW	TEXT("[AA]")
#define AST_SEPARATERA	("[AA]")
#define AST_SPRT_CCH	4

//!	テンプレのアレ
#define TMPLE_BEGINW	TEXT("[ListName=")
#define TMPLE_ENDW		TEXT("[end]")

//!	改行
#define CH_CRLFW	TEXT("\r\n")
#define CH_CRLFA	("\r\n")
#define CH_CRLF_CCH	2

//!	EOFマーク
#define EOF_SIZE	5
CONST  TCHAR	gatEOF[] = TEXT("[EOF]");
#define EOF_WIDTH	39


#define YY2_CRLF	6	//!<	ＹＹカキコの改行バイト数
#define STRB_CRLF	4	//!<	したらば板の改行バイト数

#define PAGE_BYTE_MAX	4096	//!<	１頁の最大バイト数・ただし板による

#define MODIFY_MSG	TEXT("[変更]")
//-------------------------------------------------------------------------------------------------

//	したらば用色指定タグ
#define COLOUR_TAG_WHITE	TEXT("<jbbs fontcolor=\"#ffffff\">")	//!<	したらば色指定：白
#define COLOUR_TAG_BLUE		TEXT("<jbbs fontcolor=\"#0000ff\">")	//!<	したらば色指定：蒼
#define COLOUR_TAG_BLACK	TEXT("<jbbs fontcolor=\"#000000\">")	//!<	したらば色指定：黒
#define COLOUR_TAG_RED		TEXT("<jbbs fontcolor=\"#ff0000\">")	//!<	したらば色指定：紅
#define COLOUR_TAG_GREEN	TEXT("<jbbs fontcolor=\"#00ff00\">")	//!<	したらば色指定：翠

//-------------------------------------------------------------------------------------------------



#define CC_TAB	0x09
#define CC_CR	0x0D
#define CC_LF	0x0A

//-----------------------------------------------------------------------------------------------------------------------------------------

//アンドゥ用COMMANDO
#define DO_INSERT	1	//!<	アンドゥ用COMMANDO　文字入力・ペーストとか
#define DO_DELETE	2	//!<	アンドゥ用COMMANDO　文字削除・切り取りとか

//-----------------------------------------------------------------------------------------------------------------------------------------


#define LINE_HEIGHT	18	//!<	ＡＡの壱行の高さドット

#define RULER_AREA	13	//!<	編集窓のルーラーエリア高さ

#define LINENUM_WID	37	//!<	編集窓の行番号表示エリアの幅
#define LINENUM_COLOUR		0xFF8000	//!<	編集窓の行番号表示エリアの色

#define RUL_LNNUM_COLOURBK	0xC0C0C0
//-------------------------------------------------------------------------------------------------

//	スペースの幅
#define SPACE_HAN	5	//!<	半角スペースの幅ドット
#define SPACE_ZEN	11	//!<	全角スペースの幅ドット
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
#define CT_NORMAL	0x0000	//!<	普通の文字列
#define CT_WARNING	0x0001	//!<	連続半角空白のような警告
#define CT_SPACE	0x0002	//!<	空白
#define CT_SELECT	0x0004	//!<	選択状態である
#define CT_CANTSJIS	0x0008	//!<	シフトJISに変換できない文字
#define CT_LYR_TRNC	0x0010	//!<	レイヤボックスで透過範囲
#define CT_FINDED	0x0020	//!<	検索ヒット文字列

#define CT_SELRTN	0x0100	//!<	行末改行も選択状態
#define CT_LASTSP	0x0200	//!<	行末が空白である
#define CT_RETURN	0x0400	//!<	改行が必要
#define CT_EOF		0x0800	//!<	末端である
#define CT_FINDRTN	0x1000	//!<	行末改行が検索ヒット
//-------------------------------------------------------------------------------------------------



#define OPENHIST_MAX	12
//!	開いた履歴用・プロファイルにも使う
typedef struct tagOPENHISTORY
{
	TCHAR	atFile[MAX_PATH];	//!<	ファイルパス
	DWORD	dMenuNumber;		//!<	メニュー番号の割当

} OPENHIST, *LPOPENHIST;
typedef list<OPENHIST>::iterator	OPHIS_ITR;
//----------------

//!	壱文字の情報・受け渡しにも使う
typedef struct tagLETTER
{
	TCHAR	cchMozi;	//!<	文字
	INT		rdWidth;	//!<	この文字の幅
	UINT	mzStyle;	//!<	文字のタイプ・空白とかそういうの
	CHAR	acSjis[10];	//!<	シフトJISコード、もしくは「&#dddd;」形式をいれる
	INT_PTR	mzByte;		//!<	SJISバイトサイズ

} LETTER, *LPLETTER;
typedef vector<LETTER>::iterator	LETR_ITR;
//-----------------------------

#ifndef _ORRVW

#define PARTS_CCH	130

//!	枠パーツデータ	20120105	複数行に向けて調整
typedef struct tagFRAMEITEM
{
	TCHAR	atParts[PARTS_CCH];	//!<	パーツ文字列・９字まで
	INT		dDot;	//!<	横幅ドット数
	INT		iLine;	//!<	使用行数
	INT		iNowLn;	//!<	使用行番号・配置時に使う

} FRAMEITEM, *LPFRAMEITEM;
//----------------
//!	枠処理用
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

	UINT	dRestPadd;		//!<	あまりを埋めるかどうか・

} FRAMEINFO, *LPFRAMEINFO;
//-----------------------------

//!	トレスモード用Parameter保持
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
//-----------------------------

//!	アンドゥバッファ
typedef struct tagUNDOBUFF
{
	UINT_PTR	dNowSqn;	//!<	参章中の操作位置？
	UINT		dTopSqn;	//!<	最新の操作番号１インデックス・追加はインクリしてから
	UINT		dGrpSqn;	//!<	操作グループ・１インデックス

	vector<OPERATELOG>	vcOpeSqn;	//!<	操作ログ本体

} UNDOBUFF, *LPUNDOBUFF;
//-----------------------------

//壱文字の情報構造体宣言を↑に移動

//!	壱行の管理
typedef struct tagONELINE
{
	INT		iDotCnt;		//!<	ドット数
	INT		iByteSz;		//!<	バイト数
	UINT	dStyle;			//!<	この行の特殊状態
	BOOLEAN	bBadSpace;		//!<	警告付き空白があるか

	vector<LETTER>	vcLine;	//!<	この行の内容・改行は含まない

	//	レイヤボックス用
	INT		dFrtSpDot;		//!<	前の空白ドット
	INT		dFrtSpMozi;		//!<	前の空白文字数

} ONELINE, *LPONELINE;
typedef list<ONELINE>::iterator		LINE_ITR;
//-----------------------------

//!	SPLITページ壱分
typedef struct tagONEPAGE
{
	TCHAR	atPageName[SUB_STRING];	//!<	

	INT		dByteSz;		//!<	バイト数

	//	選択状態について
	INT		dSelLineTop;	//!<	一番上の選択がある行
	INT		dSelLineBottom;	//!<	一番下の選択がある行
	UNDOBUFF	stUndoLog;	//!<	操作履歴・アンドゥに使う

	list<ONELINE>	ltPage;	//!<	行全体

	LPTSTR	ptRawData;		//!<	頁の生データ
	INT		iLineCnt;		//!<	行数・生データ用
	INT		iMoziCnt;		//!<	文字数・生データ用

} ONEPAGE, *LPONEPAGE;
typedef vector<ONEPAGE>::iterator	PAGE_ITR;
//-----------------------------

//!	一つのファイル保持
typedef struct tagONEFILE
{
	TCHAR	atFileName[MAX_PATH];	//!<	ファイル名
	//	頁数はヴェクタルをカウンツすればＯＫ？
	UINT	dModify;		//!<	変更したかどうか

	LPARAM	dUnique;		//!<	通し番号・１インデックス
	TCHAR	atDummyName[MAX_PATH];	//!<	ファイル名ないときの仮名称

	INT		dNowPage;		//!<	見てる頁

	POINT	stCaret;		//!<	Caret位置・ドット、行数

	vector<ONEPAGE>	vcCont;	//!<	ページを保持する

} ONEFILE, *LPONEFILE;

typedef list<ONEFILE>::iterator	FILES_ITR;
//-----------------------------

//	複数ファイル扱うなら、さらにコレを包含すればいい？

//!	壱行・ブラシテンプレ用・カテゴリ名保持のアレ
typedef struct tagAATEMPLATE
{
	TCHAR	atCtgryName[SUB_STRING];	//!<	セットの名前

	vector<wstring>	vcItems;	//!<	テンプレ文字列本体

} AATEMPLATE, *LPAATEMPLATE;
typedef vector<AATEMPLATE>::iterator	TEMPL_ITR;	
//-----------------------------

//!	リバーの位置確定用
typedef struct tagREBARLAYOUTINFO
{
	UINT	wID;
	UINT	cx;
	UINT	fStyle;

} REBARLAYOUTINFO, *LPREBARLAYOUTINFO;
//-----------------------------


//!	頁情報確保用
typedef struct tagPAGEINFOS
{
	UINT	dMasqus;	//!<	必要な情報のアレ

	INT_PTR	iLines;	//!<	行数
	INT_PTR	iBytes;	//!<	使用バイト数
	INT_PTR	iMozis;	//!<	使用文字数

	TCHAR	atPageName[SUB_STRING];	//!<	

} PAGEINFOS, *LPPAGEINFOS;
#define PI_LINES	0x01
#define PI_BYTES	0x02
#define PI_MOZIS	0x04
#define PI_NAME		0x08
#define PI_RECALC	0x80000000
//-----------------------------

//-------------------------------------------------------------------------------------------------

typedef UINT (CALLBACK* PAGELOAD)(LPTSTR, LPCTSTR, INT);	//!<	頁ロード用コールバック関数の型宣言

#endif	//	NOT _ORRVW


//	MaaCatalogue.cppから移動
//!	MLTの保持
typedef struct tagAAMATRIX
{
	CHAR	acAstName[MAX_STRING];	//!<	ASTの場合、頁名称を持っておく

	UINT	ixNum;	//!<	通し番号０インデックス
	DWORD	cbItem;	//!<	AAの文字バイト数・

	LPSTR	pcItem;	//!<	読み込んだAAを保持しておくポインタ・SJIS形式のままでいいか？

	INT		iByteSize;	//!<	
	//	サムネ用
	INT		iMaxDot;	//!<	横幅最大ドット数
	INT		iLines;		//!<	使用行数

	SIZE	stSize;		//!<	ピクセルサイズ
	HBITMAP	hThumbBmp;	//!<	サムネイル用ビットマップハンドル

} AAMATRIX, *LPAAMATRIX;
typedef vector<AAMATRIX>::iterator	MAAM_ITR;	
//-----------------------------


#ifdef USE_HOVERTIP

//!	HoverTip用の表示内容確保・内容は増やすかも
//typedef struct tagHOVERTIPINFO
//{
//	LPTSTR	ptInfo;	//	文字列内容を示すポインタ
//
//} HOVERTIPINFO, *LPHOVERTIPINFO;

//!	HoverTip用コールバック函数
typedef LPTSTR (CALLBACK* HOVERTIPDISP)( LPVOID );	//!<	HoverTipコールバック関数の型宣言


HRESULT	HoverTipInitialise( HINSTANCE, HWND );	//!<	
HRESULT	HoverTipResist( HWND  );	//!<	
HRESULT	HoverTipSizeChange( INT );	//!<	
LRESULT	HoverTipOnMouseHover( HWND, WPARAM, LPARAM, HOVERTIPDISP );	//!<	
LRESULT	HoverTipOnMouseLeave( HWND );	//!<	


#endif
//-------------------------------------------------------------------------------------------------


//	このコード モジュールに含まれる関数の宣言
INT_PTR		CALLBACK About( HWND, UINT, WPARAM, LPARAM );	//!<	

INT_PTR		MessageBoxCheckBox( HWND, HINSTANCE, UINT );	//!<	

VOID		WndTagSet( HWND, LONG_PTR );	//!<	
LONG_PTR	WndTagGet( HWND );	//!<	

#ifdef SPMOZI_ENCODE
UINT		IsSpMozi( TCHAR );
#endif

HRESULT		InitWindowPos( UINT, UINT, LPRECT );	//!<	
INT			InitParamValue( UINT, UINT, INT );		//!<	
HRESULT		InitParamString( UINT, UINT, LPTSTR );	//!<	

HRESULT		OpenProfileInitialise( HWND );	//!<	
HRESULT		InitProfHistory( UINT, UINT, LPTSTR );	//!<	
  #ifdef _ORRVW
HRESULT		OpenProfMenuModify( HWND );	//!<	
  #endif

BOOLEAN		SelectDirectoryDlg( HWND, LPTSTR, UINT_PTR );	//!<	

UINT		ViewMaaMaterialise( HWND, LPSTR, UINT, UINT );	//!<	
INT			ViewStringWidthGet( LPCTSTR );	//!<	
INT			ViewLetterWidthGet( TCHAR );	//!<	

UINT		ViewMaaItemsModeGet( PUINT );	//!<	

LPTSTR		SjisDecodeAlloc( LPSTR );	//!<	
LPSTR		SjisEntityExchange( LPCSTR );	//!<	
BOOLEAN		HtmlEntityCheckA( TCHAR, LPSTR , UINT_PTR );	//!<	
BOOLEAN		HtmlEntityCheckW( TCHAR, LPTSTR, UINT_PTR );	//!<	

BOOLEAN		DocIsSjisTrance( TCHAR, LPSTR );	//!<	
INT_PTR		DocLetterByteCheck( LPLETTER );	//!<	
INT_PTR		DocLetterDataCheck( LPLETTER, TCHAR );

BOOLEAN		FileExtensionCheck( LPTSTR, LPTSTR );	//!<	

HWND		MaaTmpltInitialise( HINSTANCE, HWND, LPRECT );	//!<	
HRESULT		MaaTmpltPositionReset( HWND );	//!<	
VOID		MaaTabBarSizeGet( LPRECT  );	//!<	

HRESULT		AaItemsTipSizeChange( INT, UINT );	//!<	

HRESULT		ViewingFontGet( LPLOGFONT );	//!<	

LPTSTR		FindStringProc( LPTSTR, LPTSTR, LPINT );	//!<	


#ifndef _ORRVW

VOID		AacBackupDirectoryInit( LPTSTR );	//!<	

BOOLEAN		MaaViewToggle( UINT );				//!<	

UINT		UnicodeUseToggle( LPVOID  );		//!<	

LPSTR		SjisEncodeAlloc( LPCTSTR  );		//!<	

ATOM		InitWndwClass( HINSTANCE  );		//!<	
BOOL		InitInstance( HINSTANCE , INT, LPTSTR );	//!<	
LRESULT		CALLBACK WndProc( HWND , UINT, WPARAM, LPARAM );	//!<	

HRESULT		MainStatusBarSetText( INT, LPCTSTR );	//!<	
HRESULT		MainSttBarSetByteCount( UINT  );	//!<	

HRESULT		WindowPositionReset( HWND );		//!<	

HRESULT		MenuItemCheckOnOff( UINT, UINT );	//!<	
HRESULT		NotifyBalloonExist( LPTSTR, LPTSTR, DWORD );	//!<	

HRESULT		BrushModeToggle( VOID );		//!<	

HRESULT		WindowFocusChange( INT, INT );	//!<	

HRESULT		OptionDialogueOpen( VOID  );	//!<	

COLORREF	InitColourValue( UINT, UINT, COLORREF );	//!<	
INT			InitTraceValue( UINT, LPTRACEPARAM );	//!<	
//HRESULT	InitLastOpen( UINT, LPTSTR );			//	
INT			InitWindowTopMost( UINT, UINT, INT );	//!<	
HRESULT		InitToolBarLayout( UINT, INT, LPREBARLAYOUTINFO );	//!<	

UINT		DocHugeFileTreatment( UINT );

#ifdef ACCELERATOR_EDIT
LPACCEL		AccelKeyTableGetAlloc( LPINT  );	//!<	
LPACCEL		AccelKeyTableLoadAlloc( LPINT );	//!<	
HRESULT		AccelKeyDlgOpen( HWND );			//!<	
HACCEL		AccelKeyHandleGet( HINSTANCE  );	//!<	

HACCEL		AccelKeyTableCreate( LPACCEL, INT );	//!<	
HRESULT		AccelKeyMenuRewrite( HWND, LPACCEL, CONST INT );	//!<	
#endif

HRESULT		OpenHistoryInitialise( HWND );			//!<	
HRESULT		OpenHistoryLogging( HWND , LPTSTR );	//!<	
HRESULT		OpenHistoryLoad( HWND, INT );			//!<	

VOID		ToolBarCreate( HWND, HINSTANCE );		//!<	
HRESULT		ToolBarInfoChange( LPACCEL, INT );		//!<	
VOID		ToolBarDestroy( VOID  );				//!<	
HRESULT		ToolBarSizeGet( LPRECT );				//!<	
HRESULT		ToolBarCheckOnOff( UINT, UINT );		//!<	
HRESULT		ToolBarOnSize( HWND, UINT, INT, INT );	//!<	
LRESULT		ToolBarOnNotify( HWND, INT, LPNMHDR );	//!<	
LRESULT		ToolBarOnContextMenu( HWND , HWND, LONG, LONG );	//!<	
VOID		ToolBarPseudoDropDown( HWND , INT );	//!<	
UINT		ToolBarBandInfoGet( LPVOID );			//!<	
HRESULT		ToolBarBandReset( HWND );				//!<	

UINT		AppClientAreaCalc( LPRECT );	//!<	

HRESULT		AppTitleChange( LPTSTR );	//!<	
HRESULT		AppTitleTrace( UINT );		//!<	

LPTSTR		ExePathGet( VOID  );		//!<	

HRESULT		UniDlgInitialise( HWND , UINT );	//!<	
HRESULT		UniDialogueEntry( HINSTANCE, HWND );		//!<	

HRESULT		FrameInitialise( LPTSTR, HINSTANCE );		//!<	枠設定のINIファイル名確保・アプリ起動後すぐ呼ばれる
HRESULT		FrameNameModifyPopUp( HMENU, UINT );		//!<	
INT_PTR		FrameEditDialogue( HINSTANCE, HWND, UINT );	//!<	
HRESULT		FrameNameLoad( UINT, LPTSTR, UINT_PTR );	//!<	指定された枠の名前を返す

HWND		FrameInsBoxCreate( HINSTANCE, HWND );	//!<	
HRESULT		FrameMoveFromView( HWND, UINT );		//!<	

HRESULT		CntxEditInitialise( LPTSTR, HINSTANCE );	//!<	
HRESULT		CntxEditDlgOpen( HWND );	//!<	
HMENU		CntxMenuGet( VOID );		//!<	

HRESULT		AccelKeyTextBuild( LPTSTR, UINT_PTR, DWORD, LPACCEL, INT );

HRESULT		MultiFileTabFirst( LPTSTR );	//!<	
HRESULT		MultiFileTabAppend( LPARAM, LPTSTR );	//!<	
HRESULT		MultiFileTabSelect( LPARAM );	//!<	
HRESULT		MultiFileTabSlide( INT );		//!<	
HRESULT		MultiFileTabRename( LPARAM, LPTSTR );	//!<	
HRESULT		MultiFileTabClose( INT );		//!<	
INT			MultiFileTabSearch( LPARAM );	//!<	
INT			InitMultiFileTabOpen( UINT, INT, LPTSTR );	//!<	

VOID		OperationOnCommand( HWND, INT, HWND, UINT );	//!<	

VOID		AaFontCreate( UINT );	//!<	

#ifdef TODAY_HINT_STYLE
VOID		TodayHintPopup( HWND, HINSTANCE, LPTSTR );
#endif

HWND		ViewInitialise( HINSTANCE, HWND, LPRECT, LPTSTR );	//!<	
HRESULT		ViewSizeMove( HWND, LPRECT );	//!<	
HRESULT		ViewFocusSet( VOID );			//!<	

BOOL		ViewShowCaret( VOID );			//!<	
VOID		ViewHideCaret( VOID );			//!<	
INT			ViewCaretPosGet( PINT, PINT );	//!<	

HRESULT		ViewFrameInsert( INT  );		//!<	
HRESULT		ViewMaaItemsModeSet( UINT, UINT );	//!<	

HRESULT		ViewNowPosStatus( VOID );		//!<	

HRESULT		ViewRedrawSetLine( INT );		//!<	
HRESULT		ViewRedrawSetRect( LPRECT );	//!<	
HRESULT		ViewRedrawSetVartRuler( INT );	//!<	
HRESULT		ViewRulerRedraw( INT, INT );	//!<	
HRESULT		ViewEditReset( VOID );			//!<	

COLORREF	ViewMoziColourGet( LPCOLORREF );	//!<	
COLORREF	ViewBackColourGet( LPVOID );	//!<	

HRESULT		ViewCaretCreate( HWND, COLORREF, COLORREF );	//!<	
HRESULT		ViewCaretDelete( VOID );		//!<	
BOOLEAN		ViewDrawCaret( INT, INT , BOOLEAN );	//!<	本当はドローじゃなくてポジションチェンジだけ
BOOLEAN		ViewPosResetCaret( INT, INT );	//!<	
HRESULT		ViewCaretReColour( COLORREF );	//!<	

HRESULT		ViewPositionTransform( PINT, PINT, BOOLEAN );	//!<	
BOOLEAN		ViewIsPosOnFrame( INT, INT );	//!<	
INT			ViewAreaSizeGet( PINT );

HRESULT		ViewSelPositionSet( LPVOID );	//!<	
HRESULT		ViewSelMoveCheck( UINT );		//!<	
UINT		ViewSelRangeCheck( UINT );		//!<	
UINT		ViewSelBackCheck( INT );		//!<	
INT			ViewSelPageAll( INT );			//!<	
UINT		ViewSqSelModeToggle( UINT, LPVOID );	//!<	
HRESULT		ViewSelAreaSelect( LPVOID );	//!<	

INT			ViewInsertUniSpace( UINT );	//!<	
INT			ViewInsertColourTag( UINT );	//!<	
INT			ViewInsertTmpleString( LPCTSTR );	//!<	

HRESULT		ViewBrushStyleSetting( UINT, LPTSTR );	//!<	

VOID		Evw_OnMouseMove( HWND, INT, INT, UINT );	//!<	
VOID		Evw_OnLButtonDown( HWND, BOOL, INT, INT, UINT );	//!<	
VOID		Evw_OnLButtonUp( HWND, INT, INT, UINT );	//!<	
VOID		Evw_OnRButtonDown( HWND, BOOL, INT, INT, UINT );	//!<	

VOID		Evw_OnKey( HWND, UINT, BOOL, INT, UINT );	//!<	
VOID		Evw_OnChar( HWND, TCHAR, INT );				//!<	
VOID		Evw_OnMouseWheel( HWND, INT, INT, INT, UINT );	//!<	

VOID		Evw_OnImeComposition( HWND, WPARAM, LPARAM );	//!<	

BOOLEAN		IsSelecting( PUINT );	//!<	

HRESULT		OperationOnStatusBar( VOID );	//!<	

HWND		PageListInitialise( HINSTANCE, HWND, LPRECT );	//!<	
VOID		PageListResize( HWND , LPRECT );	//!<	
HRESULT		PageListClear( VOID );	//!<	
HRESULT		PageListInsert( INT );	//!<	
HRESULT		PageListDelete( INT );	//!<	
HRESULT		PageListViewChange( INT , INT );	//!<	
HRESULT		PageListInfoSet( INT, INT, INT );	//!<	
HRESULT		PageListNameSet( INT , LPTSTR );	//!<	
HRESULT		PageListNameRewrite( LPTSTR );	//!<	
INT			PageListIsNamed( FILES_ITR );	//!<	
HRESULT		PageListPositionReset( HWND );	//!<	

HRESULT		PageListViewRewrite( INT  );	//!<	
HRESULT		PageListBuild( LPVOID );	//!<	


HRESULT		TemplateItemLoad( LPTSTR, PAGELOAD );	//!<	
UINT		TemplateGridFluctuate( HWND , INT );	//!<	

HWND		LineTmpleInitialise( HINSTANCE , HWND, LPRECT );	//!<	
VOID		LineTmpleResize( HWND, LPRECT );	//!<	
HRESULT		LineTmplePositionReset( HWND  );		//!<	

VOID		DockingTabSizeGet( LPRECT );	//!<	
HRESULT		DockingTabContextMenu( HWND, HWND, LONG, LONG );	//!<	
HWND		DockingTabGet( VOID );			//!<	
HRESULT		DockingTmplViewToggle( UINT );	//!<	函数の場所違うから注意

HWND		BrushTmpleInitialise( HINSTANCE, HWND, LPRECT, HWND );	//!<	
LPTSTR		BrushStringMake( INT , LPTSTR );	//!<	
VOID		BrushTmpleResize( HWND, LPRECT );	//!<	
HRESULT		BrushTmplePositionReset( HWND );	//!<	

INT			UserDefInitialise( HWND, UINT );	//!<	
HRESULT		UserDefItemInsert( HWND, UINT );	//!<	
HRESULT		UserDefMenuWrite( HMENU, UINT );	//!<	ユーザ定義メニューの中身をくっつける
HRESULT		UserDefItemNameget( UINT, LPTSTR, UINT_PTR );	//!<	アイテムの名前を引っ張る
HRESULT		UserDefSetString( vector<ONELINE> *, LPCTSTR, UINT );	//!<	保持構造体に文字列を記録する

HRESULT		FrameNameModifyMenu( HWND );	//!<	

VOID		PreviewInitialise( HINSTANCE, HWND );	//!<	
HRESULT		PreviewVisibalise( INT, BOOLEAN );	//!<	

INT			TraceInitialise( HWND, UINT );	//!<	
HRESULT		TraceDialogueOpen( HINSTANCE, HWND );	//!<	
HRESULT		TraceImgViewTglExt( VOID );	//!<	
UINT		TraceImageAppear( HDC, INT, INT );	//!<	
UINT		TraceMoziColourGet( LPCOLORREF );	//!<	

HRESULT		ImageFileSaveDC( HDC, LPTSTR, INT );	//!<	


VOID		LayerBoxInitialise( HINSTANCE, LPRECT );	//!<	
HRESULT		LayerBoxAlphaSet( UINT );	//!<	
HRESULT		LayerMoveFromView( HWND, UINT );	//!<	
HWND		LayerBoxVisibalise( HINSTANCE, LPCTSTR, UINT );	//!<	
INT			LayerHeadSpaceCheck( vector<LETTER> *, PINT );	//!<	
HRESULT		LayerTransparentToggle( HWND, UINT );			//!<	
HRESULT		LayerContentsImportable( HWND, UINT, LPINT, LPINT, UINT );	//!<	
HRESULT		LayerBoxPositionChange( HWND , LONG, LONG );	//!<	
HRESULT		LayerStringReplace( HWND , LPTSTR );	//!<	



HRESULT		DocInitialise( UINT );	//!<	

BOOLEAN		DocRangeIsError( FILES_ITR , INT, INT );	//!<	

UINT_PTR	DocNowFilePageCount( VOID );	//!<	
UINT_PTR	DocNowFilePageLineCount( VOID );	//!<	

UINT		DocRawDataParamGet( LPCTSTR, PINT, PINT );	//!<	

VOID		DocCaretPosMemory( UINT , LPPOINT );	//!<	

HRESULT		DocOpenFromNull( HWND );	//!<	

UINT		DocPageParamGet( PINT, PINT );	//!<	
UINT		DocPageByteCount( FILES_ITR , INT, PINT, PINT );	//!<	
HRESULT		DocPageInfoRenew( INT, UINT );	//!<	
INT			DocPageMaxDotGet( INT, INT );	//!<	
HRESULT		DocPageNameSet( LPTSTR );	//!<	

INT			DocPageCreate( INT );	//!<	
HRESULT		DocPageDelete( INT, INT );	//!<	
HRESULT		DocPageChange( INT );	//!<	

UINT		DocDelayPageLoad( FILES_ITR , INT );	//!<	ディレイ頁のロード

HRESULT		DocModifyContent( UINT );	//!<	

LPARAM		DocMultiFileCreate( LPTSTR );	//!<	
HRESULT		DocActivateEmptyCreate( LPTSTR );	//!<	

INT			DocLineParamGet( INT , PINT, PINT );	//!<	

UINT		DocBadSpaceCheck( INT );	//!<	
BOOLEAN		DocBadSpaceIsExist( INT );	//!<	

HRESULT		DocPageDivide( HWND, HINSTANCE, INT );	//!<	

INT			DocInputLetter( INT, INT, TCHAR );	//!<	
INT			DocInputBkSpace( PINT, PINT );	//!<	
INT			DocInputDelete( INT , INT );	//!<	
INT			DocInputFromClipboard( PINT, PINT, PINT, UINT );	//!<	

INT			DocAdditionalLine( INT, PBOOLEAN );	//!<	

INT			DocStringAdd( PINT, PINT, LPCTSTR, INT );	//!<	
HRESULT		DocCrLfAdd( INT, INT, BOOLEAN );	//!<	
INT			DocSquareAdd( PINT, PINT, LPCTSTR, INT, LPPOINT * );	//!<	
INT			DocStringErase( INT, INT, LPTSTR, INT );	//!<	

INT			DocInsertLetter( PINT, INT, TCHAR );	//!<	
INT			DocInsertString( PINT, PINT, PINT, LPCTSTR, UINT, BOOLEAN );	//!<	

INT			DocIterateDelete( LETR_ITR, INT );	//!<	
HRESULT		DocLineCombine( INT );	//!<	

HRESULT		DocLineErase( INT, PBOOLEAN );	//!<	


HRESULT		DocFrameInsert( INT, INT );	//!<	
HRESULT		DocScreenFill( LPTSTR );	//!<	

HRESULT		DocPageNumInsert( HINSTANCE, HWND );	//!<	

INT			DocExClipSelect( UINT );	//!<	
HRESULT		DocPageAllCopy( UINT );	//!<	

INT			DocLetterShiftPos( INT, INT, INT, PINT, PBOOLEAN );	//!<	
INT			DocLetterPosGetAdjust( PINT, INT, INT );	//!<	

HRESULT		DocReturnSelStateToggle( INT, INT );	//!<	
INT			DocRangeSelStateToggle( INT, INT, INT, INT );	//!<	
UINT		DocLetterSelStateGet( INT, INT );	//!<	
INT			DocPageSelStateToggle( INT );	//!<	
HRESULT		DocSelRangeSet( INT, INT );	//!<	
HRESULT		DocSelRangeGet( PINT, PINT );	//!<	
HRESULT		DocSelRangeReset( PINT, PINT );	//!<	
VOID		DocSelByteSet( INT );	//!<	
//BOOLEAN		DocIsSelecting( VOID );

HRESULT		DocSelText2PageName( VOID );	//!<	

LPTSTR		DocClipboardDataGet( PUINT );	//!<	
HRESULT		DocClipboardDataSet( LPVOID, INT, UINT );	//!<	

INT			DocLineDataGetAlloc( INT, INT, LPLETTER *, PINT, PUINT );	//!<	
LPSTR		DocPageTextPreviewAlloc( INT, PINT );	//!<	

HRESULT		DocThreadDropCopy( VOID );	//!<	

INT			DocPageTextGetAlloc( FILES_ITR, INT, UINT, LPVOID *, BOOLEAN );	//!<	
INT			DocPageGetAlloc( UINT, LPVOID * );	//!<	

INT			DocLineTextGetAlloc( FILES_ITR, INT, UINT, UINT, LPVOID * );	//!<	

INT			DocSelectedDelete( PINT, PINT, UINT, BOOLEAN );	//!<	
INT			DocSelectedBrushFilling( LPTSTR, PINT, PINT );	//!<	
INT			DocSelectTextGetAlloc( UINT, LPVOID *, LPPOINT * );	//!<	

HRESULT		DocExtractExecute( HINSTANCE  );	//!<	

LPARAM		DocOpendFileCheck( LPTSTR );		//!<	
HRESULT		DocFileSave( HWND, UINT );			//!<	
HRESULT		DocFileOpen( HWND );				//!<	
HRESULT		DocDoOpenFile( HWND, LPTSTR );		//!<	
HRESULT		DocImageSave( HWND, UINT, HFONT );	//!<	

HRESULT		DocHtmlExport( HWND );

UINT		DocStringSplitMLT( LPTSTR, INT, PAGELOAD );	//!<	
UINT		DocStringSplitAST( LPTSTR, INT, PAGELOAD );	//!<	

UINT		DocImportSplitASD( LPSTR, INT, PAGELOAD );	//!<	

INT			DocLineStateCheckWithDot( INT, INT, PINT, PINT, PINT, PINT, PBOOLEAN );	//!<	
HRESULT		DocRightGuideline( LPVOID );			//!<	
INT			DocSpaceShiftProc( UINT, PINT, INT );	//!<	
LPTSTR		DocPaddingSpaceMake( INT  );			//!<	
LPTSTR		DocPaddingSpaceUni( INT, PINT, PINT, PINT );	//!<	
LPTSTR		DocPaddingSpaceWithGap( INT, PINT, PINT );	//!<	
LPTSTR		DocPaddingSpaceWithPeriod( INT, PINT, PINT, PINT, BOOLEAN );	//!<	
HRESULT		DocLastSpaceErase( PINT , INT );		//!<	
HRESULT		DocTopLetterInsert( TCHAR, PINT, INT );	//!<	
HRESULT		DocLastLetterErase( PINT, INT );		//!<	
HRESULT		DocTopSpaceErase( PINT, INT );			//!<	
HRESULT		DocRightSlide( PINT , INT );			//!<	

HRESULT		DocPositionShift( UINT, PINT, INT );	//!<	
#ifdef DOT_SPLIT_MODE
HRESULT		DocCentreWidthShift( UINT vk, PINT, INT );	//!<	
#endif
HRESULT		DocHeadHalfSpaceExchange( HWND );	//!<	

LPTSTR		DocLastSpDel( vector<LETTER> * );	//!<	

INT			DocDiffAdjBaseSet( INT );		//!<	
INT			DocDiffAdjExec( PINT, INT );	//!<	

VOID		ZeroONELINE( LPONELINE );		//!<	
INT			DocStringInfoCount( LPCTSTR, UINT_PTR, PINT, PINT );	//!<	

BOOLEAN		NowPageInfoGet( UINT, LPPAGEINFOS );	//!<	

BOOLEAN		PageIsDelayed( FILES_ITR, UINT );	//!<	

UINT		DocRangeDeleteByMozi( INT, INT, INT, INT, PBOOLEAN );	//!<	

INT			DocUndoExecute( PINT, PINT );		//!<	
INT			DocRedoExecute( PINT, PINT );		//!<	

LPARAM		DocFileInflate( LPTSTR );			//!<	
INT			DocFileCloseCheck( HWND, UINT );	//!<	
HRESULT		DocClipLetter( TCHAR  );			//!<	
VOID		DocBackupDirectoryInit( LPTSTR );	//!<	
HRESULT		DocFileBackup( HWND );				//!<	

HRESULT		DocMultiFileCloseAll( VOID );		//!<	
LPARAM		DocMultiFileClose( HWND, LPARAM );	//!<	
HRESULT		DocMultiFileSelect( LPARAM );		//!<	
HRESULT		DocMultiFileModify( UINT  );		//!<	
HRESULT		DocMultiFileStore( LPTSTR );		//!<	
INT			DocMultiFileFetch( INT, LPTSTR, LPTSTR );	//!<	
LPTSTR		DocMultiFileNameGet( INT  );		//!<	

HRESULT		DocInverseInit( UINT  );	//!<	
HRESULT		DocInverseTransform( UINT, UINT, PINT, INT );	//!<	

HRESULT		SqnInitialise( LPUNDOBUFF );	//!<	
HRESULT		SqnFreeAll( LPUNDOBUFF );		//!<	
HRESULT		SqnSetting( VOID  );			//!<	
UINT		SqnAppendLetter( LPUNDOBUFF, UINT, TCHAR, INT, INT, UINT );	//!<	
UINT		SqnAppendString( LPUNDOBUFF, UINT, LPCTSTR, INT, INT, UINT );	//!<	
UINT		SqnAppendSquare( LPUNDOBUFF, UINT, LPCTSTR, LPPOINT, INT, UINT );	//!<	

HRESULT		UnicodeRadixExchange( LPVOID  );		//!<	

INT			MoziInitialise( LPTSTR, HINSTANCE );	//!<	
HWND		MoziScripterCreate( HINSTANCE, HWND );	//!<	
HRESULT		MoziMoveFromView( HWND , UINT );		//!<	

INT			VertInitialise( LPTSTR, HINSTANCE );	//!<	
HWND		VertScripterCreate( HINSTANCE, HWND );	//!<	
HRESULT		VertMoveFromView( HWND , UINT );		//!<	

#ifdef FIND_STRINGS
HRESULT		FindDialogueOpen( HINSTANCE, HWND );		//!<	
HRESULT		FindDirectly( HINSTANCE, HWND, INT );
//INT			FindStringJump( UINT, PINT, PINT, PINT );	//!<	
#ifdef SEARCH_HIGHLIGHT
HRESULT		FindNowPageReSearch( VOID );				//!<	
HRESULT		FindDelayPageReSearch( INT );				//!<	
HRESULT		FindHighlightOff( VOID );					//!<	
#endif
#endif

#endif	//	NOT _ORRVW

LPCTSTR		NextLineW( LPCTSTR );	//!<	
LPTSTR		NextLineW( LPTSTR );	//!<	

LPSTR		NextLineA( LPSTR  );	//!<	


//Viewerも有り
HRESULT	DraughtInitialise( HINSTANCE, HWND );	//!<	
HWND	DraughtWindowCreate( HINSTANCE, HWND, UINT );	//!<	

UINT	DraughtItemAddFromSelect( HWND , UINT );	//!<	
UINT	DraughtItemAdding( HWND, LPSTR );		//!<	

UINT	DraughtAaImageing( HWND, LPAAMATRIX );	//!<	


INT		TextViewSizeGet( LPCTSTR, PINT );	//!<	

INT_PTR	AacItemCount( UINT );				//!<	
HBITMAP	AacArtImageGet( HWND, INT, LPSIZE, LPSIZE );	//!<	

LPSTR	AacAsciiArtGet( DWORD );			//!<	
INT		AacArtSizeGet( DWORD, PINT, PINT );

