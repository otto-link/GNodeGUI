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

  if (this->color == QColor(0, 0, 0, 0))
    this->color = style.link.color_default;

  this->setPen(QPen(this->color, style.link.pen_width));
  this->setZValue(-1);
}

QRectF GraphicsLink::boundingRect() const
{
  QRectF bbox = this->path().boundingRect();
  bbox.adjust(-style.link.port_tip_radius,
              -style.link.port_tip_radius,
              style.link.port_tip_radius,
              style.link.port_tip_radius);
  return bbox;
}

void GraphicsLink::paint(QPainter                       *painter,
                         const QStyleOptionGraphicsItem *option,
                         QWidget                        *widget)
{
  Q_UNUSED(option);
  Q_UNUSED(widget);

  QColor pcolor = this->isSelected() ? style.link.color_selected : this->color;
  float  pwidth = this->isSelected() ? style.link.pen_width_selected
                                     : style.link.pen_width;

  // link
  painter->setBrush(Qt::NoBrush);
  painter->setPen(QPen(pcolor, pwidth));
  painter->drawPath(this->path());

  // port tips
  if (this->path().elementCount() > 0)
  {
    QPointF start_point = this->path().elementAt(0);
    QPointF end_point = this->path().elementAt(this->path().elementCount() - 1);

    painter->setBrush(pcolor);
    painter->drawEllipse(start_point,
                         style.link.port_tip_radius,
                         style.link.port_tip_radius);
    painter->drawEllipse(end_point,
                         style.link.port_tip_radius,
                         style.link.port_tip_radius);
  }
}

void GraphicsLink::set_endpoints(const QPointF &start_point, const QPointF &end_point)
{
  QPainterPath path(start_point);

  // Define control points for the cubic spline
  float dx = std::copysign(1.f, end_point.x() - start_point.x()) *
             style.link.control_point_dx;

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

} // namespace gngui
