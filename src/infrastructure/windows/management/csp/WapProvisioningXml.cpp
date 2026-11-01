// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/management/csp/WapProvisioningXml.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <limits>

namespace uwf::windows::csp {
namespace {
constexpr std::size_t kMaximumXmlNesting = 32;

std::optional<std::uint32_t> providerCode(const QString& text) {
  bool valid = false;
  if (text.startsWith(u'-')) {
    const auto number = text.toLongLong(&valid, 10);
    if (valid && number >= std::numeric_limits<std::int32_t>::min() && number < 0) return static_cast<std::uint32_t>(number);
  } else {
    const auto number = text.toULongLong(&valid, text.startsWith(u"0x", Qt::CaseInsensitive) ? 16 : 10);
    if (valid && number > 0 && number <= std::numeric_limits<std::uint32_t>::max()) return static_cast<std::uint32_t>(number);
  }
  return {};
}

const WapNode& uniqueChild(const WapNode& parent, QStringView element, const QString& name) {
  const WapNode* found = nullptr;
  for (const auto& child : parent.children) {
    if (child.element != element || child.name != name) continue;
    if (found) throw WapProtocolError("Duplicate node in WAP response");
    found = &child;
  }
  if (!found) throw WapProtocolError("Requested node is missing from WAP response");
  return *found;
}
}  // namespace

QString encodeWapCommand(const WapCommand& command) {
  if (command.name.isEmpty()) throw WapProtocolError("WAP command requires a node name");
  QString xml;
  QXmlStreamWriter writer(&xml);
  writer.writeStartElement(u"wap-provisioningdoc");
  for (const auto& parent : command.parents) {
    writer.writeStartElement(u"characteristic");
    writer.writeAttribute(u"type", parent);
  }
  switch (command.action) {
    case WapAction::GetValue:
      writer.writeStartElement(u"parm-query");
      writer.writeAttribute(u"name", command.name);
      break;
    case WapAction::GetChildren:
      writer.writeStartElement(u"characteristic-query");
      writer.writeAttribute(u"type", command.name);
      writer.writeAttribute(u"recursive", u"false");
      break;
    case WapAction::PutValue:
      if (!command.value) throw WapProtocolError("WAP value command requires a value");
      writer.writeStartElement(u"parm");
      writer.writeAttribute(u"name", command.name);
      writer.writeAttribute(u"datatype", command.value->datatype);
      writer.writeAttribute(u"value", command.value->text);
      break;
    case WapAction::AddCharacteristic:
      writer.writeStartElement(u"characteristic");
      writer.writeAttribute(u"type", command.name);
      break;
    case WapAction::RemoveValue:
      writer.writeStartElement(u"noparm");
      writer.writeAttribute(u"name", command.name);
      break;
    case WapAction::RemoveCharacteristic:
      writer.writeStartElement(u"nocharacteristic");
      writer.writeAttribute(u"type", command.name);
      break;
  }
  writer.writeEndElement();
  for (qsizetype i = 0; i < command.parents.size(); ++i) writer.writeEndElement();
  writer.writeEndElement();
  if (writer.hasError()) throw WapEncodingError("The WAP command contains text that cannot be represented in XML");
  return xml;
}

WapDocument decodeWapDocument(const QString& xml) {
  if (xml.isEmpty()) throw WapProtocolError("Missing WAP response");
  QXmlStreamReader reader(xml);
  WapDocument document;
  std::vector<WapNode*> stack;
  bool hasRoot = false;
  while (!reader.atEnd()) {
    const auto token = reader.readNext();
    if (token == QXmlStreamReader::DTD || token == QXmlStreamReader::EntityReference)
      throw WapProtocolError("DTD and entities are not permitted in WAP responses");
    if (reader.isStartElement()) {
      if (!reader.namespaceUri().isEmpty()) throw WapProtocolError("Unexpected WAP XML namespace");
      if (stack.size() >= kMaximumXmlNesting) throw WapProtocolError("WAP response exceeds the XML nesting limit");
      const auto element = reader.name().toString();
      if (stack.empty()) {
        if (hasRoot || element != u"wap-provisioningdoc") throw WapProtocolError("Expected a single WAP provisioning document");
        hasRoot = true;
        document.root.element = element;
        stack.push_back(&document.root);
        continue;
      }
      const auto attributes = reader.attributes();
      const bool characteristic = element == u"characteristic" || element == u"nocharacteristic" || element == u"characteristic-error";
      const bool parameter = element == u"parm" || element == u"noparm" || element == u"parm-error";
      if (!characteristic && !parameter && element != u"error") throw WapProtocolError("Unexpected element in WAP response");
      if (stack.back()->element != u"characteristic" && stack.back()->element != u"wap-provisioningdoc") throw WapProtocolError("Unexpected nested WAP value");
      WapNode node{element, attributes.value(characteristic ? u"type" : u"name").toString(), {}, {}};
      const bool error = element == u"parm-error" || element == u"characteristic-error" || element == u"error";
      if (node.name.isEmpty() && !error) throw WapProtocolError("WAP node has no name");
      if (element == u"parm") {
        if (!attributes.hasAttribute(u"value")) throw WapProtocolError("WAP parameter has no value");
        node.value = WapValue{attributes.value(u"datatype").toString(), attributes.value(u"value").toString()};
      }
      if (error)
        document.errors.push_back({providerCode(attributes.value(u"hresult").toString()), element, node.name, attributes.value(u"description").toString()});
      stack.back()->children.push_back(std::move(node));
      stack.push_back(&stack.back()->children.back());
    } else if (reader.isEndElement()) {
      if (stack.empty()) throw WapProtocolError("Unbalanced WAP XML response");
      stack.pop_back();
    } else if (reader.isCharacters() && !reader.isWhitespace()) {
      throw WapProtocolError("Unexpected text in WAP response");
    }
  }
  if (reader.hasError() || !hasRoot || !stack.empty()) throw WapProtocolError("Malformed WAP XML response");
  return document;
}

const WapNode& wapCharacteristic(const WapDocument& document, const QStringList& path) {
  const WapNode* node = &document.root;
  for (const auto& name : path) node = &uniqueChild(*node, u"characteristic", name);
  return *node;
}

const WapValue& wapParameter(const WapNode& characteristic, const QString& name) {
  const auto& node = uniqueChild(characteristic, u"parm", name);
  if (!node.value) throw WapProtocolError("WAP parameter has no value");
  return *node.value;
}
}  // namespace uwf::windows::csp
