import re
from pathlib import Path

# Папка, где хранятся результаты
results_archive_folder = Path("results_archive")
# Итоговый файл для статистики
summary_file = results_archive_folder / "summary.csv"

# Регулярное выражение для строки с тестом
test_line_pattern = re.compile(
    r".*?(\d+/\d+)\s+\*?\s+(\w+)"
)

# Регулярные выражения для определения минимальных значений
min_pass_rate_default_pattern = re.compile(
    r"he minimum pass rate for each statistical test.*?(\d+).*?(\d+)\s+",
    re.DOTALL
)
min_pass_rate_excursion_pattern = re.compile(
    r"The minimum pass rate for the random excursion.*?(\d+).*?(\d+)\s+",
    re.DOTALL
)

# Открываем итоговый файл для записи
with summary_file.open("w", encoding="utf-8") as summary:
    # Записываем заголовок CSV файла
    summary.write("Round,Test,Proportion,Passed\n")

    # Сортируем папки раундов по порядку
    round_folders = sorted(
        results_archive_folder.glob("round_*_results"),
        key=lambda x: int(x.stem.split("_")[1])
    )

    # Проходим по всем раундам
    for round_folder in round_folders:
        round_name = round_folder.stem.split("_")[1]  # stem возвращает имя файла
        final_report_path = round_folder / "finalAnalysisReport.txt"

        # Проверяем, существует ли файл finalAnalysisReport.txt
        if not final_report_path.exists():
            print(f"Warning: {final_report_path} not found. Skipping.")
            continue

        # Считываем файл отчета
        with final_report_path.open("r", encoding="utf-8") as report_file:
            lines = report_file.readlines()

        # Парсим минимальные значения для тестов
        min_pass_rate = {
            "default": (),
            "RandomExcursions": (),
            "RandomExcursionsVariant": (),
        }

        full_text = "".join(lines)
        default_match = min_pass_rate_default_pattern.search(full_text)
        excursion_match = min_pass_rate_excursion_pattern.search(full_text)

        if default_match:
            min_pass_rate["default"] = tuple(map(int, default_match.groups()))
        else:
            print(f"ERROR: Default minimum pass rate not found in {round_name}.")

        if excursion_match:
            min_pass_rate["RandomExcursions"] = tuple(map(int, excursion_match.groups()))
            min_pass_rate["RandomExcursionsVariant"] = tuple(map(int, excursion_match.groups()))
        else:
            print(f"ERROR: Excursion minimum pass rate not found in {round_name}.")

        print(min_pass_rate)
        # Парсим строки
        recording = False
        for line in lines:
            # Начинаем читать после раздела "RESULTS FOR THE UNIFORMITY..."
            if "RESULTS FOR THE UNIFORMITY" in line:
                recording = True
                continue
            if not recording:
                continue

            # Останавливаем запись после строки с минимальными значениями
            if "The minimum pass rate for each statistical test" in line:
                break

            # Ищем строки с данными тестов
            match = test_line_pattern.search(line)
            if match:
                proportion, test_name = match.groups()

                passed_sequences, total_sequences = map(int, proportion.split("/"))
                min_pass, min_total = min_pass_rate.get(
                    test_name, min_pass_rate["default"]
                )

                # Определяем, прошел ли тест
                passed = passed_sequences >= min_pass

                # Записываем результат в файл
                summary.write(
                    f"{round_name},{test_name},{proportion},{'Yes' if passed else 'No'}\n"
                )

print(f"Results summary saved to {summary_file}")
