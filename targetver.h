#pragma once

//	最低限必要なプラットフォームを定義。
//	最低限必要なプラットフォームとは、アプリケーションを実行するために必要な機能を備えた最も古いバージョンの Windows や Internet Explorer。
//	これらのマクロは、指定したバージョンと、それ以前のバージョンのプラットフォーム上で利用できるすべての機能を有効にすることによって動作。

//	下で指定された定義の前に対象プラットフォームを指定しなければならない場合、以下の定義を変更してください。
//	異なるプラットフォームに対応する値に関する最新情報については、MSDN を参照してください。


#define NTDDI_VERSION	0x05000000

#ifndef WINVER			//	最低限必要なプラットフォームが Windows 2000 であることを指定します。
#define WINVER	0x0500	//	これを Windows の他のバージョン向けに適切な値に変更してください。
#endif

#ifndef _WIN32_WINNT		//	最低限必要なプラットフォームが Windows 2000 であることを指定します。
#define _WIN32_WINNT	WINVER	//	これを Windows の他のバージョン向けに適切な値に変更してください。
#endif

#ifndef _WIN32_WINDOWS			//	最低限必要なプラットフォームが Windows 98 であることを指定します。
#define _WIN32_WINDOWS	_WIN32_WINNT	//	これを Windows Me またはそれ以降のバージョン向けに適切な値に変更してください。
#endif

#ifndef _WIN32_IE			//	最低限必要なプラットフォームが Internet Explorer 5.01 であることを指定します。
#define _WIN32_IE 0x0501	//	これを IE の他のバージョン向けに適切な値に変更してください。
#endif
