<!-- =================================================================== -->
<!-- ===== 前半: 汎用ルール（プロジェクト非依存・移植可能）           ===== -->
<!-- ===== 移植時はこの前半セクションのみを別リポジトリにコピーする   ===== -->
<!-- =================================================================== -->

## Language

- ユーザーとの会話は日本語で行う
- コード内のコメント・変数名は英語

## Output Format Rules

### Report Structure (設計レポート・分析レポート)

```markdown
## タイトル

### 1. 概要
- 1-3文で目的を述べる

### 2. 現状分析
- 表形式で構造化する
- Mermaid 図で関係性を示す

### 3. 設計案
- Before/After を対比する表
- 判断根拠を箇条書きで述べる

### 4. 実装計画
- 番号付きステップ
- 各ステップの依存関係を明示

### 5. リスク・注意点
- 表形式（項目 | リスク | 対策）
```

### Mermaid Diagram Guidelines

- `flowchart` — 処理フロー、パッケージ関係
- `sequenceDiagram` — API 呼び出しシーケンス、ロック取得順序
- `stateDiagram-v2` — 状態遷移
- `graph TD/LR` — 依存関係、アーキテクチャ
- ノードラベルは日本語可、変数名はバッククォート

### Table Guidelines

- 比較には必ず表を使う（3項目以上の列挙は箇条書きより表を優先）
- 列ヘッダーは簡潔に（5文字以内推奨）
- コード片はバッククォートで囲む

### Code Block Guidelines

- 言語指定を必ず付ける（```go, ```mermaid, etc.）
- 説明対象のコードのみ示す（不要な前後は省略）
- 変更点を示す場合は Before/After を並べる

### Report Delivery And Approval Gate

- 設計レポート要求時は、レポート本文を必ずチャット上に直接表示する（先にファイルへ保存しない）
- 設計レポートを伴う作業は、実装・設定変更・ファイル編集の前に「内容確認と承認」をユーザーへ求める
- 承認前は read-only の調査とレポート提示までに留め、変更系ツール（編集・実行）を開始しない

## Subagent Delegation Rules

### When to Delegate

| 状況 | 委譲先 | プロンプト要件 |
|------|--------|--------------|
| 300行超のファイル構造把握 | Explore | "Read fully, return: type list, function list, import list" |
| 3ファイル以上のパターン調査 | Explore | "Search pattern X in dir Y, return: file, line, context" |
| ドキュメント全読解 | Explore | "Read all files in dir, return: per-file summary" |

### Delegation Prompt Template

```
Research only — do NOT modify files.
Target: {files or directory}
Task: {what to extract}
Return format: {structured format specification}
Thoroughness: {quick|medium|thorough}
```

## Context Management

- フェーズ切り替え時に session memory に決定事項を書き出す
- 同じファイルを2回以上 read する場合は、前回の読み取り結果を参照する
- compaction を避けるため、大量のコード出力結果は即座に要約する

## Model Selection (Agent Auto)

**運用方針: Agent は Auto モードをメインとして使用する。**
Auto モードの場合、タスク種別に応じてモデルを選択する。
モデルは常に各系列の**最新版**を使用する（版名は固定しない）。

| タスク種別 | 複雑度 | 推奨モデル |
|-----------|--------|-----------|
| コード読解・検索 | 低 | GPT 最新 mini / MAI 最新 |
| サブエージェント探索 | 低 | GPT 最新 mini / MAI 最新 |
| 一括置換・機械的修正 | 低 | GPT 最新 mini / MAI 最新 |
| ドキュメント更新 | 中 | Claude 最新 Sonnet |
| ビルドエラー修正 | 中 | Claude 最新 Sonnet |
| 設計レポート生成 | 高 | Claude 最新 Opus |
| アーキテクチャ設計 | 高 | Claude 最新 Opus |

- 優先順位: 品質 > トークンコスト
- 軽量タスクは GPT 最新 mini または MAI 最新に集約しコストを削減する

## Generic Workflow Rules

### Analysis Phase

