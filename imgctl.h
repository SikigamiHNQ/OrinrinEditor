/*
 ************************************************
 *	[imgctl.h] Version 1.22
 *		For <imgctl.dll> version 1.22.
 *		<imgctl.dll> is free library.
 ************************************************
 */
/*
 *	[著作権について]
 *		imgctl.dll は ルーチェ に著作権があります。
 *		<imgctl.dll> copyright (C) ruche.
 *
 *	[JPEGについて]
 *		imgctl.dll の JPEG読み書き処理には The Independent JPEG Group (IJG) の
 *		作成した、jpeg.lib が利用されています。
 *
 *	[PNGについて]
 *		imgctl.dll のPNG読み書き処理には libpng.lib と zlib.lib が利用されています。
 *		また、PNGの読み込みは、宮坂賢さん作の ifpng.spi のソースが参考にされています。
 */
/*
 *	[更新履歴]
 *		* Version 1.00B
 *			: 動作テスト掲示板で仮公開
 *		* Version 1.00
 *			: 正式に公開
 *			: HeadDIB関数追加
 *			: ColorDIB関数追加
 *			: DIBto16Bit関数追加
 *			: DIBto24Bit関数仕様変更
 *			: DIBtoRLE関数仕様変更
 *			: RLEtoDIB関数仕様変更
 *			: SizeDIB関数削除
 *		* Version 1.01B
 *			: ToDIB関数追加
 *			: PaletteDIB関数追加
 *			: PixelDIB関数追加
 *			: CutDIB関数追加
 *			: DIBtoPNGex関数追加
 *			: HeadDIB関数バグ修正
 *		* Version 1.01B2
 *			: ImgctlBeta関数追加
 *			: DCtoDIB関数追加
 *		* Version 1.01
 *			: DIBtoPNGex関数内部仕様変更
 *		* Version 1.02B
 *			: DIBto8Bit関数追加
 *			: DIBto24Bit関数バグ修正
 *			: DIBto16Bit関数バグ修正
 *		* Version 1.02B2
 *			: GrayDIB関数追加
 *			: ToneDIB関数追加
 *			: ReplaceDIB関数追加
 *		* Version 1.02
 *			: PNGtoDIB関数不備修正
 *		* Version 1.03B
 *			: GetImageType関数拡張
 *			: DIBto8Bit関数拡張
 *			: BMPtoDIB関数拡張
 *		* Version 1.03B2
 *			: JPGtoDIB関数拡張
 *		* Version 1.03B3
 *			: GetImageType関数修正
 *		* Version 1.03
 *			: GetImageType関数拡張
 *		* Version 1.04B
 *			: GetImageType関数修正
 *		* Version 1.04
 *			: DIBStretchDIBits2関数追加
 *		* Version 1.05B
 *			: PasteDIB関数追加
 *			: CutDIB関数不備修正
 *			: DIBDIBits関数不備修正
 *			: DIBStretchDIBits関数不備修正
 *		* Version 1.05
 *			: InfoPNG関数追加
 *		* Version 1.06B
 *			: TurnDIB関数追加
 *			: CutDIB関数微修正
 *			: DIBStretchDIBits2関数微修正
 *			: 異なるVersion間の互換(v1.05より)
 *		* Version 1.06
 *			: RepaintDIB関数追加
 *		* Version 1.07B
 *			: GetImageType関数微修正
 *			: DIBDIBits関数修正
 *			: DIBStretchDIBits関数修正
 *		* Version 1.07B2
 *			: ImgctlError関数追加
 *			: ImgctlErrorClear関数追加
 *			: PixelDIB関数バグ修正
 *			: GetDIB関数修正
 *		* Version 1.07B3
 *			: ImgctlError関数用エラーコード拡張
 *			: HDIB型を引数に取る関数のNULLチェック処理追加
 *			: ファイル名を引数に取る関数のNULLチェック処理追加
 *			: 各種関数の引数における不正NULLのチェック処理追加
 *		* Version 1.07
 *			: JPEGライブラリ及びPNGライブラリの再構築
 *		* Version 1.08B (v1.08と誤設定)
 *			: PointerOf関数追加
 *			: バージョン文字列定数IMGCTL_VERSION_STRING追加
 *		* Version 1.08B2
 *			: DataDIB関数追加
 *			: DIBtoDC関数追加(DIBDIBits関数と同機能)
 *			: DIBtoDCex関数追加(DIBStretchDIBits関数と同機能)
 *			: DIBtoDCex2関数追加(DIBStretchDIBits2関数と同機能)
 *			: MapDIB関数の第1引数をHDIBC型からHDIB型に変更
 *			: メモリの確保方法をmallocからGlobalAllocに変更
 *		* Version 1.08
 *			: DataDIB関数仕様変更
 *			: メモリの確保方法をGlobalAllocからmallocに戻した
 *		* Version 1.09
 *			: DIBtoDC関数大幅修正(BitBltと完全に同機能)
 *			: DIBtoDCex関数大幅修正(StretchBltと完全に同機能)
 *			: 上2つの関数を内部で利用する関数も連鎖的に仕様変更
 *			: ImgctlError関数用エラーコード拡張
 *			: DIBtoPNGex関数バグ修正
 *		* Version 1.10B
 *			: DIBtoDC関数修正(今度こそBitBltと完全に同機能)
 *			: DIBtoDCex関数修正(今度こそStretchBltと完全に同機能)
 *			: 上2つの関数を内部で利用する関数も連鎖的に仕様変更
 *			: PasteDIB関数バグ＆仕様修正及び拡張(座標指定はDIBtoDCと同じ仕様)
 *			: ImgctlError関数用エラーコード拡張
 *		* Version 1.10B2
 *			: GetImageType関数用定数修正
 *		* Version 1.10B3
 *			: DIBtoJPG関数拡張
 *			: JPGtoDIB関数拡張
 *			: DIBtoPNG関数拡張
 *			: DIBtoPNGex関数拡張
 *			: PNGtoDIB関数修正
 *			: InfoPNG関数修正
 *		* Version 1.10B4
 *			: DIBto16BitEx関数追加
 *			: GetImageType関数拡張
 *			: CreateDIB関数拡張
 *			: DIBto16Bit関数微修正
 *			: DIBto8Bit関数拡張
 *			: BMPtoDIB関数微修正
 *			: 16Bit/32Bitの色処理を修正
 *		* Version 1.10B5
 *			: DIBto16BitEx関数拡張
 *			: DIBto8Bit関数拡張
 *		* Version 1.10B6
 *			: DIBto16BitEx関数修正及び拡張
 *			: DIBto8Bit関数修正及び拡張
 *		* Version 1.10B7
 *			: DIBto16BitEx関数仕様変更及び修正及び拡張
 *			: DIBto8Bit関数仕様変更及び修正及び拡張
 *		* Version 1.10B8
 *			: DIBto16BitEx関数拡張
 *			: DIBto8Bit関数拡張
 *		* Version 1.10
 *			: DIBto16BitEx関数仕様変更
 *			: DIBto8Bit関数仕様変更
 *		* Version 1.11B
 *			: DeleteDIB関数微修正
 *			: libpng.lib及びzlib.libをバージョンアップ
 *		* Version 1.11
 *			: DIBtoJPG関数致命的バグ修正
 *		* Version 1.12B
 *			: ResizeDIB関数追加
 *			: InfoPNG関数バグ修正
 *		* Version 1.12B2
 *			: GammaDIB関数追加
 *			: ContrastDIB関数追加
 *			: ResizeDIB関数内部仕様変更
 *		* Version 1.12B3
 *			: TableDIB関数追加
 *			: ShadeDIB関数追加
 *			: GammaDIB関数仕様変更
 *			: ContrastDIB関数仕様変更
 *			: CreateDIB関数内部仕様変更及び拡張
 *			: ToneDIB関数内部仕様変更
 *			: ImgctlError関数用エラーコード拡張
 *		* Version 1.12B4
 *			: TurnDIBex関数追加
 *		* Version 1.12B4f
 *			: DataDIB関数修正(C/C++)
 *		* Version 1.12B5
 *			: PNGAtoDIB関数追加
 *			: PNGtoDIB関数内部仕様変更
 *			: ImgctlError関数用エラーコード拡張
 *		* Version 1.12B6
 *			: DIBtoPNGex関数修正
 *		* Version 1.12B7
 *			: DIBtoPNGex関数修正
 *			: 構造体メンバのアラインメントを指定
 *			: PNGOPT構造体に予約変数追加
 *		* Version 1.12B8
 *			: DIBtoJPG関数バグ修正
 *			: JPGtoDIB関数バグ修正
 *		* Version 1.12B9
 *			: GetImageType関数仕様変更
 *		* Version 1.12B10
 *			: GetImageType関数修正
 *		* Version 1.12
 *			: DIBtoBMP関数修正
 *		* Version 1.13B
 *			: JPGtoDIB関数拡張
 *		* Version 1.13B2
 *			: DIBtoJPG関数を試験的に内部仕様変更
 *		* Version 1.13B3
 *			: JPGtoDIB関数修正
 *		* Version 1.13B4
 *			: DIBtoGIF関数追加
 *			: DIBtoGIFex関数追加
 *			: GIFtoDIB関数追加
 *			: GIFtoDIBex関数追加
 *		* Version 1.13
 *			: DIBtoGIFAni関数追加
 *			: DIBtoGIFAniEx関数追加
 *		* Version 1.14
 *			: GIFtoDIB関数致命的バグ修正
 *			: GIFtoDIBex関数致命的バグ修正
 *		* Version 1.15
 *			: DIBto24Bit関数バグ修正
 *			: DIBto8Bit関数を試験的に内部仕様変更
 *		* Version 1.16B
 *			: libpng.lib及びzlib.libをバージョンアップ
 *		* Version 1.16B2
 *			: DIBtoPNG関数及びDIBtoPNGex関数致命的バグ修正
 *			: libpng.libをバージョンアップ
 *		* Version 1.16B3
 *			: BMPMtoDIB関数追加
 *			: JPGMtoDIB関数追加
 *			: PNGMtoDIB関数追加
 *			: PNGMAtoDIB関数追加
 *			: InfoPNGM関数追加
 *			: GIFMtoDIB関数追加
 *			: GIFMtoDIBex関数追加
 *			: ImgctlError関数用エラーコード拡張
 *		* Version 1.16B4
 *			: libpng.libをバージョンアップ(αチャンネルPNG読み込みバグ修正)
 *		* Version 1.16B5
 *			: DIBtoGIF関数バグ修正
 *			: DIBtoGIFex関数バグ修正
 *			: DIBtoGIFAni関数バグ修正
 *			: DIBtoGIFAniEx関数バグ修正
 *		* Version 1.16
 *			: GetImageMType関数追加
 *			: MtoDIB関数追加
 *			: GetImageType関数内部仕様変更
 *		* Version 1.17
 *			: GIFtoDIB関数バグ修正
 *			: GIFMtoDIB関数バグ修正
 *			: GIFtoDIBex関数バグ修正
 *			: GIFMtoDIBex関数バグ修正
 *		* Version 1.18
 *			: BMPtoDIB関数修正
 *			: TurnDIB関数バグ修正
 *		* Version 1.19
 *			: PNGtoDIB関数バグ修正
 *			: PNGMtoDIB関数バグ修正
 *		* Version 1.20
 *			: DIBtoGIFex関数バグ修正
 *			: DIBtoGIFAniEx関数バグ修正
 *		* Version 1.21
 *			: BMPtoDIB関数バグ修正
 *			: BMPMtoDIB関数バグ修正
 *			: DIBtoRLE関数バグ修正
 *			: CutDIB関数バグ修正
 *		* Version 1.22
 *			: BMPtoDIB関数バグ修正
 *			: BMPMtoDIB関数バグ修正
 */

