/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QGraphicsSceneMouseEvent>
#include <QPainterPath>
#include <QPen>

#include "gnodegui/icons/dots_icon.hpp"
#include "gnodegui/logger.hpp"

namespace gngui
{

DotsIcon::DotsIcon(float width, QColor color, float pen_width, QGraphicsItem *parent)
    : AbstractIcon(width, color, pen_width, parent)
{
  this->set_path();
  this->tooltip = "More...";
}

void DotsIcon::set_path()
{
  QPainterPath path;
  QBrush       brush = this->brush();

  brush.setColor(this->color);
  brush.setStyle(Qt::SolidPattern);
  this->setBrush(brush);

  float radius = 0.07f * this->width;
  float dx = this->width / 4.f;
  float lm = 0.5 * this->width;

  path.addEllipse(QPointF(dx, lm), radius, radius);
  path.addEllipse(QPointF(2.f * dx, lm), radius, radius);
  path.addEllipse(QPointF(3.f * dx, lm), radius, radius);

  this->setPath(path);
}

} // namespace gngui
