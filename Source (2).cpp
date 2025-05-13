#include <sstream>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stack>

using namespace std;

struct Token {
    int index;
    string category;
    string lexeme;
    Token(int idx, string cat, string lex = "") : index(idx), category(cat), lexeme(lex) {}
};

struct ParseTreeNode {
    string value;
    vector<ParseTreeNode*> children;

    ParseTreeNode(string val) : value(val) {}
};

vector<Token> tokens;
int current = 0;
stack<ParseTreeNode*> parseTreeStack;

unordered_map<int, string> identifiers;
unordered_map<int, string> keywords;
unordered_map<int, string> literals;

Token peek() {
    return (current < tokens.size()) ? tokens[current] : Token(-1, "EOF");
}

Token advance() {
    if (current < tokens.size()) return tokens[current++];
    return Token(-1, "EOF");
}

bool match(const string& expected) {
    if (peek().category == expected || peek().lexeme == expected) {
        advance();
        return true;
    }
    return false;
}

void error(const string& msg) {
    cerr << "Parse error at token " << current << ": " << msg << endl;
    exit(1);
}

void Type();
void Expr();
void Stmt();
void StmtList();
void CompStmt();
void Declaration();
void IdentList();
void ArgList();
void Arg();
void Function();
void Functions();
void Rvalue();
void Mag();
void Term();
void Factor();
void StmtPrime();
void Match();
void Open();
void OpenPrime();

// Modified addNode to include actual data when needed
void addNode(string value, bool useActualData = false, Token* token = nullptr) {
    string displayValue = value;
    if (useActualData && token) {
        if (token->category == "identifier" && identifiers.count(token->index)) {
            displayValue += " (" + identifiers[token->index] + ")";
        }
        else if (token->category == "keyword" && keywords.count(token->index)) {
            displayValue += " (" + keywords[token->index] + ")";
        }
        else if (!token->lexeme.empty() && token->lexeme != "UNKNOWN_ID" &&
            token->lexeme != "UNKNOWN_KW" && token->lexeme != "UNKNOWN_LIT") {
            displayValue += " (" + token->lexeme + ")";
        }
    }

    ParseTreeNode* node = new ParseTreeNode(displayValue);
    parseTreeStack.top()->children.push_back(node);
    parseTreeStack.push(node);
}

void endNode() {
    parseTreeStack.pop();
}

void Type() {
    Token t = peek();
    addNode("Type", true, &t);
    if (t.lexeme == "Adadi" || t.lexeme == "Ashriyal" || t.lexeme == "Harf" || t.lexeme == "Math" || t.lexeme == "Mantiqi") {
        advance();
    }
    else {
        error("Expected a Type");
    }
    endNode();
}

void IdentList() {
    addNode("IdentList");
    Token t = peek();
    if (match("identifier")) {
        addNode("Identifier", true, &t);
        endNode(); // Close the identifier node

        while (peek().lexeme == ",") {
            Token comma = peek();
            addNode("Comma", true, &comma);
            endNode(); // Close the comma node
            advance();

            t = peek();
            if (!match("identifier")) {
                error("Expected identifier after ','");
            }
            addNode("Identifier", true, &t);
            endNode(); // Close the identifier node
        }
    }
    else {
        error("Expected identifier in IdentList");
    }
    endNode();
}

void Declaration() {
    addNode("Declaration");
    Type();
    IdentList();
    if (!match("::")) error("Expected '::' at end of declaration");
    endNode();
}

void Arg() {
    addNode("Arg");
    Type();
    Token t = peek();
    if (!match("identifier")) error("Expected identifier in argument");
    addNode("Identifier", true, &t);
    endNode(); // Close the identifier node
    endNode();
}

void ArgListPrime() {
    addNode("ArgListPrime");
    while (peek().lexeme == ",") {
        Token comma = peek();
        addNode("Comma", true, &comma);
        endNode(); // Close the comma node
        advance();
        Arg();
    }
    endNode();
}