#ifndef _INC_IMGCTL
#define _INC_IMGCTL

/* imgctl.h support version */
#define IMGCTL_VERSION			122
#define IMGCTL_BETA				0
#define IMGCTL_VERSION_STRING	"1.22"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef _WINDOWS_
#include <windows.h>
#endif /* _WINDOWS_ */

#ifndef IMGAPI
#define IMGAPI	WINAPI
#endif /* IMGAPI */

/* DIB data handle */
typedef void *HDIB;
typedef const void *HDIBC;

/* 4 bytes alignment [v1.12B7] */
#pragma pack(push, 4)

/* DIB image data ([v1.08] please read dibdata.txt) */
typedef struct
{
	DWORD dwbmiSize;	/* BITMAPINFOサイズ */
	DWORD dwDataSize;	/* 実データサイズ */
	BITMAPINFO *pbmi;	/* BITMAPINFOポインタ */
	BYTE *pData;		/* 実データポインタ(pbmiと連続) */
}
IMGDATA, *PIMGDATA, FAR *LPIMGDATA;
typedef const IMGDATA FAR *LPCIMGDATA;

/* DIB paste information structure */
typedef struct
{
	COLORREF colDest;	/* 貼り付け先の色 */
	COLORREF colSrc;	/* 貼り付け元の色 */
	DWORD dwReserved;	/* 予約(常に0) */
	LPARAM lParam;		/* 関数に渡したLPARAM値 */

	/* v1.10B */
	RECT rcArea;		/* 貼り付け位置 */
	long lXDest;		/* 現在の貼り付け先X座標 */
	long lYDest;		/* 現在の貼り付け先Y座標 */
	long lXSrc;			/* 現在の貼り付け元X座標 */
	long lYSrc;			/* 現在の貼り付け元Y座標 */
}
PASTEINFO, *PPASTEINFO, FAR *LPPASTEINFO;
typedef const PASTEINFO FAR *LPCPASTEINFO;

