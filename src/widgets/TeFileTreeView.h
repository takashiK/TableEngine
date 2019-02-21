#pragma once

#include <QTreeView>
#include <QList>

class TeFileFolderView;

class TeFileTreeView : public QTreeView
{
	Q_OBJECT

public:
	TeFileTreeView(QWidget *parent = Q_NULLPTR);
	virtual  ~TeFileTreeView();

	void	setVisualRootIndex(const QModelIndex &index);
	QModelIndex visualRootIndex();
	virtual TeFileFolderView* folderView();
	void setFolderView(TeFileFolderView* view);

protected slots:
	virtual void	rowsInserted(const QModelIndex &parent, int start, int end);

private:
	QModelIndex m_rootIndex;
	QModelIndex m_rootIndexParent;
	TeFileFolderView* mp_folderView;
};
