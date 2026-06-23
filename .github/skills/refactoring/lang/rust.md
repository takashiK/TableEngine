# Refactoring — Rust フラグメント

## ビルドコマンド

| 用途 | コマンド |
|------|---------|
| 中間/全体ビルド | `cargo build` |
| 型チェックのみ（高速） | `cargo check` |
| テスト | `cargo test` |
| Lint | `cargo clippy` |

## フォーマット / use 整理

```bash
cargo fmt
# 未使用 use の検出
cargo fix --allow-dirty --allow-staged
```

## use 一括追加（PowerShell 例）

```powershell
foreach ($f in $files) {
    $content = Get-Content $f -Raw
    $content = $content -replace '(use crate::base;)',
        "`$1`nuse crate::newmod;"
    Set-Content $f $content -NoNewline
}
```

## 注意点

- `mod` 宣言と `pub` 可視性を併せて更新する
- 未使用 `use` は警告（`deny` 設定ではエラー）になるため除去
- `impl` ブロックの移動はトレイト境界を確認する
