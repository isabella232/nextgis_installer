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

#ifndef REMOTESERVERCONNECTION_P_H
#define REMOTESERVERCONNECTION_P_H

#include "protocol.h"

#include <QMutex>
#include <QProcess>
#include <QVariant>

namespace QInstaller {

class QProcessSignalReceiver : public QObject
{
    Q_OBJECT
    friend class RemoteServerConnection;

private:
    explicit QProcessSignalReceiver(QProcess *process)
        : QObject(process)
    {
        connect(process, SIGNAL(bytesWritten(qint64)), SLOT(onBytesWritten(qint64)));
        connect(process, SIGNAL(aboutToClose()), SLOT(onAboutToClose()));
        connect(process, SIGNAL(readChannelFinished()), SLOT(onReadChannelFinished()));
        connect(process, SIGNAL(error(QProcess::ProcessError)),
            SLOT(onError(QProcess::ProcessError)));
        connect(process, SIGNAL(readyReadStandardOutput()), SLOT(onReadyReadStandardOutput()));
        connect(process, SIGNAL(readyReadStandardError()), SLOT(onReadyReadStandardError()));
        connect(process, SIGNAL(finished(int, QProcess::ExitStatus)),
            SLOT(onFinished(int, QProcess::ExitStatus)));
        connect(process, SIGNAL(readyRead()), SLOT(onReadyRead()));
        connect(process, SIGNAL(started()), SLOT(onStarted()));
        connect(process, SIGNAL(stateChanged(QProcess::ProcessState)),
            SLOT(onStateChanged(QProcess::ProcessState)));
    }

private Q_SLOTS:
    void onBytesWritten(qint64 count) {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::QProcessSignalBytesWritten));
        m_receivedSignals.append(count);
    }

    void onAboutToClose() {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::QProcessSignalAboutToClose));
    }

    void onReadChannelFinished() {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::QProcessSignalReadChannelFinished));
    }

    void onError(QProcess::ProcessError error) {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::QProcessSignalError));
        m_receivedSignals.append(static_cast<int> (error));
    }

    void onReadyReadStandardOutput() {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::QProcessSignalReadyReadStandardOutput));
    }

    void onReadyReadStandardError() {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::QProcessSignalReadyReadStandardError));
    }

    void onFinished(int exitCode, QProcess::ExitStatus exitStatus) {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::QProcessSignalFinished));
        m_receivedSignals.append(exitCode);
        m_receivedSignals.append(static_cast<int> (exitStatus));
    }

    void onReadyRead() {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::QProcessSignalReadyRead));
    }

    void onStarted() {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::QProcessSignalStarted));
    }

    void onStateChanged(QProcess::ProcessState newState) {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::QProcessSignalStateChanged));
        m_receivedSignals.append(static_cast<int>(newState));
    }

private:
    QMutex m_lock;
    QVariantList m_receivedSignals;
};

} // namespace QInstaller

#endif // REMOTESERVERCONNECTION_P_H
