/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <sstream>

#include <QApplication>
#include <QGraphicsProxyWidget>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#include "gnodegui/graphics_link.hpp"
#include "gnodegui/graphics_node.hpp"
#include "gnodegui/icons/reload_icon.hpp"
#include "gnodegui/icons/show_settings_icon.hpp"
#include "gnodegui/logger.hpp"
#include "gnodegui/style.hpp"
#include "gnodegui/utils.hpp"

namespace gngui
{

GraphicsNode::GraphicsNode(NodeProxy *p_node_proxy, QGraphicsItem *parent)
    : QGraphicsRectItem(parent), p_node_proxy(p_node_proxy)
{
  this->setFlag(QGraphicsItem::ItemIsSelectable, true);
  this->setFlag(QGraphicsItem::ItemIsMovable, true);
  this->setFlag(QGraphicsItem::ItemDoesntPropagateOpacityToChildren, false);
  this->setFlag(QGraphicsItem::ItemIsFocusable, true);
  this->setFlag(QGraphicsItem::ItemClipsChildrenToShape, false);
  this->setAcceptHoverEvents(true);
  this->setOpacity(1.f);
  this->setZValue(0);

  this->is_port_hovered.resize(this->get_nports());
  this->connected_link_ref.resize(this->get_nports());

  this->update_geometry();

  // add buttons
  if (GN_STYLE->node.reload_button)
  {
    gngui::ReloadIcon *reload = new gngui::ReloadIcon(this->geometry.reload_rect.width(),
                                                      GN_STYLE->node.color_icon,
                                                      GN_STYLE->node.pen_width,
                                                      this);
    reload->setPos(this->geometry.reload_rect.topLeft());

    this->connect(reload,
                  &ReloadIcon::hit_icon,
                  [this]() { Q_EMIT this->reload_request(this->get_id()); });
  }

  if (GN_STYLE->node.settings_button)
  {
    gngui::ShowSettingsIcon *settings = new gngui::ShowSettingsIcon(
        this->geometry.settings_rect.width(),
        GN_STYLE->node.color_icon,
        GN_STYLE->node.pen_width,
        this);
    settings->setPos(this->geometry.settings_rect.topLeft());

    this->connect(settings,
                  &ShowSettingsIcon::hit_icon,
                  [this]() { Q_EMIT this->toggle_widget_visibility(this->get_id()); });

    this->connect(settings,
                  &ShowSettingsIcon::hit_icon,
                  [this]()
                  {
                    this->is_widget_visible = !this->is_widget_visible;
                    this->set_qwidget_visibility(this->is_widget_visible);
                  });
  }

  // add widget
  if (QWidget *widget = this->p_node_proxy->get_qwidget_ref())
  {
    // ensure it's a top-level widget
    widget->setParent(nullptr);

    QGraphicsProxyWidget *proxy_widget = new QGraphicsProxyWidget(this);
    proxy_widget->setWidget(widget);
    proxy_widget->resize(this->p_node_proxy->get_qwidget_size());
    QSizeF widget_size = proxy_widget->size();

    // update the geometry
    this->update_geometry(widget_size);
    proxy_widget->setPos(this->geometry.widget_pos);
  }
}

std::vector<std::string> GraphicsNode::get_category_splitted(char delimiter) const
{
  return split_string(this->get_category(), delimiter);
}

int GraphicsNode::get_hovered_port_index() const
{
  auto it = std::find(this->is_port_hovered.begin(), this->is_port_hovered.end(), true);

  // if found, calculate the index; otherwise, return -1
  if (it != this->is_port_hovered.end())
  {
    int index = std::distance(this->is_port_hovered.begin(), it);
    return index;
  }
  else
    return -1;
}

std::string GraphicsNode::get_main_category() const
{
  std::string node_category = this->get_category();
  int         pos = node_category.find("/");
  return node_category.substr(0, pos);
}

int GraphicsNode::get_port_index(const std::string &id) const
{
  for (int k = 0; k < this->get_nports(); k++)
    if (this->get_port_id(k) == id)
      return k;

  return -1;
}

void GraphicsNode::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
  this->is_node_hovered = true;
  this->update();

  QGraphicsRectItem::hoverEnterEvent(event);
}

void GraphicsNode::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
  this->is_node_hovered = false;
  this->update();

  QGraphicsRectItem::hoverLeaveEvent(event);
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

bool GraphicsNode::is_port_available(int port_index)
{
  if (this->get_port_type(port_index) == PortType::OUT)
    return true;
  else
    return !this->connected_link_ref[port_index];
}

