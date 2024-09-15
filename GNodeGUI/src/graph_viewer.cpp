/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <fstream>
#include <iostream>

#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QWidgetAction>

#include "gnodegui/graph_viewer.hpp"
#include "gnodegui/graphics_group.hpp"
#include "gnodegui/logger.hpp"
#include "gnodegui/style.hpp"
#include "gnodegui/utils.hpp"

namespace gngui
{

GraphViewer::GraphViewer(std::string id) : QGraphicsView(), id(id)
{
  GUILOG->trace("GraphViewer::GraphViewer");
  this->setRenderHint(QPainter::Antialiasing);
  this->setRenderHint(QPainter::SmoothPixmapTransform);
  this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setDragMode(QGraphicsView::NoDrag);

  this->setScene(new QGraphicsScene());

  this->setBackgroundBrush(QBrush(GN_STYLE->viewer.color_bg));
}

void GraphViewer::add_item(QGraphicsItem *item, QPointF scene_pos)
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

std::string GraphViewer::add_node(NodeProxy *p_node_proxy, QPointF scene_pos)
{
  GraphicsNode *p_node = new GraphicsNode(p_node_proxy);
  this->add_item(p_node, scene_pos);

  this->connect(p_node,
                &GraphicsNode::right_clicked,
                this,
                &GraphViewer::on_node_right_clicked);

  this->connect(p_node,
                &GraphicsNode::connection_started,
                this,
                &GraphViewer::on_connection_started);

  this->connect(p_node,
                &GraphicsNode::connection_finished,
                this,
                &GraphViewer::on_connection_finished);

  this->connect(p_node,
                &GraphicsNode::connection_dropped,
                this,
                &GraphViewer::on_connection_dropped);

  this->connect(p_node,
                &GraphicsNode::reload_request,
                this,
                &GraphViewer::on_node_reload_request);

  // generate a unique id based on the object address
  std::ostringstream oss;
  oss << std::to_string((unsigned long long)(void **)p_node);
  return oss.str();
}

void GraphViewer::clear()
{
  this->scene()->clear();
  this->viewport()->update();
}

void GraphViewer::contextMenuEvent(QContextMenuEvent *event)
{
  // --- skip this if there is an item is under the cursor

  QGraphicsItem *item = this->itemAt(event->pos());

  if (item)
  {
    QGraphicsView::contextMenuEvent(event);
    return;
  }

  // --- if not keep going

  QMenu *menu = new QMenu(this);

  // add filterbox to the context menu
  QLineEdit *text_box = new QLineEdit(menu);
  text_box->setPlaceholderText(QStringLiteral("Filter"));
  text_box->setClearButtonEnabled(true);
  // hesiod::resize_font(text_box, -2);

  QWidgetAction *text_box_action = new QWidgetAction(menu);
  text_box_action->setDefaultWidget(text_box);

  menu->addAction(text_box_action);

  // to keep track of created submenus
  std::map<std::string, QMenu *> category_map;

  for (auto &[key, cat] : this->node_inventory)
  {
    const std::string              &action_name = key;
    const std::vector<std::string> &action_categories = split_string(cat, '/');

    // // backup actions
    // std::vector<QAction *> actions = {};

    QMenu *parent_menu = menu;

    // traverse the category hierarchy
    for (const std::string &category : action_categories)
    {
      // create submenu if it does not exist or add
      if (!category_map.contains(category))
        category_map[category] = parent_menu->addMenu(category.c_str());

      // and set the submenu as the "current" menu
      parent_menu = category_map.at(category);
    }

    // eventually add the action at the deepest category level
    parent_menu->addAction(action_name.c_str());
  }

  // setup filtering
  bool submenu_active = true;
  bool filtering_active = false;

  connect(
      text_box,
      &QLineEdit::textEdited,
      [this, menu, category_map, &submenu_active, &filtering_active](const QString &text)
      {
        // TODO not sure about this one, feels overly brute forcing

        // rebuild the menu from scratch
        if (submenu_active)
        {
          for (auto &[_, submenu] : category_map)
            menu->removeAction(submenu->menuAction());

          submenu_active = false;
        }

        // add everything
        if (!filtering_active)
        {
          for (const auto &[key, _] : this->node_inventory)
            menu->addAction(QString::fromStdString(key));

          filtering_active = true;
        }

        // determine who's visible
        std::map<std::string, bool> is_visible = {};

        for (const auto &[key, _] : this->node_inventory)
        {
          QString    key_qstr = QString::fromStdString(key);
          const bool match = key_qstr.contains(text, Qt::CaseInsensitive);

          if (text.isEmpty())
            is_visible[key] = true;
          else
            is_visible[key] = match;
        }

        // apply visibility
        for (auto action : menu->actions())
        {
          std::string key = action->text().toStdString();
          if (key != "") // skip text box...
            action->setVisible(is_visible.at(key));
        }
      });

  // make sure the text box gets focus so the user doesn't have to click on it
  text_box->setFocus();

  QAction *selected_action = menu->exec(event->globalPos());

  if (selected_action)
  {
    QPoint  view_pos = this->mapFromGlobal(event->globalPos());
    QPointF scene_pos = this->mapToScene(view_pos);

    Q_EMIT this->new_node_request(selected_action->text().toStdString(), scene_pos);
    qDebug() << selected_action->text();
  }

  QGraphicsView::contextMenuEvent(event);
}

void GraphViewer::delete_graphics_link(GraphicsLink *p_link)
{
  GUILOG->trace("GraphicsLink removing");

  if (!p_link)
  {
    GUILOG->error("GraphViewer::delete_graphics_link: invalid link provided.");
    return;
  }

  GraphicsNode *node_out = p_link->get_node_out();
  GraphicsNode *node_in = p_link->get_node_in();
  int           port_out = p_link->get_port_out_index();
  int           port_in = p_link->get_port_in_index();

  GUILOG->trace("GraphViewer::delete_graphics_link, {}:{} -> {}:{}",
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

void GraphViewer::delete_graphics_node(GraphicsNode *p_node)
{
  GUILOG->trace("GraphicsNode removing, id: {}", p_node->get_id());

  if (!p_node)
  {
    GUILOG->error("GraphViewer::delete_graphics_node: invalid node provided.");
    return;
  }

  GUILOG->trace("GraphicsNode removing, id: {}", p_node->get_id());

  // remove any connected links
  for (QGraphicsItem *item : this->scene()->items())
    if (GraphicsLink *p_link = dynamic_cast<GraphicsLink *>(item))
      if (p_link->get_node_out()->get_id() == p_node->get_id() ||
          p_link->get_node_in()->get_id() == p_node->get_id())
        this->delete_graphics_link(p_link);

  delete p_node;
  Q_EMIT this->node_deleted(p_node->get_id());
}

void GraphViewer::delete_selected_items()
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
        GUILOG->trace("item removed");
        delete item;
      }
    }
  }
}