/* DIB paste hook procedure */
typedef COLORREF (CALLBACK *PASTEPROC)(LPPASTEINFO);

/* Repaint information structure */
typedef struct
{
	COLORREF colBefore;	/* 元の色 */
	COLORREF colAfter;	/* 置き換える色 */
}
REPAINTINFO, *PREPAINTINFO, FAR *LPREPAINTINFO;
typedef const REPAINTINFO FAR *LPCREPAINTINFO;

/* Convert table structure [v1.12B3] */
typedef struct
{
	BYTE tblB[256];	/* B変換テーブル */
	BYTE tblG[256];	/* G変換テーブル */
	BYTE tblR[256];	/* R変換テーブル */
}
CONVTABLE, *PCONVTABLE, FAR *LPCONVTABLE;
typedef const CONVTABLE FAR *LPCCONVTABLE;

/* PNGOPT structure */
typedef struct
{
	DWORD dwFlag;		/* 各種フラグの論理和 */
	WORD wCompLevel;	/* 圧縮率: [低] 0 ～ 9 [高] */

	WORD wReserved;		/* 予約(0にする) [v1.12B7] */

	DWORD dwFilter;		/* フィルタ */
	DWORD dwGamma;		/* ガンマ補正値 (0 ～ 100000) */
	LPARAM clrTrans;	/* 透明色 */
	LPARAM clrBack;		/* 背景色 */
	LPSTR lpText;		/* テキスト */

	DWORD dwReserved1;	/* 予約(0にする) */
	DWORD dwReserved2;	/* 予約(0にする) */
}
PNGOPT, *PPNGOPT, FAR *LPPNGOPT;
typedef const PNGOPT FAR *LPCPNGOPT;

/* 透過情報構造体 for PNGOPT */
typedef struct
{
	BYTE trans[256];	/* 各パレットの透過度(数値が小さいほど透明) */
	DWORD dwNum;		/* transの設定を有効にする数 */
}
PALTRANS, *PPALTRANS, FAR *LPPALTRANS;

/* GIFOPT structure [v1.13B4] */
typedef struct
{
	DWORD dwFlag;		/* 各種フラグの論理和 */

	LPARAM clrTrans;	/* 透過色 */
	LPARAM clrBack;		/* 背景色 */

	WORD wLogWidth;		/* 論理画面幅 */
	WORD wLogHeight;	/* 論理画面高さ */
	WORD wLogLeft;		/* 論理画面での画像開始X位置 */
	WORD wLogTop;		/* 論理画面での画像開始Y位置 */

	DWORD dwLzwCount;	/* LZW辞書クリアを行うエントリ数 */
	DWORD dwBitCount;	/* ビット数 */
}
GIFOPT, *PGIFOPT, FAR *LPGIFOPT;
typedef const GIFOPT FAR *LPCGIFOPT;

/* GIFANIOPT structure [v1.13] */
typedef struct
{
	DWORD dwFlag;		/* 各種フラグの論理和 */

	COLORREF clrBack;	/* 背景色 */
	WORD wLogWidth;		/* 論理画面幅 */
	WORD wLogHeight;	/* 論理画面高さ */
	WORD wLoopCount;	/* 繰り返し回数 */

	WORD wReserved;		/* 予約(0にする) */
}
GIFANIOPT, *PGIFANIOPT, FAR *LPGIFANIOPT;
typedef const GIFANIOPT FAR *LPCGIFANIOPT;

