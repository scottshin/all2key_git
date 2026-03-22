#!/usr/bin/env python3
import json
import re

def convert_dict_to_json(input_file, output_file):
    dictionary = {}

    with open(input_file, 'r', encoding='utf-16') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue

            # Match "key"="value" pattern
            match = re.match(r'"([^"]+)"="([^"]+)"', line)
            if match:
                key, value = match.groups()
                if key not in dictionary:
                    dictionary[key] = []
                dictionary[key].append(value)

    # Sort keys for consistency
    sorted_dict = {k: dictionary[k] for k in sorted(dictionary.keys())}

    with open(output_file, 'w', encoding='utf-8') as f:
        json.dump(sorted_dict, f, ensure_ascii=False, indent=2)

if __name__ == "__main__":
    input_file = "Dictionary/SampleIMESimplifiedQuanPin.txt"
    output_file = "Dictionary/SampleIMESimplifiedQuanPin.json"
    convert_dict_to_json(input_file, output_file)
    print(f"Converted {input_file} to {output_file}")