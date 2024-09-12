/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QGraphicsSceneMouseEvent>
#include <QPainterPath>
#include <QPen>

#include "gnodegui/icons/group_icon.hpp"
#include "gnodegui/logger.hpp"

namespace gngui
{

GroupIcon::GroupIcon(float width, QColor color, float pen_width, QGraphicsItem *parent)
    : AbstractIcon(width, color, pen_width, parent)
{
  this->set_path();
  this->tooltip = "Group";
}

void GroupIcon::set_path()
{
  QPainterPath path;

  // rounded rectangle
  QRectF rect(0.f, 0.f, this->width, this->width);
  path.addRoundedRect(rect, 0.2f * this->width, 0.2f * this->width);

  // three rectangles
  float dx = 0.15f * this->width;
  float lx = 0.3f * this->width;

  path.addRect(QRectF(dx, dx, lx, lx));
  path.addRect(QRectF(this->width - dx - lx, dx, lx, lx));
  path.addRect(QRectF(dx, this->width - dx - lx, lx, lx));

  this->setPath(path);
}

} // namespace gngui