/* GIF animation scene block [v1.13] */
typedef struct
{
	DWORD dwFlag;		/* 各種フラグの論理和 */

	LPARAM clrTrans;	/* 透過色 */
	WORD wLogLeft;		/* 論理画面での画像開始X位置 */
	WORD wLogTop;		/* 論理画面での画像開始Y位置 */
	DWORD dwLzwCount;	/* LZW辞書クリアを行うエントリ数 */
	DWORD dwBitCount;	/* ビット数 */
	WORD wDisposal;		/* 画像表示後処理 */

	WORD wTime;			/* 1/100秒単位での表示時間 */
	HDIB hDIB;			/* DIBハンドル */
}
GIFANISCENE, *PGIFANISCENE, FAR *LPGIFANISCENE;
typedef const GIFANISCENE FAR *LPCGIFANISCENE;

/* Default alignment [v1.12B7] */
#pragma pack(pop)

/* PNGOPT flags (複数可) */
#define POF_COMPLEVEL	0x00000001	/* 圧縮率情報を使う */
#define POF_FILTER		0x00000002	/* フィルタ情報を使う/取得する */
#define POF_GAMMA		0x00000004	/* ガンマ補正情報を使う/取得する */
#define POF_TRNSCOLOR	0x00000008	/* 透明色情報を使う/取得する */
#define POF_BACKCOLOR	0x00000010	/* 背景色情報を使う/取得する */
#define POF_TEXT		0x00000020	/* テキスト情報を使う */
#define POF_TEXTCOMP	0x00000040	/* テキスト圧縮を行う */
#define POF_INTERLACING	0x00000080	/* インターレースにする/である */
#define POF_TIME		0x00000100	/* 最終更新時間を保存する */
#define POF_ALPHACHAN	0x00000200	/* アルファチャネルを取得する [v1.12B5] */
#define POF_BACKPALETTE	0x00010000	/* 背景色の指定/取得にパレット番号を使う(1,4,8Bit) */
#define POF_TRNSPALETTE	0x00020000	/* 透明色の指定/取得にパレット番号を使う(1,4,8Bit) */
#define POF_TRNSALPHA	0x00040000	/* 透明色の指定/取得に透過情報構造体を使う(1,4,8Bit) */

/* PNGOPT filters (複数可) */
#define PO_FILTER_NONE	0x00000008	/* フィルタを適用しない */
#define PO_FILTER_SUB	0x00000010	/* 左のピクセルからの差分情報 */
#define PO_FILTER_UP	0x00000020	/* 上のピクセルからの差分情報 */
#define PO_FILTER_AVG	0x00000040	/* 左と上のピクセルからの予測との差分情報 */
#define PO_FILTER_PAETH	0x00000080	/* 左と上と左上のピクセルからの予測との差分情報 */
#define PO_FILTER_ALL \
	(PO_FILTER_NONE | PO_FILTER_SUB | PO_FILTER_UP | PO_FILTER_AVG | PO_FILTER_PAETH)

/* Populer gammas for PNGOPT */
#define PO_GAMMA_NORMAL	45455		/* (1.0/2.2)*100000 */
#define PO_GAMMA_WIN	PO_GAMMA_NORMAL
#define PO_GAMMA_MAC	55556		/* (1.0/1.8)*100000 */

/* GIFOPT flags (複数可) */
#define GOF_LOGICAL		0x00000001	/* 論理画面情報を使う/がある */
#define GOF_TRNSCOLOR	0x00000008	/* 透明色情報を使う/がある */
#define GOF_BACKCOLOR	0x00000010	/* 背景色情報を使う/がある */
#define GOF_INTERLACING	0x00000080	/* インターレースにする/である */
#define GOF_BACKPALETTE	0x00010000	/* 背景色の指定/取得にパレット番号を使う */
#define GOF_TRNSPALETTE	0x00020000	/* 透明色の指定/取得にパレット番号を使う */
#define GOF_LZWCLRCOUNT	0x00080000	/* 辞書クリアカウントを指定する */
#define GOF_LZWNOTUSE	0x00100000	/* LZW圧縮を行わない(無圧縮GIF) */
#define GOF_BITCOUNT	0x00200000	/* 書き込み/読み込み時の最小ビット数を指定する */

/* GIFANIOPT flags (複数可) */
#define GAF_LOGICAL		0x00000001	/* 論理画面情報を使う */
#define GAF_BACKCOLOR	0x00000010	/* 背景色情報を使う */
#define GAF_LOOPCOUNT	0x00000400	/* ループ回数を指定する */
#define GAF_NOTANI		0x00000800	/* アニメーションしない(複数画像格納GIF) */

/* GIFANISCENE flags (複数可) */
#define GSF_LOGICAL		0x00000001	/* 論理画面情報を使う */
#define GSF_TRNSCOLOR	0x00000008	/* 透明色情報を使う */
#define GSF_INTERLACING	0x00000080	/* インターレースにする */
#define GSF_TRNSPALETTE	0x00020000	/* 透明色の指定にパレット番号を使う */
#define GSF_LZWCLRCOUNT	0x00080000	/* 辞書クリアカウントを指定する */
#define GSF_LZWNOTUSE	0x00100000	/* LZW圧縮を行わない(無圧縮GIF) */
#define GSF_BITCOUNT	0x00200000	/* 書き込み時の最小ビット数を指定する */
#define GSF_DISPOSAL	0x00001000	/* 画像表示後の処理を指定する */
#define GSF_USERINPUT	0x00002000	/* ユーザの入力処理を受け付ける */

/* GIFANISCENE disposal methods */
#define GS_DISP_NONE	0			/* 指定無し(デフォルト) */
#define GS_DISP_LEAVE	1			/* 今の表示を残す */
#define GS_DISP_BACK	2			/* 背景色で塗り潰す */
#define GS_DISP_PREV	3			/* 以前の表示に戻す */

