#include "mainwindow.h"
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_taskManager(new TaskManager(this))
    , m_taskModel(new TaskModel(this))
    , m_addTaskDialog(new AddTaskDialog(this))
    , m_categoryDialog(new CategoryDialog(this))
{
    initUI(); // 初始化整体UI

    // 更新任务完成率的函数
    auto updateTaskCompletionRate = [=]() {
        int totalTasks = 0;
        int completedTasks = 0;
        m_taskManager->getTaskStatistics(totalTasks, completedTasks);
        
        QString statusText;
        if (totalTasks > 0) {
            double completionRate = (double)completedTasks / totalTasks * 100;
            statusText = QString(tr("任务完成率：%1% (%2/%3) | 数据库连接正常")).arg(completionRate, 0, 'f', 1).arg(completedTasks).arg(totalTasks);
        } else {
            statusText = tr("暂无任务 | 数据库连接正常");
        }
        
        statusBar()->showMessage(statusText);
    };
    
    // 连接任务变化信号（刷新表格并更新完成率）
    connect(m_taskManager, &TaskManager::tasksChanged, this, [=](const QList<Task> &tasks) {
        m_taskModel->setTasks(tasks, m_taskManager->getCategories());
        updateTaskCompletionRate();
    });
    
    // 程序启动时更新一次完成率
    updateTaskCompletionRate();

    // 连接分类变化信号（刷新下拉框）
    connect(m_taskManager, &TaskManager::categoriesChanged, this, &MainWindow::loadCategoriesToComboBox);

    // 初始化系统托盘图标
    m_systemTrayIcon = new QSystemTrayIcon(this);
    m_systemTrayIcon->setIcon(windowIcon()); // 使用窗口图标作为托盘图标
    m_systemTrayIcon->setToolTip(tr("个人工作与任务管理系统"));
    m_systemTrayIcon->show();
    
    // 连接提醒信号（使用系统托盘通知）
    connect(m_taskManager, &TaskManager::taskReminder, this, [=](const Task &task) {
        QString title = tr("任务提醒");
        QString message = tr("⚠️ 即将到期任务\n任务名称：%1\n截止时间：%2\n请及时处理！")
                          .arg(task.title).arg(task.deadline.toString("yyyy-MM-dd HH:mm"));
        
        // 显示桌面通知
        m_systemTrayIcon->showMessage(title, message, QSystemTrayIcon::Warning, 5000);
    });

    // 连接分类对话框信号
    connect(m_categoryDialog, &CategoryDialog::categoryAdded, this, [=](const Category &category) {
        if (m_taskManager->addCategory(category.categoryName)) {
            QMessageBox::information(this, tr("成功"), tr("分类 '%1' 添加成功！").arg(category.categoryName));
        } else {
            QMessageBox::critical(this, tr("失败"), tr("分类添加失败，请检查输入或数据库连接！"));
        }
    });
    
    connect(m_categoryDialog, &CategoryDialog::categoryDeleted, this, [=](int categoryId) {
        // 检查分类是否被任务使用
        if (m_taskManager->isCategoryUsed(categoryId)) {
            QMessageBox::warning(this, tr("警告"), tr("该分类下还有任务，无法删除！"));
            return;
        }
        
        if (m_taskManager->deleteCategory(categoryId)) {
            QMessageBox::information(this, tr("成功"), tr("分类删除成功！"));
        } else {
            QMessageBox::critical(this, tr("失败"), tr("分类删除失败，请检查分类是否存在！"));
        }
    });

    // 初始化系统（加载数据、启动线程）
    m_taskManager->init();
}

MainWindow::~MainWindow()
{
    // 清理系统托盘图标
    delete m_systemTrayIcon;
}

void MainWindow::initUI()
{
    // 设置窗口基本属性
    setWindowTitle(tr("个人工作与任务管理系统"));
    setMinimumSize(800, 600); // 最小窗口尺寸

    // 创建中心部件
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *centralLayout = new QVBoxLayout(centralWidget);
    centralLayout->setContentsMargins(10, 10, 10, 10);
    centralLayout->setSpacing(8);
    setCentralWidget(centralWidget);

    // 初始化各模块
    initToolBar();
    initFilterWidget();
    initTableView();

    // 将筛选区和任务列表加入中心布局
    centralLayout->addWidget(m_filterWidget);
    centralLayout->addWidget(m_tableView);

    // 初始化状态栏
    statusBar()->showMessage(tr("系统已就绪"), 2000);
}

