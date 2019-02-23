/****************************************************************************
**
** Copyright (C) 2018 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
** This file is part of the Table Engine.
**
** $QT_BEGIN_LICENSE:GPL$
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public license version 3 or any later version.
** The licenses are as published by the Free Software Foundation and
** appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#pragma once

#include <QObject>

class TeTypes : public QObject
{
	Q_GADGET
public:
	enum WidgetType {
		WT_NONE,
		WT_MAINWINDOW,
		WT_MENU,
		WT_TOOLBAR,
		WT_STATUSBAR,
		WT_TAB,
		WT_FOLDERVIEW,
		WT_TREEVIEW,
		WT_LISTVIEW,
	};
	Q_ENUM(WidgetType)

	enum CmdId {
		CMDID_NONE,
		CMDID_SYSTEM         = 0x00000000,
		CMDID_SYSTEM_FILE    = CMDID_SYSTEM + 0x1000,     // メニュー : ファイル
		CMDID_SYSTEM_FILE_OPEN,                           // ファイルを開く
		CMDID_SYSTEM_FILE_COPY_TO,                        // ★フォルダへコピー
		CMDID_SYSTEM_FILE_MOVE_TO,                        // ★フォルダへ移動
		CMDID_SYSTEM_FILE_DELETE,                         // ★削除
		CMDID_SYSTEM_FILE_CREATE_SHORTCUT,                // ショートカットを作成
		CMDID_SYSTEM_FILE_RENAME,                         // 名前変更
		CMDID_SYSTEM_FILE_CHANGE_ATTRIBUTE,               // 属性変更
		CMDID_SYSTEM_FILE_RENAME_MULTI,                   // 一括名前変更
		CMDID_SYSTEM_FILE_DIVIDE,                         // ファイル分割
		CMDID_SYSTEM_FILE_COMBINE,                        // ファイル結合
		CMDID_SYSTEM_FILE_ARCHIVE,                        // ファイル圧縮
		CMDID_SYSTEM_FILE_EXTRACT,                        // ファイル展開
		CMDID_SYSTEM_FILE_RUN_WITH_COMMAND,               // コマンドを指定して実行
		CMDID_SYSTEM_FILE_EXIT,                           // TableEngine終了

		CMDID_SYSTEM_EDIT    = CMDID_SYSTEM + 0x2000,     // メニュー : 編集
		CMDID_SYSTEM_EDIT_CUT,                            // 切り取り
		CMDID_SYSTEM_EDIT_COPY,                           // コピー
		CMDID_SYSTEM_EDIT_PASTE,                          // 貼り付け
		CMDID_SYSTEM_EDIT_PASTE_AS_SHORTCUT,              // ショートカットを貼り付け
		CMDID_SYSTEM_EDIT_SELECT_ALL,                     // ★全選択/解除
		CMDID_SYSTEM_EDIT_SELECT_TOGGLE,                  // ★選択反転
		CMDID_SYSTEM_EDIT_SELECT_BY_NAME,                 // 名前を指定して選択

		CMDID_SYSTEM_FOLDER  = CMDID_SYSTEM + 0x3000,     // メニュー : フォルダ
		CMDID_SYSTEM_FOLDER_OPEN_ONE,                     // ★選択フォルダを展開
		CMDID_SYSTEM_FOLDER_OPEN_UNDER,                   // ★選択フォルダ以下を展開
		CMDID_SYSTEM_FOLDER_OPEN_ALL,                     // ★全フォルダを展開
		CMDID_SYSTEM_FOLDER_CLOSE_UNDER,                  // ★全フォルダを閉じる
		CMDID_SYSTEM_FOLDER_CLOSE_ALL,                    // ★全フォルダを閉じる
		CMDID_SYSTEM_FOLDER_CREATE_FOLDER,                // ★フォルダ作成
		CMDID_SYSTEM_FOLDER_ADD_FAVORITES,                // お気に入りフォルダ追加
		CMDID_SYSTEM_FOLDER_DEL_FAVORITES,                // お気に入りフォルダ削除
		CMDID_SYSTEM_FOLDER_PREV_FOLDER,                  // 前のフォルダに戻る
		CMDID_SYSTEM_FOLDER_NEXT_FOLDER,                  // 次のフォルダに進む
		CMDID_SYSTEM_FOLDER_GOTO_ROOT,                    // ルートフォルダに移動
		CMDID_SYSTEM_FOLDER_GOTO_FAVORITE,                // お気に入りフォルダに移動
		CMDID_SYSTEM_FOLDER_GOTO_HISTORY,                 // 履歴で移動
		CMDID_SYSTEM_FOLDER_GOTO_FOLDER,                  // 指定フォルダに移動
		CMDID_SYSTEM_FOLDER_FIND,                         // ファイル検索

		CMDID_SYSTEM_VIEW    = CMDID_SYSTEM + 0x4000,     // メニュー : 表示


		CMDID_SYSTEM_TOOL    = CMDID_SYSTEM + 0x5000,     // メニュー : 外部ツール

		CMDID_SYSTEM_WINDOW  = CMDID_SYSTEM + 0x6000,     // メニュー : ウィンドウ

		CMDID_SYSTEM_SETTING = CMDID_SYSTEM + 0x7000,     // メニュー : 設定
		CMDID_SYSTEM_SETTING_OPTION,                      // 全体設定
		CMDID_SYSTEM_SETTING_KEY,                         // キー設定
		CMDID_SYSTEM_SETTING_TOOLBAR,                     // ツールバー設定
		CMDID_SYSTEM_SETTING_MENU,                        // ポップアップ&メニュー設定
		CMDID_SYSTEM_SETTING_STYLE,                       // スタイル設定
		CMDID_SYSTEM_SETTING_USER_COMMAND,                // ユーザーコマンド登録

		CMDID_SYSTEM_HELP    = CMDID_SYSTEM + 0x8000,     // メニュー : ヘルプ
		CMDID_SYSTEM_HELP_SHOW,
		CMDID_SYSTEM_HELP_VERSION,
		
		CMDID_USER        = 0xD000,                       // ユーザー登録コマンド

		CMDID_SPECIAL     = 0xE000,                       // 特殊用途ID
		CMDID_SPECIAL_FOLDER,                             // Menu内のフォルダ
		CMDID_SPECIAL_SEPARATOR,                          // Menu内セパレータ

		CMDID_MASK_TYPE   = 0xF000,
	};

	Q_ENUM(CmdId)

	TeTypes();
	~TeTypes();
};

