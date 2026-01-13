#include "categorydialog.h"

CategoryDialog::CategoryDialog(QWidget *parent)
    : QDialog(parent)
{
    initUI();
}

CategoryDialog::~CategoryDialog()
{
}

void CategoryDialog::setCategories(const QList<Category> &categories)
{
    m_categories = categories;
    refreshCategoryList();
}

QList<Category> CategoryDialog::getCategories() const
{
    return m_categories;
}

void CategoryDialog::initUI()
{
    // 设置窗口属性
    setWindowTitle(tr("分类管理"));
    setMinimumSize(400, 300);
    
    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // 标题
    m_titleLabel = new QLabel(tr("任务分类管理"), this);
    m_titleLabel->setStyleSheet("font-size: 16pt; font-weight: bold;");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(m_titleLabel);
    
    // 分类列表
    m_mainLayout->addWidget(new QLabel(tr("现有分类：")));
    m_categoryListWidget = new QListWidget(this);
    m_categoryListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_mainLayout->addWidget(m_categoryListWidget);
    
    // 添加分类输入区
    m_inputLayout = new QHBoxLayout();
    m_newCategoryEdit = new QLineEdit(this);
    m_newCategoryEdit->setPlaceholderText(tr("输入新分类名称..."));
    m_inputLayout->addWidget(m_newCategoryEdit);
    
    m_addButton = new QPushButton(tr("添加"), this);
    m_inputLayout->addWidget(m_addButton);
    m_mainLayout->addLayout(m_inputLayout);
    
    // 按钮区
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->addStretch();
    
    m_deleteButton = new QPushButton(tr("删除选中"), this);
    m_deleteButton->setEnabled(false);
    m_buttonLayout->addWidget(m_deleteButton);
    
    m_closeButton = new QPushButton(tr("关闭"), this);
    m_buttonLayout->addWidget(m_closeButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
    
    // 连接信号
    connect(m_addButton, &QPushButton::clicked, this, &CategoryDialog::onAddCategoryClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &CategoryDialog::onDeleteCategoryClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &CategoryDialog::accept);
    connect(m_categoryListWidget, &QListWidget::itemClicked, this, &CategoryDialog::onListWidgetItemClicked);
    connect(m_newCategoryEdit, &QLineEdit::returnPressed, this, &CategoryDialog::onAddCategoryClicked);
}

void CategoryDialog::refreshCategoryList()
{
    m_categoryListWidget->clear();
    
    for (const Category &category : m_categories) {
        QListWidgetItem *item = new QListWidgetItem(category.categoryName);
        item->setData(Qt::UserRole, category.categoryId);
        m_categoryListWidget->addItem(item);
    }
}

void CategoryDialog::onAddCategoryClicked()
{
    QString categoryName = m_newCategoryEdit->text().trimmed();
    
    if (categoryName.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("分类名称不能为空！"));
        return;
    }
    
    // 检查分类是否已存在
    for (const Category &category : m_categories) {
        if (category.categoryName == categoryName) {
            QMessageBox::warning(this, tr("警告"), tr("该分类已存在！"));
            return;
        }
    }
    
    // 发出添加分类信号
    Category newCategory;
    newCategory.categoryName = categoryName;
    emit categoryAdded(newCategory);
    
    m_newCategoryEdit->clear();
}

void CategoryDialog::onDeleteCategoryClicked()
{
    if (m_selectedCategoryId == -1) {
        QMessageBox::warning(this, tr("警告"), tr("请先选择要删除的分类！"));
        return;
    }
    
    // 找到选中的分类名称
    QString categoryName;
    for (const Category &category : m_categories) {
        if (category.categoryId == m_selectedCategoryId) {
            categoryName = category.categoryName;
            break;
        }
    }
    
    // 确认删除
    if (QMessageBox::question(this, tr("确认删除"),
                              tr("确定要删除分类 '%1' 吗？\n注意：如果该分类下有任务，删除操作将失败。").arg(categoryName),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }
    
    // 发出删除分类信号
    emit categoryDeleted(m_selectedCategoryId);
    m_selectedCategoryId = -1;
    m_deleteButton->setEnabled(false);
}

void CategoryDialog::onListWidgetItemClicked(QListWidgetItem *item)
{
    if (item) {
        m_selectedCategoryId = item->data(Qt::UserRole).toInt();
        m_deleteButton->setEnabled(true);
    }
}