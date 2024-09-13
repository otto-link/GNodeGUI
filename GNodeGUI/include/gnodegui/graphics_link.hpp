/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */

/**
 * @file graphics_link.hpp
 * @author Otto Link (otto.link.bv@gmail.com)
 * @brief
 * @copyright Copyright (c) 2024 Otto Link. Distributed under the terms of the
 * GNU General Public License. See the file LICENSE for the full license.
 */
#pragma once
#include <memory>

#include <QGraphicsPathItem>
#include <QObject>

#include "nlohmann/json.hpp"

#include "gnodegui/graphics_node.hpp"

namespace gngui
{

class GraphicsNode;

enum LinkType
{
  CUBIC,
};

class GraphicsLink : public QObject, public QGraphicsPathItem
{
  Q_OBJECT

public:
  // TODO fix zbuffer and selection
  GraphicsLink(QColor         color = QColor(0, 0, 0, 0),
               LinkType       link_type = LinkType::CUBIC,
               QGraphicsItem *parent = nullptr);

  GraphicsNode *get_node_out() { return this->node_out; }

  int get_port_out_index() const { return this->port_out_index; }

  GraphicsNode *get_node_in() { return this->node_in; }

  int get_port_in_index() const { return this->port_in_index; }

  nlohmann::json json_to() const;

  void set_endnodes(GraphicsNode *from,
                    int           port_from_index,
                    GraphicsNode *to,
                    int           port_to_index);

  void set_endpoints(const QPointF &start_point, const QPointF &end_point);

  void set_link_type(const LinkType &new_link_type);

  void set_pen_style(const Qt::PenStyle &new_pen_style)
  {
    this->pen_style = new_pen_style;
  }

protected:
  // add some margin to take into account additional items painted around the link
  QRectF boundingRect() const override;

  void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

  void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

  void paint(QPainter                       *painter,
             const QStyleOptionGraphicsItem *option,
             QWidget                        *widget) override;

  // Override shape() to enlarge the clickable area around the path
  QPainterPath shape() const override;

private:
  QColor       color;
  LinkType     link_type;
  Qt::PenStyle pen_style = Qt::DashLine;

  bool is_link_hovered = false;

  // link infos
  GraphicsNode *node_out = nullptr;
  int           port_out_index;
  GraphicsNode *node_in = nullptr;
  int           port_in_index;
};

} // namespace gngui