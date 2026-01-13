#include "sqlrepository.h"
#include "QFile"
#include "QFileInfo"
#include "qdir.h"
#include <QCoreApplication>

SqlRepository::SqlRepository(QObject *parent) : QObject(parent)
{
    qDebug() << "开始初始化SqlRepository";
    initDatabase();
    if (database.isOpen()) {
        qDebug() << "数据库已打开，开始初始化表";
        if (initTables()) {
            qDebug() << "表初始化成功";
            emit statusUpdated("数据库连接成功");
            qDebug() << "数据库连接成功";
        } else {
            qCritical() << "数据库表初始化失败";
            emit statusUpdated("数据库表初始化失败");
        }
    } else {
        QString error = "数据库连接失败：" + database.lastError().text();
        emit statusUpdated(error);
        qCritical() << error;
    }
    qDebug() << "SqlRepository初始化完成";
}

SqlRepository::~SqlRepository()
{
    // 析构函数中关闭数据库连接
    if (database.isOpen()) {
        database.close();
        qDebug() << "数据库连接已关闭，所有更改已保存";
    }
    qDebug() << "SqlRepository析构函数被调用";
}

void SqlRepository::initDatabase()
{
    // 检查SQLite驱动是否可用
    qDebug() << "=== SQLite数据库初始化开始 ===";
    qDebug() << "SQLite驱动是否可用：" << QSqlDatabase::isDriverAvailable("QSQLITE");
    
    // 获取可用的数据库驱动列表
    qDebug() << "可用的数据库驱动：" << QSqlDatabase::drivers();

    // 使用独立连接名避免冲突（与idatabase保持风格且不冲突）
    database = QSqlDatabase::addDatabase("QSQLITE", "SqlRepoConnection");
    qDebug() << "数据库连接名称：" << database.connectionName();
    // 数据库路径 - 使用构建目录下的数据库文件（确保有写入权限）
    QString dbPath = QCoreApplication::applicationDirPath() + "/TaskManager.db";
    database.setDatabaseName(dbPath);

    // 路径验证信息（保持原调试输出风格）
    qDebug() << "=== 数据库路径验证 ===";
    qDebug() << "配置的路径：" << dbPath;
    qDebug() << "路径是否存在：" << QFile::exists(dbPath);
    qDebug() << "所在目录是否存在：" << QDir(QFileInfo(dbPath).dir().path()).exists();
    
    // 检查文件权限
    QFile dbFile(dbPath);
    qDebug() << "文件是否可读：" << dbFile.isReadable();
    qDebug() << "文件是否可写：" << dbFile.isWritable();

    // 尝试打开数据库
    if (!database.open()) {
        qCritical() << "数据库打开失败：" << database.lastError().text();
        return;
    }
    qDebug() << "数据库打开成功！";
    
    // 获取数据库连接信息
    qDebug() << "数据库名称：" << database.databaseName();
    qDebug() << "数据库驱动：" << database.driverName();
    qDebug() << "数据库是否打开：" << database.isOpen();
    
    // 查询SQLite版本
    QSqlQuery versionQuery(database);
    versionQuery.exec("SELECT sqlite_version()");
    if (versionQuery.next()) {
        qDebug() << "SQLite版本：" << versionQuery.value(0).toString();
    }
    
    // 检查是否存在category表
    QSqlQuery tableQuery(database);
    tableQuery.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='category'");
    qDebug() << "category表是否存在：" << tableQuery.next();
    
    // 检查是否存在task表
    tableQuery.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='task'");
    qDebug() << "task表是否存在：" << tableQuery.next();

    // 初始化时查询任务和分类数量（保持原逻辑）
    QSqlQuery countQuery(database);

    // 查询未完成任务数
    countQuery.exec("SELECT COUNT(*) FROM task WHERE is_completed=0");
    if (countQuery.next()) {
        qDebug() << "当前db中未完成任务数：" << countQuery.value(0).toInt();
    } else {
        qDebug() << "查询未完成任务数失败：" << countQuery.lastError().text();
    }

    // 查询分类数
    countQuery.exec("SELECT COUNT(*) FROM category");
    if (countQuery.next()) {
        qDebug() << "当前db中分类数：" << countQuery.value(0).toInt();
    } else {
        qDebug() << "查询分类数失败：" << countQuery.lastError().text();
    }

    // 查询所有分类数据
    countQuery.exec("SELECT category_id, category_name FROM category ORDER BY category_id");
    qDebug() << "所有分类数据：";
    while (countQuery.next()) {
        qDebug() << "分类ID：" << countQuery.value(0).toInt() << "，名称：" << countQuery.value(1).toString();
    }

    // 查询所有任务数据
    countQuery.exec("SELECT task_id, title, is_completed FROM task LIMIT 10");
    qDebug() << "前10个任务数据：";
    while (countQuery.next()) {
        qDebug() << "任务ID：" << countQuery.value(0).toInt() << "，标题：" << countQuery.value(1).toString() << "，完成状态：" << countQuery.value(2).toInt();
    }
    qDebug() << "=== 数据库初始化完成 ===";
}