void GraphViewer::export_to_graphviz(const std::string &fname)
{
  // after export: to convert, command line: dot export.dot -Tsvg > output.svg

  GUILOG->trace("exporting to graphviz format...");

  std::ofstream file(fname);

  if (!file.is_open())
    throw std::runtime_error("Failed to open file: " + fname);

  file << "digraph root {\n";
  file << "label=\"" << "GraphViewer::export_to_graphviz" << "\";\n";
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

GraphicsNode *GraphViewer::get_graphics_node_by_id(const std::string &id)
{
  for (QGraphicsItem *item : this->scene()->items())
    if (GraphicsNode *p_node = dynamic_cast<GraphicsNode *>(item))
      if (p_node->get_id() == id)
        return p_node;

  return nullptr;
}

void GraphViewer::json_from(nlohmann::json json)
{
  // check that the graph ID is indeed available
  if (!json["GraphViewer"].contains(this->id))
  {
    GUILOG->error("GraphViewer::json_from, could not file graph ID {} in the json data",
                  this->id);
    return;
  }

  // generate graph from json data
  this->clear();

  if (!json["GraphViewer"][this->id]["groups"].is_null())
  {
    for (auto &json_group : json["GraphViewer"][this->id]["groups"])
    {
      GraphicsGroup *p_group = new GraphicsGroup();
      this->add_item(p_group);
      p_group->json_from(json_group);
    }
  }

  if (!json["GraphViewer"][this->id]["nodes"].is_null())
  {
    for (auto &json_node : json["GraphViewer"][this->id]["nodes"])
    {
      std::string        caption = json_node["caption"];
      std::string        nid = json_node["id"];
      std::vector<float> pos = json_node["position"];

      // nodes are not generated in this class, it is outsourced to the
      // outter headless nodes manager
      Q_EMIT this->new_node_request(caption, nid, QPointF(pos[0], pos[1]));
    }
  }

  if (!json["GraphViewer"][this->id]["links"].is_null())
  {
    for (auto &json_link : json["GraphViewer"][this->id]["links"])
    {
      std::string node_out_id = json_link["node_out_id"];
      std::string node_in_id = json_link["node_in_id"];
      std::string port_out_id = json_link["port_out_id"];
      std::string port_in_id = json_link["port_in_id"];

      // same here, the graphic links are generated but the data
      // connection itself is outsourced to the outter headless nodes
      // manager
      this->temp_link = new GraphicsLink();

      GraphicsNode *from_node = this->get_graphics_node_by_id(node_out_id);
      GraphicsNode *to_node = this->get_graphics_node_by_id(node_in_id);

      if (from_node && to_node)
      {
        int port_from_index = from_node->get_port_index(port_out_id);
        int port_to_index = from_node->get_port_index(port_in_id);

        this->on_connection_finished(from_node, port_from_index, to_node, port_to_index);
      }
      else
        GUILOG->error(
            "GraphViewer::json_from, nodes instance cannot be found, IDs: {} and/or {}",
            node_out_id,
            node_in_id);
    }
  }
}

nlohmann::json GraphViewer::json_to() const
{
  nlohmann::json json;

  std::vector<nlohmann::json> json_node_list = {};
  std::vector<nlohmann::json> json_link_list = {};
  std::vector<nlohmann::json> json_group_list = {};

  for (QGraphicsItem *item : this->scene()->items())
  {
    if (GraphicsNode *p_node = dynamic_cast<GraphicsNode *>(item))
      json_node_list.push_back(p_node->json_to());
    else if (GraphicsLink *p_link = dynamic_cast<GraphicsLink *>(item))
      json_link_list.push_back(p_link->json_to());
    else if (GraphicsGroup *p_group = dynamic_cast<GraphicsGroup *>(item))
      json_group_list.push_back(p_group->json_to());
  }

  json["nodes"] = json_node_list;
  json["links"] = json_link_list;
  json["groups"] = json_group_list;

  return json;
}

void GraphViewer::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Shift)
    this->setDragMode(QGraphicsView::RubberBandDrag);

  QGraphicsView::keyPressEvent(event);
}

