import os
import random

'''data_size_set = [500, 1000, 1500]
#data_size_set = [4]
random_range_set = [0.50, 0.75, 1.00]
k_ratio_set = [0.50, 1.00, 1.50] '''

data_size_set = [1600]
random_range_set = [0.10, 0.20, 0.30, 0.40, 0.50]
k_ratio_set = [2] 

for random_range in random_range_set:
    for data_size in data_size_set:
        max_random = int(data_size * random_range)
        for k_ratio in k_ratio_set:
            k = int(data_size * k_ratio)
            fout = open('data_size_' + str(data_size) + '_max_random_' + str(max_random) + '_k_' + str(k) + '.pl', 'w')
            fout.write('data_call(' + str(data_size) + ',' + str(k) + ').\n\n')
            random.seed()
            for i in range(0, data_size + 1):
                fout.write('item(' + str(i) + ',' + str(random.randint(1, max_random)) + ',' + str(random.randint(1, max_random))+ '). \n')
            fout.close()
            ##### all items different - increasing
            # fout = open('data_size_' + str(data_size) + '_k_' + str(k) + '_inc.pl', 'w')
            # fout.write('data_call(' + str(data_size) + ',' + str(k) + ').\n\n')
            # for i in range(0, data_size + 1):
            #     fout.write('item(' + str(i) + ',' + str(i + 1) + ',' + str(i + 1) + '). \n')
            ###### all items different -decreasing
            # fout = open('data_size_' + str(data_size) + '_k_' + str(k) + '_dec.pl', 'w')
            # fout.write('data_call(' + str(data_size) + ',' + str(k) + ').\n\n')
            # for i in range(0, data_size + 1):
            #     fout.write('item(' + str(i) + ',' + str(data_size + 1- i) + ',' + str(data_size + 1- i) + '). \n')
            


