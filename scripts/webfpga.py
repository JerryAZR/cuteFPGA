# MIT License
#
# Copyright (c) 2019 Auburn Ventures, LLC
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import sys
import datetime
from sys import argv
import re
import usb.core
import usb.util
from time import sleep
import textwrap

# Compression and blocking constants
MAX_REC_SIZE = 63
MAX_BLK_SIZE = 64
MAX_ZEROS    = 127

# Returns true if the binary bitstream is already
# in the compressed format. If the binary buffer starts
# with 0xFF00, then it's definitely uncompressed.
def is_compressed(input_bytes):
    return (input_bytes[0] != 0xFF and input_bytes[1] != 0x00)

# Compresses the input bitstream if it isn't compressed already
# (If it's already compressed, it just returns the same bytes.)
def compress(input_bytes):
    if is_compressed(input_bytes):
        return input_bytes

    # Open input file and load into array
    print("Reading input bitstream...")
    bitstream = input_bytes
    print(f"Read {len(bitstream)} bytes.\n")

    # (0xFF signifies the start/end of the header)
    data_segment = []
    data_segment = [0xFF, 0x00 ] 
    data_segment.extend("E+".encode('utf-8'))

    x = datetime.datetime.now()
    print (f"date: {x}")

    data_segment.extend(x.strftime("%c").encode('utf-8'))
    data_segment.extend("+shastaplus".encode('utf-8'))
    data_segment.append(0x00)
    data_segment.append(0xFF)

    # Locate the start of actual data in the input bitstream
    assert bitstream[0] == 0xFF, "first byte of input bitstream != 0xFF"
    print("Searching start of input bitstream data segment...")
    for index in range(len(bitstream)):
        # 0xFF marks the end of the input bitstream's header
        print(f"Byte {index}: {bitstream[index]}")
        if bitstream[index] == 0xFF and index != 0:
            index += 1
            break
    print(f"Found data segment start at byte {index}.\n")
    data_segment.extend(bitstream[index:])

    #######################################################################
    # compress
    #
    # Compressed data is stored in records. The first byte in a record is the
    # length. If the byte is 127 or less, then the following LEN-1 bytes are
    # non zero values of the data stream. If the byte is 128 or larger, (MSBit is set)
    # then the LEN masked with 0x7f is the number of zeros to be decompressed.
    # Records are repeated until EOF. Record maximum size can be set with
    # MAX_REC_SIZE. For WebFPGA, it is set to 63.

    index        = 0
    zero_count   = 0
    rec_number   = 0

    rec_buffer      = []
    compress_buffer = []

    rec_buffer.append(127)  # placeholder for length

    if data_segment[index] != 0:   # start zero flag with correct value
        zeros_flag = False
    else:
        zeros_flag = True
        
          
    while True:
        # once we hit a zero while collecting non-zeros, we flush record and switch to zero collecting
        if data_segment[index] == 0 and not zeros_flag:
            zeros_flag = True
            rec_buffer[0] = len(rec_buffer)  #len record
            #print(f"Compress A: Record is non zero, {index} {rec_number}#, len {len(rec_buffer)}.\n")
            rec_number += 1
            compress_buffer.extend(rec_buffer)   
            zero_count = 1   # we found a zero, so count
        else:
            if data_segment[index] != 0:       #check for zero
                if zeros_flag:                 # not a zero, but were counting zeros
                    zeros_flag = False
                    #print(f"Compress B: Record is zero, {index} {rec_number}#, #zeros {zero_count}.\n")
                    rec_number += 1
                    compress_buffer.append (128+zero_count) #set zero flag in MSBit
                    rec_buffer = []                # create new record of non zeros
                    rec_buffer.append(127)         # placehold for Length
                    
                rec_buffer.append(data_segment[index])
                # since we are counting non-zeros, check if max'ed bufer size
                if len(rec_buffer) == MAX_REC_SIZE-1:   # flush buffer if full
                    #print(f"Compress C: Record is non-zero, {index} {rec_number}#, len {len(rec_buffer)}.\n")
                    rec_number += 1
                    rec_buffer[0] = len(rec_buffer)     # set len of non zeros
                    compress_buffer.extend(rec_buffer)

                    rec_buffer = []          # create new record of non zeros
                    rec_buffer.append(127)   # placehold for Length
            else:                            # we got a zero
                if zero_count == MAX_ZEROS:
                    # flush zero count to output data stream
                    #print(f"Compress D: Record is zero, {index} {rec_number}#, #zeros {zero_count}.\n")
                    rec_number += 1
                    compress_buffer.append(128+zero_count)
                    zero_count = 1
                else:
                    zero_count += 1
                    
        index += 1
        if  index >= len(data_segment):
          break

    # now flush any remaining data in the buffer or zero counts
    if zeros_flag:
        print(f"Compress E: Final subrecord is zero rec# {rec_number}, #zeros {zero_count}.")
        rec_number += 1
        compress_buffer.append(128+zero_count)
    else:
        rec_buffer[0] = len(rec_buffer)
        print(f"Compress F: Final subrecord is non zero rec# {rec_number}, len {len(rec_buffer)}.")
        rec_number += 1
        compress_buffer.extend(rec_buffer)

    print("Compress G: Compressed file len: %6d, compression ratio %2.1f:1" % (len(compress_buffer),1/(len(compress_buffer)/len(bitstream))))

    print(f"Returning compressed bitstream...")
    return block(compress_buffer)

