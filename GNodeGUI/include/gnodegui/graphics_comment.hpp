/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#pragma once
#include <QGraphicsRectItem>

#include "nlohmann/json.hpp"

namespace gngui
{

class GraphicsComment : public QGraphicsRectItem
{
public:
  GraphicsComment(QGraphicsItem *parent = nullptr);

  void           json_from(nlohmann::json json);
  nlohmann::json json_to() const;

  void set_comment_text(const std::string &new_comment_text);

  void         mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
  virtual void paint(QPainter                       *painter,
                     const QStyleOptionGraphicsItem *option,
                     QWidget                        *widget) override;

private:
  std::string comment_text;
};

} // namespace gngui
