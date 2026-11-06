// qpwgraph_node.cpp
//
/****************************************************************************
   Copyright (C) 2021-2026, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#include "qpwgraph_node.h"
#include "qpwgraph_connect.h"

#include <QGraphicsScene>
#include <QCoreApplication>

#include <QStyleOptionGraphicsItem>

#include <QPainter>
#include <QPalette>

#include <QLinearGradient>

#include <QGraphicsDropShadowEffect>

#include <algorithm>


//----------------------------------------------------------------------------
// qpwgraph_node -- Node graphics item.

// Constructor.
qpwgraph_node::qpwgraph_node (
	uint id, const QString& name, qpwgraph_item::Mode mode, uint type )
	: qpwgraph_item(nullptr),
		m_id(id), m_name(name), m_mode(mode), m_type(type),
		m_num(0), m_name_ex(false), m_label_ex(false),
		m_input_collapsed(false), m_output_collapsed(false)
{
	QGraphicsPathItem::setZValue(0.0);

	const QPalette pal;
	const int base_value = pal.base().color().value();
	const bool is_dark = (base_value < 128);

	const QColor& text_color = pal.text().color();
	QColor foreground_color(is_dark
		? text_color.darker()
		: text_color);
	qpwgraph_item::setForeground(foreground_color);

	const QColor& window_color = pal.window().color();
	QColor background_color(is_dark
		? window_color.lighter()
		: window_color);
	background_color.setAlpha(160);
	qpwgraph_item::setBackground(background_color);

	m_pixmap = new QGraphicsPixmapItem(this);
	m_text = new QGraphicsTextItem(this);

	QGraphicsPathItem::setFlag(QGraphicsItem::ItemIsMovable);
	QGraphicsPathItem::setFlag(QGraphicsItem::ItemIsSelectable);

	setNodeTitle(QString());

	const bool is_darkest = (base_value < 24);
	QColor shadow_color = (is_darkest ? Qt::white : Qt::black);
	shadow_color.setAlpha(180);

	QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect();
	effect->setColor(shadow_color);
	effect->setBlurRadius(is_darkest ? 8 : 16);
	effect->setOffset(is_darkest ? 0 : 2);
	QGraphicsPathItem::setGraphicsEffect(effect);

	qpwgraph_item::raise();
}


// Destructor.
qpwgraph_node::~qpwgraph_node (void)
{
	removePorts();

	// No actual need to destroy any children here...
	//
	//	QGraphicsPathItem::setGraphicsEffect(nullptr);

	//	delete m_text;
	//	delete m_pixmap;
}


// accessors.
uint qpwgraph_node::nodeId (void) const
{
	return m_id;
}

void qpwgraph_node::setNodeName ( const QString& name )
{
	m_name = name;

	QGraphicsPathItem::setToolTip(nodeNameLabelEx());
}


QString qpwgraph_node::nodeName (void) const
{
	QString name = m_name;

	if (m_num > 0) {
		name += '-';
		name += QString::number(m_num);
	}

	return name;
}


void qpwgraph_node::setNodeMode ( qpwgraph_item::Mode mode )
{
	m_mode = mode;
}


qpwgraph_item::Mode qpwgraph_node::nodeMode (void) const
{
	return m_mode;
}


void qpwgraph_node::setNodeType ( uint type )
{
	m_type = type;
}


uint qpwgraph_node::nodeType (void) const
{
	return m_type;
}


void qpwgraph_node::setNodeIcon ( const QIcon& icon )
{
	m_icon = icon;

	m_pixmap->setPixmap(m_icon.pixmap(24, 24));
}


const QIcon& qpwgraph_node::nodeIcon (void) const
{
	return m_icon;
}


void qpwgraph_node::setNodeNum ( uint num )
{
	m_num = num;

	setNodeTitle(QString()); // reset title.
}


uint qpwgraph_node::nodeNum (void) const
{
	return m_num;
}


void qpwgraph_node::setNodeLabel ( const QString& label )
{
	m_label = label;

	setNodeTitle(QString()); // reset title.
}


const QString& qpwgraph_node::nodeLabel (void) const
{
	return m_label;
}


QString qpwgraph_node::nodeNameLabel (void) const
{
	QString name_label = nodeName();

	if (!m_label.isEmpty()) {
		name_label += ' ';
		name_label += '[';
		name_label += m_label;
		name_label += ']';
	}

	return name_label;
}


void qpwgraph_node::setNodeTitle ( const QString& title )
{
	const QString& name_label = nodeNameLabel();
	const QFont& font = m_text->font();
	m_text->setFont(QFont(font.family(), font.pointSize(), QFont::Bold));
	m_title = (title.isEmpty() ? name_label : title);

	static const int MAX_TITLE_LENGTH = 29;
	static const QString ellipsis(3, '.');

	QString text = m_title.simplified();
	if (text.length() >= MAX_TITLE_LENGTH  + ellipsis.length())
		text = text.left(MAX_TITLE_LENGTH).trimmed() + ellipsis;

	m_text->setPlainText(text);

	QGraphicsPathItem::setToolTip(nodeNameLabelEx());
}


const QString& qpwgraph_node::nodeTitle (void) const
{
	return m_title;
}


void qpwgraph_node::setNodePrefix ( const QString& prefix )
{
	m_prefix = prefix;
}


const QString& qpwgraph_node::nodePrefix (void) const
{
	return m_prefix;
}


void qpwgraph_node::setNodeNameEx ( bool name_ex )
{
	m_name_ex = name_ex;
}

bool qpwgraph_node::isNodeNameEx (void) const
{
	return m_name_ex;
}


QString qpwgraph_node::nodeNameEx (void) const
{
	return (m_name_ex ? m_name : nodeName());
}


void qpwgraph_node::setNodeLabelEx ( bool label_ex )
{
	m_label_ex = label_ex;

	setNodeTitle(QString()); // reset title.
}


bool qpwgraph_node::isNodeLabelEx (void) const
{
	return m_label_ex;
}


QString qpwgraph_node::nodeNameLabelEx (void) const
{
	QString name_label;

	if (m_label_ex) {
		name_label += QString::number(nodeId());
		name_label += ':';
	}

	return name_label + nodeNameLabel();
}


// Port-list methods.
qpwgraph_port *qpwgraph_node::addPort (
	uint id, const QString& name, qpwgraph_item::Mode mode, int type )
{
	qpwgraph_port *port = new qpwgraph_port(this, id, name, mode, type);

	m_ports.append(port);
	m_port_ids.insert(qpwgraph_port::PortIdKey(port), port);
	m_port_names.insert(qpwgraph_port::PortNameKey(port), port);

	updatePath();

	return port;
}


qpwgraph_port *qpwgraph_node::addInputPort (
	uint id, const QString& name, int type )
{
	return addPort(id, name, qpwgraph_item::Input, type);
}


qpwgraph_port *qpwgraph_node::addOutputPort (
	uint id, const QString& name, int type )
{
	return addPort(id, name, qpwgraph_item::Output, type);
}


void qpwgraph_node::removePort ( qpwgraph_port *port )
{
	m_port_names.remove(qpwgraph_port::PortNameKey(port));
	m_port_ids.remove(qpwgraph_port::PortIdKey(port));
	m_ports.removeAll(port);

	updatePath();
}


void qpwgraph_node::removePorts (void)
{
	foreach (qpwgraph_port *port, m_ports)
		port->removeConnects();

	// Do not delete ports here as they are node's child items...
	//
	//qDeleteAll(m_ports);
	m_ports.clear();
	m_port_ids.clear();
	m_port_names.clear();
}


// Port finder (by id/name, mode and type)
qpwgraph_port *qpwgraph_node::findPort (
	uint id, qpwgraph_item::Mode mode, uint type )
{
	return m_port_ids.value(qpwgraph_port::PortIdKey(id, mode, type), nullptr);
}


qpwgraph_port *qpwgraph_node::findPort (
	const QString& name, qpwgraph_item::Mode mode, uint type )
{
	return m_port_names.value(qpwgraph_port::PortNameKey(name, mode, type), nullptr);
}


// Port-list accessor.
const QList<qpwgraph_port *>& qpwgraph_node::ports (void) const
{
	return m_ports;
}


// Reset port markings, destroy if unmarked.
void qpwgraph_node::resetPorts (void)
{
	QList<qpwgraph_port *> ports;

	foreach (qpwgraph_port *port, m_ports) {
		if (port->isMarked()) {
			port->setMarked(false);
		} else {
			ports.append(port);
		}
	}

	foreach (qpwgraph_port *port, ports) {
		port->removeConnects();
		removePort(port);
		delete port;
	}
}


// Input/output port groups.
void qpwgraph_node::setPortGroupCollapsed ( Mode mode, bool collapsed )
{
	if (mode == Input && m_input_collapsed != collapsed) {
		m_input_collapsed = collapsed;
		updatePath();
	}
	else
	if (mode == Output && m_output_collapsed != collapsed) {
		m_output_collapsed = collapsed;
		updatePath();
	}
}


bool qpwgraph_node::isPortGroupCollapsed ( Mode mode ) const
{
	if (mode == Input)
		return m_input_collapsed;
	if (mode == Output)
		return m_output_collapsed;
	return false;
}


QRectF qpwgraph_node::portGroupHeaderRect ( Mode mode ) const
{
	const qreal width = itemRect().width();
	const qreal height = m_text->boundingRect().height() + 2;
	const qreal half = width / 2.0;
	if (mode == Input)
		return QRectF(6.0, height, half - 8.0, 18.0);
	return QRectF(half + 2.0, height, half - 8.0, 18.0);
}


qpwgraph_item::Mode qpwgraph_node::portGroupAt ( const QPointF& scene_pos ) const
{
	const QPointF pos = mapFromScene(scene_pos);
	if (portGroupHeaderRect(Input).contains(pos)) {
		foreach (qpwgraph_port *port, m_ports) {
			if (port->isInput())
				return Input;
		}
	}
	if (portGroupHeaderRect(Output).contains(pos)) {
		foreach (qpwgraph_port *port, m_ports) {
			if (port->isOutput())
				return Output;
		}
	}
	return None;
}


QPointF qpwgraph_node::portGroupPos ( Mode mode ) const
{
	const QRectF header = portGroupHeaderRect(mode);
	const qreal x = (mode == Output ? itemRect().width() : 0.0);
	return mapToScene(QPointF(x, header.center().y()));
}


// Path/shape updater.
void qpwgraph_node::updatePath (void)
{
	const QRectF& title_rect = m_text->boundingRect();
	int width = title_rect.width() / 2 + 24;
	int wi = width;
	int wo = width;
	QList<qpwgraph_port *> inputs;
	QList<qpwgraph_port *> outputs;
	foreach (qpwgraph_port *port, m_ports) {
		const int port_width = port->itemRect().width();
		if (port->isOutput()) {
			wo = qMax(wo, port_width);
			outputs.append(port);
		} else {
			wi = qMax(wi, port_width);
			inputs.append(port);
		}
	}
	width = qMax(156, 4 * ((wi + wo) / 4));
	std::sort(inputs.begin(), inputs.end(), qpwgraph_port::Compare());
	std::sort(outputs.begin(), outputs.end(), qpwgraph_port::Compare());

	const qreal group_top = title_rect.height() + 2.0;
	const qreal group_header_height = 18.0;
	qreal input_bottom = group_top;
	qreal output_bottom = group_top;

	if (!inputs.isEmpty()) {
		input_bottom += group_header_height;
		if (m_input_collapsed) {
			foreach (qpwgraph_port *port, inputs)
				port->setVisible(false);
		} else {
			int type = 0;
			for (qpwgraph_port *port : inputs) {
				port->setVisible(true);
				if (type - int(port->portType())) {
					type = int(port->portType());
					input_bottom += 2.0;
				}
				const QRectF& port_rect = port->itemRect();
				port->setPos(-6.0, input_bottom);
				input_bottom += port_rect.height() + 1.0;
			}
		}
	}

	if (!outputs.isEmpty()) {
		output_bottom += group_header_height;
		if (m_output_collapsed) {
			foreach (qpwgraph_port *port, outputs)
				port->setVisible(false);
		} else {
			int type = 0;
			for (qpwgraph_port *port : outputs) {
				port->setVisible(true);
				if (type - int(port->portType())) {
					type = int(port->portType());
					output_bottom += 2.0;
				}
				const QRectF& port_rect = port->itemRect();
				port->setPos(width + 6.0 - port_rect.width(), output_bottom);
				output_bottom += port_rect.height() + 1.0;
			}
		}
	}

	const qreal height = qMax(input_bottom, output_bottom) + 6.0;
	QPainterPath path;
	path.addRoundedRect(0, 0, width, height, 5, 5);
	/*QGraphicsPathItem::*/setPath(path);

	foreach (qpwgraph_port *port, m_ports) {
		foreach (qpwgraph_connect *connect, port->connects()) {
			if (connect && connect->port1() && connect->port2())
				connect->updatePath();
		}
	}
}


