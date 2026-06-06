# Folder Structure

プロジェクトのソースツリーと各フォルダの責務を説明します。

## Repository Root

```
TableEngine/
├── main/              # エントリポイント（main() 関数）
├── src/               # TableEngine 本体ライブラリ（tablelibs）
├── tests/             # ユニットテスト（gtest/gmock）
├── viewer_tests/      # Viewer モジュール向けテスト
├── support/           # 外部サポートライブラリ（git submodule / vcpkg）
│   ├── vcpkg/         #   vcpkg パッケージマネージャ（git submodule）
│   ├── QHexView/      #   QHexView ウィジェット（git submodule, v5.1.0）
│   └── valijson/      #   valijson ヘッダオンリーライブラリ（git submodule, v1.1.0）
├── doc/               # ドキュメント
│   ├── doxygen/       # Doxygen 設定・画像
│   ├── markdown/      # 設計ドキュメント（開発者向け）
│   └── manual/        # ユーザー向けヘルプガイド
├── scripts/           # ユーティリティスクリプト
│   └── package-release.ps1  # リリース ZIP パッケージ生成スクリプト
├── CMakeLists.txt     # CMake ルート設定
├── CMakePresets.json  # CMake プリセット（Ninja Multi-Config / MSVC2022）
├── vcpkg.json         # vcpkg マニフェスト（libarchive / gtest）
└── TableEngine.pro    # qmake プロジェクトファイル（非サポート・参照用）
```

## src/ — Main Library

`src/` 以下は `tablelibs` という静的ライブラリとしてビルドされます。  
`main/` の実行ファイルがこのライブラリをリンクする構成です。

```
src/
├── *.h / *.cpp              # コアクラス群（TeViewStore / TeDispatcher 等）
├── commands/                # コマンドパターンの実装
│   ├── edit/                # 編集系コマンド
│   ├── file/                # ファイル操作系コマンド
│   ├── folder/              # フォルダナビゲーション系コマンド
│   ├── help/                # ヘルプ系コマンド
│   ├── navi/                # ナビゲーション補助コマンド
│   ├── setting/             # 設定系コマンド
│   ├── tool/                # ツール系コマンド
│   ├── user/                # ユーザー定義系コマンド
│   ├── view/                # 表示設定コマンド
│   ├── window/              # ウィンドウ操作コマンド
│   ├── TeCommandBase.h/cpp
│   ├── TeCommandFactory.h/cpp
│   └── TeCommandInfo.h/cpp
├── dialogs/                 # 各種ダイアログウィジェット
├── platform/                # OS 依存処理の抽象化レイヤー
│   ├── platform_util.h      # プラットフォーム中立 API 宣言
│   ├── TeNativeEvent.h/cpp  # ネイティブイベントフィルタ
│   └── windows/             # Windows 固有実装
├── utils/                   # ファイル操作・検索・履歴等のユーティリティ
├── viewer/                  # ファイルビューワ
│   ├── binary/              # バイナリビューワ
│   ├── document/            # テキスト / マークアップビューワ
│   │   ├── markup/
│   │   ├── schema/
│   │   └── text/
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
| `TeKeyMap.h/cpp` | キー入力とコマンド ID の対応管理 |

## support/ — 外部依存ライブラリ

git submodule と vcpkg を用いた依存管理の拠点です。ソースリポジトリにはサブモジュール参照のみが含まれます。

```
support/
├── vcpkg/       # vcpkg 本体（git submodule）
│                #   vcpkg.json により libarchive / gtest を
│                #   x64-windows-static-md トリプレットで自動ビルド
├── QHexView/    # QHexView v5.1.0（git submodule）
│                #   バイナリビューワで使用するヘックスビューウィジェット
└── valijson/    # valijson v1.1.0（git submodule）
                 #   JSON スキーマ検証（ヘッダオンリー）
```

vcpkg のビルド成果物は `out/build/<preset>/vcpkg_installed/` に出力されます（`.gitignore` で除外済み）。
