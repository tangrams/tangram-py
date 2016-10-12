from setuptools import setup, find_packages

setup(
    name='tangram',
    version=0.1,
    description='A Python wrapper for the 2D/3D map engine Tangram-ES',
    long_description=open('README.md').read(),
    url='https://github.com/tangrams/tangram-py',
    author='Mapzen',
    maintainer="Patricio Gonzalez Vivo",
    maintainer_email="patricio@mapzen.com",
    license='MIT',
    packages=find_packages(exclude=('scripts')),
    package_dir={'tangram': 'tangram'},
    package_data={'tangram': ['tangram-py']},
)
