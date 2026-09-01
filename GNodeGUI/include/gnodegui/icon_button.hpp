/* Copyright (c) 2026 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */

/**
 * @file icon_button.hpp
 * @author Otto Link (otto.link.bv@gmail.com)
 * @brief Defines the IconButton class, a plain QWidget that renders an
 *        AbstractIcon and reproduces its hover/click behavior.
 * @details IconButton hosts an AbstractIcon outside of any QGraphicsScene: the
 *          icon instance is only used as a description of the glyph (painter
 *          path, pen, brush, tooltip). Because the button is a regular widget,
 *          it can be parented to a QGraphicsView to build overlay chrome
 *          (e.g. the GraphViewer toolbar) that is completely decoupled from
 *          the scene transform.
 *
 * @copyright Copyright (c) 2026 Otto Link. Distributed under the terms of the
 *            GNU General Public License. See the file LICENSE for details.
 */

#pragma once
#include <memory>

#include <QWidget>

#include "gnodegui/icons/abstract_icon.hpp"

namespace gngui
{

/**
 * @class IconButton
 * @brief A widget-based button that displays an AbstractIcon glyph.
 *
 * The button takes ownership of the icon, which must not belong to a
 * QGraphicsScene. Hovering dims the glyph, pressing thickens the pen and emits
 * the clicked() signal, mirroring AbstractIcon's in-scene behavior.
 */
class IconButton : public QWidget
{
  Q_OBJECT

public:
  /**
   * @brief Constructs an IconButton wrapping the given icon.
   *
   * @param icon The icon to display; ownership is transferred to the button.
   * @param parent The parent widget, if any.
   */
  IconButton(AbstractIcon *icon, QWidget *parent = nullptr);

Q_SIGNALS:
  /**
   * @brief Signal emitted when the button is clicked (left button press).
   */
  void clicked();

protected:
  void enterEvent(QEnterEvent *event) override;
  void leaveEvent(QEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;

private:
  /**
   * @brief The icon providing the glyph path, pen, brush and tooltip.
   */
  std::unique_ptr<AbstractIcon> icon;

  /**
   * @brief Extra pen width applied while the button is pressed.
   */
  float extra_pen_width = 0.f;

  /**
   * @brief Opacity factor applied while the button is hovered.
   */
  qreal opacity_factor = 1.;
};

} // namespace gngui
