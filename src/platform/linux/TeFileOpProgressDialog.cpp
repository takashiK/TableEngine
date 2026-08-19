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

#include "platform/linux/TeFileOpProgressDialog.h"
#include "platform/TeFileOpProgress.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include <QCloseEvent>
#include <QPainter>
#include <QVector>
#include <QFontMetrics>

/**
 * @file TeFileOpProgressDialog.cpp
 * @brief Implementation of TeFileOpProgressDialog.
 * @ingroup platform
 */

namespace TeFileOpProgressDialogPrivate {

/**
 * @class RateGraph
 * @brief Minimal byte-rate line graph drawn with QPainter (Qt Widgets only,
 *        no external plotting dependency), similar in spirit to the transfer
 *        graph shown by Windows Explorer's copy dialog.
 */
class RateGraph : public QWidget
{
public:
	explicit RateGraph(QWidget* parent = nullptr) : QWidget(parent)
	{
		setMinimumHeight(48);
	}

	void addSample(qreal bytesPerSec)
	{
		m_samples.append(bytesPerSec);
		while (m_samples.size() > kMaxSamples) {
			m_samples.removeFirst();
		}
		update();
	}

protected:
	void paintEvent(QPaintEvent* /*event*/) override
	{
		QPainter painter(this);
		painter.fillRect(rect(), palette().base());
		painter.setPen(palette().mid().color());
		painter.drawRect(rect().adjusted(0, 0, -1, -1));

		if (m_samples.size() < 2) {
			return;
		}

		qreal maxValue = 1.0;
		for (qreal value : m_samples) {
			maxValue = qMax(maxValue, value);
		}

		painter.setPen(QPen(palette().highlight().color(), 1.5));
		QPolygonF polyline;
		const qreal stepX = qreal(width()) / (m_samples.size() - 1);
		for (int i = 0; i < m_samples.size(); ++i) {
			const qreal x = i * stepX;
			const qreal y = height() - (m_samples[i] / maxValue) * (height() - 4) - 2;
			polyline << QPointF(x, y);
		}
		painter.drawPolyline(polyline);
	}

private:
	static constexpr int kMaxSamples = 60;
	QVector<qreal> m_samples;
};

} // namespace TeFileOpProgressDialogPrivate

namespace {
	QString formatBytes(qint64 bytes)
	{
		static const char* units[] = { "B", "KB", "MB", "GB", "TB" };
		double value = static_cast<double>(bytes);
		int unit = 0;
		while (value >= 1024.0 && unit < 4) {
			value /= 1024.0;
			++unit;
		}
		return QString::number(value, 'f', (unit == 0) ? 0 : 1) + QLatin1Char(' ') + QLatin1String(units[unit]);
	}
}

TeFileOpProgressDialog::TeFileOpProgressDialog(TeFileOpProgress* progress, const QString& title, QWidget* parent)
	: QDialog(parent)
	, mp_progress(progress)
{
	setWindowTitle(title);
	setModal(false);
	resize(420, 180);
	QVBoxLayout* layout = new QVBoxLayout(this);

	mp_pathLabel = new QLabel(this);
	mp_pathLabel->setFixedWidth(400);
	layout->addWidget(mp_pathLabel);

	mp_progressBar = new QProgressBar(this);
	mp_progressBar->setRange(0, 100);
	layout->addWidget(mp_progressBar);

	mp_rateGraph = new TeFileOpProgressDialogPrivate::RateGraph(this);
	layout->addWidget(mp_rateGraph);

	mp_statsLabel = new QLabel(this);
	layout->addWidget(mp_statsLabel);

	QHBoxLayout* buttonRow = new QHBoxLayout();
	buttonRow->addStretch();
	mp_cancelButton = new QPushButton(tr("Cancel"), this);
	connect(mp_cancelButton, &QPushButton::clicked, this, &TeFileOpProgressDialog::requestCancellation);
	buttonRow->addWidget(mp_cancelButton);
	layout->addLayout(buttonRow);

	mp_timer = new QTimer(this);
	connect(mp_timer, &QTimer::timeout, this, &TeFileOpProgressDialog::poll);
	mp_timer->start(150);

	// Delay showing the dialog so operations that finish quickly never flash
	// it on screen. The receiver-context overload of singleShot() is safe if
	// "this" is destroyed first (e.g. the operation finished before the delay
	// elapsed): Qt automatically disconnects and never re-shows a dead dialog.
	// It is not, however, enough on its own: TeFileOperationManager::endProgress()
	// only calls deleteLater() (not an immediate delete), and a nested
	// QMessageBox event loop started right after can run long enough for this
	// timer to still fire before that deferred deletion happens, re-showing a
	// dialog for an operation that already finished. m_detached (set by
	// detachProgress(), called from endProgress() before deleteLater()) guards
	// against exactly that race.
	constexpr int kShowDelayMs = 450;
	QTimer::singleShot(kShowDelayMs, this, [this]() {
		if (!m_detached) {
			show();
		}
	});
}

