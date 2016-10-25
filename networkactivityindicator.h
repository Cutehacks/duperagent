// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#ifndef NETWORKACTIVITYINDICATOR_H
#define NETWORKACTIVITYINDICATOR_H

#include <QObject>

namespace com { namespace cutehacks { namespace duperagent {

class NetworkActivityIndicator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool enabled READ enabled NOTIFY enabledChanged)
    Q_PROPERTY(int activationDelay READ activationDelay WRITE setActivationDelay
               NOTIFY activationDelayChanged)
    Q_PROPERTY(int completionDelay READ completionDelay WRITE setCompletionDelay
               NOTIFY completionDelayChanged)
    Q_PROPERTY(bool enableNativeIndicator READ enableNativeIndicator
               WRITE setEnableNativeIndicator NOTIFY enableNativeIndicatorChanged)

public:
    explicit NetworkActivityIndicator(QObject *parent = 0);
    virtual ~NetworkActivityIndicator();

    static NetworkActivityIndicator *instance();

    bool enabled() const { return m_enabled; }
    int activationDelay() const { return m_activationDelay; }
    int completionDelay() const { return m_completionDelay; }
    bool enableNativeIndicator() const { return m_enableNativeIndicator; }

    void incrementActivityCount();
    void decrementActivityCount();

signals:
    void activationDelayChanged(int activationDelay);
    void completionDelayChanged(int completionDelay);
    void enableNativeIndicatorChanged(bool enableNativeIndicator);
    void enabledChanged(bool enabled);

public slots:
    void setActivationDelay(int activationDelay);
    void setCompletionDelay(int completionDelay);
    void setEnableNativeIndicator(bool enableNativeIndicator);

protected:
    void timerEvent(QTimerEvent*);
    void setEnabled(bool);
    void setEnabledNative(bool);

private:
    static NetworkActivityIndicator *m_instance;
    QMutex *m_mutex;
    bool m_enabled;
    int m_activationDelay;
    int m_activationTimer;
    int m_completionDelay;
    int m_completionTimer;
    bool m_enableNativeIndicator;
    uint m_activityCounter;
};

} } }

#endif // NETWORKACTIVITYINDICATOR_H
