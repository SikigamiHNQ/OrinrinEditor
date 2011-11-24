OrinrinEditor / OrinrinViewer / OrinrinCollector
======================
ＡＡストーリーエディタ  
MLT(AST)ファイルビューワ  
コピペ保存ツール

## ライセンス ##
General Public License Ver3

## 開発環境 ##

+   **言語** ： C言語（一部C++）
+   **環境** ： Visual C++ Express 2008。2005でも2010でも多分OK

##ライブラリ等##
###WindowsDriverKit710###
[ダウソ](http://www.microsoft.com/download/en/details.aspx?displaylang=en&id=11800 "WindowsDriverKit710")

    インクルードファイルパスに、"(DDKフォルダ)\inc\atl71"
    ライブラリファイルパスに、"(DDKフォルダ)\lib\atl\i386"を追加。

    名前はATL71だけどバージョンは80

    リンクでエラーが出るなら、
    リンカの追加の依存ファイルに、atlthunk.libを付ける。

    プレビューで使ってるIEコンポーネントにATLが要るので、そのために必要。
    VisualStudioの上位エディションにはATLが入ってるので、そういう場合は不要。

###SQLite3###
[ダウソ](http://www.sqlite.org/ "SQLite3" )  

    Win32-DLLと、sourceに含まれているheaderが必要。  
    sqlite3.h、sqlite3.lib をプロジェクトに加える必要がある。  
    libの用意の仕方はグーグル先生が知っている。

###ImgCtl.dll###
[ダウソ](http://www.ruche-home.net/ "ImgCtl.dll")

    imgctl.h、imgctl.dll が必要。

##コンパイル方法##
VC++にWDKを登録して、SQLite3とImgCtlをプロジェクトに参加させたら、  
ビルドすればおｋ。  
EditorとViewerは、ソースの大半を共有してるが、同時にコンパイルはできない。  
どちらかのプロジェクトをアンロードしておくか、一つずつリビルドすること。  
リンクエラーが出たら、クリーンして再度やってみる。  
Collectorは共同で大丈夫なはず。  

##連絡先##
+  [したらばのスレ](http://jbbs.livedoor.jp/bbs/read.cgi/otaku/12368/1318151605/ "したらばのスレ")  
ここがメイン。バイナリ配布はこちら。次スレに進んでる場合アリ  
+  IRC  
irc.2ch.net　#yaruo  
夜に居ることが多いかもしれない。


 