/* Image types */
#define IMG_ERROR	0x00FFFFFF		/* v1.12B9 */
#define IMG_UNKNOWN	0x00000000
#define IMG_BMP		0x00000001
#define IMG_BITMAP	IMG_BMP
#define IMG_DIB		IMG_BMP
#define IMG_RLE		IMG_BMP
#define IMG_JPEG	0x00000002
#define IMG_JPE		IMG_JPEG
#define IMG_JPG		IMG_JPEG
#define IMG_EXIF	IMG_JPEG		/* v1.10B4 */
#define IMG_PNG		0x00000003
#define IMG_GIF		0x00000004
#define IMG_TIFF	0x00000005
#define IMG_TIF		IMG_TIFF
#define IMG_PIC		0x00000006
#define IMG_MAG		0x00000007
#define IMG_MAKI	IMG_MAG
#define IMG_PCX		0x00000008		/* v1.03 */

/* Enough buffer size [v1.16B3] */
#define BUFSIZE_ENOUGH	0xFFFFFFFF

/* Bitfield types */
#define RGB555_R	0x00007C00
#define RGB555_G	0x000003E0
#define RGB555_B	0x0000001F
#define BF16_RGB555	RGB555_R, RGB555_G, RGB555_B
#define RGB565_R	0x0000F800
#define RGB565_G	0x000007E0
#define RGB565_B	0x0000001F
#define BF16_RGB565	RGB565_R, RGB565_G, RGB565_B
#define RGB888_R	0x00FF0000
#define RGB888_G	0x0000FF00
#define RGB888_B	0x000000FF
#define BF32_RGB888	RGB888_R, RGB888_G, RGB888_B

/* Types for DIBto16BitEx & DIBto8Bit [v1.10B4] */
#define TOBIT_DEFAULT	0x00000000
#define TOBIT_ORG		0x00100000	/* v1.10 */
#define TOBIT_DIFF		0x00000001
#define TOBIT_DIFFFS	0x00000002	/* v1.10B5 */	/* default */
#define TOBIT_DIFFJJN	0x00000003	/* v1.10B5 */
#define TOBIT_DIFFX		0x00000101	/* v1.10B7 */
#define TOBIT_DIFFXFS	0x00000102	/* v1.10B7 */
#define TOBIT_DIFFXJJN	0x00000103	/* v1.10B7 */
#define TOBIT_DIFFDX	0x00000201	/* v1.10B8 */
#define TOBIT_DIFFDXFS	0x00000202	/* v1.10B8 */
#define TOBIT_DIFFDXJJN	0x00000203	/* v1.10B8 */

/* Flags for DIBto8Bit */
#define TO8_DIV_RGB		0x00000000
#define TO8_DIV_LIGHT	0x00000001
#define TO8_SEL_CENTER	0x00000000
#define TO8_SEL_AVGRGB	0x00000100
#define TO8_SEL_AVGPIX	0x00000200
#define TO8_PUT_RGB		0x00000000
#define TO8_PUT_LIGHT	0x00010000
#define TO8_PUT_YUV		0x00020000

/* Resize flags [v1.12B] */
#define RESZ_SAME	0		/* そのまま */
#define RESZ_RATIO	(-1)	/* 縦横比を揃える */

/* Replace colors */
#define REP_R		0
#define REP_RED		REP_R
#define REP_G		1
#define REP_GREEN	REP_G
#define REP_B		2
#define REP_BLUE	REP_B

/* Turn types */
#define TURN_90		90
#define TURN_180	180
#define TURN_270	270

/* Error codes (v1.07B2) */
#define ICERR_NONE			0x00000000
#define ICERR_FILE_OPEN		0x00010001
#define ICERR_FILE_READ		0x00010002
#define ICERR_FILE_WRITE	0x00010003
#define ICERR_FILE_TYPE		0x00010004	/* v1.07B3 */
#define ICERR_FILE_NULL		0x00010005	/* v1.07B3 */
#define ICERR_FILE_SEEK		0x00010006	/* v1.13B4 */
#define ICERR_FILE_SIZE		0x00010007	/* v1.16B3 */
#define ICERR_PARAM_NULL	0x00020001
#define ICERR_PARAM_SIZE	0x00020002
#define ICERR_PARAM_TYPE	0x00020003
#define ICERR_PARAM_RANGE	0x00020004	/* v1.13 */
#define ICERR_MEM_ALLOC		0x00030001
#define ICERR_MEM_SIZE		0x00030002	/* v1.16B3 */
#define ICERR_IMG_COMPRESS	0x00040001
#define ICERR_IMG_RLESIZE	0x00040002
#define ICERR_IMG_BITCOUNT	0x00040003	/* v1.12B3 */
#define ICERR_IMG_AREA		0x00040004	/* v1.12B3 */
#define ICERR_IMG_RLETOP	0x00040005	/* v1.12B3 */
#define ICERR_DIB_RLECOMP	0x00050001
#define ICERR_DIB_RLEEXP	0x00050002
#define ICERR_DIB_RLEBIT	0x00050003
#define ICERR_DIB_NULL		0x00050004	/* v1.07B3 */
#define ICERR_DIB_UPPER16	0x00050005	/* v1.07B3 */
#define ICERR_DIB_AREAOUT	0x00050006	/* v1.10B */
#define ICERR_BMP_FILEHEAD	0x00060001
#define ICERR_BMP_HEADSIZE	0x00060002
#define ICERR_BMP_IMGSIZE	0x00060003
#define ICERR_BMP_COMPRESS	0x00060004
#define ICERR_RLE_TOPDOWN	0x00070001
#define ICERR_RLE_DATASIZE	0x00070002
#define ICERR_JPEG_LIBERR	0x00080001
#define ICERR_PNG_LIBERR	0x00090001
#define ICERR_PNG_NOALPHA	0x00090002	/* v1.12B5 */
#define ICERR_GIF_FILEHEAD	0x000B0001	/* v1.13B4 */
#define ICERR_GIF_BLOCK		0x000B0002	/* v1.13B4 */
#define ICERR_API_STRETCH	0x000A0001
#define ICERR_API_SETMODE	0x000A0002
#define ICERR_API_SECTION	0x000A0003	/* v1.09 */
#define ICERR_API_COMDC		0x000A0004	/* v1.09 */
#define ICERR_API_SELOBJ	0x000A0005	/* v1.09 */
#define ICERR_API_BITBLT	0x000A0006	/* v1.09 */

