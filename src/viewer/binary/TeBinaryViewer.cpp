#include "TeBinaryViewer.h"

#include <QFileInfo>

#include <QAction>
#include <QByteArray>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDialog>
#include <QDockWidget>
#include <QFontDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSettings>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QStatusBar>
#include <QStyle>
#include <QVBoxLayout>
#include <QtEndian>
#include <cstring>
#include <limits>
#include <type_traits>

#include <QHexView/model/qhexcursor.h>
#include <QHexView/qhexview.h>

#include "TeBinaryViewerSettings.h"

/**
 * @file TeBinaryViewer.cpp
 * @brief Declaration of TeBinaryViewer.
 * @ingroup viewer
 */


TeBinaryViewer::TeBinaryViewer(QWidget *parent)
	: QMainWindow(parent)
{
	setupViewer();
	setupDecodePane();
	setupMenu();
	loadSettings();
}

TeBinaryViewer::~TeBinaryViewer()
{
	delete mp_hexDocument;
}

bool TeBinaryViewer::open(const QString& path)
{
	QFileInfo info = QFileInfo(path);
	if (!info.exists() || !info.isFile())
	{
		return false;
	}

	delete mp_hexDocument;
	mp_hexDocument = nullptr;

	mp_hexDocument = QHexDocument::fromFile(path);
	if (mp_hexDocument)
	{
		mp_hexEdit->setDocument(mp_hexDocument);
		updateDecodePane();
		return true;
	}
	return false;
}

void TeBinaryViewer::closeEvent(QCloseEvent* event)
{
	QSettings settings;
	const QSize size = isMaximized() ? normalGeometry().size() : this->size();
	if (size.isValid() && size.width() > 0 && size.height() > 0) {
		settings.setValue(SETTING_BINARY_VIEWER_WINDOW_WIDTH, size.width());
		settings.setValue(SETTING_BINARY_VIEWER_WINDOW_HEIGHT, size.height());
	}
	if (mp_decodeDock) {
		settings.setValue(SETTING_BINARY_VIEWER_DECODE_PANE_VISIBLE, mp_decodeDock->isVisible());
		settings.setValue(SETTING_BINARY_VIEWER_DECODE_PANE_FLOATING, mp_decodeDock->isFloating());
	}

	QMainWindow::closeEvent(event);
}

void TeBinaryViewer::setupViewer()
{
	mp_hexEdit = new QHexView();
	mp_hexEdit->setReadOnly(true);

	QFont font;
	font.setFamily("Courier");
	font.setFixedPitch(true);
	font.setPointSize(10);
	mp_hexEdit->setFont(font);

	connect(mp_hexEdit, &QHexView::positionChanged, this, [this]() { updateDecodePane(); });

	setCentralWidget(mp_hexEdit);
}

