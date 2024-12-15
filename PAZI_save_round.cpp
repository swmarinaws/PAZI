#include <iostream>  
#include <fstream>   
#include <iomanip>   
#include <cstdint>   
#include <vector>   
#include <string>
#include <bitset>

using namespace std;

template<typename T>
void printBinaryRepresentation(T value) {
    // Получаем указатель на байты числа
    uint8_t* bytePtr = reinterpret_cast<uint8_t*>(&value);
    size_t numBytes = sizeof(T);

    // Вывод байтов числа
    for (size_t i = 0; i < numBytes; ++i) {
        cout << bitset<8>(bytePtr[i]) << " ";
    }
}

bool isLittleEndian() {
    uint16_t testValue = 1;
    uint8_t* bytePtr = reinterpret_cast<uint8_t*>(&testValue); // указатель на первый байт памяти, занимаемой testValue
    return *bytePtr == 1;    // Если младший байт первый, то little-endian
}

uint64_t convertEndian(uint64_t value) {
    return ((value & 0xFF00000000000000ULL) >> 56) |
        ((value & 0x00FF000000000000ULL) >> 40) |
        ((value & 0x0000FF0000000000ULL) >> 24) |
        ((value & 0x000000FF00000000ULL) >> 8) |
        ((value & 0x00000000FF000000ULL) << 8) |
        ((value & 0x0000000000FF0000ULL) << 24) |
        ((value & 0x000000000000FF00ULL) << 40) |
        ((value & 0x00000000000000FFULL) << 56);
}

void encryptWithRounds(uint64_t ai, uint32_t* key, int rounds, vector<vector<uint32_t>>& buffers, size_t bufferLimit) {
    uint32_t sum = 0, v0 = static_cast<uint32_t>(ai & 0xFFFFFFFF), v1 = static_cast<uint32_t>(ai >> 32);
    uint32_t delta = 0x9e3779b9;

    // Сохраняем открытый текст
    buffers[0].push_back(v0);
    buffers[0].push_back(v1);
    // Если буфер достиг лимита, записываем его в файл
    if (buffers[0].size() >= bufferLimit) {
        string filename = "./output/round_0.bin";
        ofstream file(filename, ios::binary | ios::app);
        if (!file.is_open()) {
            cerr << "Error: Unable to open file " << filename << endl;
        }
        file.write(reinterpret_cast<char*>(buffers[0].data()), buffers[0].size() * sizeof(uint32_t));
        file.close();

        // Очищаем буфер после записи
        buffers[0].clear();
    }

    for (int i = 1; i <= rounds; i++) {
        sum += delta;
        v0 += ((v1 << 4) + key[0]) ^ (v1 + sum) ^ ((v1 >> 5) + key[1]);
        // Добавляем данные в соответствующий буфер
        buffers[i*2-1].push_back(v1);
        buffers[i*2-1].push_back(v0);
        v1 += ((v0 << 4) + key[2]) ^ (v0 + sum) ^ ((v0 >> 5) + key[3]);

        // Добавляем данные в соответствующий буфер
        buffers[i*2].push_back(v0);
        buffers[i*2].push_back(v1);

        // Если буфер достиг лимита, записываем его в файл
        if (buffers[i*2-1].size() >= bufferLimit) {
            string filename = "./output/round_" + to_string(i*2-1) + ".bin";
            ofstream file(filename, ios::binary | ios::app);
            if (!file.is_open()) {
                cerr << "Error: Unable to open file " << filename << endl;
                continue;
            }
            file.write(reinterpret_cast<char*>(buffers[i * 2 - 1].data()), buffers[i * 2 - 1].size() * sizeof(uint32_t));
            file.close();

            // Очищаем буфер после записи
            buffers[i * 2 - 1].clear();

            filename = "./output/round_" + to_string(i * 2) + ".bin";
            ofstream file2(filename, ios::binary | ios::app);
            if (!file.is_open()) {
                cerr << "Error: Unable to open file " << filename << endl;
                continue;
            }
            file2.write(reinterpret_cast<char*>(buffers[i * 2].data()), buffers[i * 2].size() * sizeof(uint32_t));
            file2.close();

            // Очищаем буфер после записи
            buffers[i * 2].clear();
        }
    }

    cout << "encrypted: v0 = " << v0 << " v1 = " << v1 << endl;
}

void decrypt(uint32_t* values, uint32_t* key) {

    uint32_t delta = 0x9e3779b9;
    uint32_t v0 = values[0], v1 = values[1],
        sum = delta << 5, i;

    for (i = 0; i < 32; i++) {
        v1 -= ((v0 << 4) + key[2]) ^ (v0 + sum) ^ ((v0 >> 5) + key[3]);
        v0 -= ((v1 << 4) + key[0]) ^ (v1 + sum) ^ ((v1 >> 5) + key[1]);
        sum -= delta;
    }

    cout << "decrypted: " << convertEndian(((static_cast<uint64_t>(v1) << 32) | v0)) << endl;
    cout << "binary: ";
    printBinaryRepresentation(((static_cast<uint64_t>(v1) << 32) | v0));
    cout << endl;

}

void flushBuffers(vector<vector<uint32_t>>& buffers, int rounds) {
    for (int i = 0; i <= rounds*2; i++) {
        if (!buffers[i].empty()) {
            string filename = "./output/round_" + to_string(i) + ".bin";
            ofstream file(filename, ios::binary | ios::app);
            if (!file.is_open()) {
                cerr << "Error: Unable to open file " << filename << endl;
                continue;
            }
            file.write(reinterpret_cast<char*>(buffers[i].data()), buffers[i].size() * sizeof(uint32_t));
            file.close();

            // Очищаем буфер после записи
            buffers[i].clear();
        }
    }
}

int main() {
    uint64_t vn = 4194303;  // Максимальное значение ai 4194303 для 32мб
    uint32_t key[4] = { 1, 2, 3, 4 };
    int rounds = 32;
    size_t bufferLimit = 5000;  // Лимит данных в буфере (количество элементов)

    // Инициализируем буферы для последующей записи в файлы
    vector<vector<uint32_t>> buffers(rounds*2 + 1);

    for (uint64_t ai_n = 511; ai_n <= 511; ai_n++) {
        //cout << "Input data: " << ai << endl;
        uint64_t ai;
        if (isLittleEndian())
            ai = convertEndian(ai_n);
        /*cout << "binary: ";
        printBinaryRepresentation(ai);
        cout << endl;*/
        if (ai_n % 10000 == 0) {
            cout << "Processing ai_n = " << ai_n << endl;
            cout << "Processing ai = " << ai << endl;
        }
        encryptWithRounds(ai, key, rounds, buffers, bufferLimit);
    }

    // Записываем остатки данных из буферов
    flushBuffers(buffers, rounds);

    cout << "Computation complete. Data saved in separate binary files for each round." << endl;

    //uint32_t v[2] = {1583827400, 2584619314};
    //decrypt(v, key);

    return 0;
}
