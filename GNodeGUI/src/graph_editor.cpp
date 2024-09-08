/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QKeyEvent>

#include "gnodegui/graph_editor.hpp"
#include "gnodegui/logger.hpp"
#include "gnodegui/style.hpp"

namespace gngui
{

GraphEditor::GraphEditor() : QGraphicsView()
{
  SPDLOG->trace("GraphEditor::GraphEditor");
  this->setRenderHint(QPainter::Antialiasing);
  this->setRenderHint(QPainter::SmoothPixmapTransform);
  this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setDragMode(QGraphicsView::NoDrag);

  this->setScene(new QGraphicsScene());

  this->setBackgroundBrush(QBrush(gngui::style.editor.color_bg));
}

void GraphEditor::add_item(QGraphicsItem *item, QPointF scene_pos)
{
  item->setPos(scene_pos);
  this->scene()->addItem(item);

  // if this item is GraphicsNode, install the required event filter
  if (GraphicsNode *node = dynamic_cast<GraphicsNode *>(item))
    for (QGraphicsItem *other_item : this->scene()->items())
      if (GraphicsNode *other_node = dynamic_cast<GraphicsNode *>(other_item))
        if (node != other_node)
        {
          node->installSceneEventFilter(other_node);
          other_node->installSceneEventFilter(node);
        }
}

void GraphEditor::add_node(NodeProxy *p_node_proxy, QPointF scene_pos)
{
  GraphicsNode *p_node = new GraphicsNode(p_node_proxy);
  this->add_item(p_node, scene_pos);

  this->connect(p_node,
                &GraphicsNode::right_clicked,
                this,
                &GraphEditor::on_node_right_clicked);

  this->connect(p_node,
                &GraphicsNode::connection_started,
                this,
                &GraphEditor::on_connection_started);

  this->connect(p_node,
                &GraphicsNode::connection_finished,
                this,
                &GraphEditor::on_connection_finished);

  this->connect(p_node,
                &GraphicsNode::connection_dropped,
                this,
                &GraphEditor::on_connection_dropped);
}

void GraphEditor::delete_selected_items()
{
  QGraphicsScene *scene = this->scene();

  if (!scene)
    return;

  // iterate over the selected items and delete them
  QList<QGraphicsItem *> selected_items = scene->selectedItems();

  for (QGraphicsItem *item : selected_items)
  {
    scene->removeItem(item);

    GraphicsNode *p_node = qgraphicsitem_cast<GraphicsNode *>(item);
    if (p_node)
    {
      Q_EMIT this->node_deleted(p_node->get_proxy_ref()->get_id());
      SPDLOG->trace("GraphicsNode removed, id: {}", p_node->get_proxy_ref()->get_id());
    }
    else
      SPDLOG->trace("item removed");

    delete item;
  }
}

void GraphEditor::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Shift)
    this->setDragMode(QGraphicsView::RubberBandDrag);

  QGraphicsView::keyPressEvent(event);
}

void GraphEditor::keyReleaseEvent(QKeyEvent *event)
{
  switch (event->key())
  {
  case Qt::Key_Shift:
    this->setDragMode(QGraphicsView::NoDrag);
    break;

    // case Qt::Key_A:
    // {
    //   QPoint  view_pos = this->mapFromGlobal(QCursor::pos());
    //   QPointF scene_pos = this->mapToScene(view_pos);
    //   this->add_node(scene_pos, "test");
    //   break;
    // }

  case Qt::Key_D:
  case Qt::Key_Delete:
    this->delete_selected_items();
    break;
  }

  QGraphicsView::keyReleaseEvent(event);
}

void GraphEditor::mouseMoveEvent(QMouseEvent *event)
{
  if (this->temp_link)
  {
    // Update the end of the temporary cubic spline to follow the mouse
    QPointF end_pos = mapToScene(event->pos());
    this->temp_link->set_endpoints(this->temp_link->path().pointAtPercent(0), end_pos);
  }

  QGraphicsView::mouseMoveEvent(event);
}

void GraphEditor::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton && (event->modifiers() & Qt::ShiftModifier))
    this->setDragMode(QGraphicsView::RubberBandDrag);

  else if (event->button() == Qt::LeftButton)
    this->setDragMode(QGraphicsView::ScrollHandDrag);

  else if (event->button() == Qt::RightButton)
  {
    // Only handle the right-click if no item handled it
    if (this->itemAt(event->pos()) == nullptr)
    {
      QPoint  view_pos = this->mapFromGlobal(QCursor::pos());
      QPointF scene_pos = this->mapToScene(view_pos);
      Q_EMIT this->background_right_clicked(scene_pos);
    }
    else
      event->ignore();
  }

  QGraphicsView::mousePressEvent(event);
}

