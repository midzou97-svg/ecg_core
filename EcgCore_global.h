#ifndef ECGCORE_GLOBAL_H
#define ECGCORE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(ECGCORE_LIBRARY)
#define ECGCORE_EXPORT Q_DECL_EXPORT
#else
#define ECGCORE_EXPORT Q_DECL_IMPORT
#endif

#endif // ECGCORE_GLOBAL_H
