#ifndef ADDTASKDIALOG_H
#define ADDTASKDIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QPushButton>
#include <QMessageBox>
#include "sqlrepository.h"

class AddTaskDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddTaskDialog(QWidget *parent = nullptr);
    ~AddTaskDialog() override;

    // 设置要编辑的任务数据
    void setTask(const Task &task);
    // 获取用户输入的任务数据
    Task getTask() const;
    // 设置分类列表
    void setCategories(const QList<Category> &categories);

private:
    // 初始化UI
    void initUI();
    // 加载优先级/分类选项
    void initComboBoxes();

    // UI控件
    QLineEdit *m_edtTitle;          // 任务标题
    QTextEdit *m_edtDescription;    // 任务描述
    QDateTimeEdit *m_dtDeadline;    // 截止时间
    QComboBox *m_cmbPriority;       // 优先级
    QComboBox *m_cmbCategory;       // 分类

    // 临时存储任务数据
    Task m_task;
};

#endif // ADDTASKDIALOG_H
