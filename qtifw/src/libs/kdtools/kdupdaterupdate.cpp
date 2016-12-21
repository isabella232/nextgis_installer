/****************************************************************************
**
** Copyright (C) 2013 Klaralvdalens Datakonsult AB (KDAB)
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Installer Framework.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "kdupdaterupdate.h"

using namespace KDUpdater;

/*!
   \inmodule kdupdater
   \class KDUpdater::Update
   \brief Represents a single update

   The KDUpdater::Update class contains information about an update. It is created by KDUpdater::UpdateFinder
   corresponding to the update.

   The constructor of the KDUpdater::Update class is made protected, because it can be instantiated only by
   KDUpdater::UpdateFinder (which is a friend class). The destructor however is public.
*/


/*!
   \internal
*/
Update::Update(int priority, const QUrl &sourceInfoUrl, const QHash<QString, QVariant> &data)
    : m_priority(priority)
    , m_sourceInfoUrl(sourceInfoUrl)
    , m_data(data)
{
}

/*!
   Returns the data specified by \a name, or an invalid \a defaultValue if the
   data does not exist.
*/
QVariant Update::data(const QString &name, const QVariant &defaultValue) const
{
    return m_data.value(name, defaultValue);
}

/*!
   Returns the priority of the update.
*/
int Update::priority() const
{
    return m_priority;
}

/*!
    Returns the URL of the update source. An update source is a repository that
    contains an update for the application.
*/
QUrl Update::sourceInfoUrl() const
{
    return m_sourceInfoUrl;
}