def block(compress_buffer):
    decompress_buffer = []
    index      = 0
    rec_number = 0

    while True:
        if compress_buffer[index] > 128:  #decompress zeros
            clen = compress_buffer[index] & 0x7f
            #print(f"Decompress A: Record is zero, {index} rec# {rec_number}#, #zeros {clen}.\n")
            for i in range(clen):
                decompress_buffer.append(0x0)
            index += 1
            rec_number += 1
        else:
            clen = compress_buffer[index] -1
            #print(f"Decompress B: Record is non-zero, {index} rec# {rec_number}#, len {clen}.\n")
            for i in range(clen):
                decompress_buffer.append(compress_buffer[i+index+1])
            index += clen + 1
            rec_number += 1

        if index >= len(compress_buffer):
            break
              
    print(f"Decompress B: number of records decompressed {rec_number} length {len(decompress_buffer)}.")
    print (f"decompressed: {decompress_buffer} \n")

    #######################################################################
    # blocking
    #
    # blocking is putting a group of records into a block.
    # This is done to help make transfers to the de-compressor code in
    # a MCU very easy to handle. For WebUSB we use 64 bytes for the block size. 
    # Each block contains complete records.
    # The first byte of a block is the block length, followed
    # by complete record(s), until max block size is reached.
    # No partial records are allowed.
    # Last block is marked with its length's byte MSBit set.

    index        = 0
    block_index  = 1

    block_buffer = []
    blocks_buffer= []

    block_number = 0
    record_number= 0

    block_buffer.append(127)  # placeholder for length

    while True:
        if compress_buffer[index] > 128:   # zero record
            #print(f"Blocks A: zero record {index} #{record_number}, # of zeros {compress_buffer[index]-128}.\n")
            if block_index == MAX_BLK_SIZE:
                #print(f"Blocks B: flush case, blk# {block_number} then move zero record to block buffer.\n")
                block_buffer[0] = block_index      # put block len at start of block
                block_number += 1
                blocks_buffer.extend(block_buffer)
                #print(f"Blocks B1: {block_buffer}\n")
                block_buffer = []
                block_buffer.append(127)  # placeholder for length
                block_index = 1
                block_buffer.append(compress_buffer[index]) 
                block_index += 1
            else:
                block_buffer.append(compress_buffer[index])
                block_index += 1
            index += 1
        else:       # non zero record
            #print(f"Blocks C: nonzero record {index} #{record_number}, len {compress_buffer[index]}.\n")
            if block_index + (compress_buffer[index]) >= MAX_BLK_SIZE+1:
                #print(f"Blocks D: flush first and start new block.\n")
                block_buffer[0] = block_index      # put block len at start of block
                block_index = 1
                block_number += 1
                blocks_buffer.extend(block_buffer)
                #print(f"Blocks D1: {block_buffer}\n")
                block_buffer = []
                block_buffer.append(127)  # placeholder for length

                # move current record to block buffer
                #print(f"Blocks E: flush case, blk# {block_number} then move non-zero record to block buffer.\n")
                for i in range(compress_buffer[index]):       # of non-zero transfers
                    block_buffer.append(compress_buffer[i+index])
                block_index += compress_buffer[index]
                #print(f"Blocks E1: {block_buffer}\n")

            else:      # move non-zero record to block buffer
                #print(f"Blocks F: Non-flush case, just move non-zero record to block buffer.\n")
                for i in range(compress_buffer[index]):       # of non-zero transfers
                    block_buffer.append(compress_buffer[i+index])
                block_index += compress_buffer[index]

            index = index + compress_buffer[index]
                
        
        record_number += 1
        if index >= len(compress_buffer):
            break

    print(f"Blocks G: Last block, flush it case.")
    block_buffer[0] = block_index + 128  # set last block flag with length
    blocks_buffer.extend(block_buffer)

    print(f"Blocks: Blocks {block_number+1}, records {record_number}.")
    #print (f"blocking: {blocks_buffer} \n")

    print(f"Returning blocks bitstream\n")
    return bytes(blocks_buffer)


COMMANDS = {
    "AT":  1,  "API": 3,  "APR": 4,  "APWE": 11, "APFU": 15,
    "AMQ": 20, "AMW": 24, "AMR": 25, "AMBE": 28, "AMWD": 29, "AFCIO": 30,
}

# https://www.beyondlogic.org/usbnutshell/usb6.shtml
BM_REQUEST_TYPES = {
  "vendor": 0x40,
  "device_to_host_AND_vendor": 0xC0
}

