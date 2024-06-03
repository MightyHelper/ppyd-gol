all: data/weak_scale_1.png data/weak_scale_2.png data/linear_in_os.png data/communication.png data/file.png

data/project.csv: main.py
	python3 main.py

data/weak_scale_1.png data/weak_scale_2.png: data/project.csv get_weak_scale.py
	python3 get_weak_scale.py

data/linear_in_os.png: data/project.csv get_linear_in_os.py
	python3 get_linear_in_os.py

data/communication.png: data/project.csv get_communications.py
	python3 get_communications.py

data/file.png: data/project.csv get_file.py
	python3 get_file.py
