#ifndef TASKMODEL_H
#define TASKMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include "sqlrepository.h"

class TaskModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    // 列定义（与UI表格列对应）
    enum TaskColumn {
        Column_Title,      // 标题
        Column_Description,// 描述
        Column_Deadline,   // 截止时间
        Column_Priority,   // 优先级
        Column_Category,   // 分类
        Column_Completed,  // 完成状态
        Column_Count       // 列数
    };

    explicit TaskModel(QObject *parent = nullptr);

    // 设置任务数据（刷新模型）
    void setTasks(const QList<Task> &tasks, const QList<Category> &categories);
    // 获取指定行的任务ID
    int getTaskId(int row) const;

    // QAbstractTableModel 纯虚函数重写
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    QList<Task> m_tasks;         // 任务数据列表
    QList<Category> m_categories;// 分类列表
    // 列名映射
    QStringList m_columnNames = {"标题", "描述", "截止时间", "优先级", "分类", "完成状态"};
};

#endif // TASKMODEL_H