void MainWindow::initToolBar()
{
    m_toolBar = addToolBar(tr("主工具栏"));
    m_toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon); // 图标+文字

    // 添加按钮
    QAction *actAdd = new QAction(QIcon::fromTheme("list-add"), tr("添加任务"), this);
    QAction *actEdit = new QAction(QIcon::fromTheme("document-edit"), tr("编辑任务"), this);
    QAction *actDelete = new QAction(QIcon::fromTheme("list-remove"), tr("删除任务"), this);
    QAction *actMark = new QAction(QIcon::fromTheme("task-complete"), tr("标记完成"), this);
    QAction *actManageCategories = new QAction(QIcon::fromTheme("configure"), tr("分类管理"), this);
    QAction *actBackup = new QAction(QIcon::fromTheme("document-save-as"), tr("备份数据"), this);
    QAction *actRestore = new QAction(QIcon::fromTheme("document-open"), tr("恢复数据"), this);
    QAction *actExport = new QAction(QIcon::fromTheme("document-export"), tr("导出报表"), this);

    // 连接按钮信号
    connect(actAdd, &QAction::triggered, this, &MainWindow::onAddTaskClicked);
    connect(actEdit, &QAction::triggered, this, &MainWindow::onEditTaskClicked);
    connect(actDelete, &QAction::triggered, this, &MainWindow::onDeleteTaskClicked);
    connect(actMark, &QAction::triggered, this, &MainWindow::onMarkCompletedClicked);
    connect(actManageCategories, &QAction::triggered, this, &MainWindow::onManageCategoriesClicked);
    connect(actBackup, &QAction::triggered, this, &MainWindow::onBackupDatabaseClicked);
    connect(actRestore, &QAction::triggered, this, &MainWindow::onRestoreDatabaseClicked);
    connect(actExport, &QAction::triggered, this, &MainWindow::onExportCsvClicked);

    // 添加到工具栏
    m_toolBar->addAction(actAdd);
    m_toolBar->addAction(actEdit);
    m_toolBar->addAction(actDelete);
    m_toolBar->addAction(actMark);
    m_toolBar->addSeparator(); // 分隔线
    m_toolBar->addAction(actManageCategories);
    m_toolBar->addSeparator(); // 分隔线
    m_toolBar->addAction(actBackup);
    m_toolBar->addAction(actRestore);
    m_toolBar->addSeparator(); // 分隔线
    m_toolBar->addAction(actExport);
}

void MainWindow::initFilterWidget()
{
    m_filterWidget = new QWidget(this);
    QHBoxLayout *filterLayout = new QHBoxLayout(m_filterWidget);
    filterLayout->setContentsMargins(0, 0, 0, 0);
    filterLayout->setSpacing(10);

    // 优先级筛选
    m_cmbPriority = new QComboBox(this);
    m_cmbPriority->addItems({tr("全部优先级"), tr("低优先级"), tr("中优先级"), tr("高优先级")});
    filterLayout->addWidget(new QLabel(tr("优先级：")));
    filterLayout->addWidget(m_cmbPriority);

    // 分类筛选
    m_cmbCategory = new QComboBox(this);
    m_cmbCategory->addItem(tr("全部分类"), -1); // 后续加载数据库分类
    filterLayout->addWidget(new QLabel(tr("分类：")));
    filterLayout->addWidget(m_cmbCategory);

    // 完成状态筛选
    m_cmbCompleted = new QComboBox(this);
    m_cmbCompleted->addItems({tr("全部状态"), tr("未完成"), tr("已完成")});
    filterLayout->addWidget(new QLabel(tr("完成状态：")));
    filterLayout->addWidget(m_cmbCompleted);

    // 搜索功能
    filterLayout->addWidget(new QLabel(tr("搜索：")));
    m_leSearch = new QLineEdit(this);
    m_leSearch->setPlaceholderText(tr("按标题或描述搜索..."));
    filterLayout->addWidget(m_leSearch);
    
    m_btnSearch = new QPushButton(tr("搜索"), this);
    connect(m_btnSearch, &QPushButton::clicked, this, &MainWindow::onSearchClicked);
    filterLayout->addWidget(m_btnSearch);

    // 重置筛选按钮
    m_btnResetFilter = new QPushButton(tr("重置筛选"), this);
    connect(m_btnResetFilter, &QPushButton::clicked, this, &MainWindow::onResetFilterClicked);
    filterLayout->addStretch(); // 拉伸空白
    filterLayout->addWidget(m_btnResetFilter);

    // 连接筛选变化信号
    connect(m_cmbPriority, &QComboBox::currentIndexChanged, this, &MainWindow::onFilterChanged);
    connect(m_cmbCategory, &QComboBox::currentIndexChanged, this, &MainWindow::onFilterChanged);
    connect(m_cmbCompleted, &QComboBox::currentIndexChanged, this, &MainWindow::onFilterChanged);
}

