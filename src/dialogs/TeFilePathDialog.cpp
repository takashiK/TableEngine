#include "TeFilePathDialog.h"
#include "TeSelectPathDialog.h"

#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QKeyEvent>
#include <QDir>
#include <QCompleter>
#include <QFileSystemModel>

TeFilePathDialog::TeFilePathDialog(QWidget *parent)
	: QDialog(parent)
{
	//Helpボタン削除
	Qt::WindowFlags flags = windowFlags();
	setWindowFlags(flags & ~Qt::WindowContextHelpButtonHint);

	QVBoxLayout *layout = new QVBoxLayout();

	//FilePath指定用 CommboBox
	mp_combo = new QComboBox();
	mp_combo->setEditable(true);
	mp_combo->setMinimumWidth(300);
	mp_combo->installEventFilter(this);

	QFileSystemModel* model = new QFileSystemModel();
	model->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	model->setRootPath("");
	QCompleter* completer = new QCompleter(model);
	completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	mp_combo->setCompleter(completer);

	layout->addWidget(mp_combo);

	QLabel* label = new QLabel(tr("Shift+Enter: Select path  Up: History  Down: Favorite"));
	layout->addWidget(label);
	layout->setAlignment(label, Qt::AlignHCenter);

	//OK Cancelボタン登録
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->setCenterButtons(true);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	layout->addWidget(buttonBox);


	setLayout(layout);
}

TeFilePathDialog::~TeFilePathDialog()
{
}

void TeFilePathDialog::setCurrentPath(const QString & path)
{
	m_currentPath = path;
}

QString TeFilePathDialog::currentPath()
{
	return m_currentPath;
}

void TeFilePathDialog::setTargetPath(const QString & path)
{
	mp_combo->setCurrentText(QDir::toNativeSeparators(path));
}

QString TeFilePathDialog::targetPath()
{
	if (m_currentPath.isEmpty()) {
		return QDir::cleanPath(QDir::fromNativeSeparators(mp_combo->currentText()));
	}
	else {
		QDir dir(m_currentPath);
		QString path = QDir::fromNativeSeparators(mp_combo->currentText());
		return QDir::cleanPath(dir.absoluteFilePath(path));
	}
}

bool TeFilePathDialog::eventFilter(QObject * obj, QEvent * event)
{
	if (obj == mp_combo) {
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
			//Shift+Enterによるフォルダ選択ツリー表示
			if ((keyEvent->modifiers() == Qt::ShiftModifier) && (keyEvent->key() == Qt::Key_Return)) {
				TeSelectPathDialog dlg(this);
				dlg.setWindowTitle(tr("Select Path"));
				dlg.setTargetPath(targetPath());
				if (dlg.exec() == QDialog::Accepted) {
					setTargetPath(dlg.targetPath());
				}
				return true;
			}
		}
	}
	return false;
}

