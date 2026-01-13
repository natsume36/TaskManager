#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QPushButton>
#include <QComboBox>
#include <QTableView>
#include <QStatusBar>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QSystemTrayIcon>
#include "taskmanager.h"
#include "taskmodel.h"
#include "addtaskdialog.h"
#include "categorydialog.h"

class TaskManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    // 工具栏按钮槽函数
    void onAddTaskClicked();    // 添加任务
    void onEditTaskClicked();   // 编辑任务
    void onDeleteTaskClicked(); // 删除任务
    void onMarkCompletedClicked(); // 标记完成
    void onExportCsvClicked();  // 导出CSV
    // 筛选区槽函数
    void onFilterChanged();     // 筛选条件变化
    void onResetFilterClicked();// 重置筛选
    void onSearchClicked();     // 搜索任务
    // 分类管理槽函数
    void onManageCategoriesClicked(); // 打开分类管理对话框
    // 数据库管理槽函数
    void onBackupDatabaseClicked(); // 备份数据库
    void onRestoreDatabaseClicked(); // 恢复数据库

private:
    // 初始化UI布局
    void initUI();
    // 初始化工具栏
    void initToolBar();
    // 初始化筛选区
    void initFilterWidget();
    // 初始化任务列表
    void initTableView();
    // 加载分类到下拉框
    void loadCategoriesToComboBox();

    // 核心成员变量
    TaskManager *m_taskManager; // 数据管理核心
    TaskModel *m_taskModel;     // 任务列表模型
    AddTaskDialog *m_addTaskDialog; // 添加/编辑任务对话框
    CategoryDialog *m_categoryDialog; // 分类管理对话框

    // UI控件
    QToolBar *m_toolBar;                // 工具栏
    QWidget *m_filterWidget;            // 筛选区容器
    QComboBox *m_cmbPriority;           // 优先级筛选下拉框
    QComboBox *m_cmbCategory;           // 分类筛选下拉框
    QComboBox *m_cmbCompleted;          // 完成状态筛选下拉框
    QLineEdit *m_leSearch;              // 搜索输入框
    QPushButton *m_btnSearch;           // 搜索按钮
    QPushButton *m_btnResetFilter;      // 重置筛选按钮
    QTableView *m_tableView;            // 任务列表
    
    // 系统托盘和通知相关
    QSystemTrayIcon *m_systemTrayIcon;  // 系统托盘图标
};

#endif // MAINWINDOW_H
