/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QFontMetrics>

#include "gnodegui/graphics_node_geometry.hpp"
#include "gnodegui/logger.hpp"
#include "gnodegui/style.hpp"

namespace gngui
{

GraphicsNodeGeometry::GraphicsNodeGeometry(NodeProxy *p_node_proxy)
    : p_node_proxy(p_node_proxy)
{
  SPDLOG->trace("GraphicsNodeGeometry::GraphicsNodeGeometry");

  QFont        font;
  QFontMetrics font_metrics(font);

  // base increment
  float dy = style.node.vertical_stretching * font_metrics.height();
  float margin = 2 * style.node.port_radius;

  float caption_to_ports_gap = dy;

  // node caption
  this->caption_size = font_metrics.size(Qt::TextSingleLine,
                                         this->p_node_proxy->get_caption().c_str());

  this->caption_pos = QPointF(margin + style.node.padding, dy);

  // Qt graphics item full width and height (including everything,
  // i.e. not only the node body)
  this->full_width = std::max((float)this->caption_size.width() + 2 * style.node.padding,
                              style.node.width) +
                     2 * margin;

  this->full_height = dy * (0.5f + this->p_node_proxy->get_nports()) +
                      caption_to_ports_gap + 2 * margin;

  // node body
  float ybody = this->caption_pos.y() + style.node.padding;
  this->body_rect = QRectF(margin, ybody, style.node.width, this->full_height - ybody);

  // ports bounding box
  float ypos = 1.5f * dy + caption_to_ports_gap;

  for (int k = 0; k < this->p_node_proxy->get_nports(); k++)
  {
    float dx = 2.f * style.node.padding;

    this->port_label_rects.push_back(
        QRectF(margin + dx, ypos, style.node.width - 2 * dx, dy));

    if (this->p_node_proxy->get_port_type(k) == PortType::IN)
      this->port_rects.push_back(
          QRectF(margin - style.node.port_radius,
                 ypos + 0.5f * font_metrics.height() - style.node.port_radius,
                 2 * style.node.port_radius,
                 2 * style.node.port_radius));
    else
      this->port_rects.push_back(
          QRectF(margin + style.node.width - style.node.port_radius,
                 ypos + 0.5f * font_metrics.height() - style.node.port_radius,
                 2 * style.node.port_radius,
                 2 * style.node.port_radius));

    ypos += dy;
  }
}

} // namespace gngui