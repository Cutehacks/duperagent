// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.
#include <QQmlEngine>
#include <QQmlContext>
#include <QtQuickTest/quicktest.h>
#include "../duperagent.h"

class Setup : public QObject
{
    Q_OBJECT

public:
    Setup() {}

public slots:
    void qmlEngineAvailable(QQmlEngine *)    {
        com::cutehacks::duperagent::registerTypes();
    }
};

QUICK_TEST_MAIN_WITH_SETUP("duperagent", Setup)


#include "tst_duperagent.moc"

