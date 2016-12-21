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
#include <QFile>
#include <QString>
#include <iostream>

static void crash() {
    QString* nemesis = 0;
    nemesis->clear();
}

int main( int argc, char** argv ) {
    std::cout << "Hello." << std::endl;
    if ( argc < 2 )
        return 0;
    const QString arg = QString::fromLocal8Bit( argv[1] ); 
    bool ok = false;
    const int num = arg.toInt( &ok );
    if ( ok )
        return num;
    if ( arg == QLatin1String("crash") ) {
        std::cout << "Yeth, mather. I Crash." << std::endl;
        crash();
    }
    if ( arg == QLatin1String("--script") ) {
        const QString fn = QString::fromLocal8Bit( argv[2] ); 
        QFile f( fn );
        if ( !f.open( QIODevice::ReadOnly ) ) {
            std::cerr << "Could not open file for reading: " << qPrintable(f.errorString()) << std::endl;
            crash();
        }
        bool ok;
        const int rv = QString::fromLatin1( f.readAll() ).toInt( &ok );
        if ( ok )
            return rv;
        else
            crash();
    }

}
