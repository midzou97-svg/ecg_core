qt_add_library(EcgRing STATIC
    RingEcgSource.h
    RingEcgSource.cpp
    RingSourceApi.h
    RingSourceApi.cpp
)

target_link_libraries(EcgRing
    PUBLIC
        EcgCore
        Qt6::Core
        QCoro6::Core
)

target_include_directories(EcgRing
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
