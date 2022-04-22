set(
    SRC_FILES 
        main.cc
        ByteCode.cc
        Value.cc

        debug.cc
)


set(
    HEADER_FILES
        log.hh
	    common.hh
        ByteCode.hh
        Value.hh


        
        debug.hh
)

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
