#pragma once

#include <QStyledItemDelegate>
#include "TeTypes.h"

class TeFileItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TeFileItemDelegate(QObject* parent = nullptr);
    ~TeFileItemDelegate() override;

    void setInfoFlags(TeTypes::FileInfoFlags flags);
    TeTypes::FileInfoFlags infoFlags() const;

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    void paintIconMode(QPainter* painter, QStyleOptionViewItem& opt, QStyle* style, const QWidget* widget) const;
    void paintListMode(QPainter* painter, QStyleOptionViewItem& opt, const QModelIndex& index, QStyle* style, const QWidget* widget) const;
    static QString formatFileSize(qint64 bytes);

    TeTypes::FileInfoFlags m_infoFlags = TeTypes::FILEINFO_NONE;
};