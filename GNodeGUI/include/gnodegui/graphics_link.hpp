/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */

/**
 * @file graphics_link.hpp
 * @author Otto Link (otto.link.bv@gmail.com)
 * @brief This file contains the declaration of the GraphicsLink class which
 *        handles visual links between GraphicsNode objects in a graphical interface.
 * @copyright Copyright (c) 2024 Otto Link.
 * Distributed under the terms of the GNU General Public License.
 * See the file LICENSE for the full license.
 */
#pragma once
#include <memory>

#include <QGraphicsPathItem>
#include <QObject>

#include "nlohmann/json.hpp"

#include "gnodegui/graphics_node.hpp"

namespace gngui
{

class GraphicsNode;

/**
 * @enum LinkType
 * @brief Defines the various types of graphical links that can be drawn between nodes.
 */
enum LinkType
{
  BROKEN_LINE, ///< A broken or dashed line.
  CIRCUIT,     ///< A circuit-style link.
  CUBIC,       ///< A cubic bezier curve link.
  DEPORTED,    ///< A deported (offset) link.
  LINEAR       ///< A simple linear link.
};

/**
 * @class GraphicsLink
 * @brief Represents a visual link between two nodes in a graphical interface.
 *
 * This class handles graphical links that connect two GraphicsNode objects,
 * managing the link's appearance, interaction, and connection points.
 */
class GraphicsLink : public QObject, public QGraphicsPathItem
{
  Q_OBJECT

public:
  /**
   * @brief Constructs a new GraphicsLink object.
   *
   * @param color The color of the link (default is transparent).
   * @param link_type The type of the link, defined by the LinkType enum (default is
   * CUBIC).
   * @param parent The parent QGraphicsItem, if any (default is nullptr).
   */
  GraphicsLink(QColor         color = QColor(0, 0, 0, 0),
               LinkType       link_type = LinkType::CUBIC,
               QGraphicsItem *parent = nullptr);

  /**
   * @brief Gets the output node connected by this link.
   *
   * @return Pointer to the output GraphicsNode.
   */
  GraphicsNode *get_node_out() { return this->node_out; }

  /**
   * @brief Gets the index of the output port.
   *
   * @return Index of the output port.
   */
  int get_port_out_index() const { return this->port_out_index; }

  /**
   * @brief Gets the input node connected by this link.
   *
   * @return Pointer to the input GraphicsNode.
   */
  GraphicsNode *get_node_in() { return this->node_in; }

  /**
   * @brief Gets the index of the input port.
   *
   * @return Index of the input port.
   */
  int get_port_in_index() const { return this->port_in_index; }

  /**
   * @brief Serializes the link to a JSON object.
   *
   * @return A JSON representation of the link.
   */
  nlohmann::json json_to() const;

  /**
   * @brief Sets the nodes and ports that this link connects.
   *
   * @param from The output node.
   * @param port_from_index The index of the output port.
   * @param to The input node.
   * @param port_to_index The index of the input port.
   */
  void set_endnodes(GraphicsNode *from,
                    int           port_from_index,
                    GraphicsNode *to,
                    int           port_to_index);

  /**
   * @brief Sets the endpoints of the link.
   *
   * @param start_point The start point of the link.
   * @param end_point The end point of the link.
   */
  void set_endpoints(const QPointF &start_point, const QPointF &end_point);

  /**
   * @brief Sets the link type.
   *
   * @param new_link_type The new LinkType for the link.
   */
  void set_link_type(const LinkType &new_link_type);

  /**
   * @brief Sets the pen style for the link.
   *
   * @param new_pen_style The new pen style, based on Qt::PenStyle.
   */
  void set_pen_style(const Qt::PenStyle &new_pen_style)
  {
    this->pen_style = new_pen_style;
  }

  /**
   * @brief Toggles the link type through the available types.
   *
   * @return The new LinkType after toggling.
   */
  LinkType toggle_link_type();

protected:
  /**
   * @brief Provides the bounding rectangle for the link, accounting for additional
   * margins.
   *
   * @return The bounding rectangle.
   */
  QRectF boundingRect() const override;

  /**
   * @brief Handles the hover enter event when the mouse hovers over the link.
   *
   * @param event The hover event.
   */
  void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

  /**
   * @brief Handles the hover leave event when the mouse stops hovering over the link.
   *
   * @param event The hover event.
   */
  void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

  /**
   * @brief Paints the link on the scene.
   *
   * @param painter The QPainter used for drawing the link.
   * @param option The style options for the item.
   * @param widget The widget that is being painted on.
   */
  void paint(QPainter                       *painter,
             const QStyleOptionGraphicsItem *option,
             QWidget                        *widget) override;

  /**
   * @brief Overrides the shape to enlarge the clickable area around the link.
   *
   * @return The clickable shape of the link.
   */
  QPainterPath shape() const override;

private:
  QColor       color;                    ///< The color of the link.
  LinkType     link_type;                ///< The type of the link.
  Qt::PenStyle pen_style = Qt::DashLine; ///< The pen style of the link.

  bool is_link_hovered = false; ///< Flag to track if the link is being hovered.

  /**
   * @brief List of available link types.
   */
  std::vector<LinkType> link_types = {LinkType::BROKEN_LINE,
                                      LinkType::CUBIC,
                                      LinkType::DEPORTED,
                                      LinkType::LINEAR};

  GraphicsNode *node_out = nullptr; ///< The output node of the link.
  int           port_out_index;     ///< The output port index.
  GraphicsNode *node_in = nullptr;  ///< The input node of the link.
  int           port_in_index;      ///< The input port index.
};

} // namespace gngui
