#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

class MWayTree {
public:
    // Node結構，表示樹的每個節點
    struct Node {
        vector<int> keys;        // 儲存鍵值的向量
        vector<Node*> children;  // 儲存子節點的向量
        bool isLeaf;             // 標誌這個節點是否為葉子節點

        Node(bool leaf = true) : isLeaf(leaf) {}  // 預設節點為葉子節點
    };

    // 初始化m-way樹，m為樹的度（每個節點最多可以包含m-1個鍵值）
    MWayTree(int m) : m(m), root(nullptr) {}

    // 插入一個鍵值
    void insert(int key) {
        if (root == nullptr) {
            root = new Node(true);  // 如果樹是空的，創建一個根節點
        }

        // 如果根節點已滿，則需要分裂根節點
        if (root->keys.size() == m - 1) {
            Node* newRoot = new Node(false);  // 創建新的根節點
            newRoot->children.push_back(root);  // 將舊的根節點作為新根的子節點
            split(newRoot, 0);  // 分裂舊的根節點
            root = newRoot;  // 更新根節點
        }

        insertNonFull(root, key);  // 在非滿的節點上插入鍵值
    }

    // 刪除一個鍵值
    void remove(int key) {
        if (root == nullptr) {
            cout << "Tree is empty!" << endl;  // 如果樹為空，輸出提示信息
            return;
        }

        remove(root, key);  // 從根節點開始刪除鍵值

        // 如果根節點沒有鍵值，並且有子節點，則更新根節點為它的第一個子節點
        if (root->keys.empty() && !root->isLeaf) {
            Node* temp = root;
            root = root->children[0];
            delete temp;  // 刪除舊的根節點
        }
    }

    // 顯示樹的結構（以樹狀圖形式）
    void printTree() {
        printTree(root, 0);  // 從根節點開始打印
    }

private:
    int m;              // 樹的度（每個節點最多可以有m-1個鍵值）
    Node* root;         // 樹的根節點

    // 在非滿的節點上插入一個鍵值
    void insertNonFull(Node* node, int key) {
        int i = node->keys.size() - 1;  // 從節點的最後一個鍵值開始檢查

        if (node->isLeaf) {  // 如果是葉子節點
            node->keys.push_back(0);  // 為插入的鍵值預留空間
            while (i >= 0 && key < node->keys[i]) {  // 從右至左找出插入的位置
                node->keys[i + 1] = node->keys[i];
                i--;
            }
            node->keys[i + 1] = key;  // 插入鍵值
        }
        else {  // 如果不是葉子節點，找到合適的子節點並插入
            while (i >= 0 && key < node->keys[i]) {
                i--;
            }
            i++;

            // 如果子節點已滿，則分裂該子節點
            if (node->children[i]->keys.size() == m - 1) {
                split(node, i);
                if (key > node->keys[i]) {
                    i++;
                }
            }
            insertNonFull(node->children[i], key);  // 在選中的子節點上遞迴插入
        }
    }

    // 分裂滿的節點
    void split(Node* parent, int i) {
        Node* fullChild = parent->children[i];  // 取得滿的子節點
        Node* newChild = new Node(fullChild->isLeaf);  // 創建新的子節點
        parent->children.insert(parent->children.begin() + i + 1, newChild);  // 插入新子節點到父節點

        int midKey = fullChild->keys[m / 2];  // 中間鍵值（分裂的關鍵）
        parent->keys.insert(parent->keys.begin() + i, midKey);  // 將中間鍵值插入父節點

        // 將右半部分的鍵值與子節點移動到新子節點
        newChild->keys.assign(fullChild->keys.begin() + m / 2 + 1, fullChild->keys.end());
        fullChild->keys.resize(m / 2);  // 左半部分保留在原來的子節點

        // 如果子節點不是葉子節點，則也需要處理其子節點
        if (!fullChild->isLeaf) {
            newChild->children.assign(fullChild->children.begin() + m / 2 + 1, fullChild->children.end());
            fullChild->children.resize(m / 2 + 1);
        }
    }

