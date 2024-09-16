/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <sstream>
#include <string>
#include <vector>

#include <QGraphicsItem>
#include <QRectF>

namespace gngui
{

QRectF compute_bounding_rect(const std::vector<QGraphicsItem *> &items)
{
  // Return an empty rectangle if there are no items
  if (items.empty())
    return QRectF();

  // Initialize with the first item
  QRectF bounding_rect = items.front()->sceneBoundingRect();

  // Iterate over the rest of the items and unite their bounding rects
  for (const QGraphicsItem *item : items)
    bounding_rect = bounding_rect.united(item->sceneBoundingRect());

  return bounding_rect;
}

std::vector<std::string> split_string(const std::string &string, char delimiter)
{
  std::vector<std::string> result;
  std::stringstream        ss(string);
  std::string              word;

  while (std::getline(ss, word, delimiter))
    result.push_back(word);

  return result;
}

} // namespace gngui
