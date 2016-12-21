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

#include "updatechecker.h"

#include <binaryformatenginehandler.h>
#include <component.h>
#include <errors.h>
#include <init.h>
#include <kdrunoncechecker.h>
#include <packagemanagercore.h>
#include <productkeycheck.h>

#include <QDomDocument>

#include <iostream>

#include "ngaccess.h"

UpdateChecker::UpdateChecker(int &argc, char *argv[])
    : SDKApp<QCoreApplication>(argc, argv)
{
    QInstaller::init(); // register custom operations
}

int UpdateChecker::check()
{
    // NGI: add NextGIS authentication before UpdateChecker::check() in a
    // command line application.
    // If we are still not authenticated - the app is in command line mode (any
    // update checks in installer GUI are possible only after showing authentication
    // page.
    if (!NgAccess::authenticated)
    {
        std::cout << std::endl << "NextGIS authentication. Connecting ..." << std::endl;
        NgAccess ngAccessPtr;
        ngAccessPtr.readAuthData(); // read lastly saved credentials
        QEventLoop eventLoop;
        QObject::connect(&ngAccessPtr, SIGNAL(authFinished()),
                         &eventLoop, SLOT(quit()));
        QString login = QString::fromUtf8(ngAccessPtr.getCurLogin().toUtf8());
        QString password = QString::fromUtf8(ngAccessPtr.getCurPassword().toUtf8());
        ngAccessPtr.startAuthetication(login, password);
        eventLoop.exec(); // wait for the end of the authentication
        if (NgAccess::authenticated)
            std::cout << "Authentication succeded! Checking for updates ..." << std::endl;
        else
            std::cout << "Authentication failed! Anyway try to check for updates ..." << std::endl;
    }

    KDRunOnceChecker runCheck(qApp->applicationDirPath() + QLatin1String("/lockmyApp15021976.lock"));
    if (runCheck.isRunning(KDRunOnceChecker::ConditionFlag::Lockfile)) {
        // It is possible to install an application and thus the maintenance tool into a
        // directory that requires elevated permission to create a lock file. Since this
        // cannot be done without requesting credentials from the user, we silently ignore
        // the fact that we could not create the lock file and check the running processes.
        if (runCheck.isRunning(KDRunOnceChecker::ConditionFlag::ProcessList))
            throw QInstaller::Error(QLatin1String("An instance is already checking for updates."));
    }

    QString fileName = datFile(binaryFile());
    quint64 cookie = QInstaller::BinaryContent::MagicCookieDat;
    if (fileName.isEmpty()) {
        fileName = binaryFile();
        cookie = QInstaller::BinaryContent::MagicCookie;
    }

    QFile binary(fileName);
    QInstaller::openForRead(&binary);

    qint64 magicMarker;
    QList<QInstaller::OperationBlob> operations;
    QInstaller::ResourceCollectionManager manager;
    QInstaller::BinaryContent::readBinaryContent(&binary, &operations, &manager, &magicMarker,
        cookie);

    if (magicMarker == QInstaller::BinaryContent::MagicInstallerMarker)
        throw QInstaller::Error(QLatin1String("Installers cannot check for updates."));

    SDKApp::registerMetaResources(manager.collectionByName("QResources"));

    QInstaller::PackageManagerCore core(QInstaller::BinaryContent::MagicUpdaterMarker, operations);
    QInstaller::PackageManagerCore::setVirtualComponentsVisible(true);
    {
        using namespace QInstaller;
        ProductKeyCheck::instance()->init(&core);
        ProductKeyCheck::instance()->addPackagesFromXml(QLatin1String(":/metadata/Updates.xml"));
        BinaryFormatEngineHandler::instance()->registerResources(manager.collections());
    }
    if (!core.fetchRemotePackagesTree())
        throw QInstaller::Error(core.error());

    const QList<QInstaller::Component *> components =
        core.components(QInstaller::PackageManagerCore::ComponentType::Root);
    if (components.isEmpty())
        throw QInstaller::Error(QLatin1String("There are currently no updates available."));

    QDomDocument doc;
    QDomElement root = doc.createElement(QLatin1String("updates"));
    doc.appendChild(root);

    foreach (QInstaller::Component *component, components) {
        QDomElement update = doc.createElement(QLatin1String("update"));
        update.setAttribute(QLatin1String("name"), component->value(QInstaller::scDisplayName));
        update.setAttribute(QLatin1String("version"), component->value(QInstaller::scRemoteVersion));
        update.setAttribute(QLatin1String("size"), component->value(QInstaller::scUncompressedSize));
        root.appendChild(update);
    }

    std::cout << qPrintable(doc.toString(4)) << std::endl;
    return EXIT_SUCCESS;
}