1. プログラムの設計資料は、doc/markdown/ に Markdown 形式で記載されているものを参照する
2. 大ファイル（300行超）の読解は Explore サブエージェントに委譲する（→ codebase-analysis SKILL 参照）
3. 利用パターン調査（grep → read の連鎖が3回超える場合）はサブエージェントに委譲する
4. 分析結果は session memory に要約を書き出してからフェーズを進める

### Implementation Phase

1. 同一パターンの置換が3ファイル以上に及ぶ場合は PowerShell の一括置換を使う（→ refactoring SKILL 参照）
2. import 追加は言語標準のフォーマッタを意識し、グループ単位で整理する
3. 中間ビルド検証: インターフェース変更後、利用側修正前にビルドを実行する
4. 利用側更新完了後に全体ビルドを確認する（言語別コマンドは refactoring SKILL の lang/ 参照）

### Documentation Phase

1. ドキュメント読解は Explore サブエージェントに委譲する
2. コード例は必ず最新の API シグネチャに合わせる
3. Mermaid 図はシーケンス図・フローチャート・状態遷移図を適切に使い分ける

## Skills Reference

詳細手順は以下のスキルファイルを参照すること（言語判定後に lang/ フラグメントをロード）:

- `.vscode/skills/codebase-analysis/SKILL.md` — コード構造分析の手順・委譲テンプレート
- `.vscode/skills/refactoring/SKILL.md` — リファクタリングの Phase 管理・一括置換

<!-- =================================================================== -->
<!-- ===== 後半: ワークスペース固有ルール（このプロジェクト専用）     ===== -->
<!-- ===== 移植時はこの後半セクションを各プロジェクト向けに書き換える ===== -->
<!-- =================================================================== -->

## Repository Context

- C++17, Qt 6.11+ (Core / Gui / Widgets / Core5Compat), MSVC 2022, CMake 3.19+
- アプリ名: TableEngine（キーボード中心のファイルエクスプローラ, GPLv2, Windows）
- 概要: `doc/markdown/01_overview.md` / アーキテクチャ: `doc/markdown/04_architecture.md`

### ワークスペース構成の注意

本ワークスペースは2つのフォルダルートを含む:

| フォルダ | 内容 | 操作対象 |
|---------|------|----------|
| `TableEngine/` | 本プロジェクト（**編集・検索対象**） | ✓ |
| `src/` (ルート直下) | Qt 6 ソースコード（**除外**） | ✗ |

- `grep_search` / Explore の `includePattern` には必ず `TableEngine/**` を指定する
- `src/` ルートは編集・分析・検索対象に含めない

### ドキュメント構成

| ディレクトリ | 内容 | 更新方法 |
|------------|------|----------|
| `doc/markdown/` | 設計資料（アーキテクチャ・ビルド手順等） | `/update-docs` プロンプトで定期更新 |
| `doc/manual/` | ユーザー向けヘルプガイド | 同上 |

ドキュメントは `/update-docs` プロンプトで定期的に最新化し、鮮度を保証する。

### ソースコード構成

```
TableEngine/
├── main/          # エントリポイント (main.cpp)
├── src/           # コアライブラリ (tablelibs として静的リンク)
│   ├── commands/  # コマンドパターン実装
│   ├── dialogs/   # ダイアログ群
│   ├── platform/  # OS 抽象化 (Windows 実装)
│   ├── utils/     # ユーティリティクラス群
│   ├── viewer/    # テキスト・画像・バイナリビューア
│   └── widgets/   # UI ウィジェット群
├── tests/         # ユニットテスト (gtest/gmock)
├── viewer_tests/  # Viewer モジュールテスト
└── support/       # 外部ライブラリ (QHexView, valijson, vcpkg)
```

## Project Language

- 言語: C++17（言語ID: `cpp`）
- フォーマッタ: VS Code C/C++ 拡張（保存時自動フォーマット、CLI は使用しない）
- ビルド検証: CMake プリセット使用
- 詳細コマンドは各 SKILL の `lang/cpp.md` を参照
