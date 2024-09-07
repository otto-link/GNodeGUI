/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QApplication>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#include "gnodegui/graphics_node.hpp"
#include "gnodegui/logger.hpp"
#include "gnodegui/style.hpp"

namespace gngui
{

GraphicsNode::GraphicsNode(NodeProxy *p_node_proxy, QGraphicsItem *parent)
    : QGraphicsRectItem(parent), p_node_proxy(p_node_proxy)
{
  this->setFlag(QGraphicsItem::ItemIsSelectable, true);
  this->setFlag(QGraphicsItem::ItemIsMovable, true);
  this->setFlag(QGraphicsItem::ItemDoesntPropagateOpacityToChildren, true);
  this->setFlag(QGraphicsItem::ItemIsFocusable, true);
  this->setFlag(QGraphicsItem::ItemClipsChildrenToShape, false);
  this->setAcceptHoverEvents(true);
  this->setOpacity(1);
  this->setZValue(0);

  this->geometry = GraphicsNodeGeometry(this->p_node_proxy);
  this->setRect(0.f, 0.f, this->geometry.full_width, this->geometry.full_height);
  this->is_port_hovered.resize(this->p_node_proxy->get_nports());
}

int GraphicsNode::get_hovered_port_index() const
{
  auto it = std::find(this->is_port_hovered.begin(), this->is_port_hovered.end(), true);
  return (it != this->is_port_hovered.end())
             ? std::distance(this->is_port_hovered.begin(), it)
             : -1;
}

void GraphicsNode::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
  QPointF pos = event->pos();
  QPointF scene_pos = this->mapToScene(pos);
  QPointF item_pos = scene_pos - this->scenePos();

  if (this->update_is_port_hovered(item_pos))
    this->update();

  QGraphicsRectItem::hoverMoveEvent(event);
}

void GraphicsNode::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    int hovered_port_index = this->get_hovered_port_index();

    if (hovered_port_index >= 0)
    {
      SPDLOG->trace("connection_started {}:{}",
                    this->get_proxy_ref()->get_id(),
                    hovered_port_index);

      this->has_connection_started = true;
      this->setFlag(QGraphicsItem::ItemIsMovable, false);
      this->port_index_from = hovered_port_index;
      Q_EMIT connection_started(this, hovered_port_index);
      event->accept();
    }
  }

  else if (event->button() == Qt::RightButton)
    Q_EMIT this->right_clicked(this->get_proxy_ref()->get_id(), this->scenePos());

  QGraphicsRectItem::mousePressEvent(event);
}

void GraphicsNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    if (this->has_connection_started)
    {
      // get all items at the mouse release position (in stacking order)
      QList<QGraphicsItem *> items_under_mouse = scene()->items(event->scenePos());
      bool                   is_dropped = true;

      for (QGraphicsItem *item : items_under_mouse)
        if (GraphicsNode *target_node = dynamic_cast<GraphicsNode *>(item))
        {
          // check if the new link indeed land on a port
          int hovered_port_index = target_node->get_hovered_port_index();

          if (hovered_port_index >= 0)
          {
            SPDLOG->trace("connection_finished {}:{}",
                          target_node->get_proxy_ref()->get_id(),
                          hovered_port_index);

            Q_EMIT connection_finished(this,
                                       this->port_index_from,
                                       target_node,
                                       hovered_port_index);
            is_dropped = false;
            break;
          }
          else
          {
            is_dropped = true;
            break;
          }
        }

      this->reset_is_port_hovered();
      this->update();

      if (is_dropped)
      {
        SPDLOG->trace("GraphicsNode::mouseReleaseEvent connection_dropped {}",
                      this->get_proxy_ref()->get_id());
        Q_EMIT connection_dropped(this, this->port_index_from, event->scenePos());
      }

      this->has_connection_started = false;
      this->setFlag(QGraphicsItem::ItemIsMovable, true);
    }
  }
  QGraphicsRectItem::mouseReleaseEvent(event);
}

