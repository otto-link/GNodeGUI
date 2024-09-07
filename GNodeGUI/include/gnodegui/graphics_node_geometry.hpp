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

class GraphicsNodeGeometry
{
public:
  GraphicsNodeGeometry() = default;

  GraphicsNodeGeometry(NodeProxy *p_node_proxy);

  QSizeF  caption_size;
  QPointF caption_pos;
  QRectF  body_rect;
  int     full_width;
  int     full_height;

  std::vector<QRectF> port_label_rects;
  std::vector<QRectF> port_rects;

private:
  NodeProxy *p_node_proxy;
};

} // namespace gngui