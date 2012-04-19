/*! @file
	@brief ファイルに保存・開くの面倒見ます
	このファイルは DocFileCtrl.cpp です。
	@author	SikigamiHNQ
	@date	2011/04/27
*/

/*
Orinrin Editor : AsciiArt Story Editor for Japanese Only
Copyright (C) 2011 - 2012 Orinrin/SikigamiHNQ

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.
If not, see <http://www.gnu.org/licenses/>.
*/
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "OrinrinEditor.h"
//-------------------------------------------------------------------------------------------------

//	TODO:	保存するとき、同じ名前のファイルがあれば、日時をつけて自動でバックアップを取る

//	TODO:	保存するとき、ユニコードかSJISか選択出来るように・デフォはSJISでいいか


extern list<ONEFILE>	gltMultiFiles;	//!<	複数ファイル保持

extern FILES_ITR	gitFileIt;		//!<	今見てるファイルの本体
//#define gstFile	(*gitFileIt)	//!<	イテレータを構造体と見なす

extern INT		gixFocusPage;		//!<	注目中のページ・とりあえず０・０インデックス

extern  UINT	gbAutoBUmsg;		//	自動バックアップメッセージ出すか？

static TCHAR	gatBackUpDirty[MAX_PATH];

//-------------------------------------------------------------------------------------------------

INT	DocAstSeparatorGetAlloc( INT, UINT, LPVOID *, FILES_ITR );

INT	DocUnicode2UTF8( LPVOID * );
//-------------------------------------------------------------------------------------------------


/*!
	該当するファイルは開き済か
	@param[in]	ptFile	確認したいファイル名
	@return	UINT	負：無し　１以上：ヒットしたやつのUNIQUE番号
*/
LPARAM DocOpendFileCheck( LPTSTR ptFile )
{
	FILES_ITR	itFile;

	for( itFile = gltMultiFiles.begin(); gltMultiFiles.end() != itFile; itFile++ )
	{
		//	ヒットしたらそれでおｋ
		if( !( StrCmp( itFile->atFileName, ptFile ) ) ){	return  itFile->dUnique;	}
	}

	return -1;
}
//-------------------------------------------------------------------------------------------------

