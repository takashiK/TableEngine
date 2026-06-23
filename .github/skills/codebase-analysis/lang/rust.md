# Codebase Analysis — Rust フラグメント

## 型定義の呼称

| 概念 | Rust の呼称 |
|------|-----------|
| 構造体 | `struct`（フィールド一覧） |
| トレイト | `trait`（メソッド一覧） |
| 列挙型 | `enum`（バリアント一覧） |
| 実装 | `impl`（型へのメソッド付与） |
| 型エイリアス | `type X = Y` |

## 依存解析コマンド

```bash
# 依存ツリー
cargo tree

# モジュール構造
cargo modules structure   # cargo-modules 拡張が必要

# 公開 API 一覧
cargo public-api          # cargo-public-api 拡張が必要
```

## 注意点

- `mod` 階層と `pub` 可視性を確認する
- `impl Trait for Type` の対応関係を整理する
- feature flag による条件コンパイルに注意する
