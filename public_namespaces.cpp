#include "public_namespaces.h"
namespace paths {
#ifndef NDEBUG
    const QString ImageMagickPath = R"(E:\ImageMagick-7.0.7-31-portable-Q16-x86\)";
    const QString UCIENCPath = R"(D:\uci\)";
#else
    const QString ImageMagickPath = R"(.\ImageMagick\)";
    const QString UCIENCPath = R"(.\UCIENC\)";
#endif
}
