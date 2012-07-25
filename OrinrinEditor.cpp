/*! @file
	@brief アプリケーションのエントリ ポイントを定義します。
	このファイルは OrinrinEditor.cpp です。
	@author	SikigamiHNQ
	@date	2011/05/10
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
//	大日本帝国公用語は↓を見られたい
//-------------------------------------------------------------------------------------------------

//	注意・コマンドのリソースＩＤ番号は変更不可！

//	TODO:	viewerのMLTリストアップ、チェックボックスは灰色できないか
//			ディレクトリ内の選択非選択がややこしい
//			ルートディレクトリの更新日時をみて、修正か新規か判断？

//	TODO:	右ルーラーみたいな、行ルーラー（３０行とか）を

//	TODO:	バイト数警告の4096は調整出来るように

//	TODO:	プレビュー再描画しても、ALLと表示位置を維持する

//	TODO:	MLT検索、文字列ないなら検索しないようにする

//	TODO:	ＯＫ？	ディレイロード・未ロード頁の全プレビューがおかしい
//	TODO:	CtrlShiftD の動作は問題無いか
//	TODO:	ＯＫ？	枠作るときはピリヲド幅を使う
//	TODO:	ＯＫ？	最大化から戻したら、MAAが非表示でも表示になる
//	TODO:	ＯＫ？	MAAと頁一覧、選択したらメイン窓にフォーカス戻すかどうか選択出来るようにする
//					右クリメニューにON/OFF付ければいい
//	TODO:	ＯＫ？	ドラッグ移動させた後、キャレットの位置がおかしい
//	TODO:	ＯＫ？	ドラフトボード・サムネイル・移動出来るように
//	TODO:	ＯＫ？	ユーザー定義をアクセラキーで設定出来る
//	TODO:	ＯＫ？	セリフ改行、文章の途中で半角全角スペース一個あった場合、ソコは無視して文章の先端に行頭を揃える
//	TODO:	ＯＫ？	ツールチップ、マウスカーソルが重なるとちらつくのは仕様。自作しかないか。
//	TODO:	ＯＫ？	右クリメニューの個別枠やユーザアイテムに各個の名前を付けるように
//	TODO:	ＯＫ？	副タブの名称編集出来るように
//	TODO:	ＯＫ？	空白の表示非表示をメモリする
//	TODO:	ＯＫ？	ドラフトボード、ウインドウサイズがおかしい・枠付けたので
//	TODO:	ＯＫ？	合成するとき、上絵の周囲を白ヌキする機能
//	TODO:	ＯＫ？	レイヤボックス白ヌキ、１６・２２とか、いくつかパヤーン作っておく
//	TODO:	ＯＫ？	中CLICKで、開いてるファイル閉じる

//	TODO:	最終行の枠いれると落ちる？

//検索について・文字入力とか削除あったら、ハイライト再計算を？
//Ｆ５の描画リロードでハイライト付け直しするように


//	TODO:	メインのファイルタブに、このファイル意外を閉じる機能をつける
//	TODO:	頁一覧、選択が移動したら表示頁も追随する＜複数選択つくってから
//	TODO:	全ピリヲドを消去する機能・ピリヲドの前後を確認して、空白であれば幅に合わせて置き換え・単独なら3dotユニコードで置き換える

//	TODO:	MAAの一コマの編集機能・ファイル名工夫するかフラグで管理
//			保存するには、そのMAAの開きを維持しておく必要がある

//	TODO:	ページリストも、ばらしたとき閉じられるように

//	TODO:	挿入にある色指定をカスタマイズ出来る

//	TODO:	viewer側の表示色変更・背景とか


//考え中

//	レイヤー機能とは？

//	頁削除メッセージの確認無しは、起動中だけにしたほうがいい？
//	キーバインド・メニューに入ってない機能に注意
//	枠編集DIALOGUE、オフセットでENTERとか直書換に対応セヨ
//	ドッキングテンプレ、左に置けないか
//	セーブ時メッセージON/OFFできるように
//	ファイル履歴は、いじった順番になるようにすべき＜ＯＫ？
//	プレビューの更新ボタンつける
//	プレビュー開いてる時に、開く操作したら閉じる？

//	デカいファイル開こうとしたときはメッセージ出すとか
//	SQLのINSERT、prepareは重い。クエリつくって、resetしながら回すのがいいんじゃ
//	IEプレビュー、CSSでフォント表示を正規化する http://b4t.jp/823



//ファイル開くとき、ページデータは生のテキストデータだけゲットしておいて、メモリ展開はしない
//ページが選択されたら、本体に展開して、生データは消しておく・これをフラグにする
//保存するときは、生データか、本体データを保存する
//頁表示のステータスのほうは？バイト数計算だけならそんなに重くない？

//頁番号挿入みたいに、複数頁にわたる処理がヤバイ
//全プレビューもヤバイ

//	ファイルオーポン時にプログレス的なものを。バイト数表示のやつ。ディレイロードできれば要らないか


//確認：Viewer側、↑↓PageUpDownつかえてるか




//バグ
//	TODO:	最大化したり画面サイズかえると、右の頁一覧が見えなくなる
//	TODO:	最大化して終わる＞起動して、最大化解除すると、メインスプリットバーが見えなくなる//右にイッちゃってるまま
//	TODO:	矩形選択範囲バイトカウントが遅い
//	TODO:	描画更新タイミングがおかしい場合が多々ある
//	TODO:	「矩形選択」と「部分抽出」の両方にチェックを入れた状態でドラッグするとエラー

//	TODO:	複数行テンプレ窓の主タブでホイールスクロールが効かない？
//			メイン窓の編集スペース、Page List、複数行テンプレ窓のAA一覧ツリーなど、他の場所では問題ない

//	TODO:	Viewer、右クリメニューからのドラフトボードが開かない？・追加されない？




//機能追加・変更
//	TODO:	全頁の一括行末削除
//	TODO:	複数機能をまとめて実行するマクロのようなもの＜プラグインでいけるか？

//	TODO:	頁の複数選択して移動、Ｄ＆Ｄも
//	TODO:	ALT+Nを、頁名称一括編集にする？
//	TODO:	頁一覧を、複数選択して一発削除できるように

//	TODO:	履歴から開こうとしたファイル・プロフが無かったらリストから消すべき

//	TODO:	フリーカーソルモード
//	TODO:	レイヤボックス自体の操作、上書き実行とかもキボドショトカトで出来るように
//	TODO:	位置情報のリセット機能・システムメニューに搭載する
//	TODO:	メインスプリットバーの位置情報リセットが居るかも

//	TODO:	右揃え線消し
//	TODO:	文字の検索・痴漢機能	ユニコード空白一括検索とか
//	TODO:	ファイルを開き直す機能
//	TODO:	起動中に、ラインテンプレートのリストを開く＆再読み込みできるようにして欲しい//	Brushも同じか。右クリメニューで

//	TODO:	複数ファイルタブ、右クリメニューでファイル情報出す
//	TODO:	Ctrl+Kの統計機能・PageListの項目増やして対処・Ctrl+Kでリフレッシュとか・連続半角とか、行末空白とか・いろんな情報をだす・リヤルタイムだと重いので操作でリフレッシュ
//	TODO:	ファイル内容統計機能が欲しい。全バイト数、頁数とか


//	TODO:	使用をファイル出力できるように
//	TODO:	DocBadSpaceCheck内の更新指令の最適化が必要
//	TODO:	バックアップ機能の強化・上書き保存したらバックアップとか・頁新規作成したら保存
//	TODO:	保存するとき、同名ファイルがあったら、日時くっつけてバックアップとか
//	TODO:	切り抜きリファレンスAAを使って、文字のとこだけ切り抜き、ないところだけ切り抜き、とかできるようにしたい
//	TODO:	バックアップメッセージ、ステータスバーに出せないか
//	TODO:	全部保存作る
//	TODO:	よく使うユニコードを纏めておけないか、ユーザ定義でいいんじゃね
//	TODO:	テンプレのコンボックスを、リストボックス型と切り替えられるとか
//	TODO:	プレビューをリヤルタイムに変更する
//	TODO:	非ユニコのときは、ピリヲドの量の選択を？もしくは設定出来るようにするか
//	TODO:	ミニ複数行テンプレ・壱行ブラシのサブダイヤログに増やすか

//	TODO:	View部分をサムネ表示できるように
//	TODO:	テンプレの「常に最前面に表示」Orinrinediterを表示している時だけ最前面に来るように//もしくは、メイン窓に統合とか
//	TODO:	Viewer側、Ctrl↑↓で、MLTのセクション間移動する
//	TODO:	viewerの主タブをマウスとスクロールバーだけでなく、ホウィール、キーボードの矢印やPageUp,PageDnキーで操作出来る様にならない？
//	TODO:	Viewer側、ポップアップで、容量も出せないか
//	TODO:	MLTのブックマーク機能・タブ増やすか、ツリーに増やすか・副タブじゃいけない？
//	TODO:	viewの使用のグループ名の変更機能・グループ自体、単体両方で
//	TODO:	Viewの右クリメニューも編集出来るように
//	TODO:	ドラフトボード・サムネ、サイズ変更できるように
//	TODO:	MMAビュー、AAのスクロールをアイテム区切りでなく行スクロールも出来るように
//	TODO:	MAA窓も統合できないか

//	TODO:	ASD対応＜読込は作ったがどこで使う？
//	TODO:	ツールチップ付けたい

//	TODO:	トレスの１画面、２画面切り替えモード
//	TODO:	トレスの２画面だと、背景絵はどっちのペインでも表示出来るようにしたい
//	TODO:	トレスで、画像をつまんで直接移動できるように
//	TODO:	トレス機能、点滅コントラスト？と輝度調整の幅が狭いのがちょいと苦しいらしい
//	TODO:	トレスモード窓に、グリッドON/OFFを付ける
//	TODO:	トレスモード、各入力値は直入力できるように
//	TODO:	トレスモードボタンは、TOGGLEできるようにする





//	TODO:	プロファイルのツリーの編集、もっと手軽にできないか










//	ツールチップのポップディレイは、TTM_SETDELAYTIME で調整出来る
//	wParam	TTDT_INITIAL	表示までの時間
//	lParam	The LOWORD specifies the delay time, in milliseconds. The HIWORD must be zero.
//変わってるように思えない。なんか間違えてるか


//	ファイルを読み込むとランタイムエラーでることがある・なんか変更してから？
//	スクロールが発生した場合、画面全体書換必要か

//影響ないようにした
//選択範囲がある状態でCtrlU＞アンドゥ＞選択範囲がおかしくなる。さらに切り取りとかすると落ちる
//アンドゥリドゥしたら、選択範囲解除する・DocInsDelCtrl.cpp:559あたりでおかしい？

//非ユニコードの埋めパターン、半角空白気にしない用にする？＜ちゅっと変更してみた


//OK?
//	新規で開いて、それを名づけ保存したファイルは、履歴に入いってない
//	頁削除のAlt+D付ける。注意ダイヤログ出す。
//	サムネからだと使用履歴に反映されない
//	文字ＡＡの入力とか、レイヤボックスの編集とかで、キーボードショートカット使えるように
//	トレス窓が、最大化してると隠れてしまう
//	頁挿入ダイヤログのチェックボックス覚えておくように

//	TODO:	使用のアレ、別MLTに同じAAが有ったらハッシュが同じってことでラリってまう。ベースネーム書換で対処するか
//	TODO:	枠挿入BOX、縦幅がパーツ全体の高さに足りなくなると落ちる・横幅もか？


/*

ASDファイル　　壱行が壱コンテンツ

内容の改行　　　　　0x01,0x01
内容と説明の区切り　0x02,0x02

*/
//	v ^ (v >> 1)	グレイコード

/*
2011/06/30	0.10	初リリース、プレアルファ版
2011/07/04	0.11	選択中に右クリすると解除されちゃうのを修正
					全選択してコピーしたときの末端改行増殖を修正
					レイヤボックスで、編集ビューのスクロールが考慮されてなかったのを修正
					MAAで複数ファイルをタブで開けられるようにした
2011/07/19	0.12	枠の保持を１０個にした
					終了時の副タブを復帰出来るようにした
					新規保存やリネーム保存だと、ラストオーポンが記録されないのを修正
					PageListで、削除・移動できるようにした
2011/07/20	0.13	MLTツリーで、ASTも読めるようにした
					ブラシ、壱行テンプレのコンボックスはマウスホイールしないようにした
					選択状態解除されてるけどフラグが解除されてないのを修正したつもり
					矩形選択範囲を、カーソル位置に準じるようになった？
					切り取りのキーボードアクセラレータが無かった・単なる入れ忘れ
					０文字の行でAlt＋←→やるとあぼーんしてたのを修正
					MAAテンプレで、ツールチップのON/OFF出来るようにした
					枠に名前付けられるようにした
					トレス機能つけた・多分改善が必要
					選択範囲を反転表示しちゃう
					テンプレファイルを、Templatesディレクトリに移動
2011/08/01	0.14	コンテキストメニューにもアクセラレータキーを付けた
					トレス：表示非表示を Alt+I で出来るようにした☆
					トレス：非表示中は文字色戻すようにした
					トレス：特定の操作したらフォーカスが描画ビューに戻るようにした
					ブラシと壱行テンプレの表示を格子状にした・でも選択の色変えがヘン
					MAAのツールチップ文字サイズを９と１２で選択出来るようにした
					レイヤボックス、貼り付けたらすぐ閉じるかどうかの選択チェックつけた
					ユニコードパディングを使えるようにした
2011/08/09	0.15	トレス中はタイトルバーに[トレスモード]がつく
					トレスの表示非表示をAlt+Hに変更
					トレスで、絵を開いたらズームを１００％に戻すようにした
					トレスの画像開く窓にサムネイル付けた
					頁一覧にツールバー付けた
					頁の複製機能を実装
					Alt左右の場合にもユニコードつかってパディングするようにした
					Ctrl+Home,Endを効くようにした
					MAA窓にフォーカスがあるとき、TAB,Shift+TABでタブを切り替わるようにした
					MAAの副タブを削除・追加したら選択がおかしくなってるのを修正
					ユニコード数値参照、１６進数型に対応
					レイヤボックス、ブッ込む内容に合わせて、窓の大きさを拡大するようにした
					自動バックアップ機能を付けた
2011/08/18	0.16	キャプションバーの開いてるファイル名が変更されない時があるのを修正
					実体参照の読込をするようにした。ただし内部でユニコードにしてる
					ファイル開いたときに、ページリストにバイト数とかが反映されないのを修正
					レイヤ合成、ずれることがあったのを修正
					レイヤボックスの透過合成を出来るようにした
					クリップボード内テキストのユニコード判別出来るようにした
					ツールチップが、Win7だとチラつくのをしないようにできた？
					ページリストに、各頁のサムネイルっぽいのをポッパップするようにした
					MAAウインドウのAA表示を、ユニコードに変換して表示するようにした・問題ないと思うけど
					レイヤボックスのサイズ変更がおかしいのを直した
					AAViewer部分を分離して単独Projectもいけるようにした
2011/08/24	0.17	バックアップディレクトリを常に作成するようにした
					バックアップしたメッセージのON/OFF出来るようにした・ミスは常に表示
					頁を新規作成したときもバックアップするようにした
					バイト数計算の作り直し・多分正確に出せるようになった
					ALT上下で頁切換できる
					ドラッグ選択をやりやすくした
					頁の統合と分割を出来るようにした
					副タブの、最後に見ていた位置を覚えておけるようにした・終了したら消える
2011/08/31	0.18	複数ファイルオーポン出来るようになったはず・大丈夫かこれ
					Ctrl+F4 でタブ閉じできる
					ALT＋↑↓で頁移動したら、PageListもスクロールするようにした
					ツールバーに「やり直し」を追加
					空白の表示・非表示を切り替えられるようにした
					選択範囲を、空白かブラシで塗りつぶせるようにした
					各独立窓を、最前面表示出来るようにした
					部分抽出できるようになったかも
					カレントディレクトリを、EXEファイルのある場所で取るようにした
					バグ修正いろいろ
2011/09/06	0.19	コマンドラインからのファイルオーポンに対応した？最初の一つだけ
					空白警告があったら、行番号が紅くなるようにした
					最終頁削除したら吹っ飛ぶのを修正
					グリッド線を表示出来るようにした
					右端ルーラを表示できるようにした
					背景色等変更できるようにした。でもGUIがない
					この頁の前に挿入、を、後に挿入にした・末尾作成のキーバインド変更
					右クリメニュー色々追加
					マウスホイールのスクロール量をＯＳ標準を使うようにした・多分いける
					PageUp,PageDnで１０行ずつスクロールするようにした
					行が増えると重くなるのをないようにした・多分イケてる
					行数桁数を６桁まで対応
					複数ファイル間の移動を、Ctrl+PageUp,Downで移動できるようにした
					IEコンポーネントを使った簡易プレビュー搭載
					Ctrl+Home,Endで、スクロールが上手くいかないのを修正
					頁順番移動の上下が逆だったのを修正
					頁リスト、ヘッダの幅変更してもリストのほうが変わらないのを修正・大丈夫かな？
					バグ修正いろいろ
2011/09/16	0.20	レイヤボックスの内容をクリップボードできるようにした
					複数行テンプレから編集ビューに開けられるようにした
					プレビュー機能を、<DD>とか使って実際の画面っぽくなるようにした
					大量のALLプレビューしても堕ちなくなったかもだ
					投下支援機能として、頁送りSJISコピー機能を追加。ホットキー[Ctrl+Shift+D]を使用。
					改行マーク[CrLfMark]、非SJIS文字[CantSjis]の色変更出来るようにした
					ツールバーをリバーにした。でも変更の復元ができてねぇ
					右クリメニューの枠の名称が表示されるようにした
					抽出中にAlt+Spaceを押したら、抽出レイヤボックス開くでござる
					これに伴い、抽出してレイヤボックスへ機能は取りやめ
					抽出中にUNICODEコピーを出来るようになったかも
					矩形・塗潰・抽出・空白の状態をステータスバーに表示
					末端空白削除しても、行数表示が赤いままだったのを修正
					今見てる頁を、BMPとPNGで保存出来るようにした
2011/09/30	0.21	スペースを非表示にしても、警告は表示するようにした
					トレスモードの絵の上に、グリッド線出すようにした
					ツールバー、メニューの、トレスモード・ユニコード表はTOGGLE動作にした
					ユニコード表でダブルクルックしたら挿入されるようにした
					頁分割・統合・削除の確認メッセージに、次から表示しないオプション
					カーソル位置を覚えるようにした・太ましくなった
					全体のドットシフト機能・選択範囲のみ処理もできる
					複数行テンプレの区切りを、線と色違いを切り替えられるようにした
					複数行テンプレで、プロファイル機能で切り替えられるようにした
					複数行テンプレプロファイルに、ツリー内容をキャッシュした
					複数行テンプレの使用リストの分類を詳しくした
					複数行テンプレで、使用リストをタブで開けるようにした
					Viewerをバージョンアップ
2011/10/20	0.22	既存の拡張子が、ASTならそれを維持するようにした
					トレスモード、スライダをスクロールバーにしてみた
					編集エリアのスクロールに合わせて絵も動くようにした
					ちらつきをある程度押さえられたかもだ
					ブラシ・壱行テンプレで、各アイテムの横幅をツールチップに表示・でもなんかヘン
					選択して入力＞削除、矩形選択＞塗り潰し、するようにできたかもだ
					枠挿入窓を作成
					レイヤボックス、枠挿入窓を、カーソルキーで異動出来るようにした
					内容変更したら、ファイルタブが[変更]
					頁全体を空白で埋める機能
					頁番号挿入機能
					壱行テンプレ、ブラシテンプレ、頁リストをメイン窓にくっつける機能
					複数行テンプレのタブを、複数段表示出来るようにした
					バグ修正いくつか
2011/11/07	0.23	選択範囲のバイト数表示するようにした
					枠挿入窓に、挿入したら閉じるスイッチを搭載
					複数行テンプレからの割込挿入を、ちゃんと矩形挿入なるようにした
					レイヤボックス開くとき、カーソル位置に開くようにした▲
					複数行テンプレに見出し機能を付けた。ASTと行頭の【】に反応
					複数行テンプレのツリーのファイルへの、内容追加できるようにした☆
					ブラシするときに、ドットがはみ出しても気にしないようにした
					コピーモードを、UnicodeとShiftJISを入れ替えられるようにした
					文字ＡＡ変換機能出来たかも
					バグ修正いくつか
2011/11/11	0.24	コンテキストメニュー編集機能を搭載できたか？
					Ctrl＋↑↓で、テンプレグループの頁操作
					複数行テンプレの見出し機能を強化。マシになったか？
					バグ修正いろいろ
2011/11/14	0.24.1	文字ＡＡ変換窓が、画面外に行かないようにした
					多重起動防止いれた
					プレビューウインドウの位置と大きさを覚えておくようにした
					枠挿入したときに下の方が更新されないのを修正
					バグ修正いろいろ
2011/11/28	0.25	MAAのファイル名検索機能
					MAAのツリー展開が早くなった気がする（Viewer込み）
					プロファイル構築で、既存のプロフと一致するならチェキするようにした（Viewer込み）
					プロファイル作るときの時間が短縮できた気がする（Viewer込み）
					メイン窓のテンプレエリアのサイズ可変になった
					最大化状態を覚えておくようにした
					4096バイト超えたら、頁リストのバイト数のところ赤くするようにした
2011/12/22	0.26	ドラフトボード機能（Viewer込み）
					MAAサムネイル機能（Viewer込み）
					MAAの使用のグループ一括削除（Viewer込み）
					配色変更をＧＵＩで出来るようにした
					オーポン記録がなければ、説明ASTを表示するようにした
					ファイル毎のCaret位置を保存するようにした
					起動したまま、壱行テンプレ、ブラシテンプレのリロードできるようんした
					起動するとき、最後に開けてたファイルを開くとか選択できるようにした
2012/01/16	0.27	編集窓のファイルタブで、マウスオーバーでフルパス表示
					ＭＡＡとドラフトボードで、中クルックの場合処理を増やした（Viewer込み）
					台詞用Enter機能。Shift+Enterで、改行挿入ではなく次の行に空白とかで台詞位置合わせできる
					縦書き機能
					バグ修正いろいろ
2012/02/29	0.28	ＵＴＦ８でエクスポート出来るようにした
					フォント変更機能
					頁削除のAlt+D追加
					サムネから使ったら、使用タブに追加されないのを修正
					リバーの位置記憶と初期状態に戻す実装
					ファイル開いた履歴を保持できるようにした
					MAA部分に↑↓PageUpDownとか追加できたかもしれない
					MAAのリストアップ対象にTXTを追加
					改行が多いとパフォーマンスが落ちるのが少し改善できた気がする
					終了時の確認メッセージ変更
					IEプレビューのCSSを少し変更・行間について
					バグ修正いろいろ
2012/03/24	0.29	台詞改行、空白行で実行した場合は、キャレット位置で次の行を確保するようにした
					Editorで効いてなかったプロファイル履歴を修正
					右揃え線、ユニコードに対応
					行頭半角空白をユニコード空白に置き換える機能搭載
					Ctrl+Bでプレビュー開くようにした
					壱行、ブラシテンプレの表示/非表示ができる
					アクセラキー編集機能を搭載
					アンドゥリドゥをするときは選択範囲解除するようにした
					已に開いているファイルを開こうとしたら、そのタブに移るようにした
2012/05/15	0.30	MAA窓でENTER押したら、そのときトップに見えてるAAを左クリック動作するようにした
					文字列若しくは空白列でダブルクルックすると、その範囲を選択状態にする
					選択範囲をドラッグ移動出来るようにした・ちゅっと挙動怪しいかも？
					プレビュー開いてたら、保存したときに再描画するようにした
					枠作成に、複数行使えるようにした
					枠の個数を２０個保持できるようにした
					ツールバーに、ユニコード空白の使用／不使用トグルボタンつけた
					左右反転、上下反転機能を追加
					レイヤボックスに、内容削除ボタンを追加
					4096byte超えたら、ステータスバーのバイト表示位置が赤くなる。
					頁挿入ダイヤログのチェックボックス覚えておくようにした
					キャレットの位置に矩形貼付機能を追加
					壱行テンプレで中クルックしたらレイヤボックスが開く
					MAAツリーでファイル名を中クリックしたら、副タブに追加できる
					バグ修正いろいろ
2012/06/26	0.31	MAAに、ファイルの途中にアイテムを追加できるようにした
					MAAの内容表示側にＤ＆Ｄすると、そのファイルを副タブで開く（Viewer込み）
					※使用には入らないし復元もされない
					MAAツリーにＤ＆Ｄすると、追加アイテムとしてツリーに追加（Viewer込み）
					※再構築すると消える
					MAAの使用で、複数のMLTに同じAAが有る場合にキョドるのを修正。多分（Viewer込み）
					枠挿入BOXを縮めすぎるとエラーになるのを修正
					台詞改行で、単独スペースはノーカンにした
					ユーザアイテムを右クリメニューに追加できるようにした
					ドラッグ移動させたらキャレット位置がおかしくなるのを修正
					MAA窓と頁一覧で、選択したらフォーカスを編集窓に戻す機能
					ドラフトボード・サムネイルの窓を移動出来るようにした
					ツールチップがチラつくのをなんとか出来た気がする
					副タブの名称変更を出来るようにした（Viewer込み）
					ファイルオーポンを高速化出来たかもだ。
					空白の表示/非表示状態を覚えておくようにした。
					レイヤボックス貼付に白ヌキ機能追加
					中クリックで、MAAの副タブや編集ファイルを閉じれるようにした（Viewer込み）



更新日時注意

*/

