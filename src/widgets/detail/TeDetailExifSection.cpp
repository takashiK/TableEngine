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

#include "TeDetailExifSection.h"
#include "utils/TeQImageExifReader.h"

#include <QFormLayout>
#include <QLabel>
#include <QImageReader>
#include <QThreadPool>

/**
 * @file TeDetailExifSection.cpp
 * @brief Implementation of TeDetailExifSection.
 * @ingroup widgets
 */

// ---------------------------------------------------------------------------
// Ordered display spec: {map-key, translated label}
// ---------------------------------------------------------------------------
namespace {
struct ExifField { const char* key; const char* label; };
static const ExifField kFields[] = {
    { "Width",           QT_TRANSLATE_NOOP("TeDetailExifSection", "Width")          },
    { "Height",          QT_TRANSLATE_NOOP("TeDetailExifSection", "Height")         },
    { "DateTimeOriginal",QT_TRANSLATE_NOOP("TeDetailExifSection", "Date Taken")     },
    { "DateTime",        QT_TRANSLATE_NOOP("TeDetailExifSection", "Date Modified")  },
    { "Make",            QT_TRANSLATE_NOOP("TeDetailExifSection", "Camera Make")    },
    { "Model",           QT_TRANSLATE_NOOP("TeDetailExifSection", "Camera Model")   },
    { "ExposureTime",    QT_TRANSLATE_NOOP("TeDetailExifSection", "Exposure Time")  },
    { "FNumber",         QT_TRANSLATE_NOOP("TeDetailExifSection", "Aperture")       },
    { "ISO",             QT_TRANSLATE_NOOP("TeDetailExifSection", "ISO")            },
    { "FocalLength",     QT_TRANSLATE_NOOP("TeDetailExifSection", "Focal Length")   },
    { "Orientation",     QT_TRANSLATE_NOOP("TeDetailExifSection", "Orientation")    },
    { "PixelXDimension", QT_TRANSLATE_NOOP("TeDetailExifSection", "Pixel Width")    },
    { "PixelYDimension", QT_TRANSLATE_NOOP("TeDetailExifSection", "Pixel Height")   },
};
static constexpr int kFieldCount = (int)(sizeof(kFields) / sizeof(kFields[0]));
} // namespace

// ---------------------------------------------------------------------------
// TeExifReaderTask
// ---------------------------------------------------------------------------
TeExifReaderTask::TeExifReaderTask(const QString& path, const TeExifReader* reader)
    : m_path(path)
    , mp_reader(reader)
{
}

void TeExifReaderTask::run()
{
    QMap<QString,QString> data = mp_reader->read(m_path);
    emit exifReady(m_path, data);
}

// ---------------------------------------------------------------------------
// TeDetailExifSection
// ---------------------------------------------------------------------------
TeDetailExifSection::TeDetailExifSection(QWidget* parent)
    : TeDetailSection(parent)
    , mp_reader(std::make_unique<TeQImageExifReader>())
{
    mp_form = new QFormLayout(this);
    mp_form->setContentsMargins(6, 4, 6, 4);
    mp_form->setSpacing(3);
    mp_form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // Pre-create a row for each known field (hidden until data is available)
    for (int i = 0; i < kFieldCount; i++) {
        auto* lbl = new QLabel(tr(kFields[i].label) + QLatin1Char(':'), this);
        auto* val = new QLabel(this);
        val->setWordWrap(true);
        mp_form->addRow(lbl, val);
        // Hide by default
        lbl->hide();
        val->hide();
    }

    setLayout(mp_form);
}

void TeDetailExifSection::setExifReader(std::unique_ptr<TeExifReader> reader)
{
    if (reader) mp_reader = std::move(reader);
}

bool TeDetailExifSection::canHandle(const QFileInfo& info) const
{
    if (!info.isFile()) return false;
    return !QImageReader::imageFormat(info.absoluteFilePath()).isEmpty();
}

void TeDetailExifSection::load(const QFileInfo& info)
{
    clear();
    m_currentPath = info.absoluteFilePath();

    auto* task = new TeExifReaderTask(m_currentPath, mp_reader.get());
    task->setAutoDelete(true);
    connect(task, &TeExifReaderTask::exifReady,
            this,  &TeDetailExifSection::onExifReady,
            Qt::QueuedConnection);
    QThreadPool::globalInstance()->start(task);
}

void TeDetailExifSection::clear()
{
    m_currentPath.clear();
    // Hide all value rows
    for (int i = 0; i < mp_form->rowCount(); i++) {
        auto* lbl = mp_form->itemAt(i, QFormLayout::LabelRole);
        auto* fld = mp_form->itemAt(i, QFormLayout::FieldRole);
        if (lbl && lbl->widget()) lbl->widget()->hide();
        if (fld && fld->widget()) fld->widget()->hide();
    }
}

QString TeDetailExifSection::title() const
{
    return tr("EXIF Information");
}

void TeDetailExifSection::onExifReady(const QString& filePath,
                                      const QMap<QString,QString>& data)
{
    if (filePath != m_currentPath) return;
    populate(data);
    emit ready();
}

void TeDetailExifSection::populate(const QMap<QString,QString>& data)
{
    for (int i = 0; i < kFieldCount && i < mp_form->rowCount(); i++) {
        auto* lblItem = mp_form->itemAt(i, QFormLayout::LabelRole);
        auto* fldItem = mp_form->itemAt(i, QFormLayout::FieldRole);
        if (!lblItem || !fldItem) continue;

        auto* lblWidget = qobject_cast<QLabel*>(lblItem->widget());
        auto* valWidget = qobject_cast<QLabel*>(fldItem->widget());
        if (!lblWidget || !valWidget) continue;

        const QString key = QString::fromLatin1(kFields[i].key);
        if (data.contains(key)) {
            valWidget->setText(data.value(key));
            lblWidget->show();
            valWidget->show();
        } else {
            lblWidget->hide();
            valWidget->hide();
        }
    }
}
