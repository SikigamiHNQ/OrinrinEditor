/*
 ************************************************
 *	[imgctl.h] Version 1.22
 *		For <imgctl.dll> version 1.22.
 *		<imgctl.dll> is free library.
 ************************************************
 */
/*
 *	[���쌠�ɂ���]
 *		imgctl.dll �� ���[�`�F �ɒ��쌠������܂��B
 *		<imgctl.dll> copyright (C) ruche.
 *
 *	[JPEG�ɂ���]
 *		imgctl.dll �� JPEG�ǂݏ��������ɂ� The Independent JPEG Group (IJG) ��
 *		�쐬�����Ajpeg.lib �����p����Ă��܂��B
 *
 *	[PNG�ɂ���]
 *		imgctl.dll ��PNG�ǂݏ��������ɂ� libpng.lib �� zlib.lib �����p����Ă��܂��B
 *		�܂��APNG�̓ǂݍ��݂́A�{�⌫������ ifpng.spi �̃\�[�X���Q�l�ɂ���Ă��܂��B
 */
/*
 *	[�X�V����]
 *		* Version 1.00B
 *			: ����e�X�g�f���ŉ����J
 *		* Version 1.00
 *			: �����Ɍ��J
 *			: HeadDIB�֐��ǉ�
 *			: ColorDIB�֐��ǉ�
 *			: DIBto16Bit�֐��ǉ�
 *			: DIBto24Bit�֐��d�l�ύX
 *			: DIBtoRLE�֐��d�l�ύX
 *			: RLEtoDIB�֐��d�l�ύX
 *			: SizeDIB�֐��폜
 *		* Version 1.01B
 *			: ToDIB�֐��ǉ�
 *			: PaletteDIB�֐��ǉ�
 *			: PixelDIB�֐��ǉ�
 *			: CutDIB�֐��ǉ�
 *			: DIBtoPNGex�֐��ǉ�
 *			: HeadDIB�֐��o�O�C��
 *		* Version 1.01B2
 *			: ImgctlBeta�֐��ǉ�
 *			: DCtoDIB�֐��ǉ�
 *		* Version 1.01
 *			: DIBtoPNGex�֐������d�l�ύX
 *		* Version 1.02B
 *			: DIBto8Bit�֐��ǉ�
 *			: DIBto24Bit�֐��o�O�C��
 *			: DIBto16Bit�֐��o�O�C��
 *		* Version 1.02B2
 *			: GrayDIB�֐��ǉ�
 *			: ToneDIB�֐��ǉ�
 *			: ReplaceDIB�֐��ǉ�
 *		* Version 1.02
 *			: PNGtoDIB�֐��s���C��
 *		* Version 1.03B
 *			: GetImageType�֐��g��
 *			: DIBto8Bit�֐��g��
 *			: BMPtoDIB�֐��g��
 *		* Version 1.03B2
 *			: JPGtoDIB�֐��g��
 *		* Version 1.03B3
 *			: GetImageType�֐��C��
 *		* Version 1.03
 *			: GetImageType�֐��g��
 *		* Version 1.04B
 *			: GetImageType�֐��C��
 *		* Version 1.04
 *			: DIBStretchDIBits2�֐��ǉ�
 *		* Version 1.05B
 *			: PasteDIB�֐��ǉ�
 *			: CutDIB�֐��s���C��
 *			: DIBDIBits�֐��s���C��
 *			: DIBStretchDIBits�֐��s���C��
 *		* Version 1.05
 *			: InfoPNG�֐��ǉ�
 *		* Version 1.06B
 *			: TurnDIB�֐��ǉ�
 *			: CutDIB�֐����C��
 *			: DIBStretchDIBits2�֐����C��
 *			: �قȂ�Version�Ԃ̌݊�(v1.05���)
 *		* Version 1.06
 *			: RepaintDIB�֐��ǉ�
 *		* Version 1.07B
 *			: GetImageType�֐����C��
 *			: DIBDIBits�֐��C��
 *			: DIBStretchDIBits�֐��C��
 *		* Version 1.07B2
 *			: ImgctlError�֐��ǉ�
 *			: ImgctlErrorClear�֐��ǉ�
 *			: PixelDIB�֐��o�O�C��
 *			: GetDIB�֐��C��
 *		* Version 1.07B3
 *			: ImgctlError�֐��p�G���[�R�[�h�g��
 *			: HDIB�^�������Ɏ��֐���NULL�`�F�b�N�����ǉ�
 *			: �t�@�C�����������Ɏ��֐���NULL�`�F�b�N�����ǉ�
 *			: �e��֐��̈����ɂ�����s��NULL�̃`�F�b�N�����ǉ�
 *		* Version 1.07
 *			: JPEG���C�u�����y��PNG���C�u�����̍č\�z
 *		* Version 1.08B (v1.08�ƌ�ݒ�)
 *			: PointerOf�֐��ǉ�
 *			: �o�[�W����������萔IMGCTL_VERSION_STRING�ǉ�
 *		* Version 1.08B2
 *			: DataDIB�֐��ǉ�
 *			: DIBtoDC�֐��ǉ�(DIBDIBits�֐��Ɠ��@�\)
 *			: DIBtoDCex�֐��ǉ�(DIBStretchDIBits�֐��Ɠ��@�\)
 *			: DIBtoDCex2�֐��ǉ�(DIBStretchDIBits2�֐��Ɠ��@�\)
 *			: MapDIB�֐��̑�1������HDIBC�^����HDIB�^�ɕύX
 *			: �������̊m�ە��@��malloc����GlobalAlloc�ɕύX
 *		* Version 1.08
 *			: DataDIB�֐��d�l�ύX
 *			: �������̊m�ە��@��GlobalAlloc����malloc�ɖ߂���
 *		* Version 1.09
 *			: DIBtoDC�֐��啝�C��(BitBlt�Ɗ��S�ɓ��@�\)
 *			: DIBtoDCex�֐��啝�C��(StretchBlt�Ɗ��S�ɓ��@�\)
 *			: ��2�̊֐�������ŗ��p����֐����A���I�Ɏd�l�ύX
 *			: ImgctlError�֐��p�G���[�R�[�h�g��
 *			: DIBtoPNGex�֐��o�O�C��
 *		* Version 1.10B
 *			: DIBtoDC�֐��C��(���x����BitBlt�Ɗ��S�ɓ��@�\)
 *			: DIBtoDCex�֐��C��(���x����StretchBlt�Ɗ��S�ɓ��@�\)
 *			: ��2�̊֐�������ŗ��p����֐����A���I�Ɏd�l�ύX
 *			: PasteDIB�֐��o�O���d�l�C���y�ъg��(���W�w���DIBtoDC�Ɠ����d�l)
 *			: ImgctlError�֐��p�G���[�R�[�h�g��
 *		* Version 1.10B2
 *			: GetImageType�֐��p�萔�C��
 *		* Version 1.10B3
 *			: DIBtoJPG�֐��g��
 *			: JPGtoDIB�֐��g��
 *			: DIBtoPNG�֐��g��
 *			: DIBtoPNGex�֐��g��
 *			: PNGtoDIB�֐��C��
 *			: InfoPNG�֐��C��
 *		* Version 1.10B4
 *			: DIBto16BitEx�֐��ǉ�
 *			: GetImageType�֐��g��
 *			: CreateDIB�֐��g��
 *			: DIBto16Bit�֐����C��
 *			: DIBto8Bit�֐��g��
 *			: BMPtoDIB�֐����C��
 *			: 16Bit/32Bit�̐F�������C��
 *		* Version 1.10B5
 *			: DIBto16BitEx�֐��g��
 *			: DIBto8Bit�֐��g��
 *		* Version 1.10B6
 *			: DIBto16BitEx�֐��C���y�ъg��
 *			: DIBto8Bit�֐��C���y�ъg��
 *		* Version 1.10B7
 *			: DIBto16BitEx�֐��d�l�ύX�y�яC���y�ъg��
 *			: DIBto8Bit�֐��d�l�ύX�y�яC���y�ъg��
 *		* Version 1.10B8
 *			: DIBto16BitEx�֐��g��
 *			: DIBto8Bit�֐��g��
 *		* Version 1.10
 *			: DIBto16BitEx�֐��d�l�ύX
 *			: DIBto8Bit�֐��d�l�ύX
 *		* Version 1.11B
 *			: DeleteDIB�֐����C��
 *			: libpng.lib�y��zlib.lib���o�[�W�����A�b�v
 *		* Version 1.11
 *			: DIBtoJPG�֐��v���I�o�O�C��
 *		* Version 1.12B
 *			: ResizeDIB�֐��ǉ�
 *			: InfoPNG�֐��o�O�C��
 *		* Version 1.12B2
 *			: GammaDIB�֐��ǉ�
 *			: ContrastDIB�֐��ǉ�
 *			: ResizeDIB�֐������d�l�ύX
 *		* Version 1.12B3
 *			: TableDIB�֐��ǉ�
 *			: ShadeDIB�֐��ǉ�
 *			: GammaDIB�֐��d�l�ύX
 *			: ContrastDIB�֐��d�l�ύX
 *			: CreateDIB�֐������d�l�ύX�y�ъg��
 *			: ToneDIB�֐������d�l�ύX
 *			: ImgctlError�֐��p�G���[�R�[�h�g��
 *		* Version 1.12B4
 *			: TurnDIBex�֐��ǉ�
 *		* Version 1.12B4f
 *			: DataDIB�֐��C��(C/C++)
 *		* Version 1.12B5
 *			: PNGAtoDIB�֐��ǉ�
 *			: PNGtoDIB�֐������d�l�ύX
 *			: ImgctlError�֐��p�G���[�R�[�h�g��
 *		* Version 1.12B6
 *			: DIBtoPNGex�֐��C��
 *		* Version 1.12B7
 *			: DIBtoPNGex�֐��C��
 *			: �\���̃����o�̃A���C�������g���w��
 *			: PNGOPT�\���̂ɗ\��ϐ��ǉ�
 *		* Version 1.12B8
 *			: DIBtoJPG�֐��o�O�C��
 *			: JPGtoDIB�֐��o�O�C��
 *		* Version 1.12B9
 *			: GetImageType�֐��d�l�ύX
 *		* Version 1.12B10
 *			: GetImageType�֐��C��
 *		* Version 1.12
 *			: DIBtoBMP�֐��C��
 *		* Version 1.13B
 *			: JPGtoDIB�֐��g��
 *		* Version 1.13B2
 *			: DIBtoJPG�֐��������I�ɓ����d�l�ύX
 *		* Version 1.13B3
 *			: JPGtoDIB�֐��C��
 *		* Version 1.13B4
 *			: DIBtoGIF�֐��ǉ�
 *			: DIBtoGIFex�֐��ǉ�
 *			: GIFtoDIB�֐��ǉ�
 *			: GIFtoDIBex�֐��ǉ�
 *		* Version 1.13
 *			: DIBtoGIFAni�֐��ǉ�
 *			: DIBtoGIFAniEx�֐��ǉ�
 *		* Version 1.14
 *			: GIFtoDIB�֐��v���I�o�O�C��
 *			: GIFtoDIBex�֐��v���I�o�O�C��
 *		* Version 1.15
 *			: DIBto24Bit�֐��o�O�C��
 *			: DIBto8Bit�֐��������I�ɓ����d�l�ύX
 *		* Version 1.16B
 *			: libpng.lib�y��zlib.lib���o�[�W�����A�b�v
 *		* Version 1.16B2
 *			: DIBtoPNG�֐��y��DIBtoPNGex�֐��v���I�o�O�C��
 *			: libpng.lib���o�[�W�����A�b�v
 *		* Version 1.16B3
 *			: BMPMtoDIB�֐��ǉ�
 *			: JPGMtoDIB�֐��ǉ�
 *			: PNGMtoDIB�֐��ǉ�
 *			: PNGMAtoDIB�֐��ǉ�
 *			: InfoPNGM�֐��ǉ�
 *			: GIFMtoDIB�֐��ǉ�
 *			: GIFMtoDIBex�֐��ǉ�
 *			: ImgctlError�֐��p�G���[�R�[�h�g��
 *		* Version 1.16B4
 *			: libpng.lib���o�[�W�����A�b�v(���`�����l��PNG�ǂݍ��݃o�O�C��)
 *		* Version 1.16B5
 *			: DIBtoGIF�֐��o�O�C��
 *			: DIBtoGIFex�֐��o�O�C��
 *			: DIBtoGIFAni�֐��o�O�C��
 *			: DIBtoGIFAniEx�֐��o�O�C��
 *		* Version 1.16
 *			: GetImageMType�֐��ǉ�
 *			: MtoDIB�֐��ǉ�
 *			: GetImageType�֐������d�l�ύX
 *		* Version 1.17
 *			: GIFtoDIB�֐��o�O�C��
 *			: GIFMtoDIB�֐��o�O�C��
 *			: GIFtoDIBex�֐��o�O�C��
 *			: GIFMtoDIBex�֐��o�O�C��
 *		* Version 1.18
 *			: BMPtoDIB�֐��C��
 *			: TurnDIB�֐��o�O�C��
 *		* Version 1.19
 *			: PNGtoDIB�֐��o�O�C��
 *			: PNGMtoDIB�֐��o�O�C��
 *		* Version 1.20
 *			: DIBtoGIFex�֐��o�O�C��
 *			: DIBtoGIFAniEx�֐��o�O�C��
 *		* Version 1.21
 *			: BMPtoDIB�֐��o�O�C��
 *			: BMPMtoDIB�֐��o�O�C��
 *			: DIBtoRLE�֐��o�O�C��
 *			: CutDIB�֐��o�O�C��
 *		* Version 1.22
 *			: BMPtoDIB�֐��o�O�C��
 *			: BMPMtoDIB�֐��o�O�C��
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
	DWORD dwbmiSize;	/* BITMAPINFO�T�C�Y */
	DWORD dwDataSize;	/* ���f�[�^�T�C�Y */
	BITMAPINFO *pbmi;	/* BITMAPINFO�|�C���^ */
	BYTE *pData;		/* ���f�[�^�|�C���^(pbmi�ƘA��) */
}
IMGDATA, *PIMGDATA, FAR *LPIMGDATA;
typedef const IMGDATA FAR *LPCIMGDATA;

