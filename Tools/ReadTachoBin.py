#!/usr/bin/python3

import struct

from array import array

fname = 'LOG_0001.BIN'
format_struct="<LffffIBxxx"
format_names=("Timestamp", "Speed", "Temperatur", "Gradient", "Höhe", "distance", "Puls")
print(str(struct.calcsize(format_struct)) + u" bytes needed per Set")

with open(fname,'rb') as file:
    while(1):
      pdata = array( 'B', file.read(struct.calcsize(format_struct)) )  # buffer the file
      tuple_dataset=dict(zip(format_names,struct.unpack(format_struct,pdata)))
      print(tuple_dataset)
