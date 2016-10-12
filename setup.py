from setuptools import setup

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
    packages = [
        'tangram'
    ],
    package_dir={"": "src"},
)