/* DIB paste information structure */
typedef struct
{
	COLORREF colDest;	/* �\��t����̐F */
	COLORREF colSrc;	/* �\��t�����̐F */
	DWORD dwReserved;	/* �\��(���0) */
	LPARAM lParam;		/* �֐��ɓn����LPARAM�l */

	/* v1.10B */
	RECT rcArea;		/* �\��t���ʒu */
	long lXDest;		/* ���݂̓\��t����X���W */
	long lYDest;		/* ���݂̓\��t����Y���W */
	long lXSrc;			/* ���݂̓\��t����X���W */
	long lYSrc;			/* ���݂̓\��t����Y���W */
}
PASTEINFO, *PPASTEINFO, FAR *LPPASTEINFO;
typedef const PASTEINFO FAR *LPCPASTEINFO;

/* DIB paste hook procedure */
typedef COLORREF (CALLBACK *PASTEPROC)(LPPASTEINFO);

/* Repaint information structure */
typedef struct
{
	COLORREF colBefore;	/* ���̐F */
	COLORREF colAfter;	/* �u��������F */
}
REPAINTINFO, *PREPAINTINFO, FAR *LPREPAINTINFO;
typedef const REPAINTINFO FAR *LPCREPAINTINFO;

/* Convert table structure [v1.12B3] */
typedef struct
{
	BYTE tblB[256];	/* B�ϊ��e�[�u�� */
	BYTE tblG[256];	/* G�ϊ��e�[�u�� */
	BYTE tblR[256];	/* R�ϊ��e�[�u�� */
}
CONVTABLE, *PCONVTABLE, FAR *LPCONVTABLE;
typedef const CONVTABLE FAR *LPCCONVTABLE;

