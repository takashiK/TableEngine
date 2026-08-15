# Platform Abstraction Layer

## Overview

`src/platform/` は OS 依存の処理を抽象化するアダプタ層です。  
上位レイヤー（コマンド / ウィジェット）は `platform_util.h` で宣言された関数のみを呼び出し、  
OS の違いを意識せずにファイル操作・サムネイル取得・ネイティブイベント等を利用できます。

現在は **Windows** と **Linux (Ubuntu)** 向けの実装が提供されています。
Linux 実装ではデスクトップ環境固有のシェル連携（右クリックコンテキストメニュー / サムネイル / プロパティダイアログ / `IFileOperation` 相当のコピー・移動）は未実装で、安全に no-op（何もしない）またはデフォルト値を返します。
macOS 対応を追加する際は `platform/` 配下に OS 別の実装ファイルを追加し、
`CMakeLists.txt` の条件コンパイル（`if(WIN32)` / `elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")` 等）で切り替えます。

---

## File Structure

```
platform/
├── platform_util.h            # プラットフォーム中立な関数宣言（上位レイヤーが参照する唯一のヘッダ）
├── TeNativeEvent.h/cpp        # ネイティブイベント通知の抽象クラス
├── TeFileOperationManager.h/cpp  # 非同期シェルファイル操作マネージャ（ワーカースレッド）
├── windows/
│   ├── platform_util.cpp      # Windows 向け platform_util.h の実装
│   └── TeWindowsEventFilter.h/cpp  # Windows ネイティブイベントフィルタ
└── linux/
    └── platform_util.cpp      # Linux (Ubuntu) 向け platform_util.h の実装
```

---

## API Reference

`platform_util.h` で宣言されているすべての関数を以下に示します。  
実装は `platform/windows/platform_util.cpp`（Windows）と `platform/linux/platform_util.cpp`（Linux）にあります。

### Initialization

| 関数 | 説明 |
|---|---|
| `threadInitialize(app)` | プラットフォーム固有の初期化処理（Windows: COM 初期化等）を行う |
| `threadUninitialize()` | プラットフォーム固有の終了処理を行う |

### File Context Menu

| 関数 | 説明 |
|---|---|
| `showFilesContext(x, y, paths)` | 複数ファイルの OS ネイティブ右クリックコンテキストメニューを表示する |
| `showFileContext(x, y, path)` | 単一ファイルの OS ネイティブ右クリックコンテキストメニューを表示する |

### File Properties

| 関数 | 説明 |
|---|---|
| `showFilesProperties(paths)` | 複数ファイルの OS プロパティダイアログを表示する |
| `showFileProperties(path)` | 単一ファイルの OS プロパティダイアログを表示する |
| `openFile(path)` | ファイルを関連付けられたアプリで開く（`ShellExecute` 相当） |

### File Operations

| 関数 | 説明 |
|---|---|
| `copyFiles(files, path)` | 複数ファイルを指定フォルダへコピーする |
| `copyFile(fromFile, toFile)` | 単一ファイルをコピーする |
| `moveFiles(files, path)` | 複数ファイルを指定フォルダへ移動する |
| `deleteFiles(files)` | 複数ファイルを削除する（OS のゴミ箱への移動含む） |

### Thumbnails & Icons

| 関数 | 説明 |
|---|---|
| `getThumbnail(path, size)` | ファイルのサムネイル画像を取得する（OS のサムネイルキャッシュを利用） |
| `getFileIcon(path, size)` | ファイルに関連付けられたアイコンを取得する |

### Clipboard

| 関数 | 説明 |
|---|---|
| `isMoveAction(mime)` | クリップボードの MIME データが「移動」操作かどうかを判定する |
| `setMoveAction(mime)` | MIME データに「移動」フラグを設定する |
| `setCopyAction(mime)` | MIME データに「コピー」フラグを設定する |

### Native Event

