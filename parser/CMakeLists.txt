# Parser subdirectory CMakeLists.txt

# Generated SocialGaming parser.c
# (run "tree-sitter generate" in your grammar repo to regenerate)
add_library(ts_socialgaming STATIC
    tree_sitter/parser.c
)

# It uses the Tree-sitter headers
target_include_directories(ts_socialgaming PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(ts_socialgaming PUBLIC
    ${tree-sitter_SOURCE_DIR}/lib/include
)

target_link_libraries(ts_socialgaming PUBLIC treesitter)

# Our C++ loader for the grammar
add_library(parser STATIC
    GameSpecLoader.cpp
)

# Add the project root as an include dir so we can #include "parser/GameSpecLoader.h"
# and also expose tree-sitter headers to this target.
target_include_directories(parser PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${tree-sitter_SOURCE_DIR}/lib/include
)

# Link the generated grammar into our loader
target_link_libraries(parser PUBLIC ts_socialgaming)
