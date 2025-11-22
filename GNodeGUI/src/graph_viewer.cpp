/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <fstream>
#include <iostream>

#include <QCoreApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QTimer>
#include <QToolTip>
#include <QWidgetAction>

#include "gnodegui/graph_viewer.hpp"
#include "gnodegui/graphics_comment.hpp"
#include "gnodegui/graphics_group.hpp"
#include "gnodegui/logger.hpp"
#include "gnodegui/style.hpp"
#include "gnodegui/utils.hpp"

#include "gnodegui/icons/clear_all_icon.hpp"
#include "gnodegui/icons/dots_icon.hpp"
#include "gnodegui/icons/fit_content_icon.hpp"
#include "gnodegui/icons/group_icon.hpp"
#include "gnodegui/icons/import_icon.hpp"
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

GraphViewer::GraphViewer(std::string id, QWidget *parent) : QGraphicsView(parent), id(id)
{
  Logger::log()->trace("GraphViewer::GraphViewer");
  this->setRenderHint(QPainter::Antialiasing);
  this->setRenderHint(QPainter::SmoothPixmapTransform);
  this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setDragMode(QGraphicsView::NoDrag);
  this->setFocusPolicy(Qt::StrongFocus);

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

void GraphViewer::add_link(const std::string &id_out,
                           const std::string &port_id_out,
                           const std::string &to_in,
                           const std::string &port_id_in)
{
  GraphicsNode *from_node = this->get_graphics_node_by_id(id_out);
  GraphicsNode *to_node = this->get_graphics_node_by_id(to_in);

  if (from_node && to_node)
  {
    int port_from_index = from_node->get_port_index(port_id_out);
    int port_to_index = to_node->get_port_index(port_id_in);

    QColor color = get_color_from_data_type(from_node->get_data_type(port_from_index));

    GraphicsLink *p_new_link = new GraphicsLink(color, this->current_link_type);

    p_new_link->set_pen_style(Qt::SolidLine);
    p_new_link->set_endnodes(from_node, port_from_index, to_node, port_to_index);
    p_new_link->update_path();

    // mark those ports as connected
    from_node->set_is_port_connected(port_from_index, p_new_link);
    to_node->set_is_port_connected(port_to_index, p_new_link);

    this->scene()->addItem(p_new_link);
  }
  else
  {
    Logger::log()->error(
        "GraphViewer::json_from, nodes instance cannot be found, IDs: {} and/or {}",
        id_out,
        to_in);
  }
}

std::string GraphViewer::add_node(NodeProxy         *p_node_proxy,
                                  QPointF            scene_pos,
                                  const std::string &node_id)
{
  GraphicsNode *p_node = new GraphicsNode(p_node_proxy);
  this->add_item(p_node, scene_pos);

  p_node->right_clicked = [this](const std::string &port_index, QPointF scene_pos)
  { this->on_node_right_clicked(port_index, scene_pos); };

  p_node->connection_started = [this](GraphicsNode *from, int port_index)
  { this->on_connection_started(from, port_index); };

  p_node->connection_finished =
      [this](GraphicsNode *from, int port_from_index, GraphicsNode *to, int port_to_index)
  { this->on_connection_finished(from, port_from_index, to, port_to_index); };

  p_node->connection_dropped =
      [this](GraphicsNode *from, int port_index, QPointF scene_pos)
  { this->on_connection_dropped(from, port_index, scene_pos); };

  p_node->selected = [this](const std::string &node_id)
  {
    Q_EMIT this->node_selected(node_id);
    Q_EMIT this->selection_has_changed();
  };

  p_node->deselected = [this](const std::string &node_id)
  {
    Q_EMIT this->node_deselected(node_id);
    Q_EMIT this->selection_has_changed();
  };

  // if nothing provided, generate a unique id based on the object address
  std::string nid = node_id.empty() ? std::to_string(reinterpret_cast<uintptr_t>(p_node))
                                    : node_id;

  p_node_proxy->set_id(nid);
  return nid;
}

void GraphViewer::add_static_item(QGraphicsItem *item, QPoint window_pos, float z_value)
{
  item->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
  item->setFlag(QGraphicsItem::ItemIsMovable, false);
  item->setZValue(z_value);

  this->add_item(item);
  this->static_items.push_back(item);
  this->static_items_positions.push_back(window_pos);
}

void GraphViewer::add_toolbar(QPoint window_pos)
{
  const float  width = GN_STYLE->viewer.toolbar_width;
  const QColor color = GN_STYLE->viewer.color_toolbar;
  const qreal  pen_width = 1.f;
  const int    padding = (int)(0.2f * width);
  const int    dy = width + padding;
  const float  z_value = 1.f;

  int x = window_pos.x();
  int y = window_pos.y();

  auto group_icon = new GroupIcon(width, color, pen_width);
  if (GN_STYLE->viewer.add_group)
  {
    this->add_static_item(group_icon, QPoint(x, y), z_value);
    y += dy;
  }

  auto link_type_icon = new LinkTypeIcon(width, color, pen_width);
  this->add_static_item(link_type_icon, QPoint(x, y), z_value);
  y += dy;

  auto reload_icon = new ReloadIcon(width, color, pen_width);
  this->add_static_item(reload_icon, QPoint(x, y), z_value);
  y += dy;

  auto fit_content_icon = new FitContentIcon(width, color, pen_width);
  this->add_static_item(fit_content_icon, QPoint(x, y), z_value);
  y += dy;

  auto screenshot_icon = new ScreenshotIcon(width, color, pen_width);
  this->add_static_item(screenshot_icon, QPoint(x, y), z_value);
  y += dy;

  auto select_all_icon = new SelectAllIcon(width, color, pen_width);
  this->add_static_item(select_all_icon, QPoint(x, y), z_value);
  y += dy;

  auto clear_all_icon = new ClearAllIcon(width, color, pen_width);
  this->add_static_item(clear_all_icon, QPoint(x, y), z_value);
  y += dy;

  auto new_icon = new NewIcon(width, color, pen_width);
  if (GN_STYLE->viewer.add_new_icon)
  {
    y += 2.f * padding;
    this->add_static_item(new_icon, QPoint(x, y), z_value);
    y += dy;
  }

  auto load_icon = new LoadIcon(width, color, pen_width);
  auto save_icon = new SaveIcon(width, color, pen_width);
  if (GN_STYLE->viewer.add_load_save_icons)
  {
    this->add_static_item(load_icon, QPoint(x, y), z_value);
    y += dy;

    this->add_static_item(save_icon, QPoint(x, y), z_value);
    y += dy;
  }

  auto import_icon = new ImportIcon(width, color, pen_width);
  if (GN_STYLE->viewer.add_import_icon)
  {
    this->add_static_item(import_icon, QPoint(x, y), z_value);
    y += dy;
  }

  auto dots_icon = new DotsIcon(width, color, pen_width);
  this->add_static_item(dots_icon, QPoint(x, y), z_value);
  y += dy;

  auto viewport_icon = new ViewportIcon(width, color, pen_width);
  if (GN_STYLE->viewer.add_viewport_icon)
  {
    y += 2.f * padding;
    this->add_static_item(viewport_icon, QPoint(x, y), z_value);
    y += dy;
  }

  // add background
  QGraphicsRectItem *background = new QGraphicsRectItem(0.f,
                                                        0.f,
                                                        width + 2.f * padding,
                                                        y - dy + padding);
  background->setPen(QPen(QColor(0, 0, 0, 0)));
  background->setBrush(QBrush(QColor(21, 21, 21, 255)));

  QPoint pos = QPoint(window_pos.x() - padding, window_pos.y() - padding);
  this->add_static_item(background, pos, z_value - 0.001f);

  // add connections
  if (GN_STYLE->viewer.add_group)
  {
    this->connect(group_icon,
                  &AbstractIcon::hit_icon,
                  [this]()
                  { this->add_item(new GraphicsGroup(), this->get_mouse_scene_pos()); });
  }

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

  if (GN_STYLE->viewer.add_import_icon)
  {
    this->connect(import_icon,
                  &AbstractIcon::hit_icon,
                  [this]() { Q_EMIT this->graph_import_request(); });
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
  this->select_all();
  this->delete_selected_items();

  Q_EMIT this->selection_has_changed();
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

  this->execute_new_node_context_menu();

  QGraphicsView::contextMenuEvent(event);
}

void GraphViewer::delete_graphics_link(GraphicsLink *p_link, bool link_will_be_replaced)
{
  if (!p_link)
  {
    Logger::log()->error("GraphViewer::delete_graphics_link: invalid link provided.");
    return;
  }

  GraphicsNode *node_out = p_link->get_node_out();
  GraphicsNode *node_in = p_link->get_node_in();
  int           port_out = p_link->get_port_out_index();
  int           port_in = p_link->get_port_in_index();

  const std::string node_out_id = node_out ? node_out->get_id() : "";
  const std::string node_in_id = node_in ? node_in->get_id() : "";
  const std::string node_out_port_id = node_out ? node_out->get_port_id(port_out) : "";
  const std::string node_in_port_id = node_in ? node_in->get_port_id(port_in) : "";

  Logger::log()->trace("Deleting link: {}:{} -> {}:{}, will_be_replaced={}",
                       node_out_id,
                       node_out_port_id,
                       node_in_id,
                       node_in_port_id,
                       link_will_be_replaced ? "T" : "F");

  // Disconnect nodes safely
  if (node_out)
    node_out->set_is_port_connected(port_out, nullptr);
  if (node_in)
    node_in->set_is_port_connected(port_in, nullptr);

  // Delete the link
  clean_delete_graphics_item(p_link);

  // Emit signal
  if (node_out && node_in)
    Q_EMIT connection_deleted(node_out_id,
                              node_out_port_id,
                              node_in_id,
                              node_in_port_id,
                              link_will_be_replaced);
}

void GraphViewer::delete_graphics_node(GraphicsNode *p_node)
{
  if (!p_node)
  {
    Logger::log()->error("GraphViewer::delete_graphics_node: invalid node provided.");
    return;
  }

  Logger::log()->trace("GraphicsNode removing, id: {}", p_node->get_id());

  // Remove any connected links
  QList<QGraphicsItem *> items_copy = scene()->items();
  for (QGraphicsItem *item : items_copy)
  {
    if (GraphicsLink *p_link = dynamic_cast<GraphicsLink *>(item))
    {
      if (p_link->get_node_out() == p_node || p_link->get_node_in() == p_node)
        this->delete_graphics_link(p_link, false);
    }
  }

  // Delete node
  const std::string deleted_id = p_node->get_id();
  clean_delete_graphics_item(p_node);

  Q_EMIT node_deleted(deleted_id);
}

void GraphViewer::delete_selected_items()
{
  QGraphicsScene *scene = this->scene();
  if (!scene)
    return;

  this->set_enabled(false);

  auto selected_items = scene->selectedItems();

  std::vector<GraphicsLink *>  links_to_delete;
  std::vector<GraphicsNode *>  nodes_to_delete;
  std::vector<QGraphicsItem *> other_items;

  // Separate items in a single pass
  for (QGraphicsItem *item : selected_items)
  {
    if (!scene->items().contains(item))
      continue;

    if (auto p_link = dynamic_cast<GraphicsLink *>(item))
      links_to_delete.push_back(p_link);
    else if (auto p_node = dynamic_cast<GraphicsNode *>(item))
      nodes_to_delete.push_back(p_node);
    else
      other_items.push_back(item);
  }

  // Delete links first
  for (auto p_link : links_to_delete)
    this->delete_graphics_link(p_link);

  // Then nodes
  for (auto p_node : nodes_to_delete)
    this->delete_graphics_node(p_node);

  // Finally, any remaining items
  for (auto item : other_items)
    clean_delete_graphics_item(item);

  this->set_enabled(true);

  Q_EMIT this->selection_has_changed();
}

void GraphViewer::deselect_all()
{
  auto items = scene()->items();

  for (QGraphicsItem *item : items)
    if (!is_item_static(item))
      item->setSelected(false);

  Q_EMIT this->selection_has_changed();
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

bool GraphViewer::execute_new_node_context_menu()
{
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

  this->connect(
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

  QAction *selected_action = menu->exec(QCursor::pos());

  if (selected_action)
  {
    Q_EMIT this->new_node_request(selected_action->text().toStdString(),
                                  this->get_mouse_scene_pos());
    return true;
  }
  else
  {
    return false;
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

GraphicsNode *GraphViewer::get_graphics_node_by_id(const std::string &node_id)
{
  auto items = scene()->items();

  for (QGraphicsItem *item : items)
    if (GraphicsNode *p_node = dynamic_cast<GraphicsNode *>(item))
      if (p_node->get_id() == node_id)
        return p_node;

  return nullptr;
}

QRectF GraphViewer::get_bounding_box()
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

    auto items = scene()->items();

    for (QGraphicsItem *item : items)
    {
      if (!this->is_item_static(item))
        items_not_static.push_back(item);

      bbox = compute_bounding_rect(items_not_static);
    }
  }

  return bbox;
}

std::string GraphViewer::get_id() const { return this->id; }

QPointF GraphViewer::get_mouse_scene_pos()
{
  QPoint  global_pos = QCursor::pos();
  QPoint  local_pos = this->mapFromGlobal(global_pos);
  QPointF scene_pos = this->mapToScene(local_pos);
  return scene_pos;
}

std::vector<std::string> GraphViewer::get_selected_node_ids(
    std::vector<QPointF> *p_scene_pos_list)
{
  std::vector<std::string> ids = {};
  auto                     items = scene()->items();

  for (QGraphicsItem *item : items)
    if (GraphicsNode *p_node = dynamic_cast<GraphicsNode *>(item))
      if (p_node->isSelected())
      {
        ids.push_back(p_node->get_id());

        // optional, returns node positions
        if (p_scene_pos_list)
          p_scene_pos_list->push_back(p_node->pos());
      }

  return ids;
}

bool GraphViewer::is_item_static(QGraphicsItem *item)
{
  return !(std::find(this->static_items.begin(), this->static_items.end(), item) ==
           this->static_items.end());
}

void GraphViewer::json_from(nlohmann::json json, bool clear_existing_content)
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

  if (!json["comments"].is_null())
  {
    for (auto &json_comment : json["comments"])
    {
      GraphicsComment *p_comment = new GraphicsComment();
      this->add_item(p_comment);
      p_comment->json_from(json_comment);
    }
  }

  if (!json["nodes"].is_null())
  {
    for (auto &json_node : json["nodes"])
    {
      std::string nid = json_node["id"].get<std::string>();

      float x = json_node["scene_position.x"];
      float y = json_node["scene_position.y"];

      // nodes are not generated in this class, it is outsourced to the
      // outter headless nodes manager. THERE IS NO NODE FACTORY AVAILABLE
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
      std::string node_out_id = json_link["node_out_id"].get<std::string>();
      std::string node_in_id = json_link["node_in_id"].get<std::string>();
      std::string port_out_id = json_link["port_out_id"];
      std::string port_in_id = json_link["port_in_id"];

      // the graphic links are generated (but the model connections
      // itself are outsourced to the outter headless nodes manager)
      this->add_link(node_out_id, port_out_id, node_in_id, port_in_id);
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
  std::vector<nlohmann::json> json_comment_list = {};

  for (QGraphicsItem *item : this->scene()->items())
  {
    if (GraphicsNode *p_node = dynamic_cast<GraphicsNode *>(item))
      json_node_list.push_back(p_node->json_to());
    else if (GraphicsLink *p_link = dynamic_cast<GraphicsLink *>(item))
      json_link_list.push_back(p_link->json_to());
    else if (GraphicsGroup *p_group = dynamic_cast<GraphicsGroup *>(item))
      json_group_list.push_back(p_group->json_to());
    else if (GraphicsComment *p_comment = dynamic_cast<GraphicsComment *>(item))
      json_comment_list.push_back(p_comment->json_to());
  }

  json["nodes"] = json_node_list;
  json["links"] = json_link_list;
  json["groups"] = json_group_list;
  json["comments"] = json_comment_list;

  return json;
}

void GraphViewer::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Shift)
    this->setDragMode(QGraphicsView::RubberBandDrag);

  if (event->key() == Qt::Key_Delete)
  {
    this->delete_selected_items();
    event->accept();
    return;
  }

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
    std::vector<QPointF>     scene_pos_list = {};
    std::vector<std::string> id_list = this->get_selected_node_ids(&scene_pos_list);

    if (id_list.size())
      Q_EMIT this->nodes_copy_request(id_list, scene_pos_list);
  }
  else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_D)
  {
    std::vector<QPointF>     scene_pos_list = {};
    std::vector<std::string> id_list = this->get_selected_node_ids(&scene_pos_list);

    if (id_list.size())
      Q_EMIT this->nodes_duplicate_request(id_list, scene_pos_list);
  }
  else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_B)
  {
    this->add_item(new GraphicsComment(), this->get_mouse_scene_pos());
  }
  else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_G)
  {
    if (GN_STYLE->viewer.add_group)
      this->add_item(new GraphicsGroup(), this->get_mouse_scene_pos());
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
  else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_P)
  {
    Q_EMIT this->graph_automatic_node_layout_request();
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

  QGraphicsView::keyReleaseEvent(event);
}

