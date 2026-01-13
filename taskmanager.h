#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QObject>
#include <QList>
#include "sqlrepository.h"

class ReminderThread;
class FileExporter;

class TaskManager : public QObject
{
    Q_OBJECT
public:
    explicit TaskManager(QObject *parent = nullptr);
    ~TaskManager();

    // 初始化（启动线程、加载分类）
    void init();

    // 分类相关接口
    QList<Category> getCategories();
    bool addCategory(const QString &categoryName); // 添加分类
    bool deleteCategory(int categoryId); // 删除分类
    bool isCategoryUsed(int categoryId); // 检查分类是否被任务使用

    // 任务相关接口
    bool addTask(const Task &task);
    bool editTask(const Task &task);
    bool deleteTask(int taskId);
    bool markTaskCompleted(int taskId, bool isCompleted);
    QList<Task> getFilteredTasks(int priority, int categoryId, int completedFilter);
    QList<Task> searchTasks(const QString &keyword); // 按标题或描述模糊搜索任务

    // 报表导出接口
    bool exportTasksToCsv(const QString &filePath, const QList<Task> &tasks);
    
    // 数据库备份/恢复接口
    bool backupDatabase(const QString &backupPath); // 备份数据库
    bool restoreDatabase(const QString &backupPath); // 恢复数据库
    
    // 统计接口
    void getTaskStatistics(int &totalTasks, int &completedTasks); // 获取任务完成统计

    // 新增：声明getTaskById函数
    Task getTaskById(int taskId); // 根据ID获取单个任务

signals:
    // 任务数据变化信号（通知UI刷新）
    void tasksChanged(const QList<Task> &tasks);
    // 分类数据变化信号（通知UI刷新分类列表）
    void categoriesChanged(const QList<Category> &categories);
    // 提醒信号（转发线程的提醒）
    void taskReminder(const Task &task);
    // 状态通知信号（用于状态栏提示）
    void statusUpdated(const QString &status);

private slots:
    // 接收线程的提醒任务信号
    void onThreadReminder(const QList<Task> &tasks);

private:
    SqlRepository &m_repo;
    SqlRepository *m_sqlRepo;       // 数据库操作实例
    ReminderThread *m_reminderThread; // 提醒线程实例
    FileExporter *m_fileExporter;   // 文件导出实例
    QList<Category> m_categories;   // 缓存分类列表
};

#endif // TASKMANAGER_H