void TeBinaryViewer::setupMenu()
{
	QMenu* menu = nullptr;
	QAction* action = nullptr;

	menu = menuBar()->addMenu(tr("&File"));
	action = menu->addAction(tr("&Exit"));
	action->setShortcuts({ QKeySequence(Qt::Key_Escape) });
	connect(action, &QAction::triggered, this, &TeBinaryViewer::close);

	menu = menuBar()->addMenu(tr("&Search"));
	action = menu->addAction(tr("&Find"));
	action->setShortcut(QKeySequence(tr("Ctrl+F")));
	connect(action, &QAction::triggered, this, [this]() { showFindDialog(); });

	action = menu->addAction(tr("&Go to Offset"));
	action->setShortcut(QKeySequence(tr("Ctrl+G")));
	connect(action, &QAction::triggered, this, [this]() { showGotoOffsetDialog(); });

	action = menu->addAction(tr("&Go to top"));
	connect(action, &QAction::triggered, this, [this]() {
		if (mp_hexEdit && mp_hexEdit->hexCursor()) {
			mp_hexEdit->hexCursor()->move(0);
			updateDecodePane();
		}
	});

	action = menu->addAction(tr("&Go to end"));
	connect(action, &QAction::triggered, this, [this]() {
		if (mp_hexEdit && mp_hexEdit->hexCursor() && mp_hexDocument) {
			const qint64 len = mp_hexDocument->length();
			if (len > 0) {
				mp_hexEdit->hexCursor()->move(len - 1);
				updateDecodePane();
			}
		}
	});

	menu = menuBar()->addMenu(tr("&Option"));
	action = menu->addAction(tr("&Font"));
	connect(action, &QAction::triggered, this, [this]() { fontSettings(); });

	action = menu->addAction(tr("Show &Decode Pane"));
	action->setCheckable(true);
	action->setChecked(mp_decodeDock->isVisible());
	connect(action, &QAction::toggled, this, [this](bool checked) {
		if (checked) {
			mp_decodeDock->show();
		}
		else {
			mp_decodeDock->hide();
		}
		QSettings settings;
		settings.setValue(SETTING_BINARY_VIEWER_DECODE_PANE_VISIBLE, checked);
	});
	connect(mp_decodeDock, &QDockWidget::visibilityChanged, action, &QAction::setChecked);

	QMenu* endianMenu = menu->addMenu(tr("&Decode Endian"));
	action = endianMenu->addAction(tr("&Little Endian"));
	action->setCheckable(true);
	action->setChecked(m_endianMode == EndianMode::Little);
	connect(action, &QAction::triggered, this, [this]() { setEndianMode(EndianMode::Little); });

	action = endianMenu->addAction(tr("&Big Endian"));
	action->setCheckable(true);
	action->setChecked(m_endianMode == EndianMode::Big);
	connect(action, &QAction::triggered, this, [this]() { setEndianMode(EndianMode::Big); });

	connect(endianMenu, &QMenu::aboutToShow, this, [this, endianMenu]() {
		const auto actions = endianMenu->actions();
		if (actions.size() >= 2) {
			actions.at(0)->setChecked(m_endianMode == EndianMode::Little);
			actions.at(1)->setChecked(m_endianMode == EndianMode::Big);
		}
	});
}

void TeBinaryViewer::setupDecodePane()
{
	mp_decodeDock = new QDockWidget(tr("Decode"), this);
	mp_decodeDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

	QWidget* decodeWidget = new QWidget(mp_decodeDock);
	QVBoxLayout* rootLayout = new QVBoxLayout(decodeWidget);
	rootLayout->setContentsMargins(8, 8, 8, 8);
	rootLayout->setSpacing(6);
	rootLayout->setSizeConstraint(QLayout::SetFixedSize);

	QGridLayout* gridLayout = new QGridLayout();
	gridLayout->setHorizontalSpacing(10);
	gridLayout->setVerticalSpacing(4);
	gridLayout->setColumnStretch(1, 1);
	gridLayout->setColumnStretch(3, 1);

	auto setupValueLabel = [](QLabel* label) {
		label->setFrameShape(QFrame::StyledPanel);
		label->setFrameShadow(QFrame::Sunken);
		label->setTextInteractionFlags(Qt::TextSelectableByMouse);
		label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		label->setMinimumWidth(120);
		label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	};

	auto addCell = [gridLayout, &setupValueLabel](int row, int baseCol, const QString& name, QLabel*& valueLabel) {
		QLabel* nameLabel = new QLabel(name);
		nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		valueLabel = new QLabel("N/A");
		setupValueLabel(valueLabel);
		gridLayout->addWidget(nameLabel, row, baseCol);
		gridLayout->addWidget(valueLabel, row, baseCol + 1);
	};

	addCell(0, 0, "int8", mp_decodeValueInt8);
	addCell(0, 2, "uint8", mp_decodeValueUInt8);
	addCell(1, 0, "int16", mp_decodeValueInt16);
	addCell(1, 2, "uint16", mp_decodeValueUInt16);
	addCell(2, 0, "int32", mp_decodeValueInt32);
	addCell(2, 2, "uint32", mp_decodeValueUInt32);
	addCell(3, 0, "int64", mp_decodeValueInt64);
	addCell(3, 2, "uint64", mp_decodeValueUInt64);

	rootLayout->addLayout(gridLayout);

	decodeWidget->setLayout(rootLayout);
	decodeWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	mp_decodeDock->setWidget(decodeWidget);
	mp_decodeDock->setFloating(true);
	mp_decodeDock->resize(decodeWidget->sizeHint());

	addDockWidget(Qt::RightDockWidgetArea, mp_decodeDock);

	connect(mp_decodeDock, &QDockWidget::topLevelChanged, this, [this](bool floating) {
		if (floating && mp_decodeDock && mp_decodeDock->widget()) {
			const QSize hint = mp_decodeDock->widget()->sizeHint();
			mp_decodeDock->setMinimumSize(QSize(0, 0));
			mp_decodeDock->resize(hint);
		}
	});
}

