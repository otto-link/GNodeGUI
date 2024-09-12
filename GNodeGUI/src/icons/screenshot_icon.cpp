/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QGraphicsSceneMouseEvent>
#include <QPainterPath>
#include <QPen>

#include "gnodegui/icons/screenshot_icon.hpp"
#include "gnodegui/logger.hpp"

namespace gngui
{

ScreenshotIcon::ScreenshotIcon(float          width,
                               QColor         color,
                               float          pen_width,
                               QGraphicsItem *parent)
    : AbstractIcon(width, color, pen_width, parent)
{
  this->set_path();
  this->tooltip = "Screenshot";
}

void ScreenshotIcon::set_path()
{
  QPainterPath path;

  // rounded rectangle
  QRectF rect(0.f, 0.f, this->width, this->width);
  path.addRoundedRect(rect, 0.2f * this->width, 0.2f * this->width);

  // circle inside the rectangle, centered
  QPointF center(rect.center());
  path.addEllipse(center, 0.2f * this->width, 0.2f * this->width);

  this->setPath(path);
}

} // namespace gngui
