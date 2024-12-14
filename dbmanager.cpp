#include "dbmanager.h"
#include <QSqlQuery>

DbManager::DbManager()
    : m_isOpen(false)
{

        /*
        if(!query.exec("SELECT name , phone , address FROM employees WHERE ID = 201"))
        {
            qDebug() << "Can't Execute Query !";
            return;
        }

        qDebug() << "Query Executed Successfully !";

        while(query.next())
        {
            qDebug() << "Employee Name : " << query.value(0).toString();
            qDebug() << "Employee Phone Number : " << query.value(1).toString();
            qDebug() << "Employee Address : " << query.value(1).toString();
        }
        */
}

DbManager::~DbManager()
{
    if (m_isOpen) {
        m_db.close();
    }
}

bool DbManager::init()
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

bool DbManager::createTable(quint64 count)
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

        for (quint64 i = 0; i < count; i++) {
            QString queryStr = QString("INSERT INTO messages (id, message, datetime, sequence) ") +
                               "VALUES (" + QString::number(i) +
                               ", 'Bob_" + QString::number(i) + "', '5555-5555', '" + QString::number(i) + "')";

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

bool DbManager::queryLastElements(quint64 count)
{
    if(!m_query.exec("SELECT id , message , datetime , sequence FROM messages "))
    {
        qDebug() << "Can't Execute Query !";
        return;
    }

    qDebug() << "Query Executed Successfully !";

    while(query.next())
    {
        qDebug() << "Employee Name : " << query.value(0).toString();
        qDebug() << "Employee Phone Number : " << query.value(1).toString();
        qDebug() << "Employee Address : " << query.value(1).toString();
    }
}
