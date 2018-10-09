// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#include <QtCore/QRegExp>
#include <QtCore/QTextCodec>
#include <QtNetwork/QNetworkReply>
#include <QtQml/QQmlEngine>
#include <QByteArray>
#include <zlib.h>

#include "response.h"
#include "serialization.h"

namespace com { namespace cutehacks { namespace duperagent {

ResponsePrototype::ResponsePrototype(QQmlEngine *engine, QNetworkReply *reply) : QObject(0),
    m_engine(engine),
    m_reply(reply)
{
    QString type = m_reply->header(QNetworkRequest::ContentTypeHeader).toString();
    QByteArray contentEncoding = m_reply->rawHeader(QByteArray ("Content-Encoding"));

    QRegExp charsetRegexp(".*charset=(.*)[\\s]*", Qt::CaseInsensitive, QRegExp::RegExp2);
    if (charsetRegexp.exactMatch(type)) {
        m_charset = charsetRegexp.capturedTexts().at(1);
    }

    if (m_reply->isReadable()) {
        QByteArray rawdata = m_reply->readAll();
        QByteArray data;
        if (
					contentEncoding  == QString("gzip") &&
					data.at(0) == (char) 0x1f &&
					data.at(1) == (char) 0x8b
				) {
            gzipDecompress(rawdata, data);
        } else {
            data  = rawdata;
        }

        QTextCodec *text = QTextCodec::codecForName(m_charset.toLatin1());
        m_text = text ? text->makeDecoder()->toUnicode(data) : QString::fromUtf8(data);

        if (type.contains("application/json")) {
            // TODO: add error handling
            JsonCodec json(m_engine);
            m_body = json.parse(data);
//        } else if (type.contains("application/x-www-form-urlencoded")) {
            // TODO: Implement parsing of form-urlencoded
//        } else if (type.contains("multipart/form-data")) {
            // TODO: Implement parsing of form-data
        } else if (type.contains("image/")) {
            m_body = QString("data:%1;base64,%2")
                    .arg(type)
                    .arg(QString::fromLatin1(data.toBase64()));
        } else {
            m_body = QJSValue(m_text);
        }
    }

    m_header = m_engine->newObject();
    QList<QNetworkReply::RawHeaderPair>::const_iterator it = m_reply->rawHeaderPairs().cbegin();
    for(; it != m_reply->rawHeaderPairs().cend(); it++) {
        m_header.setProperty(
                    QString::fromUtf8((*it).first).toLower(),
                    QString::fromUtf8((*it).second));
    }

    m_engine->setObjectOwnership(this, QQmlEngine::JavaScriptOwnership);
}

ResponsePrototype::~ResponsePrototype()
{
    delete m_reply;
}

int ResponsePrototype::statusType() const {
    return statusCode() / 100;
}

int ResponsePrototype::statusCode() const {
    QVariant var = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    bool ok;
    int i = var.toInt(&ok);
    return ok ? i : 0;
}

bool ResponsePrototype::typeEquals(int type) const
{
    return statusType() == type;
}

bool ResponsePrototype::statusEquals(int code) const
{
    QVariant var = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    bool ok;
    int i = var.toInt(&ok);
    return ok && i == code;
}

bool ResponsePrototype::info() const { return typeEquals(1); }
bool ResponsePrototype::ok() const { return typeEquals(2); }
bool ResponsePrototype::clientError() const { return typeEquals(4); }
bool ResponsePrototype::serverError() const { return typeEquals(5); }
bool ResponsePrototype::error() const { return typeEquals(4) || typeEquals(5); }

bool ResponsePrototype::accepted() const { return statusEquals(202); }
bool ResponsePrototype::noContent() const { return statusEquals(204); }
bool ResponsePrototype::badRequest() const { return statusEquals(400); }
bool ResponsePrototype::unauthorized() const { return statusEquals(401); }
bool ResponsePrototype::notAcceptable() const { return statusEquals(406); }
bool ResponsePrototype::notFound() const { return statusEquals(404); }
bool ResponsePrototype::forbidden() const { return statusEquals(403); }

bool ResponsePrototype::fromCache() const
{
    return m_reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool();
}

QString ResponsePrototype::text() const
{
    return m_text;
}

QString ResponsePrototype::charset() const
{
    return m_charset;
}

QJSValue ResponsePrototype::body() const
{
    return m_body;
}


QJSValue ResponsePrototype::header() const
{
    return m_header;
}

bool ResponsePrototype::gzipDecompress( QByteArray input, QByteArray &output )
{

    // Prepare output
    output.clear();

    // Is there something to do?
    if(input.length() > 0)
    {
        // Prepare inflater status
        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = 0;
        strm.next_in = Z_NULL;

        // Initialize inflater
        int ret = inflateInit2(&strm, GZIP_WINDOWS_BIT);

        if (ret != Z_OK)
            return(false);

        // Extract pointer to input data
        char *input_data = input.data();
        int input_data_left = input.length();

        // Decompress data until available
        do {
            // Determine current chunk size
            int chunk_size = qMin(GZIP_CHUNK_SIZE, input_data_left);

            // Check for termination
            if(chunk_size <= 0)
                break;

            // Set inflater references
            strm.next_in = (unsigned char*)input_data;
            strm.avail_in = chunk_size;

            // Update interval variables
            input_data += chunk_size;
            input_data_left -= chunk_size;

            // Inflate chunk and cumulate output
            do {

                // Declare vars
                char out[GZIP_CHUNK_SIZE];

                // Set inflater references
                strm.next_out = (unsigned char*)out;
                strm.avail_out = GZIP_CHUNK_SIZE;

                // Try to inflate chunk
                ret = inflate(&strm, Z_NO_FLUSH);

                switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                case Z_STREAM_ERROR:
                    // Clean-up
                    inflateEnd(&strm);

                    // Return
                    return(false);
                }

                // Determine decompressed size
                int have = (GZIP_CHUNK_SIZE - strm.avail_out);

                // Cumulate result
                if(have > 0)
                    output.append((char*)out, have);

            } while (strm.avail_out == 0);

        } while (ret != Z_STREAM_END);

        // Clean-up
        inflateEnd(&strm);

        // Return
        return (ret == Z_STREAM_END);
    }
    else
        return(true);
}



} } }
