#include <QDebug>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QRandomGenerator>

#include <algorithm> // std::find_if

#include "datacontroller.h"
#include "chatmessage.h"

/*
DataController::DataController(QObject *parent)
    : QThread{parent}
{

} */

DataController::DataController(const QSharedPointer<AppData> &appData, QObject *parent)
    : m_appData(appData), m_isOpen(false), QObject{parent}
{

    //this->initCacheWithLastMessages(appData->MessagesArraySize());
}

DataController::~DataController()
{
    if (m_isOpen) {
        m_db.close();
    }
}

bool DataController::initDB()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE", "CONNECTION NAME");
    m_db.setDatabaseName(":memory:");

    if(m_db.open()) {
        m_isOpen = true;
    } else {
        m_isOpen = false;
        qDebug() << "Can't create in-memory Database!";
    }

    return m_isOpen;
}

bool DataController::createTable(quint64 count)
{
    bool res = m_isOpen;
    if (res) {
        QSqlQuery query(m_db);

        res = query.exec("CREATE TABLE IF NOT EXISTS messages (id BIGINT, message TEXT, datetime TEXT, sequence BIGINT)");

        if (!res)
        {
            qDebug() << "Can't create table!";
            return res;
        }

        QDateTime currentDateTime(QDateTime::currentDateTime());

        for (quint64 i = 0; i < count; i++) {
            // generate text message of random size
            QString messageStr;
            uint c = QRandomGenerator::global()->bounded(1, 30);
            for (uint j = 0; j < c; j++) {
                messageStr.append(" Lorem Ipsum " + QString::number(1000 + i));
            }

            QString queryStr = QString("INSERT INTO messages (id, message, datetime, sequence) ") +
                               "VALUES (" + QString::number(1000 + i) +
                               ", '" + messageStr + "', '" + currentDateTime.toString() + "', '"
                               + QString::number(1000 + i) + "')";

            res = query.exec(queryStr);

            if (!res)
            {
                qDebug() << "Can't insert record!";
                return res;
            }
        }

        qDebug() << "Database filling completed!";
    }

    return res;
}

scrolling::ChatMessage* DataController::queryMessageBySequencId(quint64 sequenceId)
{
    scrolling::ChatMessage* res = nullptr;

    if (m_isOpen) {
        QString queryString("SELECT id , message , datetime , sequence FROM messages WHERE sequence = "
                            + QString::number(sequenceId));
        QSqlQuery query(m_db);

        if (query.exec(queryString))
        {
            qDebug() << "Query Executed Successfully !";

            query.next();
            QString idStr =  query.value(0).toString();
            quint64 id = static_cast<quint64>(idStr.toULongLong());

            QString messageStr = query.value(1).toString();

            QDateTime datetime = query.value(2).toDateTime();

            QString sequenceStr =  query.value(3).toString();
            quint64 sequenceId = static_cast<quint64>(sequenceStr.toULongLong());

            res = new scrolling::ChatMessage(messageStr, id, sequenceId, datetime);
            res->printDebug();

            m_cachedMessages.append(res);
        } else {
            QString errorStr = query.lastError().databaseText();
            qDebug() << "Can't Execute Query: " << errorStr;
        }
    }

    return res;
}

bool DataController::queryLastElements(quint64 count)
{
    bool res = false;
    if (m_isOpen) {
        QSqlQuery query(m_db);

        QString queryString("SELECT id , message , datetime , sequence FROM messages ORDER BY sequence DESC LIMIT " + QString::number(count));
        //QString queryString("SELECT id , message , datetime , sequence FROM messages LIMIT "
        //                    + QString::number(count));

        if (query.exec(queryString))
        {
            qDebug() << "Query Executed Successfully !";

            while(query.next())
            {
                QString idStr =  query.value(0).toString();
                quint64 id = static_cast<quint64>(idStr.toULongLong());

                QString messageStr = query.value(1).toString();

                QDateTime datetime = query.value(2).toDateTime();

                QString sequenceStr =  query.value(3).toString();
                quint64 sequenceId = static_cast<quint64>(sequenceStr.toULongLong());

                scrolling::ChatMessage* pmsg = new scrolling::ChatMessage(messageStr, id, sequenceId, datetime);
                //pmsg->printDebug();

                m_cachedMessages.prepend(pmsg);
            }
        } else {
            qDebug() << "Can't Execute Query !";
        }
    }

    return res;
}

