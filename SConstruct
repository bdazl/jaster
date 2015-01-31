program = 'jaster'
sources = Glob('src/*.cpp')
sources = sources + Glob('src/math/*.cpp')
ccflags = '-g -std=c++0x'

sdlConfig = 'sdl-config --cflags --libs'

env = Environment(CCFLAGS = ccflags)
env.ParseConfig(sdlConfig);

env.Program(target = program, source = sources)
