/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */

/**
 * @file abstract_icon.hpp
 * @author Otto Link (otto.link.bv@gmail.com)
 * @brief
 * @copyright Copyright (c) 2024 Otto Link. Distributed under the terms of the
 * GNU General Public License. See the file LICENSE for the full license.
 */
#pragma once
#include <QGraphicsPathItem>

namespace gngui
{

class AbstractIcon : public QObject, public QGraphicsPathItem
{
  Q_OBJECT

public:
  AbstractIcon(float width, QColor color, float pen_width, QGraphicsItem *parent);

  void set_opacity(qreal new_pen_opacity)
  {
    this->pen_opacity = new_pen_opacity;
    this->setOpacity(this->pen_opacity);
  }

  void set_tooltip(QString new_tooltip) { this->tooltip = new_tooltip; }

Q_SIGNALS:
  void hit_icon();

protected:
  void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

  void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

  virtual void set_path() = 0;

  float  width;
  QColor color;
  float  pen_width;
  qreal  pen_opacity = 1.f;

  QString tooltip = "tooltip";
};
} // namespace gngui