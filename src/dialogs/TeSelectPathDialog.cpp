#include "TeSelectPathDialog.h"
#include "widgets/TeFileTreeView.h"

#include <QFileSystemModel>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QHeaderView>
#include <QDir>
#include <QCompleter>

TeSelectPathDialog::TeSelectPathDialog(QWidget *parent)
	: QDialog(parent)
{
	//Helpボタン削除
	Qt::WindowFlags flags = windowFlags();
	setWindowFlags(flags & ~Qt::WindowContextHelpButtonHint);

	QFileSystemModel* model = new QFileSystemModel();
	model->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	model->setRootPath("");

	//パス名記載用EditBox
	QVBoxLayout *layout = new QVBoxLayout();
	mp_edit = new QLineEdit();
	//パス名入力とツリーを連動させる。
	connect(mp_edit, &QLineEdit::textChanged, [this](const QString& text) {setTargetPath(QDir::fromNativeSeparators(text)); });
	layout->addWidget(mp_edit);

	//LineEditに文字補完機能
	QCompleter* completer = new QCompleter(model);
	completer->setCompletionMode(QCompleter::InlineCompletion);
	completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	mp_edit->setCompleter(completer);


	//フォルダツリー
	model = new QFileSystemModel();
	model->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	mp_tree = new TeFileTreeView();
	mp_tree->setModel(model);
	mp_tree->setVisualRootIndex(model->setRootPath(u8""));
	for (int i = 1; i<mp_tree->header()->count(); i++) {
		mp_tree->header()->setSectionHidden(i, true);
	}
	mp_tree->setHeaderHidden(true);
	mp_tree->installEventFilter(this);
	mp_tree->setAutoScroll(true);

	connect(mp_tree->selectionModel(), &QItemSelectionModel::currentChanged,
		[this,model](const QModelIndex &current, const QModelIndex &previous)
	    { setTargetPath(model->filePath(current)); });

	layout->addWidget(mp_tree);

	//OK Cancelボタン登録
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->setCenterButtons(true);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	layout->addWidget(buttonBox);

	setLayout(layout);
	setTabOrder(mp_tree, buttonBox->button(QDialogButtonBox::Ok));
	setTabOrder(buttonBox->button(QDialogButtonBox::Ok), buttonBox->button(QDialogButtonBox::Cancel));
	setTabOrder(buttonBox->button(QDialogButtonBox::Cancel), mp_edit);
}

TeSelectPathDialog::~TeSelectPathDialog()
{
}

void TeSelectPathDialog::setTargetPath(const QString & path)
{
	QFileSystemModel* model = qobject_cast<QFileSystemModel*>(mp_tree->model());
	QModelIndex index = model->index(path);
	if (index.isValid()) {
		mp_tree->setCurrentIndex(index);
		mp_tree->scrollTo(mp_tree->currentIndex(),QAbstractItemView::PositionAtCenter);
	}
	mp_edit->setText(QDir::toNativeSeparators(path));
}

QString TeSelectPathDialog::targetPath()
{
	return QDir::cleanPath(QDir::fromNativeSeparators(mp_edit->text()));
}
