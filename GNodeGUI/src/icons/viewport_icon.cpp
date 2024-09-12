/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QGraphicsSceneMouseEvent>
#include <QPainterPath>
#include <QPen>

#include "gnodegui/icons/viewport_icon.hpp"
#include "gnodegui/logger.hpp"

namespace gngui
{

ViewportIcon::ViewportIcon(float          width,
                           QColor         color,
                           float          pen_width,
                           QGraphicsItem *parent)
    : AbstractIcon(width, color, pen_width, parent)
{
  this->set_path();
  this->tooltip = "Viewport";
}

void ViewportIcon::set_path()
{
  QPainterPath path;

  float lm = 0.5f * this->width;
  float dx = 0.3f * this->width;

  path.moveTo(lm, lm);
  path.lineTo(lm, lm + dx);

  float angle = 30.f / 180.f * 3.1415f;
  path.moveTo(lm, lm);
  path.lineTo(lm + dx * std::cos(angle), lm - dx * std::sin(angle));

  path.moveTo(lm, lm);
  path.lineTo(lm - dx * std::cos(angle), lm - dx * std::sin(angle));

  path.lineTo(lm, lm - dx * std::cos(angle));
  path.lineTo(lm + dx * std::cos(angle), lm - dx * std::sin(angle));
  path.lineTo(lm + dx * std::cos(angle), lm - dx * std::sin(angle) + dx);
  path.lineTo(lm, lm + dx);
  path.lineTo(lm - dx * std::cos(angle), lm - dx * std::sin(angle) + dx);
  path.lineTo(lm - dx * std::cos(angle), lm - dx * std::sin(angle));
  path.lineTo(lm, lm - dx * std::cos(angle));

  float ratio = 0.5f;
  path.moveTo(lm, 0.f);
  path.lineTo(lm, lm - ratio * dx * std::cos(angle));

  path.moveTo(lm + ratio * dx * std::cos(angle), lm + ratio * dx * std::sin(angle));
  path.lineTo(lm + (1.f + ratio) * dx * std::cos(angle),
              lm + (1.f + ratio) * dx * std::sin(angle));

  path.moveTo(lm - ratio * dx * std::cos(angle), lm + ratio * dx * std::sin(angle));
  path.lineTo(lm - (1.f + ratio) * dx * std::cos(angle),
              lm + (1.f + ratio) * dx * std::sin(angle));

  path.translate(0.f, 0.1f * this->width);

  QRectF rect(0.f, 0.f, this->width, this->width);
  path.addRoundedRect(rect, 0.05f * this->width, 0.05f * this->width);

  this->setPath(path);
}

} // namespace gngui
