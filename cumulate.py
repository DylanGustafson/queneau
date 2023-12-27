import sys

with open(sys.argv[-1]) as f:
	lst = f.read().splitlines()

total = 0
if len(sys.argv) > 2:
	total = int(sys.argv[-2])

for i in range(len(lst)):
	lp = lst[i].split('-to-')
	if len(lp) != 2:
		continue
	[ulim, count] = lp[1].split(': ')
	total += int(count)
	print(f'{ulim}: {total}')