#if !defined(IMGCTL_RUNTIME)

#ifndef __BORLANDC__
#define	_export
#endif	/* __BORLANDC__ */

/* Standard functions */
WORD IMGAPI _export ImgctlVersion(
	void
	);
WORD IMGAPI _export ImgctlBeta(
	void
	);
DWORD IMGAPI _export ImgctlError(
	void
	);
void IMGAPI _export ImgctlErrorClear(
	void
	);
LPVOID IMGAPI _export PointerOf(
	LPVOID pvData
	);
DWORD IMGAPI _export GetImageType(
	LPCSTR lpImageFile,
	DWORD *pdwFlag
	);
DWORD IMGAPI _export GetImageMType(
	const void *pBuffer,
	DWORD dwBufSize,
	DWORD *pdwFlag
	);
HDIB IMGAPI _export ToDIB(
	LPCSTR lpImageFile
	);
HDIB IMGAPI _export MtoDIB(
	const void *pBuffer,
	DWORD dwBufSize
	);

/* DIB functions */
BOOL IMGAPI _export DeleteDIB(
	HDIB hDIB
	);
BOOL IMGAPI _export HeadDIB(
	HDIBC hDIB,
	LPBITMAPINFOHEADER pbmih
	);
DWORD IMGAPI _export PaletteDIB(
	HDIBC hDIB,
	RGBQUAD *rgbColors,
	DWORD dwClrNum
	);
COLORREF IMGAPI _export PixelDIB(
	HDIBC hDIB,
	long lXPos,
	long lYPos
	);
DWORD IMGAPI _export ColorDIB(
	HDIBC hDIB
	);
BOOL IMGAPI _export GetDIB(
	HDIBC hDIB,
	LPBITMAPINFO pbmi,
	DWORD *pdwbmiSize,
	LPVOID pvData,
	DWORD *pdwDataSize
	);
BOOL IMGAPI _export MapDIB(
	HDIB hDIB,
	LPBITMAPINFO *ppbmi,
	DWORD *pdwbmiSize,
	LPVOID *ppvData,
	DWORD *pdwDataSize
	);
LPCIMGDATA IMGAPI _export DataDIB(
	HDIB hDIB
	);
HDIB IMGAPI _export CreateDIB(
	const BITMAPINFO *pbmi,
	LPCVOID pvData
	);
HDIB IMGAPI _export CopyDIB(
	HDIBC hDIB
	);
HDIB IMGAPI _export CutDIB(
	HDIBC hDIB,
	long lX,
	long lY,
	long lWidth,
	long lHeight
	);
BOOL IMGAPI _export TurnDIB(
	HDIB hDIB,
	int iTurnType
	);
BOOL IMGAPI _export DIBto24Bit(
	HDIB hDIB
	);
BOOL IMGAPI _export DIBto16Bit(
	HDIB hDIB,
	const DWORD *dwBitFields
	);
BOOL IMGAPI _export DIBto16BitEx(
	HDIB hDIB,
	const DWORD *dwBitFields,
	DWORD dwType
	);
BOOL IMGAPI _export DIBto8Bit(
	HDIB hDIB,
	DWORD dwFlags,
	DWORD dwType
	);
/* 24Bit DIB functions */
BOOL IMGAPI _export PasteDIB(
	HDIB hDIBDest,
	long lXDest,
	long lYDest,
	long lWidth,
	long lHeight,
	HDIBC hDIBSrc,
	long lXSrc,
	long lYSrc,
	PASTEPROC pfnPasteProc,
	LPARAM lParam
	);
BOOL IMGAPI _export ResizeDIB(
	HDIB hDIB,
	long lWidth,
	long lHeight
	);
BOOL IMGAPI _export TurnDIBex(
	HDIB hDIB,
	long lAngle,
	COLORREF clrBack
	);

/* RLE-DIB functions */
BOOL IMGAPI _export IsRLE(
	HDIBC hDIB
	);
BOOL IMGAPI _export DIBtoRLE(
	HDIB hDIB
	);
BOOL IMGAPI _export RLEtoDIB(
	HDIB hRLE
	);

/* Bitmap functions */
BOOL IMGAPI _export DIBtoBMP(
	LPCSTR lpBmpFile,
	HDIBC hDIB
	);
HDIB IMGAPI _export BMPtoDIB(
	LPCSTR lpBmpFile
	);
HDIB IMGAPI _export BMPMtoDIB(
	const void *pBuffer,
	DWORD dwBufSize
	);

/* JPEG functions */
BOOL IMGAPI _export DIBtoJPG(
	LPCSTR lpJpegFile,
	HDIBC hDIB,
	int iQuality,
	BOOL bProgression
	);
HDIB IMGAPI _export JPGtoDIB(
	LPCSTR lpJpegFile
	);
HDIB IMGAPI _export JPGMtoDIB(
	const void *pBuffer,
	DWORD dwBufSize
	);

/* PNG functions */
BOOL IMGAPI _export DIBtoPNG(
	LPCSTR lpPngFile,
	HDIBC hDIB,
	BOOL bInterlacing
	);
BOOL IMGAPI _export DIBtoPNGex(
	LPCSTR lpPngFile,
	HDIBC hDIB,
	LPCPNGOPT pPngOpt
	);
HDIB IMGAPI _export PNGtoDIB(
	LPCSTR lpPngFile
	);
HDIB IMGAPI _export PNGMtoDIB(
	const void *pBuffer,
	DWORD dwBufSize
	);
HDIB IMGAPI _export PNGAtoDIB(
	LPCSTR lpPngFile
	);
