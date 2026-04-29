# Overview

## Background

TableEngine は、MS-DOS 時代のファイラー **FD** および Windows 向けの **卓駆☆** の後継として開発されたオープンソースのファイルエクスプローラです。  
これらのソフトウェアは Windows 10 以降で動作しなくなったため、作者が同等の操作感を持ちつつ、いくつかの新機能を加えた代替ソフトウェアを Qt を用いて自ら開発したものです。

## Purpose

- FD / 卓駆☆ と同様のキーボード中心の操作感をモダンな Windows 環境で再現する
- アーカイブ（ZIP / 7zip / tar 等）の内容をファイルシステムと同様に閲覧・操作できるようにする
- テキスト・画像・バイナリのビューワを内蔵し、外部ツールに依存しない閲覧体験を提供する
- 将来的には Windows 以外のプラットフォームへの対応も視野に入れた拡張性を持つ

## Features

| 機能 | 説明 |
|---|---|
| デュアルペインビュー | 左右に独立したフォルダビューをタブで管理する |
| アーカイブ内閲覧 | ZIP / 7zip / tar 等のアーカイブをディレクトリのように展開して閲覧できる |
| ファイル操作 | コピー / 移動 / 削除 / リネーム / 属性変更 / 分割・結合 / アーカイブ化 / 展開 |
| ファイル検索 | ファイル名・サイズ・更新日時による条件検索。結果は専用ビューに表示 |
| 内蔵ビューワ | テキスト（マークアップ対応）/ 画像 / バイナリの閲覧機能 |
| キー割り当てカスタマイズ | キーボードショートカットを自由に設定できる |
| お気に入り / 履歴 | よく使うフォルダへのクイックアクセスと移動履歴の管理 |

## License

TableEngine は **GNU General Public License v2 (GPLv2)** の下に公開されています。

## Dependencies

| ライブラリ | 用途 |
|---|---|
| Qt 6.x (Core / Gui / Widgets / WebChannel / WebEngineCore / WebEngineWidgets / Core5Compat / Positioning) | UI フレームワーク全般 |
| Microsoft Visual C++ 2022 | Windows 向けコンパイラ |
| Google Mock | ユニットテスト（テストコードのみ） |

## Author

Takashi Kuwabara (laffile@gmail.com)
