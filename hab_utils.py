import time, struct

binarypacketformat = "<BHHffHBBBB"
binarypacketlength = 19

def decode_binary_packet(data):
    try:
        unpacked = struct.unpack(binarypacketformat, data)
    except:
        print "Wrong string length. Packet contents:"
        print ":".join("{:02x}".format(ord(c)) for c in data)
        return

    payload_id = unpacked[0]
    counter = unpacked[1]
    time_biseconds = unpacked[2]
    latitude = unpacked[3]
    longitude = unpacked[4]
    altitude = unpacked[5]
    speed = unpacked[6]
    batt_voltage = unpacked[7]
    sats = unpacked[8]
    temp = unpacked[9]


    time_string = time.strftime("%H:%M:%S", time.gmtime(time_biseconds*2))

    batt_voltage_float = 0.5 + 1.5*batt_voltage/255.0

    #print "Decoded Packet: %s  %f,%f %d %.2f %d" % (time_string, latitude, longitude, altitude, speed*1.852, sats)

    print "\n\nDecoded Packet: %s" % (":".join("{:02x}".format(ord(c)) for c in data))
    print "      ID: %d" % payload_id
    print " Seq No.: %d" % counter
    print "    Time: %s" % time_string
    print "     Lat: %.5f" % latitude
    print "     Lon: %.5f" % longitude
    print "     Alt: %d m" % altitude
    print "   Speed: %.1f kph" % (speed*1.852)
    print "    Sats: %d" % sats
    print "    Batt: %.3f" % batt_voltage_float
    print "    Temp: %d" % temp
    print " "