bool SqlRepository::initTables()
{
    // 创建分类表
    QString createCategoryTable = "CREATE TABLE IF NOT EXISTS category (category_id INTEGER PRIMARY KEY AUTOINCREMENT, category_name TEXT NOT NULL UNIQUE)";
    qDebug() << "创建分类表SQL:" << createCategoryTable;
    if (!executeSql(createCategoryTable)) {
        qCritical() << "创建分类表失败";
        return false;
    }
    qDebug() << "分类表创建成功";

    // 创建任务表
    QString createTaskTable = "CREATE TABLE IF NOT EXISTS task (task_id INTEGER PRIMARY KEY AUTOINCREMENT, title TEXT NOT NULL, description TEXT, deadline DATETIME NOT NULL, priority INTEGER DEFAULT 2, is_completed INTEGER DEFAULT 0, category_id INTEGER DEFAULT 1, FOREIGN KEY (category_id) REFERENCES category (category_id))";
    qDebug() << "创建任务表SQL:" << createTaskTable;
    if (!executeSql(createTaskTable)) {
        qCritical() << "创建任务表失败";
        return false;
    }
    qDebug() << "任务表创建成功";

    // 初始化默认分类
    QSqlQuery query(database);
    
    // 先检查所有必要的分类
    QStringList requiredCategories = {"工作", "学习", "生活", "娱乐"};
    
    for (const QString &categoryName : requiredCategories) {
        // 检查分类是否存在
        query.exec(QString("SELECT COUNT(*) FROM category WHERE category_name = '%1'")
                  .arg(categoryName));
        
        if (query.next() && query.value(0).toInt() == 0) {
            // 分类不存在，添加
            if (addCategory(categoryName)) {
                qDebug() << "添加分类成功：" << categoryName;
            } else {
                qCritical() << "添加分类失败：" << categoryName;
            }
        } else {
            qDebug() << "分类已存在：" << categoryName;
        }
    }
    
    // 再次查询所有分类，确认添加成功
    query.exec("SELECT category_id, category_name FROM category ORDER BY category_id");
    qDebug() << "最终分类列表：";
    while (query.next()) {
        qDebug() << "分类ID：" << query.value(0).toInt() << "，名称：" << query.value(1).toString();
    }

    return true;
}

bool SqlRepository::isConnected() const
{
    return database.isOpen();
}

QString SqlRepository::getDatabasePath() const
{
    return database.databaseName();
}

bool SqlRepository::backupDatabase(const QString &backupPath)
{
    // 检查备份路径是否为空
    if (backupPath.isEmpty()) {
        qCritical() << "备份路径不能为空";
        return false;
    }
    
    // 检查数据库是否打开
    if (!database.isOpen()) {
        qCritical() << "数据库未打开，无法备份";
        return false;
    }
    
    // 关闭数据库连接
    bool wasOpen = database.isOpen();
    database.close();
    
    // 复制数据库文件
    QString currentPath = database.databaseName();
    QFile::remove(backupPath); // 先删除已存在的备份文件
    bool success = QFile::copy(currentPath, backupPath);
    
    if (success) {
        qDebug() << "数据库备份成功，备份路径：" << backupPath;
    } else {
        qCritical() << "数据库备份失败";
    }
    
    // 重新打开数据库连接
    if (wasOpen && !database.open()) {
        qCritical() << "备份后重新打开数据库失败：" << database.lastError().text();
    }
    
    return success;
}

