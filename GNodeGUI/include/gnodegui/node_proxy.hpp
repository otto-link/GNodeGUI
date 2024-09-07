/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */

/**
 * @file node_proxy.hpp
 * @author Otto Link
 * @brief Defines the NodeProxy struct, an abstract class for handling node
 * representations.
 *
 * This file contains the declaration of the `NodeProxy` struct, which represents an
 * abstract interface for nodes with input and output ports. The class provides methods
 * for accessing the node's ID, label, port count, and port types, making it useful for
 * building node-based systems such as flow diagrams or signal processing pipelines.
 *
 * @date 2024-09-01
 * @copyright Copyright (c) 2024
 */
#pragma once

#include <string>

namespace gngui
{

/**
 * @enum PortType
 * @brief Enum representing the type of a port.
 *
 * This enum defines two possible types for a port:
 * - `IN`: Represents an input port.
 * - `OUT`: Represents an output port.
 */
enum PortType
{
  IN,  ///< Input port type.
  OUT, ///< Output port type.
};

/**
 * @struct NodeProxy
 * @brief Abstract base class representing a proxy for a node with ports.
 *
 * The `NodeProxy` struct is designed to represent a node with input and output ports.
 * It provides an interface for accessing and manipulating node-related information,
 * such as the node ID, labels, port counts, and port types. This is an abstract class,
 * so it cannot be instantiated directly.
 */
struct NodeProxy
{
  /**
   * @brief Deleted default constructor.
   *
   * The default constructor is deleted to ensure that the `NodeProxy`
   * cannot be instantiated without an ID.
   */
  NodeProxy() = delete;

  /**
   * @brief Constructor that initializes the node with a unique ID.
   *
   * @param id A string representing the unique ID of the node.
   */
  NodeProxy(std::string id) : id(id) {}

  /**
   * @brief Virtual destructor for proper cleanup in derived classes.
   */
  virtual ~NodeProxy() = default;

  /**
   * @brief Get the category of the node.
   *
   * This function is pure virtual and must be implemented by derived classes.
   *
   * @return A string representing the category of the node.
   */
  virtual std::string get_category() const = 0;

  /**
   * @brief Get the label of the node.
   *
   * This function is pure virtual and must be implemented by derived classes.
   *
   * @return A string representing the label of the node.
   */
  virtual std::string get_label() const = 0;

  /**
   * @brief Get the number of ports in the node.
   *
   * This function is pure virtual and must be implemented by derived classes.
   *
   * @return An integer representing the number of ports.
   */
  virtual int get_nports() const = 0;

  /**
   * @brief Get the label of a specific port.
   *
   * This function is pure virtual and must be implemented by derived classes.
   *
   * @param port_index The index of the port.
   * @return A string representing the label of the specified port.
   */
  virtual std::string get_port_label(int port_index) const = 0;

  /**
   * @brief Get the type of a specific port (input or output).
   *
   * This function is pure virtual and must be implemented by derived classes.
   *
   * @param port_index The index of the port.
   * @return The type of the port (either `IN` or `OUT`).
   */
  virtual PortType get_port_type(int port_index) const = 0;

  /**
   * @brief Retrieves a reference to the current NodeProxy instance.
   *
   * This method returns a pointer to the current instance of the `NodeProxy`.
   * It can be used to access the proxy object representing the node in a graph or other
   * structure.
   *
   * @return NodeProxy* A pointer to the current `NodeProxy` instance (`this`).
   */
  NodeProxy *get_proxy_ref() { return this; }

  /**
   * @brief Get the ID of the node.
   *
   * @return A string representing the ID of the node.
   */
  std::string get_id() const { return this->id; }

  /**
   * @brief Set the ID of the node.
   *
   * @param new_id A new string representing the ID to assign to the node.
   */
  void set_id(const std::string &new_id) { this->id = new_id; }

private:
  std::string id; ///< The unique ID of the node.
};

} // namespace gngui