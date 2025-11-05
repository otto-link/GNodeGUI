/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <sstream>
#include <string>
#include <vector>

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QObject>
#include <QPointer>
#include <QRectF>
#include <QTimer>

#include "gnodegui/graphics_link.hpp"
#include "gnodegui/graphics_node.hpp"
#include "gnodegui/logger.hpp"

namespace gngui
{

void clean_delete_graphics_item(QGraphicsItem *item)
{
  if (!item)
    return;

  // disable interaction immediately so Qt stops querying it
  item->setEnabled(false);
  item->setAcceptHoverEvents(false);
  item->setAcceptedMouseButtons(Qt::NoButton);
  item->setFlag(QGraphicsItem::ItemIsMovable, false);
  item->setFlag(QGraphicsItem::ItemIsSelectable, false);
  item->setFlag(QGraphicsItem::ItemHasNoContents, true); // prevent paint()

  // remove from scene now to stop further scene iteration referencing it
  if (item->scene())
    item->scene()->removeItem(item);

  // if QObject / QGraphicsObject, schedule deletion safely
  if (auto obj = dynamic_cast<QObject *>(item))
  {
    obj->disconnect();
    obj->deleteLater();
  }
  else
  {
    if (auto p_link = dynamic_cast<GraphicsLink *>(item))
    {
      QPointer<GraphicsLink> guard(p_link);
      QTimer::singleShot(0,
                         [guard]() mutable
                         {
                           // safe: delete executed after paint events
                           if (guard)
                             delete guard;
                         });
    }
    else if (auto p_node = dynamic_cast<GraphicsNode *>(item))
    {
      QPointer<GraphicsNode> guard(p_node);
      QTimer::singleShot(0,
                         [guard]() mutable
                         {
                           // safe: delete executed after paint events
                           if (guard)
                             delete guard;
                         });
    }
    else
    {
      QTimer::singleShot(0, [item]() { if (item) delete item; });
    }
  }
}

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
