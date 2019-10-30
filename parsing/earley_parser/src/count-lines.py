import os



total = 0
code = 0

paths = ["../incl", "../src"]

for path in paths:
	for file in os.listdir(path):
		if file in ["utf8", "getopt"]:
			continue
		with open(path+'/'+file, "r") as f:
			for line in f:
				total+=1
				line = line.split()
				if len(line) > 0:
					if line[0][0] not in ['/', '*']:
						code+=1

print "lines total:\t~", total
print "lines code:\t~", code
print "comments:\t~", int((float(total-code)/total)*100), "%"