void TeBinaryViewer::loadSettings()
{
	QSettings settings;

	const int windowWidth = qBound(480, settings.value(SETTING_BINARY_VIEWER_WINDOW_WIDTH, 900).toInt(), 8192);
	const int windowHeight = qBound(360, settings.value(SETTING_BINARY_VIEWER_WINDOW_HEIGHT, 700).toInt(), 8192);
	resize(windowWidth, windowHeight);

	QFont font = mp_hexEdit->font();
	font.setFamily(settings.value(SETTING_BINARY_VIEWER_FONT_FAMILY, font.family()).toString());
	font.setPointSize(settings.value(SETTING_BINARY_VIEWER_FONT_SIZE, font.pointSize()).toInt());
	mp_hexEdit->setFont(font);

	const bool decodeVisible = settings.value(SETTING_BINARY_VIEWER_DECODE_PANE_VISIBLE, true).toBool();
	mp_decodeDock->setVisible(decodeVisible);

	const bool decodeFloating = settings.value(SETTING_BINARY_VIEWER_DECODE_PANE_FLOATING, true).toBool();
	mp_decodeDock->setFloating(decodeFloating);
	if (decodeFloating && mp_decodeDock->widget()) {
		mp_decodeDock->resize(mp_decodeDock->widget()->sizeHint());
	}

	const QString endian = settings.value(SETTING_BINARY_VIEWER_ENDIAN, QStringLiteral("little")).toString();
	setEndianMode(endian == QStringLiteral("big") ? EndianMode::Big : EndianMode::Little);
}

void TeBinaryViewer::fontSettings()
{
	bool ok = false;
	QFont font = QFontDialog::getFont(&ok, mp_hexEdit->font(), this);
	if (!ok) {
		return;
	}

	mp_hexEdit->setFont(font);
	QSettings settings;
	settings.setValue(SETTING_BINARY_VIEWER_FONT_FAMILY, font.family());
	settings.setValue(SETTING_BINARY_VIEWER_FONT_SIZE, font.pointSize());
}

