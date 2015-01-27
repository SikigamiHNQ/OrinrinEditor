/*! @file
	@brief 全般的な処理とかです
	このファイルは stdafx.cpp です。
	@author	SikigamiHNQ
	@date	2011/05/10
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
#ifdef _ORCOLL
#include "OrinrinCollector.h"
#else
#include "OrinrinEditor.h"
#endif
#include "Entity.h"
//------------------------------------------------------------------------------------------------------------------------

#ifndef _ORRVW
extern  UINT	gbUniRadixHex;	//	ユニコード数値参照が１６進数であるか
#endif
//------------------------------------------------------------------------------------------------------------------------

//	文字列検索は簡易Boyer-Moore法
typedef struct tagFINDPATTERN
{
	TCHAR	cchMozi;
	INT		iDistance;

} FINDPATTERN, *LPFINDPATTERN;
//--------------------------------

//	確認チェックメッセージダイヤログ用
typedef struct tagMSGBOXMSG
{
	TCHAR	atMsg1[MAX_PATH];	//!<	文字列１
	TCHAR	atMsg2[MAX_PATH];	//!<	文字列２

	UINT	bChecked;	//!<	非０チェキられた　０ノーチェック

} MSGBOXMSG, *LPMSGBOXMSG;

//------------------------------------------------------------------------------------------------------------------------

#ifdef SPMOZI_ENCODE

EXTERNED UINT	gbSpMoziEnc;	//!<	機種依存文字を数値参照コピーする

//	機種依存文字	2015/01/23	なんか増えた
static CONST TCHAR	gatSpMoziList[] = {
	TEXT("①②③④⑤⑥⑦⑧⑨⑩⑪⑫⑬⑭⑮⑯⑰⑱⑲⑳ⅠⅡⅢⅣⅤⅥⅦⅧⅨⅩⅰⅱⅲⅳⅴⅵⅶⅷⅸⅹ㍉㌔㌢㍍㌘㌧㌃㌶㍑㍗")
	TEXT("㌍㌦㌣㌫㍊㌻㎜㎝㎞㎎㎏㏄㎡㍻〝〟№㏍℡㊤㊥㊦㊧㊨㈱㈲㈹㍾㍽㍼≒≡∫∮∑√⊥∠∟⊿∵∩∪￢￤＇＂丨纊褜")	//	100
	TEXT("鍈銈蓜俉炻昱棈鋹曻彅仡仼伀伃伹佖侒侊侚侔俍偀倢俿倞偆偰偂傔僴僘兊兤冝冾凬刕劜劦勀勛匀匇匤卲厓厲叝﨎咜")
	TEXT("咊咩哿喆坙坥垬埈埇﨏塚增墲夋奓奛奝奣妤妺孖寀甯寘寬尞岦岺峵崧嵓﨑嵂嵭嶸嶹巐弡弴彧德忞恝悅悊惞惕愠惲愑")	//	200
	TEXT("愷愰憘戓抦揵摠撝擎敎昀昕昻昉昮昞昤晥晗晙晴晳暙暠暲暿曺朎朗杦枻桒柀栁桄棏﨓楨﨔榘槢樰橫橆橳橾櫢櫤毖氿")
	TEXT("汜沆汯泚洄涇浯涖涬淏淸淲淼渹湜渧渼溿澈澵濵瀅瀇瀨炅炫焏焄煜煆煇凞燁燾犱犾猤猪獷玽珉珖珣珒琇珵琦琪琩琮")	//	300
	TEXT("瑢璉璟甁畯皂皜皞皛皦益睆劯砡硎硤硺礰礼神祥禔福禛竑竧靖竫箞精絈絜綷綠緖繒罇羡羽茁荢荿菇菶葈蒴蕓蕙蕫﨟")
	TEXT("薰蘒﨡蠇裵訒訷詹誧誾諟諸諶譓譿賰賴贒赶﨣軏﨤逸遧郞都鄕鄧釚釗釞釭釮釤釥鈆鈐鈊鈺鉀鈼鉎鉙鉑鈹鉧銧鉷鉸鋧")	//	400
	TEXT("鋗鋙鋐﨧鋕鋠鋓錥錡鋻﨨錞鋿錝錂鍰鍗鎤鏆鏞鏸鐱鑅鑈閒隆﨩隝隯霳霻靃靍靏靑靕顗顥飯飼餧館馞驎髙髜魵魲鮏鮱")
	TEXT("鮻鰀鵰鵫鶴鸙黑") };	//	457


#define SPMOZI_CNT	457	//	文字数
#endif
//------------------------------------------------------------------------------------------------------------------------

#ifdef SPMOZI_ENCODE
/*!
	対象のユニコード文字はSJISの機種依存文字であるか
	@param[in]	tMozi	チェックするユニコード文字
	@return	非０該当する　０しない
*/
UINT IsSpMozi( TCHAR tMozi )
{
	UINT	i;

	if( !(gbSpMoziEnc) )	return 0;	//	未使用ならいつでも該当しない

	for( i = 0; SPMOZI_CNT > i; i++ )
	{
		if( gatSpMoziList[i] == tMozi )	return 1;
	}

	return 0;
}
//------------------------------------------------------------------------------------------------------------------------
#endif

