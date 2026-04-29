# Platform Abstraction Layer

## Overview

`src/platform/` は OS 依存の処理を抽象化するアダプタ層です。  
上位レイヤー（コマンド / ウィジェット）は `platform_util.h` で宣言された関数のみを呼び出し、  
OS の違いを意識せずにファイル操作・サムネイル取得・ネイティブイベント等を利用できます。

現在は **Windows** 向けの実装のみが提供されています。  
Linux / macOS 対応を追加する際は `platform/` 配下に OS 別の実装ファイルを追加し、  
`CMakeLists.txt` の条件コンパイル（`if(WIN32)` 等）で切り替えます。

---

## File Structure

```
platform/
├── platform_util.h            # プラットフォーム中立な関数宣言（上位レイヤーが参照する唯一のヘッダ）
├── TeNativeEvent.h/cpp        # ネイティブイベント通知の抽象クラス
└── windows/
    ├── platform_util.cpp      # Windows 向け platform_util.h の実装
    └── TeWindowsEventFilter.h/cpp  # Windows ネイティブイベントフィルタ
```

---

## API Reference

`platform_util.h` で宣言されているすべての関数を以下に示します。  
実装は `platform/windows/platform_util.cpp` にあります。

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

---

## Adding a New Platform

新しいプラットフォームを追加する手順：

1. `platform/<os>/platform_util.cpp` を作成し、`platform_util.h` の全関数を実装する
2. `platform/<os>/Te<OS>EventFilter.h/cpp` を作成し、`QAbstractNativeEventFilter` を継承してネイティブイベントを処理する
3. `src/CMakeLists.txt` に条件コンパイルブロックを追加する

```cmake
elseif(UNIX)
    file(GLOB_RECURSE SOURCES_UNIX RELATIVE ... "platform/unix/*.cpp")
    list(APPEND SOURCES_PLATFORM ${SOURCES_UNIX})
endif()
```
