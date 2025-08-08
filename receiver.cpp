// receiver.cpp
#include <iostream>
#include <bitset>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

int detectAndCorrect(string& code) {
    int r = 4;
    int errorPos = 0;
    for (int i = 0; i < r; ++i) {
        int pos = 1 << i;
        int parity = 0;
        for (int j = 1; j <= code.length(); ++j) {
            if (j & pos)
                parity ^= (code[j - 1] - '0');
        }
        if (parity)
            errorPos += pos;
    }

    if (errorPos != 0) {
        cout << "Error at position: " << errorPos << ", correcting...\n";
        code[errorPos - 1] = (code[errorPos - 1] == '0') ? '1' : '0';
    }

    return errorPos;
}

string decodeHamming(const string& code) {
    string data = "";
    for (int i = 1; i <= code.length(); ++i) {
        if ((i & (i - 1)) != 0) {
            data += code[i - 1];
        }
    }
    return data;
}

string binaryToText(const string& binary) {
    string text = "";
    for (size_t i = 0; i + 7 < binary.size(); i += 8) {
        bitset<8> byte(binary.substr(i, 8));
        text += char(byte.to_ulong());
    }
    return text;
}

int main() {
    int serverSock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr{}, clientAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5050);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSock, 1);
    cout << "Receiver listening on port 5050...\n";

    socklen_t clientSize = sizeof(clientAddr);
    int clientSock = accept(serverSock, (struct sockaddr*)&clientAddr, &clientSize);

    uint32_t len = 0;
    recv(clientSock, &len, sizeof(len), 0);

    char* buffer = new char[len + 1];
    recv(clientSock, buffer, len, 0);
    buffer[len] = '\0';

    string received(buffer);
    delete[] buffer;

    cout << "Received code: " << received << endl;

    string fixedBinary = "";
    for (size_t i = 0; i < received.size(); i += 12) {
        string block = received.substr(i, 12);
        detectAndCorrect(block);
        fixedBinary += decodeHamming(block);
    }

    string original = binaryToText(fixedBinary);

    cout << "Decoded binary: " << fixedBinary << endl;
    cout << "Reconstructed message: " << original << endl;

    close(clientSock);
    close(serverSock);
    return 0;
}
