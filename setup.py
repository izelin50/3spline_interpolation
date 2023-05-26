from distutils.core import setup, Extension

spline = Extension(
	'spline', 
	 sources = ['spline.c'] 
)

setup(
	name = 'spline',
	version = '0.1',
	description = 'Spline interpolation,
	ext_modules= [spline])


