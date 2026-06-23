# Codebase Analysis — C/C++ フラグメント

## 型定義の呼称

| 概念 | C/C++ の呼称 |
|------|------------|
| クラス | `class`（メンバー変数・メンバー関数） |
| 構造体 | `struct` |
| テンプレート | `template`（型パラメータ） |
| 型定義 | `typedef` / `using` |
| 抽象基底 | 純粋仮想関数を持つ `class` |

## 依存解析コマンド

```powershell
# CMake プリセットでのターゲット依存グラフ生成
# プリセット名は CMakePresets.json の configurePresets[*].name
cmake --preset Qt-MSVC2022-amd64-Ninja --graphviz=deps.dot .

# include 関係の grep・検索スコープ: 必ず TableEngine/** を指定
grep -rn '#include' TableEngine/src/
```

## 検索スコープの必須ルール

- `grep_search` / Explore の `includePattern` には必ず `TableEngine/**` を指定する
- `src/`（ワークスペースルート直下）は Qt6 ソース。**絶対に検索・分析対象に含めない**

## Qt6 固有の注意点

- `Q_OBJECT` マクロを持つクラスは MOC（Meta-Object Compiler）生成ファイルが存在する
- シグナル/スロットの呼び出し元は `connect(sender, &Class::signal, receiver, &Class::slot)` 構文で追う
- `Q_PROPERTY` 宣言は `READ`/`WRITE`/`NOTIFY` アクセサーを持つ（抱える主なメンバー関数を分析対象に含める）
- ヘッダ（`*.h`）と実装（`*.cpp`）を対で分析する
- C 単独案件もこのフラグメントを使う（class/template は読み替え）
