/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QPainter>
#include <QPainterPath>
#include <QPen>

#include "gnodegui/graphics_link.hpp"
#include "gnodegui/logger.hpp"
#include "gnodegui/style.hpp"

namespace gngui
{

GraphicsLink::GraphicsLink(QColor color, LinkType link_type, QGraphicsItem *parent)
    : QGraphicsPathItem(parent), color(color), link_type(link_type)
{
  this->setFlag(QGraphicsItem::ItemIsSelectable, true);
  this->setFlag(QGraphicsItem::ItemIsMovable, false);
  this->setAcceptHoverEvents(true);

  if (this->color == QColor(0, 0, 0, 0))
    this->color = GN_STYLE->link.color_default;

  this->setPen(QPen(this->color, GN_STYLE->link.pen_width));
  this->setZValue(-1);
}

QRectF GraphicsLink::boundingRect() const
{
  QRectF bbox = this->path().boundingRect();
  bbox.adjust(-GN_STYLE->link.port_tip_radius,
              -GN_STYLE->link.port_tip_radius,
              GN_STYLE->link.port_tip_radius,
              GN_STYLE->link.port_tip_radius);
  return bbox;
}

void GraphicsLink::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
  this->is_link_hovered = true;
  this->update();

  QGraphicsPathItem::hoverEnterEvent(event);
}

void GraphicsLink::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
  this->is_link_hovered = false;
  this->update();

  QGraphicsPathItem::hoverLeaveEvent(event);
}

nlohmann::json GraphicsLink::json_to() const
{
  nlohmann::json json;

  json["node_out_id"] = this->node_out->get_id();
  json["node_in_id"] = this->node_in->get_id();

  json["port_out_id"] = this->node_out->get_port_id(this->port_out_index);
  json["port_in_id"] = this->node_in->get_port_id(this->port_in_index);

  return json;
}

void GraphicsLink::paint(QPainter                       *painter,
                         const QStyleOptionGraphicsItem *option,
                         QWidget                        *widget)
{
  Q_UNUSED(option);
  Q_UNUSED(widget);

  QColor pcolor = this->isSelected() ? GN_STYLE->link.color_selected : this->color;
  float  pwidth = this->is_link_hovered
                      ? GN_STYLE->link.pen_width_hovered
                      : (this->isSelected() ? GN_STYLE->link.pen_width_selected
                                            : GN_STYLE->link.pen_width);

  // link
  QPen pen(pcolor);
  pen.setWidth(pwidth);
  pen.setStyle(this->pen_style);
  painter->setPen(pen);
  painter->setBrush(Qt::NoBrush);

  // update path
  if (this->node_out && this->node_in)
  {
    QPointF start_point = this->node_out->scenePos() + this->node_out->get_geometry_ref()
                                                           ->port_rects[port_out_index]
                                                           .center();
    QPointF end_point = this->node_in->scenePos() + this->node_in->get_geometry_ref()
                                                        ->port_rects[port_in_index]
                                                        .center();

    this->set_endpoints(start_point, end_point);
  }

  // draw path
  painter->drawPath(this->path());

  // port tips
  if (this->path().elementCount() > 0)
  {
    QPointF start_point = this->path().elementAt(0);
    QPointF end_point = this->path().elementAt(this->path().elementCount() - 1);

    painter->setBrush(pcolor);
    painter->drawEllipse(start_point,
                         GN_STYLE->link.port_tip_radius,
                         GN_STYLE->link.port_tip_radius);
    painter->drawEllipse(end_point,
                         GN_STYLE->link.port_tip_radius,
                         GN_STYLE->link.port_tip_radius);
  }
}

void GraphicsLink::set_endnodes(GraphicsNode *from,
                                int           port_from_index,
                                GraphicsNode *to,
                                int           port_to_index)
{
  if (!from || !to)
  {
    GLOG->warn("GraphicsLink::set_endnodes: invalid nodes provided.");
    return;
  }

  // put the ports in the right order (from output to input)
  if (from->get_proxy_ref()->get_port_type(port_from_index) == PortType::OUT)
  {
    // 'from' is the output node, 'to' is the input node
    this->node_out = from;
    this->port_out_index = port_from_index;
    this->node_in = to;
    this->port_in_index = port_to_index;
  }
  else
  {
    // 'from' is the input node, 'to' is the output node
    this->node_in = from;
    this->port_in_index = port_from_index;
    this->node_out = to;
    this->port_out_index = port_to_index;
  }
}

void GraphicsLink::set_endpoints(const QPointF &start_point, const QPointF &end_point)
{
  QPainterPath path(start_point);

  float dx = std::abs(end_point.x() - start_point.x()) * GN_STYLE->link.curvature;

  QPointF control_point1(start_point.x() + dx, start_point.y());
  QPointF control_point2(end_point.x() - dx, end_point.y());
  path.cubicTo(control_point1, control_point2, end_point);

  this->setPath(path);
}

void GraphicsLink::set_link_type(const LinkType &new_link_type)
{
  this->link_type = new_link_type;
  this->update();
}

QPainterPath GraphicsLink::shape() const
{
  QPainterPathStroker stroker;
  stroker.setWidth(40);
  QPainterPath widened_path = stroker.createStroke(path());
  return widened_path.united(path());
}

} // namespace gngui
