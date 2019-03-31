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
** Public license version 2 or any later version.
** The licenses are as published by the Free Software Foundation and
** appearing in the file LICENSE.GPL2
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "platform/platform_util.h"

#include <QMap>

#include <Windows.h>
#include <commctrl.h>
#include <Shlobj.h>
#include <QDir>
#include <QFileInfo>
#include <Shobjidl.h>


static QMap<HWND, IContextMenu2*> g_context;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITMENUPOPUP) {
		IContextMenu2* pContextMenu2 = g_context.find(hwnd).value();
		if (pContextMenu2 != NULL) {
			pContextMenu2->HandleMenuMsg(uMsg, wParam, lParam);
			return 0;
		}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void showFileContext(int px, int py, const QString& path)
{
	QString rpath = QDir::toNativeSeparators(path);

	static QString cname;
	if (cname.isEmpty()) {
		cname = u8"TableEngineWindowsContext";
		WNDCLASSEX wc;
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = 0;
		wc.lpfnWndProc = WindowProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = static_cast<HINSTANCE>(GetModuleHandle(0));
		wc.hCursor = 0;
		wc.hbrBackground = 0;
		wc.hIcon = 0;
		wc.hIconSm = 0;
		wc.lpszMenuName = 0;
		wc.lpszClassName = reinterpret_cast<LPCWSTR>(cname.utf16());
		RegisterClassEx(&wc);
	}

	HWND hwnd = CreateWindowEx(0, reinterpret_cast<LPCWSTR>(cname.utf16()),
		L"ContextWindow", WS_OVERLAPPED,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		HWND_MESSAGE, NULL, static_cast<HINSTANCE>(GetModuleHandle(0)), NULL);

	PIDLIST_ABSOLUTE pidlAbsolute = ILCreateFromPath(reinterpret_cast<LPCWSTR>(rpath.utf16()));

	int                 nId = 0;
	HRESULT             hr;
	POINT               pt;
	HMENU               hmenuPopup;
	IContextMenu        *pContextMenu = NULL;
	IShellFolder        *pShellFolder = NULL;
	PITEMID_CHILD       pidlChild;

	SHBindToParent(pidlAbsolute, IID_PPV_ARGS(&pShellFolder), NULL);
	pidlChild = ILFindLastID(pidlAbsolute);

	hr = pShellFolder->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST *)&pidlChild, IID_IContextMenu, NULL, (void **)&pContextMenu);
	IContextMenu2 *pContextMenu2 = NULL;
	if (hr == S_OK) {
		hr = pContextMenu->QueryInterface(IID_PPV_ARGS(&pContextMenu2));
		pContextMenu->Release();
		g_context.insert(hwnd, pContextMenu2);
	}

	if (hr == S_OK) {
		hmenuPopup = CreatePopupMenu();
		pContextMenu2->QueryContextMenu(hmenuPopup, 0, 1, 0x7fff, CMF_NORMAL);

		nId = TrackPopupMenu(hmenuPopup, TPM_RETURNCMD, px, py, 0, hwnd, NULL);
	}

	if (nId != 0) {
		CMINVOKECOMMANDINFO ici;
		ici.cbSize = sizeof(CMINVOKECOMMANDINFO);
		ici.fMask = 0;
		ici.hwnd = hwnd;
		ici.lpVerb = (LPCSTR)MAKEINTRESOURCE(nId - 1);
		ici.lpParameters = NULL;
		ici.lpDirectory = NULL;
		ici.nShow = SW_SHOW;

		hr = pContextMenu2->InvokeCommand(&ici);
	}

	if (pContextMenu2 != NULL) {
		g_context.erase(g_context.find(hwnd));
		pContextMenu2->Release();
		pContextMenu2 = NULL;
	}
	if (pShellFolder != NULL)pShellFolder->Release();

	DestroyWindow(hwnd);
	ILFree(pidlAbsolute);
}

void openFile(const QString& path)
{
	ShellExecute(NULL,L"open", reinterpret_cast<LPCWSTR>(path.utf16()), NULL, NULL, SW_SHOWNORMAL);
}

bool copyFiles(const QStringList & files, const QString & path)
{
	HRESULT hr = S_OK;
	IFileOperation *pfo;

	//
	// Create the IFileOperation interface 
	//
	hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo));

	if (SUCCEEDED(hr)) {
		//IFileOperationの挙動を決める
		//hr = pfo->SetOperationFlags(FOF_NO_UI);
		if (SUCCEEDED(hr)) {
			IShellItem *psiTo = NULL;

			//コピー先パスが指定されていれば採用
			if (!path.isEmpty()) {
				hr = SHCreateItemFromParsingName(reinterpret_cast<LPCWSTR>(QDir::toNativeSeparators(path).utf16()), NULL, IID_PPV_ARGS(&psiTo));
			}

			if (SUCCEEDED(hr)) {
				for ( const QString& from : files) {
					IShellItem *psiFrom = NULL;
					hr = SHCreateItemFromParsingName(reinterpret_cast<LPCWSTR>(QDir::toNativeSeparators(from).utf16()), NULL, IID_PPV_ARGS(&psiFrom));

					if (SUCCEEDED(hr)) {
						//コピー登録して、コピー元開放
						hr = pfo->CopyItem(psiFrom, psiTo, NULL, NULL);
						psiFrom->Release();
					}

					if (FAILED(hr)) {
						break;
					}
				}

				//コピー先開放
				if (NULL != psiTo) {
					psiTo->Release();
				}

				if (SUCCEEDED(hr)) {
					hr = pfo->PerformOperations();
				}
			}

		}

		// IFileOperation開放
		pfo->Release();
	}

	return SUCCEEDED(hr);
}