/* PNGOPT structure */
typedef struct
{
	DWORD dwFlag;		/* �e��t���O�̘_���a */
	WORD wCompLevel;	/* ���k��: [��] 0 �` 9 [��] */

	WORD wReserved;		/* �\��(0�ɂ���) [v1.12B7] */

	DWORD dwFilter;		/* �t�B���^ */
	DWORD dwGamma;		/* �K���}�␳�l (0 �` 100000) */
	LPARAM clrTrans;	/* �����F */
	LPARAM clrBack;		/* �w�i�F */
	LPSTR lpText;		/* �e�L�X�g */

	DWORD dwReserved1;	/* �\��(0�ɂ���) */
	DWORD dwReserved2;	/* �\��(0�ɂ���) */
}
PNGOPT, *PPNGOPT, FAR *LPPNGOPT;
typedef const PNGOPT FAR *LPCPNGOPT;

/* ���ߏ��\���� for PNGOPT */
typedef struct
{
	BYTE trans[256];	/* �e�p���b�g�̓��ߓx(���l���������قǓ���) */
	DWORD dwNum;		/* trans�̐ݒ��L���ɂ��鐔 */
}
PALTRANS, *PPALTRANS, FAR *LPPALTRANS;

/* GIFOPT structure [v1.13B4] */
typedef struct
{
	DWORD dwFlag;		/* �e��t���O�̘_���a */

	LPARAM clrTrans;	/* ���ߐF */
	LPARAM clrBack;		/* �w�i�F */

	WORD wLogWidth;		/* �_����ʕ� */
	WORD wLogHeight;	/* �_����ʍ��� */
	WORD wLogLeft;		/* �_����ʂł̉摜�J�nX�ʒu */
	WORD wLogTop;		/* �_����ʂł̉摜�J�nY�ʒu */

	DWORD dwLzwCount;	/* LZW�����N���A���s���G���g���� */
	DWORD dwBitCount;	/* �r�b�g�� */
}
GIFOPT, *PGIFOPT, FAR *LPGIFOPT;
typedef const GIFOPT FAR *LPCGIFOPT;

