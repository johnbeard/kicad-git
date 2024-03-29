include_directories( BEFORE ${INC_BEFORE} )
include_directories(
    ./dialogs
    ./dialog_about
    ${CAIRO_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
    ../3d-viewer
    ../pcbnew
    ../polygon
    ${INC_AFTER}
    )

# Generate header files containing shader programs
# Order of input files is significant
add_custom_command(
    OUTPUT gal/opengl/shader_src.h
    COMMAND ${CMAKE_COMMAND}
        -DinputFiles="${PROJECT_SOURCE_DIR}/common/gal/opengl/shader.vert\\;${PROJECT_SOURCE_DIR}/common/gal/opengl/shader.frag"
        -DoutputFile="shader_src.h"
        -P ${CMAKE_MODULE_PATH}/Shaders.cmake
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/common/gal/opengl
    COMMENT "Generating headers containing GLSL source code"
)

add_custom_target(
    shader_headers ALL
    DEPENDS gal/opengl/shader_src.h
)

set( GAL_SRCS
    # Common part
    drawpanel_gal.cpp
    painter.cpp
    worksheet_viewitem.cpp
    gal/graphics_abstraction_layer.cpp
    gal/stroke_font.cpp
    gal/color4d.cpp
    view/wx_view_controls.cpp
    geometry/hetriang.cpp

    # OpenGL GAL
    gal/opengl/opengl_gal.cpp
    gal/opengl/shader.cpp
    gal/opengl/vertex_item.cpp
    gal/opengl/vertex_container.cpp
    gal/opengl/cached_container.cpp
    gal/opengl/noncached_container.cpp
    gal/opengl/vertex_manager.cpp
    gal/opengl/gpu_manager.cpp
    gal/opengl/opengl_compositor.cpp

    # Cairo GAL
    gal/cairo/cairo_gal.cpp
    gal/cairo/cairo_compositor.cpp
    )

add_library( gal STATIC ${GAL_SRCS} )
add_dependencies( gal shader_headers )

add_dependencies( gal lib-dependencies )
add_dependencies( shader_headers lib-dependencies )


# Only for win32 cross compilation using MXE
if( WIN32 AND MSYS )
    add_definitions( -DGLEW_STATIC )
endif()

set( COMMON_ABOUT_DLG_SRCS
    dialog_about/AboutDialog_main.cpp
    dialog_about/dialog_about.cpp
    dialog_about/dialog_about_base.cpp
    dialogs/dialog_display_info_HTML_base.cpp
    dialogs/dialog_exit_base.cpp
    dialogs/dialog_image_editor.cpp
    dialogs/dialog_image_editor_base.cpp
    dialogs/dialog_get_component.cpp
    dialogs/dialog_get_component_base.cpp
    dialogs/dialog_hotkeys_editor.cpp
    dialogs/dialog_hotkeys_editor_base.cpp
    dialogs/dialog_list_selector_base.cpp
    dialogs/dialog_page_settings_base.cpp
    )

set( COMMON_PAGE_LAYOUT_SRCS
    page_layout/title_block_shapes.cpp
    page_layout/class_worksheet_dataitem.cpp
    page_layout/class_worksheet_layout.cpp
    page_layout/page_layout_default_description.cpp
    page_layout/page_layout_graphic_items.cpp
    page_layout/page_layout_reader_keywords.cpp
    page_layout/page_layout_reader.cpp
    )

set( COMMON_SRCS
    ${COMMON_ABOUT_DLG_SRCS}
    ${COMMON_PAGE_LAYOUT_SRCS}
    base_struct.cpp
    basicframe.cpp
    bezier_curves.cpp
    bitmap.cpp
    block_commande.cpp
    build_version.cpp
    class_bitmap_base.cpp
    class_colors_design_settings.cpp
    class_layer_box_selector.cpp
    class_marker_base.cpp
    class_plotter.cpp
    class_undoredo_container.cpp
    common.cpp
    common_plot_functions.cpp
    common_plotHPGL_functions.cpp
    common_plotPS_functions.cpp
    common_plotPDF_functions.cpp
    common_plotGERBER_functions.cpp
    common_plotDXF_functions.cpp
    common_plotSVG_functions.cpp
    confirm.cpp
    copy_to_clipboard.cpp
    dialog_shim.cpp
    displlst.cpp
    dlist.cpp
    drawframe.cpp
    drawpanel.cpp
    drawtxt.cpp
    dsnlexer.cpp
    eda_dde.cpp
    eda_doc.cpp
    filter_reader.cpp
    gestfich.cpp
    getrunningmicrosecs.cpp
    grid_tricks.cpp
    gr_basic.cpp
    hotkeys_basic.cpp
    hotkey_grid_table.cpp
    html_messagebox.cpp
    kiway.cpp
    msgpanel.cpp
    netlist_keywords.cpp
    newstroke_font.cpp
    projet_config.cpp
    ptree.cpp
    reporter.cpp
    richio.cpp
    selcolor.cpp
    string.cpp
    trigo.cpp
    utf8.cpp
    wildcards_and_files_ext.cpp
    worksheet.cpp
    wxwineda.cpp
    xnode.cpp
    zoom.cpp
    )

# We will not want edaappl.cpp linked into the KIFACE, only into the KIWAY.
if( TRUE OR NOT USE_KIWAY_DLLS )
    list( APPEND COMMON_SRCS edaappl.cpp )
endif()

if( NOT HAVE_STRTOKR )
    list( APPEND COMMON_SRCS strtok_r.c )
endif()


set( COMMON_SRCS
    ${COMMON_SRCS}
    view/view.cpp
    view/view_item.cpp
    view/view_group.cpp

    math/math_util.cpp

    tool/tool_base.cpp
    tool/tool_manager.cpp
    tool/tool_dispatcher.cpp
    tool/tool_event.cpp
    tool/tool_interactive.cpp
    tool/action_manager.cpp
    tool/context_menu.cpp

    geometry/seg.cpp
    geometry/shape_line_chain.cpp
    geometry/shape_collisions.cpp
    geometry/shape_index.cpp
)


add_library( common STATIC ${COMMON_SRCS} )
add_dependencies( common lib-dependencies )

set( PCB_COMMON_SRCS
    base_screen.cpp
    eda_text.cpp
    class_page_info.cpp
    convert_basic_shapes_to_polygon.cpp
    pcbcommon.cpp
    footprint_info.cpp
    ../pcbnew/basepcbframe.cpp
    ../pcbnew/class_board.cpp
    ../pcbnew/class_board_connected_item.cpp
    ../pcbnew/class_board_design_settings.cpp
    ../pcbnew/class_board_item.cpp
    ../pcbnew/class_dimension.cpp
    ../pcbnew/class_drawsegment.cpp
    ../pcbnew/class_drc_item.cpp
    ../pcbnew/class_edge_mod.cpp
    ../pcbnew/class_netclass.cpp
    ../pcbnew/class_netinfo_item.cpp
    ../pcbnew/class_netinfolist.cpp
    ../pcbnew/class_marker_pcb.cpp
    ../pcbnew/class_mire.cpp
    ../pcbnew/class_module.cpp
    ../pcbnew/class_pad.cpp
    ../pcbnew/class_pad_draw_functions.cpp
    ../pcbnew/class_pcb_text.cpp
    ../pcbnew/class_text_mod.cpp
    ../pcbnew/class_track.cpp
    ../pcbnew/class_zone.cpp
    ../pcbnew/class_zone_settings.cpp
    ../pcbnew/classpcb.cpp
    ../pcbnew/ratsnest_data.cpp
    ../pcbnew/ratsnest_viewitem.cpp
    ../pcbnew/collectors.cpp
    ../pcbnew/netlist_reader.cpp
    ../pcbnew/legacy_netlist_reader.cpp
    ../pcbnew/kicad_netlist_reader.cpp
    ../pcbnew/sel_layer.cpp
    ../pcbnew/pcb_plot_params.cpp
    ../pcbnew/io_mgr.cpp
    ../pcbnew/plugin.cpp
    ../pcbnew/eagle_plugin.cpp
    ../pcbnew/legacy_plugin.cpp
    ../pcbnew/kicad_plugin.cpp
    ../pcbnew/gpcb_plugin.cpp
    ../pcbnew/pcb_netlist.cpp
    ../pcbnew/specctra.cpp
    ../pcbnew/specctra_export.cpp
    ../pcbnew/specctra_keywords.cpp
    pcb_plot_params_keywords.cpp
    pcb_keywords.cpp
    ../pcbnew/pcb_parser.cpp
    fp_lib_table_keywords.cpp
    fpid.cpp
    fp_lib_table.cpp
)

set( PCB_COMMON_SRCS
    ${PCB_COMMON_SRCS}
    ../pcbnew/pcb_painter.cpp
    )

# add -DPCBNEW to compilation of these PCBNEW sources
set_source_files_properties( ${PCB_COMMON_SRCS} PROPERTIES
    COMPILE_DEFINITIONS "PCBNEW"
    )

add_library( pcbcommon STATIC ${PCB_COMMON_SRCS} )
add_dependencies( pcbcommon lib-dependencies )

# auto-generate specctra_lexer.h and specctra_keywords.cpp
make_lexer(
    ${PROJECT_SOURCE_DIR}/pcbnew/specctra.keywords
    ${PROJECT_SOURCE_DIR}/pcbnew/specctra_lexer.h
    ${PROJECT_SOURCE_DIR}/pcbnew/specctra_keywords.cpp
    DSN

    # Pass header file with dependency on *_lexer.h as extra_arg
    specctra.h
    )

# auto-generate netlist_lexer.h and netlist_keywords.cpp
make_lexer(
    ${CMAKE_CURRENT_SOURCE_DIR}/netlist.keywords
    ${PROJECT_SOURCE_DIR}/include/netlist_lexer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/netlist_keywords.cpp
    NL_T

    # Pass header file with dependency on *_lexer.h as extra_arg
    ${CMAKE_PROJECT_SOURCE_DIR}/pcbnew/netlist_reader.h
    )

# auto-generate pcb_plot_params_lexer.h and pcb_plot_params_keywords.cpp
make_lexer(
    ${CMAKE_CURRENT_SOURCE_DIR}/pcb_plot_params.keywords
    ${PROJECT_SOURCE_DIR}/include/pcb_plot_params_lexer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pcb_plot_params_keywords.cpp
    PCBPLOTPARAMS_T

    # Pass header file with dependencies on *_lexer.h as extra_arg
    ${PROJECT_SOURCE_DIR}/pcbnew/pcb_plot_params.h
    )

# auto-generate pcbnew_sexpr.h and pcbnew_sexpr.cpp
make_lexer(
    ${CMAKE_CURRENT_SOURCE_DIR}/pcb.keywords
    ${PROJECT_SOURCE_DIR}/include/pcb_lexer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pcb_keywords.cpp
    PCB_KEYS_T

    # Pass header file with dependency on *_lexer.h as extra_arg
    ${PROJECT_SOURCE_DIR}/pcbnew/pcb_parser.h
    )

# auto-generate pcbnew s-expression footprint library table code.
make_lexer(
    ${CMAKE_CURRENT_SOURCE_DIR}/fp_lib_table.keywords
    ${PROJECT_SOURCE_DIR}/include/fp_lib_table_lexer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/fp_lib_table_keywords.cpp
    FP_LIB_TABLE_T
    )

# auto-generate page layout reader s-expression page_layout_reader_lexer.h
# and title_block_reader_keywords.cpp.
make_lexer(
    ${CMAKE_CURRENT_SOURCE_DIR}/page_layout/page_layout_reader.keywords
    ${PROJECT_SOURCE_DIR}/include/page_layout_reader_lexer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/page_layout/page_layout_reader_keywords.cpp
    TB_READER_T
    )

# This one gets made only when testing.
# to build it, first enable #define STAND_ALONE at top of dsnlexer.cpp
add_executable( dsntest EXCLUDE_FROM_ALL dsnlexer.cpp )
target_link_libraries( dsntest common ${wxWidgets_LIBRARIES} rt )

add_dependencies( dsntest lib-dependencies )

