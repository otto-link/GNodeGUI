/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */

/**
 * @file utils.hpp
 * @author Otto Link (otto.link.bv@gmail.com)
 * @brief
 * @copyright Copyright (c) 2024 Otto Link. Distributed under the terms of the
 * GNU General Public License. See the file LICENSE for the full license.
 */
#pragma once
#include <string>
#include <vector>

#include <QGraphicsItem>
#include <QRectF>

namespace gngui
{

void   clean_delete_graphics_item(QGraphicsItem *item);
QRectF compute_bounding_rect(const std::vector<QGraphicsItem *> &items);

std::vector<std::string> split_string(const std::string &string, char delimiter);

template <typename T>
inline void json_safe_get(const nlohmann::json &j, const std::string &key, T &value)
{
  if (j.contains(key))
  {
    value = j.at(key).get<T>();
  }
  else
  {
    Logger::log()->error("Required json key \"{}\" not found.", key);
  }
}

} // namespace gngui
