#pragma once
#include <QString>



extern void showFileContext( int px, int py, const QString& path);
extern void openFile(const QString& path);
extern bool copyFiles(const QStringList& files, const QString& path);
extern bool copyFile(const QString& fromFile, const QString& toFile);
extern bool moveFiles(const QStringList& files, const QString& path);
extern bool deleteFiles(const QStringList& files);

extern bool threadInitialize();
extern void threadUninitialize();
