import json




input_file = "skk_hiragana_kanji.json"
output_file = "output.txt"


with open(input_file, "r", encoding="utf-8") as f:
    data = json.load(f)

with open(output_file, "w", encoding="utf-8") as f:
    for key, values in data.items():
        if isinstance(values, list):
            for v in values:
                f.write(f"\"{key}\"=\"{v}\"\r\n")
        else:
            f.write(f"\"{key}\"=\"{values}\"\r\n")

print("변환 완료:", output_file)