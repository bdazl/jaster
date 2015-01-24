program = 'jaster'
sourceFiles = Glob('src/*.cpp')
ccflags = '-std=c++0x'

sdlConfig = 'sdl-config --cflags --libs'

env = Environment(CCFLAGS = ccflags)
env.ParseConfig(sdlConfig);

env.Program(target = program, source = sourceFiles)