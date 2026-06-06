# Build Instructions

## Prerequisites

| ツール | バージョン | 備考 |
|---|---|---|
| Qt | 6.11 以上 | MSVC 2022 64-bit キットをインストール。Ninja もここに含まれる |
| CMake | 3.19 以上 | |
| Visual Studio / MSVC | 2022 (v17) | C++17 対応コンパイラ |
| Git | — | サブモジュールの取得に必要 |

> **Note:** qmake (`TableEngine.pro`) によるビルドは CMake 移行完了に伴い非サポートとなりました。

---

## 依存関係の管理

| ライブラリ | 取得方法 | 場所 |
|---|---|---|
| libarchive | vcpkg (自動) | `out/build/<preset>/vcpkg_installed/` |
| gtest / gmock | vcpkg (自動) | 同上 |
| QHexView | git submodule (v5.1.0) | `support/QHexView/` |
| valijson | git submodule (v1.1.0) | `support/valijson/` |
| ICU | Qt 付属 | Qt インストール先 |

vcpkg のトリプレットは `x64-windows-static-md`（`/MD` スタティックリンク）を使用します。

---

## Build with CMake (推奨)

### 1. リポジトリのクローンとサブモジュールの初期化

```powershell
git clone <repo-url> TableEngine
cd TableEngine
git submodule update --init --recursive
```

### 2. vcpkg のブートストラップ（初回のみ）

```powershell
support\vcpkg\bootstrap-vcpkg.bat -disableMetrics
```

### 3. CMake プリセットで構成する

Qt の MSVC2022 64-bit キットの環境（`vcvars64.bat` + Qt bin）が PATH に入った状態で実行します。  
`scripts/package-release.ps1` を使うと自動的に環境設定されます。

```powershell
cmake --preset Qt-MSVC2022-amd64-Ninja
```

- 初回 configure 時に vcpkg が `libarchive` と `gtest` をビルドします（数分〜十数分かかります）
- 2 回目以降はキャッシュが使われるため数秒で完了します
- 出力先: `out/build/Qt-MSVC2022-amd64-Ninja/`

### 4. ビルドする

```powershell
# Debug ビルド
cmake --build out/build/Qt-MSVC2022-amd64-Ninja --config Debug

# Release ビルド
cmake --build out/build/Qt-MSVC2022-amd64-Ninja --config Release
```

`Ninja Multi-Config` ジェネレータを使用しているため、Debug/Release を同一ビルドディレクトリで共存できます。

### 5. インストール・パッケージ作成

```powershell
# exe + Qt ランタイムをステージングディレクトリに収集して ZIP 作成
.\scripts\package-release.ps1
```

スクリプトのオプション：

| パラメータ | デフォルト | 説明 |
|---|---|---|
| `-QtDir` | `C:\Qt\6.11.1\msvc2022_64` | Qt MSVC64 キットのルート |
| `-SkipConfigure` | (なし) | configure をスキップしてビルドのみ実行 |

---

## VS Code での開発

1. CMake Tools 拡張をインストール
2. コマンドパレット → **CMake: Select Configure Preset** → `Qt MSVC2022 64bit - Ninja Multi-Config`
3. Kit を `Qt 6.11.1 MSVC2022 64bit` に設定（Qt の ninja.exe と vcvarsall が自動適用される）
4. **CMake: Configure** を実行

`cmake.useCMakePresets: always` が `.vscode/settings.json` に設定されているため、プリセットが自動的に読み込まれます。

---

## Running Tests

```powershell
ctest --preset Qt-MSVC2022-amd64-Ninja-debug
```

---

## 出力物

| ファイル | 場所 |
|---|---|
| `TableEngine.exe` | `out/build/Qt-MSVC2022-amd64-Ninja/main/Debug/` または `Release/` |
| `tablelibs.lib` | `out/build/Qt-MSVC2022-amd64-Ninja/src/Debug/` または `Release/` |
| ZIP パッケージ | `out/build/Qt-MSVC2022-amd64-Ninja/TableEngine-*.zip` |

---

## Notes

- `libarchive` と `gtest` は vcpkg によりスタティックリンクされるため、DLL の同梱は不要です
- Linux 等への移植時は `src/platform/windows/` の代替実装が必要です（詳細: [11_platform.md](11_platform.md)）