void GraphEditor::mouseReleaseEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
    this->setDragMode(QGraphicsView::NoDrag);

  QGraphicsView::mouseReleaseEvent(event);
}

void GraphEditor::on_connection_dropped(GraphicsNode *from,
                                        int           port_index,
                                        QPointF       scene_pos)
{
  if (this->temp_link)
  {
    // Remove the temporary line
    this->scene()->removeItem(temp_link);
    delete this->temp_link;
    this->temp_link = nullptr;

    SPDLOG->trace("GraphEditor::on_connection_dropped connection_dropped {}:{}",
                  from->get_proxy_ref()->get_id(),
                  from->get_proxy_ref()->get_port_id(port_index));

    Q_EMIT this->connection_dropped(from->get_proxy_ref()->get_id(),
                                    from->get_proxy_ref()->get_port_id(port_index),
                                    scene_pos);
  }
}

void GraphEditor::on_connection_finished(GraphicsNode *from_node,
                                         int           port_from_index,
                                         GraphicsNode *to_node,
                                         int           port_to_index)
{
  if (this->temp_link)
  {
    PortType from_type = from_node->get_proxy_ref()->get_port_type(port_from_index);
    PortType to_type = to_node->get_proxy_ref()->get_port_type(port_to_index);

    if (from_node != to_node && from_type != to_type)
    {
      SPDLOG->trace("GraphEditor::on_connection_finished, {}:{} -> {}:{}",
                    from_node->get_proxy_ref()->get_caption(),
                    from_node->get_proxy_ref()->get_port_id(port_from_index),
                    to_node->get_proxy_ref()->get_caption(),
                    to_node->get_proxy_ref()->get_port_id(port_to_index));

      // Finalize the connection
      QPointF port_from_pos = from_node->scenePos() + from_node->get_geometry_ref()
                                                          ->port_rects[port_from_index]
                                                          .center();
      QPointF port_to_pos = to_node->scenePos() + to_node->get_geometry_ref()
                                                      ->port_rects[port_to_index]
                                                      .center();

      this->temp_link->set_endpoints(port_from_pos, port_to_pos);

      // Keep the link as a permanent connection
      this->temp_link = nullptr;

      Q_EMIT this->connection_finished(
          from_node->get_proxy_ref()->get_id(),
          from_node->get_proxy_ref()->get_port_id(port_from_index),
          to_node->get_proxy_ref()->get_id(),
          to_node->get_proxy_ref()->get_port_id(port_to_index));
    }
    else
    {
      // tried to connect but nothinh happens (same node from and to,
      // same port types...)
      this->scene()->removeItem(temp_link);
      delete this->temp_link;
      this->temp_link = nullptr;
    }
  }

  this->source_node = nullptr;
}

void GraphEditor::on_connection_started(GraphicsNode *from_node, int port_index)
{
  this->source_node = from_node;

  this->temp_link = new GraphicsLink(
      get_color_from_data_type(from_node->get_proxy_ref()->get_data_type(port_index)));

  QPointF port_pos = from_node->scenePos() +
                     from_node->get_geometry_ref()->port_rects[port_index].center();

  this->temp_link->set_endpoints(port_pos, port_pos);
  this->scene()->addItem(this->temp_link);

  Q_EMIT this->connection_started(from_node->get_proxy_ref()->get_id(),
                                  from_node->get_proxy_ref()->get_port_id(port_index));
}

void GraphEditor::on_node_right_clicked(const std::string &id, QPointF scene_pos)
{
  SPDLOG->trace("in GraphEditor, node right clicked, id: {}, scene position: {} {}",
                id,
                scene_pos.x(),
                scene_pos.y());
  Q_EMIT this->node_right_clicked(id, scene_pos);
}

void GraphEditor::wheelEvent(QWheelEvent *event)
{
  const float factor = 1.2f;
  QPointF     mouse_scene_pos = this->mapToScene(event->position().toPoint());

  if (event->angleDelta().y() > 0)
    this->scale(factor, factor);
  else
    this->scale(1.f / factor, 1.f / factor);

  // adjust the view to maintain the zoom centered on the mouse position
  QPointF new_mouse_scene_pos = this->mapToScene(event->position().toPoint());
  QPointF delta = new_mouse_scene_pos - mouse_scene_pos;
  this->translate(delta.x(), delta.y());

  event->accept();
}

} // namespace gngui
