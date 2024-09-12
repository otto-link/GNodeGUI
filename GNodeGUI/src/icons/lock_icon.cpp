/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QGraphicsSceneMouseEvent>
#include <QPainterPath>
#include <QPen>

#include "gnodegui/icons/lock_icon.hpp"
#include "gnodegui/logger.hpp"

namespace gngui
{

LockIcon::LockIcon(float width, QColor color, float pen_width, QGraphicsItem *parent)
    : AbstractIcon(width, color, pen_width, parent)
{
  this->set_path();
  this->tooltip = "Lock";
}

void LockIcon::set_path()
{
  QPainterPath path;

  float padding = 0.15f * this->width;

  QRectF rect(padding,
              0.5f * this->width,
              this->width - 2.f * padding,
              0.5f * this->width);
  path.addRoundedRect(rect, 0.1f * this->width, 0.1f * this->width);

  float radius = 0.2f * this->width;
  float lm = 0.5f * this->width;
  float dy = 1.5f * padding;

  path.moveTo(lm - radius, 0.5f * this->width);
  path.lineTo(lm - radius, 0.5f * this->width - dy);

  QRectF circle_rect(lm - radius, lm - radius - dy, 2.f * radius, 2.f * radius);
  float  angle = this->is_locked ? 0.f : 20.f;
  path.arcMoveTo(circle_rect, angle);
  path.arcTo(circle_rect, angle, 180.f - angle);

  if (this->is_locked)
  {
    path.moveTo(lm + radius, 0.5f * this->width);
    path.lineTo(lm + radius, 0.5f * this->width - dy);
  }

  this->setPath(path);
}

} // namespace gngui
