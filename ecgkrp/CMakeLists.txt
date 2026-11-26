qt_add_library(EcgKrp STATIC
    KrpEcgSource.h
    KrpEcgSource.cpp
    KrpSourceApi.h
    KrpSourceApi.cpp
)

target_link_libraries(EcgKrp
    PUBLIC
        EcgCore
        Qt6::Core
        QCoro6::Core
)

target_include_directories(EcgKrp
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