void GraphicsNode::paint(QPainter                       *painter,
                         const QStyleOptionGraphicsItem *option,
                         QWidget                        *widget)
{
  Q_UNUSED(option);
  Q_UNUSED(widget);

  // painter->setPen(Qt::red);
  // painter->drawRoundedRect(this->rect(), 8.f, 8.f);

  // --- background rectangle

  painter->setBrush(QBrush(style.node.color_bg));
  painter->setPen(Qt::NoPen);
  painter->drawRoundedRect(this->geometry.body_rect,
                           style.node.rounding_radius,
                           style.node.rounding_radius);

  // ---caption

  if (this->isSelected())
    painter->setPen(style.node.color_selected);
  else
    painter->setPen(style.node.color_caption);

  painter->drawText(this->geometry.caption_pos,
                    this->get_proxy_ref()->get_label().c_str());

  // --- rectangle border

  painter->setBrush(Qt::NoBrush);

  if (this->isSelected())
    painter->setPen(QPen(style.node.color_selected, style.node.thickness_selected));
  else
    painter->setPen(QPen(style.node.color_border, style.node.thickness_border));

  painter->drawRoundedRect(this->geometry.body_rect,
                           style.node.rounding_radius,
                           style.node.rounding_radius);

  // --- ports

  for (int k = 0; k < this->p_node_proxy->get_nports(); k++)
  {
    painter->setPen(Qt::white);

    int align_flag = this->get_proxy_ref()->get_port_type(k) == PortType::IN
                         ? Qt::AlignLeft
                         : Qt::AlignRight;

    painter->drawText(this->geometry.port_label_rects[k],
                      align_flag,
                      this->get_proxy_ref()->get_port_label(k).c_str());

    if (this->is_port_hovered[k])
      painter->setPen(QPen(style.node.color_port_hovered, style.node.thickness_selected));
    else
      painter->setPen(QPen(style.node.color_border, style.node.thickness_border));

    painter->setBrush(QColor(139, 233, 253, 255)); // TODO
    painter->drawEllipse(this->geometry.port_rects[k].center(),
                         0.5f * this->geometry.port_rects[k].width(),
                         0.5f * this->geometry.port_rects[k].height());
  }
}

void GraphicsNode::reset_is_port_hovered()
{
  this->is_port_hovered.assign(this->is_port_hovered.size(), false);
}

bool GraphicsNode::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
  // SPDLOG->trace("GraphicsNode::sceneEventFilter");

  if (GraphicsNode *node = dynamic_cast<GraphicsNode *>(watched))
  {
    // SPDLOG->trace("node id watched [{}], this [{}]",
    //               node->get_proxy_ref()->get_label(),
    //               this->get_proxy_ref()->get_label());
    // qDebug() << event->type();

    // LOOKING FOR A PORT TO CONNECT: mouse move + connection started
    // (from node) (watched is the node at beginning of the link and
    // this the node currently being hovered and possibly the end of
    // the link)

    if (event->type() == QEvent::GraphicsSceneMouseMove &&
        node->get_has_connection_started())
    {
      QGraphicsSceneMouseEvent *mouse_event = static_cast<QGraphicsSceneMouseEvent *>(
          event);

      QPointF item_pos = mouse_event->scenePos() - this->scenePos();
      if (this->update_is_port_hovered(item_pos))
        this->update();
    }

    // // DESTINATION PORT HAS BEEN CHOSEN: mouse left release +
    // // connection started (from node) + a port is hovered (to node)

    // int hovered_port_index = this->get_hovered_port_index();

    // if (event->type() == QEvent::GraphicsSceneMouseRelease &&
    //     node->get_has_connection_started() && hovered_port_index >= 0)
    // {
    //   SPDLOG->trace("connect end");

    //   QGraphicsSceneMouseEvent *mouse_event = static_cast<QGraphicsSceneMouseEvent *>(
    //       event);

    //   if (mouse_event->button() == Qt::LeftButton)
    //   {
    //     SPDLOG->trace("left release");
    //     // node->port_index_to = hovered_port_index;
    //   }
    // }
  }

  return QGraphicsRectItem::sceneEventFilter(watched, event);
}

bool GraphicsNode::update_is_port_hovered(QPointF item_pos)
{
  // set hover state
  for (size_t k = 0; k < this->geometry.port_rects.size(); k++)
    if (this->geometry.port_rects[k].contains(item_pos))
    {
      this->is_port_hovered[k] = true;
      // SPDLOG->trace("{}:{} hovered",
      //               this->get_proxy_ref()->get_label(),
      //               this->get_proxy_ref()->get_port_label(k));
      return true;
    }

  // if we end up here and one the flag is still true, it means we
  // just left a hovered port
  for (size_t k = 0; k < this->geometry.port_rects.size(); k++)
    if (this->is_port_hovered[k])
    {
      this->is_port_hovered[k] = false;
      return true;
    }

  return false;
}

} // namespace gngui
