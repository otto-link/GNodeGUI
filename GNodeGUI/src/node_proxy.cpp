/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */

#include "gnodegui/node_proxy.hpp"
#include "gnodegui/logger.hpp"

namespace gngui
{

void NodeProxy::log_debug()
{
  Logger::log()->trace("NodeProxy::log_debug, node {}({})",
                       this->get_caption(),
                       this->get_id());
  Logger::log()->trace("category: {}", this->get_category());
  Logger::log()->trace("nports: {}", this->get_nports());

  for (int k = 0; k < this->get_nports(); k++)
  {
    Logger::log()->trace("- port #: {}", k);
    Logger::log()->trace("  - caption: {}", this->get_port_caption(k));
    Logger::log()->trace("  - id: {}", this->get_port_id(k));
    Logger::log()->trace("  - type: {}", this->get_port_type(k));
    Logger::log()->trace("  - data_type: {}", this->get_data_type(k));
  }
}

} // namespace gngui
