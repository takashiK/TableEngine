# Build Instructions

## Prerequisites

| ツール | バージョン | 備考 |
|---|---|---|
| Qt | 6.x (推奨: 6.10.2) | Core / Gui / Widgets / WebChannel / WebEngineCore / WebEngineWidgets / Core5Compat / Positioning が必要 |
| CMake | 3.19 以上 | 推奨ビルドシステム |
| Visual Studio / MSVC | 2022 (v17) | Windows 向け。C++17 対応コンパイラが必要 |
| Google Mock | — | ユニットテストのみ。`support_package/` に含まれる |

> **Note:** qmake による旧来のビルドも `TableEngine.pro` で可能ですが、現在は CMake が推奨です。

---

## Build with CMake (推奨)

### 1. Qt の環境変数を設定する

Qt の CMake パッケージが検索できるよう、`Qt6_DIR` または `CMAKE_PREFIX_PATH` を設定します。

```powershell
# 例: Qt 6.10.2 MSVC 2022 64bit の場合
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.10.2\msvc2022_64"
```

### 2. CMake プリセットで構成する

リポジトリルートで以下を実行します。`CMakePresets.json` に定義された  
**Visual Studio Community 2022 Release - amd64** プリセットを使用します。

```powershell
cmake --preset "Visual Studio Community 2022 Release - amd64"
```

生成されたファイルは `out/build/Visual Studio Community 2022 Release - amd64/` に出力されます。

### 3. ビルドする

```powershell
# Debug ビルド
cmake --build --preset "Visual Studio Community 2022 Release - amd64-debug"

# または Visual Studio ソリューション (out/build/.../TableEngine.sln) を直接開いてビルド
```

### 4. 出力物

| ファイル | 場所 |
|---|---|
| `TableEngine.exe` | `out/build/<preset>/main/Debug/` または `Release/` |
| `tablelibs.lib` | `out/build/<preset>/src/Debug/` または `Release/` |
| `archive.dll` / `libmagic.dll` | exe と同ディレクトリ（ビルド後自動コピー） |

---

## Build with qmake (レガシー)

```powershell
# VisualStudio プロジェクトファイルを生成
qmake -tp vc -recursive

# 生成された TableEngine.sln を Visual Studio で開いてビルド
```

---

## Running Tests

### ユニットテスト (tests/)

```powershell
cd out/build/<preset>
ctest --build-config Debug -R "tests"
```

### ビューワテスト (viewer_tests/)

```powershell
ctest --build-config Debug -R "viewer_tests"
```

---

## Notes

- `support_package/` 以下の `archive.dll` / `libmagic.dll` は Windows 向けビルド時に自動的に exe と同じディレクトリへコピーされます（CMake の `add_custom_command` で設定済み）。
- Linux 等への移植時は `platform/windows/` の代替実装が必要です（詳細: [11_platform.md](11_platform.md)）。
