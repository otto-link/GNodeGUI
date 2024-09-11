/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */

#include "gnodegui/style.hpp"

namespace gngui
{

// Initialize the static member
std::shared_ptr<Style> Style::instance = nullptr;

std::shared_ptr<Style> &Style::get_style()
{
  if (!instance)
    instance = std::make_shared<Style>();
  return instance;
}

QColor get_color_from_data_type(const std::string &data_type)
{
  if (GN_STYLE->node.color_port_data.contains(data_type))
    return GN_STYLE->node.color_port_data.at(data_type);
  else
    return GN_STYLE->node.color_port_data_default;
}

} // namespace gngui
