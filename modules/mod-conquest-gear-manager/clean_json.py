#!/usr/bin/env python3
"""
Script to clean JSON files in s5 directory
Removes: buffs, classId, genderId, level, phase, raceId, shapeshiftForm
"""

import json
import os
import glob

def clean_json_file(filepath):
    """Clean a single JSON file"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            data = json.load(f)
        
        # Remove unwanted fields
        fields_to_remove = ['buffs', 'classId', 'genderId', 'level', 'phase', 'raceId', 'shapeshiftForm']
        for field in fields_to_remove:
            if field in data:
                del data[field]
        
        # Write back
        with open(filepath, 'w', encoding='utf-8') as f:
            json.dump(data, f, ensure_ascii=False, indent=2)
        
        print(f"Cleaned: {filepath}")
        return True
    except Exception as e:
        print(f"Error cleaning {filepath}: {e}")
        return False

def main():
    """Clean all JSON files in s5 directory"""
    base_dir = os.path.join(os.path.dirname(__file__), 'data', 'bis_by_class', 's5')
    
    if not os.path.exists(base_dir):
        print(f"Directory not found: {base_dir}")
        return
    
    json_files = glob.glob(os.path.join(base_dir, '**', '*.json'), recursive=True)
    
    print(f"Found {len(json_files)} JSON files to clean")
    
    cleaned = 0
    for json_file in json_files:
        if clean_json_file(json_file):
            cleaned += 1
    
    print(f"\nCleaned {cleaned}/{len(json_files)} files")

if __name__ == '__main__':
    main()

