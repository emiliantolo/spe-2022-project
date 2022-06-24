file = open('wifi-st0-0-0.txt', 'r')
words = file.read().split()
c = 0
for w in words:
    if w == 'Retry':
        c += 1
print(c)
