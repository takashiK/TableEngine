/****************************************************************************
**
** Copyright (C) 2021 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "platform/platform_util.h"
#include "platform/TeNativeEvent.h"
#include "platform/windows/TeWindowsEventFilter.h"

#include <QMap>
#include <QDir>
#include <QFileInfo>
#include <Shobjidl.h>
#include <QMimeData>
#include <QImage>

#include <commoncontrols.h>
#include <Windows.h>
#include <commctrl.h>
#include <Shlobj.h>
#include <Shlwapi.h>
#include <shellapi.h>
#include <thumbcache.h>
#include <GdiPlus.h>

#include <vector>

//////////////////////////////////////////////////////////
//
// Namespace

namespace {
	//////////////////////////////////////////////////////////
	// NativeEventFilter
	
	TeWindowsEventFilter g_eventFilter;
	TeNativeEvent g_event;

	//////////////////////////////////////////////////////////
	// File action

	QMap<HWND, IContextMenu2*> g_context;
	QString cname;

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

	QString registerContextWndClass()
	{
		if (cname.isEmpty()) {
			cname = u8"TableEngineWindowsContext";
			WNDCLASSEX wc = { 0 };
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
		return cname;
	}

	void popupContextWindows(int px, int py,  PIDLIST_ABSOLUTE parent, LPCITEMIDLIST* pidlChildList, UINT childCount) {
		//show context menu for multiple files
		int                 nId = 0;
		HRESULT             hr;
		HMENU               hmenuPopup;
		IContextMenu* pContextMenu = NULL;
		IShellFolder* pShellFolder = NULL;

		QString cname = registerContextWndClass();
		HWND hwnd = CreateWindowEx(0, reinterpret_cast<LPCWSTR>(cname.utf16()),
			L"ContextWindow", WS_OVERLAPPED,
			CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT,
			HWND_MESSAGE, NULL, static_cast<HINSTANCE>(GetModuleHandle(0)), NULL);

		SHBindToParent(parent, IID_PPV_ARGS(&pShellFolder), NULL);

		hr = pShellFolder->GetUIObjectOf(NULL, childCount, pidlChildList, IID_IContextMenu, NULL, (void**)&pContextMenu);
		IContextMenu2* pContextMenu2 = NULL;
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
			CMINVOKECOMMANDINFO ici = { 0 };
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
	}
}
// Namespace end
// 
//////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
//
// Initialize and uninitialize platform system
//
bool threadInitialize(QApplication* a)
{
	bool result = SUCCEEDED(CoInitialize(NULL));

	if (result) {
		//setup eventfilter for windows message.
		g_eventFilter.setNativeEvent(&g_event);
		a->installNativeEventFilter(&g_eventFilter);
	}

	return result;
}

void threadUninitialize()
{
	CoUninitialize();
}


bool showFilesContext(int px, int py, const QStringList& paths)
{
	//get base folder path
	QFileInfo info (paths[0]);
	QString baseFolder = info.absolutePath();
	//check same base
	for (const auto& path : paths) {
		QFileInfo info (path);
		if (info.absolutePath() != baseFolder) {
			return false;
		}
	}

	//make pidl list
	std::vector<PIDLIST_ABSOLUTE> pidlList(paths.size());
	std::vector< LPCITEMIDLIST> pidlChildList(paths.size());

	bool isPidlListOK = true;
	for (int i = 0; i < paths.size(); ++i) {
		QString rpath = QDir::toNativeSeparators(paths[i]);
		pidlList[i] = ILCreateFromPath(reinterpret_cast<LPCWSTR>(rpath.utf16()));
		pidlChildList[i] = ILFindLastID(pidlList[i]);
		if (pidlList[i] == NULL || pidlChildList[i] == NULL) {
			isPidlListOK = false;
			break;
		}
	}

	if (isPidlListOK) {
		//show context menu for multiple files
		popupContextWindows(px, py, pidlList[0], pidlChildList.data(), paths.size());
	}


	//cleanup
	for (int i = 0; i < paths.size(); ++i) {
		if (pidlList[i] != NULL)
			ILFree(pidlList[i]);
	}

	return true;
}

void showFileContext(int px, int py, const QString& path)
{
	QString cname = registerContextWndClass();

	QString rpath = QDir::toNativeSeparators(path);
	PIDLIST_ABSOLUTE pidlAbsolute = ILCreateFromPath(reinterpret_cast<LPCWSTR>(rpath.utf16()));
	LPCITEMIDLIST   pidlChild = ILFindLastID(pidlAbsolute);

	if (pidlAbsolute != NULL) {
		popupContextWindows(px, py, pidlAbsolute, &pidlChild, 1);
	}
	ILFree(pidlAbsolute);

}

void showFilesProperties(const QStringList& paths)
{
	//make pidl list
	std::vector<PIDLIST_ABSOLUTE> pidlList(paths.size());
	std::vector< LPCITEMIDLIST> pidlChildList(paths.size());

	bool isPidlListOK = true;
	for (int i = 0; i < paths.size(); ++i) {
		QString rpath = QDir::toNativeSeparators(paths[i]);
		pidlList[i] = ILCreateFromPath(reinterpret_cast<LPCWSTR>(rpath.utf16()));
		pidlChildList[i] = ILFindLastID(pidlList[i]);
		if (pidlList[i] == NULL || pidlChildList[i] == NULL) {
			isPidlListOK = false;
			break;
		}
	}

	PIDLIST_ABSOLUTE parent = ILClone(pidlList[0]);
	ILRemoveLastID(parent);

	IDataObject* dataObjects;
	SHCreateDataObject(parent, paths.size(), pidlChildList.data(), NULL, IID_PPV_ARGS(&dataObjects));

	SHMultiFileProperties(dataObjects, 0);

	//cleanup
	ILFree(parent);
	dataObjects->Release();
	for (int i = 0; i < paths.size(); ++i) {
		if (pidlList[i] != NULL)
			ILFree(pidlList[i]);
	}
}

void showFileProperties(const QString& path)
{
	QString rpath = QDir::toNativeSeparators(path);
	SHELLEXECUTEINFO info = { 0 };
	info.cbSize = sizeof(SHELLEXECUTEINFO);
	info.fMask = SEE_MASK_INVOKEIDLIST;
	info.hwnd = NULL;
	info.lpVerb = L"properties";
	info.lpFile = reinterpret_cast<LPCWSTR>(rpath.utf16());
	info.lpParameters = NULL;
	info.lpDirectory = NULL;
	info.nShow = SW_SHOW;
	info.hInstApp = NULL;
	ShellExecuteEx(&info);
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
		//hr = pfo->SetOperationFlags(FOF_NO_UI);
		if (SUCCEEDED(hr)) {
			IShellItem *psiTo = NULL;

			//select copy path
			if (!path.isEmpty()) {
				hr = SHCreateItemFromParsingName(reinterpret_cast<LPCWSTR>(QDir::toNativeSeparators(path).utf16()), NULL, IID_PPV_ARGS(&psiTo));
			}

			if (SUCCEEDED(hr)) {
				for ( const QString& from : files) {
					IShellItem *psiFrom = NULL;
					hr = SHCreateItemFromParsingName(reinterpret_cast<LPCWSTR>(QDir::toNativeSeparators(from).utf16()), NULL, IID_PPV_ARGS(&psiFrom));

					if (SUCCEEDED(hr)) {
						hr = pfo->CopyItem(psiFrom, psiTo, NULL, NULL);
						psiFrom->Release();
					}

					if (FAILED(hr)) {
						break;
					}
				}

				if (NULL != psiTo) {
					psiTo->Release();
				}

				if (SUCCEEDED(hr)) {
					hr = pfo->PerformOperations();
				}
			}

		}

		pfo->Release();
	}

	return SUCCEEDED(hr);
}

bool copyFile(const QString & fromFile, const QString & toFile)
{
	HRESULT hr = S_OK;
	IFileOperation *pfo;

	// split file path and file name.
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
		//hr = pfo->SetOperationFlags(FOF_NO_UI);
		if (SUCCEEDED(hr)) {
			IShellItem *psiTo = NULL;

			if (!path.isEmpty()) {
				hr = SHCreateItemFromParsingName(reinterpret_cast<LPCWSTR>(QDir::toNativeSeparators(path).utf16()), NULL, IID_PPV_ARGS(&psiTo));
			}

			if (SUCCEEDED(hr)) {
				IShellItem *psiFrom = NULL;
				hr = SHCreateItemFromParsingName(reinterpret_cast<LPCWSTR>(QDir::toNativeSeparators(fromFile).utf16()), NULL, IID_PPV_ARGS(&psiFrom));

				if (SUCCEEDED(hr)) {
					hr = pfo->CopyItem(psiFrom, psiTo, reinterpret_cast<LPCWSTR>(QDir::toNativeSeparators(fileName).utf16()), NULL);
					psiFrom->Release();
				}

				if (NULL != psiTo) {
					psiTo->Release();
				}

				if (SUCCEEDED(hr)) {
					hr = pfo->PerformOperations();
				}
			}

		}

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
		//hr = pfo->SetOperationFlags(FOF_NO_UI);
		if (SUCCEEDED(hr)) {
			IShellItem *psiTo = NULL;

			if (!path.isEmpty()) {
				hr = SHCreateItemFromParsingName(reinterpret_cast<LPCWSTR>(QDir::toNativeSeparators(path).utf16()), NULL, IID_PPV_ARGS(&psiTo));
			}

			if (SUCCEEDED(hr)) {
				for (const QString& from : files) {
					IShellItem *psiFrom = NULL;
					hr = SHCreateItemFromParsingName(reinterpret_cast<LPCWSTR>(QDir::toNativeSeparators(from).utf16()), NULL, IID_PPV_ARGS(&psiFrom));

					if (SUCCEEDED(hr)) {
						hr = pfo->MoveItem(psiFrom, psiTo, NULL, NULL);
						psiFrom->Release();
					}

					if (FAILED(hr)) {
						break;
					}
				}

				if (NULL != psiTo) {
					psiTo->Release();
				}

				if (SUCCEEDED(hr)) {
					hr = pfo->PerformOperations();
				}
			}

		}

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
		//hr = pfo->SetOperationFlags(FOF_NO_UI);
		if (SUCCEEDED(hr)) {
			for(const QString& from : files) {
				IShellItem *psiFrom = NULL;
				hr = SHCreateItemFromParsingName(reinterpret_cast<LPCWSTR>(QDir::toNativeSeparators(from).utf16()), NULL, IID_PPV_ARGS(&psiFrom));

				if (SUCCEEDED(hr)) {
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

		pfo->Release();
	}

	return SUCCEEDED(hr);
}


QPixmap getThumbnail(const QString& path, const QSize& size)
{
	UINT nSize = qMax(size.width(), size.height());
	IShellItem* psi;
	HRESULT hr = SHCreateItemFromParsingName(reinterpret_cast<PCWSTR>(QDir::toNativeSeparators(path).utf16()), NULL, IID_PPV_ARGS(&psi));
	if (SUCCEEDED(hr))
	{
		IThumbnailProvider* pThumbProvider;
		hr = psi->BindToHandler(NULL, BHID_ThumbnailHandler, IID_PPV_ARGS(&pThumbProvider));
		if (SUCCEEDED(hr))
		{
			WTS_ALPHATYPE wtsAlpha;
			HBITMAP hBitmap;
			hr = pThumbProvider->GetThumbnail(nSize, &hBitmap, &wtsAlpha);
			if (SUCCEEDED(hr))
			{
				QImage image = QImage::fromHBITMAP(hBitmap);
				if (wtsAlpha == WTSAT_ARGB) {
					image.reinterpretAsFormat(QImage::Format_ARGB32);
				}
				QPixmap pixmap = QPixmap::fromImage(image);
				DeleteObject(hBitmap);

				return pixmap;
			}
			pThumbProvider->Release();
		}
		psi->Release();
	}

	return QPixmap();
}


QPixmap getFileIcon(const QString& path, const QSize& size)
{
	int iconSize = qMax(size.width(), size.height());
	int iconType = SHIL_JUMBO;
	if (iconSize <= 16) {
		iconType = SHIL_SMALL;
	}
	else if(iconSize <= 32){
		iconType = SHIL_LARGE;
	}
	else if (iconSize <= 48) {
		iconType = SHIL_EXTRALARGE;
	}

	IImageList* piml=nullptr;
	HRESULT hr = SHGetImageList(iconType, IID_IImageList, reinterpret_cast<void**>(&piml));

	if (FAILED(hr))
	{
		return QPixmap();
	}

	SHFILEINFO shfi;
	ZeroMemory(&shfi, sizeof(SHFILEINFO));
	DWORD_PTR res = SHGetFileInfo(
		reinterpret_cast<const WCHAR*>(QDir::toNativeSeparators(path).utf16()), 0,
		&shfi, sizeof(SHFILEINFO),
		SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX);

	if (res == 0)
	{
		piml->Release();
		return QPixmap();
	}

	HICON hIcon;
	hr = piml->GetIcon(shfi.iIcon, ILD_TRANSPARENT, &hIcon);

	if (FAILED(hr))
	{
		piml->Release();
		return QPixmap();
	}

	QPixmap pixmap = QPixmap::fromImage(QImage::fromHICON(hIcon));
	DestroyIcon(hIcon);
	piml->Release();

	QSize pixSize = pixmap.size().expandedTo(size);

	return pixmap.scaled(pixSize);
}



//////////////////////////////////////////////////////////
//
// Clipboard
//

bool isMoveAction(const QMimeData* mime)
{
	if (mime->hasFormat("application/x-qt-windows-mime;value=\"Preferred DropEffect\"")) {
		const QByteArray array = mime->data("application/x-qt-windows-mime;value=\"Preferred DropEffect\"");
		return (array.at(0) == 2);
	}

	if (mime->hasFormat("Preferred DropEffect")) {
		const QByteArray array = mime->data("Preferred DropEffect");
		return (array.at(0) == 2);
	}

	return false;
}

void setMoveAction(QMimeData* mime)
{
	QByteArray data(4, 0);
	data[0] = 2;
	mime->setData("Preferred DropEffect", data);
}

void setCopyAction(QMimeData* mime)
{
	QByteArray data(4, 0);
	data[0] = 5;
	mime->setData("Preferred DropEffect", data);
}

TeNativeEvent* getNativeEvent()
{
	return &g_event;
}