bool SqlRepository::restoreDatabase(const QString &backupPath)
{
    // 检查备份文件是否存在
    if (!QFile::exists(backupPath)) {
        qCritical() << "备份文件不存在：" << backupPath;
        return false;
    }
    
    // 关闭数据库连接
    bool wasOpen = database.isOpen();
    database.close();
    
    // 复制备份文件到当前数据库路径
    QString currentPath = database.databaseName();
    QFile::remove(currentPath); // 先删除当前数据库文件
    bool success = QFile::copy(backupPath, currentPath);
    
    if (success) {
        qDebug() << "数据库恢复成功，恢复路径：" << backupPath;
    } else {
        qCritical() << "数据库恢复失败";
    }
    
    // 重新打开数据库连接
    if (wasOpen && !database.open()) {
        qCritical() << "恢复后重新打开数据库失败：" << database.lastError().text();
        return false;
    }
    
    return success;
}

void SqlRepository::getTaskStatistics(int &totalTasks, int &completedTasks)
{
    // 初始化默认值
    totalTasks = 0;
    completedTasks = 0;
    
    if (!database.isOpen()) {
        qCritical() << "数据库未打开，无法获取任务统计信息";
        return;
    }
    
    QSqlQuery query(database);
    
    // 获取总任务数
    if (query.exec("SELECT COUNT(*) FROM task")) {
        if (query.next()) {
            totalTasks = query.value(0).toInt();
        }
    } else {
        qCritical() << "查询总任务数失败：" << query.lastError().text();
    }
    
    // 获取已完成任务数
    if (query.exec("SELECT COUNT(*) FROM task WHERE is_completed = 1")) {
        if (query.next()) {
            completedTasks = query.value(0).toInt();
        }
    } else {
        qCritical() << "查询已完成任务数失败：" << query.lastError().text();
    }
    
    qDebug() << "任务统计信息：总任务数=" << totalTasks << "，已完成任务数=" << completedTasks;
}

bool SqlRepository::executeSql(const QString &sql, const QVariantList &bindValues)
{
    if (!database.isOpen()) {
        qCritical() << "执行SQL失败：数据库未连接";
        return false;
    }

    QSqlQuery query(database);
    query.prepare(sql);
    foreach (const QVariant &value, bindValues) {
        query.addBindValue(value);
    }

    if (!query.exec()) {
        qCritical() << "SQL执行失败：" << sql << "，错误：" << query.lastError().text();
        return false;
    }
    return true;
}

QList<Category> SqlRepository::getAllCategories()
{
    QList<Category> categories;
    QSqlQuery query("SELECT category_id, category_name FROM category ORDER BY category_id", database);

    while (query.next()) {
        Category cat;
        cat.categoryId = query.value(0).toInt();
        cat.categoryName = query.value(1).toString();
        categories.append(cat);
    }
    return categories;
}

bool SqlRepository::addCategory(const QString &categoryName)
{
    return executeSql("INSERT INTO category (category_name) VALUES (?)", {categoryName});
}

bool SqlRepository::deleteCategory(int categoryId)
{
    // 先检查分类是否存在
    QSqlQuery existsQuery(database);
    existsQuery.prepare("SELECT COUNT(*) FROM category WHERE category_id = ?");
    existsQuery.addBindValue(categoryId);
    
    if (!existsQuery.exec() || !existsQuery.next() || existsQuery.value(0).toInt() == 0) {
        qCritical() << "删除分类失败：分类不存在，ID=" << categoryId;
        return false;
    }
    
    // 执行删除操作
    bool success = executeSql("DELETE FROM category WHERE category_id = ?", {categoryId});
    if (success) {
        qDebug() << "成功删除分类，ID=" << categoryId;
    }
    return success;
}

