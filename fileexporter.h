#ifndef FILEEXPORTER_H
#define FILEEXPORTER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QList>
#include "sqlrepository.h"

class FileExporter : public QObject
{
    Q_OBJECT
public:
    explicit FileExporter(QObject *parent = nullptr);

    // 导出任务列表到CSV文件（支持中文编码）
    bool exportToCsv(const QString &filePath, const QList<Task> &tasks, const QList<Category> &categories);

private:
    // 根据分类ID获取分类名称
    QString getCategoryNameById(int categoryId, const QList<Category> &categories);
    // 处理CSV字段中的特殊字符（如逗号、双引号）
    QString formatCsvField(const QString &text);
};

#endif // FILEEXPORTER_H
