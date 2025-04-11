/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */

/**
 * @file graphics_node.hpp
 * @author Otto Link (otto.link.bv@gmail.com)
 * @brief Defines the GraphicsNode class, representing a graphical node in a scene.
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

/**
 * @class GraphicsNode
 * @brief Represents a graphical node with interactive elements in a GUI scene.
 */
class GraphicsNode : public QObject, public QGraphicsRectItem
{
  Q_OBJECT

public:
  /**
   * @brief Constructs a GraphicsNode with a given node proxy and optional parent item.
   *
   * This class integrates with a `NodeProxy` to interact with the underlying computation
   * logic, ensuring that the GUI representation reflects the computational state of the
   * node. It is designed to be used in node-based graphical user interfaces (GUIs) where
   * users can visually construct, modify, and inspect computational graphs or workflows.
   *
   * @param p_node_proxy Pointer to the associated NodeProxy instance.
   * @param parent Optional parent QGraphicsItem.
   */
  GraphicsNode(NodeProxy *p_node_proxy, QGraphicsItem *parent = nullptr);

  /**
   * @brief Retrieves the node's caption.
   * @return The caption of the node.
   */
  std::string get_caption() const { return this->p_node_proxy->get_caption(); }

  /**
   * @brief Retrieves the node's category.
   * @return The category of the node.
   */
  std::string get_category() const { return this->p_node_proxy->get_category(); }

  /**
   * @brief Splits the category by a specified delimiter.
   * @param delimiter Character to split the category string.
   * @return Vector of category parts.
   */
  std::vector<std::string> get_category_splitted(char delimiter = '/') const;

  /**
   * @brief Retrieves the data type of a specific port by index.
   * @param port_index Index of the port.
   * @return Data type as a string.
   */
  std::string get_data_type(int port_index) const
  {
    return this->p_node_proxy->get_data_type(port_index);
  }

  /**
   * @brief Provides a reference to the node's geometry object.
   * @return Reference to GraphicsNodeGeometry.
   */
  GraphicsNodeGeometry *get_geometry_ref() { return &(this->geometry); };

  /**
   * @brief Provides a reference to the node's geometry object.
   * @return Reference to GraphicsNodeGeometry.
   */
  std::string get_id() const { return this->p_node_proxy->get_id(); }

  /**
   * @brief Provides a reference to the node's geometry object.
   * @return Reference to GraphicsNodeGeometry.
   */
  std::string get_main_category() const;

  /**
   * @brief Provides a reference to the node's geometry object.
   * @return Reference to GraphicsNodeGeometry.
   */
  int get_nports() const { return this->p_node_proxy->get_nports(); }

  /**
   * @brief Provides a reference to the node's geometry object.
   * @return Reference to GraphicsNodeGeometry.
   */
  std::string get_port_caption(int port_index) const
  {
    return this->p_node_proxy->get_port_caption(port_index);
  }

  /**
   * @brief Provides a reference to the node's geometry object.
   * @return Reference to GraphicsNodeGeometry.
   */
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

  /**
   * @brief Provides a reference to the node's geometry object.
   * @return Reference to GraphicsNodeGeometry.
   */
  PortType get_port_type(int port_index) const
  {
    return this->p_node_proxy->get_port_type(port_index);
  }

  /**
   * @brief Provides a reference to the node's geometry object.
   * @return Reference to GraphicsNodeGeometry.
   */
  NodeProxy *get_proxy_ref() { return this->p_node_proxy; }

  /**
   * @brief Provides a reference to the node's geometry object.
   * @return Reference to GraphicsNodeGeometry.
   */
  virtual QWidget *get_qwidget_ref() { return this->p_node_proxy->get_qwidget_ref(); }

  /**
 * @brief Provides a reference to the node's geometry object (always returns true for
 outputs since we accept multiple links from one output to multiple inputs).
 * @return Reference to GraphicsNodeGeometry.
 */
  bool is_port_available(int port_index);

  /**
   * @brief Loads node data from a JSON object.
   * @param json JSON object containing node data.
   */
  void json_from(nlohmann::json json);

  /**
   * @brief Loads node data from a JSON object.
   * @param json JSON object containing node data.
   */
  nlohmann::json json_to() const;

  /**
   * @brief Loads node data from a JSON object (is set to nullptr to flag a disconnect
   * port).
   * @param json JSON object containing node data.
   */
  void set_is_port_connected(int port_index, GraphicsLink *p_link)
  {
    this->connected_link_ref[port_index] = p_link;
  }

  void set_p_node_proxy(NodeProxy *new_p_node_proxy);

  /**
   * @brief Sets the visibility of the associated QWidget.
   * @param visible True to make visible, false to hide.
   */
  void set_qwidget_visibility(bool is_visible);

  void update_proxy_widget();

public Q_SLOTS:
  /**
   * @brief Slot called when node computation is finished.
   */
  void on_compute_finished();