bool SqlRepository::isCategoryUsed(int categoryId)
{
    QSqlQuery query(database);
    query.prepare("SELECT COUNT(*) FROM task WHERE category_id = ?");
    query.addBindValue(categoryId);
    
    if (!query.exec() || !query.next()) {
        qCritical() << "检查分类使用情况失败，ID=" << categoryId;
        return false;
    }
    
    bool isUsed = query.value(0).toInt() > 0;
    qDebug() << "分类ID=" << categoryId << "，是否被使用：" << isUsed;
    return isUsed;
}

bool SqlRepository::addTask(const Task &task)
{
    QString sql = "INSERT INTO task (title, description, deadline, priority, is_completed, category_id) VALUES (?, ?, ?, ?, ?, ?)";
    return executeSql(sql, {
                               task.title,
                               task.description,
                               task.deadline.toString("yyyy-MM-dd HH:mm:ss"),
                               task.priority,
                               task.isCompleted ? 1 : 0,
                               task.categoryId
                           });
}

bool SqlRepository::editTask(const Task &task)
{
    QString sql = "UPDATE task SET title=?, description=?, deadline=?, priority=?, is_completed=?, category_id=? WHERE task_id=?";
    return executeSql(sql, {
                               task.title,
                               task.description,
                               task.deadline.toString("yyyy-MM-dd HH:mm:ss"),
                               task.priority,
                               task.isCompleted ? 1 : 0,
                               task.categoryId,
                               task.taskId
                           });
}

bool SqlRepository::deleteTask(int taskId)
{
    return executeSql("DELETE FROM task WHERE task_id=?", {taskId});
}

bool SqlRepository::markTaskCompleted(int taskId, bool isCompleted)
{
    return executeSql("UPDATE task SET is_completed=? WHERE task_id=?", {isCompleted ? 1 : 0, taskId});
}

QList<Task> SqlRepository::getAllTasks()
{
    return getTasksByFilter(-1, -1, -1); // -1表示不筛选完成状态
}

QList<Task> SqlRepository::getTasksByFilter(int priority, int categoryId, int completedFilter)
{
    QList<Task> tasks;
    QString sql = "SELECT task_id, title, description, deadline, priority, is_completed, category_id FROM task WHERE 1=1";
    QVariantList bindValues;

    if (priority != -1) {
        sql += " AND priority = ?";
        bindValues.append(priority);
        qDebug() << "筛选优先级：" << priority;
    }
    if (categoryId != -1) {
        sql += " AND category_id = ?";
        bindValues.append(categoryId);
        qDebug() << "筛选分类ID：" << categoryId;
    }

    // 根据completedFilter决定是否添加完成状态筛选
    if (completedFilter != -1) {
        sql += " AND is_completed = ?";
        bindValues.append(completedFilter == 1 ? 0 : 1); // 1=未完成(0), 2=已完成(1)
        qDebug() << "筛选完成状态：" << (completedFilter == 1 ? "未完成" : "已完成");
    } else {
        qDebug() << "不筛选完成状态";
    }

    sql += " ORDER BY deadline ASC";
    qDebug() << "执行的SQL：" << sql;
    qDebug() << "绑定参数：" << bindValues;

    QSqlQuery query(database);
    query.prepare(sql);
    foreach (const QVariant &value, bindValues) {
        query.addBindValue(value);
    }

    if (!query.exec()) {
        qCritical() << "任务查询失败：" << query.lastError().text();
        return tasks;
    }

    int taskCount = 0;
    // 直接遍历查询结果
    while (query.next()) {
        Task task;
        task.taskId = query.value(0).toInt();
        task.title = query.value(1).toString();
        task.description = query.value(2).toString();

        // 调试：输出查询到的原始数据
        qDebug() << "查询到任务：ID=" << task.taskId << "，标题=" << task.title
                 << "，完成状态=" << query.value(5).toInt();

        // 解析日期时间，兼容不同格式
        QString deadlineStr = query.value(3).toString();
        QDateTime deadline;
        if (deadlineStr.contains(":")) {
            // 尝试多种格式解析
            if (deadlineStr.count(":") == 2) {
                deadline = QDateTime::fromString(deadlineStr, "yyyy-MM-dd HH:mm:ss");
            } else {
                deadline = QDateTime::fromString(deadlineStr, "yyyy-MM-dd HH:mm");
            }
        }
        task.deadline = deadline;
        task.priority = query.value(4).toInt();
        task.isCompleted = query.value(5).toInt() == 1;
        task.categoryId = query.value(6).toInt();
        tasks.append(task);
        taskCount++;
    }
    qDebug() << "查询到的任务数：" << taskCount;

    // 如果没有查询到任务，尝试查询所有任务（不包含筛选条件）用于调试
    if (taskCount == 0) {
        QSqlQuery allQuery(database);
        allQuery.exec("SELECT COUNT(*) FROM task");
        if (allQuery.next()) {
            qDebug() << "数据库中总任务数：" << allQuery.value(0).toInt();
        }
    }
    return tasks;
}