/*!
	ファイルから読み込む
	@param[in]	hWnd	親にするウインドウハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT DocFileOpen( HWND hWnd )
{
	OPENFILENAME	stOpenFile;
	BOOLEAN	bOpened;
//	LPARAM	dNumber;
	TCHAR	atFilePath[MAX_PATH], atFileName[MAX_STRING];


	ZeroMemory( &stOpenFile, sizeof(OPENFILENAME) );

	ZeroMemory( atFilePath,  sizeof(atFilePath) );
	ZeroMemory( atFileName,  sizeof(atFileName) );

	stOpenFile.lStructSize     = sizeof(OPENFILENAME);
	stOpenFile.hwndOwner       = hWnd;
	stOpenFile.lpstrFilter     = TEXT("アスキーアートファイル ( mlt, ast, txt )\0*.mlt;*.ast;*.txt\0全ての形式(*.*)\0*.*\0\0");
	stOpenFile.nFilterIndex    = 1;
	stOpenFile.lpstrFile       = atFilePath;
	stOpenFile.nMaxFile        = MAX_PATH;
	stOpenFile.lpstrFileTitle  = atFileName;
	stOpenFile.nMaxFileTitle   = MAX_STRING;
//	stOpenFile.lpstrInitialDir = 
	stOpenFile.lpstrTitle      = TEXT("開くファイルを指定するかしらー");
	stOpenFile.Flags           = OFN_EXPLORER | OFN_HIDEREADONLY;
	stOpenFile.lpstrDefExt     = TEXT("mlt");

	//ここで FileOpenDialogue を出す
	bOpened = GetOpenFileName( &stOpenFile );

	ViewFocusSet(  );

	if( !(bOpened) ){	return  E_ABORT;	}	//	キャンセルしてたら何もしない

	DocDoOpenFile( hWnd, atFilePath );	//	開いて中身展開

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ファイル名を受けて、オーポン処理する
	@param[in]	ptFile	開くファイルフルパス
	@return	HRESULT	終了状態コード
*/
HRESULT DocDoOpenFile( HWND hWnd, LPTSTR ptFile )
{
	LPARAM	dNumber;

	//	既存のファイルを開こうとしたらそっちをフォーカスするだけにするのが良いはず
	dNumber = DocOpendFileCheck( ptFile );
	if( 1 <= dNumber )	//	既存のファイルヒット・そっちに移動する
	{
		if( SUCCEEDED( MultiFileTabSelect( dNumber ) ) )	//	該当のタブにフォーカス移して
		{
			DocMultiFileSelect( dNumber );	//	そのタブのファイルを表示
			return S_OK;
		}
	}

	dNumber = DocFileInflate( ptFile );	//	開いて中身展開
	if( !(dNumber) )
	{
		MessageBox( hWnd, TEXT("ファイルを開けなかったかしらー！？"), NULL, MB_OK | MB_ICONERROR );
		return E_HANDLE;
	}
	else
	{
		MultiFileTabAppend( dNumber, ptFile );	//	ダイヤログからファイルオーポン
		OpenHistoryLogging( hWnd , ptFile );	//	ファイルオーポン記録を追加
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------


/*!
	バックアップディレクトリーを確保
	@param[in]	ptCurrent	基準ディレクトリ
*/
VOID DocBackupDirectoryInit( LPTSTR ptCurrent )
{
	StringCchCopy( gatBackUpDirty, MAX_PATH, ptCurrent );
	PathAppend( gatBackUpDirty, BACKUP_DIR );
	CreateDirectory( gatBackUpDirty, NULL );
	//	已にディレクトリがあったら函数が失敗するだけなので問題無い

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	インターバルで自動保存
	@param[in]	hWnd	ウインドウハンドル・どこのウインドウハンドルかは非固定
	@return		HRESULT	終了状態コード
*/
HRESULT DocFileBackup( HWND hWnd )
{
	CONST  TCHAR	aatExte[3][5] = { {TEXT(".ast")}, {TEXT(".mlt")}, {TEXT(".txt")} };

	TCHAR	atFilePath[MAX_PATH], atFileName[MAX_STRING];
	TCHAR	atBuffer[MAX_PATH];

	HANDLE	hFile;
	DWORD	wrote;

	LPTSTR	ptExten;	//	ファイル名の拡張子
	TCHAR	atExBuf[10];

	LPVOID	pBuffer;	//	文字列バッファ用ポインター
	INT		iByteSize, iNullTmt, iCrLf;

	LPVOID	pbSplit;
	UINT	cbSplSz;

	INT		isAST, isMLT, ddExten;

	UINT_PTR	iPages, i;	//	頁数

	FILES_ITR	itFile;

	ZeroMemory( atFilePath,  sizeof(atFilePath) );
	ZeroMemory( atFileName,  sizeof(atFileName) );
	ZeroMemory( atBuffer,  sizeof(atBuffer) );

//複数ファイル、各ファイルをセーブするには？
	for( itFile = gltMultiFiles.begin(); itFile != gltMultiFiles.end(); itFile++ )
	{
		iPages = itFile->vcCont.size( );	//	総頁数

		if( 1 >= iPages )	isMLT = FALSE;
		else				isMLT = TRUE;

		isAST = PageListIsNamed( itFile );	//	頁に名前が付いてる？

		if( isAST ){		ddExten = 0;	}	//	AST
		else if( isMLT ){	ddExten = 1;	}	//	MLT
		else{				ddExten = 2;	}	//	TXT

		StringCchCopy( atBuffer, MAX_PATH, itFile->atFileName );

		if( atBuffer[0] == NULL )	//	名称未設定状態
		{
			StringCchCopy( atFileName, MAX_STRING, itFile->atDummyName );
		}
		else
		{
			PathStripPath( atBuffer );
			StringCchCopy( atFileName, MAX_STRING, atBuffer );
		}

		//	拡張子を確認・ドット込みだよ～ん
		ptExten = PathFindExtension( atFileName );	//	拡張子が無いならNULL、というか末端になる
		if( 0 == *ptExten )
		{
			//	拡張子指定がないならそのまま対応のをくっつける
			StringCchCopy( ptExten, 5, aatExte[ddExten] );
		}
		else	//	既存の拡張子があったら
		{
			StringCchCopy( atExBuf, 10, ptExten );
			CharLower( atExBuf );	//	比較のために小文字にしちゃう

			if( isAST )	//	ASTは優先的に適用
			{
				if( StrCmp( atExBuf, aatExte[0] ) )	//	もしASTじゃなかったら変更
				{
					StringCchCopy( ptExten, 5, aatExte[0] );
				}
			}
			else if( isMLT )	//	名前無いけど複数頁ならMLTじゃないとダメ
			{
				if( StrCmp( atExBuf, aatExte[1] ) )	//	もしMLTじゃなかったら変更
				{
					StringCchCopy( ptExten, 5, aatExte[1] );
				}
			}
			//	一枚なら、TXTでもMLTでも気にしなくてよかばい
		}

		StringCchCopy( atFilePath, MAX_STRING, gatBackUpDirty );
		PathAppend( atFilePath, atFileName );	//	Backupファイル名


		hFile = CreateFile( atFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if( INVALID_HANDLE_VALUE == hFile )
		{
			NotifyBalloonExist( TEXT("あぅあぅ、バックアップ出来なかったのです。あぅあぅ"), TEXT("あぅあぅ"), NIIF_ERROR );
			//	gbAutoBUmsg	バックアップ出来なかったメッセージは常に表示がいいか
			return E_HANDLE;
		}

		iNullTmt = 1;
		iCrLf = CH_CRLF_CCH;
		SetFilePointer( hFile, 0, NULL, FILE_BEGIN );

		if( isAST )
		{
			pbSplit = NULL;
		}
		else
		{
			pbSplit = malloc( 30 );
			ZeroMemory( pbSplit, 30 );

			cbSplSz = MLT_SPRT_CCH + CH_CRLF_CCH;
			StringCchPrintfA( (LPSTR)pbSplit, 30, ("%s%s"), MLT_SEPARATERA, CH_CRLFA );
		}

		for( i = 0; iPages > i; i++ )	//	全頁保存
		{
			if( isAST )
			{
				cbSplSz = DocAstSeparatorGetAlloc( i, D_SJIS, &pbSplit, itFile );

				WriteFile( hFile , pbSplit, (cbSplSz- iNullTmt), &wrote, NULL );
				FREE(pbSplit);
			}
			else
			{
				if( 1 <= i ){	WriteFile( hFile , pbSplit, cbSplSz, &wrote, NULL );	}
			}

			iByteSize = DocPageTextGetAlloc( itFile, i, D_SJIS, &pBuffer, TRUE );

			if( (i+1) == iPages ){	iByteSize -=  iCrLf;	}
			//	最終頁の末端の改行は不要のはず
			WriteFile( hFile, pBuffer, iByteSize - iNullTmt, &wrote, NULL );

			FREE( pBuffer );
		}

		SetEndOfFile( hFile );
		CloseHandle( hFile );

		FREE( pbSplit );
	}

	if( gbAutoBUmsg ){	NotifyBalloonExist( TEXT("作業中のファイルをバックアップ保存したのです。あぅあぅ"), TEXT("あぅあぅ"), NIIF_INFO );	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ファイルに保存する
	@param[in]	hWnd	親にするウインドウハンドル
	@param[in]	bStyle	上書きかリネームか・フォーマット選択はダイヤログでやる
	@return		HRESULT	終了状態コード
*/
HRESULT DocFileSave( HWND hWnd, UINT bStyle )
{
	CONST  TCHAR	aatExte[3][5] = { {TEXT(".ast")}, {TEXT(".mlt")}, {TEXT(".txt")} };
	CONST  WCHAR	rtHead = 0xFEFF;	//	ユニコードテキストヘッダ

	SYSTEMTIME		stSysTile;
	OPENFILENAME	stSaveFile;

	BOOLEAN	bOpened;

	TCHAR	atFilePath[MAX_PATH], atFileName[MAX_STRING];
	TCHAR	atBuffer[MAX_STRING];

	HANDLE	hFile;
	DWORD	wrote;

	LPTSTR	ptExten;	//	ファイル名の拡張子
	TCHAR	atExBuf[10];

	LPVOID	pBuffer;	//	文字列バッファ用ポインター
	INT		iByteSize, iNullTmt, iCrLf;

	LPVOID	pbSplit;
	UINT	cbSplSz;

	INT		isAST, isMLT, ddExten;
	BOOLEAN	bExtChg =FALSE, bLastChg = FALSE;

	BOOLEAN	bNoName = FALSE;

	BOOLEAN	bUtf8 = FALSE;	//	ＵＴＦ８で保存セヨ
	BOOLEAN	bUnic = FALSE;	//	ユニコードで保存セヨ

	UINT_PTR	iPages, i;	//	頁数

	ZeroMemory( &stSaveFile, sizeof(OPENFILENAME) );

	ZeroMemory( atFilePath,  sizeof(atFilePath) );
	ZeroMemory( atFileName,  sizeof(atFileName) );
	ZeroMemory( atBuffer,  sizeof(atBuffer) );

	//	保存時は常に選択しているファイルを保存

	iPages = (*gitFileIt).vcCont.size( );	//	総頁数
	if( 1 >= iPages )	isMLT = FALSE;
	else				isMLT = TRUE;

//既存の拡張子がASTなら、それを優先する

	isAST = PageListIsNamed( gitFileIt );	//	頁に名前が付いてる？

	if( isAST ){		ddExten = 0;	}	//	AST
	else if( isMLT ){	ddExten = 1;	}	//	MLT
	else{				ddExten = 2;	}	//	TXT

	GetLocalTime( &stSysTile );

	StringCchCopy( atFilePath, MAX_PATH, (*gitFileIt).atFileName );

	if( NULL == (*gitFileIt).atFileName[0] )	bNoName = TRUE;

	//	リネームか、ファイル名が無かったら保存ダイヤログ開く
	if( (bStyle & D_RENAME) || bNoName )
	{

		//ここで FileSaveDialogue を出す
		stSaveFile.lStructSize     = sizeof(OPENFILENAME);
		stSaveFile.hwndOwner       = hWnd;
		stSaveFile.lpstrFilter     = TEXT("[ShiftJIS]アスキーアートファイル ( mlt, ast, txt )\0*.mlt;*.ast;*.txt\0[UTF8]アスキーアートファイル ( mlt, ast, txt )\0*.mlt;*.ast;*.txt\0\0");
		stSaveFile.nFilterIndex    = 1;	//	デフォのフィルタ選択肢
		stSaveFile.lpstrFile       = atFilePath;
		stSaveFile.nMaxFile        = MAX_PATH;
		stSaveFile.lpstrFileTitle  = atFileName;
		stSaveFile.nMaxFileTitle   = MAX_STRING;
//		stSaveFile.lpstrInitialDir = 
		stSaveFile.lpstrTitle      = TEXT("保存するファイル名を指定するですぅ");
		stSaveFile.Flags           = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
		//if( 1 >= iPages )	stSaveFile.lpstrDefExt = TEXT("txt");
		//else				stSaveFile.lpstrDefExt = TEXT("mlt");

		bOpened = GetSaveFileName( &stSaveFile );

		ViewFocusSet(  );

		if( !(bOpened) ){	return  E_ABORT;	}
		//	キャンセルしてたら何もしない

		//選択したフィルタ番号が１インデックスで入る
		if(  2 == stSaveFile.nFilterIndex ){	bUtf8 = TRUE;	}
		//	ＵＴＦ８で保存・ユニコードから変換すればよろし

		if( bUnic || bUtf8 )
		{	//	名無しのままエクスポートしようとしてたら無効
			if( NULL == (*gitFileIt).atFileName[0] )
			{
				MessageBox( hWnd, TEXT("先に通常の保存をしておいて欲しいのです。"), NULL, MB_OK | MB_ICONINFORMATION );
				return E_FAIL;
			}
		}
		else{	bLastChg = TRUE;	}	//	新規保存・リネーム保存
		//	特殊フォーマットの場合はエクスポートとし、内部状態には影響しないようにする
	}

	//	拡張子を確認・ドット込みだよ～ん
	ptExten = PathFindExtension( atFilePath );	//	拡張子が無いならNULL、というか末端になる
	if( 0 == *ptExten )
	{
		//	拡張子指定がないならそのまま対応のをくっつける
		StringCchCopy( ptExten, 5, aatExte[ddExten] );
		bExtChg = TRUE;
	}
	else	//	既存の拡張子があったら
	{
		StringCchCopy( atExBuf, 10, ptExten );
		CharLower( atExBuf );	//	比較のために小文字にしちゃう

		//	既存の拡張子が、ASTならそれを優先する
		if( !( StrCmp( atExBuf, aatExte[0] ) ) )	//	ASTであるなら
		{
			//	AST形式を維持する
			isAST = TRUE;	isMLT = FALSE;	ddExten = 0;
		}

		if( isAST )	//	ASTは優先的に適用
		{
			if( StrCmp( atExBuf , aatExte[0] ) )	//	もしASTじゃなかったら変更
			{
				StringCchCopy( ptExten, 5, aatExte[0] );
				bExtChg = TRUE;
			}
		}
		else if( isMLT )	//	名前無いけど複数頁ならMLTじゃないとダメ
		{
			if( StrCmp( atExBuf , aatExte[1] ) )	//	もしMLTじゃなかったら変更
			{
				StringCchCopy( ptExten, 5, aatExte[1] );
				bExtChg = TRUE;
			}
		}
		//	一枚なら、TXTでもMLTでも気にしなくてよかばい
	}


	//	上書きなら直前の状態のバックアップとか取るべき
	//	同じ名前のファイルがあれば、ってことで

	//	オリジナルファイル名に注意
	if( !(bUnic) &&  !(bUtf8) ){	StringCchCopy( (*gitFileIt).atFileName, MAX_PATH, atFilePath );	}

	hFile = CreateFile( atFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile )
	{
		MessageBox( hWnd, TEXT("ファイルを開けなかったですぅ"), NULL, MB_OK | MB_ICONERROR );
		return E_HANDLE;
	}

	iNullTmt = 1;
	iCrLf = CH_CRLF_CCH;
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );

	if( bUnic )	//	ユニコードモードなら、BOMをつける
	{
		WriteFile( hFile, &rtHead, 2, &wrote, NULL );
		iNullTmt = 2;
		iCrLf *= 2;
	}

	if( isAST )
	{
		pbSplit = NULL;
	}
	else
	{
		pbSplit = malloc( 30 );
		ZeroMemory( pbSplit, 30 );

		if( bUnic )
		{
			cbSplSz = (MLT_SPRT_CCH + CH_CRLF_CCH) * sizeof(TCHAR);
			StringCchPrintfW( (LPTSTR)pbSplit, 15, TEXT("%s%s"), MLT_SEPARATERW, CH_CRLFW );
		}
		else	//	UTF8の場合は、ASCII文字はそのままでおｋ
		{
			cbSplSz = MLT_SPRT_CCH + CH_CRLF_CCH;
			StringCchPrintfA( (LPSTR)pbSplit, 30, ("%s%s"), MLT_SEPARATERA, CH_CRLFA );
		}
	}

	//	本文の取込はユニコードでやる必要がある
	if( bUnic || bUtf8 ){	bStyle |= D_UNI;	}

	for( i = 0; iPages > i; i++ )	//	全頁保存
	{
		if( isAST )	//	ＡＳＴの場合は、頁先頭にタイトルが入ってる
		{
			//	返り値の確保バイト数にはＮＵＬＬターミネータ含んでるので注意
			cbSplSz = DocAstSeparatorGetAlloc( i, bStyle, &pbSplit, gitFileIt );

			if( bUtf8 ){	cbSplSz = DocUnicode2UTF8( &pbSplit );	}
			//	pbSplitの中身を付け替える

			WriteFile( hFile , pbSplit, (cbSplSz- iNullTmt), &wrote, NULL );
			FREE(pbSplit);
		}
		else	//	MLTの場合は、二つ目以降で区切りが必要
		{
			if( 1 <= i ){	WriteFile( hFile , pbSplit, cbSplSz, &wrote, NULL );	}
		}

		iByteSize = DocPageTextGetAlloc( gitFileIt, i, bStyle, &pBuffer, TRUE );

		if( bUtf8 ){	iByteSize = DocUnicode2UTF8( &pBuffer );	}
		//	pBufferの中身を付け替える

		if( (i+1) == iPages ){	iByteSize -=  iCrLf;	}	//	最終頁の末端の改行は不要のはず
		WriteFile( hFile, pBuffer, iByteSize - iNullTmt, &wrote, NULL );

		FREE( pBuffer );
	}

	SetEndOfFile( hFile );
	CloseHandle( hFile );

	FREE( pbSplit );

	//	エクスポートなので保存してないことに
	if( !(bUnic) &&  !(bUtf8) ){	DocModifyContent( FALSE );	}

	//	なんかメッセージ
	if( bExtChg )
	{
		//InitLastOpen( INIT_SAVE, atFilePath );	//	ラストオーポンを書換
		MultiFileTabRename( (*gitFileIt).dUnique, atFilePath );	//	タブ名称変更
		AppTitleChange( atFilePath );
		StringCchPrintf( atBuffer, MAX_STRING, TEXT("拡張子を %s にして保存したのです。あぅあぅ"), aatExte[ddExten] );
		NotifyBalloonExist( atBuffer, TEXT("拡張子を変更したのです"), NIIF_INFO );

		OpenHistoryLogging( hWnd , atFilePath );	//	ファイル名変更したので記録取り直し
	}
	else
	{
		//	20110713	新規かリネームしてたらラストオーポンを書換
		if( bLastChg )
		{
			//InitLastOpen( INIT_SAVE, atFilePath );
			MultiFileTabRename( (*gitFileIt).dUnique, atFilePath );	//	タブ名称変更
			AppTitleChange( atFilePath );

			OpenHistoryLogging( hWnd , atFilePath );	//	ファイル名変更したので記録取り直し
		}

		if( bUnic || bUtf8 )
		{
			NotifyBalloonExist( TEXT("保存したのです。あぅ"), TEXT("エクスポートしたのです"), NIIF_INFO );
		}
		else
		{
			NotifyBalloonExist( TEXT("保存したのです。あぅあぅ"), TEXT("ファイル出力したのです"), NIIF_INFO );
		}
	}

	return 0;
}
//-------------------------------------------------------------------------------------------------

/*!
	ユニコード文字列を受け取って、ＵＴＦ８のアドレスを確保してもどす。
	@param[in,out]	pText	動的ユニコード文字列受取・動的ＵＴＦ８文字列入れる。メモリの扱い注意
	@return	INT	確保したバイト数・NULLターミネータ含む
*/
INT DocUnicode2UTF8( LPVOID *pText )
{
	UINT_PTR	cchSz;	//	ユニコード用
	INT	cbSize, rslt;	//	UTF8用
	LPVOID		pUtf8;	//	確保

	StringCchLength( (LPTSTR)(*pText), STRSAFE_MAX_CCH, &cchSz );

	//	必要バイト数確認
	cbSize = WideCharToMultiByte( CP_UTF8, 0, (LPTSTR)(*pText), -1, NULL, 0, NULL, NULL );
	TRACE( TEXT("cbSize[%d]"), cbSize );
	pUtf8 = (LPSTR)malloc( cbSize );
	ZeroMemory( pUtf8, cbSize );
	rslt = WideCharToMultiByte( CP_UTF8, 0, (LPTSTR)(*pText), -1, (LPSTR)(pUtf8), cbSize, NULL, NULL );
	TRACE( TEXT("rslt[%d]"), rslt );

	FREE( *pText );	//	ユニコード文字列のほうは破壊する

	*pText = pUtf8;	//	ＵＴＦ８のほうに付け替える

	return cbSize;
}
//-------------------------------------------------------------------------------------------------

/*!
	ページ名前をAST区切り付きで確保する・freeは呼んだ方でやる
	@param[in]	dPage	確保する頁番号
	@param[in]	bStyle	１ユニコードかシフトJISで、矩形かどうか
	@param[out]	pText	確保した領域を返す・ワイド文字かマルチ文字になる・NULLだと必要バイト数を返すのみ
	@return				確保したバイト数・NULLターミネータ含む
*/
INT DocAstSeparatorGetAlloc( INT dPage, UINT bStyle, LPVOID *pText, FILES_ITR itFile )
{
	UINT	cchSize, cbSize;
	TCHAR	atBuffer[MAX_PATH];

	StringCchPrintf( atBuffer, MAX_PATH, TEXT("[AA][%s]\r\n"), itFile->vcCont.at( dPage ).atPageName );
	StringCchLength( atBuffer, MAX_PATH, &cchSize );

	if( bStyle & D_UNI )
	{
		cbSize = (cchSize + 1) * sizeof(TCHAR);	//	NULLターミネータ

		*pText = (LPTSTR)malloc( cbSize );
		ZeroMemory( *pText, cbSize );
		StringCchCopy( (LPTSTR)(*pText), (cchSize + 1), atBuffer );
	}
	else
	{
		cbSize = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, atBuffer, cchSize, NULL, 0, NULL, NULL );
		cbSize++;	//	NULLターミネータ
		*pText = (LPSTR)malloc( cbSize );
		ZeroMemory( *pText, cbSize );
		WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, atBuffer, cchSize, (LPSTR)(*pText), cbSize, NULL, NULL );
	}

	return cbSize;
}
//-------------------------------------------------------------------------------------------------Yippee-ki-yay!

/*!
	画像で頁を保存・BMPかPNG、JPEGは向いてない
	@param[in]	hWnd	親にするウインドウハンドル
	@param[in]	bStyle	なんかフラグ・とりやえず未使用
	@param[in]	hFont	描画に使うフォント受け取る
	@return		HRESULT	終了状態コード
*/
HRESULT DocImageSave( HWND hWnd, UINT bStyle, HFONT hFont )
{

	LPVOID	pBuffer;
	UINT	dLines;
	INT		iDotX, iDotY, iByteSize, bType;
	UINT_PTR	cchSize;
	RECT	rect;

	BOOL	bOpened;
	OPENFILENAME	stSaveFile;

	TCHAR	atOutName[MAX_PATH], atFileName[MAX_STRING];
	TCHAR	atPart[MIN_STRING];

	HDC		hdc, hMemDC;
	HBITMAP	hBitmap,hOldBmp;
	HFONT	hOldFont;



	//	とりあえづダミー名前でファイル
	StringCchCopy( atOutName, MAX_PATH, gitFileIt->atFileName );
	//	拡張子より選択を優先するようにしちゃう
	PathRemoveExtension( atOutName );	//	拡張子あぼ～ん

	StringCchPrintf( atPart, MIN_STRING, TEXT("_Page%d"), gixFocusPage );
	StringCchCat( atOutName, MAX_PATH, atPart );

	ZeroMemory( &stSaveFile, sizeof(OPENFILENAME) );
	stSaveFile.lStructSize     = sizeof(OPENFILENAME);
	stSaveFile.hwndOwner       = hWnd;
	stSaveFile.lpstrFilter     = TEXT("BMP ファイル ( *.bmp )\0*.bmp\0PNG ファイル ( *.png )\0*.png\0\0");
	stSaveFile.nFilterIndex    = 1;	//	デフォのフィルタ選択肢
	stSaveFile.lpstrFile       = atOutName;
	stSaveFile.nMaxFile        = MAX_PATH;
	stSaveFile.lpstrFileTitle  = atFileName;
	stSaveFile.nMaxFileTitle   = MAX_STRING;
//		stSaveFile.lpstrInitialDir = 
	stSaveFile.lpstrTitle      = TEXT("保存するファイル名と形式を指定するですぅ");
	stSaveFile.Flags           = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

	bOpened = GetSaveFileName( &stSaveFile );

	ViewFocusSet(  );

	if( !(bOpened) ){	return  E_ABORT;	}

	//選択したフィルタ番号が１インデックスで入る
	switch( stSaveFile.nFilterIndex )
	{
		default:	bType = ISAVE_BMP;	break;
		case  2:	bType = ISAVE_PNG;	break;
	}

	//	頁情報確保
	dLines = DocPageParamGet( NULL, NULL );
	iDotX  = DocPageMaxDotGet( -1, -1 );
	iDotY  = dLines * LINE_HEIGHT;
	//	ちゅっと余裕いれとく
	iDotX += 8;
	iDotY += 8;

	SetRect( &rect, 4, 4, iDotX - 4, iDotY- 4 );

	TRACE( TEXT("サイズ %d x %d"), iDotX, iDotY );

	iByteSize = DocPageTextGetAlloc( gitFileIt, gixFocusPage, D_UNI, &pBuffer, TRUE );
	StringCchLength( (LPTSTR)pBuffer, STRSAFE_MAX_CCH, &cchSize );

	//	描画用ビットマップ作成
	hdc = GetDC( hWnd );

	hBitmap = CreateCompatibleBitmap( hdc, iDotX, iDotY );
	hMemDC  = CreateCompatibleDC( hdc );

	hOldBmp  = SelectBitmap( hMemDC, hBitmap );
	hOldFont = SelectFont( hMemDC, hFont );

	PatBlt( hMemDC, 0, 0, iDotX, iDotY, WHITENESS );

	ReleaseDC( hWnd, hdc );

	DrawText( hMemDC, (LPTSTR)pBuffer, cchSize, &rect, DT_LEFT | DT_NOPREFIX | DT_NOCLIP | DT_WORDBREAK );

	FREE(pBuffer);

	if( SUCCEEDED( ImageFileSaveDC( hMemDC, atOutName, bType ) ) )
	{
		//	せいこう
		TRACE( TEXT("保存 %s"), atOutName );
	}
	else
	{
		//	しっぱい
		TRACE( TEXT("失敗 %s"), atOutName );
	}

	SelectBitmap( hMemDC, hOldBmp );
	DeleteBitmap( hBitmap );

	SelectFont( hMemDC, hOldFont );

	DeleteDC( hMemDC );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

