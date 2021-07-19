# .cpp files are compiled with extra ${WSHADOW_FLAGS}
if( COMPILER_SUPPORTS_WSHADOW )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WSHADOW_FLAGS}")
endif()
if( BUILD_GITHUB_PLUGIN )
    set( GITHUB_PLUGIN_LIBRARIES github_plugin )
endif()

include_directories( BEFORE ${INC_BEFORE} )
include_directories(
    ../3d-viewer
    ../exporters
    ../pcbnew
    ../utils/idftools
    ../pcbnew/exporters
    ../pcbnew/dialogs
    ${INC_AFTER}
    )

add_definitions(-DPCBNEW)
set( KICAD2VRML_SRCS
    kicad2vrml.cpp
    ../pcbnew/exporters/export_vrml.cpp
    ../pcbnew/zone_filler.cpp
    ../pcbnew/board_commit.cpp
    ../pcbnew/tools/zone_filler_tool.cpp
    ../pcbnew/tools/pcb_tool.cpp
    ../pcbnew/tools/tool_event_utils.cpp
    ../common/base_units.cpp
    )

if( MINGW )
    # KICAD2VRML_RESOURCES variable is set by the macro.
    mingw_resource_compiler( kicad2vrml )
else()
    set( KICAD2VRML_RESOURCES kicad2vrml.rc )
endif()

if( APPLE )
    # setup bundle
    set( MACOSX_BUNDLE_GUI_IDENTIFIER org.kicad-pcb.kicad )
    set( MACOSX_BUNDLE_NAME kicad2vrml )
endif()

add_executable( kicad2vrml MACOSX_BUNDLE
    ${KICAD2VRML_SRCS}
    ${KICAD2VRML_RESOURCES}
    )

target_link_libraries( kicad2vrml
    3d-viewer
    connectivity
    pcbcommon
    pnsrouter
    pcad2kicadpcb
    legacy_wx
    common
    gal
    lib_dxf
    idf3
    ${wxWidgets_LIBRARIES}
    ${GITHUB_PLUGIN_LIBRARIES}
    ${GDI_PLUS_LIBRARIES}
    ${PYTHON_LIBRARIES}
    ${Boost_LIBRARIES}      # must follow GITHUB
    ${PCBNEW_EXTRA_LIBS}    # -lrt must follow Boost
    )

if( APPLE )
    set_target_properties( kicad2vrml PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST ${PROJECT_BINARY_DIR}/kicad2vrml/Info.plist
        )

    # put individual bundle outside of main bundle as a first step
    # will be pulled into the main bundle when creating main bundle
    install( TARGETS kicad2vrml
        DESTINATION ${KICAD_BIN}
        COMPONENT binary
        )
    install( CODE "
        # override default embedded path settings
        ${OSX_BUNDLE_OVERRIDE_PATHS}

        # do all the work
        include( BundleUtilities )
        fixup_bundle( ${KICAD_BIN}/kicad2vrml.app/Contents/MacOS/kicad2vrml
            \"\"
            \"\"
            )
        " COMPONENT Runtime
        )
else()
    install( TARGETS kicad2vrml
        DESTINATION ${KICAD_BIN}
        COMPONENT binary
        )
endif()


if( false )     # linker map with cross reference
    set_target_properties( kicad2vrml PROPERTIES
        LINK_FLAGS "${TO_LINKER},-cref ${TO_LINKER},-Map=kicad2vrml.map"
        )
endif()
