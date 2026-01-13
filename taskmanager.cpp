#include "taskmanager.h"
#include "reminderthread.h"
#include "fileexporter.h"

TaskManager::TaskManager(QObject *parent)
    : QObject(parent)
    , m_repo(SqlRepository::getInstance())
{
    m_sqlRepo = &m_repo; // 初始化m_sqlRepo为单例指针
    m_reminderThread = new ReminderThread(this);
    m_fileExporter = new FileExporter(this);

    // 连接线程提醒信号
    connect(m_reminderThread, &ReminderThread::taskReminder, this, &TaskManager::onThreadReminder);
    // 连接数据库状态信号
    connect(m_sqlRepo, &SqlRepository::statusUpdated, this, &TaskManager::statusUpdated);
    if (!m_repo.isConnected()) {
        qCritical() << "数据库单例连接失败！";
    }
}

TaskManager::~TaskManager()
{
    // 停止线程
    if (m_reminderThread->isRunning()) {
        m_reminderThread->stop();
        m_reminderThread->wait();
    }
}

void TaskManager::init()
{
    // 加载分类列表
    m_categories = m_sqlRepo->getAllCategories();
    emit categoriesChanged(m_categories); // 发出分类变化信号，通知UI更新
    emit statusUpdated(m_sqlRepo->isConnected() ? "数据库连接正常" : "数据库连接失败");

    // 启动提醒线程（定时查询需提醒任务）
    m_reminderThread->start();
    emit statusUpdated("提醒线程已启动");

    // 初始加载所有未完成任务
    auto tasks = getFilteredTasks(-1, -1, 1); // 1表示未完成
    emit tasksChanged(tasks);
}

QList<Category> TaskManager::getCategories()
{
    return m_categories;
}

bool TaskManager::addCategory(const QString &categoryName)
{
    bool success = m_sqlRepo->addCategory(categoryName);
    if (success) {
        // 更新分类列表并发出信号
        m_categories = m_sqlRepo->getAllCategories();
        emit categoriesChanged(m_categories);
        emit statusUpdated(QString("成功添加分类：%1").arg(categoryName));
    } else {
        emit statusUpdated(QString("添加分类失败：%1").arg(categoryName));
    }
    return success;
}

bool TaskManager::deleteCategory(int categoryId)
{
    bool success = m_sqlRepo->deleteCategory(categoryId);
    if (success) {
        // 更新分类列表并发出信号
        m_categories = m_sqlRepo->getAllCategories();
        emit categoriesChanged(m_categories);
        // 重新加载任务（因为分类变化可能影响当前筛选结果）
        emit tasksChanged(getFilteredTasks(-1, -1, -1));
        emit statusUpdated(QString("成功删除分类，ID：%1").arg(categoryId));
    } else {
        emit statusUpdated(QString("删除分类失败，ID：%1").arg(categoryId));
    }
    return success;
}

bool TaskManager::isCategoryUsed(int categoryId)
{
    return m_sqlRepo->isCategoryUsed(categoryId);
}

bool TaskManager::addTask(const Task &task)
{
    bool success = m_sqlRepo->addTask(task);
    if (success) {
        emit statusUpdated("任务添加成功");
        // 刷新任务列表
        auto tasks = getFilteredTasks(-1, -1, 1); // 1表示未完成
        emit tasksChanged(tasks);
    } else {
        emit statusUpdated("任务添加失败");
    }
    return success;
}

bool TaskManager::editTask(const Task &task)
{
    bool success = m_sqlRepo->editTask(task);
    if (success) {
        emit statusUpdated("任务编辑成功");
        auto tasks = getFilteredTasks(-1, -1, 1); // 1表示未完成
        emit tasksChanged(tasks);
    } else {
        emit statusUpdated("任务编辑失败");
    }
    return success;
}

bool TaskManager::deleteTask(int taskId)
{
    bool success = m_sqlRepo->deleteTask(taskId);
    if (success) {
        emit statusUpdated("任务删除成功");
        auto tasks = getFilteredTasks(-1, -1, 1); // 1表示未完成
        emit tasksChanged(tasks);
    } else {
        emit statusUpdated("任务删除失败");
    }
    return success;
}

bool TaskManager::markTaskCompleted(int taskId, bool isCompleted)
{
    bool success = m_sqlRepo->markTaskCompleted(taskId, isCompleted);
    if (success) {
        emit statusUpdated(isCompleted ? "任务标记为已完成" : "任务标记为未完成");
        // 重新加载当前筛选条件的任务
        auto tasks = getFilteredTasks(-1, -1, 1); // 1表示未完成
        emit tasksChanged(tasks);
    } else {
        emit statusUpdated("任务状态更新失败");
    }
    return success;
}

QList<Task> TaskManager::getFilteredTasks(int priority, int categoryId, int completedFilter)
{
    // 将completedFilter转换为sqlrepository所需的格式：
    // 0=全部(-1), 1=未完成(1), 2=已完成(2)
    int sqlCompletedFilter = completedFilter == 0 ? -1 : completedFilter;
    
    // 直接调用数据库层的筛选函数
    return m_sqlRepo->getTasksByFilter(priority, categoryId, sqlCompletedFilter);
}

QList<Task> TaskManager::searchTasks(const QString &keyword)
{
    // 直接调用数据库层的搜索函数
    return m_sqlRepo->searchTasks(keyword);
}

bool TaskManager::exportTasksToCsv(const QString &filePath, const QList<Task> &tasks)
{
    bool success = m_fileExporter->exportToCsv(filePath, tasks, m_categories);
    emit statusUpdated(success ? "报表导出成功" : "报表导出失败");
    return success;
}

bool TaskManager::backupDatabase(const QString &backupPath)
{
    bool success = m_sqlRepo->backupDatabase(backupPath);
    emit statusUpdated(success ? "数据库备份成功" : "数据库备份失败");
    return success;
}

bool TaskManager::restoreDatabase(const QString &backupPath)
{
    bool success = m_sqlRepo->restoreDatabase(backupPath);
    if (success) {
        // 恢复成功后重新加载数据
        m_categories = m_sqlRepo->getAllCategories();
        emit categoriesChanged(m_categories);
        
        // 重新加载任务
        QList<Task> tasks = m_sqlRepo->getAllTasks();
        emit tasksChanged(tasks);
        
        emit statusUpdated("数据库恢复成功，数据已重新加载");
    } else {
        emit statusUpdated("数据库恢复失败");
    }
    return success;
}

void TaskManager::getTaskStatistics(int &totalTasks, int &completedTasks)
{
    m_sqlRepo->getTaskStatistics(totalTasks, completedTasks);
}

void TaskManager::onThreadReminder(const QList<Task> &tasks)
{
    // 转发每个任务的提醒信号（UI接收后弹出对话框）
    foreach (const Task &task, tasks) {
        emit taskReminder(task);
    }
}

Task TaskManager::getTaskById(int taskId)
{
    // 1. 从数据库中查询指定ID的任务
    QList<Task> allTasks = m_repo.getAllTasks(); // m_repo是引用，使用.运算符
    for (const Task &task : allTasks) {
        if (task.taskId == taskId) {
            return task; // 找到匹配ID的任务，返回
        }
    }

    // 2. 若未找到，返回空任务（taskId=-1标识无效）
    Task emptyTask;
    emptyTask.taskId = -1;
    return emptyTask;
}
