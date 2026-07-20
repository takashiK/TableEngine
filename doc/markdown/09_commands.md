# Commands

## Overview

`src/commands/` はユーザー操作のビジネスロジックを実装するコマンドクラス群です。  
すべてのコマンドは `TeCommandBase` を継承し、`execute(TeViewStore*)` を実装するだけで機能します。  
`TeCommandFactory` がすべてのコマンドを管理し、`CmdId` を渡すと対応するインスタンスを生成して返します。

コマンドパターンの詳細な仕組みは [06_dispatcher_command.md](06_dispatcher_command.md) を参照してください。

---

## Command Categories

コマンドは `CmdId` の値域でカテゴリに分類されます（`0x1000` 刻み）。  
ソースコードの配置もこの分類に対応しています。

| カテゴリ | CmdId 範囲 | ディレクトリ | 概要 |
|---|---|---|---|
| File | `0x1000〜` | `commands/file/` | ファイル・フォルダ操作 |
| Edit | `0x2000〜` | `commands/edit/` | 選択・クリップボード操作 |
| Folder | `0x3000〜` | `commands/folder/` | フォルダナビゲーション |
| View | `0x4000〜` | `commands/view/` | 表示設定 |
| Tool | `0x5000〜` | `commands/tool/` | ツール設定・ビューワ / エディタ起動 |
| Window | `0x6000〜` | `commands/window/` | ウィンドウ・タブ操作 |
| Setting | `0x7000〜` | `commands/setting/` | 設定ダイアログ |
| Help | `0x8000〜` | `commands/help/` | ヘルプ・バージョン情報 |
| Navi | `0x9000〜` | `commands/navi/` | カーソル移動・フォーカス操作 |
| User | `0xA000〜` | `commands/user/` | ユーザーコマンドの登録・実行 |
| CMDID_USER | `0x00010000〜` | — | 登録済みユーザーコマンドの実体 ID（動的登録） |
| Special | `0x00020000〜` | — | メニュー区切り・フォルダ等の特殊 ID |

---

## Command List

### File Commands (`commands/file/`)

| CmdId | クラス | 説明 |
|---|---|---|
| `CMDID_SYSTEM_FILE_NEW` | `TeCmdFileCreate` | 新規ファイル作成 |
| `CMDID_SYSTEM_FILE_OPEN` | `TeCmdOpenFile` | ファイルを関連アプリで開く |
| `CMDID_SYSTEM_FILE_COPY_TO` | `TeCmdCopyTo` | 指定フォルダへコピー |
| `CMDID_SYSTEM_FILE_MOVE_TO` | `TeCmdMoveTo` | 指定フォルダへ移動 |
| `CMDID_SYSTEM_FILE_DELETE` | `TeCmdDelete` | ファイル / フォルダ削除 |
| `CMDID_SYSTEM_FILE_RENAME` | `TeCmdRename` | リネーム |
| `CMDID_SYSTEM_FILE_RENAME_MULTI` | `TeCmdRenameMulti` | 連番付き一括リネーム |
| `CMDID_SYSTEM_FILE_CHANGE_ATTRIBUTE` | `TeCmdChangeAttribute` | 属性変更 |
| `CMDID_SYSTEM_FILE_DIVIDE` | `TeCmdDivideFile` | ファイル分割 |
| `CMDID_SYSTEM_FILE_COMBINE` | `TeCmdCombineFile` | ファイル結合 |
| `CMDID_SYSTEM_FILE_ARCHIVE` | `TeCmdArchive` | アーカイブ圧縮 |
| `CMDID_SYSTEM_FILE_ARCHIVE_NEW` | `TeCmdArchiveCreate` | 新規アーカイブ作成（Explorer モード） |
| `CMDID_SYSTEM_FILE_EXTRACT` | `TeCmdExtract` | アーカイブ展開 |
| `CMDID_SYSTEM_FILE_RUN_WITH_COMMAND` | `TeCmdRunCommand` | 選択ファイルをコマンドで実行 |
| `CMDID_SYSTEM_FILE_PROPERTY` | `TeCmdProperties` | プロパティ表示 |
| `CMDID_SYSTEM_FILE_EXIT` | `TeCmdExit` | アプリ終了 |