/*!
	ウインドウにユーザデータを書き込む
	@param[in]	hWnd	書き込みたいウインドウのハンドル
	@param[in]	tag		書き込む数値。LONG値
	@return	無し
*/
VOID WndTagSet( HWND hWnd, LONG_PTR tag )
{
	SetWindowLongPtr( hWnd, GWLP_USERDATA, tag );
}
//------------------------------------------------------------------------------------------------------------------------

/*!
	ウインドウのユーザデータを読み込む
	@param[in]	hWnd	読み込むウインドウのハンドル
	@return	ユーザデータ値。初期値は0
*/
LONG_PTR WndTagGet( HWND hWnd )
{
	return GetWindowLongPtr( hWnd, GWLP_USERDATA );
}
//-------------------------------------------------------------------------------------------------

/*!
ファイルの拡張子を比較
	@param[in]	ptFile	チェキりたいファイル名
	@param[in]	ptExte	チェキりたい拡張子・ピリヲドを含める
	@return		BOOLEAN	非０該当拡張子である　０違うか、拡張子が無い
*/
BOOLEAN FileExtensionCheck( LPTSTR ptFile, LPTSTR ptExte )
{
	TCHAR	atExBuf[10];
	LPTSTR	ptExten;

	//	拡張子を確認・ドット込みだよ～ん
	ptExten = PathFindExtension( ptFile );	//	拡張子が無いならNULL、というか末端になる
	if( 0 == *ptExten ){	return 0;	}
	//	拡張子指定がないならなにもできない

	StringCchCopy( atExBuf, 10, ptExten );
	CharLower( atExBuf );	//	比較のために小文字にしちゃう

	//	内容違うなら真値となる
	if( StrCmp( atExBuf, ptExte ) ){	return 0;	}

	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	プレビュ用に、HTML的にヤバイ文字をエンティティする・SJIS用
	@param[in]	adMozi	チェキりたいユニコード文字
	@param[out]	pcStr	エンティティした場合は、その文字列を入れる
	@param[in]	cbSize	文字列バッファの文字数（バイト数）
	@return		BOOLEAN	非０エンティティした　０問題無い
*/
BOOLEAN HtmlEntityCheckA( TCHAR adMozi, LPSTR pcStr, UINT_PTR cbSize )
{
	INT	i;

	ZeroMemory( pcStr, cbSize );

	for( i = 0; 4 > i; i++ )
	{
		if( 0 == gstEttySP[i].dUniCode )	break;

		if( gstEttySP[i].dUniCode == adMozi )
		{
			StringCchCopyA( pcStr, cbSize, gstEttySP[i].acCodeA );
			return TRUE;
		}
	}

	return FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	プレビュ用に、HTML的にヤバイ文字をエンティティする・Unicode用
	@param[in]	adMozi	チェキりたいユニコード文字
	@param[out]	ptStr	エンティティした場合は、その文字列を入れる
	@param[in]	cchSize	文字列バッファの文字数
	@return		BOOLEAN	非０エンティティした　０問題無い
*/
BOOLEAN HtmlEntityCheckW( TCHAR adMozi, LPTSTR ptStr, UINT_PTR cchSize )
{
	INT	i;

	ZeroMemory( ptStr, cchSize * sizeof(TCHAR) );

	for( i = 0; 4 > i; i++ )
	{
		if( 0 == gstEttySP[i].dUniCode )	break;

		if( gstEttySP[i].dUniCode == adMozi )
		{
			StringCchCopy( ptStr, cchSize, gstEttySP[i].atCodeW );
			return TRUE;
		}
	}

	return FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	SJISから"&#0000;"を考慮してユニコードに変換
	"&#"から始まる部分が実体参照かどうかチェック・１０文字みる、ヌルターミネータまで
	@param[in]	pcStr	確認する文字列の先頭
	@return		TCHAR	ユニコード文字（数値の部分そのまま）
*/
TCHAR UniRefCheck( LPSTR pcStr )
{
	CHAR	acValue[10];
	PCHAR	pcEnd;
	UINT	i, code;
	INT		radix = 10;
	BOOLEAN	bXcode = FALSE;

	ZeroMemory( acValue, sizeof(acValue) );
	if( NULL == pcStr[2] )	return 0x0000;	//	末端だった場合

	pcStr += 2;	//	&#部分を進める

	//	&#xhhhh;な１６進数を確認
	if( 'x' == pcStr[0] || 'X' == pcStr[0] )
	{
		bXcode = TRUE;
		pcStr++;	//	次からが本番
		radix = 16;
	}

	for( i = 0; 10 > i; i++ )
	{
		if( ';' == pcStr[i] )	break;	//	末端確認

		if( NULL == pcStr[i] )	return 0x0000;	//	末端だった場合

		if( 0 > (INT)(pcStr[i]) )	return 0x0000;	//	とにかく関係なかった場合

		if( bXcode )
		{
			if( isxdigit( pcStr[i] ) ){	acValue[i] = pcStr[i];	}
			else	return 0x0000;	//	後が数値じゃなかった場合
		}
		else
		{
			if( isdigit( pcStr[i] ) ){	acValue[i] = pcStr[i];	}
			else	return 0x0000;	//	後が数値じゃなかった場合
		}
	}
	if( 10 <= i ){	return 0x0000;	}	//	なんか振り切ってる

	code = strtoul( acValue, &pcEnd, radix );	//	進数に合わせて変換しちゃう

//	機種依存文字変換　ユニコードとして扱っているので、ここで一律処理で問題無い
//	SPMOZI_ENCODE


	if( 0xFFFF < code ){	code = 0x0000;	}	//	明らかにおかしい場合

	return (TCHAR)code;
}
//-------------------------------------------------------------------------------------------------

/*!
	SJIS文字列を受け取って、実体参照をユニコード数値参照に変換する
	@param[in]	pcMoto	元の文字列
	@return	LPSTR	変換した文字列・受け取ったほうでFREEする・おかしかったらNULL
*/
LPSTR SjisEntityExchange( LPCSTR pcMoto )
{
	LPSTR		pcOutput, pcTemp;
	CHAR		acSrp[12], acChk[3], acUni[10];
	UINT		check, el;
	UINT_PTR	szMoto, szStr, sc, dt, se, rp, cbSz;
	BOOLEAN		bStr = FALSE, bHit = FALSE;

	CONST ENTITYLIST	*pstEty;

	szMoto = strlen( pcMoto );
	szStr = szMoto + 2;
	pcOutput = (LPSTR)malloc( szStr );	//	ヌルターミネータ用
	if( !(pcOutput) )	return NULL;
	ZeroMemory( pcOutput, szStr );	


	for( sc = 0, dt = 0; szMoto > sc; sc++ )
	{
		if( '&' == pcMoto[sc] )
		{
			se = sc;	//	使用する
			bStr = FALSE;

			for( rp = 0; 10 > rp; rp++ )
			{
				acSrp[rp] = pcMoto[se++];	acSrp[rp+1] = 0x00;
				if( ';'  == acSrp[rp] ){	bStr = TRUE;	break;	}

				if( 1 == rp )
				{
					if( 0 > acSrp[1] )	break;	//	isalphaにマイナス値でおちる？
					if( !( isalpha( acSrp[1] ) ) )	break;
				}
			}
			//	この時点で、bStrがFALSEなら、&xxxx; な内容じゃなかった
			if( !(bStr) ){	pcOutput[dt++] = pcMoto[sc];	continue;	}

			//	＆の次の字を確認して、検索範囲を絞り込む
			acChk[0] = acSrp[1];	acChk[1] = 0x00;
			CharLowerA( acChk );
			check = acChk[0] - 'a';	//	位置を確認
			//	20120301	０～２５の範囲外であるのなら、&xxxx;ではない
			if( 26 <= check ){	pcOutput[dt++] = pcMoto[sc];	continue;	}

			pstEty = gpstEntitys[check];

			bHit = FALSE;
			//	あたりがあるかどうか見ていく
			for( el = 0; 0 != pstEty[el].dUniCode; el++ )
			{
				if( 0 == StrCmpA( acSrp, pstEty[el].acCodeA ) )	//	ヒット
				{
					szStr += 4;	//	はみ出したら困るので少し増やす
					pcTemp = (LPSTR)realloc( pcOutput, szStr );
					if( pcTemp )	pcOutput = pcTemp;

					bHit = TRUE;
					StringCchPrintfA( acUni, 10, ("&#%d;"), pstEty[el].dUniCode );
					StringCchLengthA( acUni, 10, &cbSz );
					for( rp = 0; cbSz > rp; rp++ )
					{
						pcOutput[dt++] = acUni[rp];
					}
					sc = se - 1;	//	位置情報を書き戻す・ループで＋＋されるから引いておく
					break;
				}
			}
			//	この時点で、bHitがFALSEなら、実体のある内容じゃなかった
			if( !(bHit) ){	pcOutput[dt++] = pcMoto[sc];	}
		}
		else	//	エンティティじゃないならそのまま移せばいい
		{
			pcOutput[dt++] = pcMoto[sc];
		}
	}

	return pcOutput;
}
//-------------------------------------------------------------------------------------------------

/*!
	シフトJIS文字列を受け取って、ユニコード文字列に変換して返り値・呼んだほうでfreeするの忘れるな
	@param[in]	pcBuff	シフトJIS文字列バッファのポインター
	@return		確保したユニコード文字列バッファのポインター・freeセヨ
*/
LPTSTR SjisDecodeAlloc( LPSTR pcBuff )
{
	DWORD	cbWrtSize;
	LPSTR	pcPos, pcChk, pcPosEx;

	DWORD	cchSize, cchWrtSize;
	LPTSTR	ptBuffer, ptWrtpo;
	TCHAR	chMozi;

	DWORD	dStart, dEnd;

	if( !(pcBuff) ){	return NULL;	}	//	データおかしかったら終わり

	//	出力用ユニコードバッファを確保する。必要なサイズを求める
	cchSize = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pcBuff, -1, NULL, 0 );

	cchSize += 2;	//	バッファ自体の量なのでここで変更しておｋ
	ptBuffer = (LPTSTR)malloc( cchSize * sizeof(TCHAR) );
	ZeroMemory( ptBuffer, cchSize * sizeof(TCHAR) );

	dStart = 0;
	dEnd   = 0;

//	pcPos = pcBuff;
	ptWrtpo = ptBuffer;

	//まずSJISのままで、実体参照をユニコード参照に変換する
	pcPosEx = SjisEntityExchange( pcBuff );

	pcPos = pcPosEx;

	pcChk = StrStrA( pcPosEx, "&#" );	//	キーワードにヒットするかどうか
	while( pcChk )
	{
		//	問題の箇所の手前まで変換
		cbWrtSize  = pcChk - pcPos;	//	そこまでのサイズ求めて
		cchWrtSize = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pcPos, cbWrtSize, ptWrtpo, cchSize );

		ptWrtpo += cchWrtSize;	//	書込場所移動
		cchSize -= cchWrtSize;	//	残り文字数

		chMozi = UniRefCheck( pcChk );	//	有効な内容かどうか確認
		if( 0 != chMozi )	//	有効なナニカがあった
		{
			*ptWrtpo = chMozi;	//	そいつをぶち込んで
			ptWrtpo++;			//	次に進める
			cchSize--;			//	残り文字数は減らす
			pcChk = StrStrA( pcChk, ";" );	//	終端まで進めておく
			pcChk++;	//	';'まで含むので、さらにその次からが本番
		}
		else	//	関係ないなら、ヒットした２文字をカキコして、次に進む
		{
			cchWrtSize = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pcChk, 2, ptWrtpo, cchSize );

			ptWrtpo += cchWrtSize;	//	書込場所移動
			cchSize -= cchWrtSize;	//	残り文字数
			pcChk += 2;	//	使った分進めて
		}
		pcPos = pcChk;	//	次はここから調べていく

		pcChk = StrStrA( pcPos , "&#" );	//	キーワードにヒットするかどうか
	}
	//	残りの部分、もしくは最初から最後までを変換
	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pcPos, -1, ptWrtpo, cchSize );

	FREE( pcPosEx );

	return ptBuffer;
}
//-------------------------------------------------------------------------------------------------

