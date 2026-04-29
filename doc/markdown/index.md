# TableEngine Design Documentation

TableEngine の設計情報をまとめたドキュメント一覧です。  
各ドキュメントはモジュール/レイヤー単位の設計概要を記載しています。  
特定クラスの詳細設計は、各サブディレクトリに格納されています。

---

## Overview & Project Setup

| ドキュメント | 内容 |
|---|---|
| [01_overview.md](01_overview.md) | TableEngine の目的・背景・ライセンス |
| [02_folder_structure.md](02_folder_structure.md) | ソースツリーの構成と各フォルダの責務 |
| [03_build.md](03_build.md) | ビルド手順（CMake / qmake） |

## Architecture & Core Mechanisms

| ドキュメント | 内容 |
|---|---|
| [04_architecture.md](04_architecture.md) | アーキテクチャ全体・設計方針・コンポーネント関連図 |
| [05_viewstore.md](05_viewstore.md) | ViewStore による UI 構成管理（`TeViewStore` / `TeSettings` / `TeTypes`） |
| [06_dispatcher_command.md](06_dispatcher_command.md) | Dispatcher / Command パターン（`TeDispatcher` / `TeCommandBase` / `TeCommandFactory`） |
| [07_event_emitter.md](07_event_emitter.md) | EventEmitter（`TeEventEmitter`） |

## Modules

| ドキュメント | 内容 |
|---|---|
| [08_widgets.md](08_widgets.md) | Widgets モジュール概要 |
| [09_commands.md](09_commands.md) | Commands モジュール概要（CmdId 分類・一覧） |
| [10_utils.md](10_utils.md) | Utils モジュール概要 |
| [11_platform.md](11_platform.md) | Platform アダプタ層 |
| [12_dialogs.md](12_dialogs.md) | Dialogs モジュール概要 |
| [13_viewer.md](13_viewer.md) | Viewer モジュール概要 |

---

## Detailed Design (Class Level)

### Widgets

| ドキュメント | 対象クラス |
|---|---|
| [widgets/TeFolderView.md](widgets/TeFolderView.md) | `TeFolderView` — フォルダビューの抽象基底クラス |
| [widgets/TeFileFolderView.md](widgets/TeFileFolderView.md) | `TeFileFolderView` — ファイルシステム向けフォルダビュー |
| [widgets/TeArchiveFolderView.md](widgets/TeArchiveFolderView.md) | `TeArchiveFolderView` — アーカイブ内閲覧ビュー |
| [widgets/TeFindFolderView.md](widgets/TeFindFolderView.md) | `TeFindFolderView` — 検索結果表示ビュー |

### Utils

| ドキュメント | 対象クラス |
|---|---|
| [utils/TeArchive.md](utils/TeArchive.md) | `TeArchive` — アーカイブ操作（圧縮・展開） |
| [utils/TeArchiveFileInfoAcsr.md](utils/TeArchiveFileInfoAcsr.md) | `TeArchiveFileInfoAcsr` — アーカイブ内ファイル情報アクセサ |
| [utils/TeFinder.md](utils/TeFinder.md) | `TeFinder` — 非同期検索の基底クラス |
| [utils/TeSearchQuery.md](utils/TeSearchQuery.md) | `TeSearchQuery` — 検索クエリのデータ構造と評価ロジック |

### Viewer

| ドキュメント | 対象クラス |
|---|---|
| [viewer/TeDocument.md](viewer/TeDocument.md) | `TeDocument` — ドキュメントビューワのデータモデル |
| [viewer/TeMarkupLoader.md](viewer/TeMarkupLoader.md) | `TeMarkupLoader` — マークアップ形式とコンテナの対応管理 |