void TeBinaryViewer::showFindDialog()
{
	if (mp_findDialog == nullptr) {
		mp_findDialog = new QDialog(this);
		mp_findDialog->setWindowTitle(tr("Find"));

		QVBoxLayout* root = new QVBoxLayout(mp_findDialog);
		QFormLayout* form = new QFormLayout();

		mp_findModeCombo = new QComboBox(mp_findDialog);
		mp_findModeCombo->addItem(tr("Text"), static_cast<int>(SearchMode::Text));
		mp_findModeCombo->addItem(tr("Binary"), static_cast<int>(SearchMode::Binary));
		mp_findModeCombo->addItem(tr("Number"), static_cast<int>(SearchMode::Number));

		mp_findLineEdit = new QLineEdit(mp_findDialog);
		mp_caseSensitiveCheck = new QCheckBox(tr("Case sensitive"), mp_findDialog);
		mp_numberTypeCombo = new QComboBox(mp_findDialog);
		mp_numberTypeCombo->addItem(labelForNumberType(NumberType::Int8), static_cast<int>(NumberType::Int8));
		mp_numberTypeCombo->addItem(labelForNumberType(NumberType::UInt8), static_cast<int>(NumberType::UInt8));
		mp_numberTypeCombo->addItem(labelForNumberType(NumberType::Int16), static_cast<int>(NumberType::Int16));
		mp_numberTypeCombo->addItem(labelForNumberType(NumberType::UInt16), static_cast<int>(NumberType::UInt16));
		mp_numberTypeCombo->addItem(labelForNumberType(NumberType::Int32), static_cast<int>(NumberType::Int32));
		mp_numberTypeCombo->addItem(labelForNumberType(NumberType::UInt32), static_cast<int>(NumberType::UInt32));
		mp_numberTypeCombo->addItem(labelForNumberType(NumberType::Int64), static_cast<int>(NumberType::Int64));
		mp_numberTypeCombo->addItem(labelForNumberType(NumberType::UInt64), static_cast<int>(NumberType::UInt64));

		form->addRow(tr("Mode"), mp_findModeCombo);
		form->addRow(tr("Find"), mp_findLineEdit);
		form->addRow(QString(), mp_caseSensitiveCheck);
		form->addRow(tr("Number type"), mp_numberTypeCombo);

		root->addLayout(form);

		QHBoxLayout* buttons = new QHBoxLayout();
		QPushButton* prevButton = new QPushButton(mp_findDialog);
		prevButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarShadeButton));
		prevButton->setToolTip(tr("Find previous"));
		prevButton->setAutoDefault(false);

		QPushButton* nextButton = new QPushButton(mp_findDialog);
		nextButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarUnshadeButton));
		nextButton->setToolTip(tr("Find next"));
		nextButton->setDefault(true);

		buttons->addStretch(1);
		buttons->addWidget(prevButton);
		buttons->addWidget(nextButton);
		root->addLayout(buttons);

		auto updateDialogState = [this]() {
			if (!mp_findModeCombo || !mp_caseSensitiveCheck || !mp_numberTypeCombo) {
				return;
			}
			const SearchMode mode = static_cast<SearchMode>(mp_findModeCombo->currentData().toInt());
			mp_caseSensitiveCheck->setVisible(mode == SearchMode::Text);
			mp_numberTypeCombo->setVisible(mode == SearchMode::Number);
		};

		connect(mp_findModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), mp_findDialog, [updateDialogState](int) {
			updateDialogState();
		});
		connect(prevButton, &QPushButton::clicked, this, [this]() { findNext(true); });
		connect(nextButton, &QPushButton::clicked, this, [this]() { findNext(false); });
		connect(mp_findLineEdit, &QLineEdit::returnPressed, this, [this]() { findNext(false); });

		updateDialogState();
	}

	QPoint pos = mapToGlobal(rect().topRight() - QPoint(mp_findDialog->sizeHint().width(), 0));
	mp_findDialog->move(pos);
	mp_findDialog->show();
	mp_findDialog->raise();
	mp_findDialog->activateWindow();
	if (mp_findLineEdit) {
		mp_findLineEdit->setFocus();
		mp_findLineEdit->selectAll();
	}
}