### Edit Commands (`commands/edit/`)

| CmdId | クラス | 説明 |
|---|---|---|
| `CMDID_SYSTEM_EDIT_CUT` | `TeCmdCut` | 切り取り |
| `CMDID_SYSTEM_EDIT_COPY` | `TeCmdCopy` | コピー |
| `CMDID_SYSTEM_EDIT_PASTE` | `TeCmdPaste` | 貼り付け |
| `CMDID_SYSTEM_EDIT_SELECT_ALL` | `TeCmdSelectAll` | 全選択 |
| `CMDID_SYSTEM_EDIT_SELECT_TOGGLE` | `TeCmdSelectToggle` | 選択トグル |
| `CMDID_SYSTEM_EDIT_SELECT_BY_FILTER` | `TeCmdSelectFilter` | フィルタで選択 |
| `CMDID_SYSTEM_EDIT_SELECTION_STYLE_EXPLORER` | `TeCmdSelectionStyle` | Explorer 互換選択スタイル |
| `CMDID_SYSTEM_EDIT_SELECTION_STYLE_TABLEENGINE` | `TeCmdSelectionStyle` | TableEngine 独自選択スタイル |

### Folder Commands (`commands/folder/`)

| CmdId | クラス | 説明 |
|---|---|---|
| `CMDID_SYSTEM_FOLDER_OPEN_ONE` | `TeCmdFolderOpenOne` | 現在フォルダを展開 |
| `CMDID_SYSTEM_FOLDER_OPEN_UNDER` | `TeCmdFolderOpenUnder` | 現在フォルダ以下を展開 |
| `CMDID_SYSTEM_FOLDER_OPEN_ALL` | `TeCmdFolderOpenAll` | すべて展開 |
| `CMDID_SYSTEM_FOLDER_CLOSE_UNDER` | `TeCmdFolderCloseUnder` | 現在フォルダ以下を折りたたむ |
| `CMDID_SYSTEM_FOLDER_CLOSE_ALL` | `TeCmdFolderCloseAll` | すべて折りたたむ |
| `CMDID_SYSTEM_FOLDER_CREATE_FOLDER` | `TeCmdFolderCreate` | フォルダ作成 |
| `CMDID_SYSTEM_FOLDER_ADD_QUICKACCESS` | `TeCmdAddQuickAccess` | ドライブバーにクイックアクセスを追加 |
| `CMDID_SYSTEM_FOLDER_EDIT_QUICKACCESS` | `TeCmdEditQuickAccess` | クイックアクセスの編集 |
| `CMDID_SYSTEM_FOLDER_ADD_FAVORITE` | `TeCmdAddFavorite` | お気に入りに追加 |
| `CMDID_SYSTEM_FOLDER_EDIT_FAVORITES` | `TeCmdEditFavorites` | お気に入り編集 |
| `CMDID_SYSTEM_FOLDER_PREV_FOLDER` | `TeCmdPrevFolder` | 前のフォルダに戻る |
| `CMDID_SYSTEM_FOLDER_NEXT_FOLDER` | `TeCmdNextFolder` | 次のフォルダに進む |
| `CMDID_SYSTEM_FOLDER_GOTO_PARENT` | `TeCmdGotoParent` | 親フォルダへ移動 |
| `CMDID_SYSTEM_FOLDER_GOTO_ROOT` | `TeCmdGotoRoot` | ルートへ移動 |
| `CMDID_SYSTEM_FOLDER_GOTO_FOLDER` | `TeCmdGotoFolder` | 指定フォルダへ移動 |
| `CMDID_SYSTEM_FOLDER_CHANGE_ROOT` | `TeCmdFolderChangeRoot` | ルートパス変更 |
| `CMDID_SYSTEM_FOLDER_FIND` | `TeCmdFind` | ファイル検索 |

