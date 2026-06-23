# Refactoring — C/C++ フラグメント

## ビルドコマンド

| 用途 | コマンド |
|------|---------|
| 構成（初回） | `cmake --preset Qt-MSVC2022-amd64-Ninja` |
| 中間/全体ビルド | `cmake --build --preset Qt-MSVC2022-amd64-Ninja-debug` |
| 単一ターゲット | `cmake --build --preset Qt-MSVC2022-amd64-Ninja-debug --target {name}` |
| テスト | `ctest --preset Qt-MSVC2022-amd64-Ninja-debug` |

> プリセット名は `CMakePresets.json` の `buildPresets[*].name` を参照（**小文字** `-debug` / `-release`）。
> Visual Studio 環境（`vcvars64.bat`）+ Qt bin が PATH に入ったターミナルで実行すること。

### ローカル環境セットアップ（実地検証済み）

`scripts/setup-msvc-env.ps1` は `$env:GITHUB_ENV` へ書き込む **CI 専用**スクリプトで、
ローカルでは効果がない。ローカルでは vcvars64 をインラインで取り込む:

```powershell
# 1. MSVC 環境を現在のセッションへ取り込む（vswhere でインストール先を自動検出）
#    -products * を付けないと BuildTools 版が検出されないことに注意
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vsRoot = & $vswhere -products * -latest -property installationPath
& "$vsRoot\VC\Auxiliary\Build\vcvars64.bat"
# 2. Qt の DLL を解決するため Qt bin を PATH に追加
$env:PATH = "C:\Qt\6.11.1\msvc2022_64\bin;$env:PATH"
```

> **複数行コマンドの注意**: このターミナルでは複数行コマンドが化けて末尾しか実行されないことがある。
> 環境取り込み＋ビルドのような複合処理は一時 `.ps1` に書き出し、
> `pwsh -NoProfile -ExecutionPolicy Bypass -File <script>.ps1` で実行すると確実。

### 単体テストの直接実行（gtest）

ctest を介さず exe を直接起動するとフィルタ・高速反復が可能:

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'   # ヘッドレス実行に必須
$exe = "out/build/Qt-MSVC2022-amd64-Ninja/Debug/tst_tengine.exe"
& $exe                                       # 全テスト
& $exe --gtest_filter='tst_TeFileListView.*' # 特定スイートのみ
& $exe --gtest_filter='*key_shift_inline_search*' # 単一テスト
```

> exe はビルドルート直下の `Debug/` に出力される（`tests/Debug` ではない）。
> GUI を伴うウィジェットテストは `QT_QPA_PLATFORM=offscreen` を設定しないと
> 表示環境が無いと失敗する。

## フォーマット / include 整理

- **フォーマットは VS Code C/C++ 拡張に任せる**（保存時自動フォーマット）
- Agent から clang-format CLI を直接呼び出すことはしない
- include 整理はファイル保存で自動適用される

## include 一括追加（PowerShell 例）

```powershell
foreach ($f in $files) {
    $content = Get-Content $f -Raw
    $content = $content -replace '(#include "base.h")',
        "`$1`n#include `"newmod.h`""
    Set-Content $f $content -NoNewline
}
```

## Qt6 固有の注意点

- `Q_OBJECT` マクロを持つクラスの移動・リネーム時は MOC 再実行が必要（ビルドで自動実行）
- シグナル/スロットの変更は `connect()` 呼び出し側も必ず更新する
- ヘッダ（宣言）と実装（定義）を対で修正する
- インクルードガード / `#pragma once` を保持する
- 前方宣言で済む箇所は include を増やさない
- C 単独案件もこのフラグメントを使う