void GraphicsNode::json_from(nlohmann::json json)
{
  this->get_id() = json["id"];
  this->get_caption() = json["caption"];
  this->is_widget_visible = json["is_widget_visible"];

  float x = json["scene_position.x"];
  float y = json["scene_position.y"];
  this->setPos(QPointF(x, y));
}

nlohmann::json GraphicsNode::json_to() const
{
  nlohmann::json json;

  json["id"] = this->get_id();
  json["caption"] = this->get_caption();
  json["is_widget_visible"] = this->is_widget_visible;
  json["scene_position.x"] = this->scenePos().x();
  json["scene_position.y"] = this->scenePos().y();

  return json;
}

void GraphicsNode::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    int hovered_port_index = this->get_hovered_port_index();

    if (hovered_port_index >= 0)
    {
      GUILOG->trace("connection_started {}:{}", this->get_id(), hovered_port_index);

      this->has_connection_started = true;
      this->setFlag(QGraphicsItem::ItemIsMovable, false);
      this->port_index_from = hovered_port_index;
      this->data_type_connecting = this->get_data_type(hovered_port_index);
      Q_EMIT connection_started(this, hovered_port_index);
      event->accept();
    }
    else
      this->is_node_dragged = true;
  }

  else if (event->button() == Qt::RightButton)
  {
    QPointF pos = event->pos();
    QPointF scene_pos = this->mapToScene(pos);
    Q_EMIT this->right_clicked(this->get_id(), scene_pos);
  }

  QGraphicsRectItem::mousePressEvent(event);
}

void GraphicsNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    if (this->is_node_dragged)
    {
      this->is_node_dragged = false;
    }
    else if (this->has_connection_started)
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
            GUILOG->trace("connection_finished {}:{}",
                          target_node->get_id(),
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
        GUILOG->trace("GraphicsNode::mouseReleaseEvent connection_dropped {}",
                      this->get_id());
        Q_EMIT connection_dropped(this, this->port_index_from, event->scenePos());
      }

      this->has_connection_started = false;

      // clean-up port color state
      for (QGraphicsItem *item : this->scene()->items())
        if (GraphicsNode *node = dynamic_cast<GraphicsNode *>(item))
        {
          node->data_type_connecting = "";
          node->update();
        }

      // this->data_type_connecting = "";

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

  // --- Background rectangle

  painter->setBrush(QBrush(GN_STYLE->node.color_bg));
  painter->setPen(Qt::NoPen);
  painter->drawRoundedRect(this->geometry.body_rect,
                           GN_STYLE->node.rounding_radius,
                           GN_STYLE->node.rounding_radius);

  // --- Caption

  // Set pen based on whether the node is selected or not
  painter->setPen(this->isSelected() ? GN_STYLE->node.color_selected
                                     : GN_STYLE->node.color_caption);
  painter->drawText(this->geometry.caption_pos, this->get_caption().c_str());

  // --- Header

  std::string main_category = this->get_main_category();
  if (GN_STYLE->node.color_category.contains(main_category))
    painter->setBrush(GN_STYLE->node.color_category.at(main_category));
  else
    painter->setBrush(GN_STYLE->node.color_bg_light);

  painter->setPen(Qt::NoPen);

  QPainterPath path;
  QRectF       rect = this->geometry.header_rect;
  float        radius = GN_STYLE->node.rounding_radius;

  path.moveTo(rect.left(), rect.bottom());
  path.lineTo(rect.left(), rect.top() + radius);
  path.arcTo(rect.left(), rect.top(), radius * 2, radius * 2, 180, -90);
  path.lineTo(rect.right() - radius, rect.top());
  path.arcTo(rect.right() - radius * 2, rect.top(), radius * 2, radius * 2, 90, -90);
  path.lineTo(rect.right(), rect.bottom());
  path.closeSubpath();

  painter->drawPath(path);

  // --- Border

  painter->setBrush(Qt::NoBrush);
  if (this->isSelected())
    painter->setPen(
        QPen(GN_STYLE->node.color_selected, GN_STYLE->node.pen_width_selected));
  else if (this->is_node_hovered)
    painter->setPen(
        QPen(GN_STYLE->node.color_border_hovered, GN_STYLE->node.pen_width_hovered));
  else
    painter->setPen(QPen(GN_STYLE->node.color_border, GN_STYLE->node.pen_width));

  painter->drawRoundedRect(this->geometry.body_rect,
                           GN_STYLE->node.rounding_radius,
                           GN_STYLE->node.rounding_radius);

  // --- Ports

  for (int k = 0; k < this->p_node_proxy->get_nports(); k++)
  {
    // Set alignment based on port type (IN/OUT)
    int align_flag = (this->get_port_type(k) == PortType::IN) ? Qt::AlignLeft
                                                              : Qt::AlignRight;

    // Draw port labels
    painter->setPen(Qt::white); // Assuming labels are always white
    painter->drawText(this->geometry.port_label_rects[k],
                      align_flag,
                      this->get_port_caption(k).c_str());

    // Port appearance when selected or not
    if (this->is_port_hovered[k])
      painter->setPen(
          QPen(GN_STYLE->node.color_port_hovered, GN_STYLE->node.pen_width_hovered));
    else
    {
      // if (this->isSelected())
      //   painter->setPen(QPen(GN_STYLE->node.color_selected,
      //   GN_STYLE->node.pen_width_selected));
      // else
      if (this->is_node_hovered)
        painter->setPen(
            QPen(GN_STYLE->node.color_border_hovered, GN_STYLE->node.pen_width_hovered));
      else
        painter->setPen(QPen(GN_STYLE->node.color_border, GN_STYLE->node.pen_width));
    }

    // Set port brush based on data type compatibility
    std::string data_type = this->get_data_type(k);
    float       port_radius = GN_STYLE->node.port_radius;

    if (!this->data_type_connecting.empty() && data_type != this->data_type_connecting)
    {
      painter->setBrush(GN_STYLE->node.color_port_not_selectable);
      port_radius = GN_STYLE->node.port_radius_not_selectable;
    }
    else
      painter->setBrush(get_color_from_data_type(data_type));

    // Draw the port as a circle (ellipse with equal width and height)
    painter->drawEllipse(this->geometry.port_rects[k].center(), port_radius, port_radius);
  }
}

