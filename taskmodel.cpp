#include "taskmodel.h"
#include <QColor>

TaskModel::TaskModel(QObject *parent) : QAbstractTableModel(parent)
{
}

void TaskModel::setTasks(const QList<Task> &tasks, const QList<Category> &categories)
{
    beginResetModel(); // 开始重置模型（通知View数据即将变化）
    m_tasks = tasks;
    m_categories = categories;
    endResetModel(); // 结束重置（View自动刷新）
}

int TaskModel::getTaskId(int row) const
{
    if (row >= 0 && row < m_tasks.size()) {
        return m_tasks[row].taskId;
    }
    return -1;
}

int TaskModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_tasks.size(); // 行数 = 任务数
}

int TaskModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return Column_Count; // 列数 = 预定义的列数
}

QVariant TaskModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_tasks.size() || index.column() >= Column_Count) {
        return QVariant();
    }

    const Task &task = m_tasks[index.row()];
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case Column_Title:
            return task.title;
        case Column_Description:
            return task.description.isEmpty() ? "无" : task.description;
        case Column_Deadline:
            return task.deadline.toString("yyyy-MM-dd HH:mm");
        case Column_Priority:
            return task.priority == 1 ? "低" : (task.priority == 2 ? "中" : "高");
        case Column_Category:
            foreach (const Category &cat, m_categories) {
                if (cat.categoryId == task.categoryId) {
                    return cat.categoryName;
                }
            }
            return "未知分类";
        case Column_Completed:
            return task.isCompleted ? "✓" : "✗";
        default:
            return QVariant();
        }
    } else if (role == Qt::TextAlignmentRole) {
        // 文本居中对齐
        return Qt::AlignCenter;
    } else if (role == Qt::ForegroundRole) {
        // 未完成且已超时的任务，文字标红
        if (!task.isCompleted && task.deadline < QDateTime::currentDateTime()) {
            return QColor(Qt::red);
        }
    }

    return QVariant();
}

QVariant TaskModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            // 水平表头：列名
            return m_columnNames[section];
        } else {
            // 垂直表头：行号（从1开始）
            return section + 1;
        }
    } else if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }

    return QVariant();
}

Qt::ItemFlags TaskModel::flags(const QModelIndex &index) const
{
    // 禁止编辑（编辑通过对话框实现）
    return QAbstractItemModel::flags(index) & ~Qt::ItemIsEditable;
}