  /**
   * @brief Slot called when node computation starts.
   */
  void on_compute_started();

Q_SIGNALS:
  /**
   * @brief Emitted when a connection is dropped from a port.
   * @param from The node from which the connection was dropped.
   * @param port_index The index of the port.
   * @param scene_pos The scene position of the drop.
   */
  void connection_dropped(GraphicsNode *from, int port_index, QPointF scene_pos);

  /**
   * @brief Emitted when a connection is successfully established between ports.
   * @param from The source node of the connection.
   * @param port_from_index The index of the source port.
   * @param to The destination node of the connection.
   * @param port_to_index The index of the destination port.
   */
  void connection_finished(GraphicsNode *from,
                           int           port_from_index,
                           GraphicsNode *to,
                           int           port_to_index);

  /**
   * @brief Emitted when a connection attempt starts from a port.
   * @param from The source node initiating the connection.
   * @param port_index The index of the source port.
   */
  void connection_started(GraphicsNode *from, int port_index);

  /**
   * @brief Emitted when the node is deselected.
   * @param id The unique identifier of the node.
   */
  void deselected(const std::string &id);

  /**
   * @brief Emitted to request a reload of the node.
   * @param id The unique identifier of the node.
   */
  void reload_request(const std::string &id);

  /**
   * @brief Emitted when the node is right-clicked.
   * @param id The unique identifier of the node.
   * @param scene_pos The scene position of the click.
   */
  void right_clicked(const std::string &id, QPointF scene_pos);

  /**
   * @brief Emitted when the node is selected.
   * @param id The unique identifier of the node.
   */
  void selected(const std::string &id);

  /**
   * @brief Emitted to toggle the visibility of the associated widget.
   * @param id The unique identifier of the node.
   */
  void toggle_widget_visibility(const std::string &id);

protected:
  /**
   * @brief Handles the event when the mouse hover enters the node area.
   * @param event Hover enter event details.
   */
  void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

  /**
   * @brief Handles the event when the mouse hover leaves the node area.
   * @param event Hover leave event details.
   */
  void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

  /**
   * @brief Handles the event when the mouse moves within the node area.
   * @param event Hover move event details.
   */
  void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

  /**
   * @brief Handles changes to the item, such as position or visibility.
   * @param change Type of item change.
   * @param value New value for the item change.
   * @return The modified QVariant value.
   */
  QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

  /**
   * @brief Handles the mouse press event on the node.
   * @param event Mouse press event details.
   */
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

  /**
   * @brief Handles the mouse release event on the node.
   * @param event Mouse release event details.
   */
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

  /**
   * @brief Custom paint function for rendering the node.
   * @param painter Painter object used for drawing.
   * @param option Options for the graphics item.
   * @param widget Widget on which the node is drawn.
   */
  virtual void paint(QPainter                       *painter,
                     const QStyleOptionGraphicsItem *option,
                     QWidget                        *widget) override;

  /**
   * @brief Filters events occurring on the scene that the node observes.
   * @param watched Item being watched.
   * @param event Event details.
   * @return True if the event is handled, otherwise false.
   */
  bool sceneEventFilter(QGraphicsItem *watched, QEvent *event) override;

private:
  NodeProxy           *p_node_proxy; /**< Pointer to the associated NodeProxy instance. */
  GraphicsNodeGeometry geometry;     /**< Geometry data for the node. */
  bool is_node_dragged = false; /**< Indicates if the node is currently being dragged. */
  bool is_node_hovered = false; /**< Indicates if the mouse is hovering over the node. */
  std::vector<bool> is_port_hovered; /**< Flags for each port's hover state. */
  std::vector<GraphicsLink *>
       connected_link_ref;        /**< References to links connected to this node. */
  bool is_node_computing = false; /**< Indicates if the node is currently computing. */
  bool is_widget_visible = true;  /**< Indicates if the associated widget is visible. */
  bool has_connection_started = false; /**< Tracks if a connection attempt has started. */
  int  port_index_from;                /**< Index of the port initiating a connection. */
  std::string data_type_connecting = ""; /**< Data type of the port currently attempting a
                                            connection. */
  QGraphicsProxyWidget *proxy_widget = nullptr;

  /**
   * @brief Retrieves the index of the port currently hovered by the mouse.
   * @return Index of the hovered port, or a sentinel value if none.
   */
  int get_hovered_port_index() const;

  /**
   * @brief Updates the geometry of the node based on the specified widget size.
   * @param widget_size Optional new widget size; default is (-1, -1) for no change.
   */
  void update_geometry(QSizeF widget_size = QSizeF(-1.f, -1.f));

  /**
   * @brief Updates the hover state of ports based on the mouse position.
   * @param scene_pos The current mouse position in the scene.
   * @return True if any port hover state changed, otherwise false.
   */
  bool update_is_port_hovered(QPointF scene_pos);

  /**
   * @brief Resets all port hover states to false.
   */
  void reset_is_port_hovered();
};

} // namespace gngui