void qpwgraph_node::paint ( QPainter *painter,
	const QStyleOptionGraphicsItem *option, QWidget */*widget*/ )
{
	const QPalette& pal = option->palette;

	const QRectF& node_rect = itemRect();
	QLinearGradient node_grad(0, node_rect.top(), 0, node_rect.bottom());
	QColor node_color;
	if (QGraphicsPathItem::isSelected()) {
		const QColor& hilitetext_color = pal.highlightedText().color();
		m_text->setDefaultTextColor(hilitetext_color);
		painter->setPen(hilitetext_color);
		node_color = pal.highlight().color();
	} else {
		const QColor& foreground
			= qpwgraph_item::foreground();
		const QColor& background
			= qpwgraph_item::background();
		const bool is_dark
			= (background.value() < 192);
		m_text->setDefaultTextColor(is_dark
			? foreground.lighter()
			: foreground.darker());
		painter->setPen(foreground);
		node_color = background;
	}
	node_color.setAlpha(180);
	node_grad.setColorAt(0.6, node_color);
	node_grad.setColorAt(1.0, node_color.darker(120));
	painter->setBrush(node_grad);

	painter->drawPath(QGraphicsPathItem::path());

	QColor group_color = painter->pen().color();
	group_color.setAlpha(210);
	painter->setPen(group_color);
	QFont group_font = painter->font();
	group_font.setPointSizeF(qMax(7.0, group_font.pointSizeF() - 1.0));
	painter->setFont(group_font);
	const qreal group_y = m_text->boundingRect().height() + 2.0;
	int input_count = 0;
	int output_count = 0;
	foreach (qpwgraph_port *port, m_ports) {
		if (port->isInput()) ++input_count;
		if (port->isOutput()) ++output_count;
	}
	if (input_count > 0 || output_count > 0) {
		painter->setPen(QPen(group_color, 0));
		painter->drawLine(QPointF(6.0, group_y + 18.0),
			QPointF(node_rect.width() - 6.0, group_y + 18.0));
		if (input_count > 0) {
			const QString marker = (m_input_collapsed ? QStringLiteral("▸ ")
				: QStringLiteral("▾ "));
			painter->drawText(portGroupHeaderRect(Input),
				Qt::AlignVCenter | Qt::AlignLeft,
				marker + QCoreApplication::translate("qpwgraph_node", "Inputs")
				+ QStringLiteral(" · ")
				+ QString::number(input_count));
		}
		if (output_count > 0) {
			const QString marker = (m_output_collapsed ? QStringLiteral("▸ ")
				: QStringLiteral("▾ "));
			painter->drawText(portGroupHeaderRect(Output),
				Qt::AlignVCenter | Qt::AlignLeft,
				marker + QCoreApplication::translate("qpwgraph_node", "Outputs")
				+ QStringLiteral(" · ")
				+ QString::number(output_count));
		}
	}

	m_pixmap->setPos(node_rect.x() + 4, node_rect.y() + 4);

	const QRectF& text_rect = m_text->boundingRect();
	const qreal w2 = (node_rect.width() - text_rect.width()) / 2;
	m_text->setPos(node_rect.x() + w2 + 4, node_rect.y() + 2);
}


QVariant qpwgraph_node::itemChange (
	GraphicsItemChange change, const QVariant& value )
{
	if (change == QGraphicsItem::ItemSelectedHasChanged) {
		const bool is_selected = value.toBool();
		foreach (qpwgraph_port *port, m_ports)
			port->setSelected(is_selected);
	}

	return value;
}


// Rectangular editor extents.
QRectF qpwgraph_node::editorRect (void) const
{
	return m_text->sceneBoundingRect();
}


// end of qpwgraph_node.cpp
