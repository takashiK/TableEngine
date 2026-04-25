#include "TeSearchQuery.h"
#include "dialogs/TeFindDialog.h"

bool TeSearchQuery::isValid() const
{
	return !targetPath.isEmpty();
}

TeSearchQuery TeSearchQuery::fromDialog(const TeFindDialog& dlg, const QString& targetPath)
{
	TeSearchQuery query;
	query.targetPath = targetPath;
	query.recursive  = true;

	// --- Name filter ---
	if (dlg.hasFileNameSearch()) {
		query.hasNameFilter = true;
		const QRegularExpression::PatternOptions opts =
			dlg.isCaseSensitive()
				? QRegularExpression::NoPatternOption
				: QRegularExpression::CaseInsensitiveOption;
		if (dlg.isRegex()) {
			query.namePattern = QRegularExpression(dlg.searchTerm(), opts);
		} else {
			query.namePattern = QRegularExpression(
				QRegularExpression::wildcardToRegularExpression(dlg.searchTerm()), opts);
		}
	}

	// --- Size filter ---
	if (dlg.hasSizeSearch()) {
		query.hasSizeFilter = true;
		const qsizetype multiplier =
			(dlg.sizeUnit() == TeFindDialog::SizeUnit_MB) ? (1024LL * 1024) : 1024LL;
		query.sizeThreshold = static_cast<qsizetype>(dlg.sizeValue()) * multiplier;
		query.sizeLessThan  = (dlg.sizeCondition() == TeFindDialog::SizeLessThan);
	}

	// --- Date filter ---
	if (dlg.hasDateSearch()) {
		query.hasDateFilter = true;
		query.dateFrom = QDateTime(dlg.dateFrom(), QTime(0, 0, 0));
		query.dateTo   = QDateTime(dlg.dateTo(),   QTime(23, 59, 59));
	}

	return query;
}
