#ifndef CHATMESSAGE_H
#define CHATMESSAGE_H

#include <QObject>
#include <QDateTime>
#include <QString>

namespace scrolling {

class ChatMessage : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString Message READ Message WRITE setMessage NOTIFY MessageChanged);
    Q_PROPERTY(bool IsLoaded READ IsLoaded WRITE setIsLoaded);
    Q_PROPERTY(quint64 Id READ Id WRITE setId);
    Q_PROPERTY(quint64 SequenceId READ SequenceId WRITE setSequenceId);
public:
    ChatMessage(QObject *parent = nullptr);

    //ChatMessage(const ChatMessage &msg) = delete;

    ChatMessage(const QString &message,
                quint64 id,
                quint64 sequeceId,
                const QDateTime &dateTime,
                QObject *parent = nullptr);

    ~ChatMessage();

    QString Message() const;
    void setMessage(const QString &message);

    bool IsLoaded() const { return m_isLoaded; }
    void setIsLoaded(bool isLoaded);

    quint64 Id() const { return m_id; }
    void setId(quint64 id);

    quint64 SequenceId() const { return m_sequenceId; }
    void setSequenceId(quint64 sequenceId);

    void printDebug();
signals:
    void MessageChanged(const QString &message);
private:
    QString     m_message;
    quint64     m_id;
    quint64     m_sequenceId;
    QDateTime   m_dateTime;
    bool        m_isLoaded;
};

} //namespace

#endif // CHATMESSAGE_H