void GraphicsNode::reset_is_port_hovered()
{
  this->is_port_hovered.assign(this->is_port_hovered.size(), false);
}

bool GraphicsNode::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
  if (GraphicsNode *node = dynamic_cast<GraphicsNode *>(watched))
  {

    // LOOKING FOR A PORT TO CONNECT: mouse move + connection started
    // (from node) (watched is the node at beginning of the link and
    // this the node currently being hovered and possibly the end of
    // the link)

    if (event->type() == QEvent::GraphicsSceneMouseMove && node->has_connection_started)
    {
      QGraphicsSceneMouseEvent *mouse_event = static_cast<QGraphicsSceneMouseEvent *>(
          event);

      QPointF item_pos = mouse_event->scenePos() - this->scenePos();

      // update current data type of the building connection
      if (this->data_type_connecting != node->data_type_connecting)
      {
        this->data_type_connecting = node->data_type_connecting;
        this->update();
      }

      // update hovering port status
      if (this->update_is_port_hovered(item_pos))
      {
        // if a port is hovered, check that the port type (in/out)
        // and data type are compatible with the incoming link,
        // deactivate hovering for this port
        for (int k = 0; k < this->get_nports(); k++)
          if (this->is_port_hovered[k])
          {
            int from_pidx = node->port_index_from;

            PortType from_ptype = node->get_port_type(from_pidx);
            PortType to_ptype = this->get_port_type(k);

            std::string from_pdata = node->get_data_type(from_pidx);
            std::string to_pdata = this->get_data_type(k);

            if (from_ptype == to_ptype || from_pdata != to_pdata)
              this->is_port_hovered[k] = false;
          }
        this->update();
      }
    }
  }

  return QGraphicsRectItem::sceneEventFilter(watched, event);
}

void GraphicsNode::set_qwidget_visibility(bool is_visible)
{
  // recompute geometry based on widget visiblity status
  QWidget *widget = this->get_qwidget_ref();
  QSizeF   widget_size = QSizeF(-1.f, -1.f);

  if (widget)
  {
    if (is_visible)
      widget_size = widget->size();

    widget->setVisible(is_visible);
  }

  this->update_geometry(widget_size);
  this->update();
}

void GraphicsNode::update_geometry(QSizeF widget_size)
{
  this->geometry = GraphicsNodeGeometry(this->p_node_proxy, widget_size);
  this->setRect(0.f, 0.f, this->geometry.full_width, this->geometry.full_height);
}

bool GraphicsNode::update_is_port_hovered(QPointF item_pos)
{
  // set hover state
  for (size_t k = 0; k < this->geometry.port_rects.size(); k++)
    if (this->geometry.port_rects[k].contains(item_pos))
    {
      this->is_port_hovered[k] = true;
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
