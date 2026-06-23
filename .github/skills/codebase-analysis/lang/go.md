# Codebase Analysis — Go フラグメント

## 型定義の呼称

| 概念 | Go の呼称 |
|------|----------|
| 構造体 | `struct`（フィールド一覧） |
| インターフェース | `interface`（メソッド一覧） |
| 型エイリアス | `type X = Y` |
| メンバー | フィールド / メソッド |

## 依存解析コマンド

```bash
# モジュール依存グラフ
go mod graph

# パッケージ間 import 関係
go list -deps ./...

# 特定パッケージの import 元
go list -f '{{.ImportPath}}: {{.Imports}}' ./...
```

## 注意点

- import グループは stdlib / internal / external の3区分
- 公開シンボルは大文字始まり