bool DataController::createDB(quint64 count)
{
    bool res = true;

    if (this->initDB()) {
        if (!this->createTable(count)) {
            qDebug() << "Database table creation and filling failed";
            res = false;
        }
    } else {
        qDebug() << "Database initialization failed";
        res = false;
    }

    return res;
}

void DataController::initCaching()
{
    QObject::connect(m_appData.data(), &AppData::UpBufferExpired, this, &DataController::updateUpBuffer, Qt::QueuedConnection);
    QObject::connect(m_appData.data(), &AppData::DownBufferExpired, this, &DataController::updateDownBuffer, Qt::QueuedConnection);

    QObject::connect(m_appData.data(), &AppData::visibleMessagesRequested, this, &DataController::updateVisibleMessages, Qt::QueuedConnection);
    QObject::connect(m_appData.data(), &AppData::prependMessagesRequested, this, &DataController::prependMessages, Qt::QueuedConnection);

    QObject::connect(m_appData.data(), &AppData::latestMessagesRequested, this, &DataController::fillLatestMessages, Qt::QueuedConnection);

    QObject::connect(this, &DataController::UiMessagesListUpdated, m_appData.data(), &AppData::updateMessagesList, Qt::QueuedConnection);


    this->initCacheWithLastMessages(m_appData->MessagesArraySize());
}

void DataController::updateUpBuffer()
{
    qDebug() << __PRETTY_FUNCTION__;
}

void DataController::updateDownBuffer()
{
    qDebug() << __PRETTY_FUNCTION__;
}

void DataController::updateVisibleMessages(uint beginIndex, uint endIndex)
{
    std::lock_guard<std::mutex> guard(m_appData->m_messagesListMutex);
    qDebug() << __PRETTY_FUNCTION__;

    for (uint i = beginIndex; i <= endIndex; i++) {
        scrolling::ChatMessage* pmsg = static_cast<scrolling::ChatMessage*>(m_appData->Messages()[i]);

        if (pmsg->IsLoaded()) {
            continue;
        }

        scrolling::ChatMessage* cachedMsg{nullptr};

        // get message by Id
        // may be this step is not needed
        if (pmsg->Id() > 0) {
            qDebug() << "Getting message by id : " << pmsg->Id();
            cachedMsg = getCachedMessageById(pmsg->Id());
        }

        // get message by SequencId
        if (!cachedMsg) {
            qDebug() << "Getting message by sequenceId : " << pmsg->SequenceId();
            cachedMsg = getCachedMessageBySequenceId(pmsg->SequenceId());
        }

        if (cachedMsg) {
            pmsg->setMessage(cachedMsg->Message());
            pmsg->setId(cachedMsg->Id());
            pmsg->setSequenceId(cachedMsg->SequenceId());
            pmsg->setIsLoaded(true);
        }
    }

   // emit UiMessagesListUpdated();
}

void DataController::prependMessages(uint count, quint64 baseSequenceId)
{
    std::lock_guard<std::mutex> guard(m_appData->m_messagesListMutex);
    qDebug() << __PRETTY_FUNCTION__;
    //quint64 baseSequenceId = static_cast<scrolling::ChatMessage*>(m_messages[0])->SequenceId();

    for (int i = 0; i < count; i++)
    {
        // get message by SequencId


        scrolling::ChatMessage* pmsg = getCachedMessageBySequenceId(baseSequenceId - i - 1);
        //QDateTime currentDateTime = QDateTime::currentDateTime();
        //scrolling::ChatMessage* pmsg = new scrolling::ChatMessage("Not found " + currentDateTime.toString(), 0, baseSequenceId - i - 1, currentDateTime);

        if (pmsg) {
            //auto msgList = m_appData->Messages();
            m_appData->m_messages.prepend(pmsg);
            //m_appData->setMessages(msgList);

            //m_appData->Messages().prepend(pmsg);

            /*
            scrolling::ChatMessage *msg = static_cast<scrolling::ChatMessage*>(m_appData->Messages().last());
            m_appData->Messages().removeLast();
            delete msg;
*/
        }
    }

    //emit m_appData->MessagesChanged();

    // is it needed?
    //m_messages.squeeze();

    //emit MessagesChanged(m_messages);

}

