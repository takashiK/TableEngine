# TeMarkupLoader

## Overview

このドキュメントは旧実装の補足です。  
現行実装（`src/viewer/document/`）には `TeMarkupLoader` クラスは存在しません。

現在のマークアップ表示は `TeDocViewer::markupMode()` に統合されています。  
拡張子に応じて `QTextEdit` の表示 API を切り替えます。

---

## Current Behavior (TeDocViewer)

| 拡張子 | 表示処理 |
|---|---|
| `html` / `htm` | `QTextEdit::setHtml()` |
| `md` | `QTextEdit::setMarkdown()` |
| その他 | `QTextEdit::setPlainText()` |

---

## Notes

- 旧バージョンで使われていたコンテナ HTML / WebChannel ベースの仕組みは廃止されています
- 現在は `TeDocument` + `TeDocViewer` のみでマークアップ表示を完結します
- 詳細は [TeDocument.md](TeDocument.md) と [../13_viewer.md](../13_viewer.md) を参照してください