/*!
	文字列をうけとって、行数と最大ドット幅を計算
	@param[in]	ptText	チェキりたいユニコード文字列受け取る
	@param[out]	piLine	行数返す
	@return		最大ドット数
*/
INT TextViewSizeGet( LPCTSTR ptText, PINT piLine )
{
	UINT_PTR	cchSize, i;
	INT		xDot, yLine, dMaxDot;

	wstring	wString;


	StringCchLength( ptText, STRSAFE_MAX_CCH, &cchSize );

	yLine = 1;	dMaxDot = 0;
	for( i = 0; cchSize > i; i++ )
	{
		if( CC_CR == ptText[i] && CC_LF == ptText[i+1] )	//	改行であったら
		{
			//	ドット数確認
			xDot = ViewStringWidthGet( wString.c_str() );
			if( dMaxDot < xDot )	dMaxDot = xDot;

			wString.clear( );
			i++;		//	0x0D,0x0Aだから、壱文字飛ばすのがポイント
			yLine++;	//	改行したから行数数える
		}
		else if( CC_TAB == ptText[i] )
		{
			//	タブは無かったことにする
		}
		else
		{
			wString += ptText[i];
		}
	}

	if( 1 <= wString.size() )	//	最終行確認
	{
		//	ドット数確認
		xDot = ViewStringWidthGet( wString.c_str() );
		if( dMaxDot < xDot )	dMaxDot = xDot;
	}

	if( piLine )	*piLine = yLine;	//	空行だったとしても１行はある
	return dMaxDot;

//ViewStringWidthGetは、ViewCentral、OrinrinViewerに、それぞれある
}
//-------------------------------------------------------------------------------------------------