void ArgList() {
    addNode("ArgList");
    if (peek().lexeme != ")") {
        Arg();
        ArgListPrime();
    }
    endNode();
}

void CompStmt() {
    addNode("CompStmt");
    Token brace = peek();
    if (!match("{")) error("Expected '{'");
    addNode("Open Brace", true, &brace);
    endNode(); // Close the brace node

    StmtList();

    brace = peek();
    if (!match("}")) error("Expected '}'");
    addNode("Close Brace", true, &brace);
    endNode(); // Close the brace node
    endNode();
}

void StmtList() {
    addNode("StmtList");
    while (peek().category != "EOF" && peek().lexeme != "}") {
        Stmt();
    }
    endNode();
}

void ForStmt() {
    addNode("ForStmt");
    Token t = peek();
    if (!match("for")) error("Expected 'for'");
    addNode("Keyword", true, &t);
    endNode(); // Close the keyword node

    t = peek();
    if (!match("(")) error("Expected '('");
    addNode("Open Paren", true, &t);
    endNode(); // Close the paren node

    Expr();

    t = peek();
    if (!match("::")) error("Expected '::'");
    addNode("Separator", true, &t);
    endNode(); // Close the separator node

    Expr();

    t = peek();
    if (!match("::")) error("Expected '::'");
    addNode("Separator", true, &t);
    endNode(); // Close the separator node

    Expr();

    t = peek();
    if (!match(")")) error("Expected ')'");
    addNode("Close Paren", true, &t);
    endNode(); // Close the paren node

    Stmt();
    endNode();
}

void WhileStmt() {
    addNode("WhileStmt");
    Token t = peek();
    if (!match("while")) error("Expected 'while'");
    addNode("Keyword", true, &t);
    endNode(); // Close the keyword node

    t = peek();
    if (!match("(")) error("Expected '('");
    addNode("Open Paren", true, &t);
    endNode(); // Close the paren node

    Expr();

    t = peek();
    if (!match(")")) error("Expected ')'");
    addNode("Close Paren", true, &t);
    endNode(); // Close the paren node

    Stmt();
    endNode();
}

void Stmt() {
    addNode("Stmt");
    Token t = peek();
    if (t.lexeme == "for") {
        ForStmt();
    }
    else if (t.lexeme == "while") {
        WhileStmt();
    }
    else if (t.lexeme == "::") {
        addNode("Separator", true, &t);
        endNode(); // Close the separator node
        advance();
    }
    else if (t.category == "identifier") {
        Expr();
        t = peek();
        if (!match("::")) error("Expected '::' after expression");
        addNode("Separator", true, &t);
        endNode(); // Close the separator node
    }
    else if (t.lexeme == "Agar") {
        addNode("Keyword", true, &t);
        endNode(); // Close the keyword node
        advance();

        t = peek();
        if (!match("(")) error("Expected '(' after Agar");
        addNode("Open Paren", true, &t);
        endNode(); // Close the paren node

        Expr();

        t = peek();
        if (!match(")")) error("Expected ')'");
        addNode("Close Paren", true, &t);
        endNode(); // Close the paren node

        StmtPrime();
    }
    else if (t.lexeme == "{") {
        CompStmt();
    }
    else {
        Declaration();
    }
    endNode();
}

void StmtPrime() {
    addNode("StmtPrime");
    Token t = peek();
    if (t.lexeme == "match") {
        Match();
        t = peek();
        if (!match("Wagarna")) error("Expected 'Wagarna'");
        addNode("Keyword", true, &t);
        endNode(); // Close the keyword node
        Match();
    }
    else {
        OpenPrime();
    }
    endNode();
}

