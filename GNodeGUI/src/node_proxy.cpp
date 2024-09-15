/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */

#include "gnodegui/node_proxy.hpp"
#include "gnodegui/logger.hpp"

namespace gngui
{

void NodeProxy::log_debug()
{
  GLOG->trace("NodeProxy::log_debug, node {}({})", this->get_caption(), this->get_id());
  GLOG->trace("category: {}", this->get_category());
  GLOG->trace("nports: {}", this->get_nports());

  for (int k = 0; k < this->get_nports(); k++)
  {
    GLOG->trace("- port #: {}", k);
    GLOG->trace("  - caption: {}", this->get_port_caption(k));
    GLOG->trace("  - id: {}", this->get_port_id(k));
    GLOG->trace("  - type: {}", this->get_port_type(k));
    GLOG->trace("  - data_type: {}", this->get_data_type(k));
  }
}

} // namespace gngui
