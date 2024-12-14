#include <QDebug>

#include "chatmessage.h"

namespace scrolling {
ChatMessage::ChatMessage(QObject *parent)
    : m_isLoaded(false), QObject{parent}
{

}
/*
ChatMessage::ChatMessage(const ChatMessage &msg)
    //: m_messagem_isLoaded(msg.m_isLoaded)
{
    qDebug() << __PRETTY_FUNCTION__;
} */

ChatMessage::ChatMessage(const QString &message, quint64 id, quint64 sequeceId,
                         const QDateTime &dateTime, QObject *parent)
    : m_message(message), m_id(id), m_sequenceId(sequeceId), m_dateTime(dateTime),
        m_isLoaded(false), QObject{parent}
{

}

ChatMessage::~ChatMessage()
{
    //qDebug() << __PRETTY_FUNCTION__;
}

QString ChatMessage::Message() const
{
    return m_message;
}

void ChatMessage::setMessage(const QString &message)
{
    if (m_message != message) {
        m_message = message;
        emit MessageChanged(m_message);
    }
}

void ChatMessage::setIsLoaded(bool isLoaded)
{
    m_isLoaded = isLoaded;
}

void ChatMessage::setId(quint64 id)
{
    m_id = id;
}

void ChatMessage::setSequenceId(quint64 sequenceId)
{
    m_sequenceId = sequenceId;
}

void ChatMessage::printDebug()
{
    qDebug() << "message: " << m_message
             << " id: " << m_id
             << " sequence: " << m_sequenceId
             << " datetime: " << m_dateTime;
}

} // namespace
