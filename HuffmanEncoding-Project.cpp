#include <iostream>
#include <queue>
#include <fstream>
using namespace std;

class Node
{
public:
    char letter;
    int freq;
    Node *left;
    Node *right;

    Node(char a, int freq) : letter(a), freq(freq) { left = right = NULL; }
};

class compare
{
public:
    bool operator()(Node *a, Node *b) const
    {
        return a->freq > b->freq;
    }
};

void buildMinHeap(priority_queue<Node *, vector<Node *>, compare> &q, char letters[], int freq[], int size)
{

    for (int i = 0; i < size; i++)
    {
        if (freq[i] > 0)
        {
            Node *temp = new Node(letters[i], freq[i]);
            q.push(temp);
        }
    }
}

void printHeap(priority_queue<Node *, vector<Node *>, compare> q)
{
    while (q.size() > 0)
    {
        Node *temp = q.top();
        q.pop();
        cout << temp->letter << "  " << temp->freq << endl;
    }
}

Node *compress(priority_queue<Node *, vector<Node *>, compare> q)
{
    cout << "Compressing....!" << endl;
    while (q.size() > 1)
    {
        Node *left = q.top();
        q.pop();
        Node *right = q.top();
        q.pop();

        int sumFreq = left->freq + right->freq;
        Node *temp = new Node('$', sumFreq);
        temp->left = left;
        temp->right = right;
        q.push(temp);
    }
    return q.top();
}

void LevelOrdertraversal(Node *root)
{
    cout << "Level Order traversal...." << endl;
    queue<Node *> q;
    if (root == NULL)
    {
        cout << "Empty!" << endl;
        return;
    }
    cout << "Step 1" << endl;
    q.push(root);
    // q.pop();
    while (q.size() > 0)
    {
        int size = q.size();
        for (int i = 0; i < size; i++)
        {
            Node *temp = q.front();
            q.pop();
            cout << temp->letter << " " << temp->freq << ", ";
            if (temp->left)
            {
                q.push(temp->left);
            }
            if (temp->right)
            {
                q.push(temp->right);
            }
        }
        cout << "" << endl;
    }
    cout << "" << endl;
}

class StoreCodes
{
public:
    char alphabet;
    string code;
    StoreCodes()
    {
        alphabet = '$';
        code = "";
    }
};

void AssignCodes(Node *root, string code, StoreCodes codes[])
{
    if (root->left == NULL && root->right == NULL)
    {
        codes[root->letter - 'a'].alphabet = root->letter;
        codes[root->letter - 'a'].code = code;
        return;
    }
    AssignCodes(root->left, code + "0", codes);
    AssignCodes(root->right, code + "1", codes);
}

void PrintCodes(Node *root, string code)
{
    if (root->left == NULL && root->right == NULL)
    {
        cout << root->letter << "  " << code << endl;
        return;
    }
    PrintCodes(root->left, code + "0");
    PrintCodes(root->right, code + "1");
}

void writeTable(const char letters[], const int freq[], int n, const std::string &OutPutfilename)
{
    std::ofstream out(OutPutfilename, std::ios::binary);
    if (!out)
        return;

    // First write how many entries will follow
    int count = 0;
    for (int i = 0; i < n; i++)
        if (freq[i] > 0)
            count++;

    out.write((char *)&count, sizeof(count));

    // Now write each (char, freq) pair
    for (int i = 0; i < n; i++)
    {
        if (freq[i] == 0)
            continue;

        out.write(&letters[i], 1);                // 1 byte
        out.write((char *)&freq[i], sizeof(int)); // 4 bytes
    }
    // reserve space for total bits (will be backpatched by WriteBitStream)
    long long totalBitsPlaceholder = 0;
    out.write((char *)&totalBitsPlaceholder, sizeof(totalBitsPlaceholder));

    out.close();
}

void readTableFromFile(int freq[], int n, const std::string &filename)
{
    cout << "Reading from file....." << endl;
    std::ifstream in(filename, std::ios::binary);
    if (!in)
        return;

    // Initialize freq to 0
    for (int i = 0; i < n; i++)
        freq[i] = 0;

    int count;
    in.read((char *)&count, sizeof(count));

    for (int k = 0; k < count; k++)
    {
        char ch;
        int fr;

        in.read(&ch, 1);
        cout << ch << " ";
        in.read((char *)&fr, sizeof(int));

        // find where this character belongs
        freq[ch - 'a'] = fr;
    }
    cout << "" << endl;

    in.close();
}

