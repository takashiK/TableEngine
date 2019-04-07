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

#include <QTreeView>
#include <QList>

class TeFolderView;

class TeFileTreeView : public QTreeView
{
	Q_OBJECT

public:
	TeFileTreeView(QWidget *parent = Q_NULLPTR);
	virtual  ~TeFileTreeView();

	void	setVisualRootIndex(const QModelIndex &index);
	QModelIndex visualRootIndex();
	virtual TeFolderView* folderView();
	void setFolderView(TeFolderView* view);

protected slots:
	virtual void	rowsInserted(const QModelIndex &parent, int start, int end);

private:
	QModelIndex m_rootIndex;
	QModelIndex m_rootIndexParent;
	TeFolderView* mp_folderView;
};