# Grab the first available WebFPGA device
def get_device():
    device = usb.core.find(idVendor=0x16D0, idProduct=0x0e6c)
    # print(device)
    if device is None:
        raise ValueError("Device not found")
    return device

# Issue a command to the USB device
def issue_command(device, command, wIndex=0, data=None):
    # Assemble USB xfer out
    bmRequestType = BM_REQUEST_TYPES["vendor"]
    bmRequest = 250 # user-defined by our firmware
    wValue = COMMANDS[command]

    # Run request
    bytes_written = device.ctrl_transfer(bmRequestType, bmRequest, wValue, wIndex, data)

    print(command + ": ", end="")
    return bytes_written

def expect(device, expected):
    bmRequestType = BM_REQUEST_TYPES["device_to_host_AND_vendor"]
    bmRequest = 249 # user-defined by our firmware
    wValue = 0x70   # our command
    wIndex = 0x81   # data index for command
    wLength = 128   # bytes to read

    ret = device.ctrl_transfer(bmRequestType, bmRequest, wValue, wIndex, wLength)
    result = "".join([chr(x) for x in ret]).rstrip(" \t\r\n\0")
    match = re.match(expected, result)

    if match:
        print(result)
        return result.strip("\00\0A")
    else:
        print(result + "(wanted " + expected + ")")

def handshake(device, command, expected, wIndex=0, data=None):
    issue_command(device, command, wIndex, data)
    return expect(device, expected)

# Set a MCU bit to 0 or 1
# toggle --> offset 0, set 1 --> offset 4, set 0 --> offset 8
def set_bit(dev, bit, value):
    bit += (4 if (value == 1) else 8)
    handshake(dev, "AFCIO", "^Done", wIndex=bit)

# Set a MCU bit to 0 or 1
# toggle --> offset 0, set 1 --> offset 4, set 0 --> offset 8
def toggle_bit(dev, bit):
    handshake(dev, "AFCIO", "^Done", wIndex=bit)

# Set all bits zero
# This is required because of a firmware bug on the MCU.
# The GPIO port direction is not set until we issue the toggle command.
def init_bits(dev):
    for bit in range(4):
        toggle_bit(dev, bit)
        set_bit(dev, bit, 0)

def SetBitstring(bitstring):
    bitstring = bitstring.lower()

    # valid bitstring
    match = re.match("[01x][01x][01x][01x]", bitstring)
    if not match and bitstring != "init":
        print(f"error: bitstring is invalid: {bitstring}")
        print( "       Try something like '0101'")
        sys.exit(1)

    # open usb device and validate connection
    print("Opening USB device...")
    dev = get_device()
    handshake(dev, "AT", "Hi")

    if bitstring == "init":
        init_bits(dev)
    else:
        # Set bits from bitstring
        print("\nSetting bits...")
        for bit in range(len(bitstring)):
            char = bitstring[bit]
            if char == "x":
                continue
            set_bit(dev, bit, int(char))

# Main flash routine:
# Flash device with bitstream
# If the bitstream is uncompressed, then compress it
def Flash(bitstream):
    print("Opening USB device...")
    dev = get_device()

    print("\nPreparing device for flashing...")
    prepare(dev)

    print("\nErasing device...")
    erase(dev)

    print("\nFlashing device...")
    flash(dev, compress(bitstream))

def prepare(device):
    handshake(device, "AT", "Hi")
    handshake(device, "API", "C_WEBUSB|CWEBUSB+")
    handshake(device, "APR", "000921|01010(4|5|6|7)")
    print("Found programmer.")

    print("Checking for FPGA module and its flash configuration...")
    handshake(device, "APWE", "wren")
    amq = handshake(device, "AMQ", ".*")

    assert len(amq) >= 9, "Bad AMQ response, too short."
    assert amq[0] == "S", "Flash device not supported."
    assert amq[6] == "H", "Flash device has bad Cascadia header."

def erase(device):
    handshake(device, "AMBE", "DONE")
    amq = handshake(device, "AMQ", ".*")

    assert len(amq) >= 9, "Bad AMQ response, too short."
    assert amq[0] == "S", "Flash device not supported."
    assert amq[5] == "W", "Flash device is write protected."
    assert amq[6] == "H", "Flash device has bad Cascadia header."
    assert amq[8] == "E", "Flash device is not erased."

def flash(device, buf):
    handshake(device, "AMW", "OK")

    idx = 0
    while idx < len(buf):
        # Read the block size,
        # then and use that to slice a block
        block_size = buf[idx]
        block = buf[idx:idx+block_size]
        idx += block_size

        # transmit the block
        res = issue_command(device, "AMWD", wIndex=0, data=block)
        expect(device, ".*")

        # print the device's response
        print("RESPONSE =>", res, "\n")

if __name__ == "__main__":
    with open(argv[1], "rb") as infile:
        Flash(infile.read())
