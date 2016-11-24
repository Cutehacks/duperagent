// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#ifndef IMAGEUTILS_H
#define IMAGEUTILS_H

#include <QObject>
#include <QtQml/QJSValue>
#include "multipartsource.h"

class QQmlEngine;
class QImageReader;

namespace com { namespace cutehacks { namespace duperagent {

class Image : public AbstractMultipartSource
{
    Q_OBJECT

public:
    Image(QQmlEngine *, const QString &);
    ~Image();

    QJSValue self() const { return m_self; }

    Q_INVOKABLE QJSValue size() const;
    Q_INVOKABLE int fileSize() const;

    Q_INVOKABLE QJSValue setScaledSize(int, int, const QJSValue& = QJSValue());
    Q_INVOKABLE QJSValue setClipRect(int, int, int, int);
    Q_INVOKABLE QJSValue setScaledClipRect(int, int, int, int);
    Q_INVOKABLE QJSValue setAutoTransform(bool);

    Q_INVOKABLE QString toJSON();
    Q_INVOKABLE QJSValue read(const QJSValue& = QJSValue());

    QByteArray data() const;
    QString mimeType() const;

private:
    QQmlEngine *m_engine;
    QImageReader *m_reader;
    QByteArray m_data;
    QString m_mimeType;
    QJSValue m_self;
};

class ImageUtils : public QObject
{
    Q_OBJECT

public:
    explicit ImageUtils(QQmlEngine *);
    Q_INVOKABLE QJSValue createReader(const QString&);

private:
    QQmlEngine *m_engine;
};

} } }

#endif // IMAGEUTILS_H
