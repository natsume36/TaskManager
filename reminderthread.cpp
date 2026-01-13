#include "reminderthread.h"
#include "sqlrepository.h"

ReminderThread::ReminderThread(QObject *parent)
    : QThread(parent)
    , m_repo(SqlRepository::getInstance()) // 唯一合法的获取方式
{
    m_isRunning = false;
    m_reminderMinutes = 30; // 默认提前30分钟提醒
    m_timer = nullptr; // 不再使用定时器，设置为nullptr
}

ReminderThread::~ReminderThread()
{
    stop();
    // 不再使用定时器，无需deleteLater
}

void ReminderThread::stop()
{
    m_isRunning = false;
    requestInterruption(); // 请求线程中断
}

void ReminderThread::run()
{
    while (!isInterruptionRequested()) {
        // ✅ 通过单例引用调用接口
        QList<Task> reminderTasks = m_repo.getPendingTasksWithReminder(m_reminderMinutes);
        if (!reminderTasks.isEmpty()) {
            // 发送提醒信号（任务列表）
            emit taskReminder(reminderTasks); // 发送任务列表
            qDebug() << "提醒线程：发现" << reminderTasks.size() << "个需提醒任务";
        }
        msleep(60000); // 每分钟检查一次提醒任务
    }
}