### View Commands (`commands/view/`)

| CmdId | クラス | 説明 |
|---|---|---|
| `CMDID_SYSTEM_VIEW_FILESIZE` | `TeCmdViewShowInfo` | ファイルサイズ列の表示切替 |
| `CMDID_SYSTEM_VIEW_MODIFIED` | `TeCmdViewShowInfo` | 更新日時列の表示切替 |
| `CMDID_SYSTEM_VIEW_SHOW_HIDDEN` | `TeCmdViewShowFile` | 隠しファイルの表示切替 |
| `CMDID_SYSTEM_VIEW_SHOW_SYSTEM` | `TeCmdViewShowFile` | システムファイルの表示切替 |
| `CMDID_SYSTEM_VIEW_FILTER` | `TeCmdViewFilter` | フィルタ設定ダイアログ |
| `CMDID_SYSTEM_VIEW_ORDERBY_*` | `TeCmdViewFileOrderBy` | ソート基準の変更 |
| `CMDID_SYSTEM_VIEW_ORDER_*` | `TeCmdViewFileOrder` | 昇順 / 降順の切替 |
| `CMDID_SYSTEM_VIEW_SORT_SETTING` | `TeCmdViewSortSetting` | ソート設定ダイアログ |
| `CMDID_SYSTEM_VIEW_SMALL_ICON` 等 | `TeCmdViewLayout` | 表示モードの変更 |

### Window Commands (`commands/window/`)

| CmdId | クラス | 説明 |
|---|---|---|
| `CMDID_SYSTEM_WINDOW_TOOLBAR` 等 | `TeCmdWindowBar` | 各種バーの表示切替 |
| `CMDID_SYSTEM_WINDOW_NEW_TAB` | `TeCmdWindowNewTab` | 新規タブ |
| `CMDID_SYSTEM_WINDOW_CLOSE_TAB` | `TeCmdWindowCloseTab` | タブを閉じる |
| `CMDID_SYSTEM_WINDOW_MOVE_TAB` | `TeCmdWindowMoveTab` | タブを別ペインへ移動 |
| `CMDID_SYSTEM_WINDOW_NEXT_TAB` | `TeCmdWindowNextTab` | 次のタブへ切替 |
| `CMDID_SYSTEM_WINDOW_PREV_TAB` | `TeCmdWindowPrevTab` | 前のタブへ切替 |

### Setting Commands (`commands/setting/`)

| CmdId | クラス | 説明 |
|---|---|---|
| `CMDID_SYSTEM_SETTING_OPTION` | `TeCmdOption` | グローバル設定 |
| `CMDID_SYSTEM_SETTING_KEY` | `TeCmdKeySetting` | キー割り当て設定 |
| `CMDID_SYSTEM_SETTING_MENU` | `TeCmdMenuSetting` | メニュー設定 |
| `CMDID_SYSTEM_SETTING_TOOLBAR` | `TeCmdToolbarSetting` | ツールバー設定 |

### Help Commands (`commands/help/`)

| CmdId | クラス | 説明 |
|---|---|---|
| `CMDID_SYSTEM_HELP_SHOW` | `TeCmdHelp` | 既定のブラウザでヘルプを開く |
| `CMDID_SYSTEM_HELP_VERSION` | `TeCmdVersion` | バージョン情報ダイアログを表示 |

### Tool Commands (`commands/tool/`)

| CmdId | クラス | 説明 |
|---|---|---|
| `CMDID_SYSTEM_TOOL_VIEW_FILE` | `TeCmdToolFile` | テキスト / ドキュメントビューワ起動 |
| `CMDID_SYSTEM_TOOL_VIEW_BINARY` | `TeCmdToolBinary` | バイナリビューワ起動 |
| `CMDID_SYSTEM_TOOL_EDIT_FILE` | `TeCmdToolFileEdit` | `tools/text_edit` に保存されたコマンドを展開して実行し、ファイルを編集する |
| `CMDID_SYSTEM_TOOL_EDIT_BINARY` | `TeCmdToolBinaryEdit` | `tools/binary_edit` に保存されたコマンドを展開して実行し、バイナリファイルを編集する |
| `CMDID_SYSTEM_TOOL_SETUP` | `TeCmdToolSetting` | `TeToolDialog` を表示し、外部ツール設定（text / image / binary 編集コマンド、ユーザー定義ツールリスト）を編集する |

