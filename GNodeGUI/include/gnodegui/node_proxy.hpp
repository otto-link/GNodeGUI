/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */

/* This file contains the declaration of the `NodeProxy` struct, which represents an
 * abstract interface for nodes with input and output ports. The class provides methods
 * for accessing the node's ID, label, port count, and port types, making it useful for
 * building node-based systems such as flow diagrams or signal processing pipelines.
 */
#pragma once
#include <QWidget>

#include <string>

namespace gngui
{

enum PortType
{
  IN,  ///< Input port type.
  OUT, ///< Output port type.
};

/**
 * The `NodeProxy` struct is designed to represent a node with input and output ports.
 * It provides an interface for accessing and manipulating node-related information,
 * such as the node ID, labels, port counts, and port types. This is an abstract class,
 * so it cannot be instantiated directly.
 */
class NodeProxy
{
public:
  NodeProxy() = default;
  NodeProxy(std::string id) : id(id) {}
  virtual ~NodeProxy() = default;

  // --- ID / Identity
  virtual std::string get_id() const { return this->id; }
  virtual void        set_id(const std::string &new_id) { this->id = new_id; }
  NodeProxy          *get_proxy_ref() { return this; }

  // --- Node description
  virtual std::string get_caption() const = 0;
  virtual std::string get_category() const = 0;
  virtual std::string get_tool_tip_text() { return std::string(); }

  // --- Ports
  virtual int         get_nports() const = 0;
  virtual std::string get_port_caption(int port_index) const = 0;
  virtual std::string get_port_id(int port_index) const;
  virtual PortType    get_port_type(int port_index) const = 0;
  virtual std::string get_data_type(int port_index) const = 0;
  virtual void       *get_data_ref(int /*port_index*/) { return nullptr; }

  // --- Embedded widget
  virtual QWidget *get_qwidget_ref() { return nullptr; }
  virtual QSize    get_qwidget_size() { return QSize(-1, -1); }

  // --- Debugging
  void log_debug();

private:
  std::string id;
};

} // namespace gngui
