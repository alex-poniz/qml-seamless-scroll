#ifndef DATACONTROLLER_H
#define DATACONTROLLER_H

#include <QSharedPointer>
#include <QSqlDatabase>

#include "appdata.h"
#include "chatmessage.h"


class DataController : public QObject
{
    Q_OBJECT
public:
    explicit DataController(const QSharedPointer<AppData> &appData, QObject *parent = nullptr);
    virtual ~DataController();

signals:
    void UiMessagesListUpdated();

public slots:
    void updateUpBuffer();
    void updateDownBuffer();

    bool createDB(quint64 count);
    void initCaching();
    void updateVisibleMessages(uint beginIndex, uint endIndex);
    void prependMessages(uint count, quint64 baseSequenceId);
    void fillLatestMessages(uint count);

private:
    // SQL database support
    bool initDB();
    bool createTable(quint64 count);
    bool queryLastElements(quint64 count);
    scrolling::ChatMessage* queryMessageBySequencId(quint64 sequenceId);

    // internal cache support
    scrolling::ChatMessage* getCachedMessageById(quint64 id);
    scrolling::ChatMessage* getCachedMessageBySequenceId(quint64 sequenceId);
    void initCacheWithLastMessages(uint count);


    QSharedPointer<AppData> m_appData;    
    QList<scrolling::ChatMessage*> m_cachedMessages;
    QSqlDatabase m_db;
    bool m_isOpen;
};

#endif // DATACONTROLLER_H
