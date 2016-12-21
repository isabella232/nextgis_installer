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

#include "selfrestartoperation.h"
#include "packagemanagercore.h"

#include <kdselfrestarter.h>

using namespace QInstaller;

SelfRestartOperation::SelfRestartOperation()
{
    setName(QLatin1String("SelfRestart"));
}

void SelfRestartOperation::backup()
{
    setValue(QLatin1String("PreviousSelfRestart"), KDSelfRestarter::restartOnQuit());
}

bool SelfRestartOperation::performOperation()
{
    PackageManagerCore *const core = value(QLatin1String("installer")).value<PackageManagerCore*>();
    if (!core) {
        setError(UserDefinedError);
        setErrorString(tr("Installer object needed in '%1' operation is empty.").arg(name()));
        return false;
    }

    if (!core->isUpdater() && !core->isPackageManager()) {
        setError(UserDefinedError);
        setErrorString(tr("Self Restart: Only valid within updater or packagemanager mode."));
        return false;
    }

    if (!arguments().isEmpty()) {
        setError(InvalidArguments);
        setErrorString(tr("Self Restart: Invalid arguments"));
        return false;
    }
    KDSelfRestarter::setRestartOnQuit(true);
    return KDSelfRestarter::restartOnQuit();
}

bool SelfRestartOperation::undoOperation()
{
    KDSelfRestarter::setRestartOnQuit(value(QLatin1String("PreviousSelfRestart")).toBool());
    return true;
}

bool SelfRestartOperation::testOperation()
{
    return true;
}

Operation *SelfRestartOperation::clone() const
{
    return new SelfRestartOperation();
}
