#include "EcgSourceFactory.h"
#include "RingEcgSource.h"
#include "KrpEcgSource.h"

namespace ecg {

IEcgSource* EcgSourceFactory::create(SourceType type, QObject* parent)
{
    switch (type) {
    case SourceType::Ring:
        return new RingEcgSource(parent);

    case SourceType::Krp01:
        return new KrpEcgSource(parent);
    }

    return nullptr;
}

bool EcgSourceFactory::isAvailable(SourceType type)
{
    return type == SourceType::Ring || type == SourceType::Krp01;
}

QString EcgSourceFactory::sourceName(SourceType type)
{
    switch (type) {
    case SourceType::Ring:
        return QStringLiteral("ЭКГ-кольцо");

    case SourceType::Krp01:
        return QStringLiteral("КРП-01");
    }

    return QStringLiteral("Неизвестный источник");
}

} // namespace ecg