/*!
	現在行から、次の行の先頭へ移動
	@param[in]	pt	改行を検索開始するところ
	@return		改行の次の位置
*/
LPCTSTR NextLineW( LPCTSTR pt )
{
	while( *pt && *pt != 0x000D ){	pt++;	}

	if( 0x000D == *pt )
	{
		pt++;
		if( 0x000A == *pt ){	pt++;	}
	}

	return pt;
}
//-------------------------------------------------------------------------------------------------
LPTSTR NextLineW( LPTSTR pt )
{
	while( *pt && *pt != 0x000D ){	pt++;	}

	if( 0x000D == *pt )
	{
		pt++;
		if( 0x000A == *pt ){	pt++;	}
	}

	return pt;
}
//-------------------------------------------------------------------------------------------------


/*!
	現在行から、次の行の先頭へ移動
	@param[in]	pt	改行を検索開始するところ
	@return		改行の次の位置
*/
LPSTR NextLineA( LPSTR pt )
{
	while( *pt && *pt != 0x0D ){	pt++;	}

	if( 0x0D == *pt )
	{
		pt++;
		if( 0x0A == *pt ){	pt++;	}
	}

	return pt;
}
//-------------------------------------------------------------------------------------------------


#ifndef _ORRVW

/*!
	ユニコード文字列を受け取って、SJIS文字列に変換・返り値は呼んだ方でfreeせにゃ
	@param[in]	ptTexts	ユニコード文字列バッファのポインター
	@return		確保したSJIS文字列バッファのポインター・freeセヨ
*/
LPSTR SjisEncodeAlloc( LPCTSTR ptTexts )
{
	TCHAR	atMozi[2];
	CHAR	acSjis[10];
	BOOL	bCant = FALSE;
	INT		iRslt;
	UINT_PTR	cchSize, d, cbSize;
	LPSTR	pcString;

	string	sString;

	if( !(ptTexts) ){	return NULL;	}

	//	長さ確認
	StringCchLength( ptTexts, STRSAFE_MAX_CCH, &cchSize );

	sString.clear( );

	atMozi[1] = 0;
	for( d = 0; cchSize > d; d++ )	//	壱文字ずつ変換していく
	{
		atMozi[0] = ptTexts[d];
		ZeroMemory( acSjis, sizeof(acSjis) );

		iRslt = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, atMozi, 1, acSjis, 10, "?", &bCant );
		//	変換出来ればそれでおｋ、ダメなら数値参照形式にしちゃう
		if( bCant )
		{
			if( gbUniRadixHex ){	StringCchPrintfA( acSjis, 10 , ("&#x%X;"), ptTexts[d] );	}
			else{					StringCchPrintfA( acSjis, 10 , ("&#%d;"),  ptTexts[d] );	}
		}

#ifdef SPMOZI_ENCODE
		if( IsSpMozi( ptTexts[d] ) )	//	機種依存文字変換
		{
			if( gbUniRadixHex ){	StringCchPrintfA( acSjis, 10 , ("&#x%X;"), ptTexts[d] );	}
			else{					StringCchPrintfA( acSjis, 10 , ("&#%d;"),  ptTexts[d] );	}
		}
#endif

		sString += string( acSjis );
	}

	cbSize = sString.size( ) + 2;
	pcString = (LPSTR)malloc( cbSize );
	ZeroMemory( pcString, cbSize );
	StringCchCopyA( pcString, cbSize, sString.c_str() );

	return pcString;
}
//-------------------------------------------------------------------------------------------------
#endif


