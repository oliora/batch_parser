set(SOURCES
    main.cpp
    batch_parser.h
)

include_directories(${Boost_INCLUDE_DIRS})

add_executable(batch_parser ${SOURCES})
#target_link_libraries(batch_parser Shlwapi Psapi)
#target_link_libraries(batch_parser ${Boost_LIBRARIES})
