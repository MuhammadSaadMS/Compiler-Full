#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <sstream>
#include <algorithm>

using namespace std;

class TACGenerator {
private:
    vector<string> tacCode;
    int tempCounter = 0;

public:
    string newTemp() {
        return "t" + to_string(tempCounter++);
    }

    void emit(const string& code) {
        tacCode.push_back(code);
    }

    void saveToFile(const string& filename) {
        ofstream outFile(filename);
        if (outFile.is_open()) {
            for (const auto& line : tacCode) {
                outFile << line << endl;
            }
            outFile.close();
        }
        else {
            cerr << "Unable to open " << filename << " for writing" << endl;
        }
    }

    void print() {
        for (const auto& line : tacCode) {
            cout << line << endl;
        }
    }
};

struct TreeNode {
    string type;
    string value;
    vector<TreeNode*> children;

    TreeNode(const string& t, const string& v = "") : type(t), value(v) {}
};

TreeNode* buildTreeFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return nullptr;
    }

    stack<pair<TreeNode*, int>> nodeStack;
    TreeNode* root = nullptr;
    string line;

    while (getline(file, line)) {
        // Count indentation level
        size_t indent = 0;
        while (indent < line.size() && (line[indent] == ' ' || line[indent] == '|' || line[indent] == '+')) {
            indent++;
        }

        // Clean the line
        string nodeStr = line.substr(indent);
        size_t prefix = nodeStr.find("-- ");
        if (prefix != string::npos) {
            nodeStr = nodeStr.substr(prefix + 3);
        }

        // Extract type and value
        string nodeType, nodeValue;
        size_t paren = nodeStr.find("(");
        if (paren != string::npos) {
            nodeType = nodeStr.substr(0, paren - 1);
            nodeValue = nodeStr.substr(paren + 1, nodeStr.find(")") - paren - 1);
        }
        else {
            nodeType = nodeStr;
        }

        // Create node
        TreeNode* newNode = new TreeNode(nodeType, nodeValue);

        // Set root if first node
        if (nodeStack.empty()) {
            root = newNode;
            nodeStack.push({ newNode, indent });
            continue;
        }

        // Find parent
        while (!nodeStack.empty() && nodeStack.top().second >= indent) {
            nodeStack.pop();
        }

        // Add to parent's children
        if (!nodeStack.empty()) {
            nodeStack.top().first->children.push_back(newNode);
        }
        nodeStack.push({ newNode, indent });
    }

    return root;
}

string processNode(TreeNode* node, TACGenerator& tacGen) {
    if (!node) return "";

    // Handle leaf nodes
    if (node->type == "Identifier") {
        return node->value;
    }
    else if (node->type == "Operator") {
        return node->value;
    }

    // Handle expression nodes
    if (node->type == "Expr") {
        if (node->children.size() >= 3) {
            // Assignment case
            if (node->children[1]->type == "Operator" && node->children[1]->value == ":=") {
                string left = processNode(node->children[0], tacGen);
                string right = processNode(node->children[2], tacGen);
                tacGen.emit(left + " := " + right);
                return left;
            }
            // Binary operation case
            else if (node->children[1]->type == "Operator") {
                string left = processNode(node->children[0], tacGen);
                string op = node->children[1]->value;
                string right = processNode(node->children[2], tacGen);
                string temp = tacGen.newTemp();
                tacGen.emit(temp + " := " + left + " " + op + " " + right);
                return temp;
            }
        }
        return processNode(node->children[0], tacGen);
    }
    // Handle Rvalue, Mag, Term, Factor nodes by processing their first child
    else if (node->type == "Rvalue" || node->type == "Mag" || node->type == "Term" || node->type == "Factor") {
        if (!node->children.empty()) {
            return processNode(node->children[0], tacGen);
        }
    }

    // Handle statement nodes
    else if (node->type == "StmtList") {
        for (auto child : node->children) {
            processNode(child, tacGen);
        }
    }
    else if (node->type == "Stmt") {
        for (auto child : node->children) {
            processNode(child, tacGen);
        }
    }
    else if (node->type == "CompStmt") {
        for (auto child : node->children) {
            processNode(child, tacGen);
        }
    }

    return "";
}

int main() {
    // Build parse tree from file
    TreeNode* parseTree = buildTreeFromFile("tree.txt");
    if (!parseTree) {
        cerr << "Failed to build parse tree" << endl;
        return 1;
    }

    // Generate TAC from parse tree
    TACGenerator tacGen;

    // Start processing from the CompStmt node which contains the actual code
    for (auto child : parseTree->children) {
        if (child->type == "Function") {
            for (auto funcChild : child->children) {
                if (funcChild->type == "CompStmt") {
                    processNode(funcChild, tacGen);
                    break;
                }
            }
            break;
        }
    }

    // Output results
    cout << "Generated Three Address Code:\n";
    tacGen.print();
    tacGen.saveToFile("result.tac");
    cout << "TAC saved to result.tac" << endl;

    return 0;
}