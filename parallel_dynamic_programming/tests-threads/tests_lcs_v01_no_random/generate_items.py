import os
import random

data_size_set = [3200]
#data_size_set = [4, 16]
random_range_set = [0.10, 0.20, 0.30, 0.40, 0.50]

for random_range in random_range_set:
    for data_size in data_size_set:
        max_random = int(data_size * random_range)
        fout = open('data_size_' + str(data_size) + '_max_random_' + str(max_random) + '.pl', 'w')
        fout.write('data_call(' + str(data_size) + ',' + str(data_size) + ').\n\n')
        random.seed()
        for i in range(0, data_size + 1):
            fout.write('item1(' + str(i) + ',' + str(random.randint(0, max_random)) + '). \n')
            fout.write('item2(' + str(i) + ',' + str(random.randint(0, max_random)) + '). \n\n')
        fout.close()
        # all items different
        # fout = open('data_size_' + str(data_size) + '_all_different.pl', 'w')
        # fout.write('data_call(' + str(data_size) + ',' + str(data_size) + ').\n\n')
        # for i in range(0, data_size + 1):
        #     fout.write('item1(' + str(i) + ',' + str(0) + '). \n')
        #     fout.write('item2(' + str(i) + ',' + str(1) + '). \n\n')
            


