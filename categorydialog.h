#ifndef CATEGORYDIALOG_H
#define CATEGORYDIALOG_H

#include <QDialog>
#include <QList>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include "sqlrepository.h"

class CategoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CategoryDialog(QWidget *parent = nullptr);
    ~CategoryDialog() override;

    void setCategories(const QList<Category> &categories);
    QList<Category> getCategories() const;

signals:
    void categoryAdded(const Category &category);
    void categoryDeleted(int categoryId);

private slots:
    void onAddCategoryClicked();
    void onDeleteCategoryClicked();
    void onListWidgetItemClicked(QListWidgetItem *item);

private:
    void initUI();
    void refreshCategoryList();

    QList<Category> m_categories;
    int m_selectedCategoryId = -1;

    // UI控件
    QVBoxLayout *m_mainLayout;
    QLabel *m_titleLabel;
    QListWidget *m_categoryListWidget;
    QHBoxLayout *m_inputLayout;
    QLineEdit *m_newCategoryEdit;
    QPushButton *m_addButton;
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_deleteButton;
    QPushButton *m_closeButton;
};

#endif // CATEGORYDIALOG_H