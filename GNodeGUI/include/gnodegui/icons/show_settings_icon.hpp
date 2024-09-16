/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */

/**
 * @file lock_icon.hpp
 * @author Otto Link (otto.link.bv@gmail.com)
 * @brief
 * @copyright Copyright (c) 2024 Otto Link. Distributed under the terms of the
 * GNU General Public License. See the file LICENSE for the full license.
 */
#pragma once
#include <QGraphicsPathItem>

#include "gnodegui/icons/abstract_icon.hpp"

namespace gngui
{

class ShowSettingsIcon : public AbstractIcon
{
public:
  ShowSettingsIcon(float          width,
                   QColor         color = Qt::black,
                   float          pen_width = 1.f,
                   QGraphicsItem *parent = nullptr);

  void set_is_shown(bool new_state)
  {
    this->is_shown = new_state;
    this->set_path(); // update path
  };

  void set_path() override;

private:
  bool is_shown = false;
};

} // namespace gngui