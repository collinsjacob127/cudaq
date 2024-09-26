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
import time as time
import matplotlib.pyplot as plt
from os import makedirs, listdir, getcwd

def get_color(i, darktheme=False):
    # red = Color("red")
    # color_options = list(red.range_to(Color("")))
    # Generate color gradient with the colour library
    color_options = [
        "#000000", # Black
        "#92140C", # Red
        "#390099", # Purple
        "#F5853F", # Orange
        "#2E86AB", # Blue
    ]
    if darktheme:
        color_options = [
            "#FFFFFF", # White
            "#A4243B", # Red
            "#2B9720", # Green
            "#D8973C", # Orange
            "#45A3D9", # Blue
        ]
    return color_options[i % len(color_options)]

def compareLines(
    x_list,
    y_lists,
    y_labels=None,
    title=None,
    subtitle=None,
    xlabel=None,
    ylabel=None,
    logx=False,
    logy=False,
    xrange=None, #Tuple
    yrange=None, #Tuple
    legend_pos=0,
    size=10,
    darktheme=False,
    name="temp_filename",
    dirname="figs",
):
    x_len = len(x_list)
    for y_list in y_lists:
        if len(y_list) != x_len:
            print(
                f"Array size mismatch in {title}"
                + f"\ny len {len(y_list)} != {x_len}"
            )

    alpha = 0.8
    if len(y_lists) > 1:
        alpha = 0.3
    fig, ax = plt.subplots(1, 1, figsize=(10, 5), dpi=300)
    # Set up Axis Definition
    min_x = min(x_list)
    max_x = max(x_list)
    min_y = min([min(y_list) for y_list in y_lists])
    max_y = max([max(y_list) for y_list in y_lists]) 
    max_y = max_y + (max_y - min_y)*0.05 # 5% vertical buffer
    if (xrange):
        min_x = xrange[0]
        max_x = xrange[1]
    if (yrange):
        min_y = yrange[0]
        max_y = yrange[1]
    ax.axis([
        min_x,
        max_x,
        min_y,
        max_y])
    fg_color = 'black'
    fg_color2 = '#1a1a1a'
    bg_color='white'
    if darktheme:
        fg_color = 'white'
        fg_color2 = '#D9D9D9'
        bg_color='black'
    ax.patch.set_facecolor(bg_color)
    ax.tick_params(color=fg_color, labelcolor=fg_color)
    for spine in ax.spines.values():
        spine.set_edgecolor(fg_color)
    fig.patch.set_facecolor(bg_color)
    ax.grid(
        visible=True,
        alpha=0.7,
        color=fg_color,
        linewidth=0.5,
    )
    
    if y_labels == None:
        for i, y_list in enumerate(y_lists):
            ax.plot(
                x_list,
                y_list,
                color=get_color(i, darktheme),
                alpha=alpha,
                # edgecolor="black",
                linewidth=2,
            )
            ax.scatter(
                x_list, y_list,
                color=get_color(i, darktheme),
                s=size,
                alpha=alpha + ((1-alpha)/2),
            )
    else:
        for i, label in enumerate(y_labels):
            ax.plot(
                x_list,
                y_lists[i],
                color=get_color(i, darktheme),
                alpha=alpha,
                # edgecolor="black",
                linewidth=2,
                label=label,
            )
            ax.scatter(
                x_list, y_lists[i],
                color=get_color(i, darktheme),
                alpha=alpha + ((1-alpha)/2),
                s=size,
            )
        ax.legend(loc=legend_pos, facecolor=bg_color, framealpha=0.5)
    # if len(y_lists) == 1 and label_vals:
    #     for i, x in enumerate(x_list):
    #         text_str = f'{np.round(y_lists[0][i], 2).__float__()}'
    #         flip = 1 # 1 => left side, 0 => right side
    #         if i == 0:
    #             flip = 0 # First label should be right of point
    #         elif i < len(x_list)-1 and i > 0:
    #             if y_list[i] < y_list[i-1]: # Was decreasing, don't put left
    #                 flip = 0
    #         if flip:
    #             x_offset = -1*(len(text_str)+0.5)*size*2
    #         ax.annotate(
    #             text_str,
    #             (x, y_lists[0][i]),
    #             # xytext=(-len(text_str)*size,-0.5*size),
    #             xytext=(x_offset,0),
    #             textcoords="offset pixels",
    #             color=fg_color,
    #             fontsize=8)
    if title:
        if subtitle:
            mid = (fig.subplotpars.right + fig.subplotpars.left)/2
            plt.title(f"{subtitle}", color=fg_color2, size=14)
            plt.suptitle(f"{title}", color=fg_color, size=18, x=mid)
        else:
            plt.title(f"{title}", color=fg_color, size=18)
    if xlabel:
        if logx:
            plt.xscale('log')
            plt.xlabel(f'Log {xlabel}', color=fg_color)
        else:
            plt.xlabel(xlabel, color=fg_color)
    if ylabel:
        if logy:
            plt.yscale('log')
            # ax.yaxis.set_major_formatter(ticker.FuncFormatter(myLogFormat))
            plt.ylabel(f'Log {ylabel}', color=fg_color)
        else:
            plt.ylabel(ylabel, color=fg_color)
    svgdirname = dirname + "-svg" 
    try:
        makedirs(dirname, exist_ok=True)
        makedirs(svgdirname, exist_ok=True)
    except FileExistsError:
        pass
    plt.savefig(f"{dirname}/{name}.png")
    plt.savefig(f"{svgdirname}/{name}.svg", format='svg')
    plt.clf()
    plt.close()

# Function to start the timer (in nanoseconds)
def start_timer(title=None):
    if title is not None:
        print(f'{title} starting')
    start_time = time.time_ns()  # Get the current time in nanoseconds
    return start_time

def separate_ns(ns):
    total_seconds = ns // 1_000_000_000  # Get the full seconds
    remainder_ns = ns % 1_000_000_000  # Get the remaining nanoseconds after seconds
    total_milliseconds = remainder_ns // 1_000_000  # Get the full milliseconds from the remainder
    total_nanoseconds = remainder_ns % 1_000_000  # Get the remaining nanoseconds
    return f'{total_seconds} s, {total_milliseconds} ms, {total_nanoseconds} ns'

# Function to end the timer (in nanoseconds)
def end_timer(start_time, title=None):
    end_time = time.time_ns()  # Get the current time in nanoseconds
    time_diff = end_time - start_time  # Calculate the total time difference in nanoseconds

    # Print the result in components
    if title is not None:
        # Convert to seconds, milliseconds, and nanoseconds
        print(f'{title} finished in {separate_ns(time_diff)}')
    return time_diff

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