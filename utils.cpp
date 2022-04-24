#include "utils.h"
#include <qglobal.h>
#include <cstdlib>

void setYosysEnv()
{
#ifdef Q_OS_WIN
    std::string path = getenv("PATH");
//    _putenv_s();
#endif
}
