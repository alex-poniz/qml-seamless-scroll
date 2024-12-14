#include "appdata.h"
#include "chatmessage.h"

#include <QVariant>
#include <QDebug>

AppData::AppData(quint64 messagesSize,
                 quint64 upBufferSize,
                 quint64 downBufferSize,
                 QObject *parent)
    : m_messagesArraySize(messagesSize),
      m_upBufferSize(upBufferSize),
      m_downBufferSize(downBufferSize),
      QObject{parent}
{
    updateMessagesArray();
}

QList<QObject*> AppData::Messages() const
{
    return m_messages;
}

void AppData::setMessages(const QList<QObject*> &messages)
{
    // No comparision of m_messages and messages
    // because of performance optimization

    m_messages = messages;
    //emit MessagesChanged(m_messages);
    emit MessagesChanged(m_messages);
}

quint64 AppData::MessagesArraySize() const
{
    return m_messagesArraySize;
}

void AppData::updateMessagesList()
{
    emit MessagesChanged(m_messages);
}

void AppData::updateMessagesArray()
{

    QDateTime currentDateTime(QDateTime::currentDateTime());
    for (uint i = 0; i < m_messagesArraySize; i++) {
        //scrolling::ChatMessage msg = new scrolling::ChatMessage(QString("Message " + QString::number(i)), i, i, currentDateTime);
        m_messages.push_back(new scrolling::ChatMessage(QString("Message " + QString::number(i) + " "), i, i, currentDateTime));
        //m_messages.push_back(QVariant(QString("test ")));
    }

}

// prepends buffered messages to top
void AppData::updateMessagesArrayUp()
{
    qDebug() << __PRETTY_FUNCTION__;
    std::lock_guard<std::mutex> guard(m_upBufferMutex);

    for (int i = m_upBuffer.size() - 1; i >= 0; i--){
        m_messages.prepend(m_upBuffer[i]);
        scrolling::ChatMessage *msg = static_cast<scrolling::ChatMessage*>(m_messages.last());
        m_messages.removeLast();
        delete msg;
    }

    // is it needed?
    m_messages.squeeze();

    emit UpBufferExpired();
}

// appends buffered messages to bottom
void AppData::updateMessagesArrayDown()
{
    qDebug() << __PRETTY_FUNCTION__;
    std::lock_guard<std::mutex> guard(m_downBufferMutex);

    for (int i = 0; i < m_downBuffer.size(); i++){
        m_messages.append(m_downBuffer[i]);
        m_messages.removeFirst();
    }

    // is it needed?
    m_messages.squeeze();

    emit DownBufferExpired();
}

void AppData::prependStubs(uint count)
{
    qDebug() << __PRETTY_FUNCTION__;
    std::lock_guard<std::mutex> guard(m_messagesListMutex);
    QDateTime currentDateTime(QDateTime::currentDateTime());
    quint64 baseSequenceId = static_cast<scrolling::ChatMessage*>(m_messages[0])->SequenceId();

    for (uint i = 0; i < count; i++){
        //scrolling::ChatMessage *msgIn =
        m_messages.prepend(new scrolling::ChatMessage("...", i, baseSequenceId - 1 - i, currentDateTime));

        scrolling::ChatMessage *msg = static_cast<scrolling::ChatMessage*>(m_messages.last());
        m_messages.removeLast();
        delete msg;
    }

    // is it needed?
    //m_messages.squeeze();

    //emit visibleMessagesRequested(0, count - 1);
    //emit prependMessagesRequested(count, baseSequenceId);
    emit MessagesChanged(m_messages);
}

void AppData::appendStubs(uint count)
{
    qDebug() << __PRETTY_FUNCTION__;
}

void AppData::requestContentUpdate(uint beginIndex, uint endIndex)
{
    qDebug() << __PRETTY_FUNCTION__;


    /*
    for (uint i = beginIndex; i < endIndex; i++) {

    } */
    emit visibleMessagesRequested(beginIndex, endIndex);
}

void AppData::requestLatestMessages(uint count)
{
    if (!count) {
        count = m_messages.size();
    }

    emit latestMessagesRequested(count);
}
