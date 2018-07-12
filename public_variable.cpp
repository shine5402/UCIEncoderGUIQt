#include "public_variable.h"
namespace paths {
#ifndef NDEBUG
    const QString ImageMagickPath = R"(E:\ImageMagick\)";
    const QString UCIENCPath = R"(D:\uci\)";
#else
    const QString ImageMagickPath = R"(.\ImageMagick\)";
    const QString UCIENCPath = R"(.\UCIENC\)";
#endif
}
bool isToolchain_x64 = true;
