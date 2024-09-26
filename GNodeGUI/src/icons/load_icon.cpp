/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QPainterPath>
#include <QPen>

#include "gnodegui/icons/load_icon.hpp"
#include "gnodegui/logger.hpp"

namespace gngui
{

LoadIcon::LoadIcon(float width, QColor color, float pen_width, QGraphicsItem *parent)
    : AbstractIcon(width, color, pen_width, parent)
{
  this->set_path();
  this->tooltip = "Load";
}

void LoadIcon::set_path()
{
  QPainterPath path;

  float dx = 0.1f * this->width;
  float lm = 0.5f * this->width;

  path.moveTo(0.f, this->width - dx);
  path.lineTo(0.f, this->width);
  path.lineTo(this->width, this->width);
  path.lineTo(this->width, this->width - dx);

  path.moveTo(lm, this->width - 2.f * dx);
  path.lineTo(lm, lm - dx);
  path.lineTo(lm + 2.f * dx, lm + dx);
  path.moveTo(lm, lm - dx);
  path.lineTo(lm - 2.f * dx, lm + dx);

  path.moveTo(dx, 2.f * dx);
  path.lineTo(this->width - dx, 2.f * dx);

  this->setPath(path);
}

} // namespace gngui
