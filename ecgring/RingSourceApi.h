#pragma once

#include <QObject>

#include "IEcgSource.h"

namespace ecg::ring {

IEcgSource* createSource(QObject* parent = nullptr);

} // namespace ecg::ring
