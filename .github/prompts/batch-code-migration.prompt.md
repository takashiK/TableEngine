---
description: "API変更に伴う利用側コードの一括移行を実行する"
agent: "agent"
tools: ["read_file", "grep_search", "run_in_terminal", "replace_string_in_file", "multi_replace_string_in_file", "runSubagent", "manage_todo_list"]
---

# コード移行（API 変更時）

> 言語固有のビルドコマンドは `.github/skills/refactoring/SKILL.md` の Language Detection で
> 言語を判定し、`lang/` フラグメントをロードして取得する。
> 検索スコープは必ず `TableEngine/**` に限定すること（Qt6ソースの `src/` は除外）。

## 入力情報

ユーザーから以下を受け取る:
- 変更前後の API 対応表
- 対象ディレクトリ（デフォルト: `TableEngine/` 全体）

## 実行手順

### 1. 影響箇所の網羅調査

grep_search で旧 API パターンを全検索:

```
検索パターン: 旧メソッド名・クラス名を `|` で OR 結合
例 (クラス名変更):    `TeOldClass|oldMethod\(`
例 (シグナル変更):    `connect.*oldSignal|emit oldSignal`
例 (ヘッダ include):  `#include.*TeOldClass`
includePattern: TableEngine/**
isRegexp: true
```

結果が 20 件超の場合は Explore subagent に委譲:

```
Research only — do NOT modify files.
Target: TableEngine/ 配下の全ソースファイル
Task: Find all occurrences of pattern {regex}
Return format:
  | ファイル | 行 | コード | 変換後 |
Thoroughness: thorough
```

### 2. 置換ルール定義

結果を元に置換ルールを表形式で定義:

| # | 正規表現(検索) | 置換文字列 | 対象ファイル数 |
|---|--------------|-----------|-------------|
| 1 | `TeOldUtil::open\(` | `TeNewUtil::load(` | 5 |
| 2 | `#include "TeOldUtil.h"` | `#include "TeNewUtil.h"` | 5 |

### 3. 一括置換実行

**PowerShell で一括実行:**

```powershell
$files = @('TableEngine/src/...', ...)
foreach ($f in $files) {
    $content = Get-Content $f -Raw
    $content = $content -replace 'TeOldUtil::open\(', 'TeNewUtil::load('
    $content = $content -replace '#include "TeOldUtil.h"', '#include "TeNewUtil.h"'
    Set-Content $f $content -NoNewline
}
```

### 4. include 修正

新クラスの `#include` を追加し、不要になった `#include` を削除する。
構文は言語フラグメントの「include 一括追加」例を参照（PowerShell 一括置換）。
include 整理は VS Code C/C++ 拡張のフォーマット（保存時自動適用）に任せる。

### 5. ビルド検証

言語フラグメントの「全体ビルド」コマンドを実行する。
エラーがあれば個別修正する。

### 6. 完了レポート

```markdown
## 移行完了

### 変更統計

| パターン | 変更ファイル数 | 変更箇所数 |
|---------|-------------|-----------|

### 変更ファイル一覧

| ファイル | 変更内容 |
|---------|---------|
```

