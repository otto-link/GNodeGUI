/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */

/**
 * @file style.hpp
 * @author Otto Link (otto.link.bv@gmail.com)
 * @brief
 * @copyright Copyright (c) 2024 Otto Link. Distributed under the terms of the
 * GNU General Public License. See the file LICENSE for the full license.
 */
#pragma once
#include <map>

#include <QColor>

namespace gngui
{

struct Style
{
  struct Editor
  {
    QColor color_bg = QColor(42, 42, 42, 255);
  } editor;

  struct Node
  {
    float width = 128.f;
    float padding = 6.f;
    float rounding_radius = 8.f;
    float port_radius = 6.f;
    float port_radius_not_selectable = 5.f;
    float vertical_stretching = 1.3f;

    QColor color_bg = QColor(102, 102, 102, 255);
    QColor color_bg_light = QColor(108, 108, 108, 255);
    QColor color_border = Qt::black;
    QColor color_border_hovered = Qt::black;
    QColor color_caption = Qt::white;
    QColor color_selected = QColor(80, 250, 123, 255);

    QColor color_port_hovered = Qt::white; // QColor(180, 180, 180, 255);
    QColor color_port_selected = QColor(80, 250, 123, 255);

    float pen_width = 1.5;
    float pen_width_hovered = 2;
    float pen_width_selected = 2;

    QColor color_port_data_default = Qt::lightGray;
    QColor color_port_not_selectable = QColor(102, 102, 102, 255);

    std::map<std::string, QColor> color_port_data = {
        {"float", QColor(139, 233, 253, 255)},
        {"int", QColor(189, 147, 249, 255)}};
  } node;

  struct Link
  {
    float  pen_width = 1.f;
    float  pen_width_hovered = 2.f;
    float  pen_width_selected = 3.f;
    float  port_tip_radius = 2.f;
    float  curvature = 0.5f;
    QColor color_default = Qt::lightGray;
    QColor color_selected = QColor(80, 250, 123, 255);
  } link;

  struct Group
  {
    float rounding_radius = 8.f;

    QColor color = Qt::white;
    QColor color_fill = QColor(255, 255, 255, 16);
    QColor color_hovered = Qt::lightGray;
    QColor color_selected = QColor(80, 250, 123, 255);
    QColor color_caption = Qt::white;

    bool bold_caption = false;
  } group;
};

static Style style;

QColor get_color_from_data_type(const std::string &data_type);

} // namespace gngui