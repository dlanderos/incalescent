cmake_minimum_required(VERSION 3.26)
project(incalescent C)

set(CMAKE_C_STANDARD 17)
set(SOURCE_FILES
        main.c
        main.h
        log.c
        log.h
        string.c
        string.h
        file.c
        file.h
        dialog.c
        dialog.h
        generated_error.h
        generated_error.rc
        icon.rc
)

add_executable(incalescent ${SOURCE_FILES})
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:WinMainCRTStartup /CLRTHREADATTRIBUTE:STA")
target_link_libraries(incalescent Shlwapi)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(incalescent PRIVATE
            /Zi
            /Wall
            /DEBUG
            /RTC1
            /Fd"${CMAKE_BINARY_DIR}/incalescent.pdb"
    )
    target_compile_definitions(incalescent PRIVATE DEBUG)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Set specific options for the Release build here
    target_compile_options(incalescent PRIVATE
            /O2
            /DNDEBUG
            /GL
    )
    set_target_properties(incalescent PROPERTIES
            LINK_FLAGS_RELEASE "/LTCG"
    )
endif()
