#include <QApplication>
#include <QFont>
#include <QPushButton>

#include "gnodegui/graph_editor.hpp"
#include "gnodegui/graphics_group.hpp"
#include "gnodegui/graphics_node.hpp"
#include "gnodegui/logger.hpp"
#include "gnodegui/node_proxy.hpp"
#include "gnodegui/style.hpp"

#include "gnodegui/icons/dots_icon.hpp"
#include "gnodegui/icons/fit_content_icon.hpp"
#include "gnodegui/icons/group_icon.hpp"
#include "gnodegui/icons/lock_icon.hpp"
#include "gnodegui/icons/reload_icon.hpp"
#include "gnodegui/icons/screenshot_icon.hpp"
#include "gnodegui/icons/select_all_icon.hpp"
#include "gnodegui/icons/viewport_icon.hpp"

// --- node specialization

class MyNode : public gngui::NodeProxy
{
public:
  MyNode(std::string id) : gngui::NodeProxy(id) {}

  std::string get_caption() const override { return "NoiseFbm"; }

  std::string get_category() const override { return "Primitive"; }

  std::string get_data_type(int port_index) const override
  {
    std::vector<std::string> vec = {"float", "float", "float", "image", "int"};
    return vec[port_index];
  }

  int get_nports() const override { return 5; }

  std::string get_port_caption(int port_index) const override
  {
    std::vector<std::string> vec = {"in1", "in2", "out1", "in3", "out2"};
    return vec[port_index];
  }

  gngui::PortType get_port_type(int port_index) const override
  {
    std::vector<gngui::PortType> vec = {gngui::PortType::IN,
                                        gngui::PortType::IN,
                                        gngui::PortType::OUT,
                                        gngui::PortType::IN,
                                        gngui::PortType::OUT};
    return vec[port_index];
  }

  QWidget *get_qwidget_ref() override
  {
    // if (!this->push_button)
    //   this->push_button = new QPushButton("button", this);

    return (QWidget *)this->push_button;
  }

  // QSize get_qwidget_size() override { return QSize(256, 32); }

private:
  QWidget *push_button = nullptr;
};

class LongNode : public gngui::NodeProxy
{
public:
  LongNode(std::string id) : gngui::NodeProxy(id) {}

  std::string get_caption() const override { return "Clamp very long caption"; }

  std::string get_category() const override { return "Math/Range/Bound"; }

  std::string get_data_type(int port_index) const override
  {
    std::vector<std::string> vec = {"float", "image", "int", "float"};
    return vec[port_index];
  }

  int get_nports() const override { return 4; }

  std::string get_port_caption(int port_index) const override
  {
    std::vector<std::string> vec = {"in1", "out1", "in2", "output"};
    return vec[port_index];
  }

  gngui::PortType get_port_type(int port_index) const override
  {
    std::vector<gngui::PortType> vec = {gngui::PortType::IN,
                                        gngui::PortType::OUT,
                                        gngui::PortType::IN,
                                        gngui::PortType::OUT};
    return vec[port_index];
  }
};

// --- application

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QFont font("Roboto");
  // QFont font("Aptos");
  // QFont font("Dejavu Sans Mono");
  // QFont font("Source Code Pro");
  font.setPointSize(10);
  app.setFont(font);

  SPDLOG->info("Starting test application...");

  SPDLOG->info("Updating graph parameters...");

  GN_STYLE->node.color_port_data["float"] = QColor(139, 233, 253, 255);
  GN_STYLE->node.color_port_data["int"] = QColor(189, 147, 249, 255);

  GN_STYLE->node.color_category = {{"Math", QColor(0, 43, 54, 255)},
                                   {"Primitive", QColor(42, 161, 152, 255)},
                                   {"Texture", QColor(0, 0, 0, 255)}};

  gngui::GraphEditor ed = gngui::GraphEditor();

  std::map<std::string, std::string> ni = {{"NoiseFbm", "Primitive"},
                                           {"NoiseIq", "Primitive"},
                                           {"Clamp very long caption", "Math/Range"},
                                           {"Smooth", "Filter"}};
  ed.set_node_inventory(ni);

  MyNode   node1("node1");
  LongNode node2("node2");
  MyNode   node3("node3");

  qreal width = 32.f;
  qreal x = 300.f;
  qreal pwidth = 1.f;
  ed.add_item(new gngui::ReloadIcon(width, Qt::white, pwidth),
              QPointF((x += 1.2f * width), 250));
  ed.add_item(new gngui::ScreenshotIcon(width, Qt::white, pwidth),
              QPointF((x += 1.2f * width), 250));
  ed.add_item(new gngui::GroupIcon(width, Qt::white, pwidth),
              QPointF((x += 1.2f * width), 250));
  ed.add_item(new gngui::SelectAllIcon(width, Qt::white, pwidth),
              QPointF((x += 1.2f * width), 250));
  ed.add_item(new gngui::FitContentIcon(width, Qt::white, pwidth),
              QPointF((x += 1.2f * width), 250));
  ed.add_item(new gngui::DotsIcon(width, Qt::white, pwidth),
              QPointF((x += 1.2f * width), 250));
  ed.add_item(new gngui::LockIcon(width, Qt::white, pwidth),
              QPointF((x += 1.2f * width), 250));
  ed.add_item(new gngui::ViewportIcon(width, Qt::white, pwidth),
              QPointF((x += 1.2f * width), 250));

  ed.add_node(node1.get_proxy_ref(), QPointF(300, 300));
  ed.add_node(node2.get_proxy_ref(), QPointF(500, 450));
  ed.add_node(node3.get_proxy_ref(), QPointF(700, 250));

  ed.add_item(new gngui::GraphicsGroup(), QPointF(400, 300));

  ed.resize(1000, 800);
  ed.show();

  return app.exec();
}
