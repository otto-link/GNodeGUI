/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QGraphicsSceneMouseEvent>
#include <QPainterPath>
#include <QPen>

#include "gnodegui/icons/fit_content_icon.hpp"
#include "gnodegui/logger.hpp"

namespace gngui
{

FitContentIcon::FitContentIcon(float          width,
                               QColor         color,
                               float          pen_width,
                               QGraphicsItem *parent)
    : AbstractIcon(width, color, pen_width, parent)
{
  this->set_path();
  this->tooltip = "Fit content";
}

void FitContentIcon::set_path()
{
  QPainterPath path;

  // centered rounded rectangle
  float  lx = 0.4f * this->width;
  float  dx = 0.5f * (this->width - lx);
  QRectF rect(dx, dx, lx, lx);
  path.addRoundedRect(rect, 0.1f * this->width, 0.1f * this->width);

  // triangles
  float lm = 0.5f * this->width;
  float dm = 0.3f * lx;

  path.moveTo(lm - dm, dm);
  path.lineTo(lm, 0.f);
  path.lineTo(lm + dm, dm);

  path.moveTo(dm, lm - dm);
  path.lineTo(0.f, lm);
  path.lineTo(dm, lm + dm);

  path.moveTo(lm - dm, this->width - dm);
  path.lineTo(lm, this->width);
  path.lineTo(lm + dm, this->width - dm);

  path.moveTo(this->width - dm, lm - dm);
  path.lineTo(this->width, lm);
  path.lineTo(this->width - dm, lm + dm);

  this->setPath(path);
}

} // namespace gngui