void Match() {
    addNode("Match");
    Token t = peek();
    if (match("Agar")) {
        addNode("Keyword", true, &t);
        endNode(); // Close the keyword node

        t = peek();
        if (!match("(")) error("Expected '('");
        addNode("Open Paren", true, &t);
        endNode(); // Close the paren node

        Expr();

        t = peek();
        if (!match(")")) error("Expected ')'");
        addNode("Close Paren", true, &t);
        endNode(); // Close the paren node

        Match();

        t = peek();
        if (!match("Wagarna")) error("Expected 'Wagarna'");
        addNode("Keyword", true, &t);
        endNode(); // Close the keyword node

        Match();
    }
    else {
        addNode("Token", true, &t);
        endNode(); // Close the token node
        advance();
    }
    endNode();
}

void Open() {
    addNode("Open");
    Token t = peek();
    if (!match("Agar")) error("Expected 'Agar'");
    addNode("Keyword", true, &t);
    endNode(); // Close the keyword node

    t = peek();
    if (!match("(")) error("Expected '('");
    addNode("Open Paren", true, &t);
    endNode(); // Close the paren node

    Expr();

    t = peek();
    if (!match(")")) error("Expected ')'");
    addNode("Close Paren", true, &t);
    endNode(); // Close the paren node

    OpenPrime();
    endNode();
}

void OpenPrime() {
    addNode("OpenPrime");
    Token t = peek();
    if (t.lexeme == "match") {
        Match();
        t = peek();
        if (!match("Wagarna")) error("Expected 'Wagarna'");
        addNode("Keyword", true, &t);
        endNode(); // Close the keyword node
        Open();
    }
    else {
        Stmt();
    }
    endNode();
}

void Function() {
    addNode("Function");
    Type();

    Token t = peek();
    if (!match("identifier")) error("Expected identifier after type in function");
    addNode("Function Name", true, &t);
    endNode(); // Close the function name node

    t = peek();
    if (!match("(")) error("Expected '(' in function");
    addNode("Open Paren", true, &t);
    endNode(); // Close the paren node

    ArgList();

    t = peek();
    if (!match(")")) error("Expected ')'");
    addNode("Close Paren", true, &t);
    endNode(); // Close the paren node

    CompStmt();
    endNode();
}

void Expr() {
    addNode("Expr");
    if (peek().category == "identifier" && tokens[current + 1].lexeme == ":=") {
        Token id = peek();
        addNode("Identifier", true, &id);
        endNode(); // Close the identifier node
        advance();

        Token op = peek();
        addNode("Operator", true, &op);
        endNode(); // Close the operator node
        advance();

        Expr();
    }
    else {
        Rvalue();
    }
    endNode();
}

void Rvalue() {
    addNode("Rvalue");
    Mag();
    while (peek().lexeme == "==" || peek().lexeme == "<" || peek().lexeme == ">" ||
        peek().lexeme == "<=" || peek().lexeme == ">=" || peek().lexeme == "!=" ||
        peek().lexeme == "<>") {
        Token op = peek();
        addNode("Operator", true, &op);
        endNode(); // Close the operator node
        advance();
        Mag();
    }
    endNode();
}

void Mag() {
    addNode("Mag");
    Term();
    while (peek().lexeme == "+" || peek().lexeme == "-") {
        Token op = peek();
        addNode("Operator", true, &op);
        endNode(); // Close the operator node
        advance();
        Term();
    }
    endNode();
}

void Term() {
    addNode("Term");
    Factor();
    while (peek().lexeme == "*" || peek().lexeme == "/") {
        Token op = peek();
        addNode("Operator", true, &op);
        endNode(); // Close the operator node
        advance();
        Factor();
    }
    endNode();
}

void Factor() {
    addNode("Factor");
    Token t = peek();
    if (match("(")) {
        addNode("Open Paren", true, &t);
        endNode(); // Close the paren node
        Expr();
        t = peek();
        if (!match(")")) error("Expected ')'");
        addNode("Close Paren", true, &t);
        endNode(); // Close the paren node
    }
    else if (match("identifier") || match("number")) {
        addNode(t.category == "identifier" ? "Identifier" : "Number", true, &t);
        endNode(); // Close the identifier/number node
    }
    else {
        error("Expected Factor");
    }
    endNode();
}