/*
IME名称ゲット

	HKL	hKl;
	TCHAR	atImeName[MAX_STRING];
	hKl = GetKeyboardLayout( 0 );
	ImmGetDescription( hKl, atImeName, MAX_STRING );
*/

//-------------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "OrinrinEditor.h"
//-------------------------------------------------------------------------------------------------

static CONST TCHAR	gcatLicense[] = {
TEXT("このプログラムはフリーソフトウェアです。あなたはこれを、フリーソフトウェア財団によって発行されたGNU一般公衆利用許諾書(バージョン3か、それ以降のバージョンのうちどれか)が定める条件の下で再頒布または改変することができます。\r\n\r\n")
TEXT("このプログラムは有用であることを願って頒布されますが、*全くの無保証*です。商業可能性の保証や特定目的への適合性は、言外に示されたものも含め、全く存在しません。\r\n\r\n")
TEXT("詳しくはGNU一般公衆利用許諾書をご覧ください。\r\n\r\n")
TEXT("あなたはこのプログラムと共に、GNU一般公衆利用許諾書のコピーを一部受け取っているはずです。\r\n\r\n")
TEXT("もし受け取っていなければ、<http://www.gnu.org/licenses/> をご覧ください。\r\n\r\n")
};
//-------------------------------------------------------------------------------------------------

//	表示用フォントベーステーブル・これをコピーして使う
static LOGFONT	gstBaseFont = {
	FONTSZ_NORMAL,			//	フォントの高さ
	0,						//	平均幅
	0,						//	文字送りの方向とX軸との角度
	0,						//	ベースラインとX軸との角度
	FW_NORMAL,				//	文字の太さ(0~1000まで・400=nomal)
	FALSE,					//	イタリック体
	FALSE,					//	アンダーライン
	FALSE,					//	打ち消し線
	DEFAULT_CHARSET,		//	文字セット
	OUT_OUTLINE_PRECIS,		//	出力精度
	CLIP_DEFAULT_PRECIS,	//	クリッピング精度
	PROOF_QUALITY,			//	出力品質
	VARIABLE_PITCH,			//	固定幅か可変幅
	TEXT("ＭＳ Ｐゴシック")	//	フォント名
};
//-------------------------------------------------------------------------------------------------

// グローバル変数:
static  HINSTANCE	ghInst;						//!<	現在のインターフェイス
static  TCHAR		gszTitle[MAX_STRING];		//!<	タイトルバーのテキスト
static  TCHAR		gszWindowClass[MAX_STRING];	//!<	メインウィンドウクラス名

static  HMENU		ghMenu;			//!<	メニュー

static	HACCEL		ghAccelTable;	//!<	キーボードアクセラレータハンドル

static  HWND		ghFileTabWnd;	//!<	複数ファイルタブ
static  HWND		ghFileTabTip;	//!<	複数ファイルタブツールチップ
static WNDPROC		gpfOriginFileTabProc;	//!<	複数ファイルタブの元プロシージャ

static  HWND		ghMainWnd;		//!<	メインウインドウハンドル
static  HWND		ghStsBarWnd;	//!<	ステータスバー
static  HBRUSH		ghStsRedBrush;	//!<	ステータスバー用紅ブラシ

static HANDLE		ghMutex;		//!<	多重起動防止用Mutex

#ifdef WORK_LOG_OUT
static HANDLE		ghLogFlie;		//!<	ログ出力
#endif

EXTERNED HFONT		ghNameFont;		//!<	ファイルタブ用フォント

EXTERNED HWND		ghMaaWnd;		//!<	複数行ＡＡテンプレ
EXTERNED HWND		ghPgVwWnd;		//!<	ページリスト
EXTERNED HWND		ghLnTmplWnd;	//!<	壱行テンプレ
EXTERNED HWND		ghBrTmplWnd;	//!<	ブラシテンプレ
EXTERNED BOOLEAN	gbDockTmplView;	//!<	くっついてるテンプレは見えているか

EXTERNED HWND		ghMainSplitWnd;	//!<	メインのスプリットバーハンドル
EXTERNED LONG		grdSplitPos;	//!<	スプリットバーの、左側の、画面右からのオフセット

EXTERNED UINT		gbUniPad;		//!<	パディングにユニコードをつかって、ドットを見せないようにする
EXTERNED UINT		gbUniRadixHex;	//!<	ユニコード数値参照が１６進数であるか

static   UINT		gdBUInterval;	//!<	バックアップ感覚・デフォ３分くらい？
EXTERNED UINT		gbAutoBUmsg;	//!<	自動バックアップメッセージ出すか？
EXTERNED UINT		gbCrLfCode;		//!<	改行コード：０したらば・非０ＹＹ 

static TCHAR		gatExePath[MAX_PATH];	//!<	実行ファイルの位置
static TCHAR		gatIniPath[MAX_PATH];	//!<	ＩＮＩファイルの位置

EXTERNED INT		gbTmpltDock;	//!<	テンプレのドッキング

static list<OPENHIST>	gltOpenHist;	//!<	ファイル開いた履歴・
EXTERNED HMENU	ghHistyMenu;			//!<	履歴表示する部分・動的に内容作成せないかん

#ifdef FIND_STRINGS
extern  HWND	ghFindDlg;		//	検索ダイヤログのハンドル
#endif
extern  HWND	ghMoziWnd;		//	文字ＡＡ変換ダイヤログのハンドル

extern  HWND	ghMaaFindDlg;	//	MAA検索ダイヤログハンドル

extern  UINT	gdClickDrt;		//	ドラフトボードクルックスタイル
extern  UINT	gdSubClickDrt;	//

extern  HWND	ghViewWnd;		//	ビュー

extern  UINT	gdGridXpos;		//	グリッド線のＸ間隔
extern  UINT	gdGridYpos;		//	グリッド線のＹ間隔
extern  UINT	gdRightRuler;	//	右線の位置
//-------------------------------------------------------------------------------------------------

#ifdef PLUGIN_ENABLE

//[_16in] Add - 2012/05/01
//-- プラグイン関係
plugin::PLUGIN_FILE_LIST	gPluginList;
#include "Plugin/PluginCtrlAsync.h"

#endif


//	ステータスバーの区切り
#define SB_ITEMS	8
CONST INT	gadStsBarSize[] = { 50, 200, 350, 500, 700, 800, 900, -1 };
//-------------------------------------------------------------------------------------------------

VOID	Cls_OnActivate( HWND, UINT, HWND, BOOL );		//!<	
BOOLEAN	Cls_OnCreate( HWND, LPCREATESTRUCT );			//!<	本体の WM_CREATE の処理・固定Editとかつくる
VOID	Cls_OnCommand( HWND , INT, HWND, UINT );		//!<	本体の WM_COMMAND の処理
VOID	Cls_OnPaint( HWND );							//!<	本体の WM_PAINT の処理・枠線描画とか
VOID	Cls_OnSize( HWND , UINT, INT, INT );			//!<	
VOID	Cls_OnMove( HWND, INT, INT );					//!<	
VOID	Cls_OnDestroy( HWND );							//!<	本体の WM_DESTROY の処理・BRUSHとかのオブジェクトの破壊を忘れないように
LRESULT	Cls_OnNotify( HWND , INT, LPNMHDR );			//!<	
VOID	Cls_OnTimer( HWND, UINT );						//!<	
VOID	Cls_OnDropFiles( HWND , HDROP );				//!<	
VOID	Cls_OnContextMenu(HWND,HWND,UINT,UINT );		//!<	
VOID	Cls_OnHotKey(HWND, INT, UINT, UINT );			//!<	
VOID	Cls_OnDrawItem( HWND, CONST DRAWITEMSTRUCT * );	//!<	

INT_PTR	CALLBACK OptionDlgProc( HWND, UINT, WPARAM, LPARAM );	//!<	

LRESULT	CALLBACK gpfFileTabProc( HWND, UINT, WPARAM, LPARAM );	//!<	複数ファイルタブのサブクラスプロシージャ
VOID	Ftb_OnMButtonUp( HWND, INT, INT, UINT );	//!<	

HRESULT	ViewingFontNameLoad( VOID );

#ifdef USE_NOTIFYICON
VOID	TaskTrayIconEvent( HWND, UINT, UINT );
#endif
//-------------------------------------------------------------------------------------------------


/*!
	アプリケーションのエントリポイント
	@param[in]	hInstance		このモジュールのインスタンスハンドル
	@param[in]	hPrevInstance	前のインスタンス。今は未使用
	@param[in]	lpCmdLine		コマンドライン。トークン分解はされてない、ただの文字列
	@param[in]	nCmdShow		起動時の表示状態が入ってる。表示とかそういうの
	@retval FALSE	途中終了
*/
INT APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow )
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	INT		msRslt;
	MSG		msg;

	INT		iArgc;
	TCHAR	atArgv[MAX_PATH];
	LPTSTR	*pptArgs;

	INT		iCode;

	LPACCEL	pstAccel;
	INT	iEntry;

#ifdef WORK_LOG_OUT
	SYSTEMTIME	stTime;
#endif

#ifdef _DEBUG
	//_CRTDBG_ALLOC_MEM_DF;		//	指定が必要なフラグ
	//_CRTDBG_CHECK_ALWAYS_DF;	//	メモリをチェック		_CRTDBG_CHECK_EVERY_128_DF
	//_CRTDBG_LEAK_CHECK_DF;		//	終了時にメモリリークをチェック
	//_CRTDBG_DELAY_FREE_MEM_DF;	//	
	//	ここで使用するフラグを指定
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF );
	//_CrtSetBreakAlloc( 1979 );
	//	数値はメモリ割当番号。リークしてたら、その表示のところに
	//	書いてあるから、この関数で数値を宣言してもう一回実行すると、
	//	リークする確保をしてるところで止まる
	//	結構奥の方だから注意
#endif

	//	多重起動防止
	ghMutex = CreateMutex( NULL , TRUE, TEXT("OrinrinEditor") );	//	すでに起動しているか判定
	if( GetLastError() == ERROR_ALREADY_EXISTS )	//	すでに起動している
	{
		MessageBox( NULL, TEXT("已にアプリは起動してるよ！"), TEXT("お燐からのお知らせ"), MB_OK|MB_ICONINFORMATION );
		ReleaseMutex( ghMutex );
		CloseHandle( ghMutex );
		return 0;
	}

	//	コマンドライン引数を確認・０は実行ファイル名、１以降に引数入ってる
	ZeroMemory( atArgv, sizeof(atArgv) );
	pptArgs = CommandLineToArgvW( GetCommandLine( ), &iArgc );
	//	最初の一つ目だけ有効
	if( 2 <= iArgc ){	StringCchCopy( atArgv, MAX_PATH, pptArgs[1] );	}
	LocalFree( pptArgs );



	INITCOMMONCONTROLSEX	iccex;
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	iccex.dwICC  = ICC_WIN95_CLASSES | ICC_USEREX_CLASSES | ICC_COOL_CLASSES;
	InitCommonControlsEx( &iccex );

	// グローバル文字列を初期化しています。
	LoadString(hInstance, IDS_APP_TITLE, gszTitle, MAX_STRING);
	LoadString(hInstance, IDC_ORINRINEDITOR, gszWindowClass, MAX_STRING);
	InitWndwClass( hInstance );

	//	設定ファイル位置確認
	GetModuleFileName( hInstance, gatExePath, MAX_PATH );
	PathRemoveFileSpec( gatExePath );

	StringCchCopy( gatIniPath, MAX_PATH, gatExePath );
	PathAppend( gatIniPath, INI_FILE );

	DocBackupDirectoryInit( gatExePath );
	FrameInitialise( gatExePath, hInstance );
	MoziInitialise( gatExePath, hInstance );

	AacBackupDirectoryInit( gatExePath );

	gbUniPad = 0;

	gbDockTmplView = TRUE;

