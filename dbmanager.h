#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QSqlDatabase>

class DbManager
{
public:
    DbManager();
    ~DbManager();

    bool init();
    bool createTable(quint64 count);

    bool queryLastElements(quint64 count);
private:
    QSqlDatabase m_db;
    QSqlQuery m_query;
    bool m_isOpen;
};

#endif // DBMANAGER_H
