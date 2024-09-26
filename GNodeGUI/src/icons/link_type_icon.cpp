/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QGraphicsSceneMouseEvent>
#include <QPainterPath>
#include <QPen>

#include "gnodegui/icons/link_type_icon.hpp"
#include "gnodegui/logger.hpp"

namespace gngui
{

LinkTypeIcon::LinkTypeIcon(float          width,
                           QColor         color,
                           float          pen_width,
                           QGraphicsItem *parent)
    : AbstractIcon(width, color, pen_width, parent)
{
  this->set_path();
  this->tooltip = "Toggle link type";
}

void LinkTypeIcon::set_path()
{
  QPainterPath path;

  QRectF rect(0.f, 0.f, this->width, this->width);
  path.addRoundedRect(rect, 0.2f * this->width, 0.2f * this->width);

  float lm = 0.5f * this->width;
  float dx = 0.2f * this->width;

  path.moveTo(dx, lm);
  path.lineTo(2.f * dx, lm);
  path.lineTo(this->width - 2.f * dx, dx);
  path.lineTo(this->width - dx, dx);

  path.moveTo(dx, lm);
  path.lineTo(2.f * dx, lm);
  path.lineTo(this->width - 2.f * dx, this->width - dx);
  path.lineTo(this->width - dx, this->width - dx);

  this->setPath(path);
}

} // namespace gngui
