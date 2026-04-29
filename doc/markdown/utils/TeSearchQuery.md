# TeSearchQuery

## Overview

`TeSearchQuery` はファイル検索条件を表す構造体です。  
`TeFindDialog` でユーザーが入力した条件を `fromDialog()` で変換して生成し、  
`TeFinder::startSearch()` に渡して使用します。

すべての有効なフィルタは **AND 結合** で評価されます。

---

## Fields

| フィールド | 型 | デフォルト | 説明 |
|---|---|---|---|
| `targetPath` | `QString` | （空） | 検索対象のルートパス。空の場合は `isValid()` が `false` を返す |
| `recursive` | `bool` | `true` | サブディレクトリを再帰的に検索するかどうか |
| `hasNameFilter` | `bool` | `false` | ファイル名フィルタを有効にするかどうか |
| `namePattern` | `QRegularExpression` | — | ファイル名のパターン（正規表現） |
| `hasSizeFilter` | `bool` | `false` | ファイルサイズフィルタを有効にするかどうか |
| `sizeThreshold` | `qsizetype` | `0` | サイズの閾値（バイト単位） |
| `sizeLessThan` | `bool` | `true` | `true`: size ≦ threshold、`false`: size ≧ threshold |
| `hasDateFilter` | `bool` | `false` | 更新日時フィルタを有効にするかどうか |
| `dateFrom` | `QDateTime` | — | 更新日時の開始（inclusive） |
| `dateTo` | `QDateTime` | — | 更新日時の終了（inclusive） |

---

## Filter Logic

```
entry matches = true
if hasNameFilter    → entry matches &= (namePattern matches entry.name)
if hasSizeFilter    → entry matches &= (sizeLessThan ? size <= threshold : size >= threshold)
if hasDateFilter    → entry matches &= (dateFrom <= lastModified <= dateTo)
```

---

## fromDialog()

`TeFindDialog` の入力状態を `TeSearchQuery` に変換する静的ファクトリメソッドです。

| 変換内容 | 詳細 |
|---|---|
| ワイルドカード → 正規表現 | `*.txt` → `QRegularExpression::fromWildcard()` で変換 |
| サイズ単位変換 | KB 指定 → × 1024、MB 指定 → × 1024 × 1024 でバイト値に変換 |
| 日付範囲変換 | `QDate` → `QDateTime`。`dateFrom` は当日の開始（00:00:00）、`dateTo` は当日の終了（23:59:59）に拡張 |

---

## isValid()

`targetPath` が空でない場合に `true` を返します。  
`TeFinder::startSearch()` の前に呼び出してクエリが有効かを確認してください。
