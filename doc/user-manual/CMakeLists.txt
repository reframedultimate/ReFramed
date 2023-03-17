project ("user-manual"
    LANGUAGES)

find_package (LATEX COMPONENTS PDFLATEX)
if (NOT LATEX_FOUND)
    message (WARNING "LaTeX was not found. User manual will not be generated.")
    return ()
endif ()

# We need to keep track of any changes to .tex files
file (GLOB_RECURSE TEX_FILES "*.tex")

# The name of the main tex without extension
set (MAIN_TEX "user-manual")
set (INTERMEDIATE_PDF "${PROJECT_BINARY_DIR}/${MAIN_TEX}.pdf")
set (FINAL_PDF "${REFRAMED_BUILD_DOCDIR}/${MAIN_TEX}.pdf")

# Two-pass compilation, then move the final PDF into the build directory.
# Texlive does not have the -aux-directory option so we have to compile
# the doc in a different location and then move the PDF afterwards to not
# clutter up the final directory
add_custom_command (OUTPUT ${FINAL_PDF}
    COMMAND ${PDFLATEX_COMPILER} -output-directory ${PROJECT_BINARY_DIR} -draftmode -interaction=nonstopmode ${MAIN_TEX}.tex
    COMMAND ${PDFLATEX_COMPILER} -output-directory ${PROJECT_BINARY_DIR} -interaction=nonstopmode ${MAIN_TEX}.tex
    COMMAND ${CMAKE_COMMAND} -E make_directory ${REFRAMED_BUILD_DOCDIR}
    COMMAND ${CMAKE_COMMAND} -E rename ${INTERMEDIATE_PDF} ${FINAL_PDF}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    DEPENDS ${TEX_FILES}
    COMMENT "Compiling User Manual"
    VERBATIM)
add_custom_target (user-manual ALL
    DEPENDS ${FINAL_PDF})

# Since the application has an option to open the user manual PDF,
# ensure the documentation gets built as a dependency of the application
add_dependencies (application user-manual)

install (
    FILES ${FINAL_PDF}
    DESTINATION ${REFRAMED_INSTALL_DOCDIR})

