# Author: Jacob Collins
# Sources: 
# * [Plotly Tutorial](https://plotly.com/python/network-graphs/)

from os import makedirs, listdir, getcwd
from networkx import write_gexf, set_node_attributes, shell_layout, draw_networkx
import networkx as nx
import plotly.graph_objects as go
import imageio
import matplotlib as mpl
import numpy as np
import matplotlib.pyplot as plt
import os
from colour import Color

def draw_graph(G, node_list, node_color, pos={}, name='aaaa_temp_graph.png', dir = 'figs'):
    min_size = 25
    node_size = [(1 + nx.degree(G, u)) * min_size for u in node_list]
    min_width = 0.1
    line_widths = [(1 + nx.degree(G, u)/2) * min_width for u in node_list]
    makedirs(dir, exist_ok=True)
    draw_networkx(
        G,
        pos=pos,
        nodelist=node_list,
        node_size=node_size,
        width=0.075,
        node_color=node_color,
        with_labels=False,
        linewidths=line_widths,
        edgecolors="black")
    plt.savefig(f'{dir}/{name}', bbox_inches='tight')
    plt.clf()

# Save the graph in gexf format
def save_gexf(G, filename, dirname):
    makedirs(f'graphs/{dirname}', exist_ok=True)
    write_gexf(G, f"graphs/{dirname}/{filename}.gexf")


def sort_by_prefix(filenames, prefix_list):
    filenames_out = []
    for prefix in prefix_list:
        for file in filenames:
            if file.startswith(prefix):
                filenames_out.append(file)
    return filenames_out

    
def save_gif(filename_skeleton, dirname='temp_gif'):
    print(f'Save Gif')
    makedirs(f'{dirname}', exist_ok=True)
    # pathname = f'{getcwd()}/graphs/{dirname}/'
    pathname = f'{dirname}/'
    print(f'Dir contents:\n{listdir(pathname)}')
    filenames = [file for file in listdir(pathname) if file.endswith('.png')]
    print(f"Found filenames:\n{filenames}")
    filtered_files = [
        f'{pathname}/{file}' 
        for file in sort_by_prefix(filenames, [f'{i}_' for i in range(len(filenames))])]
    print(f"Filtered files:\n{filtered_files}")
    images = [imageio.imread(filename) for filename in filtered_files]
    imageio.mimsave(f'{pathname}/{filename_skeleton}.gif', images)

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
    
    
def compareScatter(
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
    fg_color = 'white'
    fg_color2 = 'grey'
    bg_color='black'
    if darktheme:
        fg_color = 'black'
        fg_color2 = 'grey'
        bg_color='white'
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
            ax.fill_between(
                x_list,
                y_list,
                color=get_color(i, darktheme),
                alpha=alpha,
                edgecolor=get_color(i, darktheme),
                linewidth=0.5,
            )
            ax.scatter(
                x_list, y_list,
                color=get_color(i, darktheme),
                s=size,
                alpha=alpha + ((1-alpha)/2),
            )
    else:
        for i, label in enumerate(y_labels):
            ax.fill_between(
                x_list,
                y_lists[i],
                color=get_color(i, darktheme),
                alpha=alpha,
                edgecolor=get_color(i, darktheme),
                linewidth=0.5,
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
            plt.title(f"{subtitle}", color=fg_color2, size=12)
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

def manyLines(
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
    ax.patch.set_facecolor(bg_color)
    ax.tick_params(color=fg_color, labelcolor=fg_color)
    for spine in ax.spines.values():
        spine.set_edgecolor(fg_color)
    fig.patch.set_facecolor(bg_color)
    ax.grid(
        visible=True,
        alpha=0.2,
        color=fg_color2,
        linewidth=0.5,
    )
    
    if y_labels == None:
        for i, y_list in enumerate(y_lists):
            ax.plot(
                x_list,
                y_list,
                color="black",
                alpha=alpha,
                # edgecolor="black",
                linewidth=1,
            )
    else:
        for i, label in enumerate(y_labels):
            ax.fill_between(
                x_list,
                y_lists[i],
                color="black",
                alpha=alpha,
                # edgecolor="black",
                linewidth=1,
                label=label,
            )
        ax.legend(loc=legend_pos)
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
