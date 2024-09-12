/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QGraphicsSceneMouseEvent>
#include <QPainterPath>
#include <QPen>

#include "gnodegui/icons/reload_icon.hpp"
#include "gnodegui/logger.hpp"

namespace gngui
{

ReloadIcon::ReloadIcon(float width, QColor color, float pen_width, QGraphicsItem *parent)
    : AbstractIcon(width, color, pen_width, parent)
{
  this->set_path();
  this->tooltip = "Reload";
}

void ReloadIcon::set_path()
{
  float radius = 0.5f * this->width;

  // create a circular path
  QPainterPath path;
  QRectF       circle_rect(0.f, 0.f, 2.f * radius, 2.f * radius);
  float        angle = 20.f;
  path.arcMoveTo(circle_rect, angle);
  path.arcTo(circle_rect, angle, 360.f - 2.f * angle);

  // create arrowhead
  qreal        arrow_size = 0.4f * radius;
  QPainterPath arrow_head;
  arrow_head.arcMoveTo(circle_rect, angle);
  QPointF pos = arrow_head.currentPosition();
  pos += QPointF(0.25f * pen_width, 0.25f * pen_width);

  float alpha = (45.f - angle - 8.f) / 180.f * 3.1415f;
  float ca = arrow_size * std::cos(alpha);
  float sa = arrow_size * std::sin(alpha);

  arrow_head.moveTo(pos.x() - ca, pos.y() - sa);
  arrow_head.lineTo(pos.x(), pos.y());
  arrow_head.lineTo(pos.x() + sa, pos.y() - ca);
  path.addPath(arrow_head);

  // set the constructed path
  this->setPath(path);
}

} // namespace gngui
