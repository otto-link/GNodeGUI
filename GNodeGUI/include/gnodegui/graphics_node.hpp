/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */

/**
 * @file graphics_node.hpp
 * @author Otto Link (otto.link.bv@gmail.com)
 * @brief
 * @copyright Copyright (c) 2024 Otto Link. Distributed under the terms of the
 * GNU General Public License. See the file LICENSE for the full license.
 */
#pragma once
#include <memory>

#include <QEvent>
#include <QGraphicsRectItem>
#include <QMouseEvent>
#include <QObject>
#include <QWidget>

#include "nlohmann/json.hpp"

#include "gnodegui/graphics_link.hpp"
#include "gnodegui/graphics_node_geometry.hpp"
#include "gnodegui/logger.hpp"
#include "gnodegui/node_proxy.hpp"

namespace gngui
{

class GraphicsLink;

class GraphicsNode : public QObject, public QGraphicsRectItem
{
  Q_OBJECT

public:
  GraphicsNode(NodeProxy *p_node_proxy, QGraphicsItem *parent = nullptr);

  std::string get_caption() const { return this->p_node_proxy->get_caption(); }

  std::string get_category() const { return this->p_node_proxy->get_category(); }

  std::vector<std::string> get_category_splitted(char delimiter = '/') const;

  std::string get_data_type(int port_index) const
  {
    return this->p_node_proxy->get_data_type(port_index);
  }

  GraphicsNodeGeometry *get_geometry_ref() { return &(this->geometry); };

  std::string get_id() const { return this->p_node_proxy->get_id(); }

  std::string get_main_category() const;

  int get_nports() const { return this->p_node_proxy->get_nports(); }

  std::string get_port_caption(int port_index) const
  {
    return this->p_node_proxy->get_port_caption(port_index);
  }

  std::string get_port_id(int port_index) const
  {
    return this->p_node_proxy->get_port_id(port_index);
  }

  /**
   * @brief Retrieves the index of the port corresponding to the given identifier.
   *
   * This function searches for the port that matches the provided string identifier and
   * returns its index. If the identifier does not match any existing ports, it may return
   * a sentinel value (e.g., -1).
   *
   * @param id The string identifier of the port.
   * @return int The index of the port, or a sentinel value if the port is not found.
   */
  int get_port_index(const std::string &id) const;

  PortType get_port_type(int port_index) const
  {
    return this->p_node_proxy->get_port_type(port_index);
  }

  NodeProxy *get_proxy_ref() { return this->p_node_proxy; }

  virtual QWidget *get_qwidget_ref() { return this->p_node_proxy->get_qwidget_ref(); }

  // always returns true for outputs since we accept multiple links from one output to
  // multiple inputs
  bool is_port_available(int port_index);

  void json_from(nlohmann::json json);

  nlohmann::json json_to() const;

  // set to nullptr to flag a disconnect port
  void set_is_port_connected(int port_index, GraphicsLink *p_link)
  {
    this->connected_link_ref[port_index] = p_link;
  }

  void set_qwidget_visibility(bool is_visible);

public Q_SLOTS:
  void on_compute_finished();

  void on_compute_started();

Q_SIGNALS:
  void connection_dropped(GraphicsNode *from, int port_index, QPointF scene_pos);

  void connection_finished(GraphicsNode *from,
                           int           port_from_index,
                           GraphicsNode *to,
                           int           port_to_index);

  void connection_started(GraphicsNode *from, int port_index);

  void deselected(const std::string &id);

  void reload_request(const std::string &id);

  void right_clicked(const std::string &id, QPointF scene_pos);

  void selected(const std::string &id);

  void toggle_widget_visibility(const std::string &id);

protected:
  void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

  void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

  void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

  QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

  virtual void paint(QPainter                       *painter,
                     const QStyleOptionGraphicsItem *option,
                     QWidget                        *widget) override;

  bool sceneEventFilter(QGraphicsItem *watched, QEvent *event) override;

private:
  NodeProxy                  *p_node_proxy;
  GraphicsNodeGeometry        geometry;
  bool                        is_node_dragged = false;
  bool                        is_node_hovered = false;
  std::vector<bool>           is_port_hovered;
  std::vector<GraphicsLink *> connected_link_ref;
  bool                        is_node_computing = false;

  bool is_widget_visible = true;

  bool        has_connection_started = false;
  int         port_index_from;
  std::string data_type_connecting = "";

  int get_hovered_port_index() const;

  void update_geometry(QSizeF widget_size = QSizeF(-1.f, -1.f));

  // true any port has changed its hover state
  bool update_is_port_hovered(QPointF scene_pos);

  // all to false
  void reset_is_port_hovered();
};

} // namespace gngui
