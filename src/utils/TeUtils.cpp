#include "TeUtils.h"
#include "TeViewStore.h"
#include "TeSettings.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeArchiveFolderView.h"
#include "widgets/TeFileTreeView.h"
#include "widgets/TeFileListView.h"
#include "utils/TeFileInfo.h"
#include "utils/TeArchive.h"
#include "dialogs/TePasswordDialog.h"

#include <QStringList>
#include <QAbstractItemView>
#include <QFileSystemModel>
#include <QProgressDialog>
#include <QMessageBox>

#include <QSet>
#include <QFileInfo>
#include <QDir>

#include <QDebug>
#include <QSettings>
#include <QMimeDatabase>
#include <QFile>
#include <QVector>

#include <icu.h>

/**
 * @file TeUtils.cpp
 * @brief Implementation of TeUtils.
 * @ingroup utility
 */

/**
 * @file TeUtils.cpp
 * @brief Implementation of TeUtils.
 * @ingroup utility
 */


namespace {
	const QSet<const QString> txtSuffixes{ "txt","html","htm","md","h","c","cpp","ini","py","json","ts","js","sh","bat","cmd","ps1","yml","yaml","xml","csv","log" };
	const QSet<const QString> imageSuffixes{ "jpg","jpeg","png","gif","bmp","tiff","svg" };
	const QSet<const QString> archiveSuffixes{ "zip","lzh","cab","7z","rar","tar","gz","bz2","xz","tgz","cpio","ar","iso","warc","shar","mtree" };

	// Resolves the path string for a model index, supporting both filesystem
	// items (QFileInfo via FileInfoRole) and archive items (virtual path via
	// TeFileInfo::ROLE_PATH).  Returns an empty string for non-selectable
	// synthetic entries such as "..".
	QString resolveItemPath(const QModelIndex& index)
	{
		QVariant var = index.data(QFileSystemModel::FileInfoRole);
		if (var.isValid() && var.canConvert<QFileInfo>()) {
			return qvariant_cast<QFileInfo>(var).filePath();
		}

		QVariant typeVar = index.data(TeFileInfo::ROLE_TYPE);
		if (typeVar.isValid()) {
			if (typeVar.toInt() == TeFileInfo::EN_PARENT)
				return QString();
			return index.data(TeFileInfo::ROLE_PATH).toString();
		}
		return QString();
	}
}

bool getSelectedItemList(TeViewStore* p_store, QStringList* p_paths)
{
	TeFolderView* p_folder = p_store->currentFolderView();

	if (p_folder != nullptr && p_paths != nullptr) {
		//get selected files
		QAbstractItemView* p_itemView = nullptr;
		if (p_folder->tree()->hasFocus()) {
			p_itemView = p_folder->tree();
		}
		else {
			p_itemView = p_folder->list();
		}

		if (p_itemView->selectionModel()->hasSelection()) {
			//target selected files.
			QModelIndexList indexList = p_itemView->selectionModel()->selectedIndexes();
			for (const QModelIndex& index : indexList)
			{
				if (index.column() == 0) {
					QString path = resolveItemPath(index);
					if (!path.isEmpty()) {
						p_paths->append(path);
					}
				}
			}
		}
		else {
			//no files selected. so use current file.
			if (p_itemView->currentIndex().isValid()) {
				QString path = resolveItemPath(p_itemView->currentIndex());
				if (!path.isEmpty()) {
					p_paths->append(path);
				}
			}
		}
	}
	return (p_paths != nullptr) && !p_paths->isEmpty();
}

bool getSelectedFileList(TeViewStore* p_store, QStringList* p_paths)
{
	if (p_paths == nullptr) {
		return false;
	}
	QStringList allItems;
	getSelectedItemList(p_store, &allItems);
	for (const QString& path : allItems) {
		if (!QFileInfo(path).isDir()) {
			p_paths->append(path);
		}
	}
	return !p_paths->isEmpty();
}

