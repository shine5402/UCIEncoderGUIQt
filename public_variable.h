#ifndef PUBLIC_NAMESPACES_H
#define PUBLIC_NAMESPACES_H
#include <QObject>
#include <QString>
#include <QFile>
#include <QFileInfo>
namespace paths {
    extern const QString ImageMagickPath;
    extern const QString UCIENCPath;
}
#ifdef Q_OS_WIN64
    extern bool isToolchain_x64;
#endif
#endif // PUBLIC_NAMESPACES_H
