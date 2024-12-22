#include <iostream>
#include <vector>
#include <queue>
#include <iomanip>
using namespace std;

// B樹節點類別
class BTreeNode {
public:
    int* keys;       // 存放鍵值的陣列
    int t;           // 最小度數 (定義了每個節點鍵的範圍)
    BTreeNode** C;   // 子節點指標陣列
    int n;           // 當前節點的鍵數
    bool leaf;       // 若為葉節點則為真

    BTreeNode(int _t, bool _leaf);

    void insertNonFull(int k); // 插入鍵到非滿的節點
    void splitChild(int i, BTreeNode* y); // 分割子節點
    void traverse(int depth = 0); // 遍歷樹
    BTreeNode* search(int k); // 搜尋某個鍵
    int findKey(int k); // 查找鍵的位置
    void remove(int k); // 刪除鍵
    void removeFromLeaf(int idx); // 從葉節點刪除鍵
    void removeFromNonLeaf(int idx); // 從非葉節點刪除鍵
    int getPred(int idx); // 獲取前驅鍵
    int getSucc(int idx); // 獲取後繼鍵
    void fill(int idx); // 填充節點
    void borrowFromPrev(int idx); // 從前一個兄弟節點借鍵
    void borrowFromNext(int idx); // 從下一個兄弟節點借鍵
    void merge(int idx); // 合併節點

    friend class BTree; // 讓BTree類別能夠訪問BTreeNode的私有成員
};

// BTreeNode建構子
BTreeNode::BTreeNode(int _t, bool _leaf) {
    t = _t;
    leaf = _leaf;
    keys = new int[2 * t - 1];  // 節點可以最多有2*t - 1個鍵
    C = new BTreeNode * [2 * t]; // 節點最多有2*t個子節點
    n = 0; // 初始時節點中沒有任何鍵
}

// 遍歷整棵樹
void BTreeNode::traverse(int depth) {
    // 遍歷當前節點的所有鍵
    for (int i = 0; i < n; i++) {
        if (!leaf) // 如果不是葉節點，遞歸遍歷子節點
            C[i]->traverse(depth + 1);
        cout << string(depth * 4, ' ') << keys[i] << "\n"; // 顯示鍵值，根據層數加縮排
    }
    if (!leaf) // 遍歷最後一個子節點
        C[n]->traverse(depth + 1);
}

// 搜尋子樹中是否存在指定的鍵
BTreeNode* BTreeNode::search(int k) {
    int i = 0;
    // 找到第1個大於或等於k的鍵
    while (i < n && k > keys[i])
        i++;

    // 如果鍵等於當前節點的鍵，返回當前節點
    if (keys[i] == k)
        return this;

    // 如果是葉節點，表示沒有找到
    if (leaf)
        return nullptr;

    // 遞歸搜尋對應子節點
    return C[i]->search(k);
}

// 在非滿的節點中插入鍵
void BTreeNode::insertNonFull(int k) {
    int i = n - 1;

    if (leaf) {  // 如果是葉節點，直接插入鍵
        // 將大於k的鍵移動一位，為k騰出位置
        while (i >= 0 && keys[i] > k) {
            keys[i + 1] = keys[i];
            i--;
        }
        keys[i + 1] = k;  // 插入鍵
        n++;  // 鍵數加1
    }
    else { // 如果不是葉節點，找到對應的子節點繼續插入
        // 找到插入位置
        while (i >= 0 && keys[i] > k)
            i--;

        // 如果該子節點已滿，需要分裂
        if (C[i + 1]->n == 2 * t - 1) {
            splitChild(i + 1, C[i + 1]);

            // 根據分裂後的鍵值決定是否插入到右側子節點
            if (keys[i + 1] < k)
                i++;
        }
        C[i + 1]->insertNonFull(k);  // 在合適的子節點插入鍵
    }
}

// 分裂子節點
void BTreeNode::splitChild(int i, BTreeNode* y) {
    BTreeNode* z = new BTreeNode(y->t, y->leaf);
    z->n = t - 1;  // 新的子節點最多有t-1個鍵

    // 複製y節點的後半部分鍵到z節點
    for (int j = 0; j < t - 1; j++)
        z->keys[j] = y->keys[j + t];

    if (!y->leaf) { // 如果y不是葉節點，複製子節點
        for (int j = 0; j < t; j++)
            z->C[j] = y->C[j + t];
    }

    y->n = t - 1; // y節點鍵數減少到t-1

    // 將原節點的子節點右移，為新節點騰出位置
    for (int j = n; j >= i + 1; j--)
        C[j + 1] = C[j];

    C[i + 1] = z;  // 將新節點插入父節點

    // 將原節點的鍵右移，為新的分裂鍵騰出位置
    for (int j = n - 1; j >= i; j--)
        keys[j + 1] = keys[j];

    keys[i] = y->keys[t - 1]; // 插入分裂鍵
    n++;  // 父節點的鍵數增加
}

