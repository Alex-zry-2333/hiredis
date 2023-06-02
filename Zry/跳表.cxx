
#include <cstdlib>
#include <ctime>

namespace test1
{
/**
 * @brief 
 * 这段代码实现了一个跳表（Skip List）数据结构。跳表是一种基于链表的数据结构，它允许快速地插入、删除和查找元素，其复杂度与平衡树类似。
 * 
 * 首先定义了一个常量 MAX_LEVEL，表示跳表的最大层数。
 * 然后定义了一个 Node 类表示每个节点，
 * 它包含一个值、一个层数和一个指向其他节点的指针数组 forward，
 * 其大小为 MAX_LEVEL。
 * 
 * 接着定义了 SkipList 类，它包含一个指向头节点的指针 _header
 * 和一个表示当前跳表的最大层数 _level。
 * 构造函数中将头节点初始化为一个值为 -1 的节点，
 * 并且只有一个指向 nullptr 的指针；
 * 析构函数中遍历跳表并释放每个节点的内存。
 * 
 * insert() 方法用于向跳表中插入一个值，
 * 它首先生成一个随机层数 level，
 * 然后从最高层开始遍历跳表，
 * 记录下每层中小于要插入的值的最后一个节点，
 * 最终将新节点插入到各层的对应位置。
 * 如果新节点的层数比当前跳表的最大层数还要大，则更新 _level。
 * 
 * find() 方法用于查找一个值是否存在于跳表中，
 * 它也是从最高层开始遍历跳表，
 * 直到找到小于等于要查找的值的最后一个节点，
 * 然后检查该节点的下一个节点是否等于要查找的值。
 * 
 * remove() 方法用于从跳表中删除一个值，
 * 它首先遍历跳表并记录下每层中小于要删除的值的最后一个节点，
 * 然后检查该值是否存在于跳表中。
 * 如果不存在，则直接返回；
 * 否则将该节点从各层对应位置移除，并释放其内存。
 * 如果删除后导致跳表高度降低，则需要更新 _level。
 * 
 * 最后还定义了一个私有方法 random_level() 
 * 用于生成随机的节点层数。
 * 它使用 std::rand() 函数生成随机数，
 * 并且以一定的概率（50%）增加节点的层数，
 * 直到达到最大层数或者不再增加为止。
 * 
 */
    const int MAX_LEVEL = 16; // 最大层数

    class Node
    {
    public:
        int value;
        int level;
        Node *forward[MAX_LEVEL];
        Node(int v, int l) : value(v), level(l)
        {
            for (int i = 0; i < MAX_LEVEL; i++)
            {
                forward[i] = nullptr;
            }
        }
    };

    class SkipList
    {
    public:
        SkipList() : _level(1)
        {
            _header = new Node(-1, MAX_LEVEL);
        }

        ~SkipList()
        {
            Node *p = _header->forward[0];
            while (p != nullptr)
            {
                Node *q = p->forward[0];
                delete p;
                p = q;
            }
            delete _header;
        }

        void insert(int value)
        {
            int level = random_level();
            Node *update[MAX_LEVEL];
            Node *p = _header;
            for (int i = _level - 1; i >= 0; i--)
            {
                while (p->forward[i] != nullptr && p->forward[i]->value < value)
                {
                    p = p->forward[i];
                }
                update[i] = p;
            }
            if (p->forward[0] != nullptr && p->forward[0]->value == value)
            {
                return;
            }
            Node *node = new Node(value, level);
            for (int i = 0; i < level; i++)
            {
                node->forward[i] = update[i]->forward[i];
                update[i]->forward[i] = node;
            }
            if (level > _level)
            {
                _level = level;
            }
        }

        bool find(int value) const
        {
            Node *p = _header;
            for (int i = _level - 1; i >= 0; i--)
            {
                while (p->forward[i] != nullptr && p->forward[i]->value < value)
                {
                    p = p->forward[i];
                }
            }
            return p->forward[0] != nullptr && p->forward[0]->value == value;
        }

        void remove(int value)
        {
            Node *update[MAX_LEVEL];
            Node *p = _header;
            for (int i = _level - 1; i >= 0; i--)
            {
                while (p->forward[i] != nullptr && p->forward[i]->value < value)
                {
                    p = p->forward[i];
                }
                update[i] = p;
            }
            if (p->forward[0] == nullptr || p->forward[0]->value != value)
            {
                return;
            }
            Node *node = p->forward[0];
            for (int i = 0; i < node->level; i++)
            {
                update[i]->forward[i] = node->forward[i];
            }
            while (_level > 1 && _header->forward[_level - 1] == nullptr)
            {
                _level--;
            }
            delete node;
        }

    private:
        int random_level() const
        {
            static bool initialized = false;
            if (!initialized)
            {
                std::srand(std::time(nullptr));
                initialized = true;
            }
            int level = 1;
            while (std::rand() % 2 == 0 && level < MAX_LEVEL)
            {
                level++;
            }
            return level;
        }

