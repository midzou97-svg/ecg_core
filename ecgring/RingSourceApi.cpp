#include "RingSourceApi.h"
#include "RingEcgSource.h"

namespace ecg::ring {

IEcgSource *createSource(QObject *parent) { return new RingEcgSource(parent); }

} // namespace ecg::ring
