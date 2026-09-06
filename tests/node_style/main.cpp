#include <iostream>

#include <QApplication>
#include <QImage>
#include <QPainter>

#include "gnodegui/graphics_node.hpp"
#include "gnodegui/style.hpp"

// Windows headers pulled in by logging may define the annotation macro IN.
#ifdef IN
#undef IN
#endif

class Proxy : public gngui::NodeProxy
{
public:
  std::string get_id() const override { return "test"; }
  void set_id(const std::string &) override {}
  std::string get_caption() const override { return "Node"; }
  std::string get_category() const override { return {}; }
  std::string get_tool_tip_text() const override { return {}; }
  int get_nports() const override { return 1; }
  std::string get_port_caption(int) const override { return "Input"; }
  gngui::PortType get_port_type(int) const override { return gngui::IN; }
  std::string get_data_type(int) const override { return {}; }
  void *get_data_ref(int) const override { return nullptr; }
};

class Node : public gngui::GraphicsNode
{
public:
  using GraphicsNode::GraphicsNode;
  using GraphicsNode::paint;
};

QImage render(Node &node)
{
  QImage image(256, 256, QImage::Format_ARGB32_Premultiplied);
  image.fill(Qt::transparent);
  QPainter painter(&image);
  node.paint(&painter, nullptr, nullptr);
  return image;
}

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  Proxy proxy;
  Node node(&proxy);
  auto &style = GN_STYLE->node;
  const auto require = [](bool condition, const char *message)
  {
    if (!condition) std::cerr << message << '\n';
    return condition;
  };

  if (!require(style.bevel_width == 0.f &&
                   style.color_port_caption == QColor(Qt::white) &&
                   style.rounding_radius == 8.f,
               "Legacy defaults changed")) return 1;

  const QImage flat = render(node);
  style.color_bevel_top = QColor(240, 20, 30);
  style.color_bevel_bottom = QColor(20, 30, 240);
  if (!require(render(node) == flat, "Disabled bevel changed pixels")) return 1;

  style.bevel_width = 1.f;
  const QImage beveled = render(node);
  const QRectF body = node.get_geometry().body_rect;
  int top_pixels = 0;
  int bottom_pixels = 0;
  for (int y = 0; y < flat.height(); ++y)
    for (int x = 0; x < flat.width(); ++x)
    {
      if (flat.pixel(x, y) == beveled.pixel(x, y)) continue;
      if (!require(body.contains(QPointF(x + 0.5, y + 0.5)),
                   "Bevel escaped card bounds")) return 1;
      const QColor color = beveled.pixelColor(x, y);
      top_pixels += color == style.color_bevel_top;
      bottom_pixels += color == style.color_bevel_bottom;
    }
  if (!require(top_pixels > 0 && bottom_pixels > 0,
               "Both bevel colours must render")) return 1;

  node.setSelected(true);
  const QImage selected_bevel = render(node);
  style.bevel_width = 0.f;
  const QImage selected_flat = render(node);
  for (int y = 0; y < flat.height(); ++y)
    for (int x = 0; x < flat.width(); ++x)
      if (selected_flat.pixelColor(x, y) == style.color_selected &&
          !require(selected_bevel.pixel(x, y) == selected_flat.pixel(x, y),
                   "Bevel obscured selection")) return 1;

  node.setSelected(false);
  style.color_port_caption = QColor(30, 220, 60);
  const QImage ink = render(node);
  if (!require(ink != flat, "Port caption ink was ignored")) return 1;
  const QRect label = node.get_geometry().port_label_rects[0].toAlignedRect();
  for (int y = 0; y < flat.height(); ++y)
    for (int x = 0; x < flat.width(); ++x)
      if (!label.contains(x, y) &&
          !require(ink.pixel(x, y) == flat.pixel(x, y),
                   "Port caption ink changed unrelated pixels")) return 1;

  std::cout << "Node style rendering checks passed\n";
  return 0;
}
