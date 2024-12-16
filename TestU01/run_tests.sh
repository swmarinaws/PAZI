#!/bin/bash

# Создать папку для сохранения результатов, если её нет
mkdir -p results

# Цикл от 0 до 64
for ((i=0; i<=64; i++)); do
    # Имя входного файла
    input_file="output/round_${i}.bin"

    # Проверяем, существует ли входной файл
    if [[ -f "$input_file" ]]; then
        echo "Processing $input_file..."

        # Запускаем программу и фильтруем результаты
        ./read_binary "$input_file" | awk '/========= Summary results of SmallCrush =========/ {printit=1} printit' > "results/round_${i}_results.txt"

        echo "Results saved to results/round_${i}_results.txt"
    else
        echo "File $input_file does not exist. Skipping..."
    fi
done

echo "Processing completed. Results are in the 'results' folder."


