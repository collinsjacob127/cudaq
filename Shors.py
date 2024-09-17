from math import gcd, log2, ceil
import numpy as np
import random
import cudaq
from cudaq import *
import fractions
import matplotlib.pyplot as plt
import contfrac

import time # User for timer

# Function to start the timer (in nanoseconds)
def start_timer(title):
    start_time = time.time_ns()  # Get the current time in nanoseconds
    print(f'{title} starting')
    return start_time

# Function to end the timer (in nanoseconds)
def end_timer(title, start_time):
    end_time = time.time_ns()  # Get the current time in nanoseconds
    time_diff = end_time - start_time  # Calculate the total time difference in nanoseconds

    # Convert to seconds, milliseconds, and nanoseconds
    total_seconds = time_diff // 1_000_000_000  # Get the full seconds
    remainder_ns = time_diff % 1_000_000_000  # Get the remaining nanoseconds after seconds
    total_milliseconds = remainder_ns // 1_000_000  # Get the full milliseconds from the remainder
    total_nanoseconds = remainder_ns % 1_000_000  # Get the remaining nanoseconds

    # Print the result in components
    print(f'{title} finished in {total_seconds} s, {total_milliseconds} ms, {total_nanoseconds} ns')