/* GIFANIOPT structure [v1.13] */
typedef struct
{
	DWORD dwFlag;		/* �e��t���O�̘_���a */

	COLORREF clrBack;	/* �w�i�F */
	WORD wLogWidth;		/* �_����ʕ� */
	WORD wLogHeight;	/* �_����ʍ��� */
	WORD wLoopCount;	/* �J��Ԃ��� */

	WORD wReserved;		/* �\��(0�ɂ���) */
}
GIFANIOPT, *PGIFANIOPT, FAR *LPGIFANIOPT;
typedef const GIFANIOPT FAR *LPCGIFANIOPT;

/* GIF animation scene block [v1.13] */
typedef struct
{
	DWORD dwFlag;		/* �e��t���O�̘_���a */

	LPARAM clrTrans;	/* ���ߐF */
	WORD wLogLeft;		/* �_����ʂł̉摜�J�nX�ʒu */
	WORD wLogTop;		/* �_����ʂł̉摜�J�nY�ʒu */
	DWORD dwLzwCount;	/* LZW�����N���A���s���G���g���� */
	DWORD dwBitCount;	/* �r�b�g�� */
	WORD wDisposal;		/* �摜�\���㏈�� */

	WORD wTime;			/* 1/100�b�P�ʂł̕\������ */
	HDIB hDIB;			/* DIB�n���h�� */
}
GIFANISCENE, *PGIFANISCENE, FAR *LPGIFANISCENE;
typedef const GIFANISCENE FAR *LPCGIFANISCENE;

