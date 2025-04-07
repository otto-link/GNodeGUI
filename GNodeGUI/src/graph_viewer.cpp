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

#include "gnodegui/icons/clear_all_icon.hpp"
#include "gnodegui/icons/dots_icon.hpp"
#include "gnodegui/icons/fit_content_icon.hpp"
#include "gnodegui/icons/group_icon.hpp"
#include "gnodegui/icons/link_type_icon.hpp"
#include "gnodegui/icons/load_icon.hpp"
#include "gnodegui/icons/new_icon.hpp"
#include "gnodegui/icons/reload_icon.hpp"
#include "gnodegui/icons/save_icon.hpp"
#include "gnodegui/icons/screenshot_icon.hpp"
#include "gnodegui/icons/select_all_icon.hpp"
#include "gnodegui/icons/viewport_icon.hpp"

#define MAX_SIZE 40000

namespace gngui
{

GraphViewer::GraphViewer(std::string id) : QGraphicsView(), id(id)
{
  Logger::log()->trace("GraphViewer::GraphViewer");
  this->setRenderHint(QPainter::Antialiasing);
  this->setRenderHint(QPainter::SmoothPixmapTransform);
  this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setDragMode(QGraphicsView::NoDrag);

  this->setScene(new QGraphicsScene());
  this->scene()->setSceneRect(-MAX_SIZE, -MAX_SIZE, (MAX_SIZE * 2), (MAX_SIZE * 2));

  this->setBackgroundBrush(QBrush(GN_STYLE->viewer.color_bg));

  if (GN_STYLE->viewer.add_toolbar)
    this->add_toolbar(GN_STYLE->viewer.toolbar_window_pos);
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

std::string GraphViewer::add_node(NodeProxy         *p_node_proxy,
                                  QPointF            scene_pos,
                                  const std::string &node_id)
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

  this->connect(p_node,
                &GraphicsNode::toggle_widget_visibility,
                this,
                &GraphViewer::on_node_settings_request);

  this->connect(p_node,
                &GraphicsNode::selected,
                [this](const std::string &id) { Q_EMIT this->node_selected(id); });

  this->connect(p_node,
                &GraphicsNode::deselected,
                [this](const std::string &id) { Q_EMIT this->node_deselected(id); });

  // if nothing provided, generate a unique id based on the object address
  std::string nid = node_id;

  if (node_id == "")
  {
    std::ostringstream oss;
    oss << std::to_string((unsigned long long)(void **)p_node);
    nid = oss.str();
  }

  p_node_proxy->set_id(nid);

  return nid;
}

void GraphViewer::add_static_item(QGraphicsItem *item, QPoint window_pos)
{
  item->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
  item->setFlag(QGraphicsItem::ItemIsMovable, false);

  this->add_item(item);
  this->static_items.push_back(item);
  this->static_items_positions.push_back(window_pos);
}

void GraphViewer::add_toolbar(QPoint window_pos)
{
  float  width = GN_STYLE->viewer.toolbar_width;
  QColor color = GN_STYLE->viewer.color_toolbar;
  qreal  pen_width = 1.f;

  int padding = (int)(0.2f * width);
  int x = window_pos.x();
  int y = window_pos.y();
  int dy = width + padding;

  auto group_icon = new GroupIcon(width, color, pen_width);
  this->add_static_item(group_icon, QPoint(x, y));
  y += dy;

  auto link_type_icon = new LinkTypeIcon(width, color, pen_width);
  this->add_static_item(link_type_icon, QPoint(x, y));
  y += dy;

  auto reload_icon = new ReloadIcon(width, color, pen_width);
  this->add_static_item(reload_icon, QPoint(x, y));
  y += dy;

  auto fit_content_icon = new FitContentIcon(width, color, pen_width);
  this->add_static_item(fit_content_icon, QPoint(x, y));
  y += dy;

  auto screenshot_icon = new ScreenshotIcon(width, color, pen_width);
  this->add_static_item(screenshot_icon, QPoint(x, y));
  y += dy;

  auto select_all_icon = new SelectAllIcon(width, color, pen_width);
  this->add_static_item(select_all_icon, QPoint(x, y));
  y += dy;

  auto clear_all_icon = new ClearAllIcon(width, color, pen_width);
  this->add_static_item(clear_all_icon, QPoint(x, y));
  y += dy;

  auto new_icon = new NewIcon(width, color, pen_width);
  if (GN_STYLE->viewer.add_new_icon)
  {
    y += 2.f * padding;
    this->add_static_item(new_icon, QPoint(x, y));
    y += dy;
  }

  auto load_icon = new LoadIcon(width, color, pen_width);
  auto save_icon = new SaveIcon(width, color, pen_width);
  if (GN_STYLE->viewer.add_load_save_icons)
  {
    this->add_static_item(load_icon, QPoint(x, y));
    y += dy;

    this->add_static_item(save_icon, QPoint(x, y));
    y += dy;
  }

  auto dots_icon = new DotsIcon(width, color, pen_width);
  this->add_static_item(dots_icon, QPoint(x, y));
  y += dy;

  auto viewport_icon = new ViewportIcon(width, color, pen_width);
  if (GN_STYLE->viewer.add_viewport_icon)
  {
    y += 2.f * padding;
    this->add_static_item(viewport_icon, QPoint(x, y));
    y += dy;
  }

  // add background
  QGraphicsRectItem *background = new QGraphicsRectItem(0.f,
                                                        0.f,
                                                        width + 2.f * padding,
                                                        y - dy + padding);
  background->setZValue(-1);
  background->setPen(QPen(QColor(0, 0, 0, 0)));
  background->setBrush(QBrush(QColor(0, 0, 0, 64)));

  QPoint pos = QPoint(window_pos.x() - padding, window_pos.y() - padding);
  this->add_static_item(background, pos);

  // add connections
  this->connect(group_icon,
                &AbstractIcon::hit_icon,
                [this]()
                {
                  QPoint  pos = QCursor::pos();
                  QPointF scene_pos = this->mapToScene(pos);
                  this->add_item(new GraphicsGroup(), scene_pos);
                });

  this->connect(reload_icon,
                &AbstractIcon::hit_icon,
                [this]() { Q_EMIT this->graph_reload_request(); });

  this->connect(link_type_icon,
                &AbstractIcon::hit_icon,
                [this]() { this->toggle_link_type(); });

  this->connect(fit_content_icon,
                &AbstractIcon::hit_icon,
                [this]() { this->zoom_to_content(); });

  this->connect(screenshot_icon,
                &AbstractIcon::hit_icon,
                [this]() { this->save_screenshot(); });

  this->connect(select_all_icon,
                &AbstractIcon::hit_icon,
                [this]() { this->select_all(); });

  this->connect(clear_all_icon,
                &AbstractIcon::hit_icon,
                [this]() { Q_EMIT this->graph_clear_request(); });

  this->connect(new_icon,
                &AbstractIcon::hit_icon,
                [this]() { Q_EMIT this->graph_new_request(); });

  if (GN_STYLE->viewer.add_load_save_icons)
  {
    this->connect(load_icon,
                  &AbstractIcon::hit_icon,
                  [this]() { Q_EMIT this->graph_load_request(); });

    this->connect(save_icon,
                  &AbstractIcon::hit_icon,
                  [this]() { Q_EMIT this->graph_save_as_request(); });
  }

  this->connect(dots_icon,
                &AbstractIcon::hit_icon,
                [this]() { Q_EMIT this->graph_settings_request(); });

  if (GN_STYLE->viewer.add_viewport_icon)
  {
    this->connect(viewport_icon,
                  &AbstractIcon::hit_icon,
                  [this]() { Q_EMIT this->viewport_request(); });
  }
}

void GraphViewer::clear()
{
  std::vector<QGraphicsItem *> items_to_delete = {};

  for (QGraphicsItem *item : this->scene()->items())
    if (!this->is_item_static(item))
    {
      item->setSelected(false);
      this->scene()->removeItem(item);
      items_to_delete.push_back(item);
    }

  this->viewport()->update();

  for (auto item : items_to_delete)
    delete item;
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
  text_box->setPlaceholderText(QStringLiteral("Filter or [SPACE]"));
  text_box->setClearButtonEnabled(true);
  // hesiod::resize_font(text_box, -2);

  QWidgetAction *text_box_action = new QWidgetAction(menu);
  text_box_action->setDefaultWidget(text_box);

  menu->addAction(text_box_action);

  // sort node types by category (not by types for the treeview)
  std::vector<std::pair<std::string, std::string>> pairs;
  for (auto itr = this->node_inventory.begin(); itr != this->node_inventory.end(); ++itr)
    pairs.push_back(*itr);

  sort(pairs.begin(),
       pairs.end(),
       [=](std::pair<std::string, std::string> &a, std::pair<std::string, std::string> &b)
       {
         if (a.second == b.second)
           return a.first < b.first;
         else
           return a.second < b.second;
       });

  // to keep track of created submenus
  std::map<std::string, QMenu *> category_map;

  for (auto &p : pairs)
  {
    const std::string              &action_name = p.first;
    const std::vector<std::string> &action_categories = split_string(p.second, '/');

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

          if (text.isEmpty() || text.compare(" ") == 0)
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
  }

  QGraphicsView::contextMenuEvent(event);
}

void GraphViewer::delete_graphics_link(GraphicsLink *p_link)
{
  Logger::log()->trace("GraphicsLink removing");

  if (!p_link)
  {
    Logger::log()->error("GraphViewer::delete_graphics_link: invalid link provided.");
    return;
  }

  GraphicsNode *node_out = p_link->get_node_out();
  GraphicsNode *node_in = p_link->get_node_in();
  int           port_out = p_link->get_port_out_index();
  int           port_in = p_link->get_port_in_index();

  Logger::log()->trace("GraphViewer::delete_graphics_link, {}:{} -> {}:{}",
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
  Logger::log()->trace("GraphicsNode removing, id: {}", p_node->get_id());

  if (!p_node)
  {
    Logger::log()->error("GraphViewer::delete_graphics_node: invalid node provided.");
    return;
  }

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
        Logger::log()->trace("item removed");
        delete item;
      }
    }
  }
}

