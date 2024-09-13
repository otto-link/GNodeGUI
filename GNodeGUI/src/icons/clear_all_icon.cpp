/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QGraphicsSceneMouseEvent>
#include <QPainterPath>
#include <QPen>

#include "gnodegui/icons/clear_all_icon.hpp"
#include "gnodegui/logger.hpp"

namespace gngui
{

ClearAllIcon::ClearAllIcon(float          width,
                           QColor         color,
                           float          pen_width,
                           QGraphicsItem *parent)
    : AbstractIcon(width, color, pen_width, parent)
{
  this->set_path();
  this->tooltip = "Clear all";
}

void ClearAllIcon::set_path()
{
  QPainterPath path;

  // rounded rectangle
  QRectF rect(0.f, 0.f, this->width, this->width);
  path.addRoundedRect(rect, 0.2f * this->width, 0.2f * this->width);

  // circle inside the rectangle, centered
  QPointF center(rect.center());

  float dx = 0.2f * this->width;
  float lm = 0.5f * this->width;

  path.moveTo(lm - dx, lm - dx);
  path.lineTo(lm + dx, lm + dx);

  path.moveTo(lm - dx, lm + dx);
  path.lineTo(lm + dx, lm - dx);

  this->setPath(path);
}

} // namespace gngui
