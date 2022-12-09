import os
import time

file_name = "lexical_analysis"
test_dir = "./test"
token_dir = "./token"

os.system("g++ {}.cpp -o {}.exe".format(file_name, file_name))

test_points = os.listdir(test_dir)
i = 1

for test_name in test_points:
    print("测试点#{}".format(i))
    begin = time.time()
    os.system(file_name + ".exe " + test_dir + "/" + test_name +\
        " " + token_dir + "/" + test_name + ".lex")
    end = time.time()
    print("用时 {}s".format(end - begin))
    i += 1