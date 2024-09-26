/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QPainterPath>
#include <QPen>

#include "gnodegui/icons/save_icon.hpp"
#include "gnodegui/logger.hpp"

namespace gngui
{

SaveIcon::SaveIcon(float width, QColor color, float pen_width, QGraphicsItem *parent)
    : AbstractIcon(width, color, pen_width, parent)
{
  this->set_path();
  this->tooltip = "Save as";
}

void SaveIcon::set_path()
{
  QPainterPath path;

  float dx = 0.1f * this->width;
  float lm = 0.5f * this->width;

  path.moveTo(0.f, this->width - dx);
  path.lineTo(0.f, this->width);
  path.lineTo(this->width, this->width);
  path.lineTo(this->width, this->width - dx);

  path.moveTo(lm, lm - dx);
  path.lineTo(lm, this->width - 2.f * dx);
  path.lineTo(lm + 2.f * dx, this->width - 4.f * dx);
  path.moveTo(lm, this->width - 2.f * dx);
  path.lineTo(lm - 2.f * dx, this->width - 4.f * dx);

  path.moveTo(dx, this->width - dx);
  path.lineTo(this->width - dx, this->width - dx);

  this->setPath(path);
}

} // namespace gngui
