/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#pragma once

#include "gnodegui/node_proxy.hpp"

namespace gngui
{

/**
 * The `GraphicsNodeGeometry` class provides the geometry specifications for
 * node layout, such as caption positioning, widget placement, and dimensions
 * for key elements like ports and settings. It allows customization of node
 * dimensions and facilitates the layout process for graphical nodes.
 */
class GraphicsNodeGeometry
{
public:
  GraphicsNodeGeometry() = default;
  GraphicsNodeGeometry(NodeProxy *p_node_proxy, QSizeF widget_size = QSizeF(0.f, 0.f));

  QSizeF  caption_size;
  QPointF caption_pos;
  QPointF widget_pos;
  QRectF  reload_rect;
  QRectF  settings_rect;
  QRectF  body_rect;
  QRectF  header_rect;
  int     full_width;
  int     full_height;

  std::vector<QRectF> port_label_rects;
  std::vector<QRectF> port_rects;

private:
  NodeProxy *p_node_proxy; // Pointer to the associated node proxy
};

} // namespace gngui
