#include "addtaskdialog.h"

AddTaskDialog::AddTaskDialog(QWidget *parent)
    : QDialog(parent)
{
    initUI();
    initComboBoxes();
}

AddTaskDialog::~AddTaskDialog()
{
}

void AddTaskDialog::initUI()
{
    // 设置窗口属性
    setWindowTitle(tr("添加/编辑任务"));
    setModal(true); // 模态对话框
    setMinimumWidth(400);

    // 网格布局（标签+输入框）
    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->setContentsMargins(20, 20, 20, 20);
    gridLayout->setSpacing(12);

    // 标题
    gridLayout->addWidget(new QLabel(tr("任务标题 *")), 0, 0);
    m_edtTitle = new QLineEdit(this);
    m_edtTitle->setPlaceholderText(tr("请输入任务标题（必填）"));
    gridLayout->addWidget(m_edtTitle, 0, 1);

    // 描述
    gridLayout->addWidget(new QLabel(tr("任务描述")), 1, 0);
    m_edtDescription = new QTextEdit(this);
    m_edtDescription->setPlaceholderText(tr("请输入任务描述（可选）"));
    m_edtDescription->setMaximumHeight(80);
    gridLayout->addWidget(m_edtDescription, 1, 1);

    // 截止时间
    gridLayout->addWidget(new QLabel(tr("截止时间 *")), 2, 0);
    m_dtDeadline = new QDateTimeEdit(QDateTime::currentDateTime().addDays(1), this);
    m_dtDeadline->setCalendarPopup(true); // 日历弹窗
    m_dtDeadline->setDisplayFormat("yyyy-MM-dd HH:mm");
    gridLayout->addWidget(m_dtDeadline, 2, 1);

    // 优先级
    gridLayout->addWidget(new QLabel(tr("优先级 *")), 3, 0);
    m_cmbPriority = new QComboBox(this);
    gridLayout->addWidget(m_cmbPriority, 3, 1);

    // 分类
    gridLayout->addWidget(new QLabel(tr("分类 *")), 4, 0);
    m_cmbCategory = new QComboBox(this);
    gridLayout->addWidget(m_cmbCategory, 4, 1);

    // 按钮区
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnOk = new QPushButton(tr("确认"), this);
    QPushButton *btnCancel = new QPushButton(tr("取消"), this);
    btnLayout->addStretch();
    btnLayout->addWidget(btnOk);
    btnLayout->addWidget(btnCancel);
    gridLayout->addLayout(btnLayout, 5, 0, 1, 2);

    // 连接按钮信号
    connect(btnOk, &QPushButton::clicked, this, [=]() {
        // 校验必填项
        if (m_edtTitle->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, tr("提示"), tr("任务标题不能为空！"));
            return;
        }
        accept(); // 确认提交
    });
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void AddTaskDialog::initComboBoxes()
{
    // 优先级选项（值对应1=低，2=中，3=高）
    m_cmbPriority->addItem(tr("低优先级"), 1);
    m_cmbPriority->addItem(tr("中优先级"), 2);
    m_cmbPriority->addItem(tr("高优先级"), 3);
    m_cmbPriority->setCurrentIndex(1); // 默认中优先级

    // 分类选项初始为空，实际运行时会从数据库加载
    m_cmbCategory->clear();
}

void AddTaskDialog::setTask(const Task &task)
{
    m_task = task;

    // 填充控件数据
    m_edtTitle->setText(task.title);
    m_edtDescription->setText(task.description);
    if (task.deadline.isValid()) {
        m_dtDeadline->setDateTime(task.deadline);
    } else {
        m_dtDeadline->setDateTime(QDateTime::currentDateTime().addDays(1));
    }

    // 设置优先级
    int priorityIndex = m_cmbPriority->findData(task.priority);
    if (priorityIndex != -1) {
        m_cmbPriority->setCurrentIndex(priorityIndex);
    }

    // 设置分类
    int categoryIndex = m_cmbCategory->findData(task.categoryId);
    if (categoryIndex != -1) {
        m_cmbCategory->setCurrentIndex(categoryIndex);
    }
}

Task AddTaskDialog::getTask() const
{
    Task task;
    task.title = m_edtTitle->text().trimmed();
    task.description = m_edtDescription->toPlainText().trimmed();
    task.deadline = m_dtDeadline->dateTime();
    task.priority = m_cmbPriority->currentData().toInt();
    task.categoryId = m_cmbCategory->currentData().toInt();
    task.isCompleted = false; // 新增/编辑时默认未完成

    return task;
}

void AddTaskDialog::setCategories(const QList<Category> &categories)
{
    // 清空现有分类
    m_cmbCategory->clear();
    
    // 添加数据库中的所有分类
    for (const Category &cat : categories) {
        m_cmbCategory->addItem(cat.categoryName, cat.categoryId);
        qDebug() << "添加分类到对话框：" << cat.categoryName << "(ID:" << cat.categoryId << ")";
    }
    
    // 如果有分类，默认选择第一个
    if (categories.size() > 0) {
        m_cmbCategory->setCurrentIndex(0);
        // 如果当前任务有分类ID，尝试选择对应分类
        if (m_task.categoryId > 0) {
            for (int i = 0; i < m_cmbCategory->count(); i++) {
                if (m_cmbCategory->itemData(i).toInt() == m_task.categoryId) {
                    m_cmbCategory->setCurrentIndex(i);
                    break;
                }
            }
        }
    }
}
