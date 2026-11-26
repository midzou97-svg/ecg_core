qt_add_executable(EcgCoreSplitApp
    main.cpp
)

target_link_libraries(EcgCoreSplitApp
    PRIVATE
        EcgCore
        EcgRing
        EcgKrp
        Qt6::Core
        QCoro6::Core
)
