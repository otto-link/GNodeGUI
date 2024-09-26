/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */

/**
 * @file abstract_icon.hpp
 * @author Otto Link (otto.link.bv@gmail.com)
 * @brief Defines the AbstractIcon class which represents a customizable icon
 *        in a graphical user interface, derived from QObject and QGraphicsPathItem.
 *        This class is intended to be used as a base class for specific icons with
 *        custom graphical paths.
 * @details The AbstractIcon class provides mechanisms for handling hover, click, and
 *          tooltip functionality, and it allows derived classes to define their own
 *          paths by implementing the `set_path` method.
 *
 * @copyright Copyright (c) 2024 Otto Link. Distributed under the terms of the
 *            GNU General Public License. See the file LICENSE for details.
 */

#pragma once
#include <QGraphicsPathItem>

namespace gngui
{

/**
 * @class AbstractIcon
 * @brief A base class for creating customizable icons in a graphical user interface.
 *
 * The AbstractIcon class combines QObject and QGraphicsPathItem to create an icon that
 * supports interaction features such as hovering, clicking, and tooltip display.
 * It also allows for defining custom paths through the pure virtual `set_path` method.
 */
class AbstractIcon : public QObject, public QGraphicsPathItem
{
  Q_OBJECT

public:
  /**
   * @brief Constructs an AbstractIcon object with the given parameters.
   *
   * @param width The width of the icon.
   * @param color The color of the icon's path.
   * @param pen_width The width of the pen used to draw the icon's path.
   * @param parent The parent QGraphicsItem, if any.
   */
  AbstractIcon(float width, QColor color, float pen_width, QGraphicsItem *parent);

  /**
   * @brief Sets the opacity of the icon's pen.
   *
   * @param new_pen_opacity The new opacity value for the pen (0.0 to 1.0).
   */
  void set_opacity(qreal new_pen_opacity)
  {
    this->pen_opacity = new_pen_opacity;
    this->setOpacity(this->pen_opacity);
  }

  /**
   * @brief Sets the tooltip text for the icon.
   *
   * @param new_tooltip The text to be displayed as the tooltip.
   */
  void set_tooltip(QString new_tooltip) { this->tooltip = new_tooltip; }

Q_SIGNALS:
  /**
   * @brief Signal emitted when the icon is clicked.
   */
  void hit_icon();

protected:
  /**
   * @brief Handles hover enter events.
   *
   * This method is called when the mouse enters the icon's hover area.
   *
   * @param event The hover event data.
   */
  void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

  /**
   * @brief Handles hover leave events.
   *
   * This method is called when the mouse leaves the icon's hover area.
   *
   * @param event The hover event data.
   */
  void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

  /**
   * @brief Handles mouse press events.
   *
   * This method is called when the mouse button is pressed on the icon.
   *
   * @param event The mouse event data.
   */
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

  /**
   * @brief Handles mouse release events.
   *
   * This method is called when the mouse button is released on the icon.
   *
   * @param event The mouse event data.
   */
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

  /**
   * @brief Pure virtual function to define the icon's path.
   *
   * Derived classes must implement this function to set the icon's graphical path.
   */
  virtual void set_path() = 0;

  /**
   * @brief The width of the icon.
   */
  float width;

  /**
   * @brief The color of the icon's path.
   */
  QColor color;

  /**
   * @brief The width of the pen used to draw the icon's path.
   */
  float pen_width;

  /**
   * @brief The opacity of the pen used to draw the icon's path.
   *
   * Default value is 1.0 (fully opaque).
   */
  qreal pen_opacity = 1.f;

  /**
   * @brief The tooltip text for the icon.
   *
   * Default value is "tooltip".
   */
  QString tooltip = "tooltip";
};
} // namespace gngui