void GraphViewer::mouseMoveEvent(QMouseEvent *event)
{
  // temporary link follows the mouse
  if (this->temp_link)
  {
    // Update the end of the temporary cubic spline to follow the mouse
    QPointF end_pos = this->mapToScene(event->pos());
    this->temp_link->set_endpoints(this->temp_link->path().pointAtPercent(0), end_pos);
    this->temp_link->update_path();
  }

  QGraphicsView::mouseMoveEvent(event);
}

void GraphViewer::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::RightButton)
  {
    QGraphicsItem *item = this->itemAt(event->pos());

    if ((event->modifiers() & Qt::ControlModifier) && item)
    {
      // Ctrl + Right-Click on a link or a node to remove it
      if (GraphicsLink *p_link = dynamic_cast<GraphicsLink *>(item))
        this->delete_graphics_link(p_link);
      else if (GraphicsNode *p_node = dynamic_cast<GraphicsNode *>(item))
        this->delete_graphics_node(p_node);
      else if (GraphicsComment *p_comment = dynamic_cast<GraphicsComment *>(item))
        clean_delete_graphics_item(p_comment);

      // prevent context menu opening
      this->setContextMenuPolicy(Qt::NoContextMenu);

      // this is ugly... set context menu back
      QTimer::singleShot(200,
                         this,
                         [this]()
                         { this->setContextMenuPolicy(Qt::DefaultContextMenu); });

      event->accept();
      return;
    }

    QToolTip::hideText();
  }

  if (event->button() == Qt::LeftButton)
  {
    if (event->modifiers() & Qt::ShiftModifier)
    {
      this->setDragMode(QGraphicsView::RubberBandDrag);
      Q_EMIT this->rubber_band_selection_started();
    }
    else
    {
      this->setDragMode(QGraphicsView::ScrollHandDrag);
    }
  }

  QGraphicsView::mousePressEvent(event);
}