HDIB IMGAPI _export PNGMAtoDIB(
	const void *pBuffer,
	DWORD dwBufSize
	);
BOOL IMGAPI _export InfoPNG(
	LPCSTR lpPngFile,
	LPPNGOPT pPngOpt,
	LPBITMAPINFO pbmi,
	DWORD dwbmiSize
	);
BOOL IMGAPI _export InfoPNGM(
	const void *pBuffer,
	DWORD dwBufSize,
	LPPNGOPT pPngOpt,
	LPBITMAPINFO pbmi,
	DWORD dwbmiSize
	);

/* GIF functions */
BOOL IMGAPI _export DIBtoGIF(
	LPCSTR lpGifFile,
	HDIBC hDIB,
	BOOL bInterlacing
	);
BOOL IMGAPI _export DIBtoGIFex(
	LPCSTR lpGifFile,
	HDIBC hDIB,
	LPCGIFOPT pGifOpt
	);
BOOL IMGAPI _export DIBtoGIFAni(
	LPCSTR lpGifFile,
	const HDIB *hDIBs,
	DWORD dwCount,
	WORD wTime
	);
BOOL IMGAPI _export DIBtoGIFAniEx(
	LPCSTR lpGifFile,
	const GIFANISCENE *aniScenes,
	DWORD dwCount,
	LPCGIFANIOPT pGifAniOpt
	);
HDIB IMGAPI _export GIFtoDIB(
	LPCSTR lpGifFile
	);
HDIB IMGAPI _export GIFMtoDIB(
	const void *pBuffer,
	DWORD dwBufSize
	);
HDIB IMGAPI _export GIFtoDIBex(
	LPCSTR lpGifFile,
	LPGIFOPT pGifOpt
	);
HDIB IMGAPI _export GIFMtoDIBex(
	const void *pBuffer,
	DWORD dwBufSize,
	LPGIFOPT pGifOpt
	);

/* Filter functions */
BOOL IMGAPI _export GrayDIB(
	HDIB hDIB,
	WORD wStrength
	);
BOOL IMGAPI _export ReplaceDIB(
	HDIB hDIB,
	WORD rTo,
	WORD gTo,
	WORD bTo
	);
BOOL IMGAPI _export RepaintDIB(
	HDIB hDIB,
	const REPAINTINFO *repis,
	DWORD dwRepaintNum
	);
BOOL IMGAPI _export TableDIB(
	HDIB hDIB,
	LPCCONVTABLE ptable
	);
/* Convert table functions */
BOOL IMGAPI _export ToneDIB(
	HDIB hDIB,
	short rMove,
	short gMove,
	short bMove
	);
BOOL IMGAPI _export ShadeDIB(
	HDIB hDIB,
	DWORD rMul,
	DWORD gMul,
	DWORD bMul
	);
BOOL IMGAPI _export GammaDIB(
	HDIB hDIB,
	WORD rGma,
	WORD gGma,
	WORD bGma
	);
BOOL IMGAPI _export ContrastDIB(
	HDIB hDIB,
	short rStr,
	short gStr,
	short bStr
	);

/* Device context functions */
BOOL IMGAPI _export DIBtoDC(
	HDC hdc,
	int nXDest,
	int nYDest,
	int nWidth,
	int nHeight,
	HDIBC hDIB,
	int nXSrc,
	int nYSrc,
	DWORD dwRop
	);
BOOL IMGAPI _export DIBtoDCex(
	HDC hdc,
	int nXDest,
	int nYDest,
	int nDestWidth,
	int nDestHeight,
	HDIBC hDIB,
	int nXSrc,
	int nYSrc,
	int nSrcWidth,
	int nSrcHeight,
	DWORD dwRop
	);
BOOL IMGAPI _export DIBtoDCex2(
	HDC hdc,
	int nXDest,
	int nYDest,
	int nDestWidth,
	int nDestHeight,
	HDIBC hDIB,
	int nXSrc,
	int nYSrc,
	int nSrcWidth,
	int nSrcHeight,
	DWORD dwRop,
	int iStretchMode
	);
HDIB IMGAPI _export DCtoDIB(
	HDC hdc,
	long lXSrc,
	long lYSrc,
	long lWidth,
	long lHeight
	);
/* for older than v1.08 */
#define DIBDIBits			DIBtoDC
#define DIBStretchDIBits	DIBtoDCex
#define DIBStretchDIBits2	DIBtoDCex2

#elif defined(IMGCTL_RUNTIME) && !defined(IMGCTL_TYPEDEF_NOTUSE)

/* Standard functions */
typedef WORD (IMGAPI *IMGCTLVERSION)(void);
typedef WORD (IMGAPI *IMGCTLBETA)(void);
typedef DWORD (IMGAPI *IMGCTLERROR)(void);
typedef void (IMGAPI *IMGCTLERRORCLEAR)(void);
typedef DWORD (IMGAPI *GETIMAGETYPE)(LPCSTR, DWORD *);
typedef DWORD (IMGAPI *GETIMAGEMTYPE)(const void *, DWORD, DWORD *);
typedef HDIB (IMGAPI *TODIB)(LPCSTR);
typedef HDIB (IMGAPI *MTODIB)(const void *, DWORD);

