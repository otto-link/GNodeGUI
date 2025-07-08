/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QGraphicsSceneMouseEvent>
#include <QPainterPath>
#include <QPen>

#include "gnodegui/icons/import_icon.hpp"
#include "gnodegui/logger.hpp"

namespace gngui
{

ImportIcon::ImportIcon(float width, QColor color, float pen_width, QGraphicsItem *parent)
    : AbstractIcon(width, color, pen_width, parent)
{
  this->set_path();
  this->tooltip = "Import";
}

void ImportIcon::set_path()
{
  QPainterPath path;

  QRectF rect(0.f, 0.f, this->width, this->width);
  path.addRoundedRect(rect, 0.05f * this->width, 0.05f * this->width);

  float lm = 0.5f * this->width;
  float dx = 0.15f * this->width;

  path.moveTo(lm - dx, lm - dx);
  path.lineTo(lm + dx, lm);

  path.moveTo(lm - dx, lm + dx);
  path.lineTo(lm + dx, lm);

  this->setPath(path);
}

} // namespace gngui
