---
name: codebase-analysis
description: リポジトリのコード構造を効率的に分析し、型・関数・依存関係の構造化レポートを生成する。大きいファイルの読解や複数ファイルのパターン調査を Explore サブエージェントに委譲してコンテキストを節約する。コード構造の把握・利用箇所調査・ドキュメント読解の際に使用する。
---

# Codebase Analysis Skill

## Purpose

リポジトリのコード構造を効率的に分析し、構造化レポートを生成する。
大ファイルの逐次読み取りによるコンテキスト圧迫を防ぎ、サブエージェント委譲で効率化する。

## When to Use

- パッケージ/モジュールの責務分析、リファクタリング前の現状把握
- インターフェース/型変更の影響範囲調査
- 利用パターン（examples, tests）の収集

## Language Detection

手順実行前に対象言語を判定し、該当する `lang/` フラグメントを **1つだけ** read_file する。

| 検出ファイル | 言語ID | フラグメント |
|------------|--------|------------|
| go.mod | go | `lang/go.md` |
| Cargo.toml | rust | `lang/rust.md` |
| CMakeLists.txt / *.cpp / *.hpp | cpp | `lang/cpp.md` |
| *.c / *.h（単独） | c | `lang/cpp.md`（C/C++ 共用） |

- 複数該当時はユーザーに確認する
- フラグメントには「型定義の呼称」「依存解析コマンド」のみ記載

## Procedure

### Step 1: 構造マップ取得

言語フラグメントの「型定義呼称」を埋め込んで Explore subagent に委譲:
```
Research only — do NOT modify files.
Target: {package/module directory}
Task: Extract complete structure
Return format:
  - Module/namespace name and imports (grouped: stdlib / internal / external)
  - All {型定義: lang フラグメント参照} with members
  - All exported functions/methods with signatures
  - File-by-file breakdown (filename → what it contains)
Thoroughness: thorough
```

### Step 2: 依存関係分析

```
Research only — do NOT modify files.
Target: all source files depending on {module path}
Task: Find all usages of {type/interface name}
Return format:
  - Per-file: import alias, members used, call site context (3 lines)
  - Summary table: file | members used | pattern (direct/wrapped/embedded)
Thoroughness: medium
```

依存グラフの機械取得が必要な場合は言語フラグメントの「依存解析コマンド」を使う。

### Step 3: 分析結果の永続化

Session memory に以下を記録:
```markdown
## {Module} Analysis Summary
- Files: {count}, Total lines: {count}
- Exported types: {list}
- Key interfaces: {name} ({member count} members)
- Internal dependencies: {list}
- External consumers: {list with usage pattern}
```

## Output Template

分析完了後、以下の構造でレポートを生成する:

```markdown
## 構造分析: {module name}

### 概要

| 項目 | 値 |
|------|---|
| パス | `{path}` |
| ファイル数 | {n} |
| 主要インターフェース | `{name}` ({n} members) |
| 外部利用箇所 | {n} files |

### 構成図

​```mermaid
graph TD
    subgraph {module name}
        ...nodes...
    end
​```

### インターフェース定義

| メンバー | 引数 | 戻り値 | 責務カテゴリ |
|---------|------|--------|------------|
| ... | ... | ... | ... |

### 依存関係

| 利用元 | 利用メンバー | パターン |
|--------|------------|---------|
| ... | ... | ... |

### 分析所見

- {箇条書きで設計上の特徴・問題点・改善余地を列挙}
```

## Anti-patterns

| やってはいけないこと | 代替手法 |
|-------------------|---------|
| 大ファイルを100行ずつ逐次 read | Explore subagent で一括読み取り |
| grep 結果を1件ずつ read_file で確認 | subagent に "pattern + context" で一括取得 |
| 分析結果をコンテキストに保持し続ける | session memory に書き出して解放 |
| 同じファイルを2回読む | 1回目の結果を memory/変数で参照 |
| 複数言語フラグメントを同時ロード | 判定した1言語のみ read |
