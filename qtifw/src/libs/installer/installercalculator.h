/**************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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
**************************************************************************/
#ifndef INSTALLERCALCULATOR_H
#define INSTALLERCALCULATOR_H

#include "installer_global.h"

#include <QHash>
#include <QList>
#include <QSet>
#include <QString>

namespace QInstaller {

class Component;

class INSTALLER_EXPORT InstallerCalculator
{
public:
    InstallerCalculator(const QList<Component *> &allComponents);

    enum InstallReasonType
    {
        Automatic, // "Component(s) added as automatic dependencies"
        Dependent, // "Added as dependency for %1."
        Resolved,  // "Component(s) that have resolved Dependencies"
        Selected   // "Selected Component(s) without Dependencies"
    };

    InstallReasonType installReasonType(Component *component) const;
    QString installReasonReferencedComponent(Component *component) const;
    QString installReason(Component *component) const;
    QList<Component*> orderedComponentsToInstall() const;
    QString componentsToInstallError() const;

    bool appendComponentsToInstall(const QList<Component*> &components);

private:
    void insertInstallReason(Component *component,
                             InstallReasonType installReasonType,
                             const QString &referencedComponentName = QString());
    void realAppendToInstallComponents(Component *component);
    bool appendComponentToInstall(Component *components);
    QString recursionError(Component *component);

    QList<Component*> m_allComponents;
    QHash<Component*, QSet<Component*> > m_visitedComponents;
    QSet<QString> m_toInstallComponentIds; //for faster lookups
    QString m_componentsToInstallError;
    //calculate installation order variables
    QList<Component*> m_orderedComponentsToInstall;
    //we can't use this reason hash as component id hash, because some reasons are ready before
    //the component is added
    QHash<QString, QPair<InstallReasonType, QString> > m_toInstallComponentIdReasonHash;
};

}


#endif // INSTALLERCALCULATOR_H
