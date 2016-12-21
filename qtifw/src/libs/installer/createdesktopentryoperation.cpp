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
#include "createdesktopentryoperation.h"

#include "errors.h"
#include "fileutils.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QTextStream>

using namespace QInstaller;

QString CreateDesktopEntryOperation::absoluteFileName()
{
    const QString filename = arguments().first();

    // give filename is already absolute
    if (QFileInfo(filename).isAbsolute())
        return filename;

    // we're not searching for the first time, let's re-use the old value
    if (hasValue(QLatin1String("directory")))
        return QDir(value(QLatin1String("directory")).toString()).absoluteFilePath(filename);

    const QProcessEnvironment env;
    QStringList XDG_DATA_DIRS = env.value(QLatin1String("XDG_DATA_DIRS")).split(QLatin1Char(':'),
        QString::SkipEmptyParts);
    QStringList XDG_DATA_HOME = env.value(QLatin1String("XDG_DATA_HOME")).split(QLatin1Char(':'),
        QString::SkipEmptyParts);

    XDG_DATA_DIRS.push_back(QLatin1String("/usr/share")); // default path
    XDG_DATA_HOME.push_back(QDir::home().absoluteFilePath(QLatin1String(".local/share"))); // default path

    const QStringList directories = XDG_DATA_DIRS + XDG_DATA_HOME;
    QString directory;
    for (QStringList::const_iterator it = directories.begin(); it != directories.end(); ++it) {
        if (it->isEmpty())
            continue;

        directory = QDir(*it).absoluteFilePath(QLatin1String("applications"));
        QDir dir(directory);

        // let's see whether this dir exists or we're able to create it
        if (!dir.exists() && !QDir().mkpath(directory))
            continue;

        // we just try whether we're able to open the file in ReadWrite
        QFile file(QDir(directory).absoluteFilePath(filename));
        const bool existed = file.exists();
        if (!file.open(QIODevice::ReadWrite))
            continue;
        file.close();
        if (!existed)
            file.remove();
        break;
    }

    if (!QDir(directory).exists())
        QDir().mkpath(directory);

    setValue(QLatin1String("directory"), directory);

    return QDir(directory).absoluteFilePath(filename);
}

CreateDesktopEntryOperation::CreateDesktopEntryOperation()
{
    setName(QLatin1String("CreateDesktopEntry"));
}

CreateDesktopEntryOperation::~CreateDesktopEntryOperation()
{
}

void CreateDesktopEntryOperation::backup()
{
    const QString filename = absoluteFileName();
    QFile file(filename);

    if (!file.exists())
        return;

    try {
        setValue(QLatin1String("backupOfExistingDesktopEntry"), generateTemporaryFileName(filename));
    } catch (const QInstaller::Error &e) {
        setErrorString(e.message());
        return;
    }

    if (!file.copy(value(QLatin1String("backupOfExistingDesktopEntry")).toString()))
        setErrorString(tr("Could not backup file %1: %2").arg(filename, file.errorString()));
}

bool CreateDesktopEntryOperation::performOperation()
{
    const QStringList args = arguments();
    if (args.count() != 2) {
        setError(InvalidArguments);
        setErrorString(tr("Invalid arguments in %0: %1 arguments given, %2 expected%3.")
            .arg(name()).arg(arguments().count()).arg(tr("exactly 2"), QLatin1String("")));
        return false;
    }

    const QString filename = absoluteFileName();
    const QString &values = args[1];

    QFile file(filename);
    if (file.exists() && !file.remove()) {
        setError(UserDefinedError);
        setErrorString(tr("Failed to overwrite %1").arg(filename));
        return false;
    }

    if(!file.open(QIODevice::WriteOnly)) {
        setError(UserDefinedError);
        setErrorString(tr("Could not write Desktop Entry at %1").arg(filename));
        return false;
    }

    QFile::setPermissions(filename, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::ReadGroup
        | QFile::ReadOther | QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther);

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << QLatin1String("[Desktop Entry]") << endl;

    // Type=Application\nExec=qtcreator\nPath=...
    const QStringList pairs = values.split(QLatin1Char('\n'));
    for (QStringList::const_iterator it = pairs.begin(); it != pairs.end(); ++it)
        stream << *it << endl;

    return true;
}

bool CreateDesktopEntryOperation::undoOperation()
{
    const QString filename = absoluteFileName();

    // first remove the link
    QFile file(filename);
    if (file.exists() && !file.remove()) {
        qWarning() << "Could not delete file" << filename << file.errorString();
        return true;
    }

    if (!hasValue(QLatin1String("backupOfExistingDesktopEntry")))
        return true;

    QFile backupFile(value(QLatin1String("backupOfExistingDesktopEntry")).toString());
    if (!backupFile.exists()) {
        // do not treat this as a real error: The backup file might have been just nuked by the user.
        qWarning() << "Could not restore original desktop entry at" << filename
                   << ": Backup file" << backupFile.fileName() << "does not exist anymore.";
        return true;
    }

    if (!backupFile.rename(filename))
        qWarning() << "Could not restore the file" << filename << ":" << backupFile.errorString();

    return true;
}

bool CreateDesktopEntryOperation::testOperation()
{
    return true;
}

Operation *CreateDesktopEntryOperation::clone() const
{
    return new CreateDesktopEntryOperation();
}
