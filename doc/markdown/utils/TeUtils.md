# TeUtils

## Overview

`TeUtils.h` はアプリケーション全体で使用する **フリー関数** とファイル種別を表す `TeFileType` 列挙型を提供します。  
特定のクラスに属さない横断的なユーティリティをまとめたヘッダです。

---

## TeFileType Enum

ファイルの内容種別を表す列挙型です。拡張子ヒューリスティックによって分類されます。

| 値 | 意味 |
|---|---|
| `TE_FILE_UNKNOWN` | 種別不明 |
| `TE_FILE_FOLDER` | ディレクトリ |
| `TE_FILE_TEXT` | プレーンテキストファイル |
| `TE_FILE_IMAGE` | サポート画像ファイル |
| `TE_FILE_ARCHIVE` | サポートアーカイブファイル |

---

## Functions

### 選択・カレントアイテム取得

| 関数 | 説明 |
|---|---|
| `getSelectedItemList(store, paths)` | アクティブなリストビューで選択中のアイテムパスを `paths` に収集する。1件以上あれば `true` を返す |
| `getCurrentItem(store)` | アクティブなリストビューのカレントアイテムパスを返す。なければ空文字 |
| `getCurrentFolder(store)` | アクティブなリストビューが表示しているディレクトリパスを返す |

これらの関数はコマンド実装から多用されます。コマンドクラスが直接 `TeViewStore` のウィジェット構造に依存しないよう分離されています。

### お気に入り操作

| 関数 | 説明 |
|---|---|
| `getFavorites()` | `QSettings` からお気に入りパスリストを取得する |
| `updateFavorites(list)` | `QSettings` のお気に入りパスリストを `list` で上書きする |

### ファイル種別判定

| 関数 | 説明 |
|---|---|
| `getFileType(path)` | パスの拡張子から `TeFileType` を返す |

### その他

| 関数 / マクロ | 説明 |
|---|---|
| `isDir(index)` | モデルインデックスがディレクトリを指しているか判定する |
| `detectTextCodec(data, codecList)` | バイトバッファのエンコーディングを `codecList` の順に探索して最初に一致したコーデック名を返す |
| `NOT_USED(x)` | 未使用変数の警告を抑制するマクロ |

---

## detectTextCodec()

```cpp
QString detectTextCodec(const QByteArray& data, const QStringList& codecList);
```

`TeDetailTextPreviewSection` のテキストプレビューでエンコーディング自動検出に使用されます。  
`codecList` の順に `QStringConverter` / BOM 判定を試み、最初に成功したコーデック名を返します。  
すべて失敗した場合は空文字を返します。

---

## See Also

- [`TeFavorites`](TeFavorites.md)
- [`TeDetailTextPreviewSection`](../widgets/TeDetailSection.md)
