#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QLineEdit>

#include <nodes/NodeDataModel>

using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeDataModel;

/// The class can potentially incapsulate any user data which
/// need to be transferred within the Node Editor graph
class TextData : public NodeData
{
public:

  TextData() {}

  TextData(QString const &text)
    : _text(text)
  {}

  NodeDataType type() const override
  {
    return NodeDataType{ "text", "Text" };
  }

  QString text() const { return _text; }

private:

  QString _text;
};
/// The model dictates the number of inputs and outputs for the Node.
/// In this example it has no logic.
class TextSourceDataModel : public NodeDataModel
{
  Q_OBJECT

public:
  TextSourceDataModel();

  virtual
    ~TextSourceDataModel() {}

public:

  QString
    caption() const override
  {
    return QString("Text Source");
  }

  bool
    captionVisible() const override { return false; }

  QString
    name() const override
  {
    return QString("TextSourceDataModel");
  }

  std::unique_ptr<NodeDataModel>
    clone() const override
  {
    return std::make_unique<TextSourceDataModel>();
  }

public:

  unsigned int
    nPorts(PortType portType) const override;

  NodeDataType
    dataType(PortType portType, PortIndex portIndex) const override;

  std::shared_ptr<NodeData>
    outData(PortIndex port) override;

  void
    setInData(std::shared_ptr<NodeData>, int) override
  { }

  QWidget *
    embeddedWidget() override { return _lineEdit; }

  private slots:

  void
    onTextEdited(QString const &string);

private:

  QLineEdit * _lineEdit;
};