void GraphViewer::keyReleaseEvent(QKeyEvent *event)
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

  case Qt::Key_G:
  {
    QPoint  view_pos = this->mapFromGlobal(QCursor::pos());
    QPointF scene_pos = this->mapToScene(view_pos);
    this->add_item(new GraphicsGroup(), scene_pos);
    break;
  }

  case Qt::Key_Delete:
    this->delete_selected_items();
    break;

  case Qt::Key_E:
    this->export_to_graphviz();
    break;

  case Qt::Key_J:
    this->save_json();
    break;

  case Qt::Key_L:
    this->load_json();
    break;

  case Qt::Key_S:
    this->save_screenshot();
    break;

  case Qt::Key_Z:
    this->zoom_to_content();
    break;
  }

  QGraphicsView::keyReleaseEvent(event);
}

void GraphViewer::load_json(const std::string &fname)
{
  GUILOG->trace("GraphViewer::load_json");

  std::ifstream  file(fname);
  nlohmann::json json;

  if (file.is_open())
  {
    file >> json;
    file.close();
  }
  else
    GUILOG->error("GraphViewer::load_json, problem while saving file: {}", fname);

  // regenerate graph
  this->json_from(json);
}

void GraphViewer::mouseMoveEvent(QMouseEvent *event)
{
  if (this->temp_link)
  {
    // Update the end of the temporary cubic spline to follow the mouse
    QPointF end_pos = mapToScene(event->pos());
    this->temp_link->set_endpoints(this->temp_link->path().pointAtPercent(0), end_pos);
  }

  QGraphicsView::mouseMoveEvent(event);
}

void GraphViewer::mousePressEvent(QMouseEvent *event)
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

void GraphViewer::mouseReleaseEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
    this->setDragMode(QGraphicsView::NoDrag);

  QGraphicsView::mouseReleaseEvent(event);
}

void GraphViewer::on_connection_dropped(GraphicsNode *from,
                                        int           port_index,
                                        QPointF       scene_pos)
{
  if (this->temp_link)
  {
    // Remove the temporary line
    this->scene()->removeItem(temp_link);
    delete this->temp_link;
    this->temp_link = nullptr;

    GUILOG->trace("GraphViewer::on_connection_dropped connection_dropped {}:{}",
                  from->get_id(),
                  from->get_port_id(port_index));

    Q_EMIT this->connection_dropped(from->get_id(),
                                    from->get_port_id(port_index),
                                    scene_pos);
  }
}

void GraphViewer::on_connection_finished(GraphicsNode *from_node,
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

        GUILOG->trace("GraphViewer::on_connection_finished, {}:{} -> {}:{}",
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

void GraphViewer::on_connection_started(GraphicsNode *from_node, int port_index)
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

void GraphViewer::on_node_reload_request(const std::string &id)
{
  GUILOG->trace("GraphViewer::on_node_reload_request {}", id);
  Q_EMIT this->node_reload_request(id);
}

void GraphViewer::on_node_right_clicked(const std::string &id, QPointF scene_pos)
{
  Q_EMIT this->node_right_clicked(id, scene_pos);
}

void GraphViewer::save_json(const std::string &fname)
{
  GUILOG->trace("GraphViewer::save_json");

  // current data
  nlohmann::json json;
  json["GraphViewer"][this->get_id()] = this->json_to();

  // save file
  std::ofstream file(fname);

  if (file.is_open())
  {
    file << json.dump(4);
    file.close();
  }
  else
    GUILOG->error("GraphViewer::save_json, problem while saving file: {}", fname);
}

void GraphViewer::save_screenshot(const std::string &fname)
{
  QPixmap pixMap = this->grab();
  pixMap.save(fname.c_str());
}

void GraphViewer::wheelEvent(QWheelEvent *event)
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

void GraphViewer::zoom_to_content()
{
  this->fitInView(this->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
}

} // namespace gngui
