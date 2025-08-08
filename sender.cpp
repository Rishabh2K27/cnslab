// sender.cpp
#include <iostream>
#include <bitset>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

// Convert string to binary
string toBinary(const string& text) {
    string binary = "";
    for (char c : text)
        binary += bitset<8>(c).to_string();
    return binary;
}

// Hamming(12,8) encode function
string encodeHamming(const string& data) {
    int r = 4;
    int totalBits = 12;
    string encoded(totalBits + 1, '0'); // 1-based indexing

    for (int i = 1, j = 0; i <= totalBits; ++i) {
        if ((i & (i - 1)) != 0) {
            encoded[i] = data[j++];
        }
    }

    for (int i = 0; i < r; ++i) {
        int pos = 1 << i;
        int parity = 0;
        for (int j = 1; j <= totalBits; ++j) {
            if (j & pos)
                parity ^= (encoded[j] - '0');
        }
        encoded[pos] = parity + '0';
    }

    return encoded.substr(1);
}

void flipRandomBit(string& data) {
    int pos = rand() % data.size();
    cout << "Flipping bit at position: " << pos << endl;
    data[pos] = (data[pos] == '0') ? '1' : '0';
}

int main() {
    srand(time(0));

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5050);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.58"); // loopback

    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Connection failed\n";
        return 1;
    }

    string input;
    cout << "Enter message to send: ";
    getline(cin, input);

    string binaryData = toBinary(input);
    string encoded = "";

    // Encode each byte (8 bits) separately
    for (size_t i = 0; i < binaryData.size(); i += 8) {
        string byte = binaryData.substr(i, 8);
        encoded += encodeHamming(byte);
    }

    cout << "Encoded Hamming Code: " << encoded << endl;

    char choice;
    cout << "Introduce error? (y/n): ";
    cin >> choice;
    if (choice == 'y') {
        flipRandomBit(encoded);
        cout << "Corrupted code: " << encoded << endl;
    }

    uint32_t len = encoded.size();
    send(sock, &len, sizeof(len), 0);
    send(sock, encoded.c_str(), len, 0);

    close(sock);
    return 0;
}
