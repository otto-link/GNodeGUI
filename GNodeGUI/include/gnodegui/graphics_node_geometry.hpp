/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */

/**
 * @file graphics_node_geometry.hpp
 * @author Otto Link (otto.link.bv@gmail.com)
 * @brief
 * @copyright Copyright (c) 2024 Otto Link. Distributed under the terms of the
 * GNU General Public License. See the file LICENSE for the full license.
 */
#pragma once

#include "gnodegui/node_proxy.hpp"

namespace gngui
{

/**
 * @class GraphicsNodeGeometry
 * @brief Stores and manages the spatial attributes of a graphical node.
 *
 * The `GraphicsNodeGeometry` class provides the geometry specifications for
 * node layout, such as caption positioning, widget placement, and dimensions
 * for key elements like ports and settings. It allows customization of node
 * dimensions and facilitates the layout process for graphical nodes.
 */
class GraphicsNodeGeometry
{
public:
  /**
   * @brief Default constructor for GraphicsNodeGeometry.
   */
  GraphicsNodeGeometry() = default;

  /**
   * @brief Constructs a GraphicsNodeGeometry with a specified node proxy and widget size.
   *
   * @param p_node_proxy Pointer to the node proxy associated with this geometry.
   * @param widget_size Size of the widget within the graphical node.
   */
  GraphicsNodeGeometry(NodeProxy *p_node_proxy, QSizeF widget_size = QSizeF(0.f, 0.f));

  QSizeF  caption_size;  /**< Size of the caption area within the node. */
  QPointF caption_pos;   /**< Position of the caption relative to the node. */
  QPointF widget_pos;    /**< Position of the widget within the node. */
  QRectF  reload_rect;   /**< Rectangle defining the reload area. */
  QRectF  settings_rect; /**< Rectangle defining the settings area. */
  QRectF  body_rect;     /**< Rectangle defining the main body area. */
  QRectF  header_rect;   /**< Rectangle defining the header area. */
  int     full_width;    /**< Full width of the node geometry. */
  int     full_height;   /**< Full height of the node geometry. */

  std::vector<QRectF>
      port_label_rects;           /**< Rectangles for each port label within the node. */
  std::vector<QRectF> port_rects; /**< Rectangles for each port in the node. */

private:
  NodeProxy *p_node_proxy; /**< Pointer to the associated node proxy. */
};

} // namespace gngui