/* Default alignment [v1.12B7] */
#pragma pack(pop)

/* PNGOPT flags (������) */
#define POF_COMPLEVEL	0x00000001	/* ���k�������g�� */
#define POF_FILTER		0x00000002	/* �t�B���^�����g��/�擾���� */
#define POF_GAMMA		0x00000004	/* �K���}�␳�����g��/�擾���� */
#define POF_TRNSCOLOR	0x00000008	/* �����F�����g��/�擾���� */
#define POF_BACKCOLOR	0x00000010	/* �w�i�F�����g��/�擾���� */
#define POF_TEXT		0x00000020	/* �e�L�X�g�����g�� */
#define POF_TEXTCOMP	0x00000040	/* �e�L�X�g���k���s�� */
#define POF_INTERLACING	0x00000080	/* �C���^�[���[�X�ɂ���/�ł��� */
#define POF_TIME		0x00000100	/* �ŏI�X�V���Ԃ�ۑ����� */
#define POF_ALPHACHAN	0x00000200	/* �A���t�@�`���l�����擾���� [v1.12B5] */
#define POF_BACKPALETTE	0x00010000	/* �w�i�F�̎w��/�擾�Ƀp���b�g�ԍ����g��(1,4,8Bit) */
#define POF_TRNSPALETTE	0x00020000	/* �����F�̎w��/�擾�Ƀp���b�g�ԍ����g��(1,4,8Bit) */
#define POF_TRNSALPHA	0x00040000	/* �����F�̎w��/�擾�ɓ��ߏ��\���̂��g��(1,4,8Bit) */