TeFileOpProgressDialog::~TeFileOpProgressDialog()
{
}

void TeFileOpProgressDialog::detachProgress()
{
	m_detached = true;
	mp_progress = nullptr;
}

void TeFileOpProgressDialog::closeEvent(QCloseEvent* event)
{
	// Closing the window must behave exactly like pressing Cancel: request
	// cancellation first so the worker thread observes it, then stop polling
	// before the paired TeFileOpProgress is destroyed by the manager.
	requestCancellation();
	mp_timer->stop();
	QDialog::closeEvent(event);
}

void TeFileOpProgressDialog::reject()
{
	// Escape triggers reject(), not close()/closeEvent(); route it through the
	// same cancellation path so Esc behaves exactly like Cancel/closing.
	requestCancellation();
	mp_timer->stop();
	QDialog::reject();
}

void TeFileOpProgressDialog::requestCancellation()
{
	if (mp_progress) {
		mp_progress->requestCancel();
	}
	mp_cancelButton->setEnabled(false);
}

void TeFileOpProgressDialog::poll()
{
	if (mp_progress == nullptr) {
		return;
	}
	const TeFileOpProgress::Snapshot snap = mp_progress->snapshot();

	if (snap.totalBytes > 0) {
		mp_progressBar->setRange(0, 100);
		const qint64 percent = qBound<qint64>(0, snap.processedBytes * 100 / snap.totalBytes, qint64(100));
		mp_progressBar->setValue(static_cast<int>(percent));
	}
	else {
		mp_progressBar->setRange(0, 0); // indeterminate
	}

	mp_pathLabel->setText(fontMetrics().elidedText(snap.currentPath, Qt::ElideMiddle, mp_pathLabel->width()));

	const qint64 deltaBytes = snap.processedBytes - m_lastProcessedBytes;
	const qint64 deltaMs = snap.elapsedMs - m_lastElapsedMs;
	const qreal bytesPerSec = (deltaMs > 0) ? (deltaBytes * 1000.0 / deltaMs) : 0.0;
	mp_rateGraph->addSample(bytesPerSec);
	m_lastProcessedBytes = snap.processedBytes;
	m_lastElapsedMs = snap.elapsedMs;

	const int elapsedSec = static_cast<int>(snap.elapsedMs / 1000);
	mp_statsLabel->setText(tr("%1 / %2   %3/s   Elapsed %4:%5")
		.arg(formatBytes(snap.processedBytes))
		.arg(snap.totalBytes > 0 ? formatBytes(snap.totalBytes) : tr("?"))
		.arg(formatBytes(static_cast<qint64>(bytesPerSec)))
		.arg(elapsedSec / 60, 2, 10, QLatin1Char('0'))
		.arg(elapsedSec % 60, 2, 10, QLatin1Char('0')));
}
