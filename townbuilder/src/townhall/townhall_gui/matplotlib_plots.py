import matplotlib.pyplot as plt
import mpld3

def example_plot():
    """
    An example matplotlib plot converted to an interactive mpld3 plot.
    """
    fig, ax = plt.subplots()
    ax.plot([1, 2, 3, 4], [1, 4, 2, 3])
    ax.set_title("Example Matplotlib Plot")
    ax.set_xlabel("X-axis")
    ax.set_ylabel("Y-axis")

    return mpld3.fig_to_html(fig)
