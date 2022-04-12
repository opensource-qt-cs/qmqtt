/*
 * example.cpp - qmqtt example
 *
 * Copyright (c) 2013  Ery Lee <ery.lee at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of mqttc nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <qmqtt.h>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>

const QHostAddress EXAMPLE_HOST = QHostAddress::LocalHost;
//const QHostAddress EXAMPLE_HOST = QHostAddress("platpush.xxxxx.com");
const quint16 EXAMPLE_PORT = 1883;
const QString EXAMPLE_TOPIC = "ewogICAgImhlYWRlciI6IHsKICAgICAgICAiYWNjZXNzX3Rva2VuIjogImRjX2NuMS4xODAwMzc1ZjBiNi1iY2NhYTdiYjMyODQ0YTA1YTgyNDYzOTFkNTFiZGZiNSIsCiAgICAgICAgImFwcCI6ICJnbG9iYWwiLAogICAgICAgICJjaCI6ICJxdF93ZWJ1bGwiLAogICAgICAgICJjb250ZW50LXR5cGUiOiAiYXBwbGljYXRpb24vanNvbiIsCiAgICAgICAgImRldmljZS10eXBlIjogIk1hYyIsCiAgICAgICAgImRpZCI6ICI0YTNmYzJjZjdkYzJlMjZjZTZjMjgwMTExMjNlMzE0ZCIsCiAgICAgICAgImhsIjogInpoIiwKICAgICAgICAibG9jYWxlIjogImVuX1VTIiwKICAgICAgICAib2RpZCI6ICIzNGM2MjVmYzkxNjAyOGUwYmQ0YjUwZWM1ZjEyNjZiYiIsCiAgICAgICAgIm9zIjogIm1hYyIsCiAgICAgICAgIm9zdiI6ICIxMS42IiwKICAgICAgICAicGxhdGZvcm0iOiAicXQiLAogICAgICAgICJyZXFpZCI6ICI1OGE5MDAwNy1lNzU1LTRkNGYtOTE0NS1hODBlNjA3NWFkNGYiLAogICAgICAgICJ0X3RpbWUiOiAiMTY0OTc3MDQ4NzU1MyIsCiAgICAgICAgInRfdG9rZW4iOiAiIiwKICAgICAgICAidHoiOiAiQXNpYS9TaGFuZ2hhaSIsCiAgICAgICAgInZlciI6ICI1LjkuMCIsCiAgICAgICAgInZlcl9jb2RlIjogIiIKICAgIH0KfQo=";

const QString host ="platpush.xxxxx.com";

class Publisher : public QMQTT::Client
{
    Q_OBJECT
public:
    explicit Publisher(const QHostAddress& host = EXAMPLE_HOST,
                       const quint16 port = EXAMPLE_PORT,
                       QObject* parent = nullptr)
        : QMQTT::Client(host, port, parent)
        , _number(0)
    {
        connect(this, &Publisher::connected, this, &Publisher::onConnected);
        connect(&_timer, &QTimer::timeout, this, &Publisher::onTimeout);
        connect(this, &Publisher::disconnected, this, &Publisher::onDisconnected);
    }
    virtual ~Publisher() {}

    QTimer _timer;
    quint16 _number;

public slots:
    void onConnected()
    {
        subscribe(EXAMPLE_TOPIC, 0);
        _timer.start(1000);
    }

    void onTimeout()
    {
        QMQTT::Message message(_number, EXAMPLE_TOPIC,
                               QString("Number is %1").arg(_number).toUtf8());
        publish(message);
        _number++;
        if(_number >= 10)
        {
            _timer.stop();
            disconnectFromHost();
        }
    }

    void onDisconnected()
    {
        QTimer::singleShot(0, qApp, &QCoreApplication::quit);
    }
};

class Subscriber : public QMQTT::Client
{
    Q_OBJECT
public:
    explicit Subscriber(const QHostAddress& host = EXAMPLE_HOST,
                        const quint16 port = EXAMPLE_PORT,
                        QObject* parent = nullptr)
        : QMQTT::Client(host, port, parent)
        , _qout(stdout)
    {
        connect(this, &Subscriber::connected, this, &Subscriber::onConnected);
        connect(this, &Subscriber::subscribed, this, &Subscriber::onSubscribed);
        connect(this, &Subscriber::received, this, &Subscriber::onReceived);
        connect(this, &Subscriber::error, this, [=](const QMQTT::ClientError error){
            qDebug() << "ClientError" << error;
        });
    }
    virtual ~Subscriber() {}

    QTextStream _qout;

public slots:
    void onConnected()
    {
        qDebug() << "connected" << endl;
        subscribe(EXAMPLE_TOPIC, 0);
    }

    void onSubscribed(const QString& topic)
    {
        qDebug() << "subscribed " << topic << endl;
    }

    void onReceived(const QMQTT::Message& message)
    {
        qDebug() << "publish received: \"" << QString::fromUtf8(message.payload())
                 << "\"" << endl;
    }
};

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
        Subscriber subscriber;
        subscriber.setUsername("test");
        subscriber.setPassword("test");
        subscriber.setVersion(QMQTT::V3_1_1);
        subscriber.setHostName("broker.hivemq.com");
        subscriber.setPort(1883);
        subscriber.setCleanSession(true);
        QSslConfiguration config = QSslConfiguration::defaultConfiguration();
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        config.setProtocol(QSsl::TlsV1_2);
        subscriber.setSslConfiguration(config);
//        subscriber.connectToHost();

    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfig.setProtocol(QSsl::TlsV1_2);
    // Add custom SSL options here (for example extra certificates)
    QMQTT::Client *client = new QMQTT::Client(host, EXAMPLE_PORT, sslConfig);
    //    client->setClientId("clientId");
    client->cleanSession();
    client->setUsername("test");
    client->setPassword("test");
    client->setVersion(QMQTT::V3_1_1);
    // Optionally, set ssl errors you want to ignore. Be careful, because this may weaken security.
    // See QSslSocket::ignoreSslErrors(const QList<QSslError> &) for more information.
    //    client->ignoreSslErrors(<list of expected ssl errors>)
    client->connectToHost();
    // Here's another option to suppress SSL errors (again, be careful)
    QObject::connect(client, &QMQTT::Client::sslErrors, [&](const QList<QSslError> &errors) {
        // Investigate the errors here, if you find no serious problems, call ignoreSslErrors()
        // to continue connecting.
        qDebug() << "sslErrors========: "<< errors;
        client->ignoreSslErrors();
    });
    QObject::connect(client, &QMQTT::Client::error, [&](const QMQTT::ClientError errors) {
        // Investigate the errors here, if you find no serious problems, call ignoreSslErrors()
        // to continue connecting.
        qDebug() << "sslErrors=====333===: "<< errors;
        client->ignoreSslErrors();
    });
    QObject::connect(client, &QMQTT::Client::connected, [&]() {
        // Investigate the errors here, if you find no serious problems, call ignoreSslErrors()
        // to continue connecting.
        qDebug() << "connected=====connected===:"<<QDateTime::currentDateTime();
        client->subscribe(EXAMPLE_TOPIC, 0);

    });
    QObject::connect(client, &QMQTT::Client::subscribed, [&](const QString& topic, const quint8 qos) {
        // Investigate the errors here, if you find no serious problems, call ignoreSslErrors()
        // to continue connecting.
        qDebug() << "subscribed=====subscribed===:"<<topic<<QDateTime::currentDateTime();
    });
    qDebug() << "version========: "<< QDateTime::currentDateTime();
    return app.exec();
}

#include "example.moc"