void TeBinaryViewer::showGotoOffsetDialog()
{
	if (mp_gotoDialog == nullptr) {
		mp_gotoDialog = new QDialog(this);
		mp_gotoDialog->setWindowTitle(tr("Go to Offset"));

		QHBoxLayout* layout = new QHBoxLayout(mp_gotoDialog);
		QLineEdit* lineEdit = new QLineEdit(mp_gotoDialog);
		lineEdit->setPlaceholderText(tr("1234 or 0x4D2"));

		QPushButton* goButton = new QPushButton(tr("Go"), mp_gotoDialog);
		goButton->setDefault(true);

		layout->addWidget(lineEdit);
		layout->addWidget(goButton);

		connect(goButton, &QPushButton::clicked, this, [this, lineEdit]() {
			qint64 offset = -1;
			if (!parseOffset(lineEdit->text(), offset)) {
				showSearchStatus(tr("Invalid offset"));
				return;
			}
			if (!mp_hexDocument || offset < 0 || offset >= mp_hexDocument->length()) {
				showSearchStatus(tr("Offset out of range"));
				return;
			}
			mp_hexEdit->hexCursor()->move(offset);
			updateDecodePane();
			mp_gotoDialog->close();
		});
		connect(lineEdit, &QLineEdit::returnPressed, goButton, &QPushButton::click);
	}

	QPoint pos = mapToGlobal(rect().topRight() - QPoint(mp_gotoDialog->sizeHint().width(), 0));
	mp_gotoDialog->move(pos);
	mp_gotoDialog->show();
	mp_gotoDialog->raise();
	mp_gotoDialog->activateWindow();
}

void TeBinaryViewer::findNext(bool backward)
{
	if (!mp_hexEdit || !mp_hexEdit->hexCursor() || !mp_hexDocument || !mp_findLineEdit || !mp_findModeCombo) {
		return;
	}

	const QString query = mp_findLineEdit->text();
	if (query.isEmpty()) {
		showSearchStatus(tr("Find text is empty"));
		return;
	}

	QHexFindMode mode = QHexFindMode::Text;
	QByteArray value;
	unsigned int options = QHexFindOptions::None;

	const SearchMode searchMode = static_cast<SearchMode>(mp_findModeCombo->currentData().toInt());
	if (searchMode == SearchMode::Text) {
		mode = QHexFindMode::Text;
		value = query.toUtf8();
		if (mp_caseSensitiveCheck && mp_caseSensitiveCheck->isChecked()) {
			options |= QHexFindOptions::CaseSensitive;
		}
	}
	else if (searchMode == SearchMode::Binary) {
		mode = QHexFindMode::Hex;
		if (!parseBinaryPattern(query, value)) {
			showSearchStatus(tr("Invalid binary pattern"));
			return;
		}
	}
	else {
		mode = QHexFindMode::Hex;
		const NumberType numberType = static_cast<NumberType>(mp_numberTypeCombo->currentData().toInt());
		if (!parseNumberPattern(query, numberType, m_endianMode, value)) {
			showSearchStatus(tr("Invalid number value"));
			return;
		}
	}

	qint64 offset = mp_hexEdit->offset();
	if (backward) {
		offset = qMax<qint64>(0, static_cast<qint64>(mp_hexEdit->selectionStartOffset()) - 1);
	}
	else {
		offset = qMin<qint64>(mp_hexDocument->length(), static_cast<qint64>(mp_hexEdit->selectionEndOffset()) + 1);
	}

	const QHexFindDirection direction = backward ? QHexFindDirection::Backward : QHexFindDirection::Forward;
	const qint64 found = mp_hexEdit->find(value, offset, mode, options, direction);
	if (found < 0) {
		showSearchStatus(tr("Not found"));
		return;
	}

	showSearchStatus(tr("Found at 0x%1").arg(found, 0, 16));
}

void TeBinaryViewer::updateDecodePane()
{
	if (!mp_hexDocument || !mp_hexEdit || !mp_hexEdit->hexCursor()) {
		clearDecodePane();
		return;
	}

	const qint64 offset = mp_hexEdit->hexCursor()->offset();

	mp_decodeValueInt8->setText(decodeValueAtOffset<qint8>(offset, m_endianMode));
	mp_decodeValueUInt8->setText(decodeValueAtOffset<quint8>(offset, m_endianMode));
	mp_decodeValueInt16->setText(decodeValueAtOffset<qint16>(offset, m_endianMode));
	mp_decodeValueUInt16->setText(decodeValueAtOffset<quint16>(offset, m_endianMode));
	mp_decodeValueInt32->setText(decodeValueAtOffset<qint32>(offset, m_endianMode));
	mp_decodeValueUInt32->setText(decodeValueAtOffset<quint32>(offset, m_endianMode));
	mp_decodeValueInt64->setText(decodeValueAtOffset<qint64>(offset, m_endianMode));
	mp_decodeValueUInt64->setText(decodeValueAtOffset<quint64>(offset, m_endianMode));
}

