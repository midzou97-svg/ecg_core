#pragma once

#include "EcgCore_global.h"
#include "IEcgSource.h"

namespace ecg {

class ECGCORE_EXPORT EcgSourceFactory {
public:
    static IEcgSource* create(SourceType type, QObject* parent = nullptr);
    static bool isAvailable(SourceType type);
    static QString sourceName(SourceType type);
};

} // namespace ecg