QString getCurrentItem(TeViewStore* p_store)
{
	TeFolderView* p_folder = p_store->currentFolderView();

	if (p_folder != nullptr ) {
		//get selected files
		QAbstractItemView* p_itemView = nullptr;
		if (p_folder->tree()->hasFocus()) {
			p_itemView = p_folder->tree();
		}
		else {
			p_itemView = p_folder->list();
		}

		if (p_itemView->currentIndex().isValid()) {
			return resolveItemPath(p_itemView->currentIndex());
		}
	}
	return QString();
}

void extractArchives(TeViewStore* p_store, const QStringList & list, const QString & targetPath, bool createArchiveFolder)
{
	QDir dir;

	bool bSuccess = true;

	QFileInfo targetInfo(targetPath);

	//target confirm
	if (targetInfo.exists()) {
		if (targetInfo.isDir()) {
			//Acceptale target
		}
		else {
			//Target is not directory
			bSuccess = false;
		}
	}
	else {
		//directory is not found.
		bSuccess = false;
	}

	if (bSuccess) {
		//start extract
		QProgressDialog progress(QObject::tr(""), QObject::tr("Cancel"), 0, list.size(), p_store->mainWindow());
		progress.setWindowTitle(QObject::tr("Extract"));
		progress.setWindowModality(Qt::WindowModal);


		for (int i = 0; i < list.size(); i++) {
			QString basePath = targetPath;
			QFileInfo info(list[i]);
			if (createArchiveFolder) {
				basePath += QDir::separator() + info.baseName();
			}
			TeArchive::Reader reader;
			reader.open(list[i]);

			// Resolve the password for encrypted archives before extracting.
			bool needPassword = (reader.lastResult() == TeArchive::Reader::RESULT_PASSWORD_REQUIRED)
				|| (reader.lastResult() == TeArchive::Reader::RESULT_OK && reader.isEncrypted()
					&& reader.verifyPassword() != TeArchive::Reader::RESULT_OK);
			if (needPassword) {
				bool ok = false;
				while (true) {
					TePasswordDialog dlg(p_store->mainWindow());
					dlg.setPrompt(QObject::tr("Enter the password for \"%1\":").arg(info.fileName()));
					if (dlg.exec() != QDialog::Accepted) {
						break;
					}
					reader.setPassword(dlg.password());
					// Re-open so header-encrypted archives (e.g. 7-Zip) bind the path.
					reader.open(list[i]);
					if (reader.lastResult() == TeArchive::Reader::RESULT_OK
						&& reader.verifyPassword() == TeArchive::Reader::RESULT_OK) {
						ok = true;
						break;
					}
				}
				if (!ok) {
					bSuccess = false;
					continue;
				}
			}

			QString targetInfo = QObject::tr("Extract ") + QString::asprintf("(%d/%d) : ",i,list.size()) + info.fileName() + "\n";

			QObject::connect(&reader, &TeArchive::Reader::maximumValue, &progress, &QProgressDialog::setMaximum);
			QObject::connect(&reader, &TeArchive::Reader::valueChanged, &progress, &QProgressDialog::setValue);
			QObject::connect(&reader, &TeArchive::Reader::currentFileInfoChanged, [&progress,&targetInfo](const TeFileInfo& info) {
				progress.setLabelText(targetInfo + info.path.right(30));
				});
			QObject::connect(&reader, &TeArchive::Reader::finished, [&progress]() { progress.setValue(progress.maximum()); });
			QObject::connect(&progress, &QProgressDialog::canceled, &reader, &TeArchive::Reader::cancel);

			bSuccess = reader.extractAll(basePath);
		}
	}

	if (!bSuccess) {
		QMessageBox msg(p_store->mainWindow());
		msg.setIconPixmap(QIcon(":TableEngine/warning.png").pixmap(32, 32));
		msg.setText(QObject::tr("Extract to following path failed.") + QString("\n") + targetPath);
		msg.exec();
	}
}

