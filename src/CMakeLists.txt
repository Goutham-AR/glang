set(
    SRC_FILES
        main.c
        bytecode.c
        value.c
        vm.c
        compiler.c
        scanner.c
        parser.c
        object.c
        memory.c
        table.c
        debug.c
        utils.c
        repl.c
)

set(
    HEADER_FILES
        common.h
        instructions.h
        bytecode.h
        value.h
        vm.h
        compiler.h
        scanner.h
        parser.h
        object.h
        memory.h
        table.h
        debug.h
        utils.h
        repl.h
)

add_executable(
    solv
        ${SRC_FILES}
        ${HEADER_FILES}
)

target_link_libraries(solv m)
