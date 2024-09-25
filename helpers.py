'''
Author: Jacob Collins
Description:
*   Print Progress Bar is a function to print a nice
    progress bar in the console.
Sources:
*   [Progress Loading Bar](https://stackoverflow.com/questions/3173319/text-progress-bar-in-terminal-with-block-characters) 
    by [Greenstick](https://stackoverflow.com/users/2206251/greenstick)
'''
import numpy as np

def match_list_lengths(y_lists):
    max_len = max(len(y_list) for y_list in y_lists)
    for y_list in y_lists:
        while len(y_list) < max_len:
            y_list.append(0)
    return y_lists

def compl_cum_dist(y):
  p_y = [i/sum(y) for i in y]
  ccd_y = [1 - sum(p_y[:i]) for i in range(len(p_y))]
  return ccd_y

# Computes means index-wise
# If a list is shorter than others, its values are extended at undefined points
# [[5, 5, 5, 5, 5],
#  [3, 3, 3, 3]]
# => [4, 4, 4, 4, 5]
def mean_across_lists(y_lists):
    max_len = max(len(y_list) for y_list in y_lists)
    means = []
    for i in range(max_len):
        pos_vals = []
        for y_list in y_lists:
            if len(y_list) <= i:
                pos_vals.append(y_list[len(y_list)-1])
            else:
                pos_vals.append(y_list[i])
        means.append(np.mean(pos_vals))
    return means

def mean_across_like_lists(y_lists):
    y_lists = match_list_lengths(y_lists)
    max_len = max(len(y_list) for y_list in y_lists)
    means = []
    for i in range(max_len):
        pos_vals = []
        for y_list in y_lists:
            if len(y_list) <= i:
                continue
            else:
                pos_vals.append(y_list[i])
        means.append(np.mean(pos_vals))
    return means

# Print iterations progress
def printProgressBar (
    iteration, 
    total, 
    prefix = '',  
    suffix = '',  
    decimals = 1,  
    length = 100,  
    fill = '█',  
    printEnd = "\r"):
    """
    Call in a loop to create terminal progress bar
    @params:
        iteration   - Required  : current iteration (Int)
        total       - Required  : total iterations (Int)
        prefix      - Optional  : prefix string (Str)
        suffix      - Optional  : suffix string (Str)
        decimals    - Optional  : positive number of decimals in percent complete (Int)
        length      - Optional  : character length of bar (Int)
        fill        - Optional  : bar fill character (Str)
        printEnd    - Optional  : end character (e.g. "\r", "\r\n") (Str)
    """
    percent = ("{0:." + str(decimals) + "f}").format(100 * (iteration / float(total)))
    filledLength = int(length * iteration // total)
    bar = fill * filledLength + '-' * (length - filledLength)
    print(f'\r{prefix} |{bar}| {percent}% {suffix}', end = printEnd)
    # Print New Line on Complete
    if iteration == total: 
        print()