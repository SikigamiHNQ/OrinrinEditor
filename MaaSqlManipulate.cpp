/*! @file
	@brief SQLiteの操作をします。
	このファイルは MaaSqlManipulate.cpp です。
	@author	SikigamiHNQ
	@date	2011/06/22
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

#include "MaaTemplate.h"
//-------------------------------------------------------------------------------------------------



//	テーボーのバージョン
#define TREE_TBL_VER	1500	//	プロ・ファイル初期版
#define SUBTAB_NAME_VER	2000	//	副タブの表示名称変更に対応	20120613
//-------------------------------------------------------------------------------------------------

//	テーブル検索
CONST CHAR	cacDetectTable[] = { ("SELECT name FROM sqlite_master WHERE type='table'") };

//	バージョンテーブル
CONST CHAR	cacVersionTable[]  = { ("CREATE TABLE BuildVer ( id INTEGER PRIMARY KEY, number INTEGER NOT NULL, vertext TEXT NOT NULL)") };
CONST CHAR	cacVerStrInsFmt[]  = { ("INSERT INTO BuildVer ( number, vertext ) VALUES ( %d, '%s' )") };
CONST CHAR	cacVersionNumGet[] = { ("SELECT number FROM BuildVer WHERE id == 1") };
CONST CHAR	cacVersionUpdate[] = { ("UPDATE BuildVer SET number = %d, vertext = '%s' WHERE id == 1") };


static sqlite3	*gpDataBase;	//	使ったＡＡストア＋ツリーのキャッシュ

static sqlite3	*gpTreeCache;	//	ツリー編集のオンメモリキャッシュ

//-------------------------------------------------------------------------------------------------

HRESULT	SqlFavUpdate( UINT );	//!<	
HRESULT	SqlFavInsert( LPTSTR, DWORD, LPSTR, UINT );	//!<	
//-------------------------------------------------------------------------------------------------

//	WORK_LOG_OUT
//#ifdef _DEBUG
/*!
	SQLiteのエラー情報を出刃愚兎淫怒兎に表示
	@param[in]	*psqTarget	エラーチェキしたいデタベス
	@param[in]	dline		呼び出した地点の行数
*/
VOID SqlErrMsgView( sqlite3 *psqTarget, DWORD dline )
{
	//TCHAR	atStr[MAX_PATH];	//	そんなバッファ容量で大丈夫か
	//StringCchPrintf( atStr, MAX_PATH, TEXT("%s[%u]\r\n"), sqlite3_errmsg16( psqTarget ), dline );
	//OutputDebugString( atStr );
	TRACE( TEXT("%s[%u]"), sqlite3_errmsg16( psqTarget ), dline );
	return;
}
//-------------------------------------------------------------------------------------------------
//#endif

/*!
	元文字列がNULL指定だったらブッ飛ぶので対策
	@param[in]	ptDest	コピー先文字列ポインタ
	@param[in]	cchSize	コピー先文字列バッファの文字数
	@param[in]	ptSrc	元文字列
	@return		HRESULT	終了状態コード
*/
HRESULT String_Cch_Copy( LPTSTR ptDest, size_t cchSize, LPCTSTR ptSrc )
{
	ZeroMemory( ptDest, cchSize * sizeof(TCHAR) );

	if( !(ptSrc) )	return S_FALSE;

	return StringCchCopy( ptDest, cchSize , ptSrc );
}
//-------------------------------------------------------------------------------------------------