QStringList extractArchiveSelectionToPath(TeViewStore* p_store, const QString& basePath, const QStringList& entries, const QString& targetPath)
{
	QStringList result;

	TeArchiveFolderView* p_arc = qobject_cast<TeArchiveFolderView*>(p_store->currentFolderView());
	if (p_arc == nullptr || p_arc->reader() == nullptr) {
		return result;
	}

	QDir dir(targetPath);
	if (!dir.exists()) {
		if (!dir.mkpath(".")) {
			return result;
		}
	}

	TeArchive::Reader* reader = p_arc->reader();
	reader->clearCancel();

	//start extract
	QProgressDialog progress(QObject::tr(""), QObject::tr("Cancel"), 0, 100, p_store->mainWindow());
	progress.setWindowTitle(QObject::tr("Extract"));
	progress.setWindowModality(Qt::WindowModal);

	QString targetInfo = QObject::tr("Extract : ") + reader->path() + "\n";

	QObject::connect(reader, &TeArchive::Reader::maximumValue, &progress, &QProgressDialog::setMaximum);
	QObject::connect(reader, &TeArchive::Reader::valueChanged, &progress, &QProgressDialog::setValue);
	QObject::connect(reader, &TeArchive::Reader::currentFileInfoChanged, [&progress,&targetInfo](const TeFileInfo& info) {
		progress.setLabelText(targetInfo + info.path.right(30));
		});	
	QObject::connect(reader, &TeArchive::Reader::finished, [&progress]() { progress.setValue(progress.maximum()); });
	QObject::connect(&progress, &QProgressDialog::canceled, reader, &TeArchive::Reader::cancel);

	auto relativePaths = entries;
	if (!basePath.isEmpty()) {
		// Convert absolute paths to relative paths for extraction.
		for (QString& path : relativePaths) {
			if (path.startsWith(basePath)) {
				path = path.mid(basePath.length());
				if (path.startsWith('/')) {
					path = path.mid(1);
				}
			}
		}
	}

	if (!reader->extract(targetPath, basePath, relativePaths)) {
		return result;
	}

	const QString base = targetPath.endsWith('/') ? targetPath : targetPath + "/";
	for (const auto& entry : relativePaths) {
		result.append(QDir::cleanPath(base + entry));
	}
	return result;
}

QStringList extractArchiveSelectionToTempPath(TeViewStore* p_store, const QStringList& entries)
{
	QStringList result;

	TeArchiveFolderView* p_arc = qobject_cast<TeArchiveFolderView*>(p_store->currentFolderView());
	if (p_arc == nullptr || p_arc->reader() == nullptr) {
		return result;
	}

	QString tempDir = p_arc->tempPath();
	if (tempDir.isEmpty()) {
		return result;
	}

	return extractArchiveSelectionToPath(p_store, QString(), entries, tempDir);
}

QString getCurrentFolder(TeViewStore* p_store)
{
	TeFolderView* p_folder = p_store->currentFolderView();
	if (p_folder != nullptr) {
		return p_folder->currentPath();
	}
	return QString();
}

QStringList getFavorites()
{
	QSettings settings;
	QStringList favorites;
	settings.beginGroup(SETTING_FAVORITES);
	for (const auto& key : settings.childKeys()) {
		favorites.append(settings.value(key).toString());
	}
	settings.endGroup();
	return favorites;
}

void updateFavorites(const QStringList& list)
{
	QSettings settings;
	settings.beginGroup(SETTING_FAVORITES);
	settings.remove("");
	for (int i = 0; i < list.size(); i++) {
		settings.setValue(QString("path%1").arg(i, 2, 10, u'0'), list.at(i));
	}
	settings.endGroup();
}

bool isDir(const QModelIndex& index)
{
	if (index.isValid()) {
		QVariant var = index.data(QFileSystemModel::FileInfoRole);
		Q_ASSERT(var.isValid() && var.canConvert<QFileInfo>());
		QFileInfo fileInfo = qvariant_cast<QFileInfo>(var);
		return fileInfo.isDir();
	}
	return false;
}

