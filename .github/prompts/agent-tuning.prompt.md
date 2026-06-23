---
description: "instructions / prompts / skills を実地の挙動に基づいて調整・改善する"
agent: "agent"
tools: ["read_file", "create_file", "replace_string_in_file", "multi_replace_string_in_file", "grep_search", "file_search", "run_in_terminal", "manage_todo_list", "memory"]
---

# Agent 設定チューニング

VS Code Copilot のカスタマイズファイル（instructions / prompts / skills）を、
実際の挙動で観測した問題に基づいて調整する。

> 設計思想は `.github/agent_design.md` を参照する。
> 調整後は同ファイルの「設計判断の根拠記録」に追記すること。

## 対象ファイル

| 種別 | パス | 役割 |
|------|------|------|
| instructions | `.github/copilot-instructions.md` | 自動ロードの全体ルール |
| skills | `.github/skills/{name}/SKILL.md` + `lang/` | 参照される詳細手順 |
| prompts | `.github/prompts/{name}.prompt.md` | `/name` で起動する手順 |
| 設計方針 | `.github/agent_design.md` | 設計思想・判断記録 |

## 実行手順

### 1. 課題のヒアリング

ユーザーから以下を聞き取る（不足は質問する）:
- どのファイル/挙動に問題があったか
- 期待した挙動 vs 実際の挙動
- 再現条件（モデル種別・タスク種別・言語）

### 2. 現状把握

対象ファイルを read_file する。複数ファイルにまたがる場合は Explore に委譲:

```
Research only — do NOT modify files.
Target: .github/copilot-instructions.md, .github/skills/**, .github/prompts/**
Task: {調査対象のルール・記述箇所を特定}
Return format: | ファイル | 行 | 該当記述 | 問題点 |
Thoroughness: medium
```

### 3. 原因分類

| 症状 | 想定原因 | 調整先 |
|------|---------|--------|
| ルールが無視される | 記述が曖昧 / 優先度不明 | instructions を具体化 |
| プロンプトが意図と違う動作 | 手順の抜け / 順序不整合 | prompt の手順修正 |
| 言語固有コマンドが誤り | フラグメント内容の不備 | skills の `lang/` 修正 |
| 無関係情報をロード | 言語判定漏れ | Language Detection 強化 |
| トークン浪費 | コア/フラグメント責務混在 | 責務分離を再設計 |
| モデル間で品質差 | 出力フォーマット指定不足 | Output Format Rules 強化 |

### 4. 調整方針レポート

以下を提示しユーザー承認を得る:

```markdown
## チューニング方針

### 課題
{観測された問題}

### 原因
{原因分類の結果}

### 修正案

| ファイル | Before | After | 根拠 |
|---------|--------|-------|------|

### 副作用・リスク
{移植性・他言語への影響}
```

### 5. 実装

承認後に修正を適用する。原則:
- 前半（汎用）と後半（固有）の境界を崩さない
- コア = 手順、フラグメント = コマンド表の責務分離を維持する
- 言語フラグメントは1言語1ファイルを保つ

### 6. 検証

- prompt 修正時: 実際に `/name` で起動し挙動を確認する
- skills 修正時: 該当言語フラグメントの整合を確認する
- instructions 修正時: 矛盾するルールがないか全体を再読する

### 7. 記録

`.github/agent_design.md` の「設計判断の根拠記録」に
`| 判断 | 理由 | 日付 |` 形式で1行追記する。

## Anti-patterns

| やってはいけないこと | 代替 |
|-------------------|------|
| 汎用ルールにプロジェクト固有名を混入 | 後半セクションに記述 |
| 全言語の情報を1ファイルに集約 | `lang/` フラグメント分割を維持 |
| 調整理由を記録しない | agent_design.md に追記 |
| 大量ルールを一度に変更 | 課題1つずつ調整・検証 |
