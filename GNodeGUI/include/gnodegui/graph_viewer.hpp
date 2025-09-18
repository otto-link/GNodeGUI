/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */

/**
 * @file graph_editor.hpp
 * @author Otto Link (otto.link.bv@gmail.com)
 * @brief
 * @copyright Copyright (c) 2024 Otto Link. Distributed under the terms of the
 * GNU General Public License. See the file LICENSE for the full license.
 */
#pragma once
#include <functional>

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QJsonObject>

#include "nlohmann/json.hpp"

#include "gnodegui/graphics_link.hpp"
#include "gnodegui/graphics_node.hpp"
#include "gnodegui/node_proxy.hpp"

namespace gngui
{

class GraphViewer : public QGraphicsView
{
  Q_OBJECT

public:
  explicit GraphViewer(std::string id = "graph", QWidget *parent = nullptr);

  void add_item(QGraphicsItem *item, QPointF scene_pos = QPointF(0.f, 0.f));

  void add_link(const std::string &id_out,
                const std::string &port_id_out,
                const std::string &to_in,
                const std::string &port_id_in);

  // returns a unique ID for the node
  std::string add_node(NodeProxy         *p_node_proxy,
                       QPointF            scene_pos,
                       const std::string &node_id = "");

  void add_static_item(QGraphicsItem *item, QPoint window_pos, float z_value = 0.f);

  void add_toolbar(QPoint window_pos);

  void clear();

  void deselect_all();

  bool execute_new_node_context_menu();

  // useful for debugging graph actual state, after export: to convert, command line: dot
  // export.dot -Tsvg > output.svg
  void export_to_graphviz(const std::string &fname = "export.dot");

  std::string get_id() const { return this->id; }

  GraphicsNode *get_graphics_node_by_id(const std::string &node_id);

  std::vector<std::string> get_selected_node_ids();

  // prefix_id can be usefull when importing a graph into an existing
  // one, to avoid duplicate node ids
  void json_from(nlohmann::json json, bool clear_existing_content = true);

  nlohmann::json json_to() const;

  void remove_node(const std::string &node_id);

  void save_screenshot(const std::string &fname = "screenshot.png");

  void select_all();

  void set_enabled(bool state);

  void set_id(const std::string &new_id) { this->id = new_id; }

  void set_node_as_selected(const std::string &node_id);

  void set_node_inventory(const std::map<std::string, std::string> &new_node_inventory)
  {
    this->node_inventory = new_node_inventory;
  }

  void toggle_link_type();

  void zoom_to_content();

public Q_SLOTS:
  void on_compute_finished(const std::string &node_id);

  void on_compute_started(const std::string &node_id);

  void on_node_reload_request(const std::string &node_id);

  void on_node_settings_request(const std::string &node_id);

  void on_node_right_clicked(const std::string &node_id, QPointF scene_pos);

  void on_update_finished();

  void on_update_started();

Q_SIGNALS:
  void background_right_clicked(QPointF scene_pos);

  void connection_deleted(const std::string &id_out,
                          const std::string &port_id_out,
                          const std::string &to_in,
                          const std::string &port_id_in);

  void connection_dropped(const std::string &node_id,
                          const std::string &port_id,
                          QPointF            scene_pos);

  void connection_finished(const std::string &id_out,
                           const std::string &port_id_out,
                           const std::string &to_in,
                           const std::string &port_id_in);

  void connection_started(const std::string &id_from, const std::string &port_id_from);

  void graph_clear_request();

  void graph_import_request();

  void graph_load_request();

  void graph_new_request();

  void graph_reload_request();

  void graph_save_as_request();

  void graph_save_request();

  void graph_settings_request();

  void new_graphics_node_request(const std::string &node_id, QPointF scene_pos);

  void new_node_request(const std::string &type, QPointF scene_pos);

  void node_deleted(const std::string &node_id);

  void node_deselected(const std::string &node_id);

  void node_reload_request(const std::string &node_id);

  void node_selected(const std::string &node_id);

  void node_settings_request(const std::string &node_id);

  void node_right_clicked(const std::string &node_id, QPointF scene_pos);

  void nodes_copy_request(const std::vector<std::string> &id_list,
                          const std::vector<QPointF>     &scene_pos_list);

  void nodes_duplicate_request(const std::vector<std::string> &id_list,
                               const std::vector<QPointF>     &scene_pos_list);

  void nodes_paste_request();

  void quit_request();

  void selection_has_changed();

  void viewport_request();

protected: // Qt events
  void contextMenuEvent(QContextMenuEvent *event) override;

  void delete_selected_items();

  void drawForeground(QPainter *painter, const QRectF &rect) override;

  void keyPressEvent(QKeyEvent *event) override;

  void keyReleaseEvent(QKeyEvent *event) override;

  void mouseMoveEvent(QMouseEvent *event) override;

  void mousePressEvent(QMouseEvent *event) override;

  void mouseReleaseEvent(QMouseEvent *event) override;

  void resizeEvent(QResizeEvent *event) override;

  void wheelEvent(QWheelEvent *event) override;

private Q_SLOTS:
  void on_connection_dropped(GraphicsNode *from, int port_index, QPointF scene_pos);

  // reordered: 'from' is 'output' and 'to' is 'input'
  void on_connection_finished(GraphicsNode *from_node,
                              int           port_from_index,
                              GraphicsNode *to_node,
                              int           port_to_index);

  void on_connection_started(GraphicsNode *from_node, int port_index);

private:
  std::string id;

  std::vector<QGraphicsItem *> static_items;
  std::vector<QPoint>          static_items_positions;

  // all nodes available store as a map of (node type, node category)
  std::map<std::string, std::string> node_inventory;

  GraphicsLink *temp_link = nullptr;   // Temporary link
  GraphicsNode *source_node = nullptr; // Source node for the connection

  LinkType current_link_type = LinkType::CUBIC;

  void delete_graphics_link(GraphicsLink *p_link);

  void delete_graphics_node(GraphicsNode *p_node);

  bool is_item_static(QGraphicsItem *item);
};

} // namespace gngui
