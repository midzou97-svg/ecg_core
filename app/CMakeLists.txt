qt_add_executable(EcgCoreDemoApp
    main.cpp
)

target_link_libraries(EcgCoreDemoApp
    PRIVATE
        EcgCore
        Qt6::Core
        QCoro6::Core
)
