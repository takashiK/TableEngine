# Folder Structure

プロジェクトのソースツリーと各フォルダの責務を説明します。

## Repository Root

```
TableEngine/
├── main/              # エントリポイント（main() 関数）
├── src/               # TableEngine 本体ライブラリ（tablelibs）
├── tests/             # ユニットテスト
├── viewer_tests/      # Viewer モジュール向けユニットテスト
├── support_package/   # 外部サポートライブラリ（ヘッダ・バイナリ）
├── doc/               # ドキュメント
│   ├── doxygen/       # Doxygen 設定・画像
│   └── markdown/      # 設計ドキュメント（本ドキュメント群）
├── CMakeLists.txt     # CMake ルート設定
├── CMakePresets.json  # CMake プリセット（MSVC 2022 amd64）
└── TableEngine.pro    # qmake プロジェクトファイル（レガシー）
```

## src/ — Main Library

`src/` 以下は `tablelibs` という静的ライブラリとしてビルドされます。  
`main/` の実行ファイルがこのライブラリをリンクする構成です。

```
src/
├── *.h / *.cpp              # コアクラス群（TeViewStore / TeDispatcher 等）
├── commands/                # コマンドパターンの実装
│   ├── edit/                # 編集系コマンド（コピー / 切り取り / 貼り付け / 選択）
│   ├── file/                # ファイル操作系コマンド（作成 / 削除 / リネーム / アーカイブ等）
│   ├── folder/              # フォルダナビゲーション系コマンド（移動 / 履歴 / お気に入り等）
│   ├── setting/             # 設定系コマンド（キー設定 / メニュー設定 / オプション）
│   ├── tool/                # ツール系コマンド（ビューワ起動）
│   ├── view/                # 表示設定コマンド（表示順 / フィルタ / レイアウト）
│   ├── window/              # ウィンドウ操作コマンド（タブ操作 / バー表示切替）
│   ├── TeCommandBase.h/cpp  # コマンド抽象基底クラス
│   ├── TeCommandFactory.h/cpp # コマンドファクトリ（シングルトン）
│   └── TeCommandInfo.h/cpp  # コマンドメタ情報
├── dialogs/                 # 各種ダイアログウィジェット
├── platform/                # OS 依存処理の抽象化レイヤー
│   ├── platform_util.h      # プラットフォーム中立な関数宣言
│   ├── TeNativeEvent.h/cpp  # ネイティブイベントフィルタ
│   └── windows/             # Windows 固有実装
├── utils/                   # ファイル操作・検索・履歴等のユーティリティ
├── viewer/                  # ファイルビューワ
│   ├── binary/              # バイナリビューワ
│   ├── document/            # テキスト / マークアップビューワ
│   │   ├── markup/          # マークアップ形式とコンテナの対応管理
│   │   ├── schema/          # ドキュメントスキーマ定義
│   │   └── text/            # テキスト表示・シンタックスハイライト
│   └── picture/             # 画像ビューワ
├── widgets/                 # UI ウィジェット群
└── template/                # コード生成テンプレート等
```

## src/ 直下のコアクラス

`src/` 直下に配置されたクラスは、アプリケーション全体を貫く中核メカニズムを担います。  
個別の UI モジュールやコマンドモジュールから参照される基盤クラスです。

| ファイル | 責務 |
|---|---|
| `TeTypes.h/cpp` | アプリケーション共通の型定義（`WidgetType` / `CmdId` / `FileInfo` / `FileType` 等） |
| `TeViewStore.h/cpp` | UI 全体の構成管理・状態保持（詳細: [05_viewstore.md](05_viewstore.md)） |
| `TeDispatcher.h/cpp` | イベントをコマンドに変換して実行するディスパッチャ（詳細: [06_dispatcher_command.md](06_dispatcher_command.md)） |
| `TeDispatchable.h` | `dispatch()` / `execCommand()` インタフェース（`TeDispatcher` / `TeFolderView` が実装） |
| `TeEventEmitter.h/cpp` | 特定イベントを監視してシグナルとして再発行するイベントフィルタ（詳細: [07_event_emitter.md](07_event_emitter.md)） |
| `TeSettings.h/cpp` | アプリケーション設定の読み書きユーティリティ（QSettings のラッパー） |

## support_package/

ビルド済みの外部ライブラリを格納します。ソースリポジトリには含まれており、別途ビルドは不要です。

```
support_package/
├── include/   # 外部ライブラリのヘッダファイル
├── debug/     # デバッグビルド用ライブラリ（.lib / .dll）
├── release/   # リリースビルド用ライブラリ（.lib / .dll）
└── license/   # 外部ライブラリのライセンス情報
```
