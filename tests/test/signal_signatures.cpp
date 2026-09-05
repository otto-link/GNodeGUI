// Compile-time check that GraphViewer signals never carry their arguments by
// reference: a reference parameter is only valid for the duration of the emit
// and can dangle when a slot runs later (queued connection) or after an
// earlier slot has destroyed the referenced object. Passing by value keeps the
// argument alive for the whole delivery (see issue #6).

#include <type_traits>

#include "gnodegui/graph_viewer.hpp"

namespace
{

template <typename Signal> struct signal_args_by_value;

template <typename Class, typename... Args>
struct signal_args_by_value<void (Class::*)(Args...)>
    : std::bool_constant<(!std::is_reference_v<Args> && ...)>
{
};

#define GNODEGUI_CHECK_SIGNAL_BY_VALUE(signal)                                           \
  static_assert(signal_args_by_value<decltype(&gngui::GraphViewer::signal)>::value,      \
                "gngui::GraphViewer::" #signal                                           \
                " must pass its arguments by value (issue #6)")

GNODEGUI_CHECK_SIGNAL_BY_VALUE(connection_deleted);
GNODEGUI_CHECK_SIGNAL_BY_VALUE(connection_dropped);
GNODEGUI_CHECK_SIGNAL_BY_VALUE(connection_finished);
GNODEGUI_CHECK_SIGNAL_BY_VALUE(connection_started);
GNODEGUI_CHECK_SIGNAL_BY_VALUE(new_graphics_node_request);
GNODEGUI_CHECK_SIGNAL_BY_VALUE(new_node_request);
GNODEGUI_CHECK_SIGNAL_BY_VALUE(node_deleted);
GNODEGUI_CHECK_SIGNAL_BY_VALUE(node_deselected);
GNODEGUI_CHECK_SIGNAL_BY_VALUE(node_reload_request);
GNODEGUI_CHECK_SIGNAL_BY_VALUE(node_selected);
GNODEGUI_CHECK_SIGNAL_BY_VALUE(node_settings_request);
GNODEGUI_CHECK_SIGNAL_BY_VALUE(node_right_clicked);
GNODEGUI_CHECK_SIGNAL_BY_VALUE(node_type_dropped);
GNODEGUI_CHECK_SIGNAL_BY_VALUE(nodes_copy_request);
GNODEGUI_CHECK_SIGNAL_BY_VALUE(nodes_duplicate_request);

#undef GNODEGUI_CHECK_SIGNAL_BY_VALUE

} // namespace
