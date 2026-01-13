#include "fileexporter.h"
#include <QTextStream>
#include <QTextCodec>

FileExporter::FileExporter(QObject *parent) : QObject(parent)
{
}

bool FileExporter::exportToCsv(const QString &filePath, const QList<Task> &tasks, const QList<Category> &categories)
{
    QFile file(filePath);
    // 去掉 Qt::Text 模式，直接写字节
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "CSV导出失败：无法打开文件" << filePath << "，错误：" << file.errorString();
        return false;
    }

    // 兼容Qt 5/6的编码处理
    QTextCodec *codec = QTextCodec::codecForName("GBK");
    if (!codec) {
        qCritical() << "不支持GBK编码";
        file.close();
        return false;
    }

    // 先拼接CSV内容为QString
    QString csvContent;
    // 表头
    csvContent += formatCsvField("任务ID") + ","
                  + formatCsvField("标题") + ","
                  + formatCsvField("描述") + ","
                  + formatCsvField("截止时间") + ","
                  + formatCsvField("优先级") + ","
                  + formatCsvField("分类") + ","
                  + formatCsvField("完成状态") + "\n";

    // 任务数据
    foreach (const Task &task, tasks) {
        QString priorityStr = task.priority == 1 ? "低" : (task.priority == 2 ? "中" : "高");
        QString statusStr = task.isCompleted ? "已完成" : "未完成";
        QString categoryName = getCategoryNameById(task.categoryId, categories);

        csvContent += formatCsvField(QString::number(task.taskId)) + ","
                      + formatCsvField(task.title) + ","
                      + formatCsvField(task.description) + ","
                      + formatCsvField(task.deadline.toString("yyyy-MM-dd HH:mm:ss")) + ","
                      + formatCsvField(priorityStr) + ","
                      + formatCsvField(categoryName) + ","
                      + formatCsvField(statusStr) + "\n";
    }

    // 编码为GBK字节并写入
    QByteArray gbkData = codec->fromUnicode(csvContent);
    file.write(gbkData);
    file.close();
    return true;
}

QString FileExporter::getCategoryNameById(int categoryId, const QList<Category> &categories)
{
    foreach (const Category &cat, categories) {
        if (cat.categoryId == categoryId) {
            return cat.categoryName;
        }
    }
    return "未知分类";
}

QString FileExporter::formatCsvField(const QString &text)
{
    // CSV规则：字段包含逗号、双引号或换行符时，需用双引号包裹；双引号需替换为两个双引号
    QString formatted = text;
    if (formatted.contains(",") || formatted.contains("\"") || formatted.contains("\n")) {
        formatted.replace("\"", "\"\""); // 双引号转义
        formatted = "\"" + formatted + "\""; // 包裹双引号
    }
    return formatted;
}