#ifdef WORK_LOG_OUT
	iCode = InitParamValue( INIT_LOAD, VL_WORKLOG, 0 );
	if( iCode )
	{
		ghLogFlie = CreateFile( TEXT("LogFile.txt"), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if( INVALID_HANDLE_VALUE != ghLogFlie )
		{
			SetFilePointer( ghLogFlie, 0, NULL, FILE_END );

			GetLocalTime( &stTime );
			TRACE( TEXT("LOG START [%u/%02u/%02u %02u:%02u:%02u]"), stTime.wYear, stTime.wMonth, stTime.wDay, stTime.wHour, stTime.wMinute, stTime.wSecond );
		}
	}
	else
	{
		ghLogFlie = INVALID_HANDLE_VALUE;
	}
#endif


	iCode = InitParamValue( INIT_LOAD, VL_CLASHCOVER, 0 );
	if( iCode )
	{
		TRACE( TEXT("異常終了の可能性アリ") );
		iCode = MessageBox( NULL, TEXT("エディタが正しく終了出来なかった気配があるよ。\r\nバックアップが残っているかもしれないから、先に確認してみて！\r\nこのまま起動してもいいかい？　「いいえ」を選ぶと、ここで終了するよ。"), TEXT("ごめんね"), MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2 );
		if( IDNO == iCode ){	return 0;	}
	}

	InitParamValue( INIT_SAVE, VL_CLASHCOVER, 1 );


	ViewingFontNameLoad(  );	//	フォント名確保

	// アプリケーションの初期化を実行します:
	if( !InitInstance( hInstance, nCmdShow , atArgv ) ){	return FALSE;	}

	CntxEditInitialise( gatExePath, hInstance );

	VertInitialise( gatExePath, hInstance );

	RegisterHotKey( ghMainWnd, IDHK_THREAD_DROP, MOD_CONTROL | MOD_SHIFT, VK_D );

	//	読み込むテーブルに注意
	//ghAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ORINRINEDITOR));
	pstAccel = AccelKeyTableLoadAlloc( &iEntry );
	AccelKeyTableCreate( pstAccel, iEntry );
	FREE( pstAccel );

#ifdef PLUGIN_ENABLE
	//---------------------------------------
	//[_16in] Add - 2012/04/30
	//	プラグイン初期化処理

	// プラグインへ渡すパラメータの設定
	// ウィンドウハンドル
	plugin::PluginInputParam	param;
	param.hMainWnd    = ghMainWnd;
	param.hMlutAAWnd  = ghMaaWnd;
	param.hFileTabWnd = ghFileTabWnd;
	param.hViewWnd    = ghViewWnd;

	// メニューハンドル
	param.hMainMenu   = ghMenu;

	// プラグインの初期化
	plugin::InitializePlugin( param );
	plugin::InitializePluginFileListAsync( ghMainWnd );

	//---------------------------------------
#endif


	//	メインメッセージループ
	for(;;)
	{
		msRslt = GetMessage( &msg, NULL, 0, 0 );
		if( 1 != msRslt )	break;

#ifdef FIND_STRINGS	//	文字列検索ダイヤログ
		if( ghFindDlg )
		{	//トップに来てるかどうか判断する
			if( ghFindDlg == GetForegroundWindow(  ) )
			{
				if( TranslateAccelerator( ghFindDlg, ghAccelTable, &msg ) )	continue;
				if( IsDialogMessage( ghFindDlg, &msg ) )	continue;
			}
		}
#endif
		//	MAA検索ダイヤログ
		if( ghMaaFindDlg )
		{	//トップに来てるかどうか判断する
			if( ghMaaFindDlg == GetForegroundWindow(  ) )
			{
				if( TranslateAccelerator( ghMaaFindDlg, ghAccelTable, &msg ) )	continue;
				if( IsDialogMessage( ghMaaFindDlg, &msg ) )	continue;
			}
		}

		if( !TranslateAccelerator( msg.hwnd, ghAccelTable, &msg ) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

#ifdef PLUGIN_ENABLE
	//---------------------------------------------------------------------
	//[_16in] Add - 2012/05/01
	//
	//-- プラグインの廃棄処理
	plugin::FinalizePluginList( gPluginList );
	plugin::FinalizePlugin();

	//---------------------------------------------------------------------
#endif


	InitParamValue( INIT_SAVE, VL_CLASHCOVER, 0 );

	UnregisterHotKey( ghMainWnd, IDHK_THREAD_DROP );

	ReleaseMutex( ghMutex );
	CloseHandle( ghMutex );

	TRACE( TEXT("Program Terminate") );

#ifdef WORK_LOG_OUT
	if( INVALID_HANDLE_VALUE != ghLogFlie )
	{
		SetEndOfFile( ghLogFlie );	
		CloseHandle( ghLogFlie );	
	}
#endif

	return (int)msg.wParam;
}
//-------------------------------------------------------------------------------------------------

LPTSTR ExePathGet( VOID )
{
	return gatExePath;
}
//-------------------------------------------------------------------------------------------------

/*!
	ウインドウクラス生成
	この関数および使い方は、'RegisterClassEx' 関数が追加された
	Windows 95 より前の Win32 システムと互換させる場合にのみ必要です。
	アプリケーションが、関連付けられた正しい形式の小さいアイコンを
	取得できるようにするには、この関数を呼び出してください。
	@param[in]	hInstance	このモジュールのインスタンスハンドル
	@return 登録したクラスアトム
*/
ATOM InitWndwClass( HINSTANCE hInstance )
{
	WNDCLASSEX	wcex;

	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ORINRINEDITOR));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_ORINRINEDITOR);
	wcex.lpszClassName	= gszWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}
//-------------------------------------------------------------------------------------------------

/*!
	インスタンスハンドルを保存して、メインウィンドウを作成します。
	この函数で、グローバル変数でインスタンスハンドルを保存し、
	メインプログラムウィンドウを作成および表示します。
	@param[in]	hInstance	インスタンスハンドル
	@param[in]	nCmdShow	起動時の表示状態
	@param[in]	ptArgv		コマンドラインで渡されたファイル名・無ければ０クルヤ状態・NULLではない
*/
BOOL InitInstance( HINSTANCE hInstance, INT nCmdShow, LPTSTR ptArgv )
{
	HWND	hWnd;
	RECT	rect, wnRect;
	INT		isMaxim = 0, sptBuf = PLIST_DOCK;
//	DWORD	dwStyle;

	HMENU	hSubMenu;


	ghInst = hInstance;	//	グローバル変数にインスタンス処理を格納します。

	SplitBarClass( hInstance );	//	スプリットバーの準備

	isMaxim = InitParamValue( INIT_LOAD, VL_MAXIMISED, 0 );

	InitWindowPos( INIT_LOAD, WDP_MVIEW, &rect );
	if( 0 == rect.right || 0 == rect.bottom )	//	幅高さが０はデータ無し
	{
		hWnd = GetDesktopWindow( );
		GetWindowRect( hWnd, &rect );
		rect.left   = ( rect.right  - W_WIDTH ) / 2;
		rect.top    = ( rect.bottom - W_HEIGHT ) / 2;
		rect.right  = W_WIDTH;
		rect.bottom = W_HEIGHT;
		InitWindowPos( INIT_SAVE, WDP_MVIEW, &rect );
	}

	hWnd = CreateWindowEx( 0, gszWindowClass, gszTitle, WS_OVERLAPPEDWINDOW, rect.left, rect.top, rect.right, rect.bottom, NULL, NULL, hInstance, NULL);

	if( !hWnd ){	return FALSE;	}

	gbUniPad      = InitParamValue( INIT_LOAD, VL_USE_UNICODE,  1 );	//	ユニコ空白　１使う　０使わない

	gbUniRadixHex = InitParamValue( INIT_LOAD, VL_UNIRADIX_HEX, 1 );

	gdBUInterval  = InitParamValue( INIT_LOAD, VL_BACKUP_INTVL, 3 );
	gbAutoBUmsg   = InitParamValue( INIT_LOAD, VL_BACKUP_MSGON, 1 );	//	
	gbCrLfCode    = InitParamValue( INIT_LOAD, VL_CRLF_CODE, 0 );		//	０したらば　１YY

	gbTmpltDock   = InitParamValue( INIT_LOAD, VL_PLS_LN_DOCK,  1 );	//	０独立　１くっつける

	ghMainWnd = hWnd;

	//	機能チェック
	//hSubMenu = GetSystemMenu( hWnd, FALSE );
	//DeleteMenu( hSubMenu, 1, MF_BYPOSITION );
	//InsertMenu( hSubMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, 0 );

	ghMenu = GetMenu( hWnd );

	//	メニューにツリー構築はいらないのでカット
	hSubMenu = GetSubMenu( ghMenu , 0 );	//	ファイル
	DeleteMenu( hSubMenu, IDM_TREE_RECONSTRUCT, MF_BYCOMMAND );
#ifndef ACCELERATOR_EDIT
	DeleteMenu( hSubMenu, IDM_ACCELKEY_EDIT_DLG_OPEN, MF_BYCOMMAND );
#endif

	hSubMenu = GetSubMenu( ghMenu , 1 );	//	編集
#ifndef FIND_STRINGS
	DeleteMenu( hSubMenu, IDM_FIND_DLG_OPEN, MF_BYCOMMAND );
	DeleteMenu( hSubMenu, IDM_FIND_HIGHLIGHT_OFF, MF_BYCOMMAND );
//	DeleteMenu( hSubMenu, 19, MF_BYPOSITION );	//	削除順番注意
#endif

//	hSubMenu = GetSubMenu( ghMenu , 2 );	//	挿入

	hSubMenu = GetSubMenu( ghMenu , 4 );	//	表示
	if( gbTmpltDock )
	{
		DeleteMenu( hSubMenu, IDM_PAGELIST_VIEW, MF_BYCOMMAND );
		DeleteMenu( hSubMenu, IDM_LINE_TEMPLATE, MF_BYCOMMAND );
		DeleteMenu( hSubMenu, IDM_BRUSH_PALETTE, MF_BYCOMMAND );
	}


	AaFontCreate( 1 );


	AppClientAreaCalc( &rect  );	//	

	SqnSetting(  );

	if( gbTmpltDock )
	{
		//	設定からスプリットバーの位置を引っ張る
		grdSplitPos = InitParamValue( INIT_LOAD, VL_MAIN_SPLIT, PLIST_DOCK );
		if( grdSplitPos < SPLITBAR_WIDTH || rect.right <= grdSplitPos ){	grdSplitPos = PLIST_DOCK;	}
		sptBuf = grdSplitPos;

		ghMainSplitWnd = SplitBarCreate( hInstance, hWnd, rect.right - grdSplitPos, rect.top, rect.bottom );
	}
	else{	ghMainSplitWnd = NULL;	grdSplitPos = 0;	}


	ghPgVwWnd = PageListInitialise( hInstance, hWnd, &rect );

	ghLnTmplWnd = LineTmpleInitialise( hInstance, hWnd, &rect );

	ViewInitialise( hInstance, hWnd, &rect, ptArgv );
	ViewMaaItemsModeSet( InitParamValue( INIT_LOAD, VL_MAA_LCLICK, MAA_INSERT ), InitParamValue( INIT_LOAD, VL_MAA_MCLICK, MAA_INSERT ) );

	LayerBoxInitialise( hInstance, &rect );
	LayerBoxAlphaSet( InitParamValue( INIT_LOAD, VL_LAYER_TRANS, 192 ) );

	UserDefInitialise( hWnd, TRUE );

	PreviewInitialise( hInstance, hWnd );

	FrameNameModifyMenu( hWnd );

	TraceInitialise( hWnd, TRUE );

	OpenHistoryInitialise( hWnd );

	OpenProfileInitialise( hWnd );

#ifdef AA_INVERSE
	DocInverseInit( TRUE );
#endif

#ifdef USE_HOVERTIP
	HoverTipInitialise( hInstance, hWnd );
#endif

	SetFocus( ghViewWnd );

	if( 1 <= gdBUInterval )
	{
		SetTimer( hWnd, IDT_BACKUP_TIMER, (gdBUInterval * 60000), NULL );
	}

	ghBrTmplWnd = BrushTmpleInitialise( hInstance, hWnd, &rect, ghMaaWnd );	//	ブラシ窓


	ghMaaWnd = MaaTmpltInitialise( hInstance, hWnd, &wnRect );

	MenuItemCheckOnOff( IDM_MAATMPLE_VIEW, MaaViewToggle( 0 ) );	//	複数行テンプレ窓の表示・非表示のチェキ

	MenuItemCheckOnOff( IDM_UNICODE_TOGGLE , gbUniPad );	 //	ユニコードON/OFFのチェキ

	DraughtInitialise( hInstance, hWnd );

	if( isMaxim )
	{
		ShowWindow( hWnd, SW_MAXIMIZE );
		AppClientAreaCalc( &rect  );	//	
		grdSplitPos = sptBuf;
		SetWindowPos( ghMainSplitWnd, HWND_TOP, rect.right - grdSplitPos, rect.top, 0, 0, SWP_NOSIZE );
		Cls_OnSize( hWnd, SIZE_MINIMIZED, rect.right, rect.top );
	}
	else{	ShowWindow( hWnd, nCmdShow );	}

	UpdateWindow( hWnd );

	return TRUE;
}
//-------------------------------------------------------------------------------------------------

#ifdef ACCELERATOR_EDIT

/*!
	アクセラテーブルをダイナミックに確保
	@param[out]	piEntry	確保したエントリ数を戻す
	@return		確保したテーブル構造体のポインター・freeに注意
*/
LPACCEL AccelKeyTableGetAlloc( LPINT piEntry )
{
	INT	iItems;
	LPACCEL	pstAccel = NULL;

	if( !(piEntry) )	return NULL;
	*piEntry = 0;

	//	まず個数確保
	iItems = CopyAcceleratorTable( ghAccelTable, NULL, 0 );
	if( 0 >= iItems )	return NULL;

	pstAccel = (LPACCEL)malloc( iItems * sizeof(ACCEL) );
	if( !(pstAccel) )	return NULL;

	iItems = CopyAcceleratorTable( ghAccelTable, pstAccel, iItems );

	*piEntry = iItems;

	return pstAccel;
}
//-------------------------------------------------------------------------------------------------

/*!
	構造体テーブルを受け取って、それをアクセラハンドルにする・NULLならResourceから
	@param[in]	pstAccel	内容テーブル・無い時はNULL
	@param[in]	iEntry		テーブルのエントリ数
	@return	アクセラハンドル
*/
HACCEL AccelKeyTableCreate( LPACCEL pstAccel, INT iEntry )
{
	HACCEL	hAccel;
	
	if( pstAccel )
	{
		DestroyAcceleratorTable( ghAccelTable );	//	前のヤツぶっ壊して

		hAccel = CreateAcceleratorTable( pstAccel, iEntry );
	}
	else
	{
		hAccel = LoadAccelerators( ghInst, MAKEINTRESOURCE(IDC_ORINRINEDITOR) );
	}

	ghAccelTable = hAccel;	//	大域変数にいれる・本当はこういうやり方は良くないと思われ

	AccelKeyMenuRewrite( ghMainWnd, pstAccel, iEntry );

	return hAccel;
}
//-------------------------------------------------------------------------------------------------
#endif

