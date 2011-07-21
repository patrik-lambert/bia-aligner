PROJECT(bia)


include_directories(BEFORE ${CMAKE_SOURCE_DIR} )

SET (assoScoreTable_from-snt_SRCS
${CMAKE_SOURCE_DIR}/training/assoScoreTable_from-snt.cpp
)

SET (linkCosts_SRCS
${CMAKE_SOURCE_DIR}/training/linkCosts.cpp ${CMAKE_SOURCE_DIR}/training/linkCluster.cpp ${CMAKE_SOURCE_DIR}/training/phrase.cpp ${CMAKE_SOURCE_DIR}/training/alUnit.cpp ${CMAKE_SOURCE_DIR}/training/iword.cpp
)

SET (prune-assoScoreTable_SRCS
 prune-assoScoreTable.cpp alUnit.cpp
)

SET(detect-matchStemSyn_SRCS
detect-matchStemSyn.cpp sentPair.cpp iword.cpp alUnit.cpp
)

add_executable(assoScoreTable_from-snt ${assoScoreTable_from-snt_SRCS})
add_executable(linkCosts ${linkCosts_SRCS})
add_executable(prune-assoScoreTable ${prune-assoScoreTable_SRCS})

install(TARGETS assoScoreTable_from-snt linkCosts prune-assoScoreTable DESTINATION ${CMAKE_SOURCE_DIR}/training)

