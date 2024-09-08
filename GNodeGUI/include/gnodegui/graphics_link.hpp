/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */

/**
 * @file graphics_link.hpp
 * @author Otto Link (otto.link.bv@gmail.com)
 * @brief
 * @copyright Copyright (c) 2024 Otto Link. Distributed under the terms of the
 * GNU General Public License. See the file LICENSE for the full license.
 */
#pragma once
#include <memory>

#include <QGraphicsPathItem>
#include <QObject>

namespace gngui
{

enum LinkType
{
  CUBIC,
};

class GraphicsLink : public QObject, public QGraphicsPathItem
{
  Q_OBJECT

public:
  // TODO link type

  // TODO temp link dashed
  GraphicsLink(QColor         color = QColor(0, 0, 0, 0),
               LinkType       link_type = LinkType::CUBIC,
               QGraphicsItem *parent = nullptr);

  void set_endpoints(const QPointF &start_point, const QPointF &end_point);

  void set_link_type(const LinkType &new_link_type);

protected:
  // add some margin to take into account additional items painted around the link
  QRectF boundingRect() const override;

  void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

  void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

  void paint(QPainter                       *painter,
             const QStyleOptionGraphicsItem *option,
             QWidget                        *widget) override;

  // Override shape() to enlarge the clickable area around the path
  QPainterPath shape() const override;

private:
  QColor   color;
  LinkType link_type;

  bool is_link_hovered = false;
};

} // namespace gngui