// 查找鍵的索引
int BTreeNode::findKey(int k) {
    int idx = 0;
    // 找到第1個大於或等於k的鍵
    while (idx < n && keys[idx] < k)
        ++idx;
    return idx;
}

// 從樹中刪除指定的鍵
void BTreeNode::remove(int k) {
    int idx = findKey(k);

    // 如果找到鍵，根據是否是葉節點來刪除
    if (idx < n && keys[idx] == k) {
        if (leaf)
            removeFromLeaf(idx);  // 從葉節點刪除
        else
            removeFromNonLeaf(idx);  // 從非葉節點刪除
    }
    else { // 如果沒找到，遞歸刪除
        if (leaf) {
            cout << "The key " << k << " is not in the tree.\n";
            return;
        }

        bool flag = (idx == n); // 如果刪除的位置是最後一個子節點

        if (C[idx]->n < t)  // 如果子節點鍵數少於t，進行補充
            fill(idx);

        if (flag && idx > n)
            C[idx - 1]->remove(k);  // 從右側子節點刪除
        else
            C[idx]->remove(k);  // 從左側子節點刪除
    }
}

// 從葉節點刪除
void BTreeNode::removeFromLeaf(int idx) {
    // 將後面的鍵左移，刪除指定鍵
    for (int i = idx + 1; i < n; ++i)
        keys[i - 1] = keys[i];
    n--;  // 鍵數減少
}

// 從非葉節點刪除
void BTreeNode::removeFromNonLeaf(int idx) {
    int k = keys[idx];  // 需要刪除的鍵

    // 如果左子節點有足夠的鍵，從左子節點借一個前驅鍵
    if (C[idx]->n >= t) {
        int pred = getPred(idx);
        keys[idx] = pred;
        C[idx]->remove(pred);
    }
    // 如果右子節點有足夠的鍵，從右子節點借一個後繼鍵
    else if (C[idx + 1]->n >= t) {
        int succ = getSucc(idx);
        keys[idx] = succ;
        C[idx + 1]->remove(succ);
    }
    else { // 否則，合併左右子節點
        merge(idx);
        C[idx]->remove(k);
    }
}

// 獲取前驅鍵
int BTreeNode::getPred(int idx) {
    BTreeNode* cur = C[idx];
    while (!cur->leaf) // 遍歷到最右的葉節點
        cur = cur->C[cur->n];
    return cur->keys[cur->n - 1]; // 返回葉節點的最右鍵
}

// 獲取後繼鍵
int BTreeNode::getSucc(int idx) {
    BTreeNode* cur = C[idx + 1];
    while (!cur->leaf) // 遍歷到最左的葉節點
        cur = cur->C[0];
    return cur->keys[0]; // 返回葉節點的最左鍵
}

// 填充節點，確保節點的鍵數達到最小度數
void BTreeNode::fill(int idx) {
    if (idx != 0 && C[idx - 1]->n >= t) // 如果前一個兄弟節點有足夠的鍵，從中借
        borrowFromPrev(idx);
    else if (idx != n && C[idx + 1]->n >= t) // 如果後一個兄弟節點有足夠的鍵，從中借
        borrowFromNext(idx);
    else { // 如果兩個兄弟節點都沒有足夠的鍵，則合併
        if (idx != n)
            merge(idx);
        else
            merge(idx - 1);
    }
}

// 從前一個兄弟節點借鍵
void BTreeNode::borrowFromPrev(int idx) {
    BTreeNode* child = C[idx];
    BTreeNode* sibling = C[idx - 1];

    // 從兄弟節點中借一個鍵，並將其移動到子節點
    for (int i = child->n - 1; i >= 0; --i)
        child->keys[i + 1] = child->keys[i];

    if (!child->leaf) {
        for (int i = child->n; i >= 0; --i)
            child->C[i + 1] = child->C[i];
    }

    // 從父節點借一個鍵並將其移動到子節點
    child->keys[0] = keys[idx - 1];

    if (!child->leaf)
        child->C[0] = sibling->C[sibling->n];

    keys[idx - 1] = sibling->keys[sibling->n - 1];

    child->n += 1;
    sibling->n -= 1;
}

// 從下一個兄弟節點借鍵
void BTreeNode::borrowFromNext(int idx) {
    BTreeNode* child = C[idx];
    BTreeNode* sibling = C[idx + 1];

    // 從兄弟節點借一個鍵
    child->keys[child->n] = keys[idx];

    if (!child->leaf)
        child->C[child->n + 1] = sibling->C[0];

    keys[idx] = sibling->keys[0];

    // 將兄弟節點的鍵左移
    for (int i = 1; i < sibling->n; ++i)
        sibling->keys[i - 1] = sibling->keys[i];

    if (!sibling->leaf) {
        // 移動兄弟節點的子節點
        for (int i = 1; i <= sibling->n; ++i)
            sibling->C[i - 1] = sibling->C[i];
    }

    child->n += 1;
    sibling->n -= 1;
}

// 合併節點
void BTreeNode::merge(int idx) {
    BTreeNode* child = C[idx];
    BTreeNode* sibling = C[idx + 1];

    // 把父節點的鍵放到子節點中
    child->keys[t - 1] = keys[idx];

    // 把兄弟節點的鍵移到子節點中
    for (int i = 0; i < sibling->n; ++i)
        child->keys[i + t] = sibling->keys[i];

    if (!child->leaf) {
        // 合併子節點
        for (int i = 0; i <= sibling->n; ++i)
            child->C[i + t] = sibling->C[i];
    }

    // 移除父節點的鍵
    for (int i = idx + 1; i < n; ++i)
        keys[i - 1] = keys[i];

    for (int i = idx + 2; i <= n; ++i)
        C[i - 1] = C[i];

    child->n += sibling->n + 1; // 更新子節點鍵數
    n--; // 更新父節點鍵數

    delete sibling; // 刪除合併後的兄弟節點
}

// B樹類別
class BTree {
private:
    BTreeNode* root;  // 樹的根節點
    int t;            // 最小度數

public:
    BTree(int _t) {
        root = nullptr;
        t = _t;
    }

    void traverse() {
        if (root != nullptr)
            root->traverse();
    }

    BTreeNode* search(int k) {
        return (root == nullptr) ? nullptr : root->search(k);
    }

    void insert(int k);
    void remove(int k);
};

// 插入鍵
void BTree::insert(int k) {
    if (root == nullptr) {
        root = new BTreeNode(t, true);
        root->keys[0] = k;
        root->n = 1;
    }
    else {
        // 如果根節點已滿，分裂根節點
        if (root->n == 2 * t - 1) {
            BTreeNode* s = new BTreeNode(t, false);
            s->C[0] = root;
            s->splitChild(0, root);

            int i = 0;
            if (s->keys[0] < k)
                i++;
            s->C[i]->insertNonFull(k);

            root = s;  // 更新根節點
        }
        else {
            root->insertNonFull(k);  // 在根節點插入鍵
        }
    }
}

// 刪除鍵
void BTree::remove(int k) {
    if (!root) {
        cout << "The tree is empty\n";
        return;
    }

    root->remove(k);

    // 如果根節點沒有鍵，更新根節點
    if (root->n == 0) {
        BTreeNode* tmp = root;
        if (root->leaf)
            root = nullptr;
        else
            root = root->C[0];

        delete tmp;
    }
}

int main() {
    int t;
    cout << "Enter the minimum degree of the B-tree: ";
    cin >> t;

    BTree tree(t);

    while (true) {
        cout << "\n1. Insert\n2. Remove\n3. Display\n4. Search\n5. Exit\nEnter your choice: ";
        int choice;
        cin >> choice;

        switch (choice) {
        case 1: {
            int val;
            cout << "Enter value to insert: ";
            cin >> val;
            tree.insert(val);
            break;
        }
        case 2: {
            int val;
            cout << "Enter value to remove: ";
            cin >> val;
            tree.remove(val);
            break;
        }
        case 3:
            cout << "B-tree:\n";
            tree.traverse();
            break;
        case 4: {
            int val;
            cout << "Enter value to search: ";
            cin >> val;
            if (tree.search(val))
                cout << "Found\n";
            else
                cout << "Not found\n";
            break;
        }
        case 5:
            return 0;
        default:
            cout << "Invalid choice!\n";
        }
    }

    return 0;
}
