#include "TePathListDialog.h"
#include "dialogs/TeFilePathDialog.h"

#include <QLabel>
#include <QListView>
#include <QStringListModel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>

#include <algorithm>

TePathListDialog::TePathListDialog(QWidget *parent)
	: QDialog(parent)
{
	QVBoxLayout* layout = new QVBoxLayout();
	QHBoxLayout* hLayout = new QHBoxLayout();

	m_label = new QLabel(tr("Path List"));
	hLayout->addWidget(m_label);
	hLayout->addStretch();
	QPushButton* button = new QPushButton();
	button->setIcon(QIcon(":/TableEngine/newFolder.png"));
	connect(button, &QPushButton::clicked, this, &TePathListDialog::onAdd);
	hLayout->addWidget(button);

	button = new QPushButton();
	button->setIcon(QIcon(":/TableEngine/delete.png"));
	connect(button, &QPushButton::clicked, this, &TePathListDialog::onRemove);
	hLayout->addWidget(button);

	button = new QPushButton();
	button->setIcon(QIcon(":/TableEngine/up.png"));
	connect(button, &QPushButton::clicked, this, &TePathListDialog::onUp);
	hLayout->addWidget(button);

	button = new QPushButton();
	button->setIcon(QIcon(":/TableEngine/down.png"));
	connect(button, &QPushButton::clicked, this, &TePathListDialog::onDown);
	hLayout->addWidget(button);

	layout->addLayout(hLayout);

	m_listView = new QListView();
	m_listView->setContextMenuPolicy(Qt::ActionsContextMenu);
	m_listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_listView->setModel(new QStringListModel());
	QAction *action = new QAction(tr("Add"), this);
	connect(action, &QAction::triggered, this, &TePathListDialog::onAdd);
	m_listView->addAction(action);
	action = new QAction(tr("Remove"), this);
	connect(action, &QAction::triggered, this, &TePathListDialog::onRemove);
	m_listView->addAction(action);

	layout->addWidget(m_listView);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->setCenterButtons(true);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	layout->addWidget(buttonBox);

	setLayout(layout);
}

TePathListDialog::~TePathListDialog()
{}

void TePathListDialog::setLabelText(const QString & text)
{
	m_label->setText(text);
}

void TePathListDialog::setPathList(const QStringList & pathList)
{
	QStringListModel* model = qobject_cast<QStringListModel*>(m_listView->model());
	if (model)
	{
		model->setStringList(pathList);
	}
}

QStringList TePathListDialog::getPathList() const
{
	QStringListModel* model = qobject_cast<QStringListModel*>(m_listView->model());
	if (model)
	{
		return model->stringList();
	}
	return QStringList();
}

void TePathListDialog::onAdd(bool )
{
	QStringListModel* model = qobject_cast<QStringListModel*>(m_listView->model());
	QItemSelectionModel* selectionModel = m_listView->selectionModel();
	QModelIndex index = selectionModel->currentIndex();

	TeFilePathDialog dialog(this);
	if (dialog.exec() == QDialog::Accepted)
	{
		selectionModel->clear();
		QString path = dialog.targetPath();
		QStringList list = model->stringList();
		if (index.isValid())
		{
			list.insert(index.row(), path);
		}
		else
		{
			list.append(path);
		}
		model->setStringList(list);
	}
}

void TePathListDialog::onRemove(bool )
{
	QStringListModel* model = qobject_cast<QStringListModel*>(m_listView->model());
	QItemSelectionModel* selectionModel = m_listView->selectionModel();
	QModelIndexList indexList = selectionModel->selectedIndexes();
	std::sort(indexList.begin(), indexList.end(), [](const QModelIndex& a, const QModelIndex& b) { return a.row() > b.row(); });

	for (const auto& index : indexList)
	{
		model->removeRow(index.row());
	}
}

void TePathListDialog::onUp(bool )
{
	QStringListModel* model = qobject_cast<QStringListModel*>(m_listView->model());
	QItemSelectionModel* selectionModel = m_listView->selectionModel();

	QModelIndexList indexList = selectionModel->selectedIndexes();
	std::sort(indexList.begin(), indexList.end(), [](const QModelIndex& a, const QModelIndex& b) { return a.row() > b.row(); });

	QStringList list = model->stringList();
	QStringList items;
	for (const auto& index : indexList)
	{
		items.append(list.takeAt(index.row()));
	}

	if (!items.isEmpty()) {
		int row = indexList.last().row();
		if (row > 0) {
			--row;
		}
		for (const auto& item : items)
		{
			list.insert(row, item);
		}
		model->setStringList(list);
		QItemSelection selection(model->index(row, 0), model->index(row + items.size() - 1, 0));
		selectionModel->select(selection, QItemSelectionModel::ClearAndSelect);
		selectionModel->setCurrentIndex(model->index(row, 0), QItemSelectionModel::Current);
	}
}

void TePathListDialog::onDown(bool )
{
	QStringListModel* model = qobject_cast<QStringListModel*>(m_listView->model());
	QItemSelectionModel* selectionModel = m_listView->selectionModel();

	QModelIndexList indexList = selectionModel->selectedIndexes();
	std::sort(indexList.begin(), indexList.end(), [](const QModelIndex& a, const QModelIndex& b) { return a.row() > b.row(); });

	QStringList list = model->stringList();
	QStringList items;
	for (const auto& index : indexList)
	{
		items.append(list.takeAt(index.row()));
	}

	if (!items.isEmpty()) {
		int row = indexList.last().row();
		if (row < model->rowCount() - 1) {
			++row;
		}
		for (const auto& item : items)
		{
			list.insert(row, item);
		}
		model->setStringList(list);
		QItemSelection selection(model->index(row, 0), model->index(row + items.size() - 1, 0));
		selectionModel->select(selection, QItemSelectionModel::ClearAndSelect);
		selectionModel->setCurrentIndex(model->index(row, 0), QItemSelectionModel::Current);
	}
}
