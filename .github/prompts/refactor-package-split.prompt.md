---
description: "クラス/モジュールのリファクタリング（分割・メンバー移動・API変更）を実行する"
agent: "agent"
tools: ["read_file", "create_file", "replace_string_in_file", "multi_replace_string_in_file", "grep_search", "run_in_terminal", "runSubagent", "manage_todo_list", "memory"]
---

# クラス/モジュール分割リファクタリング

> 言語固有のコマンドは `.github/skills/refactoring/SKILL.md` の Language Detection で
> 言語を判定し、該当する `lang/` フラグメントをロードして取得する。
> 検索スコープは必ず `TableEngine/**` に限定すること（Qt6ソースの `src/` は除外）。

## 入力情報

ユーザーから以下を受け取る:
- 対象クラス/ディレクトリ（例: `src/utils/TeArchive`）
- 分割方針（何をどこに移すか）
- 新クラス名/新ディレクトリ名

## 実行手順

### 1. 分析（Explore subagent に委譲）

```
Research only — do NOT modify files.
Target: {対象クラスのヘッダ・実装ファイル} (includePattern: TableEngine/src/**)
Task:
  1. 全 クラス/struct とメンバー一覧（public/protected/private 別）
  2. 各メンバーの依存関係（内部の他メンバー呼び出し、Q_OBJECT/シグナル/スロット含む）
  3. public（公開 API）vs private（内部実装）の分類
Return format: Markdown tables
Thoroughness: thorough
```

### 2. 利用箇所調査（Explore subagent に委譲）

```
Research only — do NOT modify files.
Target: TableEngine/ 配下の全ソースファイル（tests/ viewer_tests/ 含む）
Task: 移動対象メンバーの全呼び出し箇所を検索
Return format:
  | ファイル | 行 | 現在のコード | 変換後のコード |
Thoroughness: thorough
```

### 3. 設計レポート生成

以下の構造で設計レポートを出力し、ユーザー承認を得る:

```markdown
## リファクタリング設計

### 概要
{1-2文で目的}

### 移動対象

| メンバー/型 | 移動元 | 移動先 | 理由 |
|------------|--------|--------|------|
| `TeArchive::extractAsync()` | `src/utils/TeArchive` | `src/utils/TeArchiveWorker` | 非同期処理の分離 |

### API 変更対比

| Before | After |
|--------|-------|
| `TeArchive::extractAsync(path)` | `TeArchiveWorker::extractAsync(archive, path)` |

### 影響範囲

| ファイル | 変更箇所数 | 変更内容 |
|---------|-----------|---------|

### 実装ステップ

1. {具体的アクション}（依存: なし）
2. {具体的アクション}（依存: Step 1）
...

### リスク

| リスク | 対策 |
|--------|------|
| Q_OBJECT 移動による MOC 再生成 | ビルド時に自動解決 |
```

### 4. 実装

承認後、refactoring SKILL の Phase 2-5 に従って実行する。
中間/全体ビルドコマンドは判定言語の `lang/` フラグメントを参照する。

### 5. 検証

言語フラグメントの「全体ビルド」コマンドを実行する。
フォーマットは VS Code C/C++ 拡張（保存時自動適用）に任せる。
