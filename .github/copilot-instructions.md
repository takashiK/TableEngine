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

## Orchestration Strategy

メインチャット（オーケストレーター）は方針決定と **計画タスク単位の委譲** に専念し、実作業は
`runSubagent`（custom agent `Task Executor`）へ **まるごと委譲** する。細粒度の作業判断は executor
内部に閉じ込め、委譲の往復回数と要約オーバーヘッドを最小化する（細かい単発委譲はコスト非効率）。

### レイヤー責務（3層階層）

| 層 | エージェント / モデル | 責務 | やらないこと |
|----|------|------|------------|
| L0 オーケストレーター（main） | default / Opus 固定 | 意図理解・**計画タスク分解**・単位委譲・結果統合・設計判断・承認ゲート | 個別 read/grep・一括編集・細粒度の単発委譲 |
| L1 タスク実行 | `Task Executor` / mid | 受領した1単位を調査→実装→検証まで自己完結。単位内の細粒度作業を自文脈で処理 | 設計の最終確定・ユーザー承認・スコープ外変更 |
| L2 サブサブ | `Explore`(read-only) / `Task Executor`(nested) / low〜mid | 重い読解の隔離・独立並列調査・大きな独立サブ単位の実装 | スコープ外変更 |

### 運用原則

- **粗粒度委譲（最重要）**: オーケストレーターは **Phase / 計画タスク = 1委譲単位** で `Task Executor` に渡す。個別ファイル読解や数行修正を単発委譲しない（往復・要約コストを回避）。
- **階層的細分化**: 単位内の細粒度判断・サブサブ起動は executor が担う。executor はコスト/応答性/品質を勘案し、必要時のみ L2 を起動する（基準は `.github/agents/task-executor.agent.md`）。
- **文脈遮断（最重要）**: 各層は中間生成物（ファイル全文・grep 生ログ・試行錯誤）を上位へ返さない。返却は次の4点に限定する。
  1. 結論サマリ
  2. 変更したファイルと行（実装委譲時）
  3. 上位の次判断に必要な最小事実
  4. 未解決事項・要確認点
- **委譲仕様の品質**: 委譲時は Goal / 前提（必要分のみ）/ Scope / Tasks / Constraints / Verify / Return format / Model tier / Thoroughness を明示する。
- **承認ゲートの保持**: 設計レポート提示とユーザー承認は L0 が保持する。executor は承認後のみ変更系を実行する（→ Report Delivery And Approval Gate 参照）。
- **直接実行の許容**: 単一ファイル参照・数行確認・小出力の自明作業、および read-only の単発 Q&A は L0 直接実行を許容する（委譲コスト > 便益を回避）。

## Subagent Delegation Rules

### 委譲の基本単位

オーケストレーター（L0）は **計画タスク（Phase 相当）を1単位** として `Task Executor` に委譲する。
executor が単位内の調査・実装・検証・サブサブ（L2）起動を自律管理する。L0 は個別の read/grep や
数行修正を単発委譲しない。

| 委譲パターン | 委譲先 | 委譲粒度 | 返却 |
|------|--------|--------|------|
| 実装を伴う計画タスク1単位 | `Task Executor` | Phase 全体 | 変更ファイル+行 / build・test 結果 / 要確認点 |
| 設計前の純調査（read-only、大規模） | `Task Executor` or `Explore` | 調査テーマ単位 | 構造化要約のみ |
| L0 自身の軽量確認 | （直接実行） | 単一ファイル・数行 | — |

> executor 内部の細粒度委譲（重い読解→`Explore`、独立サブ単位→nested `Task Executor`）の判断基準は
> `.github/agents/task-executor.agent.md` に定義する。

### Delegation Prompt Template

委譲は計画タスク単位。executor が単位内を自律処理する前提で記述する。

```
Goal: {この単位で達成する成果（Phase の目的）}
Context: {最小限の前提のみ}
Scope: {対象ファイル/ディレクトリと変更可否}
Tasks: {単位内に含む作業の列挙（細部は executor 裁量）}
Constraints: {Research only / approved edits only / スコープ外厳禁}
Verify: {ビルド・テストの実行と合否基準}
Return format: {結論・変更行・合否・要確認のみ。生ログ不可}
Model tier: {low|mid|high — per Model Selection}
Thoroughness: {quick|medium|thorough}
```

## Context Management

- フェーズ切り替え時に session memory に決定事項を書き出す
- 同じファイルを2回以上 read する場合は、前回の読み取り結果を参照する
- compaction を避けるため、大量のコード出力結果は即座に要約する
- subagent の返却は要約のみを会話に残し、raw 出力（ファイル全文・grep 生ログ・試行錯誤）はオーケストレーターへ伝搬させない（→ Orchestration Strategy 参照）

## Token Efficiency (Cache & Tools)

トークン消費とレイテンシは「プロンプトキャッシュ」と「ツール定義」の2コストが大半を占める。
この2点を意識して無駄なキャッシュ失効・ツールロードを避ける。
（参考: VS Code Blog "Improving token efficiency in GitHub Copilot", 2026-06-17）

### Prompt Cache を温存する

- 1セッション内でモデル・reasoning effort を途中変更しない（キャッシュ prefix が無効化される）。モデルは着手前に確定する（→ Model Selection 参照）
- 関連作業は連続して実行する。長時間の中断はキャッシュ失効を招く（数分〜最大1時間で破棄）
- 安定した内容（指示・ツール定義・リポジトリ文脈）を会話の冒頭から動かさない
  - Anthropic は明示的キャッシュ境界、OpenAI は自動 prefix キャッシュ。いずれも「冒頭の不変性」が効果の前提

### Tool 定義のオーバーヘッドを抑える

- 全ツールの常時ロードを前提にせず、必要なツールは tool_search で都度取得する（deferred tools）
- 一度ロードしたツールはセッション内で再検索しない
- MCP / 拡張ツールは実際に使うものだけ有効化し、常時ロード対象を最小化する
- 検索・コマンド実行・要約など狭いタスクは安価モデルのサブエージェントへ委譲する（→ Subagent Delegation / Model Selection 参照）

### Output Token を抑える

- 大量のツール出力・生データは即座に要約し、会話へ残さない（→ Context Management 参照）
- 取得済みの情報は再取得しない

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
- モデルは着手前に確定し、セッション途中で切り替えない（プロンプトキャッシュ失効を防ぐ。→ Token Efficiency 参照）
- オーケストレーター層は Opus 固定とする（→ Orchestration Strategy 参照）
- 上表の選択は **subagent 層**がタスク難易度に応じて行う。オーケストレーターは委譲時に推奨 tier を明示する

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

- `.github/skills/codebase-analysis/SKILL.md` — コード構造分析の手順・委譲テンプレート
- `.github/skills/refactoring/SKILL.md` — リファクタリングの Phase 管理・一括置換

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