/*!
	タイトルバーを変更する
	@param[in]	ptText	タイトルにくっつける文字列・NULLで終わってないと酷い目にあうぞ
	@return		HRESULT	終了状態コード
*/
HRESULT AppTitleChange( LPTSTR ptText )
{
	LPTSTR	ptName;
	TCHAR	atBuff[MAX_PATH];

	if( ptText )
	{
		if( NULL != ptText[0] ){	ptName = PathFindFileName( ptText );	}
		else{						ptName = NAMELESS_DUMMY;	}
		StringCchPrintf( atBuff, MAX_PATH, TEXT("%s - %s"), gszTitle, ptName );
	}
	else
	{
		StringCchCopy( atBuff, MAX_PATH, gszTitle );
	}

	SetWindowText( ghMainWnd, atBuff );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	タイトルバーにトレスモード表記を漬けたり消したり
	@param[in]	bMode	非０トレスモード開始　０終わり
	@return		HRESULT	終了状態コード
*/
HRESULT AppTitleTrace( UINT bMode )
{
	static	TCHAR	atOrig[MAX_PATH];
	TCHAR	atBuff[MAX_PATH];

	if( bMode )
	{
		GetWindowText( ghMainWnd, atOrig, MAX_PATH );
		StringCchPrintf( atBuff, MAX_PATH, TEXT("%s [トレスモード]"), atOrig );
		SetWindowText( ghMainWnd, atBuff );
	}
	else
	{
		if( 0 != atOrig[0] ){	SetWindowText( ghMainWnd , atOrig );	}
		else{	SetWindowText( ghMainWnd, gszTitle );	}
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	バージョン情報ボックスのメッセージハンドラです。
	@param[in]	hDlg		ダイヤログハンドル
	@param[in]	message		ウインドウメッセージの識別番号
	@param[in]	wParam		追加の情報１
	@param[in]	lParam		追加の情報２
	@retval 0	メッセージは処理していない
	@retval no0	なんか処理された
*/
INT_PTR CALLBACK About( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		case WM_INITDIALOG:
			SetDlgItemText( hDlg, IDE_ABOUT_DISP, gcatLicense );
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	メインのウインドウプロシージャ
	@param[in]	hWnd		親ウインドウのハンドル
	@param[in]	message		ウインドウメッセージの識別番号
	@param[in]	wParam		追加の情報１
	@param[in]	lParam		追加の情報２
	@retval 0	メッセージ処理済み
	@retval no0	ここでは処理せず次に回す
*/
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	HMENU	hSubMenu;
#ifdef NDEBUG
	INT_PTR	iRslt;
#endif

	switch( message )
	{
		HANDLE_MSG( hWnd, WM_CREATE,		Cls_OnCreate );		//	画面の構成パーツを作る。ボタンとか
		HANDLE_MSG( hWnd, WM_PAINT,			Cls_OnPaint );		//	画面の更新とか
		HANDLE_MSG( hWnd, WM_COMMAND,		Cls_OnCommand );	//	ボタン押されたとかのコマンド処理
		HANDLE_MSG( hWnd, WM_DESTROY,		Cls_OnDestroy );	//	ソフト終了時の処理
		HANDLE_MSG( hWnd, WM_SIZE,			Cls_OnSize );		//	サイズ変更されたら
		HANDLE_MSG( hWnd, WM_MOVE,			Cls_OnMove );		//	位置変更されたら
		HANDLE_MSG( hWnd, WM_DROPFILES,		Cls_OnDropFiles );	//	ドラッグンドロッペ
		HANDLE_MSG( hWnd, WM_ACTIVATE,		Cls_OnActivate );	//	アクティブになったりはずれたり
		HANDLE_MSG( hWnd, WM_NOTIFY,		Cls_OnNotify  );	//	コモンコントロールの個別イベント
		HANDLE_MSG( hWnd, WM_TIMER,			Cls_OnTimer );		//	タイマ
		HANDLE_MSG( hWnd, WM_CONTEXTMENU,	Cls_OnContextMenu );//	右クリめにう
		HANDLE_MSG( hWnd, WM_HOTKEY,		Cls_OnHotKey  );	//	ホットキーが押された

		HANDLE_MSG( hWnd, WM_DRAWITEM,		Cls_OnDrawItem );	//	オーナードロー

		HANDLE_MSG( hWnd, WM_KEYDOWN,		Evw_OnKey );		//	
		HANDLE_MSG( hWnd, WM_KEYUP,			Evw_OnKey );		//	
		HANDLE_MSG( hWnd, WM_CHAR,			Evw_OnChar );		//	
		HANDLE_MSG( hWnd, WM_MOUSEWHEEL,	Evw_OnMouseWheel );	//	

		case WM_MBUTTONUP:	TRACE( TEXT("MIDDLE  UP") );	break;

#ifdef NDEBUG
		case WM_CLOSE:
			iRslt = DocFileCloseCheck( hWnd, TRUE );
			if( !(iRslt)  ){	return FALSE;	}
			break;
#endif

#ifdef USE_NOTIFYICON
		//	タスクトレイのアイコンでエベンツ発生
		case WMP_TRAYNOTIFYICON:
			TaskTrayIconEvent( hWnd, (UINT)wParam, (UINT)lParam );
			return 0;
#endif

		case WMP_BRUSH_TOGGLE:
			hSubMenu = GetSubMenu( ghMenu, 1 );
			CheckMenuItem( hSubMenu, IDM_BRUSH_STYLE, wParam ? MF_CHECKED : MF_UNCHECKED );
			return 0;

		case WMP_PREVIEW_CLOSE:
			DestroyCaret(  );
			ViewFocusSet(  );
			ViewShowCaret(  );
			return 0;

		default:	break;
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------

/*!
	あくちぶなったら呼ばれる
	@param[in]	hWnd			ウインドウハンドル
	@param[in]	state			状態
	@param[in]	hWndActDeact	あくちぶ又は非あくちぶになるやつ・NULLでも可
	@param[in]	fMinimized		最小化ならNULL
*/
VOID Cls_OnActivate( HWND hWnd, UINT state, HWND hWndActDeact, BOOL fMinimized )
{
	LONG_PTR	rdExStyle;
	HWND		hWorkWnd;
//	BOOLEAN		bRslt;

/*
WA_INACTIVE		0	ウィンドウが非アクティブになります。
WA_ACTIVE		1	マウスをクリックする以外の方法 (ウィンドウの選択にキーボード インターフェイスを使用する方法など) で、ウィンドウがアクティブになります。
WA_CLICKACTIVE	2	マウスクリックによって、ウィンドウがアクティブになります。
*/
//	TRACE( TEXT("MAIN_WM_ACTIVATE STATE[%u] HWND[%X] MIN[%u]"), state, hWndActDeact, fMinimized );

	if( WA_INACTIVE == state )
	{
	}
	else
	{
		rdExStyle = GetWindowLongPtr( ghPgVwWnd, GWL_EXSTYLE );
		if( WS_EX_TOPMOST & rdExStyle )	hWorkWnd = HWND_TOPMOST;
		else							hWorkWnd = hWnd;
		SetWindowPos( ghPgVwWnd, hWorkWnd, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

		if( InitParamValue( INIT_LOAD, VL_MAA_TOPMOST, 1 ) )	//	
		{
			rdExStyle = GetWindowLongPtr( ghMaaWnd, GWL_EXSTYLE );
			if( WS_EX_TOPMOST & rdExStyle )	hWorkWnd = HWND_TOPMOST;
			else							hWorkWnd = hWnd;
			SetWindowPos( ghMaaWnd, hWorkWnd, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );
		}

		if( IsWindowVisible(ghLnTmplWnd ) )
		{
			rdExStyle = GetWindowLongPtr( ghLnTmplWnd, GWL_EXSTYLE );
			if( WS_EX_TOPMOST & rdExStyle )	hWorkWnd = HWND_TOPMOST;
			else							hWorkWnd = hWnd;
			SetWindowPos( ghLnTmplWnd, hWorkWnd, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );
		}

		if( IsWindowVisible(ghBrTmplWnd ) )
		{
			rdExStyle = GetWindowLongPtr( ghBrTmplWnd, GWL_EXSTYLE );
			if( WS_EX_TOPMOST & rdExStyle )	hWorkWnd = HWND_TOPMOST;
			else							hWorkWnd = hWnd;
			SetWindowPos( ghBrTmplWnd, hWorkWnd, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );
		}

		TRACE( TEXT("WINDOW ACTVATE") );
		ViewFocusSet(  );
	}

	return;
}
//-------------------------------------------------------------------------------------------------


/*!
	メインのクリエイト。
	@param[in]	hWnd			親ウインドウのハンドル
	@param[in]	lpCreateStruct	アプリケーションの初期化内容
	@return	TRUE	特になし
*/
BOOLEAN Cls_OnCreate( HWND hWnd, LPCREATESTRUCT lpCreateStruct )
{
	RECT	clRect, tbRect;

	RECT	tiRect;
	TCITEM	stTcItem;
	TTTOOLINFO	stToolInfo;

	HINSTANCE lcInst = lpCreateStruct->hInstance;	//	受け取った初期化情報から、インスタンスハンドルをひっぱる


	DragAcceptFiles( hWnd, TRUE );

	GetClientRect( hWnd, &clRect );

#ifdef USE_NOTIFYICON
	NOTIFYICONDATA	nid;
	//	タスクトレイアイコンとバルーンヘルプ
	ZeroMemory( &nid, sizeof(NOTIFYICONDATA) );
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd   = hWnd;
	nid.uID    = 0;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WMP_TRAYNOTIFYICON;
	nid.hIcon  = LoadIcon( lcInst, MAKEINTRESOURCE(IDI_ORINRINEDITOR) );
	StringCchCopy( nid.szTip, 128, gszTitle );
	Shell_NotifyIcon( NIM_ADD, &nid );
#endif

//ツールバー作成
	ToolBarCreate( hWnd, lcInst );

	ghNameFont = CreateFont( 12, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, VARIABLE_PITCH, TEXT("MS UI Gothic") );

//マルチファイルオーポン
	ghFileTabWnd = CreateWindowEx( 0, WC_TABCONTROL, TEXT("filetab"), WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | TCS_SINGLELINE, 0, 0, clRect.right, 0, hWnd, (HMENU)IDTB_MULTIFILE, lcInst, NULL );
	SetWindowFont( ghFileTabWnd, ghNameFont, FALSE );

	//	サブクラス
	gpfOriginFileTabProc = SubclassWindow( ghFileTabWnd, gpfFileTabProc );

	ZeroMemory( &stTcItem, sizeof(TCITEM) );
	stTcItem.mask    = TCIF_TEXT;
	stTcItem.pszText = NAMELESS_DUMMY;
	TabCtrl_InsertItem( ghFileTabWnd, 0, &stTcItem );

	ToolBarSizeGet( &tbRect );
	TabCtrl_GetItemRect( ghFileTabWnd, 1, &tiRect );
	tiRect.bottom  += tiRect.top;
	MoveWindow( ghFileTabWnd, 0, tbRect.bottom, clRect.right, tiRect.bottom, TRUE );

	//	ツールチップ・ファイルパス表示汁
	ghFileTabTip = CreateWindowEx( WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, ghFileTabWnd, NULL, lcInst, NULL );
	//	ツールチップをコールバックで割り付け
	ZeroMemory( &stToolInfo, sizeof(TTTOOLINFO) );
	GetClientRect( ghFileTabWnd, &stToolInfo.rect );
	stToolInfo.cbSize   = sizeof(TTTOOLINFO);
	stToolInfo.uFlags   = TTF_SUBCLASS;
	stToolInfo.hinst    = NULL;	//	
	stToolInfo.hwnd     = ghFileTabWnd;
	stToolInfo.uId      = IDTT_TILETAB_TIP;
	stToolInfo.lpszText = LPSTR_TEXTCALLBACK;	//	コレを指定するとコールバックになる
	SendMessage( ghFileTabTip, TTM_ADDTOOL, 0, (LPARAM)&stToolInfo );
	SendMessage( ghFileTabTip, TTM_SETMAXTIPWIDTH, 0, 0 );	//	チップの幅。０設定でいい。これしとかないと改行されない


	ghStsRedBrush = CreateSolidBrush( 0xFF );

	ghStsBarWnd = CreateStatusWindow( WS_CHILD | WS_VISIBLE | CCS_BOTTOM | SBARS_SIZEGRIP, TEXT(""), hWnd, IDSB_VIEW_STATUS_BAR );
	SendMessage( ghStsBarWnd, SB_SETPARTS, (WPARAM)SB_ITEMS, (LPARAM)(LPINT)gadStsBarSize );

	StatusBar_SetText( ghStsBarWnd, 1 , TEXT("") );

//	StatusBar_SetText( ghStsBarWnd, (SB_BYTECNT | SBT_OWNERDRAW) , 0 );	//	オーナードロー

	return TRUE;
}
//-------------------------------------------------------------------------------------------------

/*!
	メインのCOMMANDメッセージの受け取り。ボタン押されたとかで発生
	@param[in]	hWnd		親ウインドウのハンドル
	@param[in]	id			メッセージを発生させた子ウインドウの識別子	LOWORD(wParam)
	@param[in]	hWndCtl		メッセージを発生させた子ウインドウのハンドル	lParam
	@param[in]	codeNotify	通知メッセージ	HIWORD(wParam)
	@return		なし
*/
VOID Cls_OnCommand( HWND hWnd, INT id, HWND hWndCtl, UINT codeNotify )
{
	INT		iRslt;

	//	キーボードショートカットは、そのときACTIVEなウインドウに行く

	switch( id )
	{
		case  IDM_ABOUT:	DialogBox( ghInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About );	return;

		case  IDM_EXIT:	//	WM_CLOSEとの整合性に注意セヨ
			iRslt = DocFileCloseCheck( hWnd, TRUE );
			if( iRslt ){	DestroyWindow( hWnd );	}
			return;
	}

	OperationOnCommand( hWnd, id, hWndCtl, codeNotify );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	PAINT。無効領域が出来たときに発生。背景の扱いに注意。背景を塗りつぶしてから、オブジェクトを描画
	@param[in]	hWnd	親ウインドウのハンドル
	@return		無し
*/
VOID Cls_OnPaint( HWND hWnd )
{
	PAINTSTRUCT	ps;
	HDC			hdc;

	hdc = BeginPaint( hWnd, &ps );

	EndPaint( hWnd, &ps );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	タイマのアレ
	@param[in]	hWnd	親ウインドウのハンドル
	@param[in]	id		タイマＩＤ
	@return		無し
*/
VOID Cls_OnTimer( HWND hWnd, UINT id )
{
	if( IDT_BACKUP_TIMER != id )	return;
	//	タイマ増えたら注意

	DocFileBackup( hWnd );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ウインドウがサイズ変更されたとき
	@param[in]	hWnd	親ウインドウのハンドル
	@param[in]	state	変更の状態・SIZE_MINIMIZED とか
	@param[in]	cx		クライヤントＸサイズ
	@param[in]	cy		クライヤントＹサイズ
	@return		無し
*/
VOID Cls_OnSize( HWND hWnd, UINT state, INT cx, INT cy )
{
	static  UINT	ccState;

	RECT	rect;

	if( SIZE_MINIMIZED == state )	//	最小化時
	{
		ShowWindow( ghMaaWnd, SW_HIDE );
		if( !(gbTmpltDock) )
		{
			ShowWindow( ghPgVwWnd, SW_HIDE );
			ShowWindow( ghLnTmplWnd, SW_HIDE );
			ShowWindow( ghBrTmplWnd, SW_HIDE );
		}
		ccState = SIZE_MINIMIZED;

		return;
	}

	if( SIZE_MINIMIZED == ccState &&  ccState != state )	//	最小化から復帰
	{
		ShowWindow( ghMaaWnd, SW_SHOW );
		if( !(gbTmpltDock) )
		{
			ShowWindow( ghPgVwWnd,   SW_SHOW );
			ShowWindow( ghLnTmplWnd, SW_SHOW );
			ShowWindow( ghBrTmplWnd, SW_SHOW );
		}
		ccState = SIZE_RESTORED;
	}

	if( SIZE_MAXIMIZED == state )	//	最大化時
	{
		AppClientAreaCalc( &rect  );	//	右に併せて移動
		SetWindowPos( ghMainSplitWnd, HWND_TOP, rect.right - grdSplitPos, rect.top, 0, 0, SWP_NOSIZE );

		ccState = SIZE_MAXIMIZED;
	}

	if( SIZE_RESTORED == state &&  SIZE_MAXIMIZED == ccState )	//	最大化から復帰か
	{
		if( !(IsZoomed( hWnd ) ) )	//	まだ最大化中なら、スプリットバー調整のはず
		{
			AppClientAreaCalc( &rect  );	//	右に併せて移動
			SetWindowPos( ghMainSplitWnd, HWND_TOP, rect.right - grdSplitPos, rect.top, 0, 0, SWP_NOSIZE );

			ccState = SIZE_RESTORED;
		}
	}

	MoveWindow( ghStsBarWnd, 0, 0, 0, 0, TRUE );	//	ステータスバーは勝手に位置とか調整されるのでこれでいい

	ToolBarOnSize( hWnd, state, cx, cy );

	AppClientAreaCalc( &rect  );	//	

	ViewSizeMove( hWnd, &rect );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ウインドウが移動したとき
	@param[in]	hWnd	親ウインドウのハンドル
	@param[in]	x		
	@param[in]	y		
	@return		無し
*/
VOID Cls_OnMove( HWND hWnd, INT x, INT y )
{
	DWORD	dwStyle;

	dwStyle = GetWindowStyle( hWnd );
	if( dwStyle & WS_MINIMIZE )
	{
		LayerMoveFromView( hWnd, SIZE_MINIMIZED );
		FrameMoveFromView( hWnd, SIZE_MINIMIZED );
		MoziMoveFromView( hWnd, SIZE_MINIMIZED );
		VertMoveFromView( hWnd, SIZE_MINIMIZED );
	}
	else
	{
		LayerMoveFromView( hWnd, SIZE_RESTORED );
		FrameMoveFromView( hWnd, SIZE_RESTORED );
		MoziMoveFromView( hWnd, SIZE_RESTORED );
		VertMoveFromView( hWnd, SIZE_RESTORED );
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ウインドウを閉じるときに発生。デバイスコンテキストとか全部終了。
	@param[in]	hWnd	親ウインドウのハンドル
	@return		無し
*/
VOID Cls_OnDestroy( HWND hWnd )
{
	RECT	rect;
	DWORD	dwStyle;

#ifdef USE_NOTIFYICON
	NOTIFYICONDATA	nid;

	ZeroMemory( &nid, sizeof(NOTIFYICONDATA) );
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd   = hWnd;
	Shell_NotifyIcon( NIM_DELETE, &nid );
#endif

#ifdef AA_INVERSE
	DocInverseInit( FALSE );
#endif

#ifdef USE_HOVERTIP
	HoverTipInitialise( NULL, NULL );
#endif

	ToolBarBandInfoGet( NULL );

	PreviewInitialise( NULL, NULL );

	TraceInitialise( hWnd, FALSE );

	OpenHistoryInitialise( NULL );

	OpenProfileInitialise( NULL );

	BrushTmpleInitialise( NULL, NULL, NULL, NULL );

	LayerBoxInitialise( NULL, NULL );

	FrameInitialise( NULL, NULL );

	MoziInitialise( NULL, NULL );

	VertInitialise( NULL, NULL );

	DraughtInitialise( NULL, NULL );

	CntxEditInitialise( NULL, NULL );

	//	ウインドウの状態を確認して、最小化状態なら記録しない
	dwStyle = GetWindowStyle( hWnd );
	if( !(dwStyle & WS_MINIMIZE) )
	{
		if( dwStyle & WS_MAXIMIZE ){	InitParamValue( INIT_SAVE, VL_MAXIMISED, 1 );	}
		else
		{
			//	最大化してたら記録しない
			GetWindowRect( ghMainWnd, &rect );
			rect.right  = rect.right  - rect.left;
			rect.bottom = rect.bottom - rect.top;
			InitWindowPos( INIT_SAVE, WDP_MVIEW, &rect );//終了時保存

			InitParamValue( INIT_SAVE, VL_MAXIMISED, 0 );
		}

		if( !(gbTmpltDock) )
		{
			GetWindowRect( ghPgVwWnd, &rect );
			rect.right  = rect.right  - rect.left;
			rect.bottom = rect.bottom - rect.top;
			InitWindowPos( INIT_SAVE, WDP_PLIST, &rect );//終了時保存

			GetWindowRect( ghLnTmplWnd, &rect );
			rect.right  = rect.right  - rect.left;
			rect.bottom = rect.bottom - rect.top;
			InitWindowPos( INIT_SAVE, WDP_LNTMPL, &rect );//終了時保存

			GetWindowRect( ghBrTmplWnd, &rect );
			rect.right  = rect.right  - rect.left;
			rect.bottom = rect.bottom - rect.top;
			InitWindowPos( INIT_SAVE, WDP_BRTMPL, &rect );//終了時保存
		}
	}


	//	MAAのはそっち側でやってる
	DestroyWindow( ghMaaWnd );

	DestroyWindow( ghPgVwWnd );


	InitMultiFileTabOpen( INIT_SAVE, 0, NULL );

	DocInitialise( FALSE );

	SetWindowFont( ghFileTabWnd, GetStockFont(DEFAULT_GUI_FONT), FALSE );

	DeleteFont( ghNameFont );

	DeleteBrush( ghStsRedBrush );

	ToolBarDestroy(  );

	DestroyWindow( ghViewWnd );
	AaFontCreate( 0 );

	PostQuitMessage( 0 );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	コンテキストメニュー呼びだしアクション(要は右クルック）
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	hWndContext	コンテキストが発生したウインドウのハンドル
	@param[in]	xPos		スクリーンＸ座標
	@param[in]	yPos		スクリーンＹ座業
	@return		無し
*/
VOID Cls_OnContextMenu( HWND hWnd, HWND hWndContext, UINT xPos, UINT yPos )
{
	HMENU	hMenu, hSubMenu;
	UINT	dRslt;
	INT		curSel;
	INT		iCount;
	POINT	stPost;
	TCHITTESTINFO	stTcHitInfo;

	MENUITEMINFO	stMenuItemInfo;

	TCITEM	stTcItem;
	TCHAR	atText[MAX_PATH];
	UINT	cchSize;

	stPost.x = (SHORT)xPos;	//	画面座標はマイナスもありうる
	stPost.y = (SHORT)yPos;

	TRACE( TEXT("MAIN CONTEXT[%d x %d]"), stPost.x, stPost.y );

	//リバーの、バー名称と左の取っ手のところは発生する・
	if( ToolBarOnContextMenu( hWnd, hWndContext, stPost.x, stPost.y ) ){	 return;	}

	if( SUCCEEDED( DockingTabContextMenu( hWnd, hWndContext, stPost.x, stPost.y ) ) ){	 return;	}

	//	複数タブ以外にはない
	if( ghFileTabWnd != hWndContext ){	 return;	}

	iCount = TabCtrl_GetItemCount( ghFileTabWnd );

	hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_MULTIFILE_POPUP) );
	hSubMenu = GetSubMenu( hMenu, 0 );

	stTcHitInfo.pt = stPost;
	ScreenToClient( ghFileTabWnd, &(stTcHitInfo.pt) );
	curSel = TabCtrl_HitTest( ghFileTabWnd, &stTcHitInfo );

	ZeroMemory( &stTcItem, sizeof(TCITEM) );
	stTcItem.mask       = TCIF_TEXT | TCIF_PARAM;
	stTcItem.pszText    = atText;
	stTcItem.cchTextMax = MAX_PATH;
	TabCtrl_GetItem( ghFileTabWnd, curSel, &stTcItem );

	//	右クリした奴を選択する
	TabCtrl_SetCurSel( ghFileTabWnd, curSel );
	DocMultiFileSelect( stTcItem.lParam );

	StringCchCat( atText, MAX_PATH, TEXT(" を閉じる(&Q)") );
	StringCchLength( atText, MAX_PATH, &cchSize );

	ZeroMemory( &stMenuItemInfo, sizeof(MENUITEMINFO) );
	stMenuItemInfo.cbSize     = sizeof(MENUITEMINFO);
	stMenuItemInfo.fMask      = MIIM_TYPE;
	stMenuItemInfo.fType      = MFT_STRING;
	stMenuItemInfo.cch        = cchSize;
	stMenuItemInfo.dwTypeData = atText;

	if( 1 >= iCount )	//	残りファイルがいっこの時は閉じれない
	{
		stMenuItemInfo.fMask |= MIIM_STATE;
		stMenuItemInfo.fState = MFS_GRAYED;
//		EnableMenuItem( hSubMenu, IDM_FILE_CLOSE, MF_BYCOMMAND | MF_GRAYED );
	}
	
	SetMenuItemInfo( hSubMenu, IDM_FILE_CLOSE, FALSE, &stMenuItemInfo );

	dRslt = TrackPopupMenu( hSubMenu, 0, stPost.x, stPost.y, 0, hWnd, NULL );

	DestroyMenu( hMenu );

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	メインウインドウのノーティファイメッセージの処理
	@param[in]	hWnd		親ウインドウのハンドル
	@param[in]	idFrom		NOTIFYを発生させたコントロールのＩＤ
	@param[in]	pstNmhdr	NOTIFYの詳細
	@return		処理した内容とか
*/
LRESULT Cls_OnNotify( HWND hWnd, INT idFrom, LPNMHDR pstNmhdr )
{
	INT	curSel;
	TCHAR	atText[MAX_PATH];
	TCITEM	stTcItem;
	LPNMTTDISPINFO	pstDispInfo;
	TCHITTESTINFO	stTcHitTest;


	ToolBarOnNotify( hWnd, idFrom, pstNmhdr );	//	ツールバーの処理

	if( IDTB_DOCK_TAB == idFrom )
	{
		if( TCN_SELCHANGE == pstNmhdr->code )
		{
			curSel = TabCtrl_GetCurSel( pstNmhdr->hwndFrom );

			TRACE( TEXT("TMPL TAB sel [%d]"), curSel );

			//	非表示時は何もしない
			if( gbDockTmplView )
			{
				//	フォーカス入れ替え
				switch( curSel )
				{
					case  0:	ShowWindow( ghLnTmplWnd , SW_SHOW );	ShowWindow( ghBrTmplWnd , SW_HIDE );	break;
					case  1:	ShowWindow( ghLnTmplWnd , SW_HIDE );	ShowWindow( ghBrTmplWnd , SW_SHOW );	break;
					default:	break;
				}
			}
		}
	}

	if( IDTB_MULTIFILE == idFrom )	//	ファイルタブ選択
	{
		if( TCN_SELCHANGE == pstNmhdr->code )
		{
			curSel = TabCtrl_GetCurSel( ghFileTabWnd );

			TRACE( TEXT("FILE TAB sel [%d]"), curSel );

			ZeroMemory( &stTcItem, sizeof(TCITEM) );
			stTcItem.mask       = TCIF_TEXT | TCIF_PARAM;
			stTcItem.pszText    = atText;
			stTcItem.cchTextMax = MAX_PATH;
			TabCtrl_GetItem( ghFileTabWnd, curSel, &stTcItem );

			TRACE( TEXT("FILE [%s] param[%d]"), atText, stTcItem.lParam );

			DocMultiFileSelect( stTcItem.lParam );
		}
	}

	if( IDTT_TILETAB_TIP == idFrom )	//	ツールチップの場合
	{
		if( TTN_GETDISPINFO ==  pstNmhdr->code )	//	内容の問い合わせだったら
		{
			GetCursorPos( &(stTcHitTest.pt) );	//	スクリーン座標
			ScreenToClient( ghFileTabWnd, &(stTcHitTest.pt) );	//	クライヤント座標に変換

			curSel = TabCtrl_HitTest( ghFileTabWnd, &stTcHitTest );
			TRACE( TEXT("FILE TAB under [%d]"), curSel );

			pstDispInfo = (LPNMTTDISPINFO)pstNmhdr;

			ZeroMemory( &(pstDispInfo->szText), sizeof(pstDispInfo->szText) );

			pstDispInfo->lpszText = DocMultiFileNameGet( curSel );
			if( !(pstDispInfo->lpszText)  )
			{
				StringCchCopy( pstDispInfo->szText, 80, TEXT(" ") );
				pstDispInfo->lpszText = pstDispInfo->szText;
			}
		}
	}

	return 0;	//	何もないなら０を戻す
}
//-------------------------------------------------------------------------------------------------

/*!
	ドラッグンドロップの受け入れ
	@param[in]	hWnd	親ウインドウのハンドル
	@param[in]	hDrop	ドロッピンオブジェクトハンドゥ
*/
VOID Cls_OnDropFiles( HWND hWnd, HDROP hDrop )
{
	TCHAR	atFileName[MAX_PATH];
//	LPARAM	dNumber;

	ZeroMemory( atFileName, sizeof(atFileName) );

	DragQueryFile( hDrop, 0, atFileName, MAX_PATH );
	DragFinish( hDrop );

	TRACE( TEXT("DROP[%s]"), atFileName );

	DocDoOpenFile( hWnd, atFileName );	//	Ｄ＆Ｄで開く

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ホットキー押されたら呼ばれる
	@param[in]	hWnd		親ウインドウのハンドル
	@param[in]	idHotKey	登録してあるＩＤ
	@param[in]	fuModifiers	CTRL、SHIFT、ALT の組み合わせ状態
	@param[in]	vk			押されている一般キー
	@return		無し
*/
VOID Cls_OnHotKey(HWND hWnd, INT idHotKey, UINT fuModifiers, UINT vk )
{
	if( VK_D == vk )
	{
		TRACE( TEXT("Hotkey Incoming!!") );
		DocThreadDropCopy(  );
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	オーナードローの処理
	@param[in]	hWnd			ウインドウハンドル
	@param[in]	*pstDrawItem	オーナドローデータ
	@return		無し
*/
VOID Cls_OnDrawItem( HWND hWnd, CONST DRAWITEMSTRUCT *pstDrawItem )
{
	UINT	dBytes;
	TCHAR	atBuff[SUB_STRING];
	RECT	rect;

	//	ステータスバー以外は関係ないので放置
	if( IDSB_VIEW_STATUS_BAR != pstDrawItem->CtlID )	return;

	if( SB_BYTECNT == pstDrawItem->itemID )
	{
		SetBkMode( pstDrawItem->hDC, TRANSPARENT );
		rect = pstDrawItem->rcItem;

		dBytes =  pstDrawItem->itemData;	//	文字列ポインタとか入ってる
		StringCchPrintf( atBuff, SUB_STRING, TEXT("%d Bytes"), dBytes );

		if( PAGE_BYTE_MAX < dBytes )	FillRect( pstDrawItem->hDC, &(pstDrawItem->rcItem), ghStsRedBrush );

		DrawText( pstDrawItem->hDC, atBuff, -1, &(rect), DT_LEFT | DT_VCENTER | DT_SINGLELINE );
	}

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	ステータスバーに文字列追加
	@param[in]	room	入れる枠０インデックス
	@param[in]	ptText	入れる文字列
	@return		HRESULT	終了状態コード
*/
HRESULT MainStatusBarSetText( INT room, LPCTSTR ptText )
{
	StatusBar_SetText( ghStsBarWnd, room , ptText );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ステータスバーにバイト数をオーナードローで乳力
	@param[in]	dByte	バイト数
	@return		HRESULT	終了状態コード
*/
HRESULT MainSttBarSetByteCount( UINT dByte )
{
	SendMessage( ghStsBarWnd, SB_SETTEXT, (WPARAM)(SB_BYTECNT | SBT_OWNERDRAW), (LPARAM)dByte );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------




/*!
	メニュー項目をチェックしたり外したり
	@param[in]	itemID	操作するメニューのＩＤ
	@param[in]	bCheck	非０チェック・０はずす
	@return		HRESULT	終了状態コード
*/
HRESULT MenuItemCheckOnOff( UINT itemID, UINT bCheck )
{
	CheckMenuItem( ghMenu, itemID, bCheck ? MF_CHECKED : MF_UNCHECKED );

	ToolBarCheckOnOff( itemID, bCheck );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	外部でブラシ機能ON/OFFした場合
	@return		HRESULT	終了状態コード
*/
HRESULT BrushModeToggle( VOID )
{
	LRESULT	rlst;
	HMENU	hSubMenu;

	rlst = SendMessage( ghBrTmplWnd, WMP_BRUSH_TOGGLE, 0, 0 );
	hSubMenu = GetSubMenu( ghMenu, 1 );
	CheckMenuItem( hSubMenu, IDM_BRUSH_STYLE, rlst ? MF_CHECKED : MF_UNCHECKED );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	メインウインドウのクライヤント領域を求める
	@param[out]	pstRect	領域いれる構造体ポインター
*/
UINT AppClientAreaCalc( LPRECT pstRect )
{
	RECT	rect, sbRect, tbRect;
	RECT	ftRect;


	if( !(pstRect) )	return 0;

	if( !(ghMainWnd)  ){	SetRect( pstRect , 0, 0, 0, 0 );	return 0;	}

	GetClientRect( ghMainWnd, &rect );	//	

	ToolBarSizeGet( &tbRect );	//	ツールバーのサイズとって
	rect.top      += tbRect.bottom;
	rect.bottom   -= tbRect.bottom;

	GetClientRect( ghStsBarWnd, &sbRect );	//	ステータスバーのサイズ確認
	rect.bottom   -= sbRect.bottom;

	GetWindowRect( ghFileTabWnd, &ftRect );	//	タブバーのサイズ確認
	ftRect.bottom -= ftRect.top;
	SetWindowPos( ghFileTabWnd, HWND_TOP, 0, tbRect.bottom, rect.right, ftRect.bottom, SWP_NOZORDER );
	rect.top      += ftRect.bottom;
	rect.bottom   -= ftRect.bottom;	//	タブバーの分縮める

	SetRect( pstRect, rect.left, rect.top, rect.right, rect.bottom );

	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	トレスの各モードの数値
	@param[in]	dMode	非０ロード　０セーブ
	@param[in]	pstInfo	ロードセーブの内容保存
	@return		比０ロードＯＫ　０なかった
*/
INT InitTraceValue( UINT dMode, LPTRACEPARAM pstInfo )
{
	TCHAR	atBuff[MIN_STRING];
	INT	iBuff = 0;

	if( dMode )	//	ロード
	{
		iBuff = GetPrivateProfileInt( TEXT("Trace"), TEXT("Turning"), -1, gatIniPath );
		if( 0 > iBuff )	return 0;

		pstInfo->dTurning     = iBuff;
		pstInfo->dZooming     = GetPrivateProfileInt( TEXT("Trace"), TEXT("Zooming"), 0, gatIniPath );
		pstInfo->dGrayMoph    = GetPrivateProfileInt( TEXT("Trace"), TEXT("GrayMoph"), 0, gatIniPath );
		pstInfo->dGamma       = GetPrivateProfileInt( TEXT("Trace"), TEXT("Gamma"), 0, gatIniPath );
		pstInfo->dContrast    = GetPrivateProfileInt( TEXT("Trace"), TEXT("Contrast"), 0, gatIniPath );
		pstInfo->stOffsetPt.y = GetPrivateProfileInt( TEXT("Trace"), TEXT("OffsetY"), 0, gatIniPath );
		pstInfo->stOffsetPt.x = GetPrivateProfileInt( TEXT("Trace"), TEXT("OffsetX"), 0, gatIniPath );
		pstInfo->bUpset       = GetPrivateProfileInt( TEXT("Trace"), TEXT("Upset"), 0, gatIniPath );
		pstInfo->bMirror      = GetPrivateProfileInt( TEXT("Trace"), TEXT("Mirror"), 0, gatIniPath );
		pstInfo->dMoziColour  = GetPrivateProfileInt( TEXT("Trace"), TEXT("MoziColour"), 0, gatIniPath );
	}
	else	//	セーブ
	{
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstInfo->stOffsetPt.x );
		WritePrivateProfileString( TEXT("Trace"), TEXT("OffsetX"), atBuff, gatIniPath );

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstInfo->stOffsetPt.y );
		WritePrivateProfileString( TEXT("Trace"), TEXT("OffsetY"), atBuff, gatIniPath );

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstInfo->dContrast );
		WritePrivateProfileString( TEXT("Trace"), TEXT("Contrast"), atBuff, gatIniPath );

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstInfo->dGamma );
		WritePrivateProfileString( TEXT("Trace"), TEXT("Gamma"), atBuff, gatIniPath );

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstInfo->dGrayMoph );
		WritePrivateProfileString( TEXT("Trace"), TEXT("GrayMoph"), atBuff, gatIniPath );

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstInfo->dZooming );
		WritePrivateProfileString( TEXT("Trace"), TEXT("Zooming"), atBuff, gatIniPath );

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstInfo->dTurning );
		WritePrivateProfileString( TEXT("Trace"), TEXT("Turning"), atBuff, gatIniPath );

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), pstInfo->bUpset );
		WritePrivateProfileString( TEXT("Trace"), TEXT("Upset"), atBuff, gatIniPath );

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), pstInfo->bMirror );
		WritePrivateProfileString( TEXT("Trace"), TEXT("Mirror"), atBuff, gatIniPath );

		StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), pstInfo->dMoziColour );
		WritePrivateProfileString( TEXT("Trace"), TEXT("MoziColour"), atBuff, gatIniPath );
	}


	return 1;
}
//-------------------------------------------------------------------------------------------------

/*!
	色設定のセーブロード
	@param[in]	dMode	非０ロード　０セーブ
	@param[in]	dStyle	パラメータの種類
	@param[in]	nColour	ロード：デフォ値　セーブ：値
	@return	COLORREF	ロード：値　セーブ：０
*/
COLORREF InitColourValue( UINT dMode, UINT dStyle, COLORREF nColour )
{
	TCHAR	atKeyName[MIN_STRING], atBuff[MIN_STRING];
	LPTSTR	ptEnd;

	switch( dStyle )
	{
		case CLRV_BASICPEN:	StringCchCopy( atKeyName, SUB_STRING, TEXT("BasicPen") );	break;	//	文字色
		case CLRV_BASICBK:	StringCchCopy( atKeyName, SUB_STRING, TEXT("BasicBack") );	break;	//	背景色
		case CLRV_GRIDLINE:	StringCchCopy( atKeyName, SUB_STRING, TEXT("GridLine") );	break;	//	グリッド
		case CLRV_CRLFMARK:	StringCchCopy( atKeyName, SUB_STRING, TEXT("CrLfMark") );	break;	//	改行マーク
		case CLRV_CANTSJIS:	StringCchCopy( atKeyName, SUB_STRING, TEXT("CantSjis") );	break;	//	Unicode文字
		default:	return nColour;
	}

	StringCchPrintf( atBuff, MIN_STRING, TEXT("%08X"), nColour );

	if( dMode )	//	ロード
	{
		GetPrivateProfileString( TEXT("Colour"), atKeyName, atBuff, atBuff, MIN_STRING, gatIniPath );
		nColour = (COLORREF)_tcstoul( atBuff, &ptEnd, 16 );
	}
	else	//	セーブ
	{
		WritePrivateProfileString( TEXT("Colour"), atKeyName, atBuff, gatIniPath );
	}

	return nColour;
}
//-------------------------------------------------------------------------------------------------

/*!
	リバー配置のセーブロード
	@param[in]		dMode	非０ロード　０セーブ
	@param[in]		items	バンドの数
	@param[in,out]	pstInfo	データを出したり入れたりする構造体配列へのポインター
	@return	HRESULT	終了状態コード
*/
HRESULT InitToolBarLayout( UINT dMode, INT items, LPREBARLAYOUTINFO pstInfo )
{
	TCHAR	atKeyName[MIN_STRING], atBuff[MIN_STRING];
	UINT	dValue;
	INT	i;


	if( dMode )	//	ロード
	{
		//	存在確認
		dValue = GetPrivateProfileInt( TEXT("ReBar"), TEXT("IDX0_ID"), 0, gatIniPath );
		if( 0 == dValue ){	return E_NOTIMPL;	}	//	ＩＤなので０にはならない

		for( i = 0; items > i; i++ )	//	インデックス順
		{
			StringCchPrintf( atKeyName, MIN_STRING, TEXT("IDX%d_ID"), i );
			pstInfo[i].wID = GetPrivateProfileInt( TEXT("ReBar"), atKeyName, pstInfo[i].wID, gatIniPath );

			StringCchPrintf( atKeyName, MIN_STRING, TEXT("IDX%d_CX"), i );
			pstInfo[i].cx = GetPrivateProfileInt( TEXT("ReBar"), atKeyName, pstInfo[i].cx, gatIniPath );

			StringCchPrintf( atKeyName, MIN_STRING, TEXT("IDX%d_STYLE"), i );
			pstInfo[i].fStyle = GetPrivateProfileInt( TEXT("ReBar"), atKeyName, pstInfo[i].fStyle, gatIniPath );
		}
	}
	else	//	セーブ
	{
		WritePrivateProfileSection( TEXT("ReBar"), NULL, gatIniPath );	//	一旦全削除

		for( i = 0; items > i; i++ )	//	インデックス順
		{
			StringCchPrintf( atKeyName, MIN_STRING, TEXT("IDX%d_ID"), i );
			StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), pstInfo[i].wID );
			WritePrivateProfileString( TEXT("ReBar"), atKeyName, atBuff, gatIniPath );

			StringCchPrintf( atKeyName, MIN_STRING, TEXT("IDX%d_CX"), i );
			StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), pstInfo[i].cx );
			WritePrivateProfileString( TEXT("ReBar"), atKeyName, atBuff, gatIniPath );

			StringCchPrintf( atKeyName, MIN_STRING, TEXT("IDX%d_STYLE"), i );
			StringCchPrintf( atBuff, MIN_STRING, TEXT("%u"), pstInfo[i].fStyle );
			WritePrivateProfileString( TEXT("ReBar"), atKeyName, atBuff, gatIniPath );
		}
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	パラメータ値のセーブロード
	@param[in]	dMode	非０ロード　０セーブ
	@param[in]	dStyle	パラメータの種類
	@param[in]	nValue	ロード：デフォ値　セーブ：値
	@return		INT	ロード：値　セーブ：０
*/
INT InitParamValue( UINT dMode, UINT dStyle, INT nValue )
{
	TCHAR	atKeyName[MIN_STRING], atBuff[MIN_STRING];
	INT	iBuff = 0;

	switch( dStyle )
	{
		case  VL_CLASHCOVER:	StringCchCopy( atKeyName, SUB_STRING, TEXT("ClashCover") );		break;
		case  VL_GROUP_UNDO:	StringCchCopy( atKeyName, SUB_STRING, TEXT("GroupUndo") );		break;
		case  VL_USE_UNICODE:	StringCchCopy( atKeyName, SUB_STRING, TEXT("UseUnicode")  );	break;
		case  VL_LAYER_TRANS:	StringCchCopy( atKeyName, SUB_STRING, TEXT("LayerTrans")  );	break;
		case  VL_RIGHT_SLIDE:	StringCchCopy( atKeyName, SUB_STRING, TEXT("RightSlide")  );	break;
		case  VL_MAA_SPLIT:		StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaSplit") );		break;
		case  VL_MAA_LCLICK:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaMethod") );		break;
		case  VL_UNILISTLAST:	StringCchCopy( atKeyName, SUB_STRING, TEXT("UniListLast") );	break;
		case  VL_MAATIP_VIEW:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaToolTip")  );	break;
		case  VL_MAATIP_SIZE:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaToolTipSize") );	break;
		case  VL_LINETMP_CLM:	StringCchCopy( atKeyName, SUB_STRING, TEXT("LineTmplClm") );	break;
		case  VL_BRUSHTMP_CLM:	StringCchCopy( atKeyName, SUB_STRING, TEXT("BrushTmplClm") );	break;
		case  VL_UNIRADIX_HEX:	StringCchCopy( atKeyName, SUB_STRING, TEXT("UniRadixHex") );	break;
		case  VL_BACKUP_INTVL:	StringCchCopy( atKeyName, SUB_STRING, TEXT("BackUpIntvl") );	break;
		case  VL_BACKUP_MSGON:	StringCchCopy( atKeyName, SUB_STRING, TEXT("BackUpMsgOn") );	break;
		case  VL_CRLF_CODE:		StringCchCopy( atKeyName, SUB_STRING, TEXT("CrLfCode") );		break;
		case  VL_SPACE_VIEW:	StringCchCopy( atKeyName, SUB_STRING, TEXT("SpaceView") );		break;
		case  VL_GRID_X_POS:	StringCchCopy( atKeyName, SUB_STRING, TEXT("GridXpos") );		break;
		case  VL_GRID_Y_POS:	StringCchCopy( atKeyName, SUB_STRING, TEXT("GridYpos") );		break;
		case  VL_GRID_VIEW:		StringCchCopy( atKeyName, SUB_STRING, TEXT("GridView") );		break;
		case  VL_MAA_TOPMOST:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaViewOn") );		break;
		case  VL_R_RULER_POS:	StringCchCopy( atKeyName, SUB_STRING, TEXT("RightRuler")  );	break;
		case  VL_R_RULER_VIEW:	StringCchCopy( atKeyName, SUB_STRING, TEXT("RitRulerView") );	break;
		case  VL_PAGETIP_VIEW:	StringCchCopy( atKeyName, SUB_STRING, TEXT("PageToolTip") );	break;
		case  VL_PCOMBINE_NM:	StringCchCopy( atKeyName, SUB_STRING, TEXT("PageCombineNM") );	break;
		case  VL_PDIVIDE_NM:	StringCchCopy( atKeyName, SUB_STRING, TEXT("PageDivideNM") );	break;
		case  VL_PDELETE_NM:	StringCchCopy( atKeyName, SUB_STRING, TEXT("PageDeleteNM") );	break;
		case  VL_MAASEP_STYLE:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaSepLine")  );	break;
		case  VL_PLS_LN_DOCK:	StringCchCopy( atKeyName, SUB_STRING, TEXT("PLstLineDock") );	break;
	//	case  VL_BRUSH_DOCK:	StringCchCopy( atKeyName, SUB_STRING, TEXT("BrushDock") );		break;
		case  VL_SWAP_COPY:		StringCchCopy( atKeyName, SUB_STRING, TEXT("CopyModeSwap") );	break;
		case  VL_MAIN_SPLIT:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MainSplit") );		break;
		case  VL_MAXIMISED:		StringCchCopy( atKeyName, SUB_STRING, TEXT("Maximised") );		break;
		case  VL_DRT_LCLICK:	StringCchCopy( atKeyName, SUB_STRING, TEXT("DraughtDefault") );	break;
		case  VL_FIRST_READED:	StringCchCopy( atKeyName, SUB_STRING, TEXT("FirstStep") );		break;
		case  VL_LAST_OPEN:		StringCchCopy( atKeyName, SUB_STRING, TEXT("LastOpenStyle") );	break;
		case  VL_MAA_MCLICK:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaSubMethod") );	break;
		case  VL_DRT_MCLICK:	StringCchCopy( atKeyName, SUB_STRING, TEXT("DraughtSubDef") );	break;
		case  VL_WORKLOG:		StringCchCopy( atKeyName, SUB_STRING, TEXT("WorkLogDebug") );	break;
		case  VL_PAGE_UNDER:	StringCchCopy( atKeyName, SUB_STRING, TEXT("PageNumUnder") );	break;
		case  VL_PAGE_OVWRITE:	StringCchCopy( atKeyName, SUB_STRING, TEXT("PageNumOvWrite") );	break;
		case  VL_MAA_RETFCS:	StringCchCopy( atKeyName, SUB_STRING, TEXT("MaaRetFocus") );	break;
		case  VL_PGL_RETFCS:	StringCchCopy( atKeyName, SUB_STRING, TEXT("PageRetFocus") );	break;

		default:	return nValue;
	}

	StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), nValue );

	if( dMode )	//	ロード
	{
		GetPrivateProfileString( TEXT("General"), atKeyName, atBuff, atBuff, MIN_STRING, gatIniPath );
		iBuff = StrToInt( atBuff );
	}
	else	//	セーブ
	{
		WritePrivateProfileString( TEXT("General"), atKeyName, atBuff, gatIniPath );
	}

	return iBuff;
}
//-------------------------------------------------------------------------------------------------

/*!
	文字列の設定内容をセーブロード
	@param[in]		dMode	非０ロード　０セーブ
	@param[in]		dStyle	パラメータの種類
	@param[in,out]	ptFile	ロード：デフォルト文字列　セーブ：保存する文字列　MAX_PATHであること
	@return			HRESULT	終了状態コード
*/
HRESULT InitParamString( UINT dMode, UINT dStyle, LPTSTR ptFile )
{
	TCHAR	atKeyName[MIN_STRING], atDefault[MAX_PATH];

	if(  !(ptFile) )	return E_INVALIDARG;

	switch( dStyle )
	{
		case VS_PROFILE_NAME:	StringCchCopy( atKeyName, SUB_STRING, TEXT("ProfilePath") );	break;
		case VS_PAGE_FORMAT:	StringCchCopy( atKeyName, SUB_STRING, TEXT("PageFormat")  );	break;
		case VS_FONT_NAME:		StringCchCopy( atKeyName, SUB_STRING, TEXT("FontName") );		break;
		default:	return E_INVALIDARG;
	}

	if( dMode )	//	ロード
	{
		StringCchCopy( atDefault, MAX_PATH, ptFile );
		GetPrivateProfileString( TEXT("General"), atKeyName, atDefault, ptFile, MAX_PATH, gatIniPath );
	}
	else
	{
		WritePrivateProfileString( TEXT("General"), atKeyName, ptFile, gatIniPath );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	最前面状態のセーブロード
	@param[in]	dMode	非０ロード　０セーブ
	@param[in]	dStyle	２頁一覧　３壱行　３ブラシ　４複数行
	@param[in]	nValue	ロード：デフォ値　セーブ：値
	@return		INT	ロード：値　セーブ：０
*/
INT InitWindowTopMost( UINT dMode, UINT dStyle, INT nValue )
{
	TCHAR	atAppName[MIN_STRING], atBuff[MIN_STRING];
	INT	iBuff = 0;

	switch( dStyle )
	{
		case  WDP_PLIST:	StringCchCopy( atAppName, SUB_STRING, TEXT("PageList") );	break;
		case  WDP_LNTMPL:	StringCchCopy( atAppName, SUB_STRING, TEXT("LineTmple") );	break;
		case  WDP_BRTMPL:	StringCchCopy( atAppName, SUB_STRING, TEXT("BrushTmple") );	break;
		case  WDP_MAATPL:	StringCchCopy( atAppName, SUB_STRING, TEXT("MaaTmple") );	break;
		default:	return 0;
	}

	StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), nValue );

	if( dMode )	//	ロード
	{
		GetPrivateProfileString( atAppName, TEXT("TopMost"), atBuff, atBuff, MIN_STRING, gatIniPath );
		iBuff = StrToInt( atBuff );
	}
	else	//	セーブ
	{
		WritePrivateProfileString( atAppName, TEXT("TopMost"), atBuff, gatIniPath );
	}

	return iBuff;
}
//-------------------------------------------------------------------------------------------------

/*!
	ウインドウ位置のセーブロード
	@param[in]	dMode	非０ロード　０セーブ
	@param[in]	dStyle	１ビュー　２頁一覧　３壱行　３ブラシ　４複数行
	@param[in]	pstRect	ロード結果を入れるか、セーブ内容を入れる
	@return		HRESULT	終了状態コード
*/
HRESULT InitWindowPos( UINT dMode, UINT dStyle, LPRECT pstRect )
{
	TCHAR	atAppName[MIN_STRING], atBuff[MIN_STRING];

	if( !pstRect )	return E_INVALIDARG;

	switch( dStyle )
	{
		case  WDP_MVIEW:	StringCchCopy( atAppName, SUB_STRING, TEXT("MainView") );	break;
		case  WDP_PLIST:	StringCchCopy( atAppName, SUB_STRING, TEXT("PageList") );	break;
		case  WDP_LNTMPL:	StringCchCopy( atAppName, SUB_STRING, TEXT("LineTmple") );	break;
		case  WDP_BRTMPL:	StringCchCopy( atAppName, SUB_STRING, TEXT("BrushTmple") );	break;
		case  WDP_MAATPL:	StringCchCopy( atAppName, SUB_STRING, TEXT("MaaTmple") );	break;
		case  WDP_PREVIEW:	StringCchCopy( atAppName, SUB_STRING, TEXT("Preview") );	break;
		default:	return E_INVALIDARG;
	}

	if( dMode )	//	ロード
	{
		GetPrivateProfileString( atAppName, TEXT("LEFT"), TEXT("0"), atBuff, MIN_STRING, gatIniPath );
		pstRect->left   = StrToInt( atBuff );
		GetPrivateProfileString( atAppName, TEXT("TOP"), TEXT("0"), atBuff, MIN_STRING, gatIniPath );
		pstRect->top    = StrToInt( atBuff );
		GetPrivateProfileString( atAppName, TEXT("RIGHT"), TEXT("0"), atBuff, MIN_STRING, gatIniPath );
		pstRect->right  = StrToInt( atBuff );
		GetPrivateProfileString( atAppName, TEXT("BOTTOM"), TEXT("0"), atBuff, MIN_STRING, gatIniPath );
		pstRect->bottom = StrToInt( atBuff );
	}
	else	//	セーブ
	{
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstRect->left );
		WritePrivateProfileString( atAppName, TEXT("LEFT"), atBuff, gatIniPath );
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstRect->top );
		WritePrivateProfileString( atAppName, TEXT("TOP"), atBuff, gatIniPath );
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstRect->right );
		WritePrivateProfileString( atAppName, TEXT("RIGHT"), atBuff, gatIniPath );
		StringCchPrintf( atBuff, MIN_STRING, TEXT("%d"), pstRect->bottom );
		WritePrivateProfileString( atAppName, TEXT("BOTTOM"), atBuff, gatIniPath );
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

#if 0
/*!
	最終使用ファイルのセーブロード
	@param[in]	dMode	非０ロード　０セーブ
	@param[out]	ptFile	フルパス・MAX_PATHであること
	@return	HRESULT	終了状態コード
*/
HRESULT InitLastOpen( UINT dMode, LPTSTR ptFile )
{
	if(  !(ptFile) )	return E_INVALIDARG;
	if( dMode ){	GetPrivateProfileString( TEXT("History"), TEXT("LastOpen"), TEXT(""), ptFile, MAX_PATH, gatIniPath );	}
	else{			WritePrivateProfileString( TEXT("History") , TEXT("LastOpen"), ptFile, gatIniPath );	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------
#endif

/*!
	枠情報のセーブロード
	@param[in]	nowWnd	今の窓番号
	@param[in]	iDir	ジャンプ方向・＋１か－１で・０は不可
	@return	HRESULT	終了状態コード
*/
HRESULT WindowFocusChange( INT nowWnd, INT iDir )
{
	INT	nextWnd;

	if( gbTmpltDock )	//	くっついたら二つだけ
	{
		if( WND_MAIN == nowWnd )	nextWnd = WND_MAAT;
		else						nextWnd = WND_MAIN;
	}
	else
	{
		nextWnd = nowWnd + iDir;
		if( 0 >= nextWnd )			nextWnd = WND_BRUSH;	//	WND_BRUSH
		if( WND_BRUSH < nextWnd )	nextWnd = WND_MAIN;
		//	末端に注意・多分もう増えないと思う
	}

	switch( nextWnd )
	{
		default:
		case WND_MAIN:	SetForegroundWindow( ghMainWnd );	break;
		case WND_MAAT:	SetForegroundWindow( ghMaaWnd );	break;
		case WND_PAGE:	SetForegroundWindow( ghPgVwWnd );	break;
		case WND_LINE:	SetForegroundWindow( ghLnTmplWnd );	break;
		case WND_BRUSH:	SetForegroundWindow( ghBrTmplWnd );	break;
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	プロフ履歴をINIから読んだり書いたり
	@param[in]		dMode	非０ロード　０セーブ
	@param[in]		dNumber	ロードセーブ番号
	@param[in,out]	ptFile	ロード：中身を入れる　セーブ：保存する文字列　MAX_PATHであること・NULLなら内容消去
	@return			HRESULT	終了状態コード
*/
HRESULT InitProfHistory( UINT dMode, UINT dNumber, LPTSTR ptFile )
{
	TCHAR	atKeyName[MIN_STRING], atDefault[MAX_PATH];

	if( dMode  )	//	ロード
	{
		ZeroMemory( ptFile, sizeof(TCHAR) * MAX_PATH );

		StringCchPrintf( atKeyName, MIN_STRING, TEXT("Hist%X"), dNumber );
		GetPrivateProfileString( TEXT("ProfHistory"), atKeyName, TEXT(""), atDefault, MAX_PATH, gatIniPath );

		if( NULL == atDefault[0] )	return E_NOTIMPL;	//	記録無し

		StringCchCopy( ptFile, MAX_PATH, atDefault );
	}
	else	//	セーブ
	{
		if( ptFile )
		{
			StringCchPrintf( atKeyName, MIN_STRING, TEXT("Hist%X"), dNumber );
			WritePrivateProfileString( TEXT("ProfHistory"), atKeyName, ptFile, gatIniPath );
		}
		else	//	一旦全削除
		{
			WritePrivateProfileSection( TEXT("ProfHistory"), NULL, gatIniPath );
		}
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------




/*!
	ファイルから履歴取り込んだり書き込んだり
	@param[in]	hWnd	ウインドウハンドル・NULLならデストロイ
	@return	HRESULT	終了状態コード
*/
HRESULT OpenHistoryInitialise( HWND hWnd )
{
	HMENU	hSubMenu;
	TCHAR	atKeyName[MIN_STRING], atString[MAX_PATH+10];
	UINT	d;
	UINT_PTR	dItems;
	OPENHIST	stOpenHist;
	OPHIS_ITR	itHist;

	if( hWnd )
	{
		gltOpenHist.clear( );	//	とりあえず全削除

		for( d = 0; OPENHIST_MAX > d; d++ )
		{
			ZeroMemory( &stOpenHist, sizeof(OPENHIST) );
			StringCchPrintf( atKeyName, MIN_STRING, TEXT("Hist%X"), d );
			GetPrivateProfileString( TEXT("OpenHistory"), atKeyName, TEXT(""), stOpenHist.atFile, MAX_PATH, gatIniPath );
			if( NULL == stOpenHist.atFile[0] )	break;	//	記録無くなったらそこで終了だよ

			gltOpenHist.push_back( stOpenHist );
		}

		if( ghHistyMenu )	DestroyMenu( ghHistyMenu );
		//	メニュー作成
		ghHistyMenu = CreatePopupMenu(  );
		AppendMenu( ghHistyMenu, MF_SEPARATOR, 0, NULL );	//	セッパレター
		AppendMenu( ghHistyMenu, MF_STRING, IDM_OPEN_HIS_CLEAR, TEXT("履歴クリヤ") );
		//文字列固定はあまりイクナイ

		dItems = gltOpenHist.size( );
		if( 0 == dItems )
		{
			//	オーポン履歴が無い場合
			InsertMenu( ghHistyMenu, 0, MF_STRING | MF_BYPOSITION | MF_GRAYED, IDM_OPEN_HIS_FIRST, TEXT("(无)") );
		}
		else
		{
			//	オーポン履歴を並べる
			for( itHist = gltOpenHist.begin(), d = dItems-1; gltOpenHist.end() != itHist; itHist++, d-- )
			{
				StringCchPrintf( atString, MAX_PATH+10, TEXT("(&%X) %s"), d, itHist->atFile );
				InsertMenu( ghHistyMenu, 0, MF_STRING | MF_BYPOSITION, (IDM_OPEN_HIS_FIRST + d), atString );
				itHist->dMenuNumber = (IDM_OPEN_HIS_FIRST + d);
			}
		}

#pragma message ("ファイルオーポン履歴の基点メニュー、位置に注意")
		//	メニュー情報の書換
		hSubMenu = GetSubMenu( ghMenu, 0 );
		ModifyMenu( hSubMenu, 2, MF_BYPOSITION | MF_POPUP, (UINT_PTR)ghHistyMenu, TEXT("ファイル使用履歴(&H)") );
		//文字列固定はあまりイクナイ

		DrawMenuBar( hWnd );	//	要らないかも？
	}
	else	//	APPZ終了時
	{
		if( ghHistyMenu )	DestroyMenu( ghHistyMenu );

		WritePrivateProfileSection( TEXT("OpenHistory"), NULL, gatIniPath );	//	一旦全削除

		//	中身を保存
		for( itHist = gltOpenHist.begin(), d = 0; gltOpenHist.end() != itHist; itHist++, d++ )
		{
			StringCchPrintf( atKeyName, MIN_STRING, TEXT("Hist%X"), d );
			WritePrivateProfileString( TEXT("OpenHistory"), atKeyName, itHist->atFile, gatIniPath );
		}

	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	開いた履歴を番号指定して読み込む
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	id		履歴指定メッセージ・メニューＩＤである
	@return	HRESULT	終了状態コード
*/
HRESULT OpenHistoryLoad( HWND hWnd, INT id )
{
	UINT_PTR	dNumber, dItems;
	LPARAM		lUnique;
	OPHIS_ITR	itHist;

	dNumber = id - IDM_OPEN_HIS_FIRST;

	TRACE( TEXT("履歴 -> %d"), dNumber );
	if( OPENHIST_MAX <= dNumber ){	return E_OUTOFMEMORY;	}

	dItems = gltOpenHist.size();
	dNumber = (dItems-1) - dNumber;

	itHist = gltOpenHist.begin();
	std::advance( itHist , dNumber );	//	個数分進める

	lUnique = DocOpendFileCheck( itHist->atFile );
	if( 1 <= lUnique )	//	既存のファイルヒット・そっちに移動する
	{
		if( SUCCEEDED( MultiFileTabSelect( dNumber ) ) )
		{
			DocMultiFileSelect( lUnique );	//	そのタブのファイルを表示
		}
	}
	else
	{
		DocDoOpenFile( hWnd, itHist->atFile );	//	履歴から選択したファイルを開く
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	開いたファイルを記録する
	@param[in]	hWnd	ウインドウハンドル
	@param[out]	ptFile	開いたファイルのれるバッファ・MAX_PATHであること・NULLなら履歴クルヤー
	@return	HRESULT	終了状態コード
*/
HRESULT OpenHistoryLogging( HWND hWnd, LPTSTR ptFile )
{
	UINT_PTR	dItems;
	OPENHIST	stOpenHist;
	OPHIS_ITR	itHist;

	if( ptFile )
	{
		ZeroMemory( &stOpenHist, sizeof(OPENHIST) );

		StringCchCopy( stOpenHist.atFile, MAX_PATH, ptFile );
		//既存の内容なら最新に入れ替えるので、検索しておく
		for( itHist = gltOpenHist.begin(); gltOpenHist.end() != itHist; itHist++ )
		{
			if( !StrCmp( itHist->atFile, stOpenHist.atFile ) )	//	同じものがあったら削除する
			{
				gltOpenHist.erase( itHist );
				break;
			}
		}

		gltOpenHist.push_back( stOpenHist );	//	リスト末尾ほど新しい

		//	もしはみ出すようなら古いのを削除する
		dItems = gltOpenHist.size( );
		if( OPENHIST_MAX <  dItems )
		{
			gltOpenHist.pop_front(  );
		}
	}
	else	//	文字列指定無い場合は全クリ
	{
		gltOpenHist.clear();
	}

	OpenHistoryInitialise( NULL );	//	古いの破壊して
	OpenHistoryInitialise( hWnd );	//	最新の内容で作り直し

	return S_OK;
}
//-------------------------------------------------------------------------------------------------



/*!
	おぷしょんダイヤログ開く
*/
HRESULT OptionDialogueOpen( VOID )
{
	UINT	bURHbuff, bABUIbuff;
	TCHAR	atFontName[LF_FACESIZE];	//	フォント名変更に備える
	//	項目増えたら、構造体にしちゃう

	ZeroMemory( atFontName, sizeof(atFontName) );

	bURHbuff  = gbUniRadixHex;
	bABUIbuff = gdBUInterval;
	//	改行コード変更ならバイト数再計算が必要か
	DialogBoxParam( ghInst, MAKEINTRESOURCE(IDD_GENERAL_OPTION_DLG), ghMainWnd, OptionDlgProc, (LPARAM)atFontName );

	//	設定変更されてたら内容入替
	if( bURHbuff != gbUniRadixHex ){	UnicodeRadixExchange( NULL );	};
	if( bABUIbuff != gdBUInterval )
	{
		if( 1 <= bABUIbuff )	KillTimer( ghMainWnd, IDT_BACKUP_TIMER );
		if( 1 <= gdBUInterval )	SetTimer(  ghMainWnd, IDT_BACKUP_TIMER, (gdBUInterval * 60000), NULL );
	}

	MenuItemCheckOnOff( IDM_UNICODE_TOGGLE , gbUniPad );	 //	ユニコードON/OFFのチェキ

	SqnSetting(   );	//	グループアンドゥのアレ

	if( NULL != atFontName[ 0] )	//	フォント名、なんか入ってたら変更されたと見なす
	{
		InitParamString( INIT_SAVE, VS_FONT_NAME, atFontName );
		//	記録する・書換はあとで
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	おぷしょんダイヤログのプロシージャ
	@param[in]	hDlg		ダイヤログハンドル
	@param[in]	message		ウインドウメッセージの識別番号
	@param[in]	wParam		追加の情報１
	@param[in]	lParam		追加の情報２
	@retval 0	メッセージは処理していない
	@retval no0	なんか処理された
*/
INT_PTR CALLBACK OptionDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static  UINT	cdGrXp, cdGrYp, cdRtRr;

	UINT	id;
	INT		dValue, iBuff;
	TCHAR	atBuff[SUB_STRING];
//	TCHAR	atPath[MAX_PATH];

	static LPTSTR	ptFontName;
	CHOOSEFONT	stChooseFont;
	LOGFONT		stLogFont;

	switch( message )
	{
		case WM_INITDIALOG:
			ptFontName = (LPTSTR)lParam;	//	フォント名入れるやつ

			//	スライダ初期値・小さいほど薄くなる
			SendDlgItemMessage( hDlg, IDSL_LAYERBOX_TRANCED, TBM_SETRANGE, TRUE, MAKELPARAM( 0, 0xE0 ) );	//	0xE0

			Edit_SetText( GetDlgItem(hDlg,IDE_AA_DIRECTORY), TEXT("ＡＡディレクトリはプロファイルから設定してね") );
			EnableWindow( GetDlgItem(hDlg,IDE_AA_DIRECTORY), FALSE );
			ShowWindow( GetDlgItem(hDlg,IDB_AADIR_SEARCH), SW_HIDE );

			//	MAAポップアップについて
			dValue = InitParamValue( INIT_LOAD, VL_MAATIP_SIZE, 16 );	//	サイズ確認
			if( FONTSZ_REDUCE == dValue )	CheckRadioButton( hDlg, IDRB_POPUP_NOMAL, IDRB_POPUP_REDUCE, IDRB_POPUP_REDUCE );
			else							CheckRadioButton( hDlg, IDRB_POPUP_NOMAL, IDRB_POPUP_REDUCE, IDRB_POPUP_NOMAL );
			dValue = InitParamValue( INIT_LOAD, VL_MAATIP_VIEW, 1 );	//	ポッパップするか
			CheckDlgButton( hDlg, IDCB_POPUP_VISIBLE, dValue ? BST_CHECKED : BST_UNCHECKED );

			//	右寄せドット位置
			dValue = InitParamValue( INIT_LOAD, VL_RIGHT_SLIDE, 790 );
			StringCchPrintf( atBuff, SUB_STRING, TEXT("%d"), dValue );
			Edit_SetText( GetDlgItem(hDlg,IDE_RIGHTSLIDE_DOT), atBuff );

			//	グリッド位置
			cdGrXp = gdGridXpos;
			StringCchPrintf( atBuff, SUB_STRING, TEXT("%d"), gdGridXpos );
			Edit_SetText( GetDlgItem(hDlg,IDE_GRID_X_POS), atBuff );

			cdGrYp = gdGridYpos;
			StringCchPrintf( atBuff, SUB_STRING, TEXT("%d"), gdGridYpos );
			Edit_SetText( GetDlgItem(hDlg,IDE_GRID_Y_POS), atBuff );

			//	右ルーラ位置
			cdRtRr = gdRightRuler;
			StringCchPrintf( atBuff, SUB_STRING, TEXT("%d"), gdRightRuler );
			Edit_SetText( GetDlgItem(hDlg,IDE_RIGHT_RULER_POS), atBuff );

			//	自動保存間隔
			dValue = InitParamValue( INIT_LOAD, VL_BACKUP_INTVL, 3 );
			StringCchPrintf( atBuff, SUB_STRING, TEXT("%d"), dValue );
			Edit_SetText( GetDlgItem(hDlg,IDE_AUTO_BU_INTVL), atBuff );

			//	自動保存メッセージ
			CheckDlgButton( hDlg, IDCB_AUTOBU_MSG_ON, gbAutoBUmsg ? BST_CHECKED : BST_UNCHECKED );

			//	改行コード選択
			CheckRadioButton( hDlg, IDRB_CRLF_STRB, IDRB_CRLF_2CH_YY, gbCrLfCode ? IDRB_CRLF_2CH_YY : IDRB_CRLF_STRB );

			//	空白ユニコードパディング
			CheckDlgButton( hDlg, IDCB_USE_UNISPACE_SET, gbUniPad ? BST_CHECKED : BST_UNCHECKED );

			//	ユニコード１６進数
			CheckDlgButton( hDlg, IDCB_UNIRADIX_HEX, gbUniRadixHex ? BST_CHECKED : BST_UNCHECKED );

			//	グループアンドゥ
			dValue = InitParamValue( INIT_LOAD, VL_GROUP_UNDO, 1 );
			CheckDlgButton( hDlg, IDCB_GROUPUNDO_SET, dValue ? BST_CHECKED : BST_UNCHECKED );

			EnableWindow( GetDlgItem(hDlg,IDCB_COPY_STYLE_SWAP), FALSE );

			//	ドッキングスタイル
			dValue = InitParamValue( INIT_LOAD, VL_PLS_LN_DOCK, 1 );
			CheckDlgButton( hDlg, IDCB_DOCKING_STYLE, dValue ? BST_CHECKED : BST_UNCHECKED );

			//	起動時オーポン
			dValue = InitParamValue( INIT_LOAD, VL_LAST_OPEN, LASTOPEN_DO );
			CheckRadioButton( hDlg, IDRB_LASTOPEN_DO, IDRB_LASTOPEN_ASK, (IDRB_LASTOPEN_DO + dValue) );

			//	レイヤボックスの透明度
			dValue = InitParamValue( INIT_LOAD, VL_LAYER_TRANS, 192 );
			SendDlgItemMessage( hDlg, IDSL_LAYERBOX_TRANCED, TBM_SETPOS, TRUE, (dValue - 0x1F) );

			//	複数行テンプレをクルックしたときの動作
			dValue = InitParamValue( INIT_LOAD, VL_MAA_LCLICK, MAA_INSERT );
			switch( dValue )
			{
				default:
				case MAA_INSERT:	id =  IDRB_SEL_INS_EDIT;	break;
				case MAA_INTERRUPT:	id =  IDRB_SEL_INTRPT_EDIT;	break;
				case MAA_LAYERED:	id =  IDRB_SEL_SET_LAYER;	break;
				case MAA_UNICLIP:	id =  IDRB_SEL_CLIP_UNI;	break;
				case MAA_SJISCLIP:	id =  IDRB_SEL_CLIP_SJIS;	break;
				case MAA_DRAUGHT:	id =  IDRB_SEL_DRAUGHT;		break;
			}
			CheckRadioButton( hDlg, IDRB_SEL_INS_EDIT, IDRB_SEL_DRAUGHT, id );

			//	複数行テンプレをミッドクルックしたときの動作
			dValue = InitParamValue( INIT_LOAD, VL_MAA_MCLICK, MAA_INSERT );
			switch( dValue )
			{
				default:
				case MAA_INSERT:	id =  IDRB_SELSUB_INS_EDIT;		break;
				case MAA_INTERRUPT:	id =  IDRB_SELSUB_INTRPT_EDIT;	break;
				case MAA_LAYERED:	id =  IDRB_SELSUB_SET_LAYER;	break;
				case MAA_UNICLIP:	id =  IDRB_SELSUB_CLIP_UNI;		break;
				case MAA_SJISCLIP:	id =  IDRB_SELSUB_CLIP_SJIS;	break;
				case MAA_DRAUGHT:	id =  IDRB_SELSUB_DRAUGHT;		break;
			}
			CheckRadioButton( hDlg, IDRB_SELSUB_INS_EDIT, IDRB_SELSUB_DRAUGHT, id );

			//	ドラフトボードでクリックしたときの動作
			switch( gdClickDrt )
			{
				case MAA_INSERT:	id =  IDRB_DRT_INS_EDIT;	break;
				case MAA_INTERRUPT:	id =  IDRB_DRT_INTRPT_EDIT;	break;
				case MAA_LAYERED:	id =  IDRB_DRT_SET_LAYER;	break;
				case MAA_UNICLIP:	id =  IDRB_DRT_CLIP_UNI;	break;
				case MAA_SJISCLIP:	id =  IDRB_DRT_CLIP_SJIS;	break;
			}
			CheckRadioButton( hDlg, IDRB_DRT_INS_EDIT, IDRB_DRT_CLIP_SJIS, id );

			switch( gdSubClickDrt )
			{
				case MAA_INSERT:	id =  IDRB_DRTSUB_INS_EDIT;		break;
				case MAA_INTERRUPT:	id =  IDRB_DRTSUB_INTRPT_EDIT;	break;
				case MAA_LAYERED:	id =  IDRB_DRTSUB_SET_LAYER;	break;
				case MAA_UNICLIP:	id =  IDRB_DRTSUB_CLIP_UNI;		break;
				case MAA_SJISCLIP:	id =  IDRB_DRTSUB_CLIP_SJIS;	break;
			}
			CheckRadioButton( hDlg, IDRB_DRTSUB_INS_EDIT, IDRB_DRTSUB_CLIP_SJIS, id );

			return (INT_PTR)TRUE;

		case WM_COMMAND:
			id = LOWORD(wParam);
			switch( id )
			{
				case IDB_CHOOSEFONT:	//	フォント変更のアレ
					ViewingFontGet( &stLogFont );
					ZeroMemory( &stChooseFont, sizeof(CHOOSEFONT) );
					stChooseFont.lStructSize = sizeof(CHOOSEFONT);
					stChooseFont.hwndOwner = hDlg;
					stChooseFont.lpLogFont = &stLogFont;
					stChooseFont.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
					if( ChooseFont(&stChooseFont) )	//	確定したのなら中身コピっておく
					{
						StringCchCopy( ptFontName, LF_FACESIZE, stLogFont.lfFaceName );
					}
					break;

				case IDB_APPLY://適用
				case IDOK:
					//	ツールチップの文字サイズ
					dValue = FONTSZ_NORMAL;
					if( IsDlgButtonChecked( hDlg, IDRB_POPUP_REDUCE ) ){	dValue =  FONTSZ_REDUCE;	}
					InitParamValue( INIT_SAVE, VL_MAATIP_SIZE, dValue );
#ifdef USE_HOVERTIP
					HoverTipSizeChange( dValue );	//	ポッパップ全体
#endif
					//	ＭＡＡで表示するかどうか・いらない？
					iBuff = IsDlgButtonChecked( hDlg, IDCB_POPUP_VISIBLE );
					AaItemsTipSizeChange( dValue, iBuff );
					InitParamValue( INIT_SAVE, VL_MAATIP_VIEW, iBuff );

					//	右寄せ基準ドット
					Edit_GetText( GetDlgItem(hDlg,IDE_RIGHTSLIDE_DOT), atBuff, SUB_STRING );
					dValue = StrToInt( atBuff );
					InitParamValue( INIT_SAVE, VL_RIGHT_SLIDE, dValue );

					//	グリッド位置
					Edit_GetText( GetDlgItem(hDlg,IDE_GRID_X_POS), atBuff, SUB_STRING );
					gdGridXpos = StrToInt( atBuff );
					InitParamValue( INIT_SAVE, VL_GRID_X_POS, gdGridXpos );
					if( cdGrXp != gdGridXpos )	ViewRedrawSetLine( -1 );

					Edit_GetText( GetDlgItem(hDlg,IDE_GRID_Y_POS), atBuff, SUB_STRING );
					gdGridYpos = StrToInt( atBuff );
					InitParamValue( INIT_SAVE, VL_GRID_Y_POS, gdGridYpos );
					if( cdGrYp != gdGridYpos )	ViewRedrawSetLine( -1 );

					//	右ルーラ位置
					Edit_GetText( GetDlgItem(hDlg,IDE_RIGHT_RULER_POS), atBuff, SUB_STRING );
					gdRightRuler = StrToInt( atBuff );
					InitParamValue( INIT_SAVE, VL_R_RULER_POS, gdRightRuler );
					if( cdRtRr != gdRightRuler )	ViewRedrawSetLine( -1 );

					//	自動保存間隔
					Edit_GetText( GetDlgItem(hDlg,IDE_AUTO_BU_INTVL), atBuff, SUB_STRING );
					gdBUInterval = StrToInt( atBuff );
					InitParamValue( INIT_SAVE, VL_BACKUP_INTVL, gdBUInterval );
					//	自動保存メッセージ
					dValue = IsDlgButtonChecked( hDlg, IDCB_AUTOBU_MSG_ON );
					gbAutoBUmsg = dValue ? 1 : 0;
					InitParamValue( INIT_SAVE, VL_BACKUP_MSGON, gbAutoBUmsg );

					//	改行コード選択
					gbCrLfCode = 0;
					if( IsDlgButtonChecked( hDlg , IDRB_CRLF_2CH_YY ) ){	gbCrLfCode =  1;	}
					InitParamValue( INIT_SAVE, VL_CRLF_CODE, gbCrLfCode );

					//	ユニコードスペース
					dValue = IsDlgButtonChecked( hDlg, IDCB_USE_UNISPACE_SET );
					gbUniPad = dValue ? 1 : 0;
					InitParamValue( INIT_SAVE, VL_USE_UNICODE, gbUniPad );

					//	ユニコード１６進数
					dValue = IsDlgButtonChecked( hDlg, IDCB_UNIRADIX_HEX );
					gbUniRadixHex = dValue ? 1 : 0;
					InitParamValue( INIT_SAVE, VL_UNIRADIX_HEX, gbUniRadixHex );

					//	グループアンドゥ
					dValue = IsDlgButtonChecked( hDlg, IDCB_GROUPUNDO_SET );
					InitParamValue( INIT_SAVE, VL_GROUP_UNDO, dValue ? 1 : 0 );

					//	ドッキングスタイル・変更しても、再起動するまでは無効
					dValue = IsDlgButtonChecked( hDlg, IDCB_DOCKING_STYLE );
					InitParamValue( INIT_SAVE, VL_PLS_LN_DOCK, dValue ? 1 : 0 );

					//	起動時オーポン
					if( IsDlgButtonChecked( hDlg, IDRB_LASTOPEN_NON ) ){	dValue = LASTOPEN_NON;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_LASTOPEN_ASK ) ){	dValue = LASTOPEN_ASK;	}
					else{	dValue = LASTOPEN_DO;	}	//	IDRB_LASTOPEN_DO
					InitParamValue( INIT_SAVE, VL_LAST_OPEN, dValue );

					//	透明度
					dValue = SendDlgItemMessage( hDlg, IDSL_LAYERBOX_TRANCED, TBM_GETPOS, 0, 0 );
					dValue += 0x1F;
					InitParamValue( INIT_SAVE, VL_LAYER_TRANS, dValue );
					LayerBoxAlphaSet( dValue );

					//	MAAの操作
					if( IsDlgButtonChecked( hDlg, IDRB_SEL_INTRPT_EDIT )  ){	dValue = MAA_INTERRUPT;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_SEL_SET_LAYER ) ){	dValue = MAA_LAYERED;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_SEL_CLIP_UNI ) ){	dValue = MAA_UNICLIP;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_SEL_CLIP_SJIS ) ){	dValue = MAA_SJISCLIP;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_SEL_DRAUGHT ) ){	dValue = MAA_DRAUGHT;	}
					else{	dValue = MAA_INSERT;	}	//	IDRB_SEL_INS_EDIT
					InitParamValue( INIT_SAVE, VL_MAA_LCLICK, dValue );

					if( IsDlgButtonChecked( hDlg, IDRB_SELSUB_INTRPT_EDIT )   ){	iBuff = MAA_INTERRUPT;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_SELSUB_SET_LAYER ) ){	iBuff = MAA_LAYERED;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_SELSUB_CLIP_UNI ) ){	iBuff = MAA_UNICLIP;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_SELSUB_CLIP_SJIS ) ){	iBuff = MAA_SJISCLIP;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_SELSUB_DRAUGHT )  ){	iBuff = MAA_DRAUGHT;	}
					else{	iBuff =  MAA_INSERT;	}	//	IDRB_SEL_INS_EDIT
					InitParamValue( INIT_SAVE, VL_MAA_MCLICK, iBuff );

					ViewMaaItemsModeSet( dValue, iBuff );	//	MAAにも設定おくる

					//	ドラフトボードの操作
					if( IsDlgButtonChecked( hDlg, IDRB_DRT_INTRPT_EDIT )  ){	gdClickDrt = MAA_INTERRUPT;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_DRT_SET_LAYER ) ){	gdClickDrt = MAA_LAYERED;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_DRT_CLIP_UNI ) ){	gdClickDrt = MAA_UNICLIP;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_DRT_CLIP_SJIS ) ){	gdClickDrt = MAA_SJISCLIP;	}
					else{	gdClickDrt = MAA_INSERT;	}	//	IDRB_DRT_INS_EDIT
					InitParamValue( INIT_SAVE, VL_DRT_LCLICK, gdClickDrt );

					if( IsDlgButtonChecked( hDlg, IDRB_DRTSUB_INTRPT_EDIT )   ){	gdSubClickDrt = MAA_INTERRUPT;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_DRTSUB_SET_LAYER ) ){	gdSubClickDrt = MAA_LAYERED;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_DRTSUB_CLIP_UNI ) ){	gdSubClickDrt = MAA_UNICLIP;	}
					else if( IsDlgButtonChecked( hDlg, IDRB_DRTSUB_CLIP_SJIS ) ){	gdSubClickDrt = MAA_SJISCLIP;	}
					else{	gdSubClickDrt = MAA_INSERT;	}	//	IDRB_DRTSUB_INS_EDIT
					InitParamValue( INIT_SAVE, VL_DRT_MCLICK, gdSubClickDrt );



					//	ＯＫなら閉じちゃう
					if( IDOK == id ){	EndDialog( hDlg, IDOK );	}
					return (INT_PTR)TRUE;

				case IDCANCEL:
					EndDialog( hDlg, IDCANCEL );
					return (INT_PTR)TRUE;

				default:	break;
			}
			break;

		default:	break;
	}

	return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------

/*!
	ユニコード空白の使用・不使用
*/
UINT UnicodeUseToggle( LPVOID pVoid )
{
	gbUniPad =  !(gbUniPad);	//	トグル
	InitParamValue( INIT_SAVE, VL_USE_UNICODE, gbUniPad );
	MenuItemCheckOnOff( IDM_UNICODE_TOGGLE, gbUniPad );

	return gbUniPad;
}
//-------------------------------------------------------------------------------------------------

/*!
	ディレクトリ選択ダイアログの表示
	@param[in]	hWnd		親ウインドウのハンドル
	@param[in]	ptSelDir	ディレクトリ名を入れるバッファへのポインター
	@param[in]	cchLen		バッファの文字数。バイト数じゃないぞ
	@return		非０：ディレクトリとった　０：キャンセルした
*/
BOOLEAN SelectDirectoryDlg( HWND hWnd, LPTSTR ptSelDir, UINT_PTR cchLen )
{
	BROWSEINFO		stBrowseInfo;
	LPITEMIDLIST	pstItemIDList;
	TCHAR	atDisplayName[MAX_PATH];

	if( !(ptSelDir) )	return FALSE;
	ZeroMemory( ptSelDir, sizeof(TCHAR) * cchLen );

	//	BROWSEINFO構造体に値を設定
	stBrowseInfo.hwndOwner		 = hWnd;	//	ダイアログの親ウインドウのハンドル
	stBrowseInfo.pidlRoot		 = NULL;	//	ルートディレクトリを示すITEMIDLISTのポインタ・NULLの場合デスクトップ
	stBrowseInfo.pszDisplayName	 = atDisplayName;	//	選択されたディレクトリ名を受け取るバッファのポインタ
	stBrowseInfo.lpszTitle		 = TEXT("ＡＡの入ってるディレクトリを選択するのー！");	//	ツリービューの上部に表示される文字列
	stBrowseInfo.ulFlags		 = BIF_RETURNONLYFSDIRS;	//	表示されるディレクトリの種類を示すフラグ
	stBrowseInfo.lpfn			 = NULL;		//	BrowseCallbackProc関数のポインタ
	stBrowseInfo.lParam			 = (LPARAM)0;	//	コールバック関数に渡す値

	//	ディレクトリ選択ダイアログを表示
	pstItemIDList = SHBrowseForFolder( &stBrowseInfo );
	if( !(pstItemIDList) )
	{
		//	戻り値がNULLの場合、ディレクトリが選択されずにダイアログが閉じられたということ
		return FALSE;
	}
	else
	{
		//	ItemIDListをパス名に変換
		if( !SHGetPathFromIDList( pstItemIDList, atDisplayName ) )
		{
			//	エラー処理
			return FALSE;
		}
		//	atDisplayNameに選択されたディレクトリのパスが入ってる
		StringCchCopy( ptSelDir, cchLen, atDisplayName );

		//	pstItemIDListを開放せしめる
		CoTaskMemFree( pstItemIDList );
	}

	return TRUE;
}
//-------------------------------------------------------------------------------------------------


/*!
	複数ファイルタブのサブクラスプロシージャ
	@param[in]	hWnd	リストのハンドル
	@param[in]	msg		ウインドウメッセージの識別番号
	@param[in]	wParam	追加の情報１
	@param[in]	lParam	追加の情報２
	@return	処理結果とか
*/
LRESULT	CALLBACK gpfFileTabProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		HANDLE_MSG( hWnd, WM_MBUTTONUP, Ftb_OnMButtonUp );

		default:	break;
	}

	return CallWindowProc( gpfOriginFileTabProc, hWnd, msg, wParam, lParam );
}
//-------------------------------------------------------------------------------------------------

