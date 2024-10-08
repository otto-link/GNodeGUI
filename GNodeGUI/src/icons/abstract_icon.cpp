/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QGraphicsDropShadowEffect>
#include <QGraphicsSceneMouseEvent>
#include <QPainterPath>
#include <QPen>
#include <QToolTip>

#include "gnodegui/icons/abstract_icon.hpp"
#include "gnodegui/logger.hpp"

namespace gngui
{

AbstractIcon::AbstractIcon(float          width,
                           QColor         color,
                           float          pen_width,
                           QGraphicsItem *parent)
    : QGraphicsPathItem(parent), width(width), color(color), pen_width(pen_width)
{
  this->setAcceptHoverEvents(true);
  this->setOpacity(this->pen_opacity);
  this->setZValue(0);

  QPen pen(this->color);
  pen.setWidth(this->pen_width);
  pen.setCapStyle(Qt::RoundCap);
  this->setPen(pen);

  auto effect = new QGraphicsDropShadowEffect;
  effect->setOffset(4, 4);
  effect->setBlurRadius(20);
  effect->setColor(Qt::black);
  this->setGraphicsEffect(effect);
}

void AbstractIcon::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
  Q_UNUSED(event);
  this->setOpacity(0.5f * this->pen_opacity);
  QToolTip::showText(event->screenPos(), this->tooltip, nullptr);
  QGraphicsPathItem::hoverEnterEvent(event);
}

void AbstractIcon::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
  Q_UNUSED(event);
  this->setOpacity(this->pen_opacity);

  QPen pen = this->pen();
  pen.setWidth(this->pen_width);
  this->setPen(pen);

  QToolTip::hideText();
  QGraphicsPathItem::hoverLeaveEvent(event);
}

void AbstractIcon::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    QPen pen = this->pen();
    pen.setWidth(this->pen_width + 1.f);
    this->setPen(pen);
    this->setOpacity(this->pen_opacity);

    Q_EMIT this->hit_icon();
  }
  QGraphicsPathItem::mousePressEvent(event);
}

void AbstractIcon::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  Q_UNUSED(event);

  QPen pen = this->pen();
  pen.setWidth(this->pen_width);
  this->setPen(pen);

  QGraphicsPathItem::mouseReleaseEvent(event);
}

} // namespace gngui
