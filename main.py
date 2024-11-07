import glob

venues = {}
for file in glob.glob("Data_Streaming_Compression/documents/*"):
    venue = file.split("]")[1][1:]
    venues.setdefault(venue, 0)
    venues[venue] += 1

total = 0
for venue in venues:
    print(venue, venues[venue])
    total += venues[venue]
    
print(total, len(glob.glob("Data_Streaming_Compression/documents/*")))