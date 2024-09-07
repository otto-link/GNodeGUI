/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */

/**
 * @file graphics_node.hpp
 * @author Otto Link (otto.link.bv@gmail.com)
 * @brief
 * @copyright Copyright (c) 2024 Otto Link. Distributed under the terms of the
 * GNU General Public License. See the file LICENSE for the full license.
 */
#pragma once
#include <memory>

#include <QEvent>
#include <QGraphicsRectItem>
#include <QMouseEvent>
#include <QObject>

#include "gnodegui/graphics_node_geometry.hpp"
#include "gnodegui/logger.hpp"
#include "gnodegui/node_proxy.hpp"

namespace gngui
{

class GraphicsNode : public QObject, public QGraphicsRectItem
{
  Q_OBJECT

public:
  GraphicsNode(NodeProxy *p_node_proxy, QGraphicsItem *parent = nullptr);

  GraphicsNodeGeometry *get_geometry_ref() { return &(this->geometry); };

  // bool get_has_connection_started() const { return this->has_connection_started; }

  NodeProxy *get_proxy_ref() { return this->p_node_proxy; }

Q_SIGNALS:
  void connection_dropped(GraphicsNode *from, int port_index, QPointF scene_pos);

  void connection_finished(GraphicsNode *from,
                           int           port_from_index,
                           GraphicsNode *to,
                           int           port_to_index);

  void connection_started(GraphicsNode *from, int port_index);

  void right_clicked(const std::string &id, QPointF scene_pos);

protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

  void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

  virtual void paint(QPainter                       *painter,
                     const QStyleOptionGraphicsItem *option,
                     QWidget                        *widget) override;

  bool sceneEventFilter(QGraphicsItem *watched, QEvent *event) override;

private:
  NodeProxy           *p_node_proxy;
  GraphicsNodeGeometry geometry;
  std::vector<bool>    is_port_hovered;

  bool        has_connection_started = false;
  int         port_index_from;
  std::string data_type_connecting = "";

  int get_hovered_port_index() const;

  // true any port has changed its hover state
  bool update_is_port_hovered(QPointF scene_pos);

  // all to false
  void reset_is_port_hovered();
};

} // namespace gngui