void Functions() {
    while (peek().category != "EOF") {
        Function();
    }
}

void loadTokensFromFiles() {
    auto loadFile = [](const string& filename) -> vector<string> {
        vector<string> data;
        ifstream file(filename, ios::in);
        string line;
        while (getline(file, line)) {
            if (!line.empty()) data.push_back(line);
        }
        return data;
        };

    vector<string> idList = loadFile("identifiers.txt");
    vector<string> kwList = loadFile("keywords.txt");
    vector<string> litList = loadFile("literals.txt");

    for (int i = 0; i < idList.size(); ++i) identifiers[i + 1] = idList[i];
    for (int i = 0; i < kwList.size(); ++i) keywords[i + 1] = kwList[i];

    ifstream tokenFile("tokens.txt", ios::in);
    if (!tokenFile.is_open()) {
        cerr << "Error opening tokens.txt file!" << endl;
        return;
    }

    string line;
    while (getline(tokenFile, line)) {
        if (line.empty()) continue;

        istringstream iss(line);
        string token;
        while (iss >> token) {
            if (token == "<{>" || token == "<}>" || token == "<(>" || token == "<)>" ||
                token == "<::>" || token == "<+>" || token == "<->" || token == "<*>" ||
                token == "</>" || token == "<:=>" || token == "<==>" || token == "<!=>" || token == "<<>>") {
                string symbol = token.substr(1, token.size() - 2);
                tokens.emplace_back(-1, symbol, symbol);
            }
            else if (token[0] == '<' && token.back() == '>') {
                token = token.substr(1, token.size() - 2);
                stringstream ss(token);
                string idxStr, cat;
                getline(ss, idxStr, ',');
                getline(ss, cat);

                int index = stoi(idxStr);
                string lex;

                if (cat == "identifier") {
                    lex = identifiers.count(index) ? identifiers[index] : "UNKNOWN_ID";
                    tokens.emplace_back(index, "identifier", lex);
                }
                else if (cat == "keyword") {
                    lex = keywords.count(index) ? keywords[index] : "UNKNOWN_KW";
                    tokens.emplace_back(index, "keyword", lex);
                }
                else {
                    tokens.emplace_back(index, cat, "UNKNOWN_CAT");
                }
            }
        }
    }

    tokens.emplace_back(-1, "EOF", "EOF");
}

void printParseTreeToFile(ParseTreeNode* node, ofstream& outFile, const string& prefix = "", bool isLast = true) {
    outFile << prefix;

    if (!prefix.empty()) {
        outFile << (isLast ? "+-- " : "|-- ");
    }

    outFile << node->value << endl;

    for (size_t i = 0; i < node->children.size(); ++i) {
        bool last = (i == node->children.size() - 1);
        printParseTreeToFile(node->children[i], outFile, prefix + (isLast ? "    " : "|   "), last);
    }
}

void printParseTree(ParseTreeNode* node, const string& prefix = "", bool isLast = true) {
    cout << prefix;

    if (!prefix.empty()) {
        cout << (isLast ? "+-- " : "|-- ");
    }

    cout << node->value << endl;

    for (size_t i = 0; i < node->children.size(); ++i) {
        bool last = (i == node->children.size() - 1);
        printParseTree(node->children[i], prefix + (isLast ? "    " : "|   "), last);
    }
}

int main() {
    loadTokensFromFiles();

    ParseTreeNode* root = new ParseTreeNode("Program");
    parseTreeStack.push(root);

    Functions();

    cout << "Parsing successful!" << endl;

    // Print to console
    printParseTree(root);

    // Save to file
    ofstream outFile("tree.txt");
    if (outFile.is_open()) {
        printParseTreeToFile(root, outFile);
        outFile.close();
        cout << "Parse tree saved to tree.txt" << endl;
    }
    else {
        cerr << "Unable to open tree.txt for writing" << endl;
    }

    return 0;
}