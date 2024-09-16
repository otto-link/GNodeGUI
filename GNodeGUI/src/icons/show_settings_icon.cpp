/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QGraphicsSceneMouseEvent>
#include <QPainterPath>
#include <QPen>

#include "gnodegui/icons/show_settings_icon.hpp"
#include "gnodegui/logger.hpp"

namespace gngui
{

ShowSettingsIcon::ShowSettingsIcon(float          width,
                                   QColor         color,
                                   float          pen_width,
                                   QGraphicsItem *parent)
    : AbstractIcon(width, color, pen_width, parent)
{
  this->set_path();
  this->tooltip = "Show settings";
}

void ShowSettingsIcon::set_path()
{
  QPainterPath path;

  float padding = 0.15f * this->width;
  float height = 0.25f * this->width;

  QRectF rect(padding, padding, this->width - 2.f * padding, height);
  path.addRoundedRect(rect, 0.5f * height, 0.5f * height);

  rect = QRectF(padding,
                this->width - padding - height,
                this->width - 2.f * padding,
                height);
  path.addRoundedRect(rect, 0.5f * height, 0.5f * height);

  this->setPath(path);
}

} // namespace gngui
