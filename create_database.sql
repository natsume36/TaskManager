-- 创建分类表
CREATE TABLE IF NOT EXISTS category (
    category_id INTEGER PRIMARY KEY AUTOINCREMENT,
    category_name TEXT NOT NULL UNIQUE
);

-- 创建任务表
CREATE TABLE IF NOT EXISTS task (
    task_id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    description TEXT,
    deadline DATETIME NOT NULL,
    priority INTEGER DEFAULT 2,
    is_completed INTEGER DEFAULT 0,
    category_id INTEGER DEFAULT 1,
    FOREIGN KEY (category_id) REFERENCES category (category_id)
);

-- 插入初始分类数据
INSERT OR IGNORE INTO category (category_id, category_name) VALUES
(1, '工作'),
(2, '学习'),
(3, '生活'),
(4, '娱乐');

-- 插入示例任务数据
INSERT OR IGNORE INTO task (title, description, deadline, priority, is_completed, category_id) VALUES
('完成Qt课程设计', '实现一个任务管理系统', '2026-01-20 23:59', 3, 0, 2),
('购买生活用品', '牛奶、面包、水果', '2026-01-15 10:00', 1, 0, 3),
('编写项目文档', '整理架构图和接口说明', '2026-01-18 16:00', 2, 0, 1),
('学习SQLite', '掌握数据库基本操作和SQL语句', '2026-01-17 20:00', 2, 1, 2),
('看电影', '观看最新上映的科幻电影', '2026-01-16 19:30', 1, 0, 4);

-- 查询所有分类
SELECT * FROM category;

-- 查询所有任务
SELECT * FROM task;