#pragma once

#include <QMainWindow>

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
	TeBinaryViewer(QWidget *parent = nullptr);
	~TeBinaryViewer();

	bool open(const QString& path);


private:
	QHexView* mp_hexEdit;
	QHexDocument* mp_hexDocument;
};