void TeBinaryViewer::setEndianMode(EndianMode mode)
{
	if (m_endianMode == mode) {
		return;
	}
	m_endianMode = mode;

	QSettings settings;
	settings.setValue(SETTING_BINARY_VIEWER_ENDIAN, mode == EndianMode::Big ? QStringLiteral("big") : QStringLiteral("little"));

	updateDecodePane();
}

bool TeBinaryViewer::parseOffset(const QString& text, qint64& offset) const
{
	bool ok = false;
	QString trimmed = text.trimmed();
	if (trimmed.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive)) {
		offset = trimmed.mid(2).toLongLong(&ok, 16);
	}
	else {
		offset = trimmed.toLongLong(&ok, 10);
	}
	return ok;
}

bool TeBinaryViewer::parseBinaryPattern(const QString& text, QByteArray& pattern) const
{
	pattern.clear();
	QString normalized = text;
	normalized.remove(QRegularExpression(QStringLiteral("\\s+")));
	if (normalized.isEmpty() || (normalized.size() % 2) != 0) {
		return false;
	}

	for (QChar ch : normalized) {
		if (!ch.isDigit() && (ch.toLower() < QChar('a') || ch.toLower() > QChar('f'))) {
			return false;
		}
	}

	pattern = QByteArray::fromHex(normalized.toLatin1());
	return !pattern.isEmpty();
}

bool TeBinaryViewer::parseNumberPattern(const QString& text, NumberType type, EndianMode endian, QByteArray& pattern) const
{
	switch (type) {
	case NumberType::Int8:
		return buildNumberPattern<qint8>(text, endian, pattern);
	case NumberType::UInt8:
		return buildNumberPattern<quint8>(text, endian, pattern);
	case NumberType::Int16:
		return buildNumberPattern<qint16>(text, endian, pattern);
	case NumberType::UInt16:
		return buildNumberPattern<quint16>(text, endian, pattern);
	case NumberType::Int32:
		return buildNumberPattern<qint32>(text, endian, pattern);
	case NumberType::UInt32:
		return buildNumberPattern<quint32>(text, endian, pattern);
	case NumberType::Int64:
		return buildNumberPattern<qint64>(text, endian, pattern);
	case NumberType::UInt64:
		return buildNumberPattern<quint64>(text, endian, pattern);
	}
	return false;
}

void TeBinaryViewer::showSearchStatus(const QString& message)
{
	statusBar()->showMessage(message, 3000);
}

void TeBinaryViewer::clearDecodePane()
{
	for (QLabel* label : {
		mp_decodeValueInt8,
		mp_decodeValueUInt8,
		mp_decodeValueInt16,
		mp_decodeValueUInt16,
		mp_decodeValueInt32,
		mp_decodeValueUInt32,
		mp_decodeValueInt64,
		mp_decodeValueUInt64 }) {
		if (label) {
			label->setText("N/A");
		}
	}
}