/* PNGOPT filters (������) */
#define PO_FILTER_NONE	0x00000008	/* �t�B���^��K�p���Ȃ� */
#define PO_FILTER_SUB	0x00000010	/* ���̃s�N�Z������̍������ */
#define PO_FILTER_UP	0x00000020	/* ��̃s�N�Z������̍������ */
#define PO_FILTER_AVG	0x00000040	/* ���Ə�̃s�N�Z������̗\���Ƃ̍������ */
#define PO_FILTER_PAETH	0x00000080	/* ���Ə�ƍ���̃s�N�Z������̗\���Ƃ̍������ */
#define PO_FILTER_ALL \
	(PO_FILTER_NONE | PO_FILTER_SUB | PO_FILTER_UP | PO_FILTER_AVG | PO_FILTER_PAETH)

/* Populer gammas for PNGOPT */
#define PO_GAMMA_NORMAL	45455		/* (1.0/2.2)*100000 */
#define PO_GAMMA_WIN	PO_GAMMA_NORMAL
#define PO_GAMMA_MAC	55556		/* (1.0/1.8)*100000 */

/* GIFOPT flags (������) */
#define GOF_LOGICAL		0x00000001	/* �_����ʏ����g��/������ */
#define GOF_TRNSCOLOR	0x00000008	/* �����F�����g��/������ */
#define GOF_BACKCOLOR	0x00000010	/* �w�i�F�����g��/������ */
#define GOF_INTERLACING	0x00000080	/* �C���^�[���[�X�ɂ���/�ł��� */
#define GOF_BACKPALETTE	0x00010000	/* �w�i�F�̎w��/�擾�Ƀp���b�g�ԍ����g�� */
#define GOF_TRNSPALETTE	0x00020000	/* �����F�̎w��/�擾�Ƀp���b�g�ԍ����g�� */
#define GOF_LZWCLRCOUNT	0x00080000	/* �����N���A�J�E���g���w�肷�� */
#define GOF_LZWNOTUSE	0x00100000	/* LZW���k���s��Ȃ�(�����kGIF) */
#define GOF_BITCOUNT	0x00200000	/* ��������/�ǂݍ��ݎ��̍ŏ��r�b�g�����w�肷�� */

/* GIFANIOPT flags (������) */
#define GAF_LOGICAL		0x00000001	/* �_����ʏ����g�� */
#define GAF_BACKCOLOR	0x00000010	/* �w�i�F�����g�� */
#define GAF_LOOPCOUNT	0x00000400	/* ���[�v�񐔂��w�肷�� */
#define GAF_NOTANI		0x00000800	/* �A�j���[�V�������Ȃ�(�����摜�i�[GIF) */

/* GIFANISCENE flags (������) */
#define GSF_LOGICAL		0x00000001	/* �_����ʏ����g�� */
#define GSF_TRNSCOLOR	0x00000008	/* �����F�����g�� */
#define GSF_INTERLACING	0x00000080	/* �C���^�[���[�X�ɂ��� */
#define GSF_TRNSPALETTE	0x00020000	/* �����F�̎w��Ƀp���b�g�ԍ����g�� */
#define GSF_LZWCLRCOUNT	0x00080000	/* �����N���A�J�E���g���w�肷�� */
#define GSF_LZWNOTUSE	0x00100000	/* LZW���k���s��Ȃ�(�����kGIF) */
#define GSF_BITCOUNT	0x00200000	/* �������ݎ��̍ŏ��r�b�g�����w�肷�� */
#define GSF_DISPOSAL	0x00001000	/* �摜�\����̏������w�肷�� */
#define GSF_USERINPUT	0x00002000	/* ���[�U�̓��͏������󂯕t���� */

/* GIFANISCENE disposal methods */
#define GS_DISP_NONE	0			/* �w�薳��(�f�t�H���g) */
#define GS_DISP_LEAVE	1			/* ���̕\�����c�� */
#define GS_DISP_BACK	2			/* �w�i�F�œh��ׂ� */
#define GS_DISP_PREV	3			/* �ȑO�̕\���ɖ߂� */

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
#define RESZ_SAME	0		/* ���̂܂� */
#define RESZ_RATIO	(-1)	/* �c����𑵂��� */

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