void GraphViewer::drawForeground(QPainter *painter, const QRectF &rect)
{
  QGraphicsView::drawForeground(painter, rect);

  for (size_t k = 0; k < this->static_items.size(); k++)
  {
    // Keep the static item at a fixed position
    QPointF scene_pos = this->mapToScene(this->viewport()->rect().topLeft() +
                                         this->static_items_positions[k]);
    this->static_items[k]->setPos(scene_pos);
  }
}

void GraphViewer::export_to_graphviz(const std::string &fname)
{
  // after export: to convert, command line: dot export.dot -Tsvg > output.svg

  Logger::log()->trace("exporting to graphviz format...");

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

std::vector<std::string> GraphViewer::get_selected_node_ids()
{
  std::vector<std::string> ids = {};

  for (QGraphicsItem *item : this->scene()->items())
    if (GraphicsNode *p_node = dynamic_cast<GraphicsNode *>(item))
      if (p_node->isSelected())
        ids.push_back(p_node->get_id());

  return ids;
}

bool GraphViewer::is_item_static(QGraphicsItem *item)
{
  return !(std::find(this->static_items.begin(), this->static_items.end(), item) ==
           this->static_items.end());
}

void GraphViewer::json_from(nlohmann::json     json,
                            bool               clear_existing_content,
                            const std::string &prefix_id)
{
  // generate graph from json data
  if (clear_existing_content)
  {
    this->clear();
    this->id = json["id"];
    this->current_link_type = json["current_link_type"].get<LinkType>();
  }

  if (!json["groups"].is_null())
  {
    for (auto &json_group : json["groups"])
    {
      GraphicsGroup *p_group = new GraphicsGroup();
      this->add_item(p_group);
      p_group->json_from(json_group);
    }
  }

  if (!json["nodes"].is_null())
  {
    for (auto &json_node : json["nodes"])
    {
      std::string nid = prefix_id + json_node["id"].get<std::string>();

      float x = json_node["scene_position.x"];
      float y = json_node["scene_position.y"];

      // nodes are not generated in this class, it is outsourced to the
      // outter headless nodes manager
      Q_EMIT this->new_graphics_node_request(nid, QPointF(x, y));

      this->get_graphics_node_by_id(nid)->json_from(json_node);

      Logger::log()->trace("{}", json_node["caption"].get<std::string>());
      Logger::log()->trace("{}", this->get_graphics_node_by_id(nid)->get_nports());
    }
  }

  if (!json["links"].is_null())
  {
    for (auto &json_link : json["links"])
    {
      std::string node_out_id = prefix_id + json_link["node_out_id"].get<std::string>();
      std::string node_in_id = prefix_id + json_link["node_in_id"].get<std::string>();
      std::string port_out_id = json_link["port_out_id"];
      std::string port_in_id = json_link["port_in_id"];

      // same here, the graphic links are generated but the data
      // connection itself is outsourced to the outter headless nodes
      // manager
      this->temp_link = new GraphicsLink(QColor(0, 0, 0, 0), this->current_link_type);
      this->scene()->addItem(this->temp_link);

      GraphicsNode *from_node = this->get_graphics_node_by_id(node_out_id);
      GraphicsNode *to_node = this->get_graphics_node_by_id(node_in_id);

      if (from_node && to_node)
      {
        int port_from_index = from_node->get_port_index(port_out_id);
        int port_to_index = to_node->get_port_index(port_in_id);

        this->on_connection_finished(from_node, port_from_index, to_node, port_to_index);
      }
      else
        Logger::log()->error(
            "GraphViewer::json_from, nodes instance cannot be found, IDs: {} and/or {}",
            node_out_id,
            node_in_id);
    }
  }
}

nlohmann::json GraphViewer::json_to() const
{
  nlohmann::json json;

  json["id"] = this->id;
  json["current_link_type"] = this->current_link_type;

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
  if (event->key() == Qt::Key_Shift)
  {
    this->setDragMode(QGraphicsView::NoDrag);
  }
  else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_A)
  {
    this->select_all();
  }
  else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_C)
  {
    std::vector<std::string> id_list = {};
    std::vector<QPointF>     scene_pos_list = {};

    for (QGraphicsItem *item : this->scene()->items())
      if (GraphicsNode *p_node = dynamic_cast<GraphicsNode *>(item))
        if (p_node->isSelected())
        {
          id_list.push_back(p_node->get_id());
          scene_pos_list.push_back(p_node->pos());
        }

    if (id_list.size())
      Q_EMIT this->nodes_copy_request(id_list, scene_pos_list);
  }
  else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_D)
  {
    std::vector<std::string> id_list = {};
    std::vector<QPointF>     scene_pos_list = {};

    for (QGraphicsItem *item : this->scene()->items())
      if (GraphicsNode *p_node = dynamic_cast<GraphicsNode *>(item))
        if (p_node->isSelected())
        {
          id_list.push_back(p_node->get_id());
          scene_pos_list.push_back(p_node->pos());
        }

    if (id_list.size())
      Q_EMIT this->nodes_duplicate_request(id_list, scene_pos_list);
  }
  else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_G)
  {
    QPoint  view_pos = this->mapFromGlobal(QCursor::pos());
    QPointF scene_pos = this->mapToScene(view_pos);
    this->add_item(new GraphicsGroup(), scene_pos);
  }
  else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_L)
  {
    this->toggle_link_type();
  }
  else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_I)
  {
    Q_EMIT this->graph_import_request();
  }
  else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_O)
  {
    Q_EMIT this->graph_load_request();
  }
  else if (event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier) &&
           event->key() == Qt::Key_S)
  {
    Q_EMIT this->graph_save_as_request();
  }
  else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Q)
  {
    Q_EMIT this->quit_request();
  }
  else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_S)
  {
    Q_EMIT this->graph_save_request();
  }
  else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_V)
  {
    Q_EMIT this->nodes_paste_request();
  }
  else if (event->key() == Qt::Key_Delete)
  {
    this->delete_selected_items();
  }

  QGraphicsView::keyReleaseEvent(event);
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