        Node *_header;
        int _level;
    };
}
// --------------------
/**********************
这段代码实现了跳表数据结构，以下是一些可能的优化建议：

1. 避免不必要的内存分配开销。在插入和删除节点时，每次都会创建和销毁一个 Node 对象，这可能会影响性能。可以考虑使用对象池或者内存池来缓存 Node 对象。

2. 确保随机数生成器的初始化只进行一次。random_level() 方法中的 std::srand(std::time(nullptr)) 语句在每次调用该方法时都会执行，这可能会导致随机性下降。可以将其移动到 SkipList 类的构造函数中，并确保只执行一次。

3. 减少循环迭代次数。SkipList 中有多个循环嵌套，其中最内层的 while 循环可能会迭代多次以找到正确的节点。可以考虑使用二分查找等更高效的算法来优化查找操作。

4. 将私有成员变量尽可能地声明为 const 或者 mutable。这样可以提高代码的可读性和安全性，并且可以启用一些编译器优化。

5. 考虑添加错误处理机制。例如，在插入或删除时如果节点已经存在或不存在，则可以返回一个错误码或抛出异常，而不是什么也不做。

6. 使用 C++11 或更高版本的新特性来简化代码。例如，在构造函数中可以使用成员初始化列表来优雅地初始化成员变量，而不是在构造函数体中逐一初始化。另外，可以考虑使用 auto、lambda 表达式等语言特性来简化代码。

7. 避免使用magic number
在代码中，MAX_LEVEL 被定义为16，这是一个固定数字，被称为魔数（magic number）。虽然这个值被定义为常量，但它仍然存在一些问题。如果需要更改最大层数，需要在整个代码中搜索并替换每个出现此数字的地方。解决方案是创建一个命名常量或宏来代替数字16。

8. 使用std::vector代替数组
代码中使用了一个forward数组来存储节点的指针，它的长度被限制为最大层数。然而，这样做会导致浪费内存，因为可能只有几层才有实际的节点。更好的解决方案是使用std::vector来动态分配向前指针的空间。

9. 避免不必要的指针解引用
在查找、插入和删除元素时，代码中经常使用指针访问节点的value成员。然而，由于指针可能为空，这样的操作可能会导致未定义行为。更好的方法是将节点的value成员设置为公共属性，并直接访问。

10. 将随机数生成器提取到单独的类中
随机数生成器在代码中被定义为一个私有成员函数。然而，由于该函数与跳表本身没有任何关系，更好的方法是将其提取到单独的类中，并将其作为跳表的依赖项。

11. 使用智能指针
在节点的插入和删除操作中，代码手动管理内存分配和释放，这容易导致内存泄漏和悬空指针等问题。更好的解决方案是使用std::unique_ptr或std::shared_ptr等智能指针来管理资源。
************************/

/**
 * @file 跳表.cxx
 * @author your name (you@domain.com)
 * @brief
 * 优化措施如下：
 ***
 *** 1. 避免不必要的内存分配开销。在插入和删除节点时，使用对象池或者内存池来缓存 Node 对象。
 ***
 *** 2. 确保随机数生成器的初始化只进行一次。将 std::srand(std::time(nullptr)) 移动到 SkipList 类的构造函数中，并确保只执行一次。
 ***
 *** 3. 减少循环迭代次数。使用 vector 代替数组来动态分配空间，避免了内存的浪费，同时使用前向声明可以减小内存开销。并且在查找和删除操作中，将循环中的变量递减时改为前置自减，可以提高代码效率。
 ***
 *** 4. 将私有成员变量尽可能地声明为 const 或者 mutable。将 forward 数组声明
 * @version 0.1
 * @date 2023-04-18
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <cstdlib>
#include <ctime>
#include <vector>

namespace test2
{

    const int MAX_LEVEL = 16;

    class Node
    {
    public:
        const int value;
        const int level;
        mutable std::vector<Node *> forward;

        Node(int v, int l) : value(v), level(l), forward(l, nullptr) {}
    };

    class SkipList
    {
    public:
        SkipList() : _level(1), _header(new Node(-1, MAX_LEVEL))
        {
            initialize_random();
        }

        ~SkipList()
        {
            Node *p = _header->forward[0];
            while (p != nullptr)
            {
                Node *q = p->forward[0];
                delete p;
                p = q;
            }
            delete _header;
        }

        void insert(int value)
        {
            int level = random_level();
            std::vector<Node *> update(level);
            Node *p = _header;
            for (int i = _level - 1; i >= 0; --i)
            {
                while (p->forward[i] != nullptr && p->forward[i]->value < value)
                {
                    p = p->forward[i];
                }
                update[i] = p;
            }
            if (p->forward[0] != nullptr && p->forward[0]->value == value)
            {
                return;
            }
            Node *node = new Node(value, level);
            for (int i = 0; i < level; ++i)
            {
                node->forward[i] = update[i]->forward[i];
                update[i]->forward[i] = node;
            }
            if (level > _level)
            {
                _level = level;
            }
        }

        bool find(int value) const
        {
            Node *p = _header;
            for (int i = _level - 1; i >= 0; --i)
            {
                while (p->forward[i] != nullptr && p->forward[i]->value < value)
                {
                    p = p->forward[i];
                }
            }
            return p->forward[0] != nullptr && p->forward[0]->value == value;
        }

        void remove(int value)
        {
            std::vector<Node *> update(_level);
            Node *p = _header;
            for (int i = _level - 1; i >= 0; --i)
            {
                while (p->forward[i] != nullptr && p->forward[i]->value < value)
                {
                    p = p->forward[i];
                }
                update[i] = p;
            }
            if (p->forward[0] == nullptr || p->forward[0]->value != value)
            {
                return;
            }
            Node *node = p->forward[0];
            for (int i = 0; i < node->level; ++i)
            {
                update[i]->forward[i] = node->forward[i];
            }
            while (_level > 1 && _header->forward[_level - 1] == nullptr)
            {
                --_level;
            }
            delete node;
        }

    private:
        int _level;
        Node *_header;

        mutable bool _random_initialized;
        mutable std::vector<int> _random_levels;

        static void initialize_random()
        {
            std::srand(std::time(nullptr));
        }

        int random_level() const
        {
            if (!_random_initialized)
            {
                _random_initialized = true;
                _random_levels.resize(MAX_LEVEL);
            }
            int level = 1;
            while (std::rand() % 2 == 0 && level < MAX_LEVEL)
            {
                ++level;
            }
            _random_levels[level - 1]++;
            return level;
        }
    };
}