template<typename T>
QString TeBinaryViewer::decodeValueAtOffset(qint64 offset, EndianMode endian) const
{
	if (!mp_hexDocument || offset < 0) {
		return QStringLiteral("N/A");
	}

	const int bytes = static_cast<int>(sizeof(T));
	const QByteArray data = mp_hexDocument->read(offset, bytes);
	if (data.size() < bytes) {
		return QStringLiteral("N/A");
	}

	const uchar* ptr = reinterpret_cast<const uchar*>(data.constData());

	if constexpr (std::is_same_v<T, qint8>) {
		return QString::number(static_cast<qint8>(ptr[0]));
	}
	else if constexpr (std::is_same_v<T, quint8>) {
		return QString::number(static_cast<quint8>(ptr[0]));
	}
	else {
		if constexpr (std::is_same_v<T, qint16>) {
			quint16 raw = (endian == EndianMode::Little) ? qFromLittleEndian<quint16>(ptr) : qFromBigEndian<quint16>(ptr);
			return QString::number(static_cast<qint16>(raw));
		}
		else if constexpr (std::is_same_v<T, quint16>) {
			quint16 raw = (endian == EndianMode::Little) ? qFromLittleEndian<quint16>(ptr) : qFromBigEndian<quint16>(ptr);
			return QString::number(raw);
		}
		else if constexpr (std::is_same_v<T, qint32>) {
			quint32 raw = (endian == EndianMode::Little) ? qFromLittleEndian<quint32>(ptr) : qFromBigEndian<quint32>(ptr);
			return QString::number(static_cast<qint32>(raw));
		}
		else if constexpr (std::is_same_v<T, quint32>) {
			quint32 raw = (endian == EndianMode::Little) ? qFromLittleEndian<quint32>(ptr) : qFromBigEndian<quint32>(ptr);
			return QString::number(raw);
		}
		else if constexpr (std::is_same_v<T, qint64>) {
			quint64 raw = (endian == EndianMode::Little) ? qFromLittleEndian<quint64>(ptr) : qFromBigEndian<quint64>(ptr);
			return QString::number(static_cast<qint64>(raw));
		}
		else if constexpr (std::is_same_v<T, quint64>) {
			quint64 raw = (endian == EndianMode::Little) ? qFromLittleEndian<quint64>(ptr) : qFromBigEndian<quint64>(ptr);
			return QString::number(raw);
		}
	}

	return QStringLiteral("N/A");
}

template<typename T>
bool TeBinaryViewer::buildNumberPattern(const QString& text, EndianMode endian, QByteArray& pattern) const
{
	pattern.clear();
	bool ok = false;
	const QString trimmed = text.trimmed();

	if constexpr (std::is_signed_v<T>) {
		const qlonglong v = trimmed.toLongLong(&ok, 0);
		if (!ok || v < std::numeric_limits<T>::min() || v > std::numeric_limits<T>::max()) {
			return false;
		}
		const T value = static_cast<T>(v);
		if constexpr (sizeof(T) == 1) {
			pattern.push_back(static_cast<char>(static_cast<quint8>(value)));
			return true;
		}
		else {
			using UnsignedT = std::make_unsigned_t<T>;
			const UnsignedT bits = static_cast<UnsignedT>(value);
			const auto encoded = (endian == EndianMode::Little) ? qToLittleEndian(bits) : qToBigEndian(bits);
			pattern.resize(static_cast<int>(sizeof(T)));
			memcpy(pattern.data(), &encoded, sizeof(T));
			return true;
		}
	}
	else {
		const qulonglong v = trimmed.toULongLong(&ok, 0);
		if (!ok || v > std::numeric_limits<T>::max()) {
			return false;
		}
		const T value = static_cast<T>(v);
		if constexpr (sizeof(T) == 1) {
			pattern.push_back(static_cast<char>(value));
			return true;
		}
		else {
			const auto encoded = (endian == EndianMode::Little) ? qToLittleEndian(value) : qToBigEndian(value);
			pattern.resize(static_cast<int>(sizeof(T)));
			memcpy(pattern.data(), &encoded, sizeof(T));
			return true;
		}
	}
}

QString TeBinaryViewer::labelForNumberType(NumberType type)
{
	switch (type) {
	case NumberType::Int8:
		return QStringLiteral("int8");
	case NumberType::UInt8:
		return QStringLiteral("uint8");
	case NumberType::Int16:
		return QStringLiteral("int16");
	case NumberType::UInt16:
		return QStringLiteral("uint16");
	case NumberType::Int32:
		return QStringLiteral("int32");
	case NumberType::UInt32:
		return QStringLiteral("uint32");
	case NumberType::Int64:
		return QStringLiteral("int64");
	case NumberType::UInt64:
		return QStringLiteral("uint64");
	}
	return QStringLiteral("unknown");
}