void GraphViewer::on_compute_finished(const std::string &id)
{
  this->get_graphics_node_by_id(id)->on_compute_finished();
}

void GraphViewer::on_compute_started(const std::string &id)
{
  this->get_graphics_node_by_id(id)->on_compute_started();
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

    Logger::log()->trace("GraphViewer::on_connection_dropped connection_dropped {}:{}",
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

        Logger::log()->trace("GraphViewer::on_connection_finished, {}:{} -> {}:{}",
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
      get_color_from_data_type(from_node->get_data_type(port_index)),
      this->current_link_type);

  QPointF port_pos = from_node->scenePos() +
                     from_node->get_geometry_ref()->port_rects[port_index].center();

  this->temp_link->set_endpoints(port_pos, port_pos);
  this->scene()->addItem(this->temp_link);

  Q_EMIT this->connection_started(from_node->get_id(),
                                  from_node->get_port_id(port_index));
}

void GraphViewer::on_node_reload_request(const std::string &id)
{
  Logger::log()->trace("GraphViewer::on_node_reload_request {}", id);
  Q_EMIT this->node_reload_request(id);
}

void GraphViewer::on_node_settings_request(const std::string &id)
{
  Logger::log()->trace("GraphViewer::on_node_settings_request {}", id);
  Q_EMIT this->node_settings_request(id);
}

void GraphViewer::on_node_right_clicked(const std::string &id, QPointF scene_pos)
{
  Q_EMIT this->node_right_clicked(id, scene_pos);
}

void GraphViewer::on_update_finished()
{
  if (GN_STYLE->viewer.disable_during_update)
    this->set_enabled(true);

  this->setCursor(Qt::ArrowCursor);
}

void GraphViewer::on_update_started()
{
  this->setCursor(Qt::WaitCursor);

  if (GN_STYLE->viewer.disable_during_update)
    this->set_enabled(false);
}

void GraphViewer::remove_node(const std::string &node_id)
{
  for (QGraphicsItem *item : this->scene()->items())
    if (GraphicsNode *p_node = dynamic_cast<GraphicsNode *>(item))
      if (p_node->get_id() == node_id)
        this->delete_graphics_node(p_node);
}

void GraphViewer::resizeEvent(QResizeEvent *event)
{
  QGraphicsView::resizeEvent(event);

  for (size_t k = 0; k < this->static_items.size(); k++)
  {
    // Map the desired position in the view to the scene coordinates
    // and set the position relative to the view
    QPointF scene_pos = this->mapToScene(this->viewport()->rect().topLeft() +
                                         this->static_items_positions[k]);
    this->static_items[k]->setPos(scene_pos);
  }
}

void GraphViewer::save_screenshot(const std::string &fname)
{
  QPixmap pixMap = this->grab();
  pixMap.save(fname.c_str());
}

void GraphViewer::select_all()
{
  for (QGraphicsItem *item : this->scene()->items())
    if (!is_item_static(item))
      item->setSelected(true);
}

void GraphViewer::set_enabled(bool state)
{
  this->setEnabled(state);
  this->setDragMode(QGraphicsView::NoDrag);
}

void GraphViewer::toggle_link_type()
{
  for (QGraphicsItem *item : this->scene()->items())
    if (GraphicsLink *p_link = dynamic_cast<GraphicsLink *>(item))
      this->current_link_type = p_link->toggle_link_type();
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
  QRectF bbox;

  // if there are no static items, the built-in scene bounding
  // rectangle is used. If not, the bounding box is recomputed with the
  // static items excluded
  if (this->static_items.empty())
    bbox = this->scene()->itemsBoundingRect();
  else
  {
    std::vector<QGraphicsItem *> items_not_static;

    for (QGraphicsItem *item : this->scene()->items())
    {
      if (!this->is_item_static(item))
        items_not_static.push_back(item);

      bbox = compute_bounding_rect(items_not_static);
    }
  }

  // add a margin
  float margin_x = 0.1f * bbox.width();
  float margin_y = 0.1f * bbox.height();
  bbox.adjust(-margin_x, -margin_y, margin_x, margin_y);

  this->fitInView(bbox, Qt::KeepAspectRatio);
}

} // namespace gngui
