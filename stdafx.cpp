// stdafx.cpp : 標準インクルード OrinrinEditor.pch のみを
// 含むソース ファイルは、プリコンパイル済みヘッダーになります。
// stdafx.obj にはプリコンパイル済み型情報が含まれます。

#include "stdafx.h"
#include "OrinrinEditor.h"

#include "Entity.h"
//------------------------------------------------------------------------------------------------------------------------

#ifndef _ORRVW
extern  UINT	gbUniRadixHex;	//	ユニコード数値参照が１６進数であるか
#endif
//------------------------------------------------------------------------------------------------------------------------

//	確認チェックメッセージダイヤログ用
typedef struct tagMSGBOXMSG
{
	TCHAR	atMsg1[MAX_PATH];	//!<	文字列１
	TCHAR	atMsg2[MAX_PATH];	//!<	文字列２

	UINT	bChecked;	//!<	非０チェキられた　０ノーチェック

} MSGBOXMSG, *LPMSGBOXMSG;

//------------------------------------------------------------------------------------------------------------------------


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
	@param[in]	adMozi	チェキりたいユニコード文字
	@param[in]	adMozi	チェキりたいユニコード文字
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
	プレビュ用に、HTML的にヤバイ文字をエンティティする
	@param[in]	adMozi	チェキりたいユニコード文字
	@param[out]	pcStr	エンティティした場合は、その文字列を入れる
	@param[in]	cbSize	文字列バッファの文字数（バイト数）
	@return		BOOLEAN	非０エンティティした　０問題無い
*/
BOOLEAN HtmlEntityCheck( TCHAR adMozi, LPSTR pcStr, UINT_PTR cbSize )
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
	SJISから&#0000;を考慮してユニコードに変換
	&#から始まる部分が実体参照かどうかチェック・１０文字みる、ヌルターミネータまで
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
		if( NULL == pcStr[i] )	return 0x0000;	//	末端だった場合

		if( ';' == pcStr[i] )	break;	//	末端確認

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
					if( !( isalpha( acSrp[1] ) ) )	break;
				}
			}
			//	この時点で、bStrがFALSEなら、&xxxx; な内容じゃなかった
			if( !(bStr) ){	pcOutput[dt++] = pcMoto[sc];	continue;	}

			//	＆の次の字を確認して、検索範囲を絞り込む
			acChk[0] = acSrp[1];	acChk[1] = 0x00;
			CharLowerA( acChk );
			check = acChk[0] - 'a';	//	位置を確認

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


	cchSize = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pcBuff, -1, NULL, 0 );

	//	出力用ユニコードバッファ
	ptBuffer = (LPTSTR)malloc( (cchSize+1) * 2 );
	ZeroMemory( ptBuffer, (cchSize+1) * 2 );

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

#ifndef _ORRVW

/*!
	ユニコード文字列を受け取って、SJIS文字列に変換・返り値は呼んだ方でfreeせにゃ
	@param[in]	ptTexts	ユニコード文字列バッファのポインター
	@return		確保したSJIS文字列バッファのポインター・freeセヨ
*/
LPSTR SjisEncodeAlloc( LPTSTR ptTexts )
{
	TCHAR	atMozi[2];
	CHAR	acSjis[10];
	BOOL	bCant = FALSE;
	INT		iRslt;
	UINT_PTR	cchSize, d, cbSize;
	LPSTR	pcString;

	string	sString;

	//	長さ確認
	StringCchLength( ptTexts, STRSAFE_MAX_CCH, &cchSize );

	sString.clear( );

	atMozi[1] = 0;
	for( d = 0; cchSize > d; d++ )
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
			StringCchCopy( stMsg.atMsg1, MAX_PATH, TEXT("統合しちゃったら復帰できないのですよ・・・") );
			StringCchCopy( stMsg.atMsg2, MAX_PATH, TEXT("本当にくっつけていいのですか？") );
			number = VL_PCOMBINE_NM;
			break;

		case  1:	//	分離確認
			StringCchCopy( stMsg.atMsg1, MAX_PATH, TEXT("分割しちゃったら復帰できないのですよ・・・") );
			StringCchCopy( stMsg.atMsg2, MAX_PATH, TEXT("本当にバラしていいのですか？") );
			number = VL_PDIVIDE_NM;
			break;

		case  2:	//	削除確認
			StringCchCopy( stMsg.atMsg1, MAX_PATH, TEXT("削除しちゃったら復帰できないのですよ・・・") );
			StringCchCopy( stMsg.atMsg2, MAX_PATH, TEXT("本当にあぼーんしていいのですか？") );
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