/*!
 * \brief Get file type.
 * \param path file path.
 * \return file type.
 * 
 * Analyze file type by suffix and libmagic.
 */
TeFileType getFileType(const QString& path)
{
	TeFileType type = TE_FILE_UNKNOWN;
	QFileInfo fileInfo(path);

	//lazy check by suffix
	if (fileInfo.isDir()) {
		type = TE_FILE_FOLDER;
	} else if (txtSuffixes.contains(fileInfo.suffix().toLower())) {
		type = TE_FILE_TEXT;
	}
	else if (imageSuffixes.contains(fileInfo.suffix().toLower())) {
		type = TE_FILE_IMAGE;
	}
	else if (archiveSuffixes.contains(fileInfo.suffix().toLower())) {
		type = TE_FILE_ARCHIVE;
	}

	if (type != TE_FILE_UNKNOWN){
		return type;
	}

	if (fileInfo.isFile() && fileInfo.size() > 0) {
		QMimeDatabase db;

		QMimeType mime = db.mimeTypeForFile(fileInfo);
		QString typeStr = mime.name();
		if (mime.inherits("text/plain")) {
			// The file is plain text, we can display it in a QTextEdit
		}

		if (typeStr.startsWith("text/")) {
			type = TE_FILE_TEXT;
		}
		else if (typeStr.startsWith("image/")) {
			type = TE_FILE_IMAGE;
		}
		else if (typeStr.startsWith("application/zip")) {
			type = TE_FILE_ARCHIVE;
		}
	}

	return type;
}


/*!
*  \brief Detect text codec from data uing ICU
*  \param data Data to detect
*  \param codecList List of codec names to detect
*  \return The name of the codec
*/
QString detectTextCodec(const QByteArray& data, const QStringList& codecList) {
	const char* const kDefaultCodec = "UTF-8";

	UErrorCode status = U_ZERO_ERROR;
	UCharsetDetector* detector = ucsdet_open(&status);
	if (U_FAILURE(status)) {
		return QString(kDefaultCodec);
	}
	ucsdet_setText(detector, data.constData(), data.size(), &status);
	if (U_FAILURE(status)) {
		ucsdet_close(detector);
		return QString(kDefaultCodec);
	}

	int count = 0;
	const UCharsetMatch** matchs = ucsdet_detectAll(detector, &count, &status);
	for (int i = 0; i < count; i++) {
		const char* name = ucsdet_getName(matchs[i], &status);
		if (U_FAILURE(status)) {
			ucsdet_close(detector);
			return QString(kDefaultCodec);
		}
		if (codecList.contains(name)) {
			ucsdet_close(detector);
			return QString::fromUtf8(name);
		}
	}

	ucsdet_close(detector);
	return QString(kDefaultCodec);
}

