#include <QPainterPath>
#include <QPen>

#include "gnodegui/icons/select_all_icon.hpp"
#include "gnodegui/logger.hpp"

namespace gngui
{

SelectAllIcon::SelectAllIcon(float          width,
                             QColor         color,
                             float          pen_width,
                             QGraphicsItem *parent)
    : AbstractIcon(width, color, pen_width, parent)
{
  this->set_path();
  this->tooltip = "Select all";
}

void SelectAllIcon::set_path()
{
  QPainterPath path;
  QPen         pen = this->pen();

  // rounded rectangle
  pen.setStyle(Qt::DashLine);
  this->setPen(pen);

  QRectF rect(0.f, 0.f, this->width, this->width);
  path.addRoundedRect(rect, 0.2f * this->width, 0.2f * this->width);

  this->setPath(path);
}

} // namespace gngui
