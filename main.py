import serial.tools.list_ports
import argparse
import os
import time


parser = argparse.ArgumentParser(description='Send commend from .txt file'
                                             'to serial port and save the'
                                             ' answer to .bin file.')

parser.add_argument('-p', '--Port', type=str, required=False, default='/dev/ttyUSB0')
parser.add_argument('Path', metavar='path', type=str, default='comend.txt', nargs='?',
                    help='Path to .txt file with commands')
args = parser.parse_args()
cmd_path = args.Path

if not os.path.exists(cmd_path):
    print("Command file has not been found")

ports = list(serial.tools.list_ports.comports())

bin_path = time.strftime('%Y%m%d_%H%M', time.localtime())+'.bin'

ser = serial.Serial(args.Port, 9600, timeout=0.01)
# '/dev/ttyUSB0'

with open(bin_path, 'wb') as ans_file:
    with open(cmd_path, 'r') as cmd:
        for line in cmd:
            ser.write(line.encode())
            for ans in ser.readline():
                ans_file.write(ans)



ser.close()
