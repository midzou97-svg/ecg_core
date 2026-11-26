#pragma once

#include <QObject>

#include "IEcgSource.h"

namespace ecg::krp {

IEcgSource* createSource(QObject* parent = nullptr);

} // namespace ecg::krp
