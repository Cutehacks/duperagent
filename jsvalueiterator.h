#include <QtQml/QJSValue>

class JSValueIterator
{
public:
    JSValueIterator(const QJSValue &value)
        : m_value(value)
    {
        if (value.isArray()) {
            int length = value.property("length").toInt();
            for (int i = 0; i < length; ++i)
                m_keys << QString::number(i);
            m_keys << QString::number(length);
        } else {
            QJSValue object = value.property("constructor");
            QJSValue keysMethod = object.property("keys");

            if (keysMethod.isCallable()) {
                QJSValue ret = keysMethod.callWithInstance(object, QJSValueList() << value);
                foreach (QVariant key, ret.toVariant().toList())
                    m_keys << key.toString();
            }
        }
    }

    bool next()
    {
        if (m_keys.isEmpty())
            return false;
        m_currentName = m_keys.takeFirst();
        m_currentValue = m_value.property(m_currentName);
        return true;
    }

    bool hasNext() { return !m_keys.isEmpty(); }

    QString name() const { return m_currentName; }
    QJSValue value() const { return m_currentValue; }

private:
    QString m_currentName;
    QJSValue m_currentValue;

    QJSValue m_value;
    QList<QString> m_keys;
};

