import sys
import socket


_, ip_address, port, filename = sys.argv

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect((ip_address, int(port)))

ProtocolVersion,\
MappingInfoChecksum,\
MappingInfoRequest,\
MappingInfoFighterKinds,\
MappingInfoFighterStatusKinds,\
MappingInfoStageKinds,\
MappingInfoHitStatusKinds,\
MappingInfoRequestComplete,\
MatchStart,\
MatchResume,\
MatchEnd,\
TrainingStart,\
TrainingResume,\
TrainingReset,\
TrainingEnd,\
FighterState = range(16)

# Try to resume match/training session
client.send(bytes([MatchResume, TrainingResume]))

while True:
    msg = client.recv(1)
    if msg[0] == FighterState:
        client.recv(29)
    elif msg[0] in (TrainingEnd, MatchEnd):
        pass
    elif msg[0] in (MatchStart, MatchResume, TrainingStart, TrainingResume):
        start_msg = msg[0]
        break


with open(filename, "wb") as f:
    if start_msg in (MatchStart, MatchResume):
        stage_h, stage_l, player_count = client.recv(3)
        entry_ids = client.recv(player_count)
        fighter_ids = client.recv(player_count)
        tags = list()
        for player in range(player_count):
            msg = client.recv(1)
            tags.append(client.recv(msg[0]))

        blob = bytes([start_msg, stage_h, stage_l, player_count]) +\
            entry_ids + fighter_ids

        for tag in tags:
            blob += bytes([len(tag)])
            blob += tag

        print("MatchStart/MatchResume")
        print(blob)
        f.write(b'\x00')
        f.write(bytes([len(blob)]))
        f.write(blob)

    if start_msg in (TrainingStart, TrainingResume):
        blob = bytes([start_msg]) + client.recv(4)

        print("TrainingStart/TrainingResume")
        print(blob)
        f.write(b'\x00')
        f.write(bytes([len(blob)]))
        f.write(blob)

    while True:
        msg = client.recv(1)
        if msg[0] in (TrainingEnd, MatchEnd):

            print("TrainingEnd/MatchEnd")
            f.write(b'\x00')
            f.write(b'\x01')
            f.write(msg)
            break

        if msg[0] == FighterState:
            blob = msg + client.recv(29)
            f.write(b'\x10\x00')
            f.write(bytes([len(blob)]))
            f.write(blob)
