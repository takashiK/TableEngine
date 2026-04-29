# Dialogs

## Overview

`src/dialogs/` はユーザー入力を収集するモーダルダイアログ群です。  
各ダイアログは `QDialog` を継承し、コマンドから呼ばれて入力値を返します。  
ビジネスロジックは持たず、UI 表示と入力値の提供に専念します。

---

## Dialog List

| クラス | 起動コマンド | 用途 |
|---|---|---|
| `TeFilePathDialog` | `TeCmdCopyTo` / `TeCmdMoveTo` / `TeCmdGotoFolder` 等 | コピー先 / 移動先パスの入力。履歴・お気に入りリストからの選択もサポート |
| `TeSelectPathDialog` | `TeCmdFolderCreate` 等 | フォルダ選択ダイアログ |
| `TePathListDialog` | `TeCmdEditFavorites` 等 | パスのリスト編集（追加 / 削除 / 並び替え） |
| `TeFindDialog` | `TeCmdFind` | ファイル検索条件の入力（ファイル名 / サイズ / 更新日時フィルタ） |
| `TeFilterDialog` | `TeCmdViewFilter` | ファイルリストの表示フィルタ設定 |
| `TeRenameMultiDialog` | `TeCmdRenameMulti` | 連番付き一括リネームのパターン入力 |
| `TeDivideDialog` | `TeCmdDivideFile` | ファイル分割サイズの入力 |
| `TeCombineDialog` | `TeCmdCombineFile` | ファイル結合の設定入力 |
| `TeCommandInputDialog` | `TeCmdRunCommand` | コマンド実行時のパラメータ入力 |
| `TeAskCreationModeDialog` | ファイル / フォルダ作成系 | 作成モード（新規 / 上書き等）の選択 |
| `TeKeySetting` | `TeCmdKeySetting` | キーボードショートカット設定 |
| `TeMenuSetting` | `TeCmdMenuSetting` | メニュー / ポップアップメニューのカスタマイズ |
| `TeOptionSetting` | `TeCmdOption` | グローバル設定（起動オプション等） |

---

## Key Dialogs

### TeFilePathDialog

パス入力の汎用ダイアログです。TableEngine 内で最も多用されます。  
以下の機能を持ちます：

- テキストボックスによる直接パス入力
- 移動履歴（`TeHistory`）からのドロップダウン選択
- お気に入りリスト（`TeFavorites`）からのドロップダウン選択
- 任意の追加フラグ（チェックボックス）の表示（呼び出し元がカスタマイズ可能）

```
+-----------------------------------------+
| current path: [/home/user         ▼]    |
| target path:  [/home/user/copy    ▼]    |
|               [  OK  ] [Cancel]         |
+-----------------------------------------+
```

### TeFindDialog

ファイル検索条件を入力するダイアログです。  
入力された条件は `TeSearchQuery::fromDialog()` で `TeSearchQuery` に変換されます。

| 入力項目 | 説明 |
|---|---|
| ファイル名 | ワイルドカードまたは正規表現で指定。大文字小文字の区別オプションあり |
| ファイルサイズ | KB / MB 単位で指定。「以上」または「以下」の条件 |
| 更新日時 | 日付範囲（From / To）で指定 |

### TeKeySetting

コマンドとキーシーケンスの対応を編集するダイアログです。  
変更内容は `QSettings`（`SETTING_KEY` キー）に保存され、  
`TeDispatcher::loadKeySetting()` が次回起動時に読み込みます。
