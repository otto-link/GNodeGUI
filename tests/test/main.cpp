#include <iostream>

#include <QApplication>
#include <QFont>
#include <QPushButton>

#include "gnodegui/graph_viewer.hpp"
#include "gnodegui/graphics_group.hpp"
#include "gnodegui/graphics_node.hpp"
#include "gnodegui/logger.hpp"
#include "gnodegui/node_proxy.hpp"
#include "gnodegui/style.hpp"

// --- node specialization

class MyNode
{
public:
  MyNode(std::string id) : id(id) {}

  void do_something() const
  {
    std::cout << "MyNode with id " << id << " does something\n";
  }

  std::string get_caption() const { return "NoiseFbm"; }

  std::string get_category() const { return "Primitive"; }

  std::string get_comment() const { return this->comment; };

  void *get_data_ref(int /*port_index*/) const
  {
    // dummy, does nothing
    return nullptr;
  }

  std::string get_data_type(int port_index) const
  {
    std::vector<std::string> vec = {"float", "float", "float", "image", "int"};
    return vec[port_index];
  }

  std::string get_id() const { return this->id; }

  int get_nports() const { return 5; }

  std::string get_port_caption(int port_index) const
  {
    std::vector<std::string> vec = {"in1", "in2", "out1", "in3", "out2"};
    return vec[port_index];
  }

  gngui::PortType get_port_type(int port_index) const
  {
    std::vector<gngui::PortType> vec = {gngui::PortType::IN,
                                        gngui::PortType::IN,
                                        gngui::PortType::OUT,
                                        gngui::PortType::IN,
                                        gngui::PortType::OUT};
    return vec[port_index];
  }

  std::string get_tool_tip_text() const { return "This is the tool tip for this node"; }

  void set_id(const std::string &new_id) { this->id = new_id; }

  std::string comment;

private:
  std::string id;
};

class MyNodeWidget : public QWidget
{
  Q_OBJECT
public:
  // binding can be done differently, weak_ptr here if shared_ptr are
  // used elsewhere for the model nodes, more convenient for lifetime
  // management
  MyNodeWidget(std::weak_ptr<MyNode> model, QWidget *parent = nullptr)
      : QWidget(parent), model(model)
  {
    // the button affects the model directly (bypassing the
    // GraphicsNode)
    auto *button = new QPushButton("button", this);
    this->connect(button,
                  &QPushButton::pressed,
                  [this]()
                  {
                    if (auto m = this->model.lock())
                      m->do_something();
                  });
  }

  QSize sizeHint() const override { return QSize(128, 32); }

private:
  std::weak_ptr<MyNode> model;
};

class LongNode // node without widget
{
public:
  LongNode(std::string id) : id(id) {}

  std::string get_caption() const { return "Clamp very long caption"; }

  std::string get_category() const { return "Math/Range/Bound"; }

  std::string get_comment() const
  {
    return "The Qt framework contains a comprehensive set of highly intuitive and "
           "modularized C++ library classes and is loaded with APIs to simplify your "
           "application development. Qt produces highly readable, easily maintainable "
           "and reusable code with high runtime performance and small footprint - and "
           "it's cross-platform.";
  }

  void *get_data_ref(int /*port_index*/) const
  {
    // dummy, does nothing
    return nullptr;
  }

  std::string get_data_type(int port_index) const
  {
    std::vector<std::string> vec = {"float", "image", "int", "float"};
    return vec[port_index];
  }

  std::string get_id() const { return this->id; }

  int get_nports() const { return 4; }

  std::string get_port_caption(int port_index) const
  {
    std::vector<std::string> vec = {"in1", "out1", "in2", "output"};
    return vec[port_index];
  }

  gngui::PortType get_port_type(int port_index) const
  {
    std::vector<gngui::PortType> vec = {gngui::PortType::IN,
                                        gngui::PortType::OUT,
                                        gngui::PortType::IN,
                                        gngui::PortType::OUT};
    return vec[port_index];
  }

  std::string get_tool_tip_text() const { return "This is the tool tip for this node"; }

  void set_id(const std::string &new_id) { this->id = new_id; }

private:
  std::string id;
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

  gngui::Logger::log()->info("Starting test application...");

  gngui::Logger::log()->info("Updating graph parameters...");

  // (optional)
  {
    GN_STYLE->node.color_port_data["float"] = QColor(139, 233, 253, 255);
    GN_STYLE->node.color_port_data["int"] = QColor(189, 147, 249, 255);
  }

  // (optional)
  {
    GN_STYLE->node.color_category = {{"Math", QColor(0, 43, 54, 255)},
                                     {"Primitive", QColor(42, 161, 152, 255)},
                                     {"Texture", QColor(0, 0, 0, 255)}};
  }

  gngui::GraphViewer ed = gngui::GraphViewer();

  std::map<std::string, std::string> ni = {{"NoiseFbm", "Primitive"},
                                           {"NoiseIq", "Primitive"},
                                           {"Clamp very long caption", "Math/Range"},
                                           {"Smooth", "Filter"}};
  ed.set_node_inventory(ni);

  auto  node1 = std::make_shared<MyNode>("node1");
  auto *node1_proxy = new gngui::TypedNodeProxy<MyNode>(node1); // owned by GraphicsNode
  auto *node1_widget = new MyNodeWidget(node1);                 // idem

  auto  node2 = std::make_shared<LongNode>("node2");
  auto *node2_proxy = new gngui::TypedNodeProxy<LongNode>(node2);

  auto  node3 = std::make_shared<MyNode>("node3");
  auto *node3_proxy = new gngui::TypedNodeProxy<MyNode>(node3);
  auto *node3_widget = new MyNodeWidget(node3);

  std::string id1 = ed.add_node(QPointer(node1_proxy), QPointF(100, 300));
  ed.get_graphics_node_by_id(id1)->set_widget(node1_widget, QSize(256, 256));

  std::string id2 = ed.add_node(QPointer(node2_proxy), QPointF(500, 450));

  std::string id3 = ed.add_node(QPointer(node3_proxy), QPointF(700, 250));
  ed.get_graphics_node_by_id(id3)->set_widget(node3_widget); //, QSize(256, 256));

  ed.add_link(id1, "out1", id2, "in1");

  ed.add_item(new gngui::GraphicsGroup(), QPointF(400, 300));

  // node1.reset();
  // gngui::Logger::log()->info("{}", node1.use_count());

  ed.zoom_to_content();
  ed.resize(1000, 800);
  ed.show();

  // ed.json_to();

  return app.exec();
}

#include "main.moc" // <--- REQUIRED for moc on main.cpp
