if(EXISTS icon.ico AND NOT EXISTS icon.svg)
    # Inform user of existing icon.ico
    message(WARNING "Found icon.ico, delete it to regenerate")
else()
    # Requirement check
    find_program(INKSCAPE_PROGRAM inkscapecom.com)
    find_program(MAGICK_PROGRAM magick)

    if(NOT INKSCAPE_PROGRAM OR NOT MAGICK_PROGRAM)
        set(MISSING_PROGRAM "Missing program(s):")
        if(NOT INKSCAPE_PROGRAM)
            set(MISSING_PROGRAM "${MISSING_PROGRAM}\n\tinkscapecom.com - https://inkscape.org/release/1.2.2/windows")
        endif()
        if(NOT MAGICK_PROGRAM)
            set(MISSING_PROGRAM "${MISSING_PROGRAM}\n\tmagick.exe - https://imagemagick.org/script/download.php#windows")
        endif()
        message(FATAL_ERROR ${MISSING_PROGRAM})
    endif()

    # Download icon.svg
    if(NOT EXISTS icon.svg)
        message(WARNING "Downloading default icon.svg, custom icon.svg can be used if placed in project directory")

        # Prepare downloader
        find_program(ARIA2 aria2c)
        set(DOWNLOADER ${ARIA2})
        if(NOT ARIA2)
            find_program(CURL curl REQUIRED)
            set(DOWNLOADER ${CURL})
        endif()
        execute_process(
            COMMAND "${DOWNLOADER}" -o icon.svg https://upload.wikimedia.org/wikipedia/commons/e/ec/Orologio_viola.svg
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            ERROR_QUIET
        )
    endif()

    # Size guidelines from Microsoft
    # https://learn.microsoft.com/en-us/windows/win32/uxguide/vis-icons?redirectedfrom=MSDN#size-requirements

    # Recomended steps from
    # https://graphicdesign.stackexchange.com/a/77466

    # SVG to multiple PNG
    if(NOT EXISTS icon.svg)
        message(FATAL_ERROR "Failed to download icon.svg")
    endif()

    # NOTE: Stacking multiple commands in a single execute_process doesn't seem to work
    execute_process(
        COMMAND "${INKSCAPE_PROGRAM}" -w 16 -h 16 --export-filename=16.png icon.svg
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE ERRORLEVEL
    )

    if(ERRORLEVEL EQUAL 0)
        execute_process(
            COMMAND "${INKSCAPE_PROGRAM}" -w 32 -h 32 --export-filename=32.png icon.svg
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE ERRORLEVEL
        )
    endif()

    if(ERRORLEVEL EQUAL 0)
        execute_process(
            COMMAND "${INKSCAPE_PROGRAM}" -w 48 -h 48 --export-filename=48.png icon.svg
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE ERRORLEVEL
        )
    endif()

    if(ERRORLEVEL EQUAL 0)
        execute_process(
            COMMAND "${INKSCAPE_PROGRAM}" -w 64 -h 64 --export-filename=64.png icon.svg
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE ERRORLEVEL
        )
    endif()

    if(ERRORLEVEL EQUAL 0)
        execute_process(
            COMMAND "${INKSCAPE_PROGRAM}" -w 256 -h 256 --export-filename=256.png icon.svg
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE ERRORLEVEL
        )
    endif()

    # Consolidate multiple PNG into ICO
    if(ERRORLEVEL EQUAL 0)
        execute_process(
            COMMAND "${MAGICK_PROGRAM}" 16.png 32.png 48.png 64.png 256.png icon.ico
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE ERRORLEVEL
        )
    endif()

    # Remove intermediate files (skip ERRORLEVEL)
    if(ERRORLEVEL EQUAL 0)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E rm -f 16.png 32.png 48.png 64.png 256.png icon.svg
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            ERROR_QUIET
        )
    endif()

    if(ERRORLEVEL)
        message(FATAL_ERROR "Failed to generate icon.ico")
    endif()
endif()