---
name: refactoring
description: パッケージ/モジュールのリファクタリング（分割・統合・メンバー移動・API変更）を Phase 管理と中間ビルド検証・一括置換で安全かつ効率的に実行する。言語判定後に lang/ フラグメントをロードして言語固有のビルド・テストコマンドを取得する。
---

# Refactoring Skill

## Purpose

パッケージ/モジュールのリファクタリング（分割・統合・API 変更）を安全かつ効率的に実行する。
中間ビルド検証と一括置換により、手戻りとトークン消費を最小化する。

## When to Use

- モジュール分割（1モジュール → base + extensions）
- インターフェース/型変更（メンバー追加/削除/移動）
- API シグネチャ変更に伴う利用側の一括更新

## Language Detection

手順実行前に対象言語を判定し、該当する `lang/` フラグメントを **1つだけ** read_file する。

| 検出ファイル | 言語ID | フラグメント |
|------------|--------|------------|
| go.mod | go | `lang/go.md` |
| Cargo.toml | rust | `lang/rust.md` |
| CMakeLists.txt / *.cpp / *.hpp | cpp | `lang/cpp.md` |
| *.c / *.h（単独） | c | `lang/cpp.md`（C/C++ 共用） |

- 複数該当時はユーザーに確認する
- フラグメントには「ビルド」「フォーマット」「import 整理」コマンドのみ記載

## Procedure

### Phase 1: 影響範囲特定

Explore subagent で以下を取得:
```
Research only — do NOT modify files.
Target: all source files in repository
Task: Find all call sites of {members to move/change}
Return format: JSON array of {file, line, current_code, module}
Thoroughness: thorough
```

### Phase 2: 新コード作成

1. 新モジュール/ディレクトリ作成
2. 移動対象のコードを新ファイルとして作成（create_file）
3. **中間ビルド**: 言語フラグメントの「中間ビルド」コマンドで新モジュール単体の整合性確認

### Phase 3: 旧コード修正

1. インターフェース/宣言からメンバー削除
2. 実装からメンバー本体削除
3. **中間ビルド**: 言語フラグメント参照（利用側エラーは想定内）

### Phase 4: 利用側一括更新

**3ファイル以上の同一パターン置換は PowerShell を使う:**

```powershell
$files = @('path1', 'path2', ...)
foreach ($f in $files) {
    $content = Get-Content $f -Raw
    $content = $content -replace 'OldPattern\(', 'newmod.NewPattern(obj, '
    Set-Content $f $content -NoNewline
}
```

**import/include 追加も PowerShell で一括（構文は言語フラグメント参照）。**

### Phase 5: 最終検証

言語フラグメントの「全体ビルド」コマンドを実行する。
ビルド成功後、フォーマッタで import/include を整理する。

## API Migration Patterns

| 旧パターン | 新パターン | 正規表現 |
|-----------|-----------|---------|
| `obj.Member(args)` | `mod.Member(obj, args)` | `obj\.Member\(` → `mod.Member(obj, ` |
| `obj.Member(a, b)` | `mod.Member(obj, a, b)` | 同上 |
| コメント内の参照 | そのまま置換 | `//.*obj\.Member` も対象に含める |

## Decision Points

| 判断ポイント | 基準 | 選択 |
|------------|------|------|
| メンバー → 自由関数 | 内部状態不要 | モジュールレベル関数 |
| メンバー → メソッド | 内部状態必要 | 新型にメソッド定義 |
| 型の移動先 | 利用箇所の多さ | 利用が多い側に配置 |
| ブリッジIF設計 | 拡張が内部操作必要か | 委譲 + Operator パターン |

## Anti-patterns

| やってはいけないこと | 理由 | 代替 |
|-------------------|------|------|
| 1ファイルずつ置換ツール実行 | 20ファイルで20ターン消費 | PowerShell 一括 |
| ビルド検証なしで全更新 | 最後にエラー多発で手戻り | Phase 2/3 で中間ビルド |
| import/include を手動追加 | ミスしやすい | regex 一括 or フォーマッタ |
| 未使用 import/include を残す | ビルドエラー/警告 | 置換時に不要分も削除 |
| 複数言語フラグメントを同時ロード | トークン浪費 | 判定した1言語のみ read |