bool copyFile(const QString & fromFile, const QString & toFile)
{
	HRESULT hr = S_OK;
	IFileOperation *pfo;

	// ファイルパスとファイル名を分解
	QFileInfo info(toFile);
	QString path = info.path();
	QString fileName = info.fileName();

	if (info.dir().exists()) {
		hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo));
	}
	else {
		hr = ERROR_PATH_NOT_FOUND;
	}

	//
	// Create the IFileOperation interface 
	//

	if (SUCCEEDED(hr)) {
		//IFileOperationの挙動を決める
		//hr = pfo->SetOperationFlags(FOF_NO_UI);
		if (SUCCEEDED(hr)) {
			IShellItem *psiTo = NULL;

			//コピー先パスが指定されていれば採用
			if (!path.isEmpty()) {
				hr = SHCreateItemFromParsingName(reinterpret_cast<LPCWSTR>(QDir::toNativeSeparators(path).utf16()), NULL, IID_PPV_ARGS(&psiTo));
			}

			if (SUCCEEDED(hr)) {
				IShellItem *psiFrom = NULL;
				IShellItem *psiFile = NULL;
				hr = SHCreateItemFromParsingName(reinterpret_cast<LPCWSTR>(QDir::toNativeSeparators(fromFile).utf16()), NULL, IID_PPV_ARGS(&psiFrom));

				if (SUCCEEDED(hr)) {
					//コピー登録して、コピー元開放
					hr = pfo->CopyItem(psiFrom, psiTo, reinterpret_cast<LPCWSTR>(QDir::toNativeSeparators(fileName).utf16()), NULL);
					psiFrom->Release();
				}

				//コピー先開放
				if (NULL != psiTo) {
					psiTo->Release();
				}

				if (SUCCEEDED(hr)) {
					hr = pfo->PerformOperations();
				}
			}

		}

		// IFileOperation開放
		pfo->Release();
	}
	return SUCCEEDED(hr);
}

bool moveFiles(const QStringList & files, const QString & path)
{
	HRESULT hr = S_OK;
	IFileOperation *pfo;

	//
	// Create the IFileOperation interface 
	//
	hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo));

	if (SUCCEEDED(hr)) {
		//IFileOperationの挙動を決める
		//hr = pfo->SetOperationFlags(FOF_NO_UI);
		if (SUCCEEDED(hr)) {
			IShellItem *psiTo = NULL;

			//移動先パスが指定されていれば採用
			if (!path.isEmpty()) {
				hr = SHCreateItemFromParsingName(reinterpret_cast<LPCWSTR>(QDir::toNativeSeparators(path).utf16()), NULL, IID_PPV_ARGS(&psiTo));
			}

			if (SUCCEEDED(hr)) {
				for (const QString& from : files) {
					IShellItem *psiFrom = NULL;
					hr = SHCreateItemFromParsingName(reinterpret_cast<LPCWSTR>(QDir::toNativeSeparators(from).utf16()), NULL, IID_PPV_ARGS(&psiFrom));

					if (SUCCEEDED(hr)) {
						//移動登録して、移動元開放
						hr = pfo->MoveItem(psiFrom, psiTo, NULL, NULL);
						psiFrom->Release();
					}

					if (FAILED(hr)) {
						break;
					}
				}

				//移動先開放
				if (NULL != psiTo) {
					psiTo->Release();
				}

				if (SUCCEEDED(hr)) {
					hr = pfo->PerformOperations();
				}
			}

		}

		// IFileOperation開放
		pfo->Release();
	}

	return SUCCEEDED(hr);
}

bool deleteFiles(const QStringList & files)
{
	HRESULT hr = S_OK;
	IFileOperation *pfo;

	//
	// Create the IFileOperation interface 
	//
	hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo));

	if (SUCCEEDED(hr)) {
		//IFileOperationの挙動を決める
		//hr = pfo->SetOperationFlags(FOF_NO_UI);
		if (SUCCEEDED(hr)) {
			for(const QString& from : files) {
				IShellItem *psiFrom = NULL;
				hr = SHCreateItemFromParsingName(reinterpret_cast<LPCWSTR>(QDir::toNativeSeparators(from).utf16()), NULL, IID_PPV_ARGS(&psiFrom));

				if (SUCCEEDED(hr)) {
					//削除登録して、削除ファイルパス開放
					hr = pfo->DeleteItem(psiFrom, NULL);
					psiFrom->Release();
				}

				if (FAILED(hr)) {
					break;
				}
			}

			if (SUCCEEDED(hr)) {
				hr = pfo->PerformOperations();
			}
		}

		// IFileOperation開放
		pfo->Release();
	}

	return SUCCEEDED(hr);
}

bool threadInitialize()
{
	return SUCCEEDED(CoInitialize(NULL));
}

void threadUninitialize()
{
	CoUninitialize();
}
