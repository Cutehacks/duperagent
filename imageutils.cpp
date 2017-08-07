// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#include <QtCore/QFileInfo>
#include <QtCore/QBuffer>
#include <QtGui/QImageReader>
#include <QtQml/QQmlEngine>
#include "imageutils.h"

#include <QDebug>

namespace com { namespace cutehacks { namespace duperagent {

ImageUtils::ImageUtils(QQmlEngine *engine) :
    QObject(0),
    m_engine(engine)
{}

QJSValue ImageUtils::createReader(const QString &filename)
{
    return (new Image(m_engine, filename))->self();
}

Image::Image(QQmlEngine *engine, const QString &filename) :
    AbstractMultipartSource(),
    m_engine(engine)
{
    m_self = engine->newQObject(this);
    engine->setObjectOwnership(this, QQmlEngine::JavaScriptOwnership);
    if (filename.startsWith(QStringLiteral("data:"))) {
        int comma = filename.indexOf(QChar(','));
        QStringRef data = filename.midRef(comma+1);
        QBuffer *buffer = new QBuffer(this);
        buffer->setData(QByteArray::fromBase64(data.toLatin1()));
        m_reader = new QImageReader(buffer);
    } else {
        m_reader = new QImageReader(filename);
    }
}

Image::~Image()
{
    delete m_reader;
}

QJSValue Image::size() const
{
    QJSValue val = m_engine->newObject();
    QSize s = m_reader->size();
    val.setProperty("width", s.width());
    val.setProperty("height", s.height());
    return val;
}

int Image::fileSize() const
{
    QString fn = m_reader->fileName();
    if (!fn.isEmpty()) {
        QFileInfo info(m_reader->fileName());
        return (int)info.size();
    } else {
        return m_reader->device()->size();
    }
}

QJSValue Image::setScaledSize(int w, int h, const QJSValue &scaleMode)
{
    QSize newSize(w, h);
    if (scaleMode.isNumber()) {
        int mode = scaleMode.toInt();
        if (mode < 0 || mode > 2) {
            qWarning("Invalid scale mode");
        } else {
            QSize original = m_reader->size();
            newSize = original.scaled(newSize, (Qt::AspectRatioMode)mode);
        }
    }
    m_reader->setScaledSize(newSize);
    return self();
}

QJSValue Image::setClipRect(int x, int y, int w, int h)
{
    m_reader->setClipRect(QRect(x, y, w, h));
    return self();
}

QJSValue Image::setScaledClipRect(int x, int y, int w, int h)
{
    m_reader->setClipRect(QRect(x, y, w, h));
    return self();
}

QJSValue Image::setAutoTransform(bool)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
    m_reader->setAutoTransform(true);
#endif
    return self();
}

QString Image::toJSON()
{
    return QStringLiteral("data:%1;base64,%2")
        .arg(m_mimeType)
        .arg(QString::fromLatin1(m_data.toBase64()));
}

QJSValue Image::read(const QJSValue& options)
{
    QByteArray format = m_reader->format();
    int quality = -1;

    if (options.isObject()) {
        QJSValue transcodeOptions = options.property("transcode");
        if (!transcodeOptions.isUndefined()) {
            QJSValue f = transcodeOptions.property("format");
            if (!f.isUndefined())
                format = f.toString().toUtf8();

            QJSValue q = transcodeOptions.property("quality");
            if (!q.isUndefined())
                quality = q.toInt();
        }
    }

    QImage img = m_reader->read();
    if (img.isNull()) {
        qWarning("Error reading image: %s", qUtf8Printable(m_reader->errorString()));
        return self();
    }

    QBuffer buffer(&m_data);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, format.constData(), quality);
    m_mimeType = QStringLiteral("image/%1")
        .arg(QString::fromLatin1(format.constData()));

    return self();
}

QByteArray Image::data() const
{
    return m_data;
}

QString Image::mimeType() const
{
    return m_mimeType;
}

} } }