void MainWindow::initTableView()
{
    m_tableView = new QTableView(this);
    m_tableView->setModel(m_taskModel);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows); // 整行选择
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection); // 单选
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);   // 禁止直接编辑
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 列宽自适应
    m_tableView->verticalHeader()->setVisible(false); // 隐藏行号
    m_tableView->setAlternatingRowColors(true); // 隔行变色

    // 添加双击编辑功能
    connect(m_tableView, &QTableView::doubleClicked, this, &MainWindow::onEditTaskClicked);
}

void MainWindow::loadCategoriesToComboBox()
{
    // 清空原有项（保留“全部分类”）
    m_cmbCategory->clear();
    m_cmbCategory->addItem(tr("全部分类"), -1);

    // 加载数据库中的分类
    QList<Category> categories = m_taskManager->getCategories();
    for (const Category &cat : categories) {
        m_cmbCategory->addItem(cat.categoryName, cat.categoryId);
    }
}

// ------------------------ 槽函数实现 ------------------------
void MainWindow::onAddTaskClicked()
{
    m_addTaskDialog->setTask(Task{}); // 清空对话框数据（添加新任务）
    m_addTaskDialog->setCategories(m_taskManager->getCategories()); // 设置分类列表
    if (m_addTaskDialog->exec() == QDialog::Accepted) {
        Task newTask = m_addTaskDialog->getTask();
        m_taskManager->addTask(newTask);
        loadCategoriesToComboBox(); // 新增任务后刷新分类列表
    }
}

void MainWindow::onEditTaskClicked()
{
    // 获取选中行
    QModelIndex selectedIndex = m_tableView->currentIndex();
    if (!selectedIndex.isValid()) {
        QMessageBox::warning(this, tr("提示"), tr("请先选择要编辑的任务！"));
        return;
    }

    // 优化：直接通过ID获取任务，避免遍历
    int taskId = m_taskModel->getTaskId(selectedIndex.row());
    Task task = m_taskManager->getTaskById(taskId); // 现在有了函数声明/实现，不会报错
    if (task.taskId == -1) { // 无效任务
        QMessageBox::warning(this, tr("错误"), tr("选中的任务不存在！"));
        return;
    }

    m_addTaskDialog->setTask(task); // 设置对话框数据（编辑现有任务）
    m_addTaskDialog->setCategories(m_taskManager->getCategories()); // 设置分类列表
    if (m_addTaskDialog->exec() == QDialog::Accepted) {
        Task editedTask = m_addTaskDialog->getTask();
        editedTask.taskId = taskId;
        m_taskManager->editTask(editedTask);
        loadCategoriesToComboBox(); // 编辑任务后刷新分类列表
    }
}

