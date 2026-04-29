# Dispatcher / Command Pattern

## Overview

TableEngine は **Command パターン** と **ディスパッチャ** を組み合わせて、ユーザー操作とビジネスロジックを分離しています。  
ウィジェットはユーザーの入力を受け取りますが、何をするかは知りません。  
すべての操作は `CmdId` で識別されるコマンドオブジェクトに委譲されます。

---

## Class Relationships

```mermaid
classDiagram
    class TeDispatchable {
        <<interface>>
        +getType() WidgetType
        +dispatch(type, event) bool
        +execCommand(cmdId, type, event, param)
    }
    class TeDispatcher {
        +dispatch(type, event) bool
        +execCommand(cmdId, type, event, param)
        +loadKeySetting()
        +requestCommandFinalize(cmd)
        -m_keyCmdMap: QHash
        -m_cmdQueue: QList~TeCommandBase*~
        -mp_factory: TeCommandFactory*
        -mp_store: TeViewStore*
    }
    class TeCommandFactory {
        <<singleton>>
        +factory() TeCommandFactory*
        +createCommand(cmdId) TeCommandBase*
        +commandInfo(cmdId) TeCommandInfoBase*
        +commandGroup(groupId) List
        +menuGroup(groupId) List
    }
    class TeCommandInfoBase {
        <<abstract>>
        +isActive(store) bool
        +isSelected(store) bool
        +createCommand() TeCommandBase*
        +cmdId() CmdId
        +name() QString
    }
    class TeCommandBase {
        <<abstract>>
        +run(store)
        #execute(store) bool
        #finished()
        #srcType() WidgetType
        #cmdParam() TeCmdParam*
    }
    class TeFolderView {
        +dispatch(type, event) bool
        +execCommand(...)
    }
    class TeViewStore {
        +dispatch(type, event) bool
        +execCommand(...)
    }

    TeDispatchable <|.. TeDispatcher
    TeDispatchable <|.. TeFolderView
    TeDispatchable <|.. TeViewStore
    TeDispatcher --> TeCommandFactory
    TeCommandFactory --> TeCommandInfoBase
    TeCommandInfoBase --> TeCommandBase : creates
```

---

## Event Dispatch Flow

ユーザー操作からコマンド実行・終了までの流れを示します。

```mermaid
sequenceDiagram
    participant Widget as Widget (TeFolderView 等)
    participant Filter as TeEventFilter
    participant Dispatcher as TeDispatcher
    participant Factory as TeCommandFactory
    participant Cmd as TeCommandBase 派生クラス
    participant Store as TeViewStore

    Widget->>Filter: キーイベント / マウスイベント
    Filter->>Widget: dispatch(WidgetType, QEvent*)
    Widget->>Dispatcher: dispatch(WidgetType, QEvent*)
    Dispatcher->>Dispatcher: keyCmdMap でCmdId を検索
    Dispatcher-->>Dispatcher: emit requestCommand(cmdId, ...)
    Dispatcher->>Factory: createCommand(cmdId)
    Factory-->>Dispatcher: TeCommandBase* (新規インスタンス)
    Dispatcher->>Dispatcher: cmdQueue に追加
    Dispatcher->>Cmd: run(TeViewStore*)
    Cmd->>Cmd: execute(store) を呼ぶ
    Cmd->>Store: UI状態参照 / 変更
    alt 同期コマンド (execute が true を返す)
        Cmd->>Dispatcher: finished() → requestCommandFinalize()
        Dispatcher->>Dispatcher: cmdQueue から削除 / delete
    else 非同期コマンド (execute が false を返す)
        Note over Cmd: 処理完了後に finished() を呼ぶ
        Cmd-->>Dispatcher: finished() → commandFinalize シグナル
        Dispatcher->>Dispatcher: finishCommand() で delete
    end
```

---

## TeDispatchable Interface

`TeDispatchable` は `TeDispatcher` / `TeFolderView` / `TeViewStore` が実装するインタフェースです。

| メソッド | 説明 |
|---|---|
| `getType()` | このオブジェクトの `WidgetType` を返す |
| `dispatch(type, event)` | イベントを受け取り、必要に応じてコマンドに変換する。`true` を返すとイベントを消費済みとする |
| `execCommand(cmdId, type, event, param)` | コマンド ID を受け取って実行する |

---

## TeDispatcher

`TeDispatcher` はキーイベントを `CmdId` に変換し、コマンドオブジェクトを生成・実行します。

### キーマップ

`loadKeySetting()` が `QSettings` からキー割り当てを読み込み、  
`m_keyCmdMap`（`QPair<modifier, key>` → `CmdId` のマップ）を構築します。  
現状は **キーボードイベントのみ** が `CmdId` にマップされます。

### コマンドキュー

コマンドは `m_cmdQueue` に積まれ、**先頭のコマンドのみが実行**されます。  
非同期コマンドが完了するまで、次のコマンドは実行待ちになります。  
これにより、重複操作やレース条件を防止します。

---

## TeCommandFactory

`TeCommandFactory` はシングルトンです。  
アプリケーション起動時にすべてのコマンドを `TeCommandInfoBase` として登録し、  
`CmdId` を渡すと対応するコマンドオブジェクトを生成して返します。

---

## TeCommandBase

すべてのコマンドの基底クラスです。  
サブクラスは `execute(TeViewStore*)` を実装するだけで機能します。

| メソッド | アクセス | 説明 |
|---|---|---|
| `run(store)` | public | `TeDispatcher` から呼ばれる。内部で `execute()` を呼ぶ |
| `execute(store)` | protected (純粋仮想) | コマンドの処理本体。**同期なら `true`、非同期なら `false` を返す** |
| `finished()` | protected | 非同期コマンドが処理完了時に呼ぶ。`TeDispatcher` にキュー削除を要求する |
| `srcType()` | protected | コマンドを起動した `WidgetType` を返す |
| `cmdParam()` | protected | コマンドパラメータ（`TeCmdParam`）を返す |

### 同期コマンドと非同期コマンド

| 種別 | `execute()` の戻り値 | 説明 |
|---|---|---|
| 同期コマンド | `true` | 実行後すぐに完了。`run()` から `finished()` が自動呼び出しされる |
| 非同期コマンド | `false` | 別スレッドや非同期処理を開始し、完了時に自分で `finished()` を呼ぶ |

---

## TeCmdParam

コマンドに追加情報を渡すための型です。`QMap<QString, QVariant>` の typedef です。  
各コマンドクラスが独自のキー定数（例: `TeCmdFolderChangeRoot::PARAM_ROOT_PATH`）を定義して使用します。
