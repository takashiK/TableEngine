# Refactoring — Go フラグメント

## ビルドコマンド

| 用途 | コマンド |
|------|---------|
| 中間ビルド（特定パッケージ） | `go build ./pkg/...` |
| 全体ビルド | `go build ./...` |
| テスト | `go test ./...` |
| Vet 検査 | `go vet ./...` |

## フォーマット / import 整理

```bash
goimports -w .
gofmt -w .
```

## import 一括追加（PowerShell 例）

```powershell
foreach ($f in $files) {
    $content = Get-Content $f -Raw
    $content = $content -replace '("github.com/org/module/pkg/base")',
        "`$1`n`t`"github.com/org/module/pkg/newpkg`""
    Set-Content $f $content -NoNewline
}
```

## 注意点

- 公開シンボルは大文字始まり
- 未使用 import はビルドエラーになるため必ず除去
