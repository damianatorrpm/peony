/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "global-settings.h"
#include <QtConcurrent>

using namespace Peony;

static GlobalSettings *global_instance = nullptr;

GlobalSettings *GlobalSettings::getInstance()
{
    if (!global_instance) {
        global_instance = new GlobalSettings;
    }
    return global_instance;
}

GlobalSettings::GlobalSettings(QObject *parent) : QObject(parent)
{
    m_settings = new QSettings("org.ukui", "peony-qt-preferences", this);
    for (auto key : m_settings->allKeys()) {
        m_cache.insert(key, m_settings->value(key));
    }
}

GlobalSettings::~GlobalSettings()
{

}

const QVariant GlobalSettings::getValue(const QString &key)
{
    return m_cache.value(key);
}

bool GlobalSettings::isExist(const QString &key)
{
    return !m_cache.value(key).isNull();
}

void GlobalSettings::reset(const QString &key)
{
    m_cache.remove(key);
    QtConcurrent::run([=](){
        if (m_mutex.tryLock(1000)) {
            m_settings->remove(key);
            m_settings->sync();
            m_mutex.unlock();
        }
    });
    Q_EMIT this->valueChanged(key);
}

void GlobalSettings::resetAll()
{
    QStringList tmp = m_cache.keys();
    m_cache.clear();
    for (auto key : tmp) {
        Q_EMIT this->valueChanged(key);
    }
    QtConcurrent::run([=](){
        if (m_mutex.tryLock(1000)) {
            m_settings->clear();
            m_settings->sync();
            m_mutex.unlock();
        }
    });
}

void GlobalSettings::setValue(const QString &key, const QVariant &value)
{
    m_cache.insert(key, value);
    QtConcurrent::run([=](){
        if (m_mutex.tryLock(1000)) {
            m_settings->setValue(key, value);
            m_settings->sync();
            m_mutex.unlock();
        }
    });
}

void GlobalSettings::forceSync(const QString &key)
{
    m_settings->sync();
    if (key.isNull()) {
        m_cache.clear();
        for (auto key : m_settings->allKeys()) {
            m_cache.insert(key, m_settings->value(key));
        }
    } else {
        m_cache.remove(key);
        m_cache.insert(key, m_settings->value(key));
    }
}