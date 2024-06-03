all: data/strong_scale_1.png data/strong_scale_2.png
data/project.csv:
	python3 main.py
data/strong_scale_1.png data/strong_scale_2.png: data/project.csv
	python3 get_strong_scale.py
