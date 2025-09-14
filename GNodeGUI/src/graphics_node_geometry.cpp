/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QFontMetrics>

#include "gnodegui/graphics_node_geometry.hpp"
#include "gnodegui/logger.hpp"
#include "gnodegui/style.hpp"

namespace gngui
{

GraphicsNodeGeometry::GraphicsNodeGeometry(NodeProxy *p_node_proxy, QSizeF widget_size)
    : p_node_proxy(p_node_proxy)
{
  Logger::log()->trace("GraphicsNodeGeometry::GraphicsNodeGeometry");

  // --- base font metrics and increments

  QFont        font;
  QFontMetrics font_metrics(font);

  float dy = GN_STYLE->node.vertical_stretching * font_metrics.height();
  float margin = 2.f * GN_STYLE->node.port_radius;
  float caption_to_ports_gap = GN_STYLE->node.header_height_scale * dy;

  // --- compute node width

  float node_width = std::max(GN_STYLE->node.width,
                              static_cast<float>(widget_size.width()) +
                                  2.f * GN_STYLE->node.padding_widget_width);

  // --- node caption
  this->caption_size = font_metrics.size(Qt::TextSingleLine,
                                         this->p_node_proxy->get_caption().c_str());
  this->caption_pos = QPointF(margin + GN_STYLE->node.padding, dy);

  // --- full node width and height including margins
  this->full_width = std::max(static_cast<float>(this->caption_size.width()) +
                                  2.f * GN_STYLE->node.padding,
                              node_width) +
                     2.f * margin;

  this->full_height = dy * (0.5f + this->p_node_proxy->get_nports()) +
                      caption_to_ports_gap + 2.f * margin;

  // add widget height if it exists
  if (widget_size.height() > 0)
    this->full_height += widget_size.height() +
                         2.f * GN_STYLE->node.padding_widget_height;

  // --- node body rectangle

  float y_body = this->caption_pos.y() + GN_STYLE->node.padding;
  this->body_rect = QRectF(margin, y_body, node_width, this->full_height - y_body);

  // --- node header rectangle

  this->header_rect = this->body_rect;
  this->header_rect.setHeight(caption_to_ports_gap);

  // --- buttons geometry

  float button_width = 0.7f * caption_to_ports_gap;
  float button_padding = 0.5f * (caption_to_ports_gap - button_width);

  this->settings_rect = QRectF(this->body_rect.right() - button_width -
                                   2.f * button_padding,
                               this->header_rect.top() + button_padding,
                               button_width,
                               button_width);

  this->reload_rect = QRectF(this->body_rect.right() - 2.f * button_width -
                                 3.f * button_padding,
                             this->header_rect.top() + button_padding,
                             button_width,
                             button_width);

  // --- ports geometry

  float ypos = this->header_rect.bottom() + GN_STYLE->node.padding;

  for (int k = 0; k < this->p_node_proxy->get_nports(); ++k)
  {
    float dx = 2.f * GN_STYLE->node.padding;

    // port label rectangle
    this->port_label_rects.push_back(
        QRectF(margin + dx, ypos, node_width - 2.f * dx, dy));

    // port circle rectangle
    float port_y = ypos + 0.5f * font_metrics.height() - GN_STYLE->node.port_radius;
    if (this->p_node_proxy->get_port_type(k) == PortType::IN)
    {
      this->port_rects.push_back(QRectF(margin - GN_STYLE->node.port_radius,
                                        port_y,
                                        2.f * GN_STYLE->node.port_radius,
                                        2.f * GN_STYLE->node.port_radius));
    }
    else
    {
      this->port_rects.push_back(QRectF(margin + node_width - GN_STYLE->node.port_radius,
                                        port_y,
                                        2.f * GN_STYLE->node.port_radius,
                                        2.f * GN_STYLE->node.port_radius));
    }

    ypos += dy;
  }

  // --- widget position rectangle
  this->widget_pos = QPointF(margin + GN_STYLE->node.padding_widget_width,
                             ypos + GN_STYLE->node.padding_widget_height);
}

} // namespace gngui