/*!
	BM検索テーブルつくる
	@param[in]	ptPattern	検索パターン
	@return	LPFINDPATTERN	作成した検索テーブル
*/
LPFINDPATTERN FindTableMake( LPCTSTR ptPattern )
{
	UINT		i;
	UINT_PTR	dLength;
	LPFINDPATTERN	pstPtrn;

	//	テーブルをパターンの長さで初期化する
	StringCchLength( ptPattern, STRSAFE_MAX_CCH, &dLength );
	pstPtrn = (LPFINDPATTERN)malloc( (dLength+1) * sizeof(FINDPATTERN) );
	ZeroMemory( pstPtrn, (dLength+1) * sizeof(FINDPATTERN) );

	for( i = 0; dLength >= i; i++ ){	pstPtrn[i].iDistance =  dLength;	}

	//	パターンの先頭から、文字に対応する位置に末尾からの長さを登録する
	while( dLength > 0 )
	{
		i = 0;
		while( pstPtrn[i].cchMozi )
		{
			if( pstPtrn[i].cchMozi ==  *ptPattern ){	break;	}
			i++;
		}
		pstPtrn[i].cchMozi   = *ptPattern;
		pstPtrn[i].iDistance = --dLength;

		ptPattern++;
	}

	return pstPtrn;
}
//-------------------------------------------------------------------------------------------------