/*!
	SQLiteの使ったＡＡのデータベースを閉じたり開いたり
	@param[in]	mode	開くのか閉じるのか
	@param[in]	ptDbase	開くデータベースファイルの名前
	@return		HRESULT	終了状態コード
*/
HRESULT SqlDatabaseOpenClose( BYTE mode, LPCTSTR ptDbase )
{
	INT		rslt;
	TCHAR	atDbPath[MAX_PATH];

	if( M_CREATE == mode )
	{
		StringCchPrintf( atDbPath, MAX_PATH, ptDbase );

		rslt = sqlite3_open16( atDbPath, &gpDataBase );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_FAIL;	}

		if( 0 == StrCmp( atDbPath, TEXT(":memory:") ) )
		{
			StringCchCopy( atDbPath, MAX_PATH, TEXT("memory.qor") );
		}

		PathStripPath( atDbPath );	//	ファイル名だけにする
		PathRemoveExtension( atDbPath );	//	拡張子を外す

		SqlTreeTableCreate( atDbPath );	//	プロファイル名として渡す
	}
	else if( M_DESTROY == mode )
	{
		if( gpDataBase )
		{
			rslt =  sqlite3_close( gpDataBase );
			gpDataBase = NULL;
		}
	}
	else	//	オーポンしてるかどうか確認
	{
		if( !(gpDataBase) ){	return E_OUTOFMEMORY;	}
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	登録されている個数とID最大値確保・ディレクトリ毎か全体ＯＫ
	@param[in]	ptFdrName	対象ディレクトリ名・ＮＵＬＬなら全部
	@param[in]	pdMax		非ＮＵＬＬなら、最大値いれちゃう
	@return		登録されている個数
*/
UINT SqlFavCount( LPCTSTR ptFdrName, PUINT pdMax )
{
	TCHAR	atQuery[MAX_STRING];
	UINT	rslt, iCount, iMax;
	sqlite3_stmt*	statement;

	if( ptFdrName )
	{
		StringCchPrintf( atQuery, MAX_STRING, TEXT("SELECT COUNT(id) FROM ArtList WHERE folder == '%s'"), ptFdrName );
	}
	else
	{
		StringCchCopy( atQuery, MAX_STRING, TEXT("SELECT COUNT(id) FROM ArtList") );
	}


	rslt = sqlite3_prepare16( gpDataBase, atQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	sqlite3_step( statement );
	iCount = sqlite3_column_int( statement, 0 );
	sqlite3_finalize(statement);

	if( !pdMax )	return iCount;


	if( !ptFdrName )
	{
		StringCchPrintf( atQuery, MAX_STRING, TEXT("SELECT MAX(id) FROM ArtList WHERE folder == '%s'"), ptFdrName );
	}
	else
	{
		StringCchCopy( atQuery, MAX_STRING, TEXT("SELECT MAX(id) FROM ArtList") );
	}

	rslt = sqlite3_prepare16( gpDataBase, atQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	sqlite3_step( statement );
	iMax = sqlite3_column_int( statement, 0 );
	sqlite3_finalize(statement);

	*pdMax = iMax;

	return iCount;
}
//-------------------------------------------------------------------------------------------------

/*!
	使ったAAを、リスト確認して追加したり更新したり
	@param[in]	ptBaseName	基点ディレクトリ名
	@param[in]	dHash		ＡＡ認識用のハッシュ値
	@param[in]	pcConts		ＡＡ本体
	@param[in]	rdLength	バイト数
	@return		HRESULT	終了状態コード
*/
HRESULT SqlFavUpload( LPTSTR ptBaseName, DWORD dHash, LPSTR pcConts, UINT rdLength )
{
	CONST CHAR	acArtSelect[] = { ("SELECT id, folder FROM ArtList WHERE hash == ?") };
	INT		rslt;
	UINT	index, cntID, d;
	//LPCTSTR	ptFolder;
	TCHAR	atFolder[MAX_PATH];
	BOOLEAN	bIsExist;
	sqlite3_stmt	*statement;

	rslt = sqlite3_prepare( gpDataBase, acArtSelect, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}

	bIsExist = FALSE;
	cntID = SqlFavCount( NULL, NULL );

//先に登録した後、そのＡＡのベースネームが変わった場合、新しい名前で出てこない問題
//ハッシュで見てるので、ＡＡ自体は一つしか登録されない
//＞ベース名まで確認すればいい。ヒットしなくなるまで回して、ベース名がなかったら新規追加
	sqlite3_reset( statement );
	sqlite3_bind_int( statement, 1, dHash );	//	hash

	for( d = 0; cntID >  d; d++ )	//	全数回せばおｋ
	{
		rslt = sqlite3_step( statement );
		if( SQLITE_ROW == rslt )	//	存在した
		{
			ZeroMemory( atFolder, sizeof(atFolder) );

			index = sqlite3_column_int( statement , 0 );	//	id
			String_Cch_Copy( atFolder, MAX_PATH, (LPCTSTR)sqlite3_column_text16( statement, 1 ) );	//	folder
			//ptFolder = (LPCTSTR)sqlite3_column_text16( statement, 1 );
			//if( ptFolder ){	StringCchCopy( atFolder, MAX_PATH, ptFolder );	}

			SqlFavUpdate( index );	//	内容更新

			//	ベース名チェキ・同じのがあればおｋ
			if( !( StrCmp( ptBaseName, atFolder ) ) ){	bIsExist = TRUE;	}
		}
		else{	break;	}	//	それ以上ないなら終わる
	}

	sqlite3_finalize( statement );


	if( !(bIsExist) )	//	未記録のなら項目追加
	{
		SqlFavInsert( ptBaseName, dHash, pcConts, rdLength );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	使ったAAの記録を更新
	@param[in]	index	使ったAAのID番号
	@return		HRESULT	終了状態コード
*/
HRESULT SqlFavUpdate( UINT index )
{
	CONST CHAR	acArtSelCount[] = { ("SELECT count FROM ArtList WHERE id == %u") };
	CONST CHAR	acArtUpdate[] = { ("UPDATE ArtList SET count = %u, lastuse = %f WHERE id == %u") };

	CHAR	acQuery[MAX_STRING];
	UINT	iCount, rslt;
	DOUBLE	ddJulius;
	sqlite3_stmt	*statement;

	StringCchPrintfA( acQuery, MAX_STRING, acArtSelCount, index );
	//	まずカウント値取得して
	rslt = sqlite3_prepare( gpDataBase, acQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}
	rslt = sqlite3_step( statement );
	iCount = sqlite3_column_int( statement , 0 );	//	count
	sqlite3_finalize( statement );

	iCount++;	//	使用回数である・増やす

	//	今のユリウス時間をゲット
	rslt = sqlite3_prepare( gpDataBase, ("SELECT julianday('now')"), -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}
	sqlite3_step( statement );
	ddJulius = sqlite3_column_double( statement, 0 );
	sqlite3_finalize( statement );

	StringCchPrintfA( acQuery, MAX_STRING, acArtUpdate, iCount, ddJulius, index );
	rslt = sqlite3_prepare( gpDataBase, acQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}
	rslt = sqlite3_step( statement );
	if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	}

	sqlite3_finalize(statement);

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	使ったAAを記録
	@param[in]	ptBaseName	基点ディレクトリ名
	@param[in]	dHash		ＡＡ認識用のハッシュ値
	@param[in]	pcConts		ＡＡ本体
	@param[in]	rdLength	バイト数
	@return		HRESULT	終了状態コード
*/
HRESULT SqlFavInsert( LPTSTR ptBaseName, DWORD dHash, LPSTR pcConts, UINT rdLength )
{
	CONST CHAR	acArtInsert[] = { ("INSERT INTO ArtList ( count, folder, lastuse, hash, conts ) VALUES ( ?, ?, ?, ?, ? )") };

	INT		rslt;
	CHAR	acText[MAX_PATH];
	DOUBLE	ddJulius;
	sqlite3_stmt	*statement;

	ZeroMemory( acText, sizeof(acText) );

	//	今のユリウス時間をゲット
	rslt = sqlite3_prepare( gpDataBase, ("SELECT julianday('now')"), -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}
	sqlite3_step( statement );
	ddJulius = sqlite3_column_double( statement, 0 );
	sqlite3_finalize( statement );


	rslt = sqlite3_prepare( gpDataBase, acArtInsert, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}
	sqlite3_reset( statement );
	rslt = sqlite3_bind_int(    statement, 1, 1 );			//	count
	rslt = sqlite3_bind_text16( statement, 2, ptBaseName, -1, SQLITE_STATIC );	//	folder
	rslt = sqlite3_bind_double( statement, 3, ddJulius );	//	lastuse
	rslt = sqlite3_bind_int(    statement, 4, dHash );		//	hash
	rslt = sqlite3_bind_blob(   statement, 5, pcConts, rdLength, SQLITE_STATIC );	//	conts

	rslt = sqlite3_step( statement );
	if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	}
	sqlite3_finalize(statement);

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	登録されているルート名をリストアップしてコールバックする
	@param[in]	pfFolderNameSet	内容をコールバックする函数ポインター
	@return		HRESULT			終了状態コード
*/
HRESULT SqlFavFolderEnum( BUFFERBACK pfFolderNameSet )
{
	CONST CHAR	acQuery[] = { ("SELECT DISTINCT folder FROM ArtList") };

	INT		rslt, count, i;
	sqlite3_stmt	*statement;


	if( !(pfFolderNameSet) ){	return E_INVALIDARG;	}
	if( !(gpDataBase) ){	return E_OUTOFMEMORY;	}

	count = SqlFavCount( NULL, NULL );

	rslt = sqlite3_prepare( gpDataBase, acQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_FAIL;	}
	rslt = sqlite3_reset( statement );

	for( i = 0; count > i; i++ )
	{
		rslt = sqlite3_step( statement );
		if( SQLITE_ROW == rslt )
		{
			pfFolderNameSet( i, 0, 0, sqlite3_column_text16( statement, 0 ) );
		}
		else{	break;	}
	}

	sqlite3_finalize(statement);

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	記録されてるAAを、ディレクトリ名に従ってリストアップしてコールバックする
	@param[in]	ptFdrName	ディレクトリ名・NULLなら全部
	@param[in]	pfInflate	AAをコールバックする函数ポインター
	@return		HRESULT			終了状態コード
*/
HRESULT SqlFavArtEnum( LPCTSTR ptFdrName, BUFFERBACK pfInflate )
{
	TCHAR	atQuery[MAX_STRING];
	UINT	rslt, iCount, i, szCont;
	sqlite3_stmt*	statement;

	if( ptFdrName )
	{
		StringCchPrintf( atQuery, MAX_STRING, TEXT("SELECT conts FROM ArtList WHERE folder == '%s' ORDER BY lastuse DESC"), ptFdrName );
	}
	else
	{
		StringCchCopy( atQuery, MAX_STRING, TEXT("SELECT conts FROM ArtList ORDER BY lastuse") );
	}

	rslt = sqlite3_prepare16( gpDataBase, atQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_FAIL;	}
	rslt = sqlite3_reset( statement );

	iCount = SqlFavCount( ptFdrName, NULL );

	for( i = 0; iCount > i; i++ )
	{
		rslt = sqlite3_step( statement );
		if( SQLITE_ROW == rslt )
		{
			szCont = sqlite3_column_bytes( statement, 0 );
			pfInflate( szCont, 0, 0, sqlite3_column_blob( statement, 0 ) );
		}
		else{	break;	}
	}

	sqlite3_finalize(statement);

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	お気に入りリストから削除する
	@param[in]	ptBaseName	基点ディレクトリ名・使ってない
	@param[in]	dHash		ＡＡ認識用のハッシュ値
	@return		HRESULT		終了状態コード
*/
HRESULT SqlFavDelete( LPTSTR ptBaseName, DWORD dHash )
{
	CONST CHAR	acArtDelete[] = { ("DELETE FROM ArtList WHERE hash == ?") };
	INT		rslt;
	sqlite3_stmt	*statement;

	rslt = sqlite3_prepare( gpDataBase, acArtDelete, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}
	sqlite3_reset( statement );
	sqlite3_bind_int( statement, 1, dHash );	//	hash
	rslt = sqlite3_step( statement );
	sqlite3_finalize( statement );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	お気に入りリストを基点ごと削除
	@param[in]	ptBaseName	基点ディレクトリ名
	@return		HRESULT		終了状態コード
*/
HRESULT SqlFavFolderDelete( LPTSTR ptBaseName )
{
	CONST CHAR	acFolderDelete[] = { ("DELETE FROM ArtList WHERE folder == ?") };
	INT		rslt;
	sqlite3_stmt	*statement;

	rslt = sqlite3_prepare( gpDataBase, acFolderDelete, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}
	sqlite3_reset( statement );
	sqlite3_bind_text16( statement, 1, ptBaseName, -1, SQLITE_STATIC );	//	folder
	rslt = sqlite3_step( statement );
	sqlite3_finalize( statement );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------




#if 0
/*!
	SQLiteの使ったＡＡのデータベースを閉じたり開いたり
	@param[in]	mode		開くのか閉じるのか
	@param[in]	ptDbName	開くデータベースファイル名前
	@return		HRESULT		終了状態コード
*/
HRESULT SqlTreeOpenClose( BYTE mode, LPCTSTR ptDbName )
{
	INT		rslt;
	TCHAR	atDbPath[MAX_PATH];

	if( M_CREATE == mode )
	{
		StringCchPrintf( atDbPath, MAX_PATH, ptDbName );

		rslt = sqlite3_open16( atDbPath, &gpTreeCache );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpTreeCache );	return E_FAIL;	}

		if( 0 == StrCmp( atDbPath, TEXT(":memory:") ) )
		{
			StringCchCopy( atDbPath, MAX_PATH, TEXT("memory.qor") );
		}

		PathStripPath( atDbPath );	//	ファイル名だけにする
		PathRemoveExtension( atDbPath );	//	拡張子を外す

		SqlTreeTableCreate( atDbPath );	//	プロファイル名として渡す
	}
	else if( M_DESTROY == mode )
	{
		if( gpTreeCache )
		{
			rslt =  sqlite3_close( gpTreeCache );
			gpTreeCache = NULL;
		}
	}
	else	//	オーポンしてるかどうか確認
	{
		if( !(gpTreeCache) ){	return E_OUTOFMEMORY;	}
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------
#endif

/*!
	SQLiteのTransactionをしたりやめたり
	@param[in]	mode	非０開始　０終了
	@return		HRESULT	終了状態コード
*/
HRESULT SqlTransactionOnOff( BYTE mode )
{
	if( mode )
	{
		sqlite3_exec( gpDataBase, "BEGIN TRANSACTION", NULL, NULL, NULL );	//	トランザクション開始
	}
	else
	{
		sqlite3_exec( gpDataBase, "COMMIT TRANSACTION", NULL, NULL, NULL );	//	トランザクション終了
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ツリーのテーブルの存在を確認して、なかったら作成
	@param[in]	ptProfName	なにか
	@return		HRESULT		終了状態コード
*/
HRESULT SqlTreeTableCreate( LPTSTR ptProfName )
{
	//	プロフ情報
	CONST CHAR	cacProfilesTable[] = { ("CREATE TABLE Profiles ( id INTEGER PRIMARY KEY, profname TEXT NOT NULL, rootpath TEXT NULL)") };
	//	初期プロフ名作成
	CONST TCHAR	catProfInsFmt[] = { TEXT("INSERT INTO Profiles ( profname ) VALUES( '%s' )") };
	//	ツリー情報
	CONST CHAR	cacTreeNodeTable[] = { ("CREATE TABLE TreeNode ( id INTEGER PRIMARY KEY, type INTEGER NOT NULL, parentid INTEGER NOT NULL, nodename TEXT NOT NULL )") };
	//	副タブ情報
	CONST CHAR	cacMultiTabTable[] = { ("CREATE TABLE MultiTab ( id INTEGER PRIMARY KEY, filepath TEXT NOT NULL, basename TEXT NOT NULL, dispname TEXT )") };	//	20120613	表示名称追加
	//	使用ＡＡ情報
	CONST CHAR	cacArtListTable[]  = { ("CREATE TABLE ArtList ( id INTEGER PRIMARY KEY, count INTEGER NOT NULL, folder TEXT NOT NULL, lastuse  REAL NOT NULL, hash INTEGER NOT NULL, conts BLOB NOT NULL )") };

	CHAR	acBuildVer[SUB_STRING], acText[MAX_PATH];
	TCHAR	atStr[MAX_STRING];
	SYSTEMTIME	stSysTime;

	BYTE	yMode = FALSE;	//	Tableあったとかなかったとか
	INT		rslt;

	UINT	dVersion;
	CHAR	acQuery[MAX_PATH];

#ifdef _DEBUG
	INT		i;
#endif
	sqlite3_stmt	*statement;

	ZeroMemory( acText, sizeof(acText) );

	//	テーブルの存在を確認する
	rslt = sqlite3_prepare( gpDataBase, cacDetectTable, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}

	rslt = sqlite3_step( statement );
	if( SQLITE_ROW == rslt )
	{
#ifdef _DEBUG
		for( i = 0; 10 > i; i++ )
		{
			TRACE( TEXT("[TREE] %s"), (LPCTSTR)sqlite3_column_text16( statement, 0 ) );
			rslt = sqlite3_step( statement );
			if( SQLITE_DONE == rslt ){	break;	}
		}
#endif
		rslt = sqlite3_finalize( statement );
		yMode = TRUE;
	}
	//	まず、てーぶりょが有るか無いかについて

	if( !(yMode) )
	{

		TRACE( TEXT("ツリー用テーブルが見つからなかった") );

		rslt = sqlite3_finalize( statement );	//	どっちにしても解除


	//VERSION番号テーブル作成＜表には出ない
		rslt = sqlite3_prepare( gpDataBase, cacVersionTable, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		rslt = sqlite3_step( statement );	//	実行
		if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		rslt = sqlite3_finalize(statement);

		//VERSION内容を作成	20120613変更
		GetLocalTime( &stSysTime );
		StringCchPrintfA( acBuildVer, SUB_STRING, ("%d.%02d%02d.%02d%02d.%d"), stSysTime.wYear, stSysTime.wMonth, stSysTime.wDay, stSysTime.wHour, stSysTime.wMinute, SUBTAB_NAME_VER );
		StringCchPrintfA( acText, MAX_PATH, cacVerStrInsFmt, SUBTAB_NAME_VER, acBuildVer );
		//	初期データぶち込む
		rslt = sqlite3_prepare( gpDataBase, acText, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		sqlite3_reset( statement );
		rslt = sqlite3_step( statement );
		sqlite3_finalize(statement);


	//プロフテーブルを作成
		rslt = sqlite3_prepare( gpDataBase, cacProfilesTable, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		rslt = sqlite3_step( statement );	//	実行
		if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		rslt = sqlite3_finalize(statement);

		//初期値を作成
		ZeroMemory( atStr, sizeof(atStr) );
		StringCchPrintf( atStr, MAX_STRING, catProfInsFmt, ptProfName );
		rslt = sqlite3_prepare16( gpDataBase, atStr, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		sqlite3_reset( statement );
		rslt = sqlite3_step( statement );
		sqlite3_finalize(statement);

	//ツリーテーブルを生成
		rslt = sqlite3_prepare( gpDataBase, cacTreeNodeTable, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		rslt = sqlite3_step( statement );	//	実行
		if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		rslt = sqlite3_finalize(statement);

	//副タブテーブルを作成
		rslt = sqlite3_prepare( gpDataBase, cacMultiTabTable, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		rslt = sqlite3_step( statement );	//	実行
		if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		rslt = sqlite3_finalize(statement);

	//使用ＡＡテーブルを作成	
		rslt = sqlite3_prepare( gpDataBase, cacArtListTable, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		rslt = sqlite3_step( statement );	//	実行
		if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		rslt = sqlite3_finalize(statement);

	}
	else	//	存在してるなら、ヴァージョンを確かめてーテーボーを変更
	{
		dVersion = 0;
		rslt = sqlite3_prepare( gpDataBase, cacVersionNumGet, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
		rslt = sqlite3_step( statement );
		if( SQLITE_ROW == rslt ){	dVersion = sqlite3_column_int( statement, 0 );	}
		sqlite3_finalize( statement );
		TRACE( TEXT("PROFILE VERSION[%d]"), dVersion );



		//	副タブの表示名称を追加
		if( TREE_TBL_VER == dVersion )
		{
			//	テーブルに追加
			rslt = sqlite3_prepare( gpDataBase, ("ALTER TABLE MultiTab ADD COLUMN dispname TEXT DEFAULT \"\" "), -1, &statement, NULL );
			if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
			rslt = sqlite3_step( statement );	//	実行
			if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}
			rslt = sqlite3_finalize( statement );

			//バージョン情報を書き換え
			GetLocalTime( &stSysTime  );	//	更新日時とバージョン
			StringCchPrintfA( acBuildVer, SUB_STRING, ("%d.%02d%02d.%02d%02d.%d"),
				stSysTime.wYear, stSysTime.wMonth, stSysTime.wDay,
				stSysTime.wHour, stSysTime.wMinute, SUBTAB_NAME_VER );

			//	Query文字列作って
			StringCchPrintfA( acQuery, MAX_PATH, cacVersionUpdate, SUBTAB_NAME_VER, acBuildVer );

			//	情報書き換える
			rslt = sqlite3_prepare( gpDataBase, acQuery, -1, &statement, NULL);
			if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_ACCESSDENIED;	}

			sqlite3_reset( statement );
			rslt = sqlite3_step( statement );
			sqlite3_finalize( statement );

			dVersion = SUBTAB_NAME_VER;
		}
	}


	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	登録されている個数とID最大値確保
	@param[in]	bType	０プロフ・１ツリー・２副タブ・３ツリーキャッシュ
	@param[out]	pdMax	非ＮＵＬＬなら、最大値いれちゃう
	@return		登録されている個数
*/
UINT SqlTreeCount( UINT bType, PUINT pdMax )
{
	CHAR	acQuery[MAX_STRING];
	UINT	rslt, iCount, iMax;
	sqlite3_stmt*	statement;

	sqlite3	*pDB;

	switch( bType )
	{
		case  0:	pDB = gpDataBase;	StringCchCopyA( acQuery, MAX_STRING, ("SELECT COUNT(id) FROM Profiles") );	break;
		case  1:	pDB = gpDataBase;	StringCchCopyA( acQuery, MAX_STRING, ("SELECT COUNT(id) FROM TreeNode") );	break;
		case  2:	pDB = gpDataBase;	StringCchCopyA( acQuery, MAX_STRING, ("SELECT COUNT(id) FROM MultiTab") );	break;
		case  3:	pDB = gpTreeCache;	StringCchCopyA( acQuery, MAX_STRING, ("SELECT COUNT(id) FROM TreeNode") );	break;
		default:	return 0;
	}


	rslt = sqlite3_prepare( pDB, acQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( pDB );	return 0;	}

	sqlite3_step( statement );
	iCount = sqlite3_column_int( statement, 0 );
	sqlite3_finalize(statement);

	if( !pdMax )	return iCount;

	switch( bType )
	{
		case  0:	StringCchCopyA( acQuery, MAX_STRING, ("SELECT MAX(id) FROM Profiles") );	break;
		case  1:	StringCchCopyA( acQuery, MAX_STRING, ("SELECT MAX(id) FROM TreeNode") );	break;
		case  2:	StringCchCopyA( acQuery, MAX_STRING, ("SELECT MAX(id) FROM MultiTab") );	break;
		case  3:	StringCchCopyA( acQuery, MAX_STRING, ("SELECT MAX(id) FROM TreeNode") );	break;
		default:	return 0;
	}

	rslt = sqlite3_prepare( pDB, acQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( pDB );	return 0;	}

	sqlite3_step( statement );
	iMax = sqlite3_column_int( statement, 0 );
	sqlite3_finalize( statement );

	*pdMax = iMax;

	return iCount;
}
//-------------------------------------------------------------------------------------------------

/*!
	プロフ名、ルートパスを変更
	@param[in]	ptProfName	プロフ名
	@param[in]	ptRootPath	AAのルートダディレクトリ名
	@return	HRESULT	終了状態コード
*/
HRESULT SqlTreeProfUpdate( LPCTSTR ptProfName, LPCTSTR ptRootPath )
{
	//	１ファイルに付き１レコードしか存在しないので、IDは１固定
	CONST  TCHAR	catUpdateName[] = { TEXT("UPDATE Profiles SET profname = '%s' WHERE id == 1") };
	CONST  TCHAR	catUpdateRoot[] = { TEXT("UPDATE Profiles SET rootpath = '%s' WHERE id == 1") };

	TCHAR	atText[MAX_PATH];
	INT		rslt;
	sqlite3_stmt	*statement;

	//	トランザクションのON/OFFは必要に応じて外でヤル

	if( ptProfName )
	{
		StringCchPrintf( atText, MAX_PATH, catUpdateName, ptProfName );

		rslt = sqlite3_prepare16( gpDataBase, atText, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}
		rslt = sqlite3_step( statement );	if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	}
		sqlite3_finalize( statement );
	}

	if( ptRootPath )
	{
		StringCchPrintf( atText, MAX_PATH, catUpdateRoot, ptRootPath );

		rslt = sqlite3_prepare16( gpDataBase, atText, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}
		rslt = sqlite3_step( statement );	if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	}
		sqlite3_finalize( statement );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	プロフ名、ルートパスを確保
	@param[out]	ptProfName	プロフ名入れる
	@param[in]	szName		プロフ名の文字数
	@param[out]	ptRootPath	AAのルートダディレクトリ名
	@param[in]	szRoot		ルートダディの文字数
	@return	HRESULT	終了状態コード
*/
HRESULT SqlTreeProfSelect( LPTSTR ptProfName, UINT szName, LPTSTR ptRootPath, UINT szRoot )
{
	CONST CHAR	cacSelectQuery[] = { ("SELECT * FROM Profiles WHERE id == 1") };

	//LPTSTR	ptBuf;
	TCHAR	atName[MAX_STRING], atRoot[MAX_PATH];
	INT		index;
	INT		rslt;
	sqlite3_stmt	*statement;

	ZeroMemory( atName, sizeof(atName) );
	ZeroMemory( atRoot, sizeof(atRoot) );

	//	クェリセット
	rslt = sqlite3_prepare( gpDataBase, cacSelectQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}

	rslt = sqlite3_reset( statement );

	//	内容を読み出し
	rslt = sqlite3_step( statement );
	if( SQLITE_ROW == rslt )
	{
		index = sqlite3_column_int( statement , 0 );	//	id
		String_Cch_Copy( atName , MAX_STRING, (LPTSTR)sqlite3_column_text16( statement, 1 ) );	//	profname
		String_Cch_Copy( atRoot , MAX_STRING, (LPTSTR)sqlite3_column_text16( statement, 2 ) );	//	rootpath
		//ptBuf = (LPTSTR)sqlite3_column_text16( statement, 2 );	//	rootpath
		//if( ptBuf ){	StringCchCopy( atRoot , MAX_PATH, ptBuf );	}
	}
	sqlite3_finalize( statement );

	if( ptProfName )	StringCchCopy( ptProfName, szName, atName );
	if( ptRootPath )	StringCchCopy( ptRootPath, szRoot, atRoot );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------


/*!
	ディレクトリかファイルのデータをドピュッ
	@param[in]	uqID	割当済のＩＤ番号
	@param[in]	dType	ディレクトリかファイルか
	@param[in]	dPrnt	親ツリーノードのSQL的ＩＤ番号
	@param[in]	ptName	ノードの名称
	@return	UINT	いま登録したID番号
*/
UINT SqlTreeNodeInsert( UINT uqID, UINT dType, UINT dPrnt, LPTSTR ptName )
{
	CONST CHAR	acTreeNodeInsert[] = { ("INSERT INTO TreeNode ( id, type, parentid, nodename ) VALUES ( ?, ?, ?, ? )") };
	CONST CHAR	acAddNumCheck[] = { ("SELECT LAST_INSERT_ROWID( ) FROM TreeNode") };

	INT		rslt;
	UINT	iRast = 0;
	sqlite3_stmt	*statement;
	

	if( !(gpDataBase) ){	TRACE( TEXT("NoDatabase") );	return 0;	}

	rslt = sqlite3_prepare( gpDataBase, acTreeNodeInsert, -1, &statement, NULL);
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	sqlite3_reset( statement );
	rslt = sqlite3_bind_int(    statement, 1, uqID );	//	id
	rslt = sqlite3_bind_int(    statement, 2, dType );	//	type
	rslt = sqlite3_bind_int(    statement, 3, dPrnt );	//	parentid
	rslt = sqlite3_bind_text16( statement, 4, ptName, -1, SQLITE_STATIC );	//	nodename

	rslt = sqlite3_step( statement );
	if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	}

	sqlite3_finalize( statement );

	//	今追加したやつのアレを取得
	rslt = sqlite3_prepare( gpDataBase, acAddNumCheck, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	rslt = sqlite3_step( statement );

	iRast = sqlite3_column_int( statement, 0 );

	sqlite3_finalize( statement );

	return iRast;
}
//-------------------------------------------------------------------------------------------------

#ifdef EXTRA_NODE_STYLE

/*!
	エキストラファイルのデータをドピュッ
	@param[in]	dType	１ディレクトリか０ファイルか・とりあえず無視
	@param[in]	ptName	ノードの名称
	@return	UINT	いま登録したID番号
*/
UINT SqlTreeNodeExtraInsert( UINT dType, LPCTSTR ptName )
{
	CONST CHAR	acTreeNodeExIns[] = { ("INSERT INTO TreeNode ( type, parentid, nodename ) VALUES ( ?, ?, ? )") };
	CONST CHAR	acAddNumCheck[] = { ("SELECT LAST_INSERT_ROWID( ) FROM TreeNode") };

	INT		rslt;
	UINT	iRast = 0;
	sqlite3_stmt	*statement;

	if( !(gpDataBase) ){	TRACE( TEXT("NoDatabase") );	return 0;	}

	rslt = sqlite3_prepare( gpDataBase, acTreeNodeExIns, -1, &statement, NULL);
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	//	ファイル固定、親ＩＤは－１固定
	sqlite3_reset( statement );
	rslt = sqlite3_bind_int(    statement, 1, 0 );	//	type
	rslt = sqlite3_bind_int(    statement, 2, -1 );	//	parentid
	rslt = sqlite3_bind_text16( statement, 3, ptName, -1, SQLITE_STATIC );	//	nodename

	rslt = sqlite3_step( statement );
	if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	}

	sqlite3_finalize( statement );

	//	今追加したやつのアレを取得・ＩＤはいつでもユニークである
	rslt = sqlite3_prepare( gpDataBase, acAddNumCheck, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	rslt = sqlite3_step( statement );

	iRast = sqlite3_column_int( statement, 0 );

	sqlite3_finalize( statement );

	return iRast;
}
//-------------------------------------------------------------------------------------------------

/*!
	ＩＤ指定して削除
	@param[in]	delID	このＩＤのデータを削除する
	@return	UINT	特にない
*/
UINT SqlTreeNodeExtraDelete( UINT delID )
{
	CONST CHAR	cacDelete[] = { ("DELETE FROM TreeNode WHERE id == ?") };

	INT		rslt;
	sqlite3_stmt	*statement;

	if( !(gpDataBase) ){	TRACE( TEXT("NoDatabase") );	return 0;	}

	rslt = sqlite3_prepare( gpDataBase, cacDelete, -1, &statement, NULL);
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	sqlite3_reset( statement );
	rslt = sqlite3_bind_int( statement, 1 , delID );	//	type

	rslt = sqlite3_step( statement );
	if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	}

	sqlite3_finalize( statement );

	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	ＩＤ指定してリストアップ
	@param[in]	seekID	１以上の場合、このIDのを探す。こっち優先
	@param[in]	tgtID	この番号を超えて最初にヒットしたやつを返す・seekIDが０の場合
	@param[out]	ptName	ノードのパスいれるバッファ・MAX_PATHであること
	@return	UINT	引っ張ったやつのＩＤ・無かったら０
*/
UINT SqlTreeNodeExtraSelect( UINT seekID, UINT tgtID, LPTSTR ptName )
{
	//LPCTSTR	ptBuffer;
	CHAR	acQuery[MAX_STRING];
	INT		rslt;
	UINT	id = 0;
	sqlite3_stmt*	statement;

	if( !(gpDataBase) ){	TRACE( TEXT("NoDatabase") );	return 0;	}

	if( 1 <= seekID ){	StringCchPrintfA( acQuery, MAX_STRING, ("SELECT id, nodename FROM TreeNode WHERE id == %u"), seekID );	}
	else{	StringCchPrintfA( acQuery, MAX_STRING, ("SELECT id, nodename FROM TreeNode WHERE id > %u AND parentid == -1 ORDER BY id ASC"), tgtID );	}

	rslt = sqlite3_prepare( gpDataBase, acQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}
	rslt = sqlite3_step( statement );
	if( SQLITE_ROW == rslt )
	{
		id = sqlite3_column_int( statement, 0 );	//	id
		String_Cch_Copy( ptName, MAX_PATH, (LPCTSTR)sqlite3_column_text16( statement, 1 ) );	//	nodename
		//ptBuffer = (LPCTSTR)sqlite3_column_text16( statement, 1 );	//	nodename
		//if( ptBuffer  ){	StringCchCopy( ptName, MAX_PATH, ptBuffer );	}
	}

	sqlite3_finalize( statement );

	return id;
}
//-------------------------------------------------------------------------------------------------

/*!
	ファイル名指定して、同じのがあるかどうか
	@param[out]	ptName	ノードのパスいれるバッファ・MAX_PATHであること
	@return	UINT	ヒットしたやつのＩＤ・無かったら０
*/
UINT SqlTreeNodeExtraIsFileExist( LPCTSTR ptName ) 
{
	CONST CHAR	cacNameSearch[] = { ("SELECT id FROM TreeNode WHERE nodename == ?") };

	INT		rslt;
	UINT	id = 0;
	sqlite3_stmt*	statement;

	rslt = sqlite3_prepare( gpDataBase, cacNameSearch, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	sqlite3_reset( statement );
	rslt = sqlite3_bind_text16( statement, 1, ptName, -1, SQLITE_STATIC );	//	nodename

	rslt = sqlite3_step( statement );
	if( SQLITE_ROW == rslt )
	{
		id = sqlite3_column_int( statement, 0 );	//	id
	}

	sqlite3_finalize( statement );

	return id;
}
//-------------------------------------------------------------------------------------------------


#endif


#if 0
/*!
	順番に全部リストアップ
	@param[in]	dProfID		リストアップするプロフＩＤ
	@param[in]	pfDataing	データを入れる函数ポインタ
*/
HRESULT SqlTreeNodeEnum( UINT dProfID, BUFFERBACK pfDataing )
{
	CONST CHAR	acQuery[] = { ("SELECT * FROM TreeNode ORDER BY id ASC") };
	INT		rslt;
	UINT	i, cnt;
	UINT	id, type, prnt;
	TCHAR	atName[MAX_PATH];
	sqlite3_stmt*	statement;


	if( !(gpDataBase) ){	return E_OUTOFMEMORY;	}

	rslt = sqlite3_prepare( gpDataBase, acQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_FAIL;	}

	cnt = SqlTreeCount( 1, NULL );

	for( i = 0; cnt > i; i++ )
	{
		rslt = sqlite3_step( statement );
		if( SQLITE_ROW == rslt )
		{
			id   = sqlite3_column_int( statement, 0 );	//	id
			type = sqlite3_column_int( statement, 1 );	//	type
			prnt = sqlite3_column_int( statement, 2 );	//	parentid
			String_Cch_Copy( atName, MAX_PATH, (LPCTSTR)sqlite3_column_text16( statement, 3 ) );	//	nodename

			pfDataing( id, type, prnt, atName );
		}
		else{	break;	}
	}

	sqlite3_finalize( statement );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------
#endif

#if 0
/*!
	指定の名前のディレクトリはルートにあるので？
	@param[out]	ptDirName	ノードの名称
	@return	UINT			０無かった　１～ヒットしたＩＤ
*/
UINT SqlTreeNodeRootSearch( LPTSTR ptDirName )
{
	TCHAR	atQuery[BIG_STRING];
	INT		rslt;
	UINT	id = 0;
	sqlite3_stmt*	statement;


	if( !(gpDataBase) ){	return 0;	}

	StringCchPrintf( atQuery, BIG_STRING, TEXT("SELECT id FROM TreeNode WHERE parentid == 0 AND nodename == '%s'"), ptDirName );

	rslt = sqlite3_prepare16( gpDataBase, atQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	rslt = sqlite3_step( statement );
	if( SQLITE_ROW == rslt )
	{
		id = sqlite3_column_int( statement, 0 );	//	id
	}

	sqlite3_finalize( statement );

	return id;
}
//-------------------------------------------------------------------------------------------------
#endif

/*!
	ＩＤ指定してリストアップ
	@param[in]	tgtID	この番号を超えて最初にヒットしたやつを返す
	@param[out]	pType	ディレクトリ(FILE_ATTRIBUTE_DIRECTORY)かファイルか(FILE_ATTRIBUTE_NORMAL)
	@param[out]	pPrntID	親ツリーノードのSQL的ＩＤ番号
	@param[out]	ptName	ノードの名称
	@param[in]	bStyle	0x01通常　0x00ツリーキャッシュ　／　0x10ＩＤ一致　0x00ＩＤ超えた
	@return	UINT	引っ張ったやつのＩＤ・無かったら０
*/
UINT SqlTreeNodePickUpID( UINT tgtID, PUINT pType, PUINT pPrntID, LPTSTR ptName, UINT bStyle ) 
{
	CHAR	acQuery[MAX_STRING];
	INT		rslt;
	UINT	id = 0;
	sqlite3_stmt*	statement;

	sqlite3	*pDB;

	if( bStyle & 0x01 ){	pDB = gpDataBase;	}
	else{					pDB = gpTreeCache;	}

	if( !(pDB) ){	return 0;	}

	if( bStyle & 0x10 ){	StringCchPrintfA( acQuery, MAX_STRING, ("SELECT * FROM TreeNode WHERE id == %u"), tgtID );	}
	else{	StringCchPrintfA( acQuery, MAX_STRING, ("SELECT * FROM TreeNode WHERE id > %u ORDER BY id ASC"), tgtID );	}

	rslt = sqlite3_prepare( pDB, acQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( pDB );	return 0;	}
	sqlite3_reset( statement );
	rslt = sqlite3_step( statement );
	if( SQLITE_ROW == rslt )
	{
		id       = sqlite3_column_int( statement, 0 );	//	id
		*pType   = sqlite3_column_int( statement, 1 );	//	type
		*pPrntID = sqlite3_column_int( statement, 2 );	//	parentid
		String_Cch_Copy( ptName, MAX_PATH, (LPCTSTR)sqlite3_column_text16( statement, 3 ) );	//	nodename
	}

	sqlite3_finalize( statement );

	return id;
}
//-------------------------------------------------------------------------------------------------

/*!
	親ＩＤ指定してリストアップ
	@param[in]	dPrntID	親ＩＤ
	@param[in]	tgtID	この番号を超えて最初にヒットしたやつを返す
	@param[out]	pType	ディレクトリ(FILE_ATTRIBUTE_DIRECTORY)かファイルか(FILE_ATTRIBUTE_NORMAL)
	@param[out]	ptName	ノードの名称
	@return	UINT	引っ張ったやつのＩＤ・無かったら０
*/
UINT SqlChildNodePickUpID( UINT dPrntID, UINT tgtID, PUINT pType, LPTSTR ptName )
{
	CHAR	acQuery[MAX_PATH];
	INT		rslt;
	UINT	id = 0, dummy;
	sqlite3_stmt*	statement;

	StringCchPrintfA( acQuery, MAX_PATH, ("SELECT * FROM TreeNode WHERE parentid == %u AND id > %u ORDER BY id ASC"), dPrntID, tgtID );

	rslt = sqlite3_prepare( gpDataBase, acQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}
	sqlite3_reset( statement );
	rslt = sqlite3_step( statement );
	if( SQLITE_ROW == rslt )
	{
		id     = sqlite3_column_int( statement, 0 );	//	id
		*pType = sqlite3_column_int( statement, 1 );	//	type
		dummy  = sqlite3_column_int( statement, 2 );	//	parentid
		String_Cch_Copy( ptName, MAX_PATH, (LPCTSTR)sqlite3_column_text16( statement, 3 ) );	//	nodename
	}

	sqlite3_finalize( statement );

	return id;
}
//-------------------------------------------------------------------------------------------------

/*!
	ツリーデータを全て削除
	@param[in]	bStyle	非０データベース本体　０プロフ構築用キャッシュ
	@return	HRESULT	終了状態コード
*/
HRESULT SqlTreeNodeAllDelete( UINT bStyle )
{
	CONST CHAR	acTreeDelete[] = { ("DELETE FROM TreeNode") };
	INT		rslt;
	sqlite3_stmt	*statement;

	sqlite3	*pDB;

	if( bStyle )	pDB = gpDataBase;
	else			pDB = gpTreeCache;


	rslt = sqlite3_prepare( pDB, acTreeDelete, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( pDB );	return E_OUTOFMEMORY;	}

	rslt = sqlite3_step( statement );
	sqlite3_finalize( statement );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	パヤーンを受け取って、該当するＩＤを返す。
	@param[in]	ptName	パヤーン
	@param[in]	dStart	検索開始ＩＤ・このＩＤの次の値から検索開始
	@return	ヒットしたＩＤ・無かったら０
*/
UINT SqlTreeFileSearch( LPTSTR ptName, UINT dStart )
{
	TCHAR	atReqest[SUB_STRING];
	INT		rslt;
	UINT	dxID;
	sqlite3_stmt*	statement;

	if( !(gpDataBase) ){	return 0;	}

	rslt = sqlite3_prepare( gpDataBase, ("SELECT id FROM TreeNode WHERE nodename LIKE ? AND id > ? ORDER BY id ASC"), -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	sqlite3_reset( statement );

	StringCchPrintf( atReqest, SUB_STRING, TEXT("%%%s%%"), ptName );
	rslt = sqlite3_bind_text16( statement, 1, atReqest, -1, SQLITE_STATIC );	//	
	sqlite3_bind_int( statement, 2, dStart );

	rslt = sqlite3_step( statement );
	if( SQLITE_ROW == rslt ){	dxID = sqlite3_column_int( statement, 0 );	}
	else{	dxID = 0;	}

	sqlite3_finalize( statement );

	return dxID;
}
//-------------------------------------------------------------------------------------------------

/*!
	特定の親ＩＤを持つノード名称を探して、該当するＩＤを返す。
	@param[in]	ptName	パヤーン
	@param[in]	dPrntID	特定の親ＩＤ
	@return	ヒットしたＩＤ・無かったら０
*/
UINT SqlTreeFileGetOnParent( LPTSTR ptName, UINT dPrntID )
{
	INT		rslt;
	UINT	dxID;
	sqlite3_stmt*	statement;

	if( !(gpDataBase) ){	return 0;	}

	rslt = sqlite3_prepare( gpDataBase, ("SELECT id FROM TreeNode WHERE nodename == ? AND parentid == ?"), -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	sqlite3_reset( statement );

	rslt = sqlite3_bind_text16( statement, 1, ptName, -1, SQLITE_STATIC );	//	
	sqlite3_bind_int( statement, 2, dPrntID );

	rslt = sqlite3_step( statement );
	if( SQLITE_ROW == rslt ){	dxID = sqlite3_column_int( statement, 0 );	}
	else{	dxID = 0;	}

	sqlite3_finalize( statement );

	return dxID;
}
//-------------------------------------------------------------------------------------------------




/*!
	ツリーキャッシュ用オンメモリＤＢ
*/
HRESULT SqlTreeCacheOpenClose( UINT bMode )
{
	//	ツリー情報
	CONST CHAR	cacTreeNodeTable[] = { ("CREATE TABLE TreeNode ( id INTEGER PRIMARY KEY, type INTEGER NOT NULL, parentid INTEGER NOT NULL, nodename TEXT NOT NULL )") };
	INT		rslt;
	sqlite3_stmt	*statement;

	if( bMode )
	{
		rslt = sqlite3_open( (":memory:"), &gpTreeCache );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpTreeCache );	return E_FAIL;	}

		//ツリーテーブルを生成
		rslt = sqlite3_prepare( gpTreeCache, cacTreeNodeTable, -1, &statement, NULL );
		if( SQLITE_OK != rslt ){	SQL_DEBUG( gpTreeCache );	return E_ACCESSDENIED;	}
		rslt = sqlite3_step( statement );	//	実行
		if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpTreeCache );	return E_ACCESSDENIED;	}
		rslt = sqlite3_finalize(statement);

		sqlite3_exec( gpTreeCache, "BEGIN TRANSACTION", NULL, NULL, NULL );	//	トランザクション開始
	}
	else
	{
		sqlite3_exec( gpTreeCache, "COMMIT TRANSACTION", NULL, NULL, NULL );	//	トランザクション終了

		if( gpTreeCache ){	rslt = sqlite3_close( gpTreeCache );	}
		gpTreeCache = NULL;
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ツリーCacheから、IDを元に削除
	@return	HRESULT	終了状態コード
*/
HRESULT SqlTreeCacheDelID( INT tgtID )
{
	CONST CHAR	acTreeDel[] = { ("DELETE FROM TreeNode WHERE id == ? OR parentid == ?") };
	INT		rslt;
	sqlite3_stmt	*statement;

	//	パレントＩＤが該当するヤツも削除か


	rslt = sqlite3_prepare( gpTreeCache, acTreeDel, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpTreeCache );	return E_OUTOFMEMORY;	}

	sqlite3_reset( statement );
	sqlite3_bind_int( statement, 1, tgtID );
	sqlite3_bind_int( statement, 2, tgtID );
	sqlite3_step( statement );
	sqlite3_finalize( statement );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ディレクトリかファイルのデータ一時バッファにドピュッ
	@param[in]	dType	ディレクトリかファイルか
	@param[in]	dPrnt	親ツリーノードのSQL的ＩＤ番号
	@param[in]	ptName	ノードの名称
	@return	UINT	いま登録したID番号
*/
UINT SqlTreeCacheInsert( UINT dType, UINT dPrnt, LPTSTR ptName )
{
	CONST CHAR	acTreeNodeInsert[] = { ("INSERT INTO TreeNode ( type, parentid, nodename ) VALUES ( ?, ?, ? )") };
	CONST CHAR	acAddNumCheck[] = { ("SELECT LAST_INSERT_ROWID( ) FROM TreeNode") };

	INT		rslt;
	UINT	iRast = 0;
	sqlite3_stmt	*statement;


	if( !(gpTreeCache) ){	TRACE( TEXT("NoDatabase") );	return 0;	}

	rslt = sqlite3_prepare( gpTreeCache, acTreeNodeInsert, -1, &statement, NULL);
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpTreeCache );	return 0;	}

	sqlite3_reset( statement );
	rslt = sqlite3_bind_int(    statement, 1, dType );	//	type
	rslt = sqlite3_bind_int(    statement, 2, dPrnt );	//	parentid
	rslt = sqlite3_bind_text16( statement, 3, ptName, -1, SQLITE_STATIC );	//	nodename

	rslt = sqlite3_step( statement );
	if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpTreeCache );	}

	sqlite3_finalize( statement );

	//	今追加したやつのアレを取得
	rslt = sqlite3_prepare( gpTreeCache, acAddNumCheck, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpTreeCache );	return 0;	}

	rslt = sqlite3_step( statement );

	iRast = sqlite3_column_int( statement, 0 );

	sqlite3_finalize( statement );

	return iRast;
}
//-------------------------------------------------------------------------------------------------




/*!
	副タブ情報を追加
	@param[in]	ptFilePath	ファイルパス・空なら使用から開いた
	@param[in]	ptBaseName	ルート直下のディレクトリ名
	@param[in]	ptDispName	タブの表示名
	@return	UINT	いま登録したID番号・余り意味はない
*/
UINT SqlMultiTabInsert( LPCTSTR ptFilePath, LPCTSTR ptBaseName, LPCTSTR ptDispName )
{
	CONST CHAR	acMultitabInsert[] = { ("INSERT INTO MultiTab ( filepath, basename, dispname ) VALUES ( ?, ?, ? )") };
	CONST CHAR	acAddNumCheck[] = { ("SELECT LAST_INSERT_ROWID( ) FROM MultiTab") };

	INT		rslt;
	UINT	iRast = 0;
	sqlite3_stmt	*statement;

	if( !(gpDataBase) ){	return 0;	}

	rslt = sqlite3_prepare( gpDataBase, acMultitabInsert, -1, &statement, NULL);
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	sqlite3_reset( statement );
	rslt = sqlite3_bind_text16( statement, 1, ptFilePath, -1, SQLITE_STATIC );	//	filepath
	rslt = sqlite3_bind_text16( statement, 2, ptBaseName, -1, SQLITE_STATIC );	//	basename
	rslt = sqlite3_bind_text16( statement, 3, ptDispName, -1, SQLITE_STATIC );	//	dispname

	rslt = sqlite3_step( statement );
	if( SQLITE_DONE != rslt ){	SQL_DEBUG( gpDataBase );	}

	sqlite3_finalize( statement );

	//	今追加したやつのアレを取得
	rslt = sqlite3_prepare( gpDataBase, acAddNumCheck, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	rslt = sqlite3_step( statement );

	iRast = sqlite3_column_int( statement, 0 );

	sqlite3_finalize( statement );

	return iRast;
}
//-------------------------------------------------------------------------------------------------

/*!
	ＩＤを指定してデータゲット
	@param[in]	id			検索するＩＤ
	@param[out]	ptFilePath	ファイルパスバッファ・MAX_PATHであること
	@param[out]	ptBaseName	ルート直下のディレクトリ名バッファ・MAX_PATHであること
	@param[in]	ptDispName	タブの表示名・MAX_PATHであること
	@return	UINT	引っ張ったヤツのID番号・ヒットしなかったら０
*/
UINT SqlMultiTabSelect( INT id, LPTSTR ptFilePath, LPTSTR ptBaseName, LPTSTR ptDispName )
{
	CHAR	acQuery[MAX_STRING];
	INT		rslt;
	UINT	index = 0;
	sqlite3_stmt*	statement;


	if( !(gpDataBase) ){	return 0;	}

	StringCchPrintfA( acQuery, MAX_STRING, ("SELECT * FROM MultiTab WHERE id == %d"), id );

	rslt = sqlite3_prepare( gpDataBase, acQuery, -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return 0;	}

	rslt = sqlite3_step( statement );
	if( SQLITE_ROW == rslt )
	{
		index = sqlite3_column_int( statement , 0 );	//	id
		String_Cch_Copy( ptFilePath, MAX_PATH, (LPCTSTR)sqlite3_column_text16( statement, 1 ) );	//	filepath
		String_Cch_Copy( ptBaseName, MAX_PATH, (LPCTSTR)sqlite3_column_text16( statement, 2 ) );	//	basename
		String_Cch_Copy( ptDispName, MAX_PATH, (LPCTSTR)sqlite3_column_text16( statement, 3 ) );	//	dispname
	}

	sqlite3_finalize( statement );

	return index;
}
//-------------------------------------------------------------------------------------------------


/*!
	副タブデータを削除
	@return	HRESULT	終了状態コード
*/
HRESULT SqlMultiTabDelete( VOID )
{
	INT		rslt;
	sqlite3_stmt	*statement;

	if( !(gpDataBase) ){	return 0;	}

	rslt = sqlite3_prepare( gpDataBase, ("DELETE FROM MultiTab"), -1, &statement, NULL );
	if( SQLITE_OK != rslt ){	SQL_DEBUG( gpDataBase );	return E_OUTOFMEMORY;	}
	rslt = sqlite3_step( statement );
	sqlite3_finalize( statement );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