/*!
	複数ファイルタブでマウスの中ボタンがうｐされたとき
	@param[in]	hWnd		ウインドウハンドル
	@param[in]	x			発生したクライヤントＸ座標値
	@param[in]	y			発生したクライヤントＹ座標値
	@param[in]	keyFlags	他に押されてるキーについて
*/
VOID Ftb_OnMButtonUp( HWND hWnd, INT x, INT y, UINT flags )
{
	INT	curSel;
	TCHITTESTINFO	stTcHitInfo;

	stTcHitInfo.pt.x = x;
	stTcHitInfo.pt.y = y;
	curSel = TabCtrl_HitTest( ghFileTabWnd, &stTcHitInfo );

	TRACE( TEXT("FTAB start TAB [%d] [%d x %d]"), curSel, x, y );

#ifdef MIDDLE_CLICK_CLOSE
	MultiFileTabClose( curSel );
#endif

	return;
}
//-------------------------------------------------------------------------------------------------

/*!
	起動時の最初の一個のファイルタブを追加
	@param[in]	ptName	ファイル名
	@return		HRESULT	終了状態コード
*/
HRESULT MultiFileTabFirst( LPTSTR ptName )
{
	TCHAR	atName[MAX_PATH];
	TCITEM		stTcItem;

	StringCchCopy( atName, MAX_PATH, ptName );
	PathStripPath( atName );

	ZeroMemory( &stTcItem, sizeof(TCITEM) );
	stTcItem.mask = TCIF_TEXT | TCIF_PARAM;
	stTcItem.pszText = atName;
	stTcItem.lParam  = 1;	//	ユニーク番号・常にINCREMENT
	TabCtrl_InsertItem( ghFileTabWnd, 1, &stTcItem );

	TabCtrl_DeleteItem( ghFileTabWnd, 0 );

	TabCtrl_SetCurSel( ghFileTabWnd, 0 );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	二つ目以降のファイルを追加
	@param[in]	dNumber	通し番号
	@param[in]	ptName	ファイル名
	@return		HRESULT	終了状態コード
*/
HRESULT MultiFileTabAppend( LPARAM dNumber, LPTSTR ptName )
{
	INT		iCount;
	TCHAR	atName[MAX_PATH];
	TCITEM		stTcItem;

	StringCchCopy( atName, MAX_PATH, ptName );
	PathStripPath( atName );

	iCount = TabCtrl_GetItemCount( ghFileTabWnd );

	ZeroMemory( &stTcItem, sizeof(TCITEM) );
	stTcItem.mask = TCIF_TEXT | TCIF_PARAM;
	stTcItem.pszText = atName;
	stTcItem.lParam  = dNumber;	//	ユニーク番号
	TabCtrl_InsertItem( ghFileTabWnd, iCount, &stTcItem );

	TabCtrl_SetCurSel( ghFileTabWnd, iCount );

//	ここでファイルオーポンを記録すると、関係無いのまで無差別に記録してまう

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定の通し番号をもったタブをさがしてタブインデックスを返す
	@param[in]	dNumber	通し番号
	@return		INT		タブインデックス・該当なかったら－１
*/
INT MultiFileTabSearch( LPARAM dNumber )
{
	INT		iCount, i;
	TCITEM	stTcItem;

	iCount = TabCtrl_GetItemCount( ghFileTabWnd );

	ZeroMemory( &stTcItem, sizeof(TCITEM) );
	stTcItem.mask = TCIF_PARAM;

	for( i = 0; iCount > i; i++ )
	{
		TabCtrl_GetItem( ghFileTabWnd, i, &stTcItem );

		if( dNumber == stTcItem.lParam )	return i;
	}

	return -1;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定の通し番号をもったタブを選択状態にする
	@param[in]	dNumber	通し番号
	@return		HRESULT	終了状態コード
*/
HRESULT MultiFileTabSelect( LPARAM dNumber )
{
	INT	iRslt;

	iRslt = MultiFileTabSearch( dNumber );	//	指定の通し番号をもったタブをさがす

	if( 0 <= iRslt )
	{
		TabCtrl_SetCurSel( ghFileTabWnd, iRslt );	//	該当のタブにフォーカス移して
		return S_OK;
	}

	return E_OUTOFMEMORY;
}
//-------------------------------------------------------------------------------------------------

/*!
	キーボードからのタブ移動操作・隣のタブに移動
	@param[in]	xDir	正数：右へ　負数：左へ　０なにもしない
	@return		HRESULT	終了状態コード
*/
HRESULT MultiFileTabSlide( INT xDir )
{
	INT		curSel, iCount, iTarget;
	LPARAM	dSele;
	TCITEM	stTcItem;

	if( 0 == xDir ){	return  S_FALSE;	}

	//	タブの総数
	iCount = TabCtrl_GetItemCount( ghFileTabWnd );

	//	選択してる奴を選択する
	curSel = TabCtrl_GetCurSel( ghFileTabWnd );

	//	移動先タブを検索
	if( 0 < xDir )
	{
		iTarget = curSel + 1;
		if( iCount <= iTarget ){	iTarget = 0;	}
	}
	else
	{
		iTarget = curSel - 1;
		if( 0 > iTarget ){	iTarget = iCount - 1;	}
	}

	//	そのタブを選択状態にして
	TabCtrl_SetCurSel( ghFileTabWnd, iTarget );

	//	そのタブのファイル番号を確認して
	ZeroMemory( &stTcItem, sizeof(TCITEM) );
	stTcItem.mask = TCIF_PARAM;
	TabCtrl_GetItem( ghFileTabWnd, iTarget, &stTcItem );
	dSele = stTcItem.lParam;

	//	該当のファイルをフォーカスする
	DocMultiFileSelect( dSele );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	指定の通し番号をもったタブの名称を変更
	@param[in]	dNumber	ファイルのUNIQUE番号
	@param[in]	ptName	ファイル名
	@return		HRESULT	終了状態コード
*/
HRESULT MultiFileTabRename( LPARAM dNumber, LPTSTR ptName )
{
	INT	iRslt;
	TCHAR	atName[MAX_PATH];
	TCITEM		stTcItem;

	iRslt = MultiFileTabSearch( dNumber );
	if( 0 > iRslt ){	return E_OUTOFMEMORY;	}

	StringCchCopy( atName, MAX_PATH, ptName );
	PathStripPath( atName );

	ZeroMemory( &stTcItem, sizeof(TCITEM) );
	stTcItem.mask = TCIF_TEXT;
	stTcItem.pszText = atName;
	TabCtrl_SetItem( ghFileTabWnd, iRslt, &stTcItem );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	開いてるタブを保存したりよびだしたり
	@param[in]	dMode	非０ロード　０セーブ
	@param[in]	iTgt	読み出す番号・０インデックス・負数ならファイル数のみ確保
	@param[out]	ptFile	フルパス・MAX_PATHであること
	@return		INT	セーブ：０　ロード：記録数
*/
INT InitMultiFileTabOpen( UINT dMode, INT iTgt, LPTSTR ptFile )
{
	if( dMode )	//	よびだし
	{
		return DocMultiFileFetch( iTgt, ptFile, gatIniPath );
	}
	else	//	セーブ
	{
		DocMultiFileStore( gatIniPath );
	}

	return 0;
}
//-------------------------------------------------------------------------------------------------

/*!
	開いてるタブを閉じるCommando
	@param[in]	iSelTab	閉じたいタブ番号・デフォ動作なら－１
	@return		HRESULT	終了状態コード
*/
HRESULT MultiFileTabClose( INT iSelTab )
{
	INT	curSel, nowSel;
	LPARAM	dSele;
	TCITEM	stTcItem;

	nowSel = TabCtrl_GetCurSel( ghFileTabWnd );

	if( 0 > iSelTab ){	curSel = nowSel;	}	//	選択してる奴を削除する
	else{				curSel = iSelTab;	}	//	指定のタブを削除する

	ZeroMemory( &stTcItem, sizeof(TCITEM) );
	stTcItem.mask = TCIF_PARAM;
	TabCtrl_GetItem( ghFileTabWnd, curSel, &stTcItem );

	dSele = DocMultiFileClose( ghMainWnd, stTcItem.lParam );
	if( dSele )	//	値は、タブ番号ではなくファイルの通し番号であることに注意
	{
		TabCtrl_DeleteItem( ghFileTabWnd, curSel );
		//	開いてるのを消したら、隣のに開き直す
		if( curSel ==  nowSel ){	MultiFileTabSelect( dSele );	}
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	表示用フォントの名前を頂く
*/
HRESULT ViewingFontNameLoad( VOID )
{
	TCHAR	atName[LF_FACESIZE];

	ZeroMemory( atName, sizeof(atName) );	//	デフォネーム
	StringCchCopy( atName, LF_FACESIZE, TEXT("ＭＳ Ｐゴシック") );

	InitParamString( INIT_LOAD, VS_FONT_NAME, atName );	//	ゲッツ！

	StringCchCopy( gstBaseFont.lfFaceName, LF_FACESIZE, atName );

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	表示用フォントデータをコピーする
	@param[in]	pstLogFont	データコピる構造体へのポインター
*/
HRESULT ViewingFontGet( LPLOGFONT pstLogFont )
{
	ZeroMemory( pstLogFont, sizeof(LOGFONT) );	//	念のため空白にする

	*pstLogFont = gstBaseFont;
	//	構造体はコピーでおｋ
	return S_OK;
}
//-------------------------------------------------------------------------------------------------





#ifdef USE_NOTIFYICON

/*!
	タスクトレイアイコンでイベントが発生したら
	@param[in]	hWnd	ウインドウハンドル
	@param[in]	uID		イベントが発生したトレイアイコンのＩＤ
	@param[in]	message	マウス、キーボードの操作のWMmessage
	@return 特になし
*/
VOID TaskTrayIconEvent( HWND hWnd, UINT uID, UINT message )
{


	return;
}
//-------------------------------------------------------------------------------------------------
#endif

/*!
	タスクトレイアイコンにばりゅ～んめせーじを載せる
	@param[in]	ptInfo	バルーンの本文
	@param[in]	ptTitle	バルーンのタイトル
	@param[in]	dIconTy	くっつくアイコン、１情報、２警告、３エラー
	@return		HRESULT	終了状態コード
*/
HRESULT NotifyBalloonExist( LPTSTR ptInfo, LPTSTR ptTitle, DWORD dIconTy )
{
#ifdef USE_NOTIFYICON
	NOTIFYICONDATA	nid;

	ZeroMemory( &nid, sizeof(NOTIFYICONDATA) );
	nid.cbSize      = sizeof(NOTIFYICONDATA);
	nid.uFlags      = NIF_INFO;
	nid.hWnd        = ghMainWnd;
	StringCchCopy( nid.szInfoTitle, 64, ptTitle );
	StringCchCopy( nid.szInfo, 256, ptInfo );
	nid.uTimeout    = 15000;	//	ｍｓで１５秒表示させてみる
	nid.dwInfoFlags = dIconTy;	//	用いるアイコンタイプ
	Shell_NotifyIcon( NIM_MODIFY, &nid );

//	NIIF_INFO       0x00000001
//	NIIF_WARNING    0x00000002
//	NIIF_ERROR      0x00000003
#endif
	return S_OK;
}
//-------------------------------------------------------------------------------------------------

/*!
	ドッキングしてる壱行ブラシテンプレを表示/非表示
*/
HRESULT DockingTmplViewToggle( UINT bMode )
{
	HWND	hDockWnd;
	RECT	clntRect, tabRect, pageRect, sbRect;
	RECT	rect;
	INT		iHeight, curSel;

	hDockWnd = DockingTabGet( );
	//	分離状態ならタブは無い
	if( !(hDockWnd) )	return E_ABORT;

	GetClientRect( ghMainWnd, &clntRect );
	GetClientRect( hDockWnd,  &tabRect );
	GetClientRect( ghPgVwWnd, &pageRect );

	GetClientRect( ghStsBarWnd, &sbRect );	//	ステータスバーのサイズ確認

	AppClientAreaCalc( &rect );

	if( gbDockTmplView )	//	見えてるなら閉じればいい
	{
		gbDockTmplView = FALSE;
		ShowWindow( ghLnTmplWnd, SW_HIDE );
		ShowWindow( ghBrTmplWnd, SW_HIDE );

		//	場所を変えないといかん
		iHeight = clntRect.bottom - tabRect.bottom - sbRect.bottom;

		//	壱行ブラシタブは下へ
		SetWindowPos( hDockWnd, HWND_TOP, rect.right - tabRect.right, iHeight, 0, 0, SWP_NOSIZE | SWP_NOZORDER );

		//	Pageリストは広げる
		SetWindowPos( ghPgVwWnd, HWND_TOP, 0, 0, pageRect.right, iHeight - rect.top, SWP_NOMOVE | SWP_NOZORDER );
	}
	else
	{
		gbDockTmplView = TRUE;

		//	位置合わせ
		ViewSizeMove( ghMainWnd, &rect );

		//	今の状態に合わせて復帰
		curSel = TabCtrl_GetCurSel( hDockWnd );
		switch( curSel )
		{
			case  0:	ShowWindow( ghLnTmplWnd , SW_SHOW );	break;
			case  1:	ShowWindow( ghBrTmplWnd , SW_SHOW );	break;
			default:	break;
		}

	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------------

#if defined(_DEBUG) || defined(WORK_LOG_OUT)
VOID OutputDebugStringPlus( DWORD rixError, LPSTR pcFile, INT rdLine, LPSTR pcFunc, LPTSTR ptFormat, ... )
{
	va_list	argp;
	TCHAR	atBuf[MAX_PATH], atOut[MAX_PATH], atFiFu[MAX_PATH], atErrMsg[MAX_PATH];
	CHAR	acFile[MAX_PATH], acFiFu[MAX_PATH];
	UINT	length;
#ifdef WORK_LOG_OUT
	UINT_PTR	cchLen;
	DWORD	wrote;
#endif

	StringCchCopyA( acFile, MAX_PATH, pcFile );
	PathStripPathA( acFile );

	StringCchPrintfA( acFiFu, MAX_PATH, ("%s %d %s"), acFile, rdLine, pcFunc );
	length = (UINT)strlen( acFiFu );

	ZeroMemory( atFiFu, sizeof(atFiFu) );
	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, acFiFu, length, atFiFu, MAX_PATH );
	//	コードページ,文字の種類を指定するフラグ,マップ元文字列のアドレス,マップ元文字列のバイト数,
	//	マップ先ワイド文字列を入れるバッファのアドレス,バッファのサイズ

	va_start(argp, ptFormat);
	StringCchVPrintf( atBuf, MAX_PATH, ptFormat, argp );
	va_end( argp );

	StringCchPrintf( atOut, MAX_PATH, TEXT("%s @ %s\r\n"), atBuf, atFiFu );//

#ifdef _DEBUG
	OutputDebugString( atOut );
#endif

#ifdef WORK_LOG_OUT
	if( INVALID_HANDLE_VALUE != ghLogFlie )
	{
		StringCchLength( atOut, MAX_PATH, &cchLen );
		WriteFile( ghLogFlie, atOut, cchLen * sizeof(TCHAR), &wrote, NULL );
	}
#endif

	if( rixError )
	{
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, rixError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), atErrMsg, MAX_PATH, NULL );
		//	メッセージには改行が含まれているようだ
		StringCchPrintf( atBuf, MAX_PATH, TEXT("[%d]%s"), rixError, atErrMsg );//

#ifdef _DEBUG
		OutputDebugString( atBuf );
#endif
#ifdef WORK_LOG_OUT
		if( INVALID_HANDLE_VALUE != ghLogFlie )
		{
			StringCchLength( atBuf, MAX_PATH, &cchLen );
			WriteFile( ghLogFlie, atBuf, cchLen * sizeof(TCHAR), &wrote, NULL );
		}
#endif

		SetLastError( 0 );
	}
}
//-------------------------------------------------------------------------------------------------
#endif

//	Dirty Deeds Done Dirt Cheap 自分のスタンドに「いとも容易く行われるえげつない行為」なんて名前を付けるのはどうかと思う。