/*!
	BM検索する
	@param[in]	ptText		検索対象文字列
	@param[in]	ptPattern	検索パターン
	@param[out]	pdCch		ヒットした文字位置数・なかったら0
	@return		LPTSTR		ヒットした部分の開始・なかったらNULL
*/
LPTSTR FindStringProc( LPTSTR ptText, LPTSTR ptPattern, LPINT pdCch )
{
	UINT_PTR	dPtrnLen, dLength;
	LPTSTR	ptTextEnd;
	INT		i, j, k, jump, cch;

	LPFINDPATTERN	pstPattern;

	StringCchLength( ptText, STRSAFE_MAX_CCH, &dLength );

	StringCchLength( ptPattern, STRSAFE_MAX_CCH, &dPtrnLen );
	dPtrnLen--;

	ptTextEnd = ptText + dLength - dPtrnLen;

	pstPattern = FindTableMake( ptPattern );

	cch = 0;
	while( ptText < ptTextEnd )
	{
		for( i = dPtrnLen ; i >= 0 ; i-- )
		{
			if( ptText[i] != ptPattern[i] ){	break;	}
		}

		//	全て一致した
		if( i < 0 ){	FREE( pstPattern  );	*pdCch = cch;	return( ptText );	}

		//	テーブルから移動量を求める(負数なら移動量は２)
		k = 0;
		while( pstPattern[k].cchMozi )
		{
			if( pstPattern[k].cchMozi == ptText[i] ){	break;	}
			k++;
		}
		j = pstPattern[k].iDistance - ( dPtrnLen - i );
		jump = ( 0 < j ) ? j : 2;
		ptText += jump;
		cch += jump;
	}

	FREE( pstPattern );

	*pdCch = 0;

	return( NULL );
}
//-------------------------------------------------------------------------------------------------