/* DIB functions */
typedef BOOL (IMGAPI *DELETEDIB)(HDIB);
typedef BOOL (IMGAPI *HEADDIB)(HDIBC, LPBITMAPINFOHEADER);
typedef DWORD (IMGAPI *PALETTEDIB)(HDIBC, RGBQUAD *, DWORD);
typedef COLORREF (IMGAPI *PIXELDIB)(HDIBC, long, long);
typedef DWORD (IMGAPI *COLORDIB)(HDIBC);
typedef BOOL (IMGAPI *GETDIB)(HDIBC, LPBITMAPINFO, DWORD *, LPVOID, DWORD *);
typedef BOOL (IMGAPI *MAPDIB)(HDIB, LPBITMAPINFO *, DWORD *, LPVOID *, DWORD *);
typedef LPCIMGDATA (IMGAPI *DATADIB)(HDIB);
typedef HDIB (IMGAPI *CREATEDIB)(const BITMAPINFO *, LPCVOID);
typedef HDIB (IMGAPI *COPYDIB)(HDIBC);
typedef HDIB (IMGAPI *CUTDIB)(HDIBC, long, long, long, long);
typedef BOOL (IMGAPI *TURNDIB)(HDIB, int);
typedef BOOL (IMGAPI *DIBTO24BIT)(HDIB);
typedef BOOL (IMGAPI *DIBTO16BIT)(HDIB, const DWORD *);
typedef BOOL (IMGAPI *DIBTO16BITEX)(HDIB, const DWORD *, DWORD);
typedef BOOL (IMGAPI *DIBTO8BIT)(HDIB, DWORD, DWORD);
/* 24Bit DIB functions */
typedef BOOL (IMGAPI *PASTEDIB)(
	HDIB, long, long, long, long, HDIBC, long, long, PASTEPROC, LPARAM);
typedef BOOL (IMGAPI *RESIZEDIB)(HDIB, long, long);
typedef BOOL (IMGAPI *TURNDIBEX)(HDIB, long, COLORREF);

/* RLE-DIB functions */
typedef BOOL (IMGAPI *ISRLE)(HDIBC);
typedef BOOL (IMGAPI *DIBTORLE)(HDIB);
typedef BOOL (IMGAPI *RLETODIB)(HDIB);

/* Bitmap functions */
typedef BOOL (IMGAPI *DIBTOBMP)(LPCSTR, HDIBC);
typedef HDIB (IMGAPI *BMPTODIB)(LPCSTR);
typedef HDIB (IMGAPI *BMPMTODIB)(const void *, DWORD);

/* JPEG functions */
typedef BOOL (IMGAPI *DIBTOJPG)(LPCSTR, HDIBC, int, BOOL);
typedef HDIB (IMGAPI *JPGTODIB)(LPCSTR);
typedef HDIB (IMGAPI *JPGMTODIB)(const void *, DWORD);

/* PNG functions */
typedef BOOL (IMGAPI *DIBTOPNG)(LPCSTR, HDIBC, BOOL);
typedef BOOL (IMGAPI *DIBTOPNGEX)(LPCSTR, HDIBC, LPCPNGOPT);
typedef HDIB (IMGAPI *PNGTODIB)(LPCSTR);
typedef HDIB (IMGAPI *PNGMTODIB)(const void *, DWORD);
typedef HDIB (IMGAPI *PNGATODIB)(LPCSTR);
typedef HDIB (IMGAPI *PNGMATODIB)(const void *, DWORD);
typedef BOOL (IMGAPI *INFOPNG)(LPCSTR, LPPNGOPT, LPBITMAPINFO, DWORD);
typedef BOOL (IMGAPI *INFOPNGM)(const void *, DWORD, LPPNGOPT, LPBITMAPINFO, DWORD);
typedef HDIB (IMGAPI *JPGMTODIB)(const void *, DWORD);

/* GIF functions */
typedef BOOL (IMGAPI *DIBTOGIF)(LPCSTR, HDIBC, BOOL);
typedef BOOL (IMGAPI *DIBTOGIFEX)(LPCSTR, HDIBC, LPCGIFOPT);
typedef BOOL (IMGAPI *DIBTOGIFANI)(LPCSTR, const HDIB *, DWORD, WORD);
typedef BOOL (IMGAPI *DIBTOGIFANIEX)(LPCSTR, const GIFANISCENE *, DWORD, LPCGIFANIOPT);
typedef HDIB (IMGAPI *GIFTODIB)(LPCSTR);
typedef HDIB (IMGAPI *GIFMTODIB)(const void *, DWORD);
typedef HDIB (IMGAPI *GIFTODIBEX)(LPCSTR, LPGIFOPT);
typedef HDIB (IMGAPI *GIFMTODIBEX)(const void *, DWORD, LPGIFOPT);

/* Filter functions */
typedef BOOL (IMGAPI *GRAYDIB)(HDIB, WORD);
typedef BOOL (IMGAPI *REPLACEDIB)(HDIB, WORD, WORD, WORD);
typedef BOOL (IMGAPI *REPAINTDIB)(HDIB, const REPAINTINFO *, DWORD);
typedef BOOL (IMGAPI *TABLEDIB)(HDIB, LPCCONVTABLE);
/* Convert table functions */
typedef BOOL (IMGAPI *TONEDIB)(HDIB, short, short, short);
typedef BOOL (IMGAPI *SHADEDIB)(HDIB, DWORD, DWORD, DWORD);
typedef BOOL (IMGAPI *GAMMADIB)(HDIB, WORD, WORD, WORD);
typedef BOOL (IMGAPI *CONTRASTDIB)(HDIB, short, short, short);

/* Device context functions */
typedef BOOL (IMGAPI *DIBTODC)(HDC, int, int, int, int, HDIBC, int, int, DWORD);
typedef BOOL (IMGAPI *DIBTODCEX)(
	HDC, int, int, int, int, HDIBC, int, int, int, int, DWORD);
typedef BOOL (IMGAPI *DIBTODCEX2)(
	HDC, int, int, int, int, HDIBC, int, int, int, int, DWORD, int);
typedef HDIB (IMGAPI *DCTODIB)(HDC, long, long, long, long);
/* for older than v1.08 */
typedef	DIBTODC DIBDIBITS;
typedef DIBTODCEX DIBSTRETCHDIBITS;
typedef DIBTODCEX2 DIBSTRETCHDIBITS2;

#endif /* IMGCTL_RUNTIME */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _INC_IMGCTL */