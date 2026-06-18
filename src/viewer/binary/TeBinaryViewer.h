#pragma once

#include <QMainWindow>

class QCloseEvent;
class QDialog;
class QDockWidget;
class QLabel;
class QComboBox;
class QLineEdit;
class QCheckBox;

/**
 * @file TeBinaryViewer.h
 * @brief Declaration of TeBinaryViewer.
 * @ingroup viewer
 */


class QHexView;
class QHexDocument;

class TeBinaryViewer  : public QMainWindow
{
	Q_OBJECT

public:
	enum class SearchMode {
		Text,
		Binary,
		Number,
	};

	enum class NumberType {
		Int8,
		UInt8,
		Int16,
		UInt16,
		Int32,
		UInt32,
		Int64,
		UInt64,
	};

	enum class EndianMode {
		Little,
		Big,
	};

public:
	TeBinaryViewer(QWidget *parent = nullptr);
	~TeBinaryViewer();

	bool open(const QString& path);

protected:
	void closeEvent(QCloseEvent* event) override;


private:
	void setupViewer();
	void setupMenu();
	void setupDecodePane();
	void loadSettings();
	void fontSettings();
	void showFindDialog();
	void showGotoOffsetDialog();
	void findNext(bool backward);
	void updateDecodePane();
	void setEndianMode(EndianMode mode);
	bool parseOffset(const QString& text, qint64& offset) const;
	bool parseBinaryPattern(const QString& text, QByteArray& pattern) const;
	bool parseNumberPattern(const QString& text, NumberType type, EndianMode endian, QByteArray& pattern) const;
	void showSearchStatus(const QString& message);
	void clearDecodePane();

	template<typename T>
	QString decodeValueAtOffset(qint64 offset, EndianMode endian) const;

	template<typename T>
	bool buildNumberPattern(const QString& text, EndianMode endian, QByteArray& pattern) const;

	static QString labelForNumberType(NumberType type);

	QHexView* mp_hexEdit = nullptr;
	QHexDocument* mp_hexDocument = nullptr;
	QDialog* mp_findDialog = nullptr;
	QDialog* mp_gotoDialog = nullptr;
	QDockWidget* mp_decodeDock = nullptr;
	QLineEdit* mp_findLineEdit = nullptr;
	QComboBox* mp_findModeCombo = nullptr;
	QComboBox* mp_numberTypeCombo = nullptr;
	QCheckBox* mp_caseSensitiveCheck = nullptr;
	QLabel* mp_decodeValueInt8 = nullptr;
	QLabel* mp_decodeValueUInt8 = nullptr;
	QLabel* mp_decodeValueInt16 = nullptr;
	QLabel* mp_decodeValueUInt16 = nullptr;
	QLabel* mp_decodeValueInt32 = nullptr;
	QLabel* mp_decodeValueUInt32 = nullptr;
	QLabel* mp_decodeValueInt64 = nullptr;
	QLabel* mp_decodeValueUInt64 = nullptr;
	EndianMode m_endianMode = EndianMode::Little;
};
