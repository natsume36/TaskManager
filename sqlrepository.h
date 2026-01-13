#ifndef SQLREPOSITORY_H
#define SQLREPOSITORY_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QVariant>
#include <QDebug>
#include <QList>

// 任务结构体（数据传输载体）
struct Task {
    int taskId;          // 任务ID（主键）
    QString title;       // 任务标题
    QString description; // 任务描述
    QDateTime deadline;  // 截止时间
    int priority;        // 优先级（1-低、2-中、3-高）
    bool isCompleted;    // 完成状态
    int categoryId;      // 分类ID（外键）
};

// 分类结构体
struct Category {
    int categoryId;      // 分类ID（主键）
    QString categoryName;// 分类名称（工作/学习/生活等）
};

class SqlRepository : public QObject
{
    Q_OBJECT
public:
    static SqlRepository &getInstance()
    {
        static SqlRepository instance;
        return instance;
    }

    explicit SqlRepository(QObject *parent = nullptr);

    // 数据库连接状态
    bool isConnected() const;

    // 分类相关接口
    QList<Category> getAllCategories(); // 获取所有分类
    bool addCategory(const QString &categoryName); // 添加分类
    bool deleteCategory(int categoryId); // 删除分类
    bool isCategoryUsed(int categoryId); // 检查分类是否被任务使用

    // 任务相关接口
    bool addTask(const Task &task); // 添加任务
    
    ~SqlRepository(); // 析构函数
    bool editTask(const Task &task); // 编辑任务
    bool deleteTask(int taskId); // 删除任务
    bool markTaskCompleted(int taskId, bool isCompleted); // 标记任务完成状态
    QList<Task> getAllTasks(); // 获取所有任务
    QList<Task> getTasksByFilter(int priority, int categoryId, int completedFilter); // 按条件筛选任务，-1表示不筛选完成状态
    QList<Task> searchTasks(const QString &keyword); // 按标题或描述模糊搜索任务
    QList<Task> getPendingTasksWithReminder(int reminderMinutes); // 获取需提醒的未完成任务
    
    // 数据库备份/恢复接口
    bool backupDatabase(const QString &backupPath); // 备份数据库
    bool restoreDatabase(const QString &backupPath); // 恢复数据库
    QString getDatabasePath() const; // 获取当前数据库路径
    
    // 统计接口
    void getTaskStatistics(int &totalTasks, int &completedTasks); // 获取任务完成统计

signals:
    void statusUpdated(const QString &status);

private:
    SqlRepository(SqlRepository const &) = delete;
    void operator=(SqlRepository const &) = delete;

    QSqlDatabase database; // 数据库连接对象

    void initDatabase(); // 初始化数据库连接（对应idatabase风格）
    bool initTables();    // 初始化数据表（拆分原initDatabase功能）
    bool executeSql(const QString &sql, const QVariantList &bindValues = QVariantList());
};

#endif // SQLREPOSITORY_H
