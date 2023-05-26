import spline
import sortednp
import matplotlib.pyplot as plt
import numpy as np


def spline_show(x_st, y_st, x_test):
	y_test = np.array(spline.interpolate(x_st, y_st, x_test))
	x_st = np.array(x_st)
	x_test = np.array(x_test)
	y_st = np.array(y_st)
	plt.scatter(x_st, y_st, s=40, c='r')
	x = np.concatenate((x_st, x_test), axis=None)
	y = np.concatenate((y_st, y_test), axis=None)
	plt.plot(x_test, y_test, 'g--')
	plt.scatter(x_test, y_test, s=20, c='b')
	plt.show()

spline_show(np.linspace(0, 10, 19), np.sin(np.linspace(0, 10, 19)), np.linspace(0, 10, 40))
