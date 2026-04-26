/* This file is a part of the Atmo desktop experience framework project for SynOS .
 * Copyright (C) 2025 Syndromatic Ltd. All rights reserved
 * Designed by Kavish Krishnakumar in Manchester.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */
#include "decorationshadowcache.h"

#include "../atmolib/color.h"
#include "../atmolib/fx.h"

#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

namespace NSE
{

QHash<DecorationShadowCache::Key, std::weak_ptr<KDecoration3::DecorationShadow>> DecorationShadowCache::s_cache;

bool
DecorationShadowCache::Key::operator==(const Key &other) const
{
    return scale == other.scale
        && dpr == other.dpr
        && active == other.active
        && size == other.size
        && opacity == other.opacity
        && radius == other.radius
        && border == other.border
        && tint == other.tint
        && themeHash == other.themeHash;
}

static inline int scaled(qreal value, qreal dpr)
{
    return qMax(1, qRound(value * dpr));
}

std::shared_ptr<KDecoration3::DecorationShadow>
DecorationShadowCache::shadow(qreal scale,
                              qreal dpr,
                              bool active,
                              int shadowSize,
                              int shadowOpacity,
                              int radius,
                              int border,
                              const QColor &tint,
                              uint themeHash)
{
    Key key;
    key.scale = qRound(scale * 1000.0);
    key.dpr = qRound(dpr * 1000.0);
    key.active = active;
    key.size = qMax(6, shadowSize);
    key.opacity = qBound(0, shadowOpacity, 255);
    key.radius = qMax(0, radius);
    key.border = qMax(0, border);
    key.tint = tint.rgba();
    key.themeHash = themeHash;

    if (auto cached = s_cache.value(key).lock())
        return cached;

    QMarginsF padding;
    QRectF innerRect;
    QImage image = renderShadow(key, scale, dpr, padding, innerRect);

    auto decoShadow = std::make_shared<KDecoration3::DecorationShadow>();
    decoShadow->setShadow(image);
    decoShadow->setInnerShadowRect(innerRect);
    decoShadow->setPadding(padding);
    s_cache.insert(key, decoShadow);
    return decoShadow;
}

QImage
DecorationShadowCache::renderShadow(const Key &key, qreal scale, qreal dpr, QMarginsF &padding, QRectF &innerRect)
{
    Q_UNUSED(scale)
    const qreal size = key.active ? key.size : qMax(4, key.size - 6);
    const qreal left = qMax<qreal>(6.0, size * 0.72);
    const qreal top = qMax<qreal>(3.0, size * 0.42);
    const qreal right = qMax<qreal>(6.0, size * 0.72);
    const qreal bottom = qMax<qreal>(8.0, size * 1.12);
    const qreal innerW = qMax<qreal>(48.0, key.radius * 2.0 + 28.0);
    const qreal innerH = qMax<qreal>(48.0, key.radius * 2.0 + 28.0);

    padding = QMarginsF(left, top, right, bottom);
    innerRect = QRectF(left, top, innerW, innerH);

    const QSize pxSize(scaled(left + innerW + right, dpr), scaled(top + innerH + bottom, dpr));
    const QRectF pxInner(left * dpr, top * dpr, innerW * dpr, innerH * dpr);
    const qreal pxRadius = qMax<qreal>(1.0, key.radius * dpr);

    QImage soft(pxSize, QImage::Format_ARGB32_Premultiplied);
    soft.fill(Qt::transparent);

    const QColor tint = QColor::fromRgba(key.tint);
    const int baseAlpha = key.active ? qBound(80, key.opacity, 190) : qBound(35, int(key.opacity * 0.48), 90);
    const int contactAlpha = key.active ? qBound(45, int(key.opacity * 0.42), 105) : qBound(18, int(key.opacity * 0.20), 42);
    const QColor shadowColor = Color::mid(tint, Qt::black, 1, 7);

    QPainter p(&soft);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);

    QColor softColor = shadowColor;
    softColor.setAlpha(baseAlpha);
    p.setBrush(softColor);
    p.drawRoundedRect(pxInner.adjusted(0, 0, 0, bottom * dpr * 0.26), pxRadius, pxRadius);
    p.end();

    FX::expblur(soft, qMax(2, qRound(size * dpr * 0.48)));

    QImage contact(pxSize, QImage::Format_ARGB32_Premultiplied);
    contact.fill(Qt::transparent);
    p.begin(&contact);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    QColor contactColor = Qt::black;
    contactColor.setAlpha(contactAlpha);
    p.setBrush(contactColor);
    p.drawRoundedRect(pxInner.translated(0, bottom * dpr * 0.36).adjusted(left * dpr * 0.18, 0, -right * dpr * 0.18, 0),
                      pxRadius,
                      pxRadius);
    p.end();
    FX::expblur(contact, qMax(2, qRound(size * dpr * 0.30)));

    p.begin(&soft);
    p.drawImage(0, 0, contact);

    QLinearGradient vertical(0, 0, 0, soft.height());
    vertical.setColorAt(0.0, QColor(0, 0, 0, key.active ? 145 : 105));
    vertical.setColorAt(0.35, QColor(0, 0, 0, key.active ? 205 : 150));
    vertical.setColorAt(1.0, QColor(0, 0, 0, 255));
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.fillRect(soft.rect(), vertical);

    p.setCompositionMode(QPainter::CompositionMode_Clear);
    p.setBrush(Qt::black);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(pxInner.adjusted(-1, -1, 1, 1), pxRadius, pxRadius);
    p.end();

    soft.setDevicePixelRatio(dpr);
    return soft;
}

uint
qHash(const DecorationShadowCache::Key &key, uint seed)
{
    seed ^= ::qHash(key.scale, seed + 0x9e3779b9);
    seed ^= ::qHash(key.dpr, seed + 0x85ebca6b);
    seed ^= ::qHash(key.active, seed + 0xc2b2ae35);
    seed ^= ::qHash(key.size, seed + 0x27d4eb2f);
    seed ^= ::qHash(key.opacity, seed + 0x165667b1);
    seed ^= ::qHash(key.radius, seed + 0xd3a2646c);
    seed ^= ::qHash(key.border, seed + 0xfd7046c5);
    seed ^= ::qHash(key.tint, seed + 0xb55a4f09);
    seed ^= ::qHash(key.themeHash, seed + 0x94d049bb);
    return seed;
}

}