QList<Task> SqlRepository::searchTasks(const QString &keyword)
{
    QList<Task> tasks;
    QString sql = "SELECT task_id, title, description, deadline, priority, is_completed, category_id "
                  "FROM task WHERE title LIKE ? OR description LIKE ? ORDER BY deadline ASC";
    
    QString searchPattern = "%" + keyword + "%";
    QVariantList bindValues = {searchPattern, searchPattern};
    
    qDebug() << "执行任务搜索，关键字：" << keyword;
    qDebug() << "搜索模式：" << searchPattern;
    qDebug() << "执行的SQL：" << sql;
    
    QSqlQuery query(database);
    query.prepare(sql);
    foreach (const QVariant &value, bindValues) {
        query.addBindValue(value);
    }
    
    if (!query.exec()) {
        qCritical() << "任务搜索失败：" << query.lastError().text();
        return tasks;
    }
    
    int taskCount = 0;
    while (query.next()) {
        Task task;
        task.taskId = query.value(0).toInt();
        task.title = query.value(1).toString();
        task.description = query.value(2).toString();
        
        // 解析日期时间
        QString deadlineStr = query.value(3).toString();
        QDateTime deadline;
        if (deadlineStr.contains(":")) {
            if (deadlineStr.count(":") == 2) {
                deadline = QDateTime::fromString(deadlineStr, "yyyy-MM-dd HH:mm:ss");
            } else {
                deadline = QDateTime::fromString(deadlineStr, "yyyy-MM-dd HH:mm");
            }
        }
        task.deadline = deadline;
        task.priority = query.value(4).toInt();
        task.isCompleted = query.value(5).toInt() == 1;
        task.categoryId = query.value(6).toInt();
        
        tasks.append(task);
        taskCount++;
    }
    
    qDebug() << "搜索到的任务数：" << taskCount;
    return tasks;
}

QList<Task> SqlRepository::getPendingTasksWithReminder(int reminderMinutes)
{
    QList<Task> tasks;
    QString sql = R"(
        SELECT task_id, title, description, deadline, priority, is_completed, category_id
        FROM task
        WHERE is_completed=0 AND deadline BETWEEN datetime('now') AND datetime('now', '+' || ? || ' minutes')
        ORDER BY deadline ASC
    )";

    QSqlQuery query(database);
    query.prepare(sql);
    query.addBindValue(reminderMinutes);
    query.exec();

    while (query.next()) {
        Task task;
        task.taskId = query.value(0).toInt();
        task.title = query.value(1).toString();
        task.description = query.value(2).toString();
        task.deadline = QDateTime::fromString(query.value(3).toString(), "yyyy-MM-dd HH:mm:ss");
        task.priority = query.value(4).toInt();
        task.isCompleted = false;
        task.categoryId = query.value(6).toInt();
        tasks.append(task);
    }
    return tasks;
}
