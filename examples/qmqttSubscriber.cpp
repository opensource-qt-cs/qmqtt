#include <QString>
#include <qmqtt_client.h>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QObject>

using QMQTT::Client;
using QMQTT::Will;
using QMQTT::Message;


#include <iostream>
using namespace std;


class Logger : public QObject {
    public:
        Logger() : QObject(0) {
        }
        ~Logger() {
        }
    public slots:
        void showMqttData(const QMQTT::Message &message) {
            QString data(message.payload());
            cout << "received data: " << data.toStdString() << endl;
        }
        void showSubscribed() {
            cout << "subscribed" << endl;
        }
        void showConnected() {
            cout << "connected!" << endl;
        }
        void showDisConnected() {
            cout << "disconnected!" << endl;
        }
};


class MyClient : public Client {
    public:
    MyClient(const QString & host, quint32 port) : Client(host, port) {
    }
    void setTopic(const QString & topic) {
        subTopic = topic;
    }
    void setQos(const QString & qos) {
        subQos = qos.toUInt();
    }
    public slots:
    void subscribeTo() {
        subscribe(subTopic, subQos);
    }

    private:
    QString subTopic;
    quint8 subQos;
};


int main(int argc, char ** argv)
{
    QCoreApplication a(argc, argv);
    QCommandLineParser parser;
    parser.addPositionalArgument("topic", QCoreApplication::translate("main", "Topic to subscribe"));

    QCommandLineOption hostOption("host",
            QCoreApplication::translate("host", "The MQTT host to connect, localhost used if not defined."),
            "hostOption", "localhost");
    QCommandLineOption qosOption("qos",
            QCoreApplication::translate("qos", "Quality of Service level, 0 used if not defined."),
            "qosOption", "0");
    QCommandLineOption portOption("port",
            QCoreApplication::translate("port", "The MQTT port to connect, 1883 used if not defined."),
            "portOption", "1883");
    parser.process(a);
    QStringList args = parser.positionalArguments();
    if (args.size() < 2) {
        parser.showHelp(0);
        return 0;
    }
    QString host = parser.value("host");
    quint32 port = parser.value("port").toUInt();
    QString qos = parser.value("qos");
    QString topic = args.at(0);

    Logger s;
    MyClient c(host, port);
    c.setTopic(topic);
    c.setQos(qos);
    QObject::connect(&c, &MyClient::connected, &s, &Logger::showConnected);
    QObject::connect(&c, &MyClient::connected, &c, &MyClient::subscribeTo);
    QObject::connect(&c, &MyClient::disconnected, &s, &Logger::showDisConnected);
    QObject::connect(&c, &MyClient::subscribed, &s, &Logger::showSubscribed);
    QObject::connect(&c, &MyClient::received, &s, &Logger::showMqttData);
    
    c.connect();

    a.exec();

    c.unsubscribed(topic);
    c.disconnect();
}
