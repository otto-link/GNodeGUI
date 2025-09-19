/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#pragma once
#include <memory>

#include "nlohmann/json.hpp"

#include <QCursor>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>

#include "gnodegui/logger.hpp"

namespace gngui
{

class GraphicsGroup : public QGraphicsRectItem
{
public:
  GraphicsGroup(QGraphicsItem *parent = nullptr);

  void           json_from(nlohmann::json json);
  nlohmann::json json_to() const;

  void set_caption(const std::string &new_caption);
  void set_color(const QColor &new_color);

protected:
  enum Corner
  {
    NONE,
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
  } current_corner;

  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
  void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
  void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
  void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

  virtual void paint(QPainter                       *painter,
                     const QStyleOptionGraphicsItem *option,
                     QWidget                        *widget) override;

private:
  QGraphicsTextItem *caption_item;
  QColor             color;

  bool is_hovered = false;

  bool    resizing;
  QPointF resize_start_pos;
  qreal   resize_handle_size; // Size of the corner area for resizing

  bool                   dragging;
  QPointF                drag_start_pos;
  QList<QGraphicsItem *> selected_items;

  // Helper function to determine which corner is being hovered or clicked
  Corner get_resize_corner(const QPointF &pos) const;

  // Update the position of the caption to stay at the top middle of the rectangle
  void update_caption_position();
};

} // namespace gngui
