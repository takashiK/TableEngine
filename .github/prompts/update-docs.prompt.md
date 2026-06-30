---
description: "コード変更に合わせて doc/markdown/ および doc/manual/ のドキュメントを更新する。定期実行にも対応。"
agent: "agent"
tools: ["read_file", "replace_string_in_file", "multi_replace_string_in_file", "grep_search", "file_search", "runSubagent", "manage_todo_list"]
---

# ドキュメント一括更新

> **定期実行用途にも対応。** コード変更の都度実施するほか、定期メンテナンス時にも同じ手順で実行できる。
> トークン削減のため、**全ファイル読み込み前に grep で影響箇所を絞り込む**こと。

## ドキュメント構成

| ディレクトリ | 内容 | 主な読者 |
|------------|------|---------|
| `doc/markdown/` | 設計資料（アーキテクチャ・ビルド手順・クラス設計等） | 開発者 |
| `doc/manual/` | ユーザー向けヘルプガイド | エンドユーザー |

## 入力情報

ユーザーから以下を受け取る（省略時はデフォルトを使用）:
- 変更内容の概要（API 変更・クラス移動・機能追加など）
- 対象ドキュメントディレクトリ（デフォルト: `doc/markdown/`、必要に応じて `doc/manual/` も対象）

---

## 実行手順

### 1. 影響範囲の特定（grep で絞り込み — 軽量モデル推奨）

全ドキュメントを読む前に grep で影響箇所を絞り込む:

```
変更されたクラス名・メソッド名・ファイル名パターンを | で OR 結合して検索:
includePattern: doc/markdown/**  または  doc/manual/**
isRegexp: true
例: TeArchive|archiveOpen|archive_open
```

**結果が 0 件の場合**: 更新不要と判断し、その旨を報告して終了。
**結果が 5 ファイル以下**: 該当ファイルのみ read_file で直接読む（Step 2 をスキップ）。
**結果が 6 ファイル以上**: Step 2 の Explore subagent に委譲。

### 2. ドキュメント読解（Explore subagent に委譲 — Step 1 で 6 件以上の場合のみ）

```
Research only — do NOT modify files.
Target: Step 1 でヒットしたファイルのみ（全ドキュメントは読まない）
Task: 各ファイルから以下を抽出:
  1. ファイルごとの概要（1行）
  2. 変更対象クラス/メソッドへの参照箇所（行番号 + 内容）
  3. コード例の行番号と使用メソッド
  4. Mermaid 図内の参照ノード名（行番号付き）
Return format: Per-file markdown with line references
Thoroughness: medium
```

### 3. 現在の実装状態確認（grep で直接取得 — Explore 不要）

Explore の代わりに grep_search でヘッダファイルから公開 API を直接取得する（トークン削減）:

```
includePattern: TableEngine/src/**/*.h
query: 変更対象クラス名（例: TeArchive）
```

必要に応じて対象ヘッダを read_file で確認する。

### 4. 差分特定と更新計画

Step 1-3 の結果から更新箇所を特定し、一覧表を作成する:

| ドキュメント | 行 | 現在の記述 | 更新後の記述 | 種別 |
|------------|---|-----------|------------|------|
| 13_viewer.md | 45 | `TeViewer::open(path)` | `TeViewer::load(path, mode)` | コード例 |
| 04_architecture.md | 80 | `TeViewer` ノード | `TeViewer` ノード（説明更新） | Mermaid |

**ユーザーに一覧を提示し確認を得てから更新を実施する。**

### 5. 一括更新

- 同一ファイル内の複数置換 → `multi_replace_string_in_file`
- 3 ファイル以上の同一パターン置換 → PowerShell 一括
- 各ファイル更新後のルール:
  - コード例は最新の C++/Qt6 API シグネチャに合致すること
  - Mermaid 図は参加者名・ノード名を更新すること
  - 表の列が崩れていないこと
  - `doc/manual/` は **エンドユーザー向けの文体を維持**すること（技術用語を平易な表現に保つ）

### 6. 完了レポート

```markdown
## ドキュメント更新完了

### 更新ファイル

| ファイル | 変更箇所数 | 主な変更 |
|---------|-----------|---------|

### 変更パターン

| 旧 | 新 | 該当数 |
|----|---|--------|
```

