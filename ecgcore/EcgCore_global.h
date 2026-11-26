#pragma once

#include <QtCore/QtGlobal>

#if defined(ECGCORE_LIBRARY)
#  define ECGCORE_EXPORT Q_DECL_EXPORT
#else
#  define ECGCORE_EXPORT Q_DECL_IMPORT
#endif