void WriteBitStream(const string &inputFileName, const string &outputFIleName, StoreCodes codes[])
{
    ifstream in(inputFileName);
    ofstream out(outputFIleName, ios::binary | ios::app);

    if (!in || !out)
    {
        cout << "File not oppend for bitstream" << endl;
        return;
    }

    unsigned char buffer = 0;
    int bitCount = 0;
    long long totalBits = 0; // track total bits written

  
char ch;
while (in.get(ch))
{
    if (ch >= 'A' && ch <= 'Z')
    {
        ch = ch - 'A' + 'a';
    }

    if (ch < 'a' || ch > 'z')
    {
        continue;
    }

    string code = codes[ch - 'a'].code;

    for (char bit : code)
    {
        buffer <<= 1;
        buffer |= (bit == '1');
        bitCount++;
        totalBits++;

        if (bitCount == 8)
        {
            out.put(buffer);
            buffer = 0;
            bitCount = 0;
        }
    }
}

    if (bitCount > 0)
    {
        buffer <<= (8 - bitCount);
        out.put(buffer);
    }
    in.close();
    out.close();

    // Backpatch totalBits into the header placeholder written by writeTable
    // read header count to compute offset: sizeof(count) + count*(1+sizeof(int))
    std::ifstream headerIn(outputFIleName, std::ios::binary);
    if (!headerIn)
        return; // can't open file to backpatch

    int count = 0;
    headerIn.read((char *)&count, sizeof(count));
    headerIn.close();

    std::streamoff placeholderOffset = sizeof(count) + static_cast<std::streamoff>(count) * (1 + static_cast<std::streamoff>(sizeof(int)));

    std::fstream headerOut(outputFIleName, std::ios::binary | std::ios::in | std::ios::out);
    if (!headerOut)
        return;
    headerOut.seekp(placeholderOffset, std::ios::beg);
    long long bitsToWrite = totalBits;
    headerOut.write((char *)&bitsToWrite, sizeof(bitsToWrite));
    headerOut.close();
}


void DecodeBitstream(string inputFileName, string outputFileName, Node *root)
{
    ifstream in(inputFileName, ios::binary);
    ofstream out(outputFileName);

    if (!in || !out)
    {
        cout << "File Not Opened" << endl;
        return;
    }

    int count;
    in.read((char *)&count, sizeof(int));

    char ch;
    int fr;

    for (int i = 0; i < count; i++)
    {
        in.read(&ch, 1);
        in.read((char *)&fr, sizeof(int));
    }

    long long totalBits = 0;
    in.read((char *)&totalBits, sizeof(totalBits));

    cout << "Total bits: " << totalBits << endl;

    if (totalBits <= 0 || root == NULL)
    {
        cout << "Invalid compressed file!" << endl;
        return;
    }

    unsigned char byte;
    long long processedbits = 0;

    Node *current = root;

    while (processedbits < totalBits && in.read((char *)&byte, 1))
    {
        for (int i = 7; i >= 0 && processedbits < totalBits; i--)
        {
            int bit = (byte >> i) & 1;

            if (bit == 0)
                current = current->left;
            else
                current = current->right;

            processedbits++;

            if (current == NULL)
            {
                cout << "Invalid Huffman bitstream!" << endl;
                return;
            }

            if (current->left == NULL && current->right == NULL)
            {
                out.put(current->letter);
                current = root;
            }
        }
    }

    in.close();
    out.close();

    cout << "Decompression completed!" << endl;
}

void readfileCountFreq(int freq[], string filename)
{
    std::ifstream in(filename); // open in text mode
    if (!in)
        return;

    // Initialize freq to 0
    for (int i = 0; i < 26; i++)
        freq[i] = 0;

    char ch;
    while (in.get(ch))
    {
        // normalize uppercase to lowercase without extra headers
        if (ch >= 'A' && ch <= 'Z')
            ch = static_cast<char>(ch - 'A' + 'a');

        // count only english alphabets a-z
        if (ch >= 'a' && ch <= 'z')
        {
            freq[ch - 'a']++;
        }
    }

    in.close();
}

void printfreq(char letters[], int freq[], int size)
{
    for (int i = 0; i < size; i++)
    {
        if (freq[i] > 0)
        {
            cout << letters[i] << "  " << freq[i] << endl;
        }
    }
}

int main()
{
    priority_queue<Node *, vector<Node *>, compare> q;  // encoding
    priority_queue<Node *, vector<Node *>, compare> q2; // decoding

    char letters[] = {
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

    StoreCodes codes[26];
    int freq2[26] = {0};
    int size = sizeof(freq2) / sizeof(freq2[0]);
    int choice;
    do
    {

        cout << "1.Compress  2.Decompress" << endl;
        cin >> choice;
        string filename;
        Node *compressedRoot;

        switch (choice)
        {
        case 1:
            cout << "Enter File name (abc.txt): ";
            cin >> filename;
            readfileCountFreq(freq2, filename);
            buildMinHeap(q, letters, freq2, size);
            compressedRoot = compress(q);
            AssignCodes(compressedRoot, "", codes);
            // printfreq(letters, freq2, size);
            PrintCodes(compressedRoot, "");
            writeTable(letters, freq2, size, "encoded.bin");
            WriteBitStream(filename, "encoded.bin", codes);
            break;
        case 2:
            cout << "Enter File name (abc.bin): ";
            cin >> filename;
            readTableFromFile(freq2, size, filename);
            buildMinHeap(q2, letters, freq2, size);
            // printfreq(letters, freq2, size);
            compressedRoot = compress(q2); // here tree is built
            PrintCodes(compressedRoot, "");
            DecodeBitstream(filename, "decoded.txt", compressedRoot);
            break;
        default:
            break;
        }
    } while (choice != 0);

    return 0;
}