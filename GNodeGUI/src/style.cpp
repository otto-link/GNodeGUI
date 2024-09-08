/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */

#include "gnodegui/style.hpp"

namespace gngui
{

QColor get_color_from_data_type(const std::string &data_type)
{
  if (style.node.color_port_data.contains(data_type))
    return style.node.color_port_data.at(data_type);
  else
    return style.node.color_port_data_default;
}

} // namespace gngui
