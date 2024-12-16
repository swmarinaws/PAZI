#include <stdio.h>
#include <stdlib.h>
#include "unif01.h"
#include "bbattery.h"

// Глобальные переменные для хранения данных
unsigned int *buffer = NULL;
size_t buffer_size = 0;
size_t current_index = 0;

// Функция чтения данных из буфера
unsigned int ReadBinary(void) {
    if (current_index >= buffer_size) {
        return 0; // Конец данных
    }
    return buffer[current_index++];
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <binary file>\n", argv[0]);
        return 1;
    }

    // Открываем бинарный файл
    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    // Определяем размер файла
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    // Проверяем, что файл делится на 4 байта
    if (file_size % sizeof(unsigned int) != 0) {
        printf("File size is not a multiple of 4 bytes.\n");
        fclose(file);
        return 1;
    }

    // Читаем весь файл в память
    buffer_size = file_size / sizeof(unsigned int);
    buffer = (unsigned int *)malloc(buffer_size * sizeof(unsigned int));
    if (!buffer) {
        perror("Memory allocation failed");
        fclose(file);
        return 1;
    }

    if (fread(buffer, sizeof(unsigned int), buffer_size, file) != buffer_size) {
        perror("Error reading file");
        free(buffer);
        fclose(file);
        return 1;
    }
    fclose(file);

    // Создаем генератор
    unif01_Gen *gen = unif01_CreateExternGenBits("Binary File Reader", ReadBinary);
    // Запускаем статистические тесты
    bbattery_Crush(gen);
    // Удаляем генератор и освобождаем память
    unif01_DeleteExternGenBits(gen);
    free(buffer);

    return 0;
}

