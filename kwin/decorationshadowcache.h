/* This file is a part of the Atmo desktop experience framework project for SynOS .
 * Copyright (C) 2025 Syndromatic Ltd. All rights reserved
 * Designed by Kavish Krishnakumar in Manchester.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef DECORATIONSHADOWCACHE_H
#define DECORATIONSHADOWCACHE_H

#include "kdecoration_compat.h"

#include <QColor>
#include <QHash>
#include <QMarginsF>
#include <memory>

namespace NSE
{

class DecorationShadowCache
{
public:
    struct Key
    {
        int scale;
        int dpr;
        bool active;
        int size;
        int opacity;
        int radius;
        int border;
        QRgb tint;
        uint themeHash;

        bool operator==(const Key &other) const;
    };

    static std::shared_ptr<KDecoration3::DecorationShadow> shadow(qreal scale,
                                                                  qreal dpr,
                                                                  bool active,
                                                                  int shadowSize,
                                                                  int shadowOpacity,
                                                                  int radius,
                                                                  int border,
                                                                  const QColor &tint,
                                                                  uint themeHash);

private:
    static QImage renderShadow(const Key &key, qreal scale, qreal dpr, QMarginsF &padding, QRectF &innerRect);
    static QHash<Key, std::weak_ptr<KDecoration3::DecorationShadow>> s_cache;
};

uint qHash(const DecorationShadowCache::Key &key, uint seed = 0);

}

#endif // DECORATIONSHADOWCACHE_H
