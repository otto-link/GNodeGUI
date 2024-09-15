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
  GraphViewer(std::string id = "graph");

  void add_item(QGraphicsItem *item, QPointF scene_pos = QPointF(0.f, 0.f));

  // returns a unique ID for the node
  std::string add_node(NodeProxy *p_node_proxy, QPointF scene_pos);

  void clear();

  // useful for debugging graph actual state, after export: to convert, command line: dot
  // export.dot -Tsvg > output.svg
  void export_to_graphviz(const std::string &fname = "export.dot");

  std::string get_id() const { return this->id; }

  GraphicsNode *get_graphics_node_by_id(const std::string &id);

  void json_from(nlohmann::json json);

  nlohmann::json json_to() const;

  void load_json(const std::string &fname = "export.json");

  void remove_node(const std::string &node_id);

  void save_screenshot(const std::string &fname = "screenshot.png");

  void save_json(const std::string &fname = "export.json");

  void set_id(const std::string &new_id) { this->id = new_id; }

  void set_node_inventory(const std::map<std::string, std::string> &new_node_inventory)
  {
    this->node_inventory = new_node_inventory;
  }

  void zoom_to_content();

public Q_SLOTS:
  void on_node_reload_request(const std::string &id);

  void on_node_right_clicked(const std::string &id, QPointF scene_pos);

Q_SIGNALS:
  void background_right_clicked(QPointF scene_pos);

  void new_node_request(const std::string &type, QPointF scene_pos);

  void node_deleted(const std::string &id);

  void node_reload_request(const std::string &id);

  void node_right_clicked(const std::string &id, QPointF scene_pos);

  void connection_deleted(const std::string &id_out,
                          const std::string &port_id_out,
                          const std::string &to_in,
                          const std::string &port_id_in);

  void connection_dropped(const std::string &id,
                          const std::string &port_id,
                          QPointF            scene_pos);

  void connection_finished(const std::string &id_out,
                           const std::string &port_id_out,
                           const std::string &to_in,
                           const std::string &port_id_in);

  void connection_started(const std::string &id_from, const std::string &port_id_from);

protected: // Qt events
  void contextMenuEvent(QContextMenuEvent *event) override;

  void delete_selected_items();

  void keyPressEvent(QKeyEvent *event) override;

  void keyReleaseEvent(QKeyEvent *event) override;

  void mouseMoveEvent(QMouseEvent *event) override;

  void mousePressEvent(QMouseEvent *event) override;

  void mouseReleaseEvent(QMouseEvent *event) override;

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

  // all nodes available store as a map of (node type, node category)
  std::map<std::string, std::string> node_inventory;

  GraphicsLink *temp_link = nullptr;   // Temporary link
  GraphicsNode *source_node = nullptr; // Source node for the connection

  void delete_graphics_link(GraphicsLink *p_link);

  void delete_graphics_node(GraphicsNode *p_node);
};

} // namespace gngui
