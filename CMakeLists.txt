cmake_minimum_required(VERSION 3.13)
project(hust)

math(EXPR stack_size "16*1024*1024")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")

if (UNIX)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,stack-size=${stack_size}")
else ()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--stack,${stack_size}")
endif ()

add_executable(hust
        main.cpp
		lexer/lexer.h
		lexer/lexer.cpp
		lexer/tokenInfo.cpp
    	syntax/analyse.cpp
    	syntax/analyse.h
    	syntax/parseNode.cpp
    	syntax/parseNode.h
    	syntax/symbolTable.cpp
    	syntax/symbolTable.h
		syntax/Util.h
		syntax/Util.cpp
		Ir/IR.h
		Ir/IR.cpp
		Ir/IRItem.h
		Ir/IRItem.cpp
		Ir/IRBuild.h
		Ir/IRBuild.cpp
		Ir/Attribute.h
		Ir/Attribute.cpp
		assembly/Parameter.h
		assembly/Parameter.cpp
		assembly/assembler.h
		assembly/assembler.cpp
		assembly/allocater.h
		assembly/allocater.cpp
		)