void DataController::fillLatestMessages(uint count)
{
    std::lock_guard<std::mutex> guard(m_appData->m_messagesListMutex);
    uint messagesListSize = m_appData->Messages().size();
    uint cachedListSize = m_cachedMessages.size();

    if (messagesListSize >= count && cachedListSize >= count) {

        //for (uint i = messagesListSize - 1; i >= messagesListSize - count; i--) {
        for (uint i = 0; i < count; i++) {
            //QObject *tmp =  static_cast<QObject*>(m_cachedMessages.at(cachedListSize - 1 - i));
            //m_appData->Messages()[messagesListSize - 1 - i] = tmp;
            scrolling::ChatMessage *msg = static_cast<scrolling::ChatMessage*>(m_appData->Messages()[messagesListSize - 1 - i]);
            //QObject *msg = m_cachedMessages[cachedListSize - 1 - i];

            //tmp = msg;

            msg->setMessage(m_cachedMessages.at(cachedListSize - 1 - i)->Message());
            msg->setId(m_cachedMessages.at(cachedListSize - 1 - i)->Id());
            msg->setSequenceId(m_cachedMessages.at(cachedListSize - 1 - i)->SequenceId());
            msg->setIsLoaded(true);
        }
    }
}

scrolling::ChatMessage *DataController::getCachedMessageById(quint64 id)
{
    scrolling::ChatMessage *res = nullptr;

    auto findIt = std::find_if(m_cachedMessages.begin(), m_cachedMessages.end(),
                               [id](const scrolling::ChatMessage *const m){ return m->Id() == id; });

    if (findIt != m_cachedMessages.end()) {
        res = *findIt;
    }

    return res;
}

scrolling::ChatMessage *DataController::getCachedMessageBySequenceId(quint64 sequenceId)
{
    scrolling::ChatMessage *res = nullptr;

    auto findIt = std::find_if(m_cachedMessages.begin(), m_cachedMessages.end(),
                               [sequenceId](const scrolling::ChatMessage *const m){ return m->SequenceId() == sequenceId; });

    if (findIt != m_cachedMessages.end()) {
        res = *findIt;
    }

    if (!res) {
        // get message from SQL
        res = queryMessageBySequencId(sequenceId);
        if (res) {
            m_cachedMessages.append(res);
        }
    }

    if (!res) {
        QDateTime currentDateTime(QDateTime::currentDateTime());
        res = new scrolling::ChatMessage("Not found " + currentDateTime.toString(), 0, sequenceId, currentDateTime);
        //m_cachedMessages.append(res);
    }

    return res;
}

void DataController::initCacheWithLastMessages(uint count)
{
    queryLastElements(count);
    /*
    for (int i = 0; i < count; i++) {
        quint64 id = 1000 + i;

        QDateTime currentDateTime(QDateTime::currentDateTime());
        auto pmsg = new scrolling::ChatMessage("Cached " + currentDateTime.toString(), id, 0, currentDateTime);
        m_cachedMessages.append(pmsg);
    } */
}

/*
void AppData::prependStubs(uint count)
{
    qDebug() << __PRETTY_FUNCTION__;
    std::lock_guard<std::mutex> guard(m_messagesListMutex);
    QDateTime currentDateTime(QDateTime::currentDateTime());
    quint64 baseSequenceId = m_messages[0]->SequencId();

    for (uint i = 0; i < count; i++){
        scrolling::ChatMessage *msgIn =
            m_messages.prepend(new scrolling::ChatMessage("...", i, i, currentDateTime));

        scrolling::ChatMessage *msg = static_cast<scrolling::ChatMessage*>(m_messages.last());
        m_messages.removeLast();
        delete msg;
    }

    // is it needed?
    //m_messages.squeeze();

    emit MessagesChanged(m_messages);
} */
