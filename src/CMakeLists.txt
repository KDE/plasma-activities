# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

# =======================================================
# Starting the actual project definition

# The libraries do not depend on any compile-time features
add_subdirectory (lib)

if (NOT PLASMA_ACTIVITIES_LIBRARY_ONLY)
   include_directories (
      ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}
      )
   add_subdirectory (imports)
endif ()

add_subdirectory (cli)

ecm_qt_install_logging_categories(
    EXPORT PLASMA_ACTIVITIES
    FILE plasma-activities.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
