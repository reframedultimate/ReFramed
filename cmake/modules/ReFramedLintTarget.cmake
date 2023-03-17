function (reframed_lint_target)
    find_package (Python3 COMPONENTS Interpreter)
    if (NOT Python3_FOUND)
        message (WARNING "Could not find python interpreter, linting will be disabled")
        return ()
    endif ()

    add_custom_target (ReFramedLint ALL
        COMMAND Python3::Interpreter ${REFRAMED_SOURCE_ROOT}/scripts/lint.py
        WORKING_DIRECTORY ${REFRAMED_SOURCE_ROOT}
        COMMENT "Running lint..."
        VERBATIM)
endfunction ()

