from distutils.core import setup, Extension

spline = Extension(
	'spline', # название модуля внутри Python
	 sources = ['spline.c'] # Исходные файлы модуля
)

setup(
	name = 'spline',
	version = '0.1',
	description = 'Approximation using splines',
	ext_modules= [spline])


