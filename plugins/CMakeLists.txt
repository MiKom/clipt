FOREACH ( plugin_folder
	io_pnm
	node_histogram
	)
	ADD_SUBDIRECTORY(${plugin_folder})
ENDFOREACH()
