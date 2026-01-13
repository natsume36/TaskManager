#ifndef REMINDERTHREAD_H
#define REMINDERTHREAD_H

#include <QThread>
#include <QTimer>
#include <QList>
#include "sqlrepository.h"

class ReminderThread : public QThread
{
    Q_OBJECT
public:
    explicit ReminderThread(QObject *parent = nullptr);
    ~ReminderThread();

    void stop(); // 停止线程

signals:
    // 发送需提醒的任务列表
    void taskReminder(const QList<Task> &tasks);

protected:
    void run() override; // 线程执行入口

private:
    SqlRepository &m_repo;
    QTimer *m_timer;      // 定时触发器（已废弃）
    bool m_isRunning;     // 线程运行标志
    int m_reminderMinutes; // 提醒提前时间（默认30分钟）
};

#endif // REMINDERTHREAD_H
