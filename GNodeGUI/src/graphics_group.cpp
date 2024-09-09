/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QGraphicsScene>
#include <QInputDialog>
#include <QLineEdit>
#include <QPainter>

#include "gnodegui/graphics_group.hpp"
#include "gnodegui/graphics_link.hpp"
#include "gnodegui/logger.hpp"
#include "gnodegui/style.hpp"

namespace gngui
{

GraphicsGroup::GraphicsGroup(QGraphicsItem *parent)
    : QGraphicsRectItem(parent), resizing(false), resize_handle_size(20.f)
{
  this->setFlag(QGraphicsItem::ItemIsSelectable, true);
  this->setFlag(QGraphicsItem::ItemIsMovable, true);
  this->setAcceptHoverEvents(true);
  this->setRect(0.f, 0.f, 256.f, 128.f);
  this->setZValue(-1);

  // Create a caption text item at the top middle of the rectangle
  caption_item = new QGraphicsTextItem("Double-click to edit caption", this);
  caption_item->setTextInteractionFlags(Qt::NoTextInteraction);
  caption_item->setFlag(QGraphicsItem::ItemIsSelectable, false);
  caption_item->setDefaultTextColor(style.group.color_caption);

  QFont font = caption_item->font();
  font.setBold(style.group.bold_caption);
  caption_item->setFont(font);

  this->update_caption_position();
}

GraphicsGroup::Corner GraphicsGroup::get_resize_corner(const QPointF &pos) const
{
  QRectF rect = this->rect();
  QSizeF handle_size(this->resize_handle_size, this->resize_handle_size);

  QPointF top_left = rect.topLeft();
  QPointF top_right = rect.topRight() - QPointF(this->resize_handle_size, 0);
  QPointF bottom_left = rect.bottomLeft() - QPointF(0, this->resize_handle_size);
  QPointF bottom_right = rect.bottomRight() -
                         QPointF(this->resize_handle_size, this->resize_handle_size);

  QRectF top_left_rect(top_left, handle_size);
  QRectF top_right_rect(top_right, handle_size);
  QRectF bottom_left_rect(bottom_left, handle_size);
  QRectF bottom_right_rect(bottom_right, handle_size);

  if (top_left_rect.contains(pos))
    return Corner::TOP_LEFT;
  if (top_right_rect.contains(pos))
    return Corner::TOP_RIGHT;
  if (bottom_left_rect.contains(pos))
    return Corner::BOTTOM_LEFT;
  if (bottom_right_rect.contains(pos))
    return Corner::BOTTOM_RIGHT;

  return Corner::NONE;
}

void GraphicsGroup::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
  this->is_hovered = true;
  QGraphicsRectItem::hoverEnterEvent(event);
}

void GraphicsGroup::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
  this->is_hovered = false;
  QGraphicsRectItem::hoverLeaveEvent(event);
}

void GraphicsGroup::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
  // Change the cursor when hovering near the edges or corners for resizing
  Corner corner = get_resize_corner(event->pos());

  switch (corner)
  {
  case Corner::TOP_LEFT:
  case Corner::BOTTOM_RIGHT:
    setCursor(Qt::SizeFDiagCursor);
    break;
  case Corner::TOP_RIGHT:
  case Corner::BOTTOM_LEFT:
    setCursor(Qt::SizeBDiagCursor);
    break;
  default:
    setCursor(Qt::ArrowCursor);
    break;
  }

  QGraphicsRectItem::hoverMoveEvent(event);
}

void GraphicsGroup::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
  // check if double-click is on the caption to start editing
  QRectF bbox = this->caption_item->boundingRect();
  bbox.moveTo(this->caption_item->pos());

  // event position with respect current graphic item
  QPointF item_pos = event->scenePos() - this->scenePos();

  if (bbox.contains(item_pos))
  {
    bool    ok;
    QString new_caption = QInputDialog::getText(nullptr,
                                                "Edit Caption",
                                                "Enter new caption:",
                                                QLineEdit::Normal,
                                                this->caption_item->toPlainText(),
                                                &ok);
    if (ok && !new_caption.isEmpty())
    {
      this->caption_item->setPlainText(new_caption);
      this->update_caption_position();
    }
  }
  QGraphicsRectItem::mouseDoubleClickEvent(event);
}

void GraphicsGroup::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  // Check if mouse is near a corner or edge for resizing
  this->current_corner = this->get_resize_corner(event->pos());

  if (this->current_corner != NONE)
  {
    this->resizing = true;
    this->resize_start_pos = event->pos();
  }
  else
  {
    // Begin dragging all items inside the rectangle
    this->dragging = true;
    this->drag_start_pos = event->scenePos();

    QRectF bbox = this->rect();
    bbox.moveTo(this->scenePos());
    this->selected_items = this->scene()->items(bbox);
  }
  QGraphicsRectItem::mousePressEvent(event);
}

void GraphicsGroup::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  if (resizing)
  {
    // Resize the rectangle based on the corner being resized
    QRectF  new_rect = this->rect();
    QPointF delta = event->pos() - this->resize_start_pos;

    switch (this->current_corner)
    {
    case Corner::TOP_LEFT:
      new_rect.setTopLeft(new_rect.topLeft() + delta);
      break;
    case Corner::TOP_RIGHT:
      new_rect.setTopRight(new_rect.topRight() + delta);
      break;
    case Corner::BOTTOM_LEFT:
      new_rect.setBottomLeft(new_rect.bottomLeft() + delta);
      break;
    case Corner::BOTTOM_RIGHT:
      new_rect.setBottomRight(new_rect.bottomRight() + delta);
      break;
    default:
      break;
    }

    this->setRect(new_rect);
    this->resize_start_pos = event->pos();

    this->update_caption_position();
    return;
  }
  else if (dragging)
  {
    // move the rectangle and items inside it
    QPointF delta = event->scenePos() - this->drag_start_pos;

    // move all items inside the rectangle but don't move the
    // rectangle itself and don't move the links since they are
    // already moved along with the nodes
    for (QGraphicsItem *item : this->selected_items)
      if (item != this)
      {
        GraphicsLink *p_link = dynamic_cast<GraphicsLink *>(item);
        if (!p_link)
          item->moveBy(delta.x(), delta.y());
      }

    // move the rectangle itself
    this->setPos(pos() + delta);
    this->drag_start_pos = event->scenePos();

    this->update_caption_position();
  }

  QGraphicsRectItem::mouseMoveEvent(event);
}

void GraphicsGroup::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  this->resizing = false;
  this->dragging = false;
  QGraphicsRectItem::mouseReleaseEvent(event);
}

void GraphicsGroup::paint(QPainter                       *painter,
                          const QStyleOptionGraphicsItem *option,
                          QWidget                        *widget)
{
  Q_UNUSED(option);
  Q_UNUSED(widget);

  if (this->isSelected())
    painter->setPen(QPen(style.group.color_selected, 2));
  else if (this->is_hovered)
    painter->setPen(QPen(Qt::white, 2));
  else
    painter->setPen(QPen(Qt::white, 1));

  painter->setBrush(style.group.color_fill);

  painter->drawRoundedRect(this->rect(),
                           style.group.rounding_radius,
                           style.group.rounding_radius);
}

void GraphicsGroup::update_caption_position()
{
  QRectF  rect = this->rect();
  QRectF  caption_bbox = this->caption_item->boundingRect();
  QPointF top_center = rect.topLeft() +
                       QPointF(0.5f * (rect.width() - caption_bbox.width()), 0.f);
  this->caption_item->setPos(top_center);
}

} // namespace gngui
