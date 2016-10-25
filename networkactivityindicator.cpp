// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#include <QtCore/QTimerEvent>
#include <QtCore/QMutexLocker>
#include "networkactivityindicator.h"

namespace com { namespace cutehacks { namespace duperagent {

NetworkActivityIndicator *NetworkActivityIndicator::m_instance = 0;

NetworkActivityIndicator::NetworkActivityIndicator(QObject *parent) :
    QObject(parent),
    m_mutex(new QMutex),
    m_enabled(false),
    m_activationDelay(1000),
    m_activationTimer(-1),
    m_completionDelay(170),
    m_completionTimer(-1),
    m_enableNativeIndicator(false),
    m_activityCounter(0)
{
}

NetworkActivityIndicator *NetworkActivityIndicator::instance()
{
    if (!m_instance)
        m_instance = new NetworkActivityIndicator;
    return m_instance;
}

NetworkActivityIndicator::~NetworkActivityIndicator()
{
    delete m_mutex;

    if (m_activationTimer > 0)
        killTimer(m_activationTimer);
    if (m_completionTimer > 0)
        killTimer(m_completionTimer);
}

void NetworkActivityIndicator::incrementActivityCount()
{
    QMutexLocker lock(m_mutex);
    m_activityCounter++;

    if (m_activityCounter > 0) {
        if (m_completionTimer > 0) {
            killTimer(m_completionTimer);
            m_completionTimer = -1;
        }

        if (m_activationTimer < 0 && m_activationDelay >= 0)
            m_activationTimer = startTimer(m_activationDelay, Qt::PreciseTimer);
    }
}

void NetworkActivityIndicator::decrementActivityCount()
{
    QMutexLocker lock(m_mutex);
    m_activityCounter--;

    if (m_activityCounter <= 0) {
        if (m_activationTimer > 0) {
            killTimer(m_activationTimer);
            m_activationTimer = -1;
        }

        if (m_completionTimer < 0 && m_completionDelay >= 0)
            m_completionTimer = startTimer(m_completionDelay, Qt::PreciseTimer);
    }
}

void NetworkActivityIndicator::setActivationDelay(int activationDelay)
{
    if (m_activationDelay == activationDelay)
        return;

    m_activationDelay = activationDelay;
    emit activationDelayChanged(activationDelay);
}

void NetworkActivityIndicator::setCompletionDelay(int completionDelay)
{
    if (m_completionDelay == completionDelay)
        return;

    m_completionDelay = completionDelay;
    emit completionDelayChanged(completionDelay);
}

void NetworkActivityIndicator::setEnableNativeIndicator(bool enableNativeIndicator)
{
    if (m_enableNativeIndicator == enableNativeIndicator)
        return;

    m_enableNativeIndicator = enableNativeIndicator;
    emit enableNativeIndicatorChanged(enableNativeIndicator);
}

void NetworkActivityIndicator::timerEvent(QTimerEvent *event)
{
    int t = event->timerId();
    if (t == m_activationTimer) {
        setEnabled(true);
        m_activationTimer = -1;
    } else if (t == m_completionTimer) {
        setEnabled(false);
        m_completionTimer = -1;
    }

    killTimer(t);
}

void NetworkActivityIndicator::setEnabled(bool enabled)
{
    if (enabled == m_enabled)
        return;

    m_enabled = enabled;

    if (m_enableNativeIndicator) {
        setEnabledNative(m_enabled);
    }

    emit enabledChanged(m_enabled);
}

#ifndef Q_OS_IOS
void NetworkActivityIndicator::setEnabledNative(bool)
{

}
#endif

} } }