/*!
	次から表示しないチェキボックス付きダイヤログボックスプロシージャ
	@param[in]	hDlg	ダイヤログハンドル
	@param[in]	message	ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@retval 0	メッセージは処理していない
	@retval no0	なんか処理された
*/
INT_PTR CALLBACK MsgCheckBoxProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static LPMSGBOXMSG	pcstMsg = NULL;
	INT	id;

	switch( message )
	{
		default:	break;

		case WM_INITDIALOG:
			pcstMsg = (LPMSGBOXMSG)lParam;
			SetDlgItemText( hDlg, IDS_MC_MSG1, pcstMsg->atMsg1 );
			SetDlgItemText( hDlg, IDS_MC_MSG2, pcstMsg->atMsg2 );
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			id = LOWORD(wParam);
			if( pcstMsg ){	pcstMsg->bChecked = IsDlgButtonChecked( hDlg, IDCB_MC_CHECKBOX );	}
			if( IDYES == id || IDOK == id ){	EndDialog( hDlg, IDYES );	}
			if( IDNO == id || IDCANCEL == id ){	EndDialog( hDlg, IDNO );	}
			return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	チェックボックス付きメッセージボックスもどき
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	hInst	アプリの実存
	@param[in]	dStyle	表示Message　０頁統合　１頁分割　２頁削除
*/
INT_PTR MessageBoxCheckBox( HWND hWnd, HINSTANCE hInst, UINT dStyle )
{
	INT_PTR	iRslt;
	UINT	number;
	MSGBOXMSG	stMsg;


	switch( dStyle )
	{
		case  0:	//	統合確認
			StringCchCopy( stMsg.atMsg1, MAX_PATH, TEXT("統合しちゃったら復帰できないよ") );
			StringCchCopy( stMsg.atMsg2, MAX_PATH, TEXT("本当にくっつけていい？") );
			number = VL_PCOMBINE_NM;
			break;

		case  1:	//	分離確認
			StringCchCopy( stMsg.atMsg1, MAX_PATH, TEXT("分割しちゃったら復帰できないよ") );
			StringCchCopy( stMsg.atMsg2, MAX_PATH, TEXT("本当にバラしていい？") );
			number = VL_PDIVIDE_NM;
			break;

		case  2:	//	削除確認
			StringCchCopy( stMsg.atMsg1, MAX_PATH, TEXT("削除しちゃったら復帰できないよ") );
			StringCchCopy( stMsg.atMsg2, MAX_PATH, TEXT("本当にあぼーんしていい？") );
			number = VL_PDELETE_NM;
			break;

		default:	return IDCANCEL;
	}

	stMsg.bChecked = InitParamValue( INIT_LOAD, number, 0 );
	if( 1 == stMsg.bChecked ){	iRslt = IDYES;	}
	else
	{
		iRslt = DialogBoxParam( hInst, MAKEINTRESOURCE(IDD_MSGCHECKBOX_DLG), hWnd, MsgCheckBoxProc, (LPARAM)(&stMsg) );
		InitParamValue( INIT_SAVE, number, (BST_CHECKED==stMsg.bChecked) ? 1 : 0 );
	}
	return iRslt;
}
//-------------------------------------------------------------------------------------------------

#ifdef DO_TRY_CATCH
/*!
	例外きゃっち
*/
LRESULT ExceptionMessage( LPCSTR pcExpMsg, LPCSTR pcFuncName, UINT dLine, LPARAM lReturn )
{
	CHAR	acMessage[BIG_STRING];

	StringCchPrintfA( acMessage, BIG_STRING, ("異常発生＜%s＞[%s:%u]\r\nプログラムを続行できません。"), pcExpMsg, pcFuncName, dLine );

	MessageBoxA( GetDesktopWindow(), acMessage, ("致命的エラー発生"), MB_OK );

	return lReturn;
}
//-------------------------------------------------------------------------------------------------

#endif

