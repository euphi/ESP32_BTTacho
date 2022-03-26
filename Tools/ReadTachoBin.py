#!/usr/bin/python3

import struct
import csv
from array import array

fname = 'LOG_0039.BIN'
format_struct="<LffffIBxxx"
format_names=("Timestamp", "Speed", "Temperatur", "Gradient", "Höhe", "distance", "Puls")
print(str(struct.calcsize(format_struct)) + u" bytes needed per Set")

with open(fname,'rb') as file, open('output.csv', 'w', newline='') as csvfile:
    csvwriter = csv.DictWriter(csvfile, fieldnames=format_names, dialect='excel')
    csvwriter.writeheader()
    while(1):
      pdata = array( 'B', file.read(struct.calcsize(format_struct)) )  # buffer the file
      tuple_dataset=dict(zip(format_names,struct.unpack(format_struct,pdata)))
      print(tuple_dataset)
      csvwriter.writerow(tuple_dataset)

