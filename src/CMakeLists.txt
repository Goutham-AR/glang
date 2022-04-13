set(SRC_FILES main.cc)
set(HEADER_FILES)

add_executable(
    glang
        ${SRC_FILES}
        ${HEADER_FILES}
)


target_link_libraries(
    glang
    PRIVATE
        fmt
)