void MainWindow::onDeleteTaskClicked()
{
    QModelIndex selectedIndex = m_tableView->currentIndex();
    if (!selectedIndex.isValid()) {
        QMessageBox::warning(this, tr("提示"), tr("请先选择要删除的任务！"));
        return;
    }

    // 确认删除
    if (QMessageBox::question(this, tr("确认删除"), tr("是否确定删除选中的任务？"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    int taskId = m_taskModel->getTaskId(selectedIndex.row());
    m_taskManager->deleteTask(taskId);
}

void MainWindow::onMarkCompletedClicked()
{
    QModelIndex selectedIndex = m_tableView->currentIndex();
    if (!selectedIndex.isValid()) {
        QMessageBox::warning(this, tr("提示"), tr("请先选择要标记的任务！"));
        return;
    }

    int taskId = m_taskModel->getTaskId(selectedIndex.row());
    m_taskManager->markTaskCompleted(taskId, true);
    onFilterChanged(); // 标记完成后立即刷新筛选结果
}

void MainWindow::onExportCsvClicked()
{
    // 选择保存路径
    QString filePath = QFileDialog::getSaveFileName(this, tr("导出CSV报表"),
                                                    QDir::homePath(), tr("CSV文件 (*.csv)"));
    if (filePath.isEmpty()) return;

    // 确保文件后缀是.csv
    if (!filePath.endsWith(".csv", Qt::CaseInsensitive)) {
        filePath += ".csv";
    }

    // 获取当前筛选的任务列表
    int priority = m_cmbPriority->currentIndex() - 1; // 0=全部(-1),1=低(1),2=中(2),3=高(3)
    int categoryId = m_cmbCategory->currentData().toInt();
    int completedFilter = m_cmbCompleted->currentIndex(); // 0=全部,1=未完成,2=已完成
    QList<Task> filteredTasks = m_taskManager->getFilteredTasks(priority, categoryId, completedFilter);

    // 导出CSV
    bool success = m_taskManager->exportTasksToCsv(filePath, filteredTasks);
    if (success) {
        QMessageBox::information(this, tr("导出成功"), tr("报表已导出至：\n%1").arg(filePath));
    } else {
        QMessageBox::critical(this, tr("导出失败"), tr("无法导出报表，请检查文件路径权限！"));
    }
}

void MainWindow::onFilterChanged()
{
    // 解析筛选条件（使用更清晰的变量名）
    int priorityFilter = m_cmbPriority->currentIndex() - 1; // -1=全部
    int categoryFilter = m_cmbCategory->currentData().toInt(); // -1=全部
    int completedFilter = m_cmbCompleted->currentIndex(); // 0=全部,1=未完成,2=已完成

    // 按条件筛选（将筛选逻辑统一交给TaskManager处理）
    QList<Task> filteredTasks = m_taskManager->getFilteredTasks(priorityFilter, categoryFilter, completedFilter);
    m_taskModel->setTasks(filteredTasks, m_taskManager->getCategories());

    // 优化状态提示
    QString statusText;
    if (completedFilter == 0 && priorityFilter == -1 && categoryFilter == -1) {
        statusText = tr("当前显示所有任务（%1条）").arg(filteredTasks.size());
    } else {
        statusText = tr("筛选结果：%1 条任务").arg(filteredTasks.size());
    }
    statusBar()->showMessage(statusText);
}

void MainWindow::onResetFilterClicked()
{
    // 重置所有筛选下拉框
    m_cmbPriority->setCurrentIndex(0);
    m_cmbCategory->setCurrentIndex(0);
    m_cmbCompleted->setCurrentIndex(0);
    
    // 重置搜索输入框
    m_leSearch->clear();

    // 刷新为所有任务（保持筛选逻辑一致性）
    onFilterChanged();
    statusBar()->showMessage(tr("筛选已重置，显示所有任务"), 2000);
}

void MainWindow::onManageCategoriesClicked()
{
    // 加载当前分类列表到对话框
    m_categoryDialog->setCategories(m_taskManager->getCategories());
    
    // 显示对话框
    m_categoryDialog->exec();
}

void MainWindow::onBackupDatabaseClicked()
{
    // 打开文件保存对话框
    QString backupPath = QFileDialog::getSaveFileName(
        this,
        tr("备份数据库"),
        QCoreApplication::applicationDirPath() + "/TaskManager_backup.db",
        tr("SQLite数据库文件 (*.db);;所有文件 (*.*)")
    );
    
    if (backupPath.isEmpty()) {
        // 用户取消了操作
        return;
    }
    
    // 执行备份操作
    bool success = m_taskManager->backupDatabase(backupPath);
    
    if (success) {
        QMessageBox::information(this, tr("成功"), tr("数据库备份成功！\n备份路径：%1").arg(backupPath));
    } else {
        QMessageBox::critical(this, tr("失败"), tr("数据库备份失败，请检查路径权限或数据库连接！"));
    }
}

void MainWindow::onRestoreDatabaseClicked()
{
    // 打开文件打开对话框
    QString backupPath = QFileDialog::getOpenFileName(
        this,
        tr("恢复数据库"),
        QCoreApplication::applicationDirPath(),
        tr("SQLite数据库文件 (*.db);;所有文件 (*.*)")
    );
    
    if (backupPath.isEmpty()) {
        // 用户取消了操作
        return;
    }
    
    // 显示确认对话框
    int ret = QMessageBox::warning(
        this,
        tr("确认恢复"),
        tr("⚠️ 恢复操作将覆盖当前所有数据！\n您确定要继续吗？"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (ret != QMessageBox::Yes) {
        // 用户取消了恢复操作
        return;
    }
    
    // 执行恢复操作
    bool success = m_taskManager->restoreDatabase(backupPath);
    
    if (success) {
        QMessageBox::information(this, tr("成功"), tr("数据库恢复成功！数据已重新加载。"));
    } else {
        QMessageBox::critical(this, tr("失败"), tr("数据库恢复失败，请检查备份文件是否有效！"));
    }
}

void MainWindow::onSearchClicked()
{
    // 获取搜索关键字
    QString keyword = m_leSearch->text().trimmed();
    
    // 如果关键字为空，显示所有任务
    if (keyword.isEmpty()) {
        onFilterChanged();
        return;
    }
    
    // 执行搜索
    QList<Task> searchResults = m_taskManager->searchTasks(keyword);
    
    // 更新任务列表
    m_taskModel->setTasks(searchResults, m_taskManager->getCategories());
    
    // 更新状态栏信息
    statusBar()->showMessage(QString(tr("搜索结果：找到 %1 条匹配任务")).arg(searchResults.size()), 3000);
}
