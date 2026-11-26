qt_add_library(EcgCore SHARED
    EcgCore_global.h
    ecg_types.h
    IEcgSource.h
    EcgSourceFactory.h
    EcgSourceFactory.cpp
    RingEcgSource.h
    RingEcgSource.cpp
    KrpEcgSource.h
    KrpEcgSource.cpp
    IEcgDeviceDiscovery.h
)

target_link_libraries(EcgCore
    PUBLIC
        Qt6::Core
        QCoro6::Core
)

target_compile_definitions(EcgCore PRIVATE ECGCORE_LIBRARY)

target_include_directories(EcgCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
