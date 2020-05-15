﻿/*! @file
	@brief ファイルに保存・開くの面倒見ます
	このファイルは DocFileCtrl.cpp です。
	@author	SikigamiHNQ
	@date	2011/04/27
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
#include "OrinrinEditor.h"
//-------------------------------------------------------------------------------------------------

//	TODO:	保存するとき、同じ名前のファイルがあれば、日時をつけて自動でバックアップを取る

//	TODO:	保存するとき、ユニコードかSJISか選択出来るように・デフォはSJISでいいか


extern list<ONEFILE>	gltMultiFiles;	//!<	複数ファイル保持
extern FILES_ITR	gitFileIt;			//!<	今見てるファイルの本体
extern INT		gixFocusPage;			//!<	注目中のページ・とりあえず０・０インデックス

extern  UINT	gbAutoBUmsg;			//		自動バックアップメッセージ出すか？

extern  UINT	gbSaveMsgOn;			//		保存メッセージ出すか？

static TCHAR	gatBackUpDirty[MAX_PATH];

//-------------------------------------------------------------------------------------------------

INT	DocAstSeparatorGetAlloc( FILES_ITR, INT, UINT, LPVOID * );

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
	stOpenFile.lpstrFilter     = TEXT("AA文件 ( mlt, ast, txt )\0*.mlt;*.ast;*.txt\0所有格式(*.*)\0*.*\0\0");
	stOpenFile.nFilterIndex    = 1;
	stOpenFile.lpstrFile       = atFilePath;
	stOpenFile.nMaxFile        = MAX_PATH;
	stOpenFile.lpstrFileTitle  = atFileName;
	stOpenFile.nMaxFileTitle   = MAX_STRING;
//	stOpenFile.lpstrInitialDir = 
	stOpenFile.lpstrTitle      = TEXT("请您指定一下要打开的文件吧");
	stOpenFile.Flags           = OFN_EXPLORER | OFN_HIDEREADONLY;
	stOpenFile.lpstrDefExt     = TEXT("mlt");

	//ここで FileOpenDialogue を出す
	bOpened = GetOpenFileName( &stOpenFile );

	ViewFocusSet(  );

	if( !(bOpened) ){	return  E_ABORT;	}	//	キャンセルしてたら何もしない

	DocDoOpenFile( hWnd, atFilePath );	//	ファイルを指定して読み込む時

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ファイル名を受けて、オーポン処理する
	@param[in]	hWnd	親にするウインドウハンドル
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

	dNumber = DocFileInflate( ptFile  );	//	ファイル名を受けて、開いて中身展開
	if( !(dNumber) )
	{
		MessageBox( hWnd, TEXT("文件打开失败了哦。"), TEXT("阿燐燐向您确认"), MB_OK | MB_ICONERROR );
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

	INT		isAST, isMLT, idExten;

	UINT_PTR	iPages, i;	//	頁数

	FILES_ITR	itFile;

	ZeroMemory( atFilePath, sizeof(atFilePath) );
	ZeroMemory( atFileName, sizeof(atFileName) );
	ZeroMemory( atBuffer,   sizeof(atBuffer) );

//複数ファイル、各ファイルをセーブするには？
	for( itFile = gltMultiFiles.begin(); itFile != gltMultiFiles.end(); itFile++ )
	{
		iPages = itFile->vcCont.size( );	//	総頁数

		if( 1 >= iPages )	isMLT = FALSE;
		else				isMLT = TRUE;

		isAST = PageListIsNamed( itFile );	//	頁に名前が付いてる？

		if( isAST ){		idExten = 0;	}	//	AST
		else if( isMLT ){	idExten = 1;	}	//	MLT
		else{				idExten = 2;	}	//	TXT

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
			StringCchCopy( ptExten, 5, aatExte[idExten] );
		}
		else	//	既存の拡張子があったら
		{
			StringCchCopy( atExBuf, 10, ptExten );
			CharLower( atExBuf );	//	比較のために小文字にしちゃう

			if( isAST )	//	ASTは優先的に適用
			{
				if( StrCmp( atExBuf , aatExte[0] ) )	//	もしASTじゃなかったら変更
				{
					StringCchCopy( ptExten, 5, aatExte[0] );
				}
			}
			else if( isMLT )	//	名前無いけど複数頁ならMLTじゃないとダメ
			{
				if( StrCmp( atExBuf , aatExte[1] ) )	//	もしMLTじゃなかったら変更
				{
					StringCchCopy( ptExten, 5, aatExte[1] );
				}
			}
			//	一枚なら、TXTでもMLTでも気にしなくてよかばい
		}

		StringCchCopy( atFilePath, MAX_PATH, gatBackUpDirty );
		PathAppend( atFilePath, atFileName );	//	Backupファイル名


		hFile = CreateFile( atFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if( INVALID_HANDLE_VALUE == hFile )
		{
			NotifyBalloonExist( TEXT("备份失败了哦……"), TEXT("发生异常"), NIIF_ERROR );
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
				cbSplSz = DocAstSeparatorGetAlloc( itFile, i, D_SJIS, &pbSplit );

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

	if( gbAutoBUmsg ){	NotifyBalloonExist( TEXT("已经将编辑状态的文件备份了哦。"), TEXT("阿燐燐向您确认"), NIIF_INFO );	}

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

	INT		isAST, isMLT, idExten, mbRslt;
	BOOLEAN	bExtChg =FALSE, bLastChg = FALSE;
	BOOLEAN	bForceMLT = FALSE;
	BOOLEAN	bNoName = FALSE;

	BOOLEAN	bUtf8 = FALSE;	//	ＵＴＦ８で保存セヨ
	BOOLEAN	bUnic = FALSE;	//	ユニコードで保存セヨ

	UINT_PTR	iPages, i;	//	頁数

	ZeroMemory( &stSaveFile, sizeof(OPENFILENAME) );

	ZeroMemory( atFilePath,  sizeof(atFilePath) );
	ZeroMemory( atFileName,  sizeof(atFileName) );
	ZeroMemory( atBuffer,  sizeof(atBuffer) );

	//	保存時は常に選択しているファイルを保存

	iPages = DocNowFilePageCount( );	//	総頁数
	if( 1 >= iPages )	isMLT = FALSE;
	else				isMLT = TRUE;

//既存の拡張子がASTなら、それを優先する

	isAST = PageListIsNamed( gitFileIt );	//	頁に名前が付いてる？

	//if( isAST ){		idExten = 0;	}	//	AST
	//else if( isMLT ){	idExten = 1;	}	//	MLT
	//else{				idExten = 2;	}	//	TXT
	//	txtは使用しないことにする
	if( isAST ){	idExten = 0;	}	//	AST
	else{			idExten = 1;	}	//	MLT


	GetLocalTime( &stSysTile );

	StringCchCopy( atFilePath, MAX_PATH, (*gitFileIt).atFileName );

	if( NULL == (*gitFileIt).atFileName[0] )	bNoName = TRUE;

	//	リネームか、ファイル名が無かったら保存ダイヤログ開く
	if( (bStyle & D_RENAME) || bNoName )
	{

		//ここで FileSaveDialogue を出す
		stSaveFile.lStructSize     = sizeof(OPENFILENAME);
		stSaveFile.hwndOwner       = hWnd;
		stSaveFile.lpstrFilter     = TEXT("[ShiftJIS]AA文件 ( mlt, ast, txt )\0*.mlt;*.ast;*.txt\0[UTF8]AA文件 ( mlt, ast, txt )\0*.mlt;*.ast;*.txt\0\0");
		stSaveFile.nFilterIndex    = 1;	//	デフォのフィルタ選択肢
		stSaveFile.lpstrFile       = atFilePath;
		stSaveFile.nMaxFile        = MAX_PATH;
		stSaveFile.lpstrFileTitle  = atFileName;
		stSaveFile.nMaxFileTitle   = MAX_STRING;
//		stSaveFile.lpstrInitialDir = 
		stSaveFile.lpstrTitle      = TEXT("请您指定要保存的文件名呢");
		stSaveFile.Flags           = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
		//デフォルト拡張子の指定・あとで面倒見てるからここではしない方が良い
		//	stSaveFile.lpstrDefExt = TEXT("ast");
		//	stSaveFile.lpstrDefExt = TEXT("mlt");

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
				MessageBox( hWnd, TEXT("请先使用SJIS编码保存一遍之后再进行其他格式的转换呢。"), TEXT("阿燐燐向您确认"), MB_OK | MB_ICONINFORMATION );
				return E_FAIL;
			}
		}
		else{	bLastChg = TRUE;	}	//	新規保存・リネーム保存
		//	特殊フォーマットの場合はエクスポートとし、内部状態には影響しないようにする
	}

	//	拡張子を確認・ドット込みだよ～ん・拡張子の位置のポインタ確保
	ptExten = PathFindExtension( atFilePath );	//	拡張子が無いなら末端になる
	if( 0 == *ptExten )
	{
		//	拡張子指定がないならそのまま対応のをくっつける
		StringCchCopy( ptExten, 5, aatExte[idExten] );
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
			isAST = TRUE;	isMLT = FALSE;	idExten = 0;
		}

		//	保存する拡張子がMLTで、既存のASTからリネームなら確認
		if( !( StrCmp( atExBuf, aatExte[1] ) ) )
		{
			if( isAST && (bStyle & D_RENAME) )	//	既存ASTかつリネームなら
			{
				mbRslt = MessageBox( hWnd, TEXT("使用MLT格式保存的话页名称就会丢失哦。\r\n真的关系吗？"), TEXT("阿燐燐向您确认"), MB_OKCANCEL | MB_ICONQUESTION );
				if( IDOK != mbRslt )	return E_ABORT;

				isMLT = TRUE;	isAST = FALSE;	idExten = 1;
				bForceMLT = TRUE;
			}
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
		MessageBox( hWnd, TEXT("打开文件失败了哦"), TEXT("阿燐燐向您确认"), MB_OK | MB_ICONERROR );
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
			cbSplSz = DocAstSeparatorGetAlloc( gitFileIt, i, bStyle, &pbSplit );

			if( bUtf8 ){	cbSplSz = DocUnicode2UTF8( &pbSplit );	}
			//	pbSplitの中身を付け替える

			WriteFile( hFile , pbSplit, (cbSplSz- iNullTmt), &wrote, NULL );
			FREE(pbSplit);
		}
		else	//	MLTの場合は、二つ目以降で区切りが必要
		{
			if( 1 <= i ){	WriteFile( hFile , pbSplit, cbSplSz, &wrote, NULL );	}
			if( bForceMLT ){	DocAstSeparatorGetAlloc( gitFileIt, i, 0, NULL );	}
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
	if( bExtChg )	//	拡張子変更した場合
	{
		//InitLastOpen( INIT_SAVE, atFilePath );	//	ラストオーポンを書換
		MultiFileTabRename( (*gitFileIt).dUnique, atFilePath );	//	タブ名称変更
		AppTitleChange( atFilePath );
		StringCchPrintf( atBuffer, MAX_STRING, TEXT("已经保存为 %s 格式的文件了哦。"), aatExte[idExten] );
		NotifyBalloonExist( atBuffer, TEXT("阿燐燐向您确认"), NIIF_INFO );

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
			NotifyBalloonExist( TEXT("文件的导出已经成功了哦。"), TEXT("阿燐燐向您确认"), NIIF_INFO );
		}
		else
		{
			if( gbSaveMsgOn ){	NotifyBalloonExist( TEXT("文件已经保存了哦。"), TEXT("阿燐燐向您确认"), NIIF_INFO );	}
		}
	}

	//	頁一覧の書き直し
	if( bForceMLT ){	PageListViewRewrite( -1 );	}

	return S_OK;
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
	@param[in]	itFile	確保する頁番号
	@param[in]	dPage	確保する頁番号
	@param[in]	bStyle	１ユニコードかシフトJISで、矩形かどうか
	@param[out]	pText	確保した領域を返す・ワイド文字かマルチ文字になる・NULLなら頁名を削除する
	@return				確保したバイト数・NULLターミネータ含む
*/
INT DocAstSeparatorGetAlloc( FILES_ITR itFile, INT dPage, UINT bStyle, LPVOID *pText )
{
	UINT	cchSize, cbSize;
	TCHAR	atBuffer[MAX_STRING];

	StringCchPrintf( atBuffer, MAX_STRING, TEXT("[AA][%s]\r\n"), itFile->vcCont.at( dPage ).atPageName );
	StringCchLength( atBuffer, MAX_STRING, &cchSize );

	if( !(pText) )
	{
		ZeroMemory( itFile->vcCont.at( dPage ).atPageName, SUB_STRING * sizeof(TCHAR) );
		return 0;
	}

	if( bStyle & D_UNI )
	{
		cbSize = (cchSize + 1) *  sizeof(TCHAR);	//	NULLターミネータ

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
	LPTSTR	ptText;
	UINT	dLines;
	INT		iDotX, iDotY, iByteSize, bType;
	UINT_PTR	cchSize;
	RECT	rect;

	INT	iLine;
	UINT_PTR	cchLen, start, caret = 0;

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
	stSaveFile.lpstrFilter     = TEXT("BMP 文件 ( *.bmp )\0*.bmp\0PNG 文件 ( *.png )\0*.png\0\0");
	stSaveFile.nFilterIndex    = 1;	//	デフォのフィルタ選択肢
	stSaveFile.lpstrFile       = atOutName;
	stSaveFile.nMaxFile        = MAX_PATH;
	stSaveFile.lpstrFileTitle  = atFileName;
	stSaveFile.nMaxFileTitle   = MAX_STRING;
//		stSaveFile.lpstrInitialDir = 
	stSaveFile.lpstrTitle      = TEXT("请您指定要保存的文件名和格式哦");
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

	dLines = DocNowFilePageLineCount(  );//DocPageParamGet( NULL , NULL );	//	要るのは行数
	iDotX  = DocPageMaxDotGet( -1, -1 );
	iDotY  = dLines * LINE_HEIGHT;
	//	ちゅっと余裕いれとく
	iDotX += 8;
	iDotY += 8;

	SetRect( &rect, 4, 4, iDotX - 4, iDotY- 4 );

	TRACE( TEXT("大小 %d x %d"), iDotX, iDotY );

	iByteSize = DocPageTextGetAlloc( gitFileIt, gixFocusPage, D_UNI, &pBuffer, TRUE );
	ptText = (LPTSTR)pBuffer;
	StringCchLength( ptText, STRSAFE_MAX_CCH, &cchSize );

	//	描画用ビットマップ作成
	hdc = GetDC( hWnd );

	hBitmap = CreateCompatibleBitmap( hdc, iDotX, iDotY );
	hMemDC  = CreateCompatibleDC( hdc );

	hOldBmp  = SelectBitmap( hMemDC, hBitmap );
	hOldFont = SelectFont( hMemDC, hFont );

	PatBlt( hMemDC, 0, 0, iDotX, iDotY, WHITENESS );

	ReleaseDC( hWnd, hdc );

	iLine  = 0;
	cchLen = 0;
	start  = 0;
	//	文字列全体を見ていく
	for( caret = 0; cchSize > caret; )
	{
		if( TEXT('\r') == ptText[caret] )	//	壱行の終わり
		{
			TextOut( hMemDC, 0, iLine, &(ptText[start]), cchLen );
			cchLen = 0;	//	文字数リセット
			caret += 2;	//	次の行の開始位置
			start = caret;	//	開始位置確認

			iLine += LINE_HEIGHT;	//	描画Ｙ位置
		}
		else
		{
			cchLen++;
			caret++;
		}
	}
	//	最後の行描画
	TextOut( hMemDC, 0, iLine, &(ptText[start]), cchLen );


	FREE(pBuffer);

	if( SUCCEEDED( ImageFileSaveDC( hMemDC, atOutName, bType ) ) )
	{
		//	せいこう
		TRACE( TEXT("保存 %s"), atOutName );
	}
	else
	{
		//	しっぱい
		TRACE( TEXT("失败 %s"), atOutName );
	}

	SelectBitmap( hMemDC, hOldBmp );
	DeleteBitmap( hBitmap );

	SelectFont( hMemDC, hOldFont );

	DeleteDC( hMemDC );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	MLT2HTMLを使って、今開いているファイルをHTMLエクスポート
	@param[in]	hWnd	親にするウインドウハンドル
	@return		HRESULT	終了状態コード
*/
HRESULT DocHtmlExport( HWND hWnd )
{
	TCHAR	atFilePath[MAX_PATH], atCommandLine[BIG_STRING + 10];
	TCHAR	atExePath[MAX_PATH];

	 PROCESS_INFORMATION	stProInfo;
	 STARTUPINFO	stStartInfo;



	ZeroMemory( atFilePath,  sizeof(atFilePath) );

	StringCchCopy( atFilePath, MAX_PATH, (*gitFileIt).atFileName );

	//	今開いているファイルが未保存なら、チューシ
	if( gitFileIt->dModify || ( NULL == atFilePath[0] ) )
	{
		MessageBox( hWnd, TEXT("请先保存文件再进行其他步骤哦。"), TEXT("文件还没有保存哦"), MB_OK | MB_ICONERROR );
		return E_ABORT;
	}
	PathQuoteSpaces( atFilePath );

	ZeroMemory( atExePath, sizeof(atExePath) );
	InitParamString( INIT_LOAD, VS_EXT_M2H_PATH, atExePath );
	if( NULL == atExePath[0] )
	{
		MessageBox( hWnd, TEXT("请先设定MLT2HTML.exe哦。"), TEXT("缺少外部工具哦"), MB_OK | MB_ICONERROR );
		return E_ABORT;
	}
	PathQuoteSpaces( atExePath );

	ZeroMemory( atCommandLine,  sizeof(atCommandLine) );

	StringCchPrintf( atCommandLine, BIG_STRING + 10, TEXT("%s %s"), atExePath, atFilePath );
	//	パスに含まれるスペースがあるとおかしくなったらコマルのでクオートしておく

	ZeroMemory( &stProInfo, sizeof(PROCESS_INFORMATION) );

	ZeroMemory( &stStartInfo, sizeof(STARTUPINFO) );
	stStartInfo.cb = sizeof(STARTUPINFO);

	CreateProcess( NULL, atCommandLine, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &stStartInfo, &stProInfo );

	CloseHandle( stProInfo.hThread );

	WaitForSingleObject( stProInfo.hProcess, INFINITE );	//	無限ウエイト・あぶないかも？

	CloseHandle( stProInfo.hProcess );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

