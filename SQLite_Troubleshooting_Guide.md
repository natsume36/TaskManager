# SQLite数据库连接问题排查指南（Navicat环境）

## 1. 数据库文件路径验证

### 1.1 检查代码中的数据库路径
- 打开 `sqlrepository.cpp` 文件，检查 `initDatabase()` 函数中的数据库路径
- 当前配置路径：`D:\文档\Navicat\SQLite\Servers\TaskManagerDB\id_cache.db`

### 1.2 验证路径正确性
- **使用绝对路径**：确保使用完整的绝对路径，避免相对路径
- **路径分隔符**：Windows系统使用双反斜杠 `\\` 或正斜杠 `/`
- **文件存在性**：在代码中已经有路径验证，但可以手动验证：
  ```cpp
  qDebug() << "路径是否存在：" << QFile::exists(dbPath);
  qDebug() << "所在目录是否存在：" << QDir(QFileInfo(dbPath).dir().path()).exists();
  ```

### 1.3 与Navicat中的路径对比
- 在Navicat中右键点击数据库 → **数据库属性** → **位置**
- 确保代码中的路径与Navicat中显示的路径完全一致

## 2. 连接参数检查

### 2.1 数据库驱动
- 确保使用正确的SQLite驱动：`QSQLITE`
- 代码中检查：
  ```cpp
  database = QSqlDatabase::addDatabase("QSQLITE", "SqlRepoConnection");
  ```

### 2.2 连接名称
- 使用唯一的连接名称避免冲突
- 当前使用：`"SqlRepoConnection"`

### 2.3 数据库打开错误处理
- 检查数据库打开失败的具体错误信息：
  ```cpp
  if (!database.open()) {
      qCritical() << "数据库打开失败：" << database.lastError().text();
      return;
  }
  ```

## 3. 数据库文件权限

### 3.1 Windows文件权限
- 右键点击数据库文件 → **属性** → **安全**
- 确保当前用户有 **完全控制** 或 **读取/写入** 权限

### 3.2 文件锁定检查
- 确保没有其他程序锁定数据库文件
- Navicat可能会保持连接，尝试关闭Navicat后再运行应用程序

### 3.3 防病毒软件检查
- 临时关闭防病毒软件，检查是否阻止了数据库访问

## 4. 通过Navicat验证数据库

### 4.1 测试SQL查询
- 在Navicat中打开SQL编辑器，执行以下查询：
  ```sql
  -- 检查分类表
  SELECT * FROM category;
  -- 检查任务表
  SELECT * FROM task;
  -- 检查表结构
  PRAGMA table_info(category);
  PRAGMA table_info(task);
  ```

### 4.2 验证数据完整性
- 确保表结构与代码期望的结构一致
- 检查数据类型是否匹配（特别是datetime类型）

### 4.3 测试数据库连接
- 在Navicat中重新连接数据库，确保可以正常访问

## 5. 常见SQLite连接错误排查

### 5.1 数据库文件不存在
- 错误信息：`unable to open database file`
- 解决方案：检查路径是否正确，确保目录存在

### 5.2 权限错误
- 错误信息：`permission denied`
- 解决方案：检查文件权限，确保有写入权限

### 5.3 数据库文件被锁定
- 错误信息：`database is locked`
- 解决方案：关闭其他使用数据库的程序（如Navicat）

### 5.4 表不存在
- 错误信息：`no such table: category`
- 解决方案：检查表是否存在，确保数据库结构正确

### 5.5 SQLite版本不兼容
- 错误信息：版本相关错误
- 解决方案：确保Qt使用的SQLite驱动与Navicat使用的版本兼容

## 6. 代码级调试

### 6.1 增加详细日志
```cpp
// 在sqlrepository.cpp中增加更多调试信息
qDebug() << "数据库驱动可用：" << QSqlDatabase::isDriverAvailable("QSQLITE");
qDebug() << "数据库连接名：" << database.connectionName();
qDebug() << "数据库名称：" << database.databaseName();
qDebug() << "数据库是否打开：" << database.isOpen();
qDebug() << "最后错误：" << database.lastError().text();
```

### 6.2 测试基本查询
```cpp
// 测试数据库是否真的可用
QSqlQuery testQuery(database);
testQuery.exec("SELECT sqlite_version()");
if (testQuery.next()) {
    qDebug() << "SQLite版本：" << testQuery.value(0).toString();
}
```

### 6.3 检查连接是否真的关闭
- 在程序结束时确保关闭数据库连接：
  ```cpp
  if (database.isOpen()) {
      database.close();
  }
  ```

## 7. 项目特定排查

### 7.1 TaskManager项目检查点
- 确保 `TaskManager.pro` 中包含SQL模块：`QT += sql`
- 检查 `sqlrepository.h` 中是否正确包含头文件：`#include <QtSql>`
- 验证 `SqlRepository::initTables()` 函数是否正确创建了表结构

### 7.2 分类数据检查
- 在 `initTables()` 函数中增加娱乐分类检查：
  ```cpp
  // 检查是否缺少娱乐分类
  query.exec("SELECT COUNT(*) FROM category WHERE category_name = '娱乐'");
  if (query.next() && query.value(0).toInt() == 0) {
      addCategory("娱乐");
      qDebug() << "添加了缺失的娱乐分类";
  }
  ```

## 8. 解决方案总结

1. **确认路径一致**：确保代码中的路径与Navicat中的路径完全匹配
2. **检查文件权限**：确保应用程序有读写数据库文件的权限
3. **关闭其他连接**：运行应用前关闭Navicat等其他数据库连接
4. **详细错误日志**：增加调试输出，查看具体错误信息
5. **表结构验证**：确保数据库表结构与代码期望的一致
6. **Navicat同步**：在Navicat中执行代码中的建表语句，确保结构一致

## 9. 应急解决方案

### 9.1 创建新的测试数据库
1. 在Navicat中创建新的SQLite数据库
2. 执行 `create_database.sql` 中的SQL语句
3. 更新代码中的数据库路径到新创建的数据库
4. 测试应用程序是否能正常连接

### 9.2 使用相对路径测试
```cpp
// 测试使用相对路径
QString dbPath = QCoreApplication::applicationDirPath() + "/test.db";
database.setDatabaseName(dbPath);
```

## 10. 常见问题案例

### 案例1：路径错误
**问题**：代码中使用 `D:\TaskManager\db.sqlite`，但实际路径是 `D:\Documents\TaskManager\db.sqlite`
**解决方案**：修正路径，确保与实际位置一致

### 案例2：Navicat锁定文件
**问题**：Navicat保持连接导致数据库被锁定
**解决方案**：关闭Navicat或断开数据库连接

### 案例3：权限问题
**问题**：数据库文件位于受保护目录（如Program Files）
**解决方案**：将数据库文件移动到用户目录（如Documents）

### 案例4：表结构不匹配
**问题**：代码期望的表结构与实际数据库不同
**解决方案**：在Navicat中重新执行建表语句，确保结构一致