    // 刪除一個鍵值
    void remove(Node* node, int key) {
        int i = 0;
        // 找到鍵值所在位置
        while (i < node->keys.size() && key > node->keys[i]) {
            i++;
        }

        // 如果找到鍵值，並且是葉子節點
        if (i < node->keys.size() && node->keys[i] == key) {
            if (node->isLeaf) {
                node->keys.erase(node->keys.begin() + i);  // 刪除鍵值
            }
            else {
                handleNonLeafRemoval(node, i);  // 如果是非葉子節點，進行特殊處理
            }
        }
        else if (!node->isLeaf) {
            remove(node->children[i], key);  // 如果是非葉子節點，遞迴刪除
        }
    }

    // 處理非葉子節點的刪除情況
    void handleNonLeafRemoval(Node* node, int i) {
        Node* child = node->children[i];
        Node* sibling = node->children[i + 1];

        // 如果左子節點有足夠的鍵值，則替換為左邊的最大鍵值
        if (child->keys.size() >= m / 2) {
            int pred = child->keys.back();  // 左子節點的最大鍵值
            node->keys[i] = pred;
            remove(child, pred);  // 刪除左子節點的最大鍵值
        }
        // 如果右子節點有足夠的鍵值，則替換為右邊的最小鍵值
        else if (sibling->keys.size() >= m / 2) {
            int succ = sibling->keys.front();  // 右子節點的最小鍵值
            node->keys[i] = succ;
            remove(sibling, succ);  // 刪除右子節點的最小鍵值
        }
        // 如果兩個子節點都不足，則將兩個子節點合併
        else {
            mergeChildren(node, i);
            remove(child, node->keys[i]);  // 重新刪除鍵值
        }
    }

    // 合併左右子節點
    void mergeChildren(Node* node, int i) {
        Node* leftChild = node->children[i];
        Node* rightChild = node->children[i + 1];

        leftChild->keys.push_back(node->keys[i]);  // 將父節點的鍵值移到左子節點
        leftChild->keys.insert(leftChild->keys.end(), rightChild->keys.begin(), rightChild->keys.end());  // 合併右子節點的鍵值
        node->keys.erase(node->keys.begin() + i);  // 刪除父節點的鍵值

        if (!leftChild->isLeaf) {
            leftChild->children.insert(leftChild->children.end(), rightChild->children.begin(), rightChild->children.end());  // 合併子節點
        }

        node->children.erase(node->children.begin() + i + 1);  // 刪除右子節點
        delete rightChild;  // 釋放右子節點的內存
    }

    // 遞迴打印樹的結構
    void printTree(Node* node, int level) {
        if (node == nullptr) return;  // 如果節點為空，返回

        cout << string(level * 4, ' ');  // 根據層級輸出縮排
        for (int key : node->keys) {  // 打印節點的所有鍵值
            cout << key << " ";
        }
        cout << endl;

        // 如果不是葉子節點，遞迴打印每個子節點
        if (!node->isLeaf) {
            for (Node* child : node->children) {
                printTree(child, level + 1);  // 遞迴打印子節點
            }
        }
    }
};

int main() {
    int m;
    cout << "Enter the value of m (degree of tree): ";
    cin >> m;

    MWayTree tree(m);

    // 使用者輸入數字，直到輸入-1停止
    int key;
    cout << "Enter integers to insert into the tree (enter -1 to stop):" << endl;
    while (true) {
        cin >> key;
        if (key == -1) break;
        tree.insert(key);
    }

    // 顯示樹狀結構
    cout << "\nTree structure:" << endl;
    tree.printTree();

    // 提供插入與刪除功能
    cout << "\nEnter a number to insert: ";
    cin >> key;
    tree.insert(key);
    cout << "After insertion:" << endl;
    tree.printTree();

    cout << "\nEnter a number to remove: ";
    cin >> key;
    tree.remove(key);
    cout << "After deletion:" << endl;
    tree.printTree();

    return 0;
}
