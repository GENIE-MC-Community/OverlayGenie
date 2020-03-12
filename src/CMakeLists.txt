add_executable(overlay_genie overlay_genie.cxx event_source.cxx)
target_link_libraries(overlay_genie ${GENIE_LIBS_DIR_WITH_LFLAGS} ${ROOT_LIBRARIES} ${log4cpp} ${pythia6} ${libxml2})

install( TARGETS overlay_genie RUNTIME DESTINATION bin )