namespace {
constexpr int kTextSampleBytes = 1024;
// Minimum sample length required before the NUL-lane heuristic below is
// trusted; shorter samples don't carry enough signal to avoid false positives.
constexpr qsizetype kMinUtf16HeuristicBytes = 8;

// Validates that data fully decodes under the named codec. The sample is a
// bounded prefix of the file (see kTextSampleBytes), so a multi-byte sequence
// may be cut off at the very end; that trailing truncation is expected and
// must not be treated as an encoding error.
bool isValidTextEncoding(const QByteArray& data, const QString& codec)
{
	UErrorCode status = U_ZERO_ERROR;
	const QByteArray codecName = codec.toLatin1();
	UConverter* converter = ucnv_open(codecName.constData(), &status);
	if (U_FAILURE(status)) {
		return false;
	}

	UConverterToUCallback oldAction = nullptr;
	const void* oldContext = nullptr;
	ucnv_setToUCallBack(converter, UCNV_TO_U_CALLBACK_STOP, nullptr,
		&oldAction, &oldContext, &status);
	QVector<UChar> output(data.size() * 2 + 1);
	ucnv_toUChars(converter, output.data(), output.size(), data.constData(), data.size(), &status);
	ucnv_close(converter);
	return U_SUCCESS(status) || status == U_TRUNCATED_CHAR_FOUND;
}

QString detectTextCodecStrict(const QByteArray& data, const QStringList& codecList)
{
	UErrorCode status = U_ZERO_ERROR;
	UCharsetDetector* detector = ucsdet_open(&status);
	if (U_FAILURE(status)) {
		return QString();
	}

	ucsdet_setText(detector, data.constData(), data.size(), &status);
	if (U_FAILURE(status)) {
		ucsdet_close(detector);
		return QString();
	}

	int count = 0;
	const UCharsetMatch** matches = ucsdet_detectAll(detector, &count, &status);
	QString result;
	for (int i = 0; i < count && U_SUCCESS(status); ++i) {
		const char* name = ucsdet_getName(matches[i], &status);
		const int confidence = ucsdet_getConfidence(matches[i], &status);
		if (U_SUCCESS(status) && confidence >= 50 && codecList.contains(QString::fromUtf8(name), Qt::CaseInsensitive)) {
			result = QString::fromUtf8(name);
			break;
		}
	}

	ucsdet_close(detector);
	return result;
}

// Recognizes UTF-16 samples that lack a byte-order mark. BOM-less UTF-16 text
// dominated by ASCII/Latin characters has a NUL byte in every other position
// (the widened high byte); this only holds reliably for a long-enough sample
// with one lane consistently NUL and the other lane essentially NUL-free, so
// both conditions are required to avoid mistaking small binaries for UTF-16.
bool looksLikeBomLessUtf16(const QByteArray& data)
{
	if (data.size() < kMinUtf16HeuristicBytes) {
		return false;
	}

	int evenPositionNuls = 0;
	int oddPositionNuls = 0;
	for (qsizetype i = 0; i < data.size(); ++i) {
		if (data.at(i) == '\0') {
			(i % 2 == 0 ? evenPositionNuls : oddPositionNuls)++;
		}
	}

	const qsizetype pairs = data.size() / 2;
	const bool evenLaneDominant = evenPositionNuls >= pairs * 0.4 && oddPositionNuls <= pairs * 0.05;
	const bool oddLaneDominant = oddPositionNuls >= pairs * 0.4 && evenPositionNuls <= pairs * 0.05;
	return evenLaneDominant || oddLaneDominant;
}

bool hasControlByte(const QByteArray& data)
{
	for (const auto byte : data) {
		const auto value = static_cast<unsigned char>(byte);
		if (value == 0 || (value < 0x20 && value != '\t' && value != '\n'
			&& value != '\r' && value != '\f')) {
			return true;
		}
	}
	return false;
}
}

bool isTextFile(const QString& path)
{
	if (QFileInfo(path).isDir()) {
		return false;
	}

	QFile file(path);
	if (!file.open(QIODevice::ReadOnly)) {
		return false;
	}

	const QByteArray data = file.read(kTextSampleBytes);
	if (data.isEmpty()) {
		return true;
	}

	const bool hasUtf16Bom = data.startsWith("\xFF\xFE") || data.startsWith("\xFE\xFF");
	if (hasUtf16Bom || looksLikeBomLessUtf16(data)) {
		// UTF-16 is checked in its own branch so that binary or Shift-JIS/
		// EUC-JP samples can never be misclassified through an overly
		// permissive UTF-16 fallback (see review notes).
		return isValidTextEncoding(data, "UTF-16LE") || isValidTextEncoding(data, "UTF-16BE");
	}

	if (hasControlByte(data)) {
		return false;
	}

	const QStringList codecs = { "UTF-8", "Shift_JIS", "EUC-JP" };
	if (!detectTextCodecStrict(data, codecs).isEmpty()) {
		return true;
	}

	for (const QString& codec : codecs) {
		if (isValidTextEncoding(data, codec)) {
			return true;
		}
	}
	return false;
}
