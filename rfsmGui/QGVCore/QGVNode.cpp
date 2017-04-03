/***************************************************************
QGVCore
Copyright (c) 2014, Bergont Nicolas, All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.
***************************************************************/
#include <QGVNode.h>
#include <QGVCore.h>
#include <QGVScene.h>
#include <QGVGraphPrivate.h>
#include <QGVNodePrivate.h>
#include <QDebug>
#include <QPainter>

QGVNode::QGVNode(QGVNodePrivate *node, QGVScene *scene): _node(node), _scene(scene), vertex(NULL)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    activeMode = false;
}

QGVNode::~QGVNode()
{
    _scene->removeItem(this);
		delete _node;
}

void QGVNode::setActive(bool activeMode) {
    QGVNode::activeMode = activeMode;
}

void QGVNode::setError(const std::string &errorMessage) {
    QGVNode::errorMessage = errorMessage;
    QString toolTip = QString("<font color=darkred>");
    toolTip += errorMessage.c_str();
    toolTip += QString("</font>");
    setToolTip(QString(toolTip));
}

QString QGVNode::label() const
{
    return getAttribute("label");
}

void QGVNode::setLabel(const QString &label)
{
    setAttribute("label", label);
}

QRectF QGVNode::boundingRect() const
{
    return _path.boundingRect();
}

void QGVNode::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    painter->save();

    if (activeMode) {
        _brush.setColor(QGVCore::toColor("#a5cf80"));
        setAttribute("labelfontcolor", "#191970");
    }
    else {
        _brush.setColor(QGVCore::toColor(getAttribute("fillcolor")));
        _pen.setColor(QGVCore::toColor(getAttribute("color")));
        setAttribute("labelfontcolor", "#edad56");
    }

    if (errorMessage.size()) {
        _brush.setColor(QGVCore::toColor("#FA8072"));
    }

    painter->setPen(_pen);

    if(isSelected()) {
        QBrush tbrush(_brush);
        tbrush.setColor(tbrush.color().darker(170));
        painter->setBrush(tbrush);
    }
    else
        painter->setBrush(_brush);


    if(getAttribute("shape") == "box") {
        QPainterPath path;
        path.addRoundedRect(boundingRect(), 10, 10);
        painter->drawPath(path);        
        painter->drawLine(QPointF(0, 25), QPointF(boundingRect().width(), 25));
    }
    else
        painter->drawPath(_path);

    if(getAttribute("node_type") == "end") {
         QRectF rect = boundingRect();
         painter->setBrush(QBrush());
         painter->drawEllipse(rect.center(), rect.width(), rect.height());
    }

    painter->setPen(QGVCore::toColor(getAttribute("labelfontcolor")));
    QRectF rect = boundingRect().adjusted(2,5,-2,-2); //Margin
    QFont font;
    font.setPixelSize(12);
    font.setBold(true);
    painter->setFont(font);
    if(_icon.isNull()) {
        painter->drawText(rect, Qt::AlignTop | Qt::AlignHCenter , QGVNode::label());
        rect = boundingRect().adjusted(2,30,-2,-2); //Margin
        font.setPixelSize(10);
        font.setBold(false);
        font.setItalic(true);
        painter->setFont(font);
        QString callbacks;
        if(getAttribute("entry").size())
            callbacks += "+ entry()";
        if(getAttribute("doo").size())
            callbacks += "\n+ doo()";
        if(getAttribute("exit").size())
            callbacks += "\n+ exit()";

        painter->drawText(rect, Qt::AlignTop | Qt::AlignLeft , callbacks);

    }
    else {
        painter->drawText(rect.adjusted(0,0,0, -rect.height()*2/3), Qt::AlignCenter , QGVNode::label());
        const QRectF img_rect = rect.adjusted(0, rect.height()/3,0, 0);
        QImage img = _icon.scaled(img_rect.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter->drawImage(img_rect.topLeft() + QPointF((img_rect.width() - img.rect().width())/2, 0), img);
    }
    painter->restore();
}

void QGVNode::setAttribute(const QString &name, const QString &value)
{
    agsafeset(_node->node(), name.toLocal8Bit().data(), value.toLocal8Bit().data(), (char *)"");
}

QString QGVNode::getAttribute(const QString &name) const
{
    char* value = agget(_node->node(), name.toLocal8Bit().data());
    if(value)
        return value;
    return QString();
}

void QGVNode::setIcon(const QImage &icon)
{
    _icon = icon;
}

void QGVNode::setVertex(void* v) {
    vertex = v;
}

void* QGVNode::getVertex() {
    return vertex;
}

void QGVNode::updateLayout()
{
    prepareGeometryChange();
    qreal width = ND_width(_node->node())*DotDefaultDPI;
    qreal height = ND_height(_node->node())*DotDefaultDPI;

    //Node Position (center)
    qreal gheight = QGVCore::graphHeight(_scene->_graph->graph());
    setPos(QGVCore::centerToOrigin(QGVCore::toPoint(ND_coord(_node->node()), gheight), width, height));

    //Node on top
    setZValue(1);

    //Node path
    _path = QGVCore::toPath(ND_shape(_node->node())->name,
                            (polygon_t*)ND_shape_info(_node->node()),
                            width, height);
    _pen.setWidth(1);

    _brush.setStyle(QGVCore::toBrushStyle(getAttribute("style")));
    _brush.setColor(QGVCore::toColor(getAttribute("fillcolor")));
    _pen.setColor(QGVCore::toColor(getAttribute("color")));

    setToolTip(getAttribute("tooltip"));
}