コマンド文字列で利用できるマクロ（`%F` / `%f` / `%M` / `%m` / `%P`）の詳細は [10_utils.md](10_utils.md) の `TeToolCommand` を参照してください。

### Navigation Commands (`commands/navi/`)

キーボード中心操作のためのカーソル移動・フォーカス切替コマンド群です。

| CmdId | クラス | 説明 |
|---|---|---|
| `CMDID_SYSTEM_NAVI_UP` | `TeCmdNaviUp` | カーソルを上へ移動 |
| `CMDID_SYSTEM_NAVI_DOWN` | `TeCmdNaviDown` | カーソルを下へ移動 |
| `CMDID_SYSTEM_NAVI_LEFT` | `TeCmdNaviLeft` | カーソルを左へ移動 |
| `CMDID_SYSTEM_NAVI_RIGHT` | `TeCmdNaviRight` | カーソルを右へ移動 |
| `CMDID_SYSTEM_NAVI_DRIVEBAR` | `TeCmdNaviDriveBar` | フォーカスをドライブバーへ移動 |
| `CMDID_SYSTEM_NAVI_TOGGLE_FOLDER_LR` | `TeCmdNaviToggleFolderLR` | 左右ペイン間でフォーカスを切替 |
| `CMDID_SYSTEM_NAVI_TOGGLE_FOLDER_TREE` | `TeCmdNaviToggleFolderTree` | フォルダビューとツリービュー間でフォーカスを切替 |
| `CMDID_SYSTEM_NAVI_DETAIL_SCROLL_UP` | `TeCmdNaviDetailScrollUp` | 詳細ビューを上へスクロール |
| `CMDID_SYSTEM_NAVI_DETAIL_SCROLL_DOWN` | `TeCmdNaviDetailScrollDown` | 詳細ビューを下へスクロール |
| `CMDID_SYSTEM_NAVI_OPEN_ITEM_FOLDER` | `TeCmdNaviItemFolder` | 選択アイテムの親フォルダを開く |

### User Commands (`commands/user/`)

ユーザーが登録した外部ツールコマンドを管理・実行するためのコマンド群です。  
登録済みコマンドの実体は `CMDID_USER`（`0x00010000〜`）の値域で `TeCommandFactory` に動的登録され、  
`QSettings` の `user/command00`〜（最大 `TeSettings::MAX_USER_COMMANDS` 件）から読み込まれます。

| CmdId | クラス | 説明 |
|---|---|---|
| `CMDID_SYSTEM_USER_REGIST_COMMAND` | `TeCmdUserRegistCommands` | ユーザーコマンドの登録ダイアログ（`TeEditUserCommands`）を表示 |
| `CMDID_SYSTEM_USER_EXECUTE` | `TeCmdUserExecCommand` | 登録済みユーザーコマンドをパラメータ付きで実行 |

---

## CmdType Flags

`TeCommandInfoBase::type()` は `CmdType` フラグを返し、コマンドの性質を表します。

| フラグ | 説明 |
|---|---|
| `CMD_TRIGGER_NORMAL` | 通常コマンド（ステートレス） |
| `CMD_TRIGGER_SELECT` | トグルコマンド（選択状態を持つ） |
| `CMD_CATEGORY_TREE` | ツリービューに対して有効 |
| `CMD_CATEGORY_LIST` | リストビューに対して有効 |
| `CMD_TARGET_DIRECTORY` | ディレクトリを対象とするコマンド |
| `CMD_TARGET_FILE` | ファイルを対象とするコマンド |

これらのフラグは `TeCommandInfoBase::isActive()` の実装でメニュー項目のグレーアウト判定等に使用されます。