| 関数 | 説明 |
|---|---|
| `getNativeEvent()` | `TeNativeEvent` インスタンスを返す（ドライブのマウント状態変化の通知に使用） |

---

## TeNativeEvent

OS レベルのイベント（ドライブの接続 / 切断等）を `Qt` のシグナルとして再発行する抽象クラスです。

| シグナル | タイミング |
|---|---|
| `mountStateChanged(bool state)` | ドライブのマウント状態が変化したとき |

`TeDriveBar` がこのシグナルを受信してドライブボタンの表示を更新します。

### Windows 実装: TeWindowsEventFilter

`QAbstractNativeEventFilter` を継承し、Windows の `WM_DEVICECHANGE` メッセージを検知して  
`TeNativeEvent::changeMountState()` を呼び出します。  
`threadInitialize()` で `QApplication` にインストールされます。

### Linux 実装

ドライブのマウント状態変化を検知するネイティブイベントフィルタは未実装です。
`getNativeEvent()` は共有の `TeNativeEvent` インスタンスを返しますが、`mountStateChanged` シグナルは発行されません。

---

## Colour Scheme Abstraction

ライト / ダークテーマの判定は Qt のバージョン差異を吸収する `TeStyleColorScheme` で抽象化されています。

```cpp
enum class TeStyleColorScheme {
	Light,
	Dark
};

extern TeStyleColorScheme getStyleColorScheme();
```

| プラットフォーム | 実装 |
|---|---|
| Windows | `QGuiApplication::styleHints()->colorScheme()`（Qt 6.5+ が前提）を判定 |
| Linux | `QT_VERSION >= 6.5.0` の場合のみ `QStyleHints::colorScheme()` を判定し、それ以外や検出失敗時は `TeStyleColorScheme::Light` を返す |

Qt 6.5 未満（Ubuntu 24.04 の apt パッケージは Qt 6.4）でもヘッダがコンパイルできるよう、`Qt::ColorScheme` に直接依存せず独自の enum を経由しています。

利用側:

- `TeViewStore::applyStyleSheet(TeStyleColorScheme scheme)` — スキーム変化時にスタイルシートを再適用する（詳細: [05_viewstore.md](05_viewstore.md)）
- `TeAdaptiveIconEngine` — ダークモード時にアイコンを重彩色する（詳細: [10_utils.md](10_utils.md)）

---

## TeFileOperationManager

コピー / 移動 / 削除といったシェルファイル操作を専用のワーカースレッド上で実行し、  
操作中もメインウィンドウの応答性を保つためのマネージャです（モードレス動作）。

- 操作はワーカースレッド（`TeFileOpWorker`）のイベントキューで**逐次**処理され、  
  新しい操作を投入しても GUI スレッドやコマンドディスパッチャのキューをブロックしません。
- `TeFileOpWorker` はキュー接続経由で呼び出される `doCopyFiles` / `doCopyFile` / `doMoveFiles` / `doDeleteFiles` を持ち、  
  完了時にワーカースレッド上で `operationFinished(id, success)` シグナルを発行します。
- オーナーウィンドウのハンドルはキュー接続の引数型を登録可能に保つため整数（`quint64`）で受け渡します。

---

## Adding a New Platform

新しいプラットフォーム（例: macOS）を追加する手順：

1. `platform/<os>/platform_util.cpp` を作成し、`platform_util.h` の全関数を実装する
2. 必要であれば `platform/<os>/Te<OS>EventFilter.h/cpp` を作成し、`QAbstractNativeEventFilter` を継承してネイティブイベントを処理する
3. `src/CMakeLists.txt` に条件コンパイルブロックを追加する（Windows / Linux 向けの既存ブロックを参照）

```cmake
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    file(GLOB_RECURSE SOURCES_MACOS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "platform/macos/*.cpp")
    list(APPEND SOURCES_PLATFORM ${SOURCES_MACOS})
endif()
```
