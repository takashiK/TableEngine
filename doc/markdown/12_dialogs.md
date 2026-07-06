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
| `TeToolbarSetting` | `TeCmdToolbarSetting` | ツールバーに配置するボタンとコマンドのカスタマイズ |
| `TeFontDialog` | `TeToolbarSetting` や外観設定 | フォント・文字色・背景色の入力 |
| `TeToolDialog` | `TeCmdToolSetting` | 外部ツール設定（text/image/binary編集コマンドとユーザー定義ツールリスト）を編集するダイアログ |

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

### TeToolDialog

外部ツール設定（`TeCmdToolSetting` から起動）を編集するダイアログです。General タブと User タブを持ちます。

| タブ | 内容 |
|---|---|
| General | `tools/text_edit` / `tools/image_edit` / `tools/binary_edit` の3つの実行コマンドを編集する。各行は入力欄と Browse（`...`）ボタンで構成され、Browse でファイルを選択すると既存の引数を破棄して全文を置換する（キャンセル時は変更なし）。入力変更時に即座にメンバへ反映しつつ `TeToolCommand::isValidFormat()` で書式検証し、不正な場合は入力欄を赤枠表示する（入力はブロックしない） |
| User | `tools/user/tool01`〜`tool99`（最大 `TeSettings::MAX_USER_TOOLS` = 99件）に対応する可変長のユーザー定義ツールリスト。1エントリは「Command」欄（削除ボタン付き）と「Suffixes」欄（`suffix1; suffix2; suffix3` 形式）の2段組で、`QScrollArea` で縦スクロールする。Add ボタンで末尾に空エントリを追加し、既存設定分のみ初期表示する（空行は作らない） |

ダイアログ表示時（コンストラクタ）に `QSettings` から読み込みます。OK 時のみ書式検証を経て書き戻し（不正な場合は警告して閉じません）、Cancel 時は何も保存しません。  
保存時、`tools/user` グループは毎回クリアしてから `tool01` 〜順に書き戻します。

コマンド文字列で利用可能なマクロ（`%F` / `%f` / `%M` / `%m` / `%P`）の詳細は [10_utils.md](10_utils.md) の `TeToolCommand` を参照してください。
