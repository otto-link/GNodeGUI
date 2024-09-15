/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */

#include "gnodegui/node_proxy.hpp"
#include "gnodegui/logger.hpp"

namespace gngui
{

void NodeProxy::log_debug()
{
  GUILOG->trace("NodeProxy::log_debug, node {}({})", this->get_caption(), this->get_id());
  GUILOG->trace("category: {}", this->get_category());
  GUILOG->trace("nports: {}", this->get_nports());

  for (int k = 0; k < this->get_nports(); k++)
  {
    GUILOG->trace("- port #: {}", k);
    GUILOG->trace("  - caption: {}", this->get_port_caption(k));
    GUILOG->trace("  - id: {}", this->get_port_id(k));
    GUILOG->trace("  - type: {}", this->get_port_type(k));
    GUILOG->trace("  - data_type: {}", this->get_data_type(k));
  }
}

} // namespace gngui
