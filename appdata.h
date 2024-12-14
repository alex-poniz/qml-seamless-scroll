#ifndef APPDATA_H
#define APPDATA_H

#include <QObject>
//#include <QVariantList>
#include <atomic>
#include <mutex>

//#include "chatmessage.h"

class DataController;

class AppData : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<QObject*> Messages READ Messages WRITE setMessages NOTIFY MessagesChanged);

    Q_PROPERTY(quint64 MessagesArraySize READ MessagesArraySize);
    Q_PROPERTY(quint64 UpBufferSize READ UpBufferSize);
    Q_PROPERTY(quint64 DownBufferSize READ DownBufferSize);
    //Q_PROPERTY(quint64 BufferLineDown READ BufferLineDown);

    friend class DataController;

public:
    // recommended
    static const quint64 MESSAGES_ARRAY_SIZE_DEFAULT = 1000;
    static const quint64 UP_BUFFER_SIZE_DEFAULT = 50;
    static const quint64 DOWN_BUFFER_SIZE_DEFAULT = 50;

    explicit AppData(quint64 messagesSize = MESSAGES_ARRAY_SIZE_DEFAULT,
                     quint64 upBufferSize = UP_BUFFER_SIZE_DEFAULT,
                     quint64 downBufferSize = DOWN_BUFFER_SIZE_DEFAULT,
                     QObject *parent = nullptr);

    QList<QObject*> Messages() const;
    void setMessages(const QList<QObject*> &messages);

    quint64 MessagesArraySize() const;
    quint64 UpBufferSize() const { return m_upBufferSize; };
    quint64 DownBufferSize() const { return m_downBufferSize; };
signals:
    void MessagesChanged(const QList<QObject*> messages);

    void UpBufferExpired();
    void DownBufferExpired();

    void visibleMessagesRequested(uint beginIndex, uint endIndex);
    void latestMessagesRequested(uint count);
    void prependMessagesRequested(uint count, quint64 baseSequenceId);

public slots:
    // sends notification to UI
    void updateMessagesList();


    void updateMessagesArray();
    void updateMessagesArrayUp();
    void updateMessagesArrayDown();

    void prependStubs(uint count);
    void appendStubs(uint count);

    void requestContentUpdate(uint beginIndex, uint endIndex);
    void requestLatestMessages(uint count);
private:
    quint64             m_messagesArraySize;
    QList<QObject*>     m_messages;

    QList<QObject*>     m_upBuffer;
    QList<QObject*>     m_downBuffer;

    std::atomic_bool    m_requestUpBufferUpdate;
    std::atomic_bool    m_requestDownBufferUpdate;

    std::mutex          m_messagesListMutex;
    std::mutex          m_upBufferMutex;
    std::mutex          m_downBufferMutex;

    quint64             m_upBufferSize;
    quint64             m_downBufferSize;
};

#endif // APPDATA_H
