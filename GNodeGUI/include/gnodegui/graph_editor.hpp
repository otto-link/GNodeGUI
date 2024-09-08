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

#include <QGraphicsItem>
#include <QGraphicsView>

#include "gnodegui/graphics_link.hpp"
#include "gnodegui/graphics_node.hpp"
#include "gnodegui/node_proxy.hpp"

namespace gngui
{

class GraphEditor : public QGraphicsView
{
  Q_OBJECT

public:
  GraphEditor();

  void add_item(QGraphicsItem *item, QPointF scene_pos);

  void add_node(NodeProxy *p_node_proxy, QPointF scene_pos);

public Q_SLOTS:

  void on_node_right_clicked(const std::string &id, QPointF scene_pos);

Q_SIGNALS:
  void background_right_clicked(QPointF scene_pos);

  void node_deleted(const std::string &id);

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

protected:
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
  GraphicsLink *temp_link = nullptr;   // Temporary link
  GraphicsNode *source_node = nullptr; // Source node for the connection
};

} // namespace gngui