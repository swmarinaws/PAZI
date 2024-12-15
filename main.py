import subprocess
import shutil
from pathlib import Path

# Путь к assess.exe
assess_exe = Path("assess.exe")

# Путь к файлам .bin
input_folder = Path("output")
# Путь к исходной папке AlgorithmTesting
algorithm_testing_folder = Path("experiments/AlgorithmTesting")
# Путь к папке для сохранения всех результатов
results_archive_folder = Path("results_archive")
results_archive_folder.mkdir(parents=True, exist_ok=True)

# Размер битового потока
bitstream_size = '1000000'

# Обход файлов round_{i}.bin при 1 <= i <= 32
for i in range(1, 33):
    input_file = input_folder / f"round_{i}.bin"

    # Проверяем существование файла
    if not input_file.exists():
        print(f"Input file not found: {input_file}")
        continue

    # Команда для assess.exe
    args = [str(assess_exe), bitstream_size]
    print(f"Running: {' '.join(args)}")

    # Запуск assess.exe
    process = subprocess.Popen(
        args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
    )
    
    # Передача аргументов в assess.exe
    try:
        stdout, stderr = process.communicate(input=f"{input_file}\n", timeout=30000)
    except subprocess.TimeoutExpired:
        process.kill()
        print(f"Timeout expired for {input_file}")
        continue

    # Проверяем, что AlgorithmTesting создана
    if algorithm_testing_folder.exists():
        # Создаём уникальную папку для сохранения результатов
        destination_folder = results_archive_folder / f"round_{i}_results"
        shutil.copytree(algorithm_testing_folder, destination_folder)
        print(f"Results for round {i} saved to {destination_folder}")
    else:
        print(f"AlgorithmTesting folder not found after processing {input_file}")

print("Processing completed.")
