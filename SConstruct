program = 'jaster'
sources = Glob('src/*.cpp') + Glob('src/math/*.cpp') + Glob('src/geometry/*.cpp')
ccflags = '-g -std=c++0x'

sdlConfig = 'sdl-config --cflags --libs'

env = Environment(CCFLAGS = ccflags)
env.ParseConfig(sdlConfig);

env.Program(target = program, source = sources)
