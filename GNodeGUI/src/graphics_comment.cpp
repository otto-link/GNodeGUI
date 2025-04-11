/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <QPainter>

#include "gnodegui/graphics_comment.hpp"
#include "gnodegui/logger.hpp"
#include "gnodegui/style.hpp"

namespace gngui
{

GraphicsComment::GraphicsComment(QGraphicsItem *parent) : QGraphicsRectItem(parent)
{
  this->setFlag(QGraphicsItem::ItemIsSelectable, true);
  this->setFlag(QGraphicsItem::ItemIsMovable, true);
  this->setAcceptHoverEvents(true);
  this->setRect(0.f, 0.f, GN_STYLE->comment.width, 128.f);
  this->setZValue(-2);

  this->set_comment_text(
      "Comment. Comment. Comment. Comment. Comment. Comment. Comment. Comment. Comment. "
      "Comment. Comment. Comment. Comment. Comment. ");
}

void GraphicsComment::json_from(nlohmann::json json)
{
  std::vector<float> pos = json["position"];
  this->set_comment_text(this->comment_text);
  this->setPos(pos[0], pos[1]);
}

nlohmann::json GraphicsComment::json_to() const
{
  nlohmann::json json;

  json["comment_text"] = this->comment_text;
  json["position"] = {this->scenePos().x(), this->scenePos().y()};

  return json;
}

void GraphicsComment::paint(QPainter                       *painter,
                            const QStyleOptionGraphicsItem *option,
                            QWidget                        *widget)
{
  Q_UNUSED(option);
  Q_UNUSED(widget);

  // adjust rectangle height to the text height
  QFont        font = painter->font();
  QFontMetrics fm(font);
  QRect        bounding_rect = fm.boundingRect(
      QRect(0, 0, (int)GN_STYLE->comment.width, INT_MAX),
      Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
      this->comment_text.c_str());
  float height = (float)bounding_rect.height() + 4.f * GN_STYLE->comment.rounding_radius;

  this->setRect(0.f, 0.f, GN_STYLE->comment.width, height);

  // background
  QColor fill_color = GN_STYLE->comment.color_bg;
  fill_color.setAlphaF(GN_STYLE->comment.background_fill_alpha);
  painter->setBrush(fill_color);
  painter->setPen(Qt::NoPen);

  painter->drawRoundedRect(this->rect(),
                           GN_STYLE->comment.rounding_radius,
                           GN_STYLE->comment.rounding_radius);

  // comment text
  painter->setPen(QPen(GN_STYLE->comment.color_text));

  QRectF text_rect = this->rect();
  text_rect.adjust(2.f * GN_STYLE->comment.rounding_radius,
                   2.f * GN_STYLE->comment.rounding_radius,
                   -2.f * GN_STYLE->comment.rounding_radius,
                   -2.f * GN_STYLE->comment.rounding_radius);

  painter->drawText(text_rect,
                    Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                    this->comment_text.c_str());
}

void GraphicsComment::set_comment_text(const std::string &new_comment_text)
{
  this->comment_text = new_comment_text;
  this->update();
}

} // namespace gngui