void GraphViewer::mouseReleaseEvent(QMouseEvent *event)
{
  this->setDragMode(QGraphicsView::NoDrag);
  Q_EMIT this->rubber_band_selection_finished();
  QGraphicsView::mouseReleaseEvent(event);
}

void GraphViewer::on_compute_finished(const std::string &node_id)
{
  this->get_graphics_node_by_id(node_id)->on_compute_finished();
}

void GraphViewer::on_compute_started(const std::string &node_id)
{
  this->get_graphics_node_by_id(node_id)->on_compute_started();
}

void GraphViewer::on_connection_dropped(GraphicsNode *from,
                                        int           port_index,
                                        QPointF       scene_pos)
{
  if (this->temp_link)
  {
    // Remove the temporary line
    clean_delete_graphics_item(this->temp_link);
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

    if (from_node != to_node && from_type != to_type)
    {
      // remove any existing connection linked to the node 'to' input
      if (!to_node->is_port_available(port_to_index))
      {
        Logger::log()->trace("GraphViewer::on_connection_finished: replace connection");

        // loop over all graphics
        GraphicsLink *p_link_to_delete = nullptr;

        for (QGraphicsItem *item : this->scene()->items())
          if (GraphicsLink *p_link = dynamic_cast<GraphicsLink *>(item))
            if (p_link != this->temp_link)
            {
              std::string link_node_id = p_link->get_node_in()->get_id();
              int         link_port_index = p_link->get_port_in_index();

              if (link_node_id == to_node->get_id() && link_port_index == port_to_index)
              {
                p_link_to_delete = p_link;
                break;
              }
            }

        // delete the link but prevent the graph update since it's
        // going to be updated after the new link will trigger an
        // update in the next step
        bool link_will_be_replaced = true;
        this->delete_graphics_link(p_link_to_delete, link_will_be_replaced);
      }

      // create new link
      if (from_node->is_port_available(port_from_index) &&
          to_node->is_port_available(port_to_index))
      {
        Logger::log()->trace("GraphViewer::on_connection_finished: new connection");

        // Finalize the connection
        QPointF port_from_pos = from_node->scenePos() + from_node->get_geometry()
                                                            .port_rects[port_from_index]
                                                            .center();
        QPointF port_to_pos = to_node->scenePos() +
                              to_node->get_geometry().port_rects[port_to_index].center();

        this->temp_link->set_endpoints(port_from_pos, port_to_pos);
        this->temp_link->set_pen_style(Qt::SolidLine);

        // from output to input
        {
          this->temp_link->set_endnodes(from_node,
                                        port_from_index,
                                        to_node,
                                        port_to_index);

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
    }
    else
    {
      // tried to connect but nothinh happens (same node from and to,
      // same port types...)
      clean_delete_graphics_item(temp_link);
    }
  }

  this->source_node = nullptr;
}

void GraphViewer::on_connection_started(GraphicsNode *from_node, int port_index)
{
  this->source_node = from_node;

  QColor color = get_color_from_data_type(from_node->get_data_type(port_index));
  this->temp_link = new GraphicsLink(color, this->current_link_type);

  QPointF port_pos = from_node->scenePos() +
                     from_node->get_geometry().port_rects[port_index].center();

  this->temp_link->set_endpoints(port_pos, port_pos);
  this->scene()->addItem(this->temp_link);

  Q_EMIT this->connection_started(from_node->get_id(),
                                  from_node->get_port_id(port_index));
}

void GraphViewer::on_node_reload_request(const std::string &node_id)
{
  Logger::log()->trace("GraphViewer::on_node_reload_request {}", node_id);
  Q_EMIT this->node_reload_request(node_id);
}

void GraphViewer::on_node_settings_request(const std::string &node_id)
{
  Logger::log()->trace("GraphViewer::on_node_settings_request {}", node_id);
  Q_EMIT this->node_settings_request(node_id);
}

void GraphViewer::on_node_right_clicked(const std::string &node_id, QPointF scene_pos)
{
  Q_EMIT this->node_right_clicked(node_id, scene_pos);
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
  auto items = scene()->items();

  for (QGraphicsItem *item : items)
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

  Q_EMIT this->selection_has_changed();
}

void GraphViewer::set_enabled(bool state)
{
  this->setEnabled(state);
  this->setDragMode(QGraphicsView::NoDrag);
}

void GraphViewer::set_node_as_selected(const std::string &node_id)
{
  GraphicsNode *p_node = this->get_graphics_node_by_id(node_id);

  if (p_node)
    p_node->setSelected(true);

  Q_EMIT this->selection_has_changed();
}

void GraphViewer::set_node_inventory(
    const std::map<std::string, std::string> &new_node_inventory)
{
  this->node_inventory = new_node_inventory;
}

void GraphViewer::toggle_link_type()
{
  for (QGraphicsItem *item : this->scene()->items())
    if (GraphicsLink *p_link = dynamic_cast<GraphicsLink *>(item))
      this->current_link_type = p_link->toggle_link_type();
}

void GraphViewer::unpin_nodes()
{
  for (QGraphicsItem *item : this->scene()->items())
    if (GraphicsNode *p_node = dynamic_cast<GraphicsNode *>(item))
      p_node->set_is_node_pinned(false);
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
  QRectF bbox = this->get_bounding_box();

  // add a margin
  float margin_x = 0.3f * bbox.width();
  float margin_y = 0.3f * bbox.height();
  bbox.adjust(-margin_x, -margin_y, margin_x, margin_y);

  this->fitInView(bbox, Qt::KeepAspectRatio);
}

} // namespace gngui
