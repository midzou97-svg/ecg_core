#include "KrpSourceApi.h"
#include "KrpEcgSource.h"

namespace ecg::krp {

IEcgSource* createSource(QObject* parent)
{
    return new KrpEcgSource(parent);
}

} // namespace ecg::krp
