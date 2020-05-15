/*! @file
	@brief ���̎Q�ƈꗗ�ł��E�啶���������ɒ��ӂ��ꂽ��
	���̃t�@�C���� Entity.h �ł��B
	@author	SikigamiHNQ
	@date	2011/00/00
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

typedef struct tagENTITYLIST
{
	UINT	dUniCode;
	CHAR	acCodeA[15];
	TCHAR	atCodeW[15];
	LPTSTR	aptInfo;

} ENTITYLIST, *LPENTITYLIST;

//	HTML�I�Ƀ��o�C���
static CONST ENTITYLIST	gstEttySP[] = {
	{ 0x0022,	("&quot;"),		TEXT("&quot;"),		TEXT("���p��") },	//	34
	{ 0x0026,	("&amp;"),		TEXT("&amp;"),		TEXT("�A���p�T���h") },	//	38
	{ 0x003C,	("&lt;"),		TEXT("&lt;"),		TEXT("�s�����i��菬�j") },	//	60
	{ 0x003E,	("&gt;"),		TEXT("&gt;"),		TEXT("�s�����i����j") },	//	62
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

//	SJIS����\���o���Ȃ��A��
static CONST ENTITYLIST	gstEtyAlpha[] = {
	{ 0x00C1,	("&Aacute;"),	TEXT("&Aacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��A") },	//	193		
	{ 0x00E1,	("&aacute;"),	TEXT("&aacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��A������") },	//	225		
	{ 0x00C2,	("&Acirc;"),	TEXT("&Acirc;"),	TEXT("�T�[�J���t���b�N�X�A�N�Z���g�t��A") },	//	194		
	{ 0x00E2,	("&acirc;"),	TEXT("&acirc;"),	TEXT("�T�[�J���t���b�N�X�A�N�Z���g�t��A������") },	//	226		
	{ 0x00B4,	("&acute;"),	TEXT("&acute;"),	TEXT("�A�N�T���e�M��") },	//	180		
	{ 0x00C6,	("&AElig;"),	TEXT("&AElig;"),	TEXT("AE") },	//	198		
	{ 0x00E6,	("&aelig;"),	TEXT("&aelig;"),	TEXT("AE������") },	//	230		
	{ 0x00C0,	("&Agrave;"),	TEXT("&Agrave;"),	TEXT("�O���[�u�A�N�Z���g�t��A") },	//	192		
	{ 0x00E0,	("&agrave;"),	TEXT("&agrave;"),	TEXT("�O���[�u�A�N�Z���g�t��A������") },	//	224		
	{ 0x2135,	("&alefsym;"),	TEXT("&alefsym;"),	TEXT("�A���t") },	//	8501		
	{ 0x0391,	("&Alpha;"),	TEXT("&Alpha;"),	TEXT("�啶���A���t�@") },	//	913		
	{ 0x03B1,	("&alpha;"),	TEXT("&alpha;"),	TEXT("�������A���t�@") },	//	945		
	{ 0x2227,	("&and;"),		TEXT("&and;"),		TEXT("�y�сi���ځj") },	//	8743
	{ 0x2220,	("&ang;"),		TEXT("&ang;"),		TEXT("�p") },	//	8736
	{ 0x00C5,	("&Aring;"),	TEXT("&Aring;"),	TEXT("�ナ���O�t��A") },	//	197		
	{ 0x00E5,	("&aring;"),	TEXT("&aring;"),	TEXT("�ナ���O�t��A������") },	//	229		
	{ 0x2248,	("&asymp;"),	TEXT("&asymp;"),	TEXT("�ߎ��I�ɓ������A����") },	//	8776		
	{ 0x00C3,	("&Atilde;"),	TEXT("&Atilde;"),	TEXT("�`���h�t��A") },	//	195		
	{ 0x00E3,	("&atilde;"),	TEXT("&atilde;"),	TEXT("�`���h�t��A������") },	//	227		
	{ 0x00C4,	("&Auml;"),		TEXT("&Auml;"),		TEXT("�_�C�G���V�X�t��A") },	//	196
	{ 0x00E4,	("&auml;"),		TEXT("&auml;"),		TEXT("�_�C�G���V�X�t��A������") },	//	228
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyBravo[] = {
	{ 0x201E,	("&bdquo;"),	TEXT("&bdquo;"),	TEXT("���t�_�u�����p��") },	//	8222		
	{ 0x0392,	("&Beta;"),		TEXT("&Beta;"),		TEXT("�啶���x�[�^") },	//	914
	{ 0x03B2,	("&beta;"),		TEXT("&beta;"),		TEXT("�������x�[�^") },	//	946
	{ 0x00A6,	("&brvbar;"),	TEXT("&brvbar;"),	TEXT("�j�f��") },	//	166		
	{ 0x2022,	("&bull;"),		TEXT("&bull;"),		TEXT("����") },	//	8226
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyCharlie[] = {
	{ 0x2229,	("&cap;"),		TEXT("&cap;"),		TEXT("���ʏW��") },	//	8745
	{ 0x00C7,	("&Ccedil;"),	TEXT("&Ccedil;"),	TEXT("�Z�f�B���t��C") },	//	199		
	{ 0x00E7,	("&ccedil;"),	TEXT("&ccedil;"),	TEXT("�Z�f�B���t��C������") },	//	231		
	{ 0x00B8,	("&cedil;"),	TEXT("&cedil;"),	TEXT("�Z�f�B��") },	//	184		
	{ 0x00A2,	("&cent;"),		TEXT("&cent;"),		TEXT("�Z���g�L��") },	//	162
	{ 0x03A7,	("&Chi;"),		TEXT("&Chi;"),		TEXT("�啶���J�C") },	//	935
	{ 0x03C7,	("&chi;"),		TEXT("&chi;"),		TEXT("�������J�C") },	//	967
	{ 0x02C6,	("&circ;"),		TEXT("&circ;"),		TEXT("�T�[�J���t���b�N�X") },	//	710
	{ 0x2663,	("&clubs;"),	TEXT("&clubs;"),	TEXT("�N���u") },	//	9827		
	{ 0x2245,	("&cong;"),		TEXT("&cong;"),		TEXT("���`") },	//	8773
	{ 0x00A9,	("&copy;"),		TEXT("&copy;"),		TEXT("���쌠�\���L��") },	//	169
	{ 0x21B5,	("&crarr;"),	TEXT("&crarr;"),	TEXT("�L�����b�W���^�[��") },	//	8629		
	{ 0x222A,	("&cup;"),		TEXT("&cup;"),		TEXT("�����W��") },	//	8746
	{ 0x00A4,	("&curren;"),	TEXT("&curren;"),	TEXT("�s����ʉ݋L��") },	//	164		
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyDelta[] = {
	{ 0x2020,	("&dagger;"),	TEXT("&dagger;"),	TEXT("�_�K�[") },	//	8224		
	{ 0x2021,	("&Dagger;"),	TEXT("&Dagger;"),	TEXT("�_�u���_�K�[") },	//	8225		
	{ 0x2193,	("&darr;"),		TEXT("&darr;"),		TEXT("�������") },	//	8595
	{ 0x21D3,	("&dArr;"),		TEXT("&dArr;"),		TEXT("������d���") },	//	8659
	{ 0x00B0,	("&deg;"),		TEXT("&deg;"),		TEXT("�x") },	//	176
	{ 0x0394,	("&Delta;"),	TEXT("&Delta;"),	TEXT("�啶���f���^") },	//	916		
	{ 0x03B4,	("&delta;"),	TEXT("&delta;"),	TEXT("�������f���^") },	//	948		
	{ 0x2666,	("&diams;"),	TEXT("&diams;"),	TEXT("�_�C��") },	//	9830		
	{ 0x00F7,	("&divide;"),	TEXT("&divide;"),	TEXT("���Z�L��") },	//	247		
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyEcho[] = {
	{ 0x00C9,	("&Eacute;"),	TEXT("&Eacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��E") },	//	201		
	{ 0x00E9,	("&eacute;"),	TEXT("&eacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��E������") },	//	233		
	{ 0x00CA,	("&Ecirc;"),	TEXT("&Ecirc;"),	TEXT("�T�[�J���t���b�N�X�A�N�Z���g�t��E") },	//	202		
	{ 0x00EA,	("&ecirc;"),	TEXT("&ecirc;"),	TEXT("�T�[�J���t���b�N�X�A�N�Z���g�t��E������") },	//	234		
	{ 0x00C8,	("&Egrave;"),	TEXT("&Egrave;"),	TEXT("�O���[�u�A�N�Z���g�t��E") },	//	200		
	{ 0x00E8,	("&egrave;"),	TEXT("&egrave;"),	TEXT("�O���[�u�A�N�Z���g�t��E������") },	//	232		
	{ 0x2205,	("&empty;"),	TEXT("&empty;"),	TEXT("��W��") },	//	8709		
	{ 0x2003,	("&emsp;"),		TEXT("&emsp;"),		TEXT("m�����̋󔒁i�S�p�󔒁j") },	//	8195
	{ 0x2002,	("&ensp;"),		TEXT("&ensp;"),		TEXT("n�����̋󔒁i���p�󔒁j") },	//	8194
	{ 0x0395,	("&Epsilon;"),	TEXT("&Epsilon;"),	TEXT("�啶���C�v�V����") },	//	917		
	{ 0x03B5,	("&epsilon;"),	TEXT("&epsilon;"),	TEXT("�������C�v�V����") },	//	949		
	{ 0x2261,	("&equiv;"),	TEXT("&equiv;"),	TEXT("��ɓ������A����") },	//	8801		
	{ 0x0397,	("&Eta;"),		TEXT("&Eta;"),		TEXT("�啶���G�[�^") },	//	919
	{ 0x03B7,	("&eta;"),		TEXT("&eta;"),		TEXT("�������G�[�^") },	//	951
	{ 0x00D0,	("&ETH;"),		TEXT("&ETH;"),		TEXT("�A�C�X�����h��ETH") },	//	208
	{ 0x00F0,	("&eth;"),		TEXT("&eth;"),		TEXT("�A�C�X�����h��ETH������") },	//	240
	{ 0x00CB,	("&Euml;"),		TEXT("&Euml;"),		TEXT("�_�C�G���V�X�t��E") },	//	203
	{ 0x00EB,	("&euml;"),		TEXT("&euml;"),		TEXT("�_�C�G���V�X�t��E������") },	//	235
	{ 0x20AC,	("&euro;"),		TEXT("&euro;"),		TEXT("���[���L��") },	//	8364
	{ 0x2203,	("&exist;"),	TEXT("&exist;"),	TEXT("���݂���i���݌���q�j") },	//	8707		
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyFoxtrot[] = {
	{ 0x0192,	("&fnof;"),		TEXT("&fnof;"),		TEXT("������f�i�t�b�N�t���j�A�֐�") },	//	402
	{ 0x2200,	("&forall;"),	TEXT("&forall;"),	TEXT("���ׂẮi���ʌ���q�j") },	//	8704		
	{ 0x00BD,	("&frac12;"),	TEXT("&frac12;"),	TEXT("2����1") },	//	189		
	{ 0x00BC,	("&frac14;"),	TEXT("&frac14;"),	TEXT("4����1") },	//	188		
	{ 0x00BE,	("&frac34;"),	TEXT("&frac34;"),	TEXT("4����3") },	//	190		
	{ 0x2044,	("&frasl;"),	TEXT("&frasl;"),	TEXT("�����̎ΐ�") },	//	8260		
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyGolf[] = {
	{ 0x0393,	("&Gamma;"),	TEXT("&Gamma;"),	TEXT("�啶���K���}") },	//	915		
	{ 0x03B3,	("&gamma;"),	TEXT("&gamma;"),	TEXT("�������K���}") },	//	947		
	{ 0x2265,	("&ge;"),		TEXT("&ge;"),		TEXT("��Ȃ�܂��͓�����") },	//	8805
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyHotel[] = {
	{ 0x2194,	("&harr;"),		TEXT("&harr;"),		TEXT("���E�����") },	//	8596
	{ 0x21D4,	("&hArr;"),		TEXT("&hArr;"),		TEXT("���l") },	//	8660
	{ 0x2665,	("&hearts;"),	TEXT("&hearts;"),	TEXT("�n�[�g") },	//	9829		
	{ 0x2026,	("&hellip;"),	TEXT("&hellip;"),	TEXT("�O�_���[�_") },	//	8230		
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyIndia[] = {
	{ 0x00CD,	("&Iacute;"),	TEXT("&Iacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��I") },	//	205		
	{ 0x00ED,	("&iacute;"),	TEXT("&iacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��I������") },	//	237		
	{ 0x00CE,	("&Icirc;"),	TEXT("&Icirc;"),	TEXT("�T�[�J���t���b�N�X�A�N�Z���g�t��I") },	//	206		
	{ 0x00EE,	("&icirc;"),	TEXT("&icirc;"),	TEXT("�T�[�J���t���b�N�X�A�N�Z���g�t��I������") },	//	238		
	{ 0x00A1,	("&iexcl;"),	TEXT("&iexcl;"),	TEXT("�t���Q��") },	//	161		
	{ 0x00CC,	("&Igrave;"),	TEXT("&Igrave;"),	TEXT("�O���[�u�A�N�Z���g�t��I") },	//	204		
	{ 0x00EC,	("&igrave;"),	TEXT("&igrave;"),	TEXT("�O���[�u�A�N�Z���g�t��I������") },	//	236		
	{ 0x2111,	("&image;"),	TEXT("&image;"),	TEXT("�啶����I�i�u���b�N�́j�A������") },	//	8465		
	{ 0x221E,	("&infin;"),	TEXT("&infin;"),	TEXT("������") },	//	8734		
	{ 0x222B,	("&int;"),		TEXT("&int;"),		TEXT("�ϕ��L��") },	//	8747
	{ 0x0399,	("&Iota;"),		TEXT("&Iota;"),		TEXT("�啶���C�I�^") },	//	921
	{ 0x03B9,	("&iota;"),		TEXT("&iota;"),		TEXT("�������C�I�^") },	//	953
	{ 0x00BF,	("&iquest;"),	TEXT("&iquest;"),	TEXT("�t�^�╄") },	//	191		
	{ 0x2208,	("&isin;"),		TEXT("&isin;"),		TEXT("������") },	//	8712
	{ 0x00CF,	("&Iuml;"),		TEXT("&Iuml;"),		TEXT("�_�C�G���V�X�t��I") },	//	207
	{ 0x00EF,	("&iuml;"),		TEXT("&iuml;"),		TEXT("�_�C�G���V�X�t��I������") },	//	239
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyJuliette[] = {
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyKilo[] = {
	{ 0x039A,	("&Kappa;"),	TEXT("&Kappa;"),	TEXT("�啶���J�b�p") },	//	922		
	{ 0x03BA,	("&kappa;"),	TEXT("&kappa;"),	TEXT("�������J�b�p") },	//	954		
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyLima[] = {
	{ 0x039B,	("&Lambda;"),	TEXT("&Lambda;"),	TEXT("�啶�������_") },	//	923		
	{ 0x03BB,	("&lambda;"),	TEXT("&lambda;"),	TEXT("�����������_") },	//	955		
	{ 0x2329,	("&lang;"),		TEXT("&lang;"),		TEXT("���p����") },	//	9001
	{ 0x00AB,	("&laquo;"),	TEXT("&laquo;"),	TEXT("�n�ߓ�d�R���ʈ��p�L���A�n�߃M����") },	//	171		
	{ 0x2190,	("&larr;"),		TEXT("&larr;"),		TEXT("�������") },	//	8592
	{ 0x21D0,	("&lArr;"),		TEXT("&lArr;"),		TEXT("������d���") },	//	8656
	{ 0x2308,	("&lceil;"),	TEXT("&lceil;"),	TEXT("�����") },	//	8968		
	{ 0x201C,	("&ldquo;"),	TEXT("&ldquo;"),	TEXT("���_�u�����p��") },	//	8220		
	{ 0x2264,	("&le;"),		TEXT("&le;"),		TEXT("���Ȃ�܂��͓�����") },	//	8804
	{ 0x230A,	("&lfloor;"),	TEXT("&lfloor;"),	TEXT("������") },	//	8970		
	{ 0x2217,	("&lowast;"),	TEXT("&lowast;"),	TEXT("�A�X�^���X�N���Z�q") },	//	8727		
	{ 0x25CA,	("&loz;"),		TEXT("&loz;"),		TEXT("�H�`") },	//	9674
	{ 0x200E,	("&lrm;"),		TEXT("&lrm;"),		TEXT("�L�q��������i������E�ցj") },	//	8206
	{ 0x2039,	("&lsaquo;"),	TEXT("&lsaquo;"),	TEXT("���R���ʁi�n�j") },	//	8249		
	{ 0x2018,	("&lsquo;"),	TEXT("&lsquo;"),	TEXT("���V���O�����p��") },	//	8216		
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyMike[] = {
	{ 0x00AF,	("&macr;"),		TEXT("&macr;"),		TEXT("�}�N����") },	//	175
	{ 0x2014,	("&mdash;"),	TEXT("&mdash;"),	TEXT("m�����̃_�b�V���i�S�p�_�b�V���j") },	//	8212		
	{ 0x00B5,	("&micro;"),	TEXT("&micro;"),	TEXT("�}�C�N���L��") },	//	181		
	{ 0x00B7,	("&middot;"),	TEXT("&middot;"),	TEXT("���_�i���e���j") },	//	183		
	{ 0x2212,	("&minus;"),	TEXT("&minus;"),	TEXT("�������A���Z�L��") },	//	8722		
	{ 0x039C,	("&Mu;"),		TEXT("&Mu;"),		TEXT("�啶���~���[") },	//	924
	{ 0x03BC,	("&mu;"),		TEXT("&mu;"),		TEXT("�������~���[") },	//	956
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyNovember[] = {
	{ 0x2207,	("&nabla;"),	TEXT("&nabla;"),	TEXT("�i�u��") },	//	8711		
	{ 0x00A0,	("&nbsp;"),		TEXT("&nbsp;"),		TEXT("�m�[�u���[�N�X�y�[�X") },	//	160
	{ 0x2013,	("&ndash;"),	TEXT("&ndash;"),	TEXT("n�����̃_�b�V���i���p�_�b�V���j") },	//	8211		
	{ 0x2260,	("&ne;"),		TEXT("&ne;"),		TEXT("�����ے�") },	//	8800
	{ 0x220B,	("&ni;"),		TEXT("&ni;"),		TEXT("���Ƃ��Ċ܂�") },	//	8715
	{ 0x00AC,	("&not;"),		TEXT("&not;"),		TEXT("�ے�L��") },	//	172
	{ 0x2209,	("&notin;"),	TEXT("&notin;"),	TEXT("�v�f�̔ے�A���̔ے�") },	//	8713		
	{ 0x2284,	("&nsub;"),		TEXT("&nsub;"),		TEXT("�^�����W���̔ے�") },	//	8836
	{ 0x00D1,	("&Ntilde;"),	TEXT("&Ntilde;"),	TEXT("�`���h�t��N") },	//	209		
	{ 0x00F1,	("&ntilde;"),	TEXT("&ntilde;"),	TEXT("�`���h�t��N������") },	//	241		
	{ 0x039D,	("&Nu;"),		TEXT("&Nu;"),		TEXT("�啶���j���[") },	//	925
	{ 0x03BD,	("&nu;"),		TEXT("&nu;"),		TEXT("�������j���[") },	//	957
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyOscar[] = {
	{ 0x00D3,	("&Oacute;"),	TEXT("&Oacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��O") },	//	211		
	{ 0x00F3,	("&oacute;"),	TEXT("&oacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��O������") },	//	243		
	{ 0x00D4,	("&Ocirc;"),	TEXT("&Ocirc;"),	TEXT("�T�[�J���t���b�N�X�A�N�Z���g�t��O") },	//	212		
	{ 0x00F4,	("&ocirc;"),	TEXT("&ocirc;"),	TEXT("�T�[�J���t���b�N�X�A�N�Z���g�t��O������") },	//	244		
	{ 0x0152,	("&OElig;"),	TEXT("&OElig;"),	TEXT("���K�`��OE�啶���A�~�O�O��L�ꉹ") },	//	338		
	{ 0x0153,	("&oelig;"),	TEXT("&oelig;"),	TEXT("���K�`��OE�������A�~�O�O��L�E���i�ꉹ") },	//	339		
	{ 0x00D2,	("&Ograve;"),	TEXT("&Ograve;"),	TEXT("�O���[�u�A�N�Z���g�t��O") },	//	210		
	{ 0x00F2,	("&ograve;"),	TEXT("&ograve;"),	TEXT("�O���[�u�A�N�Z���g�t��O������") },	//	242		
	{ 0x203E,	("&oline;"),	TEXT("&oline;"),	TEXT("�I�[�o�[���C���A�_���ے�L��") },	//	8254		
	{ 0x03A9,	("&Omega;"),	TEXT("&Omega;"),	TEXT("�啶���I���K") },	//	937		
	{ 0x03C9,	("&omega;"),	TEXT("&omega;"),	TEXT("�������I���K") },	//	969		
	{ 0x039F,	("&Omicron;"),	TEXT("&Omicron;"),	TEXT("�啶���I�~�N����") },	//	927		
	{ 0x03BF,	("&omicron;"),	TEXT("&omicron;"),	TEXT("�������I�~�N����") },	//	959		
	{ 0x2295,	("&oplus;"),	TEXT("&oplus;"),	TEXT("���a") },	//	8853		
	{ 0x2228,	("&or;"),		TEXT("&or;"),		TEXT("���́i���ځj") },	//	8744
	{ 0x00AA,	("&ordf;"),		TEXT("&ordf;"),		TEXT("���������W��") },	//	170
	{ 0x00BA,	("&ordm;"),		TEXT("&ordm;"),		TEXT("�j�������W��") },	//	186
	{ 0x00D8,	("&Oslash;"),	TEXT("&Oslash;"),	TEXT("�X�g���[�N�t��O") },	//	216		
	{ 0x00F8,	("&oslash;"),	TEXT("&oslash;"),	TEXT("�X�g���[�N�t��O������") },	//	248		
	{ 0x00D5,	("&Otilde;"),	TEXT("&Otilde;"),	TEXT("�`���h�t��O") },	//	213		
	{ 0x00F5,	("&otilde;"),	TEXT("&otilde;"),	TEXT("�`���h�t��O������") },	//	245		
	{ 0x2297,	("&otimes;"),	TEXT("&otimes;"),	TEXT("�e���\����") },	//	8855		
	{ 0x00D6,	("&Ouml;"),		TEXT("&Ouml;"),		TEXT("�_�C�G���V�X�t��O") },	//	214
	{ 0x00F6,	("&ouml;"),		TEXT("&ouml;"),		TEXT("�_�C�G���V�X�t��O������") },	//	246
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyPapa[] = {
	{ 0x00B6,	("&para;"),		TEXT("&para;"),		TEXT("�i���L��") },	//	182
	{ 0x2202,	("&part;"),		TEXT("&part;"),		TEXT("�f���A���E���h�f�B�[") },	//	8706
	{ 0x2030,	("&permil;"),	TEXT("&permil;"),	TEXT("�p�[�~��") },	//	8240		
	{ 0x22A5,	("&perp;"),		TEXT("&perp;"),		TEXT("����") },	//	8869
	{ 0x03A6,	("&Phi;"),		TEXT("&Phi;"),		TEXT("�啶���t�@�C") },	//	934
	{ 0x03C6,	("&phi;"),		TEXT("&phi;"),		TEXT("�������t�@�C") },	//	966
	{ 0x03A0,	("&Pi;"),		TEXT("&Pi;"),		TEXT("�啶���p�C") },	//	928
	{ 0x03C0,	("&pi;"),		TEXT("&pi;"),		TEXT("�������p�C") },	//	960
	{ 0x03D6,	("&piv;"),		TEXT("&piv;"),		TEXT("�p�C�E�V���{��") },	//	982
	{ 0x00B1,	("&plusmn;"),	TEXT("&plusmn;"),	TEXT("�����͕�����") },	//	177		
	{ 0x00A3,	("&pound;"),	TEXT("&pound;"),	TEXT("�|���h�L��") },	//	163		
	{ 0x2032,	("&prime;"),	TEXT("&prime;"),	TEXT("��") },	//	8242		
	{ 0x2033,	("&Prime;"),	TEXT("&Prime;"),	TEXT("�b") },	//	8243		
	{ 0x220F,	("&prod;"),		TEXT("&prod;"),		TEXT("n���̏�Z") },	//	8719
	{ 0x221D,	("&prop;"),		TEXT("&prop;"),		TEXT("���") },	//	8733
	{ 0x03A8,	("&Psi;"),		TEXT("&Psi;"),		TEXT("�啶���v�T�C") },	//	936
	{ 0x03C8,	("&psi;"),		TEXT("&psi;"),		TEXT("�������v�T�C") },	//	968
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyQuebec[] = {
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyRomeo[] = {
	{ 0x221A,	("&radic;"),	TEXT("&radic;"),	TEXT("����") },	//	8730		
	{ 0x232A,	("&rang;"),		TEXT("&rang;"),		TEXT("�E�p����") },	//	9002
	{ 0x00BB,	("&raquo;"),	TEXT("&raquo;"),	TEXT("�I����d�R���ʈ��p�L���A�I���M����") },	//	187		
	{ 0x2192,	("&rarr;"),		TEXT("&rarr;"),		TEXT("�E�����") },	//	8594
	{ 0x21D2,	("&rArr;"),		TEXT("&rArr;"),		TEXT("�Ȃ�΁i���Ӂj") },	//	8658
	{ 0x2309,	("&rceil;"),	TEXT("&rceil;"),	TEXT("�E���") },	//	8969		
	{ 0x201D,	("&rdquo;"),	TEXT("&rdquo;"),	TEXT("�E�_�u�����p���i�I�j") },	//	8221		
	{ 0x211C,	("&real;"),		TEXT("&real;"),		TEXT("�啶����R�i�u���b�N�́j�A������") },	//	8476
	{ 0x00AE,	("&reg;"),		TEXT("&reg;"),		TEXT("�o�^���W�L��") },	//	174
	{ 0x230B,	("&rfloor;"),	TEXT("&rfloor;"),	TEXT("�E����") },	//	8971		
	{ 0x03A1,	("&Rho;"),		TEXT("&Rho;"),		TEXT("�啶�����[") },	//	929
	{ 0x03C1,	("&rho;"),		TEXT("&rho;"),		TEXT("���������[") },	//	961
	{ 0x200F,	("&rlm;"),		TEXT("&rlm;"),		TEXT("�L�q��������i�E���獶�ցj") },	//	8207
	{ 0x203A,	("&rsaquo;"),	TEXT("&rsaquo;"),	TEXT("�E�R���ʁi�I�j") },	//	8250		
	{ 0x2019,	("&rsquo;"),	TEXT("&rsquo;"),	TEXT("�E�V���O�����p��") },	//	8217		
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtySierra[] = {
	{ 0x201A,	("&sbquo;"),	TEXT("&sbquo;"),	TEXT("���t���p��") },	//	8218		
	{ 0x0160,	("&Scaron;"),	TEXT("&Scaron;"),	TEXT("�L�������t��S") },	//	352		
	{ 0x0161,	("&scaron;"),	TEXT("&scaron;"),	TEXT("�L�������t��S������") },	//	353		
	{ 0x22C5,	("&sdot;"),		TEXT("&sdot;"),		TEXT("�h�b�g���Z�q") },	//	8901
	{ 0x00A7,	("&sect;"),		TEXT("&sect;"),		TEXT("�ߋL��") },	//	167
	{ 0x00AD,	("&shy;"),		TEXT("&shy;"),		TEXT("�\�t�g�n�C�t��") },	//	173
	{ 0x03A3,	("&Sigma;"),	TEXT("&Sigma;"),	TEXT("�啶���V�O�}") },	//	931		
	{ 0x03C3,	("&sigma;"),	TEXT("&sigma;"),	TEXT("�������V�O�}") },	//	963		
	{ 0x03C2,	("&sigmaf;"),	TEXT("&sigmaf;"),	TEXT("�������t�@�C�i���E�V�O�}") },	//	962		
	{ 0x223C,	("&sim;"),		TEXT("&sim;"),		TEXT("�`���_���Z�q") },	//	8764
	{ 0x2660,	("&spades;"),	TEXT("&spades;"),	TEXT("�X�y�[�h") },	//	9824		
	{ 0x2282,	("&sub;"),		TEXT("&sub;"),		TEXT("�^�����W��") },	//	8834
	{ 0x2286,	("&sube;"),		TEXT("&sube;"),		TEXT("�����W��") },	//	8838
	{ 0x2211,	("&sum;"),		TEXT("&sum;"),		TEXT("n���̉��Z") },	//	8721
	{ 0x2283,	("&sup;"),		TEXT("&sup;"),		TEXT("�^�����W���i�t�����j") },	//	8835
	{ 0x00B9,	("&sup1;"),		TEXT("&sup1;"),		TEXT("��t��1") },	//	185
	{ 0x00B2,	("&sup2;"),		TEXT("&sup2;"),		TEXT("��t��2") },	//	178
	{ 0x00B3,	("&sup3;"),		TEXT("&sup3;"),		TEXT("��t��3") },	//	179
	{ 0x2287,	("&supe;"),		TEXT("&supe;"),		TEXT("�����W���i�t�����j") },	//	8839
	{ 0x00DF,	("&szlig;"),	TEXT("&szlig;"),	TEXT("�h�C�c��G�X�c�F�b�g") },	//	223		
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyTango[] = {
	{ 0x03A4,	("&Tau;"),		TEXT("&Tau;"),		TEXT("�啶���^�E") },	//	932
	{ 0x03C4,	("&tau;"),		TEXT("&tau;"),		TEXT("�������^�E") },	//	964
	{ 0x2234,	("&there4;"),	TEXT("&there4;"),	TEXT("�䂦��") },	//	8756		
	{ 0x0398,	("&Theta;"),	TEXT("&Theta;"),	TEXT("�啶���V�[�^") },	//	920		
	{ 0x03B8,	("&theta;"),	TEXT("&theta;"),	TEXT("�������V�[�^") },	//	952		
	{ 0x03D1,	("&thetasym;"),	TEXT("&thetasym;"),	TEXT("�������V�[�^�E�V���{��") },	//	977		
	{ 0x2009,	("&thinsp;"),	TEXT("&thinsp;"),	TEXT("���܂���") },	//	8201		
	{ 0x00DE,	("&THORN;"),	TEXT("&THORN;"),	TEXT("�A�C�X�����h��THORN") },	//	222		
	{ 0x00FE,	("&thorn;"),	TEXT("&thorn;"),	TEXT("�A�C�X�����h��THORN������") },	//	254		
	{ 0x02DC,	("&tilde;"),	TEXT("&tilde;"),	TEXT("���`���_") },	//	732		
	{ 0x00D7,	("&times;"),	TEXT("&times;"),	TEXT("��Z�L��") },	//	215		
	{ 0x2122,	("&trade;"),	TEXT("&trade;"),	TEXT("�o�^���W�L��") },	//	8482		
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyUniform[] = {
	{ 0x00DA,	("&Uacute;"),	TEXT("&Uacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��U") },	//	218		
	{ 0x00FA,	("&uacute;"),	TEXT("&uacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��U������") },	//	250		
	{ 0x2191,	("&uarr;"),		TEXT("&uarr;"),		TEXT("������") },	//	8593
	{ 0x21D1,	("&uArr;"),		TEXT("&uArr;"),		TEXT("�����d���") },	//	8657
	{ 0x00DB,	("&Ucirc;"),	TEXT("&Ucirc;"),	TEXT("�T�[�J���t���b�N�X�A�N�Z���g�t��U") },	//	219		
	{ 0x00FB,	("&ucirc;"),	TEXT("&ucirc;"),	TEXT("�T�[�J���t���b�N�X�A�N�Z���g�t��U������") },	//	251		
	{ 0x00D9,	("&Ugrave;"),	TEXT("&Ugrave;"),	TEXT("�O���[�u�A�N�Z���g�t��U") },	//	217		
	{ 0x00F9,	("&ugrave;"),	TEXT("&ugrave;"),	TEXT("�O���[�u�A�N�Z���g�t��U������") },	//	249		
	{ 0x00A8,	("&uml;"),		TEXT("&uml;"),		TEXT("�E�����E�g�A�_�C�G���V�X") },	//	168
	{ 0x03D2,	("&upsih;"),	TEXT("&upsih;"),	TEXT("�E�v�V�����i�t�b�N�E�V���{���t���j") },	//	978		
	{ 0x03A5,	("&Upsilon;"),	TEXT("&Upsilon;"),	TEXT("�啶���E�v�V����") },	//	933		
	{ 0x03C5,	("&upsilon;"),	TEXT("&upsilon;"),	TEXT("�������E�v�V����") },	//	965		
	{ 0x00DC,	("&Uuml;"),		TEXT("&Uuml;"),		TEXT("�_�C�G���V�X�t��U") },	//	220
	{ 0x00FC,	("&uuml;"),		TEXT("&uuml;"),		TEXT("�_�C�G���V�X�t��U������") },	//	252
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyVictor[] = {
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyWhiskey[] = {
	{ 0x2118,	("&weierp;"),	TEXT("&weierp;"),	TEXT("�啶����P�i�X�N���v�g�́j�AWeierstrass��P�֐�") },	//	8472		
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyXray[] = {
	{ 0x039E,	("&Xi;"),		TEXT("&Xi;"),		TEXT("�啶���O�U�C") },	//	926
	{ 0x03BE,	("&xi;"),		TEXT("&xi;"),		TEXT("�������O�U�C") },	//	958
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyYankee[] = {
	{ 0x00DD,	("&Yacute;"),	TEXT("&Yacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��Y") },	//	221		
	{ 0x00FD,	("&yacute;"),	TEXT("&yacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��Y������") },	//	253		
	{ 0x00A5,	("&yen;"),		TEXT("&yen;"),		TEXT("�~�L��") },	//	165
	{ 0x00FF,	("&yuml;"),		TEXT("&yuml;"),		TEXT("�_�C�G���V�X�t��Y������") },	//	255
	{ 0x0178,	("&Yuml;"),		TEXT("&Yuml;"),		TEXT("�_�C�G���V�X�t��Y") },	//	376
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	gstEtyZulu[] = {
	{ 0x0396,	("&Zeta;"),		TEXT("&Zeta;"),		TEXT("�啶���[�[�^") },	//	918
	{ 0x03B6,	("&zeta;"),		TEXT("&zeta;"),		TEXT("�������[�[�^") },	//	950
	{ 0x200D,	("&zwj;"),		TEXT("&zwj;"),		TEXT("���Ȃ��ڑ��q") },	//	8205
	{ 0x200C,	("&zwnj;"),		TEXT("&zwnj;"),		TEXT("���Ȃ���ڑ��q") },	//	8204
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("���[�_�~�[") }
};

static CONST ENTITYLIST	*gpstEntitys[] = { gstEtyAlpha, gstEtyBravo,
	gstEtyCharlie, gstEtyDelta, gstEtyEcho, gstEtyFoxtrot, gstEtyGolf,
	gstEtyHotel, gstEtyIndia, gstEtyJuliette, gstEtyKilo, gstEtyLima,
	gstEtyMike, gstEtyNovember, gstEtyOscar, gstEtyPapa, gstEtyQuebec,
	gstEtyRomeo, gstEtySierra, gstEtyTango, gstEtyUniform, gstEtyVictor,
	gstEtyWhiskey, gstEtyXray, gstEtyYankee, gstEtyZulu };

/*

	{ 0x00A0,	("&nbsp;"),		TEXT("&nbsp;"),		TEXT("�m�[�u���[�N�X�y�[�X") },	//	160
	{ 0x00A1,	("&iexcl;"),	TEXT("&iexcl;"),	TEXT("�t���Q��") },	//	161
	{ 0x00A2,	("&cent;"),		TEXT("&cent;"),		TEXT("�Z���g�L��") },	//	162
	{ 0x00A3,	("&pound;"),	TEXT("&pound;"),	TEXT("�|���h�L��") },	//	163
	{ 0x00A4,	("&curren;"),	TEXT("&curren;"),	TEXT("�s����ʉ݋L��") },	//	164
	{ 0x00A5,	("&yen;"),		TEXT("&yen;"),		TEXT("�~�L��") },	//	165
	{ 0x00A6,	("&brvbar;"),	TEXT("&brvbar;"),	TEXT("�j�f��") },	//	166
	{ 0x00A7,	("&sect;"),		TEXT("&sect;"),		TEXT("�ߋL��") },	//	167
	{ 0x00A8,	("&uml;"),		TEXT("&uml;"),		TEXT("�E�����E�g�A�_�C�G���V�X") },	//	168
	{ 0x00A9,	("&copy;"),		TEXT("&copy;"),		TEXT("���쌠�\���L��") },	//	169
	{ 0x00AA,	("&ordf;"),		TEXT("&ordf;"),		TEXT("���������W��") },	//	170
	{ 0x00AB,	("&laquo;"),	TEXT("&laquo;"),	TEXT("�n�ߓ�d�R���ʈ��p�L���A�n�߃M����") },	//	171
	{ 0x00AC,	("&not;"),		TEXT("&not;"),		TEXT("�ے�L��") },	//	172
	{ 0x00AD,	("&shy;"),		TEXT("&shy;"),		TEXT("�\�t�g�n�C�t��") },	//	173
	{ 0x00AE,	("&reg;"),		TEXT("&reg;"),		TEXT("�o�^���W�L��") },	//	174
	{ 0x00AF,	("&macr;"),		TEXT("&macr;"),		TEXT("�}�N����") },	//	175
	{ 0x00B0,	("&deg;"),		TEXT("&deg;"),		TEXT("�x") },	//	176
	{ 0x00B1,	("&plusmn;"),	TEXT("&plusmn;"),	TEXT("�����͕�����") },	//	177
	{ 0x00B2,	("&sup2;"),		TEXT("&sup2;"),		TEXT("��t��2") },	//	178
	{ 0x00B3,	("&sup3;"),		TEXT("&sup3;"),		TEXT("��t��3") },	//	179
	{ 0x00B4,	("&acute;"),	TEXT("&acute;"),	TEXT("�A�N�T���e�M��") },	//	180
	{ 0x00B5,	("&micro;"),	TEXT("&micro;"),	TEXT("�}�C�N���L��") },	//	181
	{ 0x00B6,	("&para;"),		TEXT("&para;"),		TEXT("�i���L��") },	//	182
	{ 0x00B7,	("&middot;"),	TEXT("&middot;"),	TEXT("���_�i���e���j") },	//	183
	{ 0x00B8,	("&cedil;"),	TEXT("&cedil;"),	TEXT("�Z�f�B��") },	//	184
	{ 0x00B9,	("&sup1;"),		TEXT("&sup1;"),		TEXT("��t��1") },	//	185
	{ 0x00BA,	("&ordm;"),		TEXT("&ordm;"),		TEXT("�j�������W��") },	//	186
	{ 0x00BB,	("&raquo;"),	TEXT("&raquo;"),	TEXT("�I����d�R���ʈ��p�L���A�I���M����") },	//	187
	{ 0x00BC,	("&frac14;"),	TEXT("&frac14;"),	TEXT("4����1") },	//	188
	{ 0x00BD,	("&frac12;"),	TEXT("&frac12;"),	TEXT("2����1") },	//	189
	{ 0x00BE,	("&frac34;"),	TEXT("&frac34;"),	TEXT("4����3") },	//	190
	{ 0x00BF,	("&iquest;"),	TEXT("&iquest;"),	TEXT("�t�^�╄") },	//	191
	{ 0x00C0,	("&Agrave;"),	TEXT("&Agrave;"),	TEXT("�O���[�u�A�N�Z���g�t��A") },	//	192
	{ 0x00C1,	("&Aacute;"),	TEXT("&Aacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��A") },	//	193
	{ 0x00C2,	("&Acirc;"),	TEXT("&Acirc;"),	TEXT("�T�[�J���t���b�N�X�A�N�Z���g�t��A") },	//	194
	{ 0x00C3,	("&Atilde;"),	TEXT("&Atilde;"),	TEXT("�`���h�t��A") },	//	195
	{ 0x00C4,	("&Auml;"),		TEXT("&Auml;"),		TEXT("�_�C�G���V�X�t��A") },	//	196
	{ 0x00C5,	("&Aring;"),	TEXT("&Aring;"),	TEXT("�ナ���O�t��A") },	//	197
	{ 0x00C6,	("&AElig;"),	TEXT("&AElig;"),	TEXT("AE") },	//	198
	{ 0x00C7,	("&Ccedil;"),	TEXT("&Ccedil;"),	TEXT("�Z�f�B���t��C") },	//	199
	{ 0x00C8,	("&Egrave;"),	TEXT("&Egrave;"),	TEXT("�O���[�u�A�N�Z���g�t��E") },	//	200
	{ 0x00C9,	("&Eacute;"),	TEXT("&Eacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��E") },	//	201
	{ 0x00CA,	("&Ecirc;"),	TEXT("&Ecirc;"),	TEXT("�T�[�J���t���b�N�X�A�N�Z���g�t��E") },	//	202
	{ 0x00CB,	("&Euml;"),		TEXT("&Euml;"),		TEXT("�_�C�G���V�X�t��E") },	//	203
	{ 0x00CC,	("&Igrave;"),	TEXT("&Igrave;"),	TEXT("�O���[�u�A�N�Z���g�t��I") },	//	204
	{ 0x00CD,	("&Iacute;"),	TEXT("&Iacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��I") },	//	205
	{ 0x00CE,	("&Icirc;"),	TEXT("&Icirc;"),	TEXT("�T�[�J���t���b�N�X�A�N�Z���g�t��I") },	//	206
	{ 0x00CF,	("&Iuml;"),		TEXT("&Iuml;"),		TEXT("�_�C�G���V�X�t��I") },	//	207
	{ 0x00D0,	("&ETH;"),		TEXT("&ETH;"),		TEXT("�A�C�X�����h��ETH") },	//	208
	{ 0x00D1,	("&Ntilde;"),	TEXT("&Ntilde;"),	TEXT("�`���h�t��N") },	//	209
	{ 0x00D2,	("&Ograve;"),	TEXT("&Ograve;"),	TEXT("�O���[�u�A�N�Z���g�t��O") },	//	210
	{ 0x00D3,	("&Oacute;"),	TEXT("&Oacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��O") },	//	211
	{ 0x00D4,	("&Ocirc;"),	TEXT("&Ocirc;"),	TEXT("�T�[�J���t���b�N�X�A�N�Z���g�t��O") },	//	212
	{ 0x00D5,	("&Otilde;"),	TEXT("&Otilde;"),	TEXT("�`���h�t��O") },	//	213
	{ 0x00D6,	("&Ouml;"),		TEXT("&Ouml;"),		TEXT("�_�C�G���V�X�t��O") },	//	214
	{ 0x00D7,	("&times;"),	TEXT("&times;"),	TEXT("��Z�L��") },	//	215
	{ 0x00D8,	("&Oslash;"),	TEXT("&Oslash;"),	TEXT("�X�g���[�N�t��O") },	//	216
	{ 0x00D9,	("&Ugrave;"),	TEXT("&Ugrave;"),	TEXT("�O���[�u�A�N�Z���g�t��U") },	//	217
	{ 0x00DA,	("&Uacute;"),	TEXT("&Uacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��U") },	//	218
	{ 0x00DB,	("&Ucirc;"),	TEXT("&Ucirc;"),	TEXT("�T�[�J���t���b�N�X�A�N�Z���g�t��U") },	//	219
	{ 0x00DC,	("&Uuml;"),		TEXT("&Uuml;"),		TEXT("�_�C�G���V�X�t��U") },	//	220
	{ 0x00DD,	("&Yacute;"),	TEXT("&Yacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��Y") },	//	221
	{ 0x00DE,	("&THORN;"),	TEXT("&THORN;"),	TEXT("�A�C�X�����h��THORN") },	//	222
	{ 0x00DF,	("&szlig;"),	TEXT("&szlig;"),	TEXT("�h�C�c��G�X�c�F�b�g") },	//	223
	{ 0x00E0,	("&agrave;"),	TEXT("&agrave;"),	TEXT("�O���[�u�A�N�Z���g�t��A������") },	//	224
	{ 0x00E1,	("&aacute;"),	TEXT("&aacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��A������") },	//	225
	{ 0x00E2,	("&acirc;"),	TEXT("&acirc;"),	TEXT("�T�[�J���t���b�N�X�A�N�Z���g�t��A������") },	//	226
	{ 0x00E3,	("&atilde;"),	TEXT("&atilde;"),	TEXT("�`���h�t��A������") },	//	227
	{ 0x00E4,	("&auml;"),		TEXT("&auml;"),		TEXT("�_�C�G���V�X�t��A������") },	//	228
	{ 0x00E5,	("&aring;"),	TEXT("&aring;"),	TEXT("�ナ���O�t��A������") },	//	229
	{ 0x00E6,	("&aelig;"),	TEXT("&aelig;"),	TEXT("AE������") },	//	230
	{ 0x00E7,	("&ccedil;"),	TEXT("&ccedil;"),	TEXT("�Z�f�B���t��C������") },	//	231
	{ 0x00E8,	("&egrave;"),	TEXT("&egrave;"),	TEXT("�O���[�u�A�N�Z���g�t��E������") },	//	232
	{ 0x00E9,	("&eacute;"),	TEXT("&eacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��E������") },	//	233
	{ 0x00EA,	("&ecirc;"),	TEXT("&ecirc;"),	TEXT("�T�[�J���t���b�N�X�A�N�Z���g�t��E������") },	//	234
	{ 0x00EB,	("&euml;"),		TEXT("&euml;"),		TEXT("�_�C�G���V�X�t��E������") },	//	235
	{ 0x00EC,	("&igrave;"),	TEXT("&igrave;"),	TEXT("�O���[�u�A�N�Z���g�t��I������") },	//	236
	{ 0x00ED,	("&iacute;"),	TEXT("&iacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��I������") },	//	237
	{ 0x00EE,	("&icirc;"),	TEXT("&icirc;"),	TEXT("�T�[�J���t���b�N�X�A�N�Z���g�t��I������") },	//	238
	{ 0x00EF,	("&iuml;"),		TEXT("&iuml;"),		TEXT("�_�C�G���V�X�t��I������") },	//	239
	{ 0x00F0,	("&eth;"),		TEXT("&eth;"),		TEXT("�A�C�X�����h��ETH������") },	//	240
	{ 0x00F1,	("&ntilde;"),	TEXT("&ntilde;"),	TEXT("�`���h�t��N������") },	//	241
	{ 0x00F2,	("&ograve;"),	TEXT("&ograve;"),	TEXT("�O���[�u�A�N�Z���g�t��O������") },	//	242
	{ 0x00F3,	("&oacute;"),	TEXT("&oacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��O������") },	//	243
	{ 0x00F4,	("&ocirc;"),	TEXT("&ocirc;"),	TEXT("�T�[�J���t���b�N�X�A�N�Z���g�t��O������") },	//	244
	{ 0x00F5,	("&otilde;"),	TEXT("&otilde;"),	TEXT("�`���h�t��O������") },	//	245
	{ 0x00F6,	("&ouml;"),		TEXT("&ouml;"),		TEXT("�_�C�G���V�X�t��O������") },	//	246
	{ 0x00F7,	("&divide;"),	TEXT("&divide;"),	TEXT("���Z�L��") },	//	247
	{ 0x00F8,	("&oslash;"),	TEXT("&oslash;"),	TEXT("�X�g���[�N�t��O������") },	//	248
	{ 0x00F9,	("&ugrave;"),	TEXT("&ugrave;"),	TEXT("�O���[�u�A�N�Z���g�t��U������") },	//	249
	{ 0x00FA,	("&uacute;"),	TEXT("&uacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��U������") },	//	250
	{ 0x00FB,	("&ucirc;"),	TEXT("&ucirc;"),	TEXT("�T�[�J���t���b�N�X�A�N�Z���g�t��U������") },	//	251
	{ 0x00FC,	("&uuml;"),		TEXT("&uuml;"),		TEXT("�_�C�G���V�X�t��U������") },	//	252
	{ 0x00FD,	("&yacute;"),	TEXT("&yacute;"),	TEXT("�A�L���[�g�A�N�Z���g�t��Y������") },	//	253
	{ 0x00FE,	("&thorn;"),	TEXT("&thorn;"),	TEXT("�A�C�X�����h��THORN������") },	//	254
	{ 0x00FF,	("&yuml;"),		TEXT("&yuml;"),		TEXT("�_�C�G���V�X�t��Y������") },	//	255
	{ 0x0152,	("&OElig;"),	TEXT("&OElig;"),	TEXT("���K�`��OE�啶���A�~�O�O��L�ꉹ") },	//	338
	{ 0x0153,	("&oelig;"),	TEXT("&oelig;"),	TEXT("���K�`��OE�������A�~�O�O��L�E���i�ꉹ") },	//	339
	{ 0x0160,	("&Scaron;"),	TEXT("&Scaron;"),	TEXT("�L�������t��S") },	//	352
	{ 0x0161,	("&scaron;"),	TEXT("&scaron;"),	TEXT("�L�������t��S������") },	//	353
	{ 0x0178,	("&Yuml;"),		TEXT("&Yuml;"),		TEXT("�_�C�G���V�X�t��Y") },	//	376
	{ 0x0192,	("&fnof;"),		TEXT("&fnof;"),		TEXT("������f�i�t�b�N�t���j�A�֐�") },	//	402
	{ 0x02C6,	("&circ;"),		TEXT("&circ;"),		TEXT("�T�[�J���t���b�N�X") },	//	710
	{ 0x02DC,	("&tilde;"),	TEXT("&tilde;"),	TEXT("���`���_") },	//	732
	{ 0x0391,	("&Alpha;"),	TEXT("&Alpha;"),	TEXT("�啶���A���t�@") },	//	913
	{ 0x0392,	("&Beta;"),		TEXT("&Beta;"),		TEXT("�啶���x�[�^") },	//	914
	{ 0x0393,	("&Gamma;"),	TEXT("&Gamma;"),	TEXT("�啶���K���}") },	//	915
	{ 0x0394,	("&Delta;"),	TEXT("&Delta;"),	TEXT("�啶���f���^") },	//	916
	{ 0x0395,	("&Epsilon;"),	TEXT("&Epsilon;"),	TEXT("�啶���C�v�V����") },	//	917
	{ 0x0396,	("&Zeta;"),		TEXT("&Zeta;"),		TEXT("�啶���[�[�^") },	//	918
	{ 0x0397,	("&Eta;"),		TEXT("&Eta;"),		TEXT("�啶���G�[�^") },	//	919
	{ 0x0398,	("&Theta;"),	TEXT("&Theta;"),	TEXT("�啶���V�[�^") },	//	920
	{ 0x0399,	("&Iota;"),		TEXT("&Iota;"),		TEXT("�啶���C�I�^") },	//	921
	{ 0x039A,	("&Kappa;"),	TEXT("&Kappa;"),	TEXT("�啶���J�b�p") },	//	922
	{ 0x039B,	("&Lambda;"),	TEXT("&Lambda;"),	TEXT("�啶�������_") },	//	923
	{ 0x039C,	("&Mu;"),		TEXT("&Mu;"),		TEXT("�啶���~���[") },	//	924
	{ 0x039D,	("&Nu;"),		TEXT("&Nu;"),		TEXT("�啶���j���[") },	//	925
	{ 0x039E,	("&Xi;"),		TEXT("&Xi;"),		TEXT("�啶���O�U�C") },	//	926
	{ 0x039F,	("&Omicron;"),	TEXT("&Omicron;"),	TEXT("�啶���I�~�N����") },	//	927
	{ 0x03A0,	("&Pi;"),		TEXT("&Pi;"),		TEXT("�啶���p�C") },	//	928
	{ 0x03A1,	("&Rho;"),		TEXT("&Rho;"),		TEXT("�啶�����[") },	//	929
	{ 0x03A3,	("&Sigma;"),	TEXT("&Sigma;"),	TEXT("�啶���V�O�}") },	//	931
	{ 0x03A4,	("&Tau;"),		TEXT("&Tau;"),		TEXT("�啶���^�E") },	//	932
	{ 0x03A5,	("&Upsilon;"),	TEXT("&Upsilon;"),	TEXT("�啶���E�v�V����") },	//	933
	{ 0x03A6,	("&Phi;"),		TEXT("&Phi;"),		TEXT("�啶���t�@�C") },	//	934
	{ 0x03A7,	("&Chi;"),		TEXT("&Chi;"),		TEXT("�啶���J�C") },	//	935
	{ 0x03A8,	("&Psi;"),		TEXT("&Psi;"),		TEXT("�啶���v�T�C") },	//	936
	{ 0x03A9,	("&Omega;"),	TEXT("&Omega;"),	TEXT("�啶���I���K") },	//	937
	{ 0x03B1,	("&alpha;"),	TEXT("&alpha;"),	TEXT("�������A���t�@") },	//	945
	{ 0x03B2,	("&beta;"),		TEXT("&beta;"),		TEXT("�������x�[�^") },	//	946
	{ 0x03B3,	("&gamma;"),	TEXT("&gamma;"),	TEXT("�������K���}") },	//	947
	{ 0x03B4,	("&delta;"),	TEXT("&delta;"),	TEXT("�������f���^") },	//	948
	{ 0x03B5,	("&epsilon;"),	TEXT("&epsilon;"),	TEXT("�������C�v�V����") },	//	949
	{ 0x03B6,	("&zeta;"),		TEXT("&zeta;"),		TEXT("�������[�[�^") },	//	950
	{ 0x03B7,	("&eta;"),		TEXT("&eta;"),		TEXT("�������G�[�^") },	//	951
	{ 0x03B8,	("&theta;"),	TEXT("&theta;"),	TEXT("�������V�[�^") },	//	952
	{ 0x03B9,	("&iota;"),		TEXT("&iota;"),		TEXT("�������C�I�^") },	//	953
	{ 0x03BA,	("&kappa;"),	TEXT("&kappa;"),	TEXT("�������J�b�p") },	//	954
	{ 0x03BB,	("&lambda;"),	TEXT("&lambda;"),	TEXT("�����������_") },	//	955
	{ 0x03BC,	("&mu;"),		TEXT("&mu;"),		TEXT("�������~���[") },	//	956
	{ 0x03BD,	("&nu;"),		TEXT("&nu;"),		TEXT("�������j���[") },	//	957
	{ 0x03BE,	("&xi;"),		TEXT("&xi;"),		TEXT("�������O�U�C") },	//	958
	{ 0x03BF,	("&omicron;"),	TEXT("&omicron;"),	TEXT("�������I�~�N����") },	//	959
	{ 0x03C0,	("&pi;"),		TEXT("&pi;"),		TEXT("�������p�C") },	//	960
	{ 0x03C1,	("&rho;"),		TEXT("&rho;"),		TEXT("���������[") },	//	961
	{ 0x03C2,	("&sigmaf;"),	TEXT("&sigmaf;"),	TEXT("�������t�@�C�i���E�V�O�}") },	//	962
	{ 0x03C3,	("&sigma;"),	TEXT("&sigma;"),	TEXT("�������V�O�}") },	//	963
	{ 0x03C4,	("&tau;"),		TEXT("&tau;"),		TEXT("�������^�E") },	//	964
	{ 0x03C5,	("&upsilon;"),	TEXT("&upsilon;"),	TEXT("�������E�v�V����") },	//	965
	{ 0x03C6,	("&phi;"),		TEXT("&phi;"),		TEXT("�������t�@�C") },	//	966
	{ 0x03C7,	("&chi;"),		TEXT("&chi;"),		TEXT("�������J�C") },	//	967
	{ 0x03C8,	("&psi;"),		TEXT("&psi;"),		TEXT("�������v�T�C") },	//	968
	{ 0x03C9,	("&omega;"),	TEXT("&omega;"),	TEXT("�������I���K") },	//	969
	{ 0x03D1,	("&thetasym;"),	TEXT("&thetasym;"),	TEXT("�������V�[�^�E�V���{��") },	//	977
	{ 0x03D2,	("&upsih;"),	TEXT("&upsih;"),	TEXT("�E�v�V�����i�t�b�N�E�V���{���t���j") },	//	978
	{ 0x03D6,	("&piv;"),		TEXT("&piv;"),		TEXT("�p�C�E�V���{��") },	//	982
	{ 0x2002,	("&ensp;"),		TEXT("&ensp;"),		TEXT("n�����̋󔒁i���p�󔒁j") },	//	8194
	{ 0x2003,	("&emsp;"),		TEXT("&emsp;"),		TEXT("m�����̋󔒁i�S�p�󔒁j") },	//	8195
	{ 0x2009,	("&thinsp;"),	TEXT("&thinsp;"),	TEXT("���܂���") },	//	8201
	{ 0x200C,	("&zwnj;"),		TEXT("&zwnj;"),		TEXT("���Ȃ���ڑ��q") },	//	8204
	{ 0x200D,	("&zwj;"),		TEXT("&zwj;"),		TEXT("���Ȃ��ڑ��q") },	//	8205
	{ 0x200E,	("&lrm;"),		TEXT("&lrm;"),		TEXT("�L�q��������i������E�ցj") },	//	8206
	{ 0x200F,	("&rlm;"),		TEXT("&rlm;"),		TEXT("�L�q��������i�E���獶�ցj") },	//	8207
	{ 0x2013,	("&ndash;"),	TEXT("&ndash;"),	TEXT("n�����̃_�b�V���i���p�_�b�V���j") },	//	8211
	{ 0x2014,	("&mdash;"),	TEXT("&mdash;"),	TEXT("m�����̃_�b�V���i�S�p�_�b�V���j") },	//	8212
	{ 0x2018,	("&lsquo;"),	TEXT("&lsquo;"),	TEXT("���V���O�����p��") },	//	8216
	{ 0x2019,	("&rsquo;"),	TEXT("&rsquo;"),	TEXT("�E�V���O�����p��") },	//	8217
	{ 0x201A,	("&sbquo;"),	TEXT("&sbquo;"),	TEXT("���t���p��") },	//	8218
	{ 0x201C,	("&ldquo;"),	TEXT("&ldquo;"),	TEXT("���_�u�����p��") },	//	8220
	{ 0x201D,	("&rdquo;"),	TEXT("&rdquo;"),	TEXT("�E�_�u�����p���i�I�j") },	//	8221
	{ 0x201E,	("&bdquo;"),	TEXT("&bdquo;"),	TEXT("���t�_�u�����p��") },	//	8222
	{ 0x2020,	("&dagger;"),	TEXT("&dagger;"),	TEXT("�_�K�[") },	//	8224
	{ 0x2021,	("&Dagger;"),	TEXT("&Dagger;"),	TEXT("�_�u���_�K�[") },	//	8225
	{ 0x2022,	("&bull;"),		TEXT("&bull;"),		TEXT("����") },	//	8226
	{ 0x2026,	("&hellip;"),	TEXT("&hellip;"),	TEXT("�O�_���[�_") },	//	8230
	{ 0x2030,	("&permil;"),	TEXT("&permil;"),	TEXT("�p�[�~��") },	//	8240
	{ 0x2032,	("&prime;"),	TEXT("&prime;"),	TEXT("��") },	//	8242
	{ 0x2033,	("&Prime;"),	TEXT("&Prime;"),	TEXT("�b") },	//	8243
	{ 0x2039,	("&lsaquo;"),	TEXT("&lsaquo;"),	TEXT("���R���ʁi�n�j") },	//	8249
	{ 0x203A,	("&rsaquo;"),	TEXT("&rsaquo;"),	TEXT("�E�R���ʁi�I�j") },	//	8250
	{ 0x203E,	("&oline;"),	TEXT("&oline;"),	TEXT("�I�[�o�[���C���A�_���ے�L��") },	//	8254
	{ 0x2044,	("&frasl;"),	TEXT("&frasl;"),	TEXT("�����̎ΐ�") },	//	8260
	{ 0x20AC,	("&euro;"),		TEXT("&euro;"),		TEXT("���[���L��") },	//	8364
	{ 0x2118,	("&weierp;"),	TEXT("&weierp;"),	TEXT("�啶����P�i�X�N���v�g�́j�AWeierstrass��P�֐�") },	//	8472
	{ 0x2111,	("&image;"),	TEXT("&image;"),	TEXT("�啶����I�i�u���b�N�́j�A������") },	//	8465
	{ 0x211C,	("&real;"),		TEXT("&real;"),		TEXT("�啶����R�i�u���b�N�́j�A������") },	//	8476
	{ 0x2122,	("&trade;"),	TEXT("&trade;"),	TEXT("�o�^���W�L��") },	//	8482
	{ 0x2135,	("&alefsym;"),	TEXT("&alefsym;"),	TEXT("�A���t") },	//	8501
	{ 0x2190,	("&larr;"),		TEXT("&larr;"),		TEXT("�������") },	//	8592
	{ 0x2191,	("&uarr;"),		TEXT("&uarr;"),		TEXT("������") },	//	8593
	{ 0x2192,	("&rarr;"),		TEXT("&rarr;"),		TEXT("�E�����") },	//	8594
	{ 0x2193,	("&darr;"),		TEXT("&darr;"),		TEXT("�������") },	//	8595
	{ 0x2194,	("&harr;"),		TEXT("&harr;"),		TEXT("���E�����") },	//	8596
	{ 0x21B5,	("&crarr;"),	TEXT("&crarr;"),	TEXT("�L�����b�W���^�[��") },	//	8629
	{ 0x21D0,	("&lArr;"),		TEXT("&lArr;"),		TEXT("������d���") },	//	8656
	{ 0x21D1,	("&uArr;"),		TEXT("&uArr;"),		TEXT("�����d���") },	//	8657
	{ 0x21D2,	("&rArr;"),		TEXT("&rArr;"),		TEXT("�Ȃ�΁i���Ӂj") },	//	8658
	{ 0x21D3,	("&dArr;"),		TEXT("&dArr;"),		TEXT("������d���") },	//	8659
	{ 0x21D4,	("&hArr;"),		TEXT("&hArr;"),		TEXT("���l") },	//	8660
	{ 0x2200,	("&forall;"),	TEXT("&forall;"),	TEXT("���ׂẮi���ʌ���q�j") },	//	8704
	{ 0x2202,	("&part;"),		TEXT("&part;"),		TEXT("�f���A���E���h�f�B�[") },	//	8706
	{ 0x2203,	("&exist;"),	TEXT("&exist;"),	TEXT("���݂���i���݌���q�j") },	//	8707
	{ 0x2205,	("&empty;"),	TEXT("&empty;"),	TEXT("��W��") },	//	8709
	{ 0x2207,	("&nabla;"),	TEXT("&nabla;"),	TEXT("�i�u��") },	//	8711
	{ 0x2208,	("&isin;"),		TEXT("&isin;"),		TEXT("������") },	//	8712
	{ 0x2209,	("&notin;"),	TEXT("&notin;"),	TEXT("�v�f�̔ے�A���̔ے�") },	//	8713
	{ 0x220B,	("&ni;"),		TEXT("&ni;"),		TEXT("���Ƃ��Ċ܂�") },	//	8715
	{ 0x220F,	("&prod;"),		TEXT("&prod;"),		TEXT("n���̏�Z") },	//	8719
	{ 0x2211,	("&sum;"),		TEXT("&sum;"),		TEXT("n���̉��Z") },	//	8721
	{ 0x2212,	("&minus;"),	TEXT("&minus;"),	TEXT("�������A���Z�L��") },	//	8722
	{ 0x2217,	("&lowast;"),	TEXT("&lowast;"),	TEXT("�A�X�^���X�N���Z�q") },	//	8727
	{ 0x221A,	("&radic;"),	TEXT("&radic;"),	TEXT("����") },	//	8730
	{ 0x221D,	("&prop;"),		TEXT("&prop;"),		TEXT("���") },	//	8733
	{ 0x221E,	("&infin;"),	TEXT("&infin;"),	TEXT("������") },	//	8734
	{ 0x2220,	("&ang;"),		TEXT("&ang;"),		TEXT("�p") },	//	8736
	{ 0x2227,	("&and;"),		TEXT("&and;"),		TEXT("�y�сi���ځj") },	//	8743
	{ 0x2228,	("&or;"),		TEXT("&or;"),		TEXT("���́i���ځj") },	//	8744
	{ 0x2229,	("&cap;"),		TEXT("&cap;"),		TEXT("���ʏW��") },	//	8745
	{ 0x222A,	("&cup;"),		TEXT("&cup;"),		TEXT("�����W��") },	//	8746
	{ 0x222B,	("&int;"),		TEXT("&int;"),		TEXT("�ϕ��L��") },	//	8747
	{ 0x2234,	("&there4;"),	TEXT("&there4;"),	TEXT("�䂦��") },	//	8756
	{ 0x223C,	("&sim;"),		TEXT("&sim;"),		TEXT("�`���_���Z�q") },	//	8764
	{ 0x2245,	("&cong;"),		TEXT("&cong;"),		TEXT("���`") },	//	8773
	{ 0x2248,	("&asymp;"),	TEXT("&asymp;"),	TEXT("�ߎ��I�ɓ������A����") },	//	8776
	{ 0x2260,	("&ne;"),		TEXT("&ne;"),		TEXT("�����ے�") },	//	8800
	{ 0x2261,	("&equiv;"),	TEXT("&equiv;"),	TEXT("��ɓ������A����") },	//	8801
	{ 0x2264,	("&le;"),		TEXT("&le;"),		TEXT("���Ȃ�܂��͓�����") },	//	8804
	{ 0x2265,	("&ge;"),		TEXT("&ge;"),		TEXT("��Ȃ�܂��͓�����") },	//	8805
	{ 0x2282,	("&sub;"),		TEXT("&sub;"),		TEXT("�^�����W��") },	//	8834
	{ 0x2283,	("&sup;"),		TEXT("&sup;"),		TEXT("�^�����W���i�t�����j") },	//	8835
	{ 0x2284,	("&nsub;"),		TEXT("&nsub;"),		TEXT("�^�����W���̔ے�") },	//	8836
	{ 0x2286,	("&sube;"),		TEXT("&sube;"),		TEXT("�����W��") },	//	8838
	{ 0x2287,	("&supe;"),		TEXT("&supe;"),		TEXT("�����W���i�t�����j") },	//	8839
	{ 0x2295,	("&oplus;"),	TEXT("&oplus;"),	TEXT("���a") },	//	8853
	{ 0x2297,	("&otimes;"),	TEXT("&otimes;"),	TEXT("�e���\����") },	//	8855
	{ 0x22A5,	("&perp;"),		TEXT("&perp;"),		TEXT("����") },	//	8869
	{ 0x22C5,	("&sdot;"),		TEXT("&sdot;"),		TEXT("�h�b�g���Z�q") },	//	8901
	{ 0x2308,	("&lceil;"),	TEXT("&lceil;"),	TEXT("�����") },	//	8968
	{ 0x2309,	("&rceil;"),	TEXT("&rceil;"),	TEXT("�E���") },	//	8969
	{ 0x230A,	("&lfloor;"),	TEXT("&lfloor;"),	TEXT("������") },	//	8970
	{ 0x230B,	("&rfloor;"),	TEXT("&rfloor;"),	TEXT("�E����") },	//	8971
	{ 0x2329,	("&lang;"),		TEXT("&lang;"),		TEXT("���p����") },	//	9001
	{ 0x232A,	("&rang;"),		TEXT("&rang;"),		TEXT("�E�p����") },	//	9002
	{ 0x25CA,	("&loz;"),		TEXT("&loz;"),		TEXT("�H�`") },	//	9674
	{ 0x2660,	("&spades;"),	TEXT("&spades;"),	TEXT("�X�y�[�h") },	//	9824
	{ 0x2663,	("&clubs;"),	TEXT("&clubs;"),	TEXT("�N���u") },	//	9827
	{ 0x2665,	("&hearts;"),	TEXT("&hearts;"),	TEXT("�n�[�g") },	//	9829
	{ 0x2666,	("&diams;"),	TEXT("&diams;"),	TEXT("�_�C��") },	//	9830

*/

