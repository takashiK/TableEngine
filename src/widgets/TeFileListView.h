#pragma once

#include <QListView>

class TeFileFolderView;

class TeFileListView : public QListView
{
	Q_OBJECT

public:
	TeFileListView(QWidget *parent = Q_NULLPTR);
	virtual  ~TeFileListView();

	virtual TeFileFolderView* folderView();
	void setFolderView(TeFileFolderView* view);

protected:
	virtual QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex &index,	const QEvent *event = Q_NULLPTR) const;
	virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

private:
	TeFileFolderView* mp_folderView;
};
