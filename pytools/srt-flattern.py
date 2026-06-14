#!/usr/bin/env python3
"""
SRT Flattener - Collapses SRT subtitle entries into single lines

[Provided by Deepseek]

I need a python program to make a .srt file flat. I mean, to collapse time-stamp and corresponding text into one line.
	
For example:

```
1
00:00:00,240 --> 00:00:03,920
sentence one


2
00:00:04,120 --> 00:00:06,920
sentence two


3
00:00:07,040 --> 00:00:08,640
sentence three
```

will be collapse to:

```
#1[00:00:00.240] sentence one
#2[00:00:04.120] sentence two
#3[00:00:07.040] sentence three
```
"""


import re
import sys
import argparse

def flatten_srt(input_file, output_file=None):
	"""
	Flatten SRT file by combining subtitle index, timestamp, and text into one line.
	
	Args:
		input_file: Path to input .srt file or file-like object
		output_file: Path to output file (if None, print to stdout)
	"""
	# Read input
	if isinstance(input_file, str):
		with open(input_file, 'r', encoding='utf-8') as f:
			content = f.read()
	else:
		content = input_file.read()
	
	# Split into entries (separated by blank lines)
	entries = re.split(r'\n\s*\n', content.strip())
	
	result_lines = []
	
	for entry in entries:
		lines = entry.strip().split('\n')
		if len(lines) >= 2:
			# Extract index
			index = lines[0].strip()
			
			# Extract timestamp (first timestamp in the line)
			timestamp_line = lines[1]
			timestamp_match = re.search(r'(\d{2}:\d{2}:\d{2},\d{3})', timestamp_line)
			
			if timestamp_match:
				timestamp = timestamp_match.group(1)
				timestamp = timestamp.replace(',', '.')
			else:
				timestamp = "??:??:??,???"
			
			# Extract text (everything after timestamp line)
			text_lines = lines[2:] if len(lines) > 2 else []
			text = ' '.join(text_lines).strip()
			
			# Format the output line
			result_lines.append(f"#{index}[{timestamp}] {text}")
	
	# Write output
	output_content = '\n'.join(result_lines)
	
	if output_file:
		with open(output_file, 'w', encoding='utf-8') as f:
			f.write(output_content)
	else:
		print(output_content)
	
	return output_content

def main():
	parser = argparse.ArgumentParser(
		description='Flatten SRT subtitle files by collapsing timestamps and text into single lines'
	)
	parser.add_argument('input', help='Input .srt file')
	parser.add_argument('-x', '--in-place', action='store_true', 
					   help='Modify file in place (overwrites input file)')
	parser.add_argument('-o', '--output', help='Output file (default: print to stdout)')
	
	args = parser.parse_args()
	
	if args.in_place:
		output_file = args.input
	else:
		output_file = args.output
	
	try:
		flatten_srt(args.input, output_file)
		if output_file:
			print(f"Successfully flattened '{args.input}' to '{output_file}'", file=sys.stderr)
	except Exception as e:
		print(f"Error: {e}", file=sys.stderr)
		sys.exit(1)

if __name__ == "__main__":
	main()
