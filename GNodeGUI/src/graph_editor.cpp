/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <fstream>

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

void GraphEditor::delete_graphics_link(GraphicsLink *p_link)
{
  SPDLOG->trace("GraphicsLink removing");

  if (!p_link)
  {
    SPDLOG->warn("GraphEditor::delete_graphics_link: invalid link provided.");
    return;
  }

  GraphicsNode *node_out = p_link->get_node_out();
  GraphicsNode *node_in = p_link->get_node_in();
  int           port_out = p_link->get_port_out_index();
  int           port_in = p_link->get_port_in_index();

  SPDLOG->trace("GraphEditor::delete_graphics_link, {}:{} -> {}:{}",
                node_out->get_id(),
                node_out->get_port_id(port_out),
                node_in->get_id(),
                node_in->get_port_id(port_in));

  node_out->set_is_port_connected(port_out, nullptr);
  node_in->set_is_port_connected(port_in, nullptr);

  delete p_link;

  Q_EMIT this->connection_deleted(node_out->get_id(),
                                  node_out->get_port_id(port_out),
                                  node_in->get_id(),
                                  node_in->get_port_id(port_in));
}

void GraphEditor::delete_graphics_node(GraphicsNode *p_node)
{
  SPDLOG->trace("GraphicsNode removing, id: {}", p_node->get_id());

  if (!p_node)
  {
    SPDLOG->warn("GraphEditor::delete_graphics_node: invalid node provided.");
    return;
  }

  SPDLOG->trace("GraphicsNode removing, id: {}", p_node->get_id());

  // remove any connected links
  for (QGraphicsItem *item : this->scene()->items())
    if (GraphicsLink *p_link = dynamic_cast<GraphicsLink *>(item))
      if (p_link->get_node_out()->get_id() == p_node->get_id() ||
          p_link->get_node_in()->get_id() == p_node->get_id())
        this->delete_graphics_link(p_link);

  delete p_node;
  Q_EMIT this->node_deleted(p_node->get_id());
}

void GraphEditor::delete_selected_items()
{
  QGraphicsScene *scene = this->scene();

  if (!scene)
    return;

  for (QGraphicsItem *item : scene->selectedItems())
  {
    // remove item from the scene (if it's not already removed by one
    // the methods called bellow)
    if (scene->items().contains(item))
    {
      scene->removeItem(item);

      if (GraphicsNode *p_node = dynamic_cast<GraphicsNode *>(item))
        this->delete_graphics_node(p_node);
      else if (GraphicsLink *p_link = dynamic_cast<GraphicsLink *>(item))
        this->delete_graphics_link(p_link);
      else
      {
        SPDLOG->trace("item removed");
        delete item;
      }
    }
  }
}

void GraphEditor::export_to_graphviz(const std::string &fname)
{
  // after export: to convert, command line: dot export.dot -Tsvg > output.svg

  SPDLOG->trace("exporting to graphviz format...");

  std::ofstream file(fname);

  if (!file.is_open())
    throw std::runtime_error("Failed to open file: " + fname);

  file << "digraph root {\n";
  file << "label=\"" << "GraphEditor::export_to_graphviz" << "\";\n";
  file << "labelloc=\"t\";\n";
  file << "rankdir=TD;\n";
  file << "ranksep=0.5;\n";
  file << "node [shape=record];\n";

  // Output nodes with their labels
  for (QGraphicsItem *item : this->scene()->items())
    if (GraphicsNode *p_node = dynamic_cast<GraphicsNode *>(item))
      file << p_node->get_id() << " [label=\"" << p_node->get_caption() << "("
           << p_node->get_id() << ")" << "\"];\n";

  for (QGraphicsItem *item : this->scene()->items())
    if (GraphicsLink *p_link = dynamic_cast<GraphicsLink *>(item))
      file << "\"" << p_link->get_node_out()->get_id() << "\" -> \""
           << p_link->get_node_in()->get_id() << "\" [fontsize=8, label=\""
           << p_link->get_node_out()->get_port_id(p_link->get_port_out_index()) << " - "
           << p_link->get_node_in()->get_port_id(p_link->get_port_in_index()) << "\"]"
           << std::endl;

  file << "}\n";
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

  case Qt::Key_E:
    this->export_to_graphviz();
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
                  from->get_id(),
                  from->get_port_id(port_index));

    Q_EMIT this->connection_dropped(from->get_id(),
                                    from->get_port_id(port_index),
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
    PortType from_type = from_node->get_port_type(port_from_index);
    PortType to_type = to_node->get_port_type(port_to_index);

    if (from_node != to_node && from_type != to_type &&
        from_node->is_port_available(port_from_index) &&
        to_node->is_port_available(port_to_index))
    {
      // Finalize the connection
      QPointF port_from_pos = from_node->scenePos() + from_node->get_geometry_ref()
                                                          ->port_rects[port_from_index]
                                                          .center();
      QPointF port_to_pos = to_node->scenePos() + to_node->get_geometry_ref()
                                                      ->port_rects[port_to_index]
                                                      .center();

      this->temp_link->set_endpoints(port_from_pos, port_to_pos);
      this->temp_link->set_pen_style(Qt::SolidLine);

      // from output to input
      {
        this->temp_link->set_endnodes(from_node, port_from_index, to_node, port_to_index);

        GraphicsNode *node_out = this->temp_link->get_node_out();
        GraphicsNode *node_in = this->temp_link->get_node_in();

        int port_out = this->temp_link->get_port_out_index();
        int port_in = this->temp_link->get_port_in_index();

        node_out->set_is_port_connected(port_out, this->temp_link);
        node_in->set_is_port_connected(port_in, this->temp_link);

        SPDLOG->trace("GraphEditor::on_connection_finished, {}:{} -> {}:{}",
                      node_out->get_id(),
                      node_out->get_port_id(port_out),
                      node_in->get_id(),
                      node_in->get_port_id(port_in));

        Q_EMIT this->connection_finished(node_out->get_id(),
                                         node_out->get_port_id(port_out),
                                         node_in->get_id(),
                                         node_in->get_port_id(port_in));
      }

      // Keep the link as a permanent connection
      this->temp_link = nullptr;
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
      get_color_from_data_type(from_node->get_data_type(port_index)));

  QPointF port_pos = from_node->scenePos() +
                     from_node->get_geometry_ref()->port_rects[port_index].center();

  this->temp_link->set_endpoints(port_pos, port_pos);
  this->scene()->addItem(this->temp_link);

  Q_EMIT this->connection_started(from_node->get_id(),
                                  from_node->get_port_id(port_index));
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
