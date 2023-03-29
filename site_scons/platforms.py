import os
import sys
from SCons.Script import Builder


target_platforms = ['linux', 'web']


def get_platform(env):
  # check for command-line argument
  if env['platform'] in target_platforms:
    return env['platform']

  # running through `emscons`, assume web
  if os.environ.get('EMSCRIPTEN_TOOL_PATH'):
    return 'web'

  # the caller's platform
  if sys.platform.startswith('linux'):
    return 'linux'

  # default
  return 'web'


def setup_linux(env):
  env['platform'] = 'linux'
  env.Replace(LINK='$CXX')
  # env.ParseConfig('pkg-config --cflags --libs icu-uc')


def setup_web(env):
  env['platform'] = 'web'

  # assume we are running through `emscons`
  env.Append(toolpath=[os.environ.get('EMSCRIPTEN_TOOL_PATH')])

  if 'EMSCRIPTEN_ROOT' in os.environ:
    # add path mainly to emscripten/bind.h
    root = os.environ['EMSCRIPTEN_ROOT']
    include = os.path.join(root, 'system', 'include')
    env.Append(CPPPATH=[include])

  env.Append(CXXFLAGS=['-fexceptions'])
  env.Append(LINKFLAGS=['-lembind', '-fexceptions', '-sWASM_BIGINT=1', '-sENVIRONMENT=web'])
  # env.Append(LINKFLAGS=['-sUSE_ICU=1'])

  env.Tool('emscripten')
  env['LIBSUFFIX'] = '.a'
  env['SHLIBSUFFIX'] = '.mjs'


def setup_for(env, platform=None):
  if platform is None:
    platform = get_platform(env)

  setup_dict = {
    'linux': setup_linux,
    'web': setup_web
  }

  setup_dict[platform](env)


# Takes a bunch of static libraries and compiles them to one
def static_lib_join_builder(env, platform=None):
  if platform is None:
    platform = get_platform(env)

  if platform == 'linux':
    action = '$AR cq --thin $TARGET $SOURCES'
    pass
  elif platform == 'web':
    action = '$AR cqL $TARGET $SOURCES'
    pass
  else:
    raise RuntimeError(f'Unsupported platform `{platform}`.')

  return Builder(
    action=action,
    prefix='$LIBPREFIX',
    suffix='$LIBSUFFIX',
  )


def shared_lib_web_emitter(target, source, env):
  first_target = target[0] # assume it's the JS target
  ext_stripped = os.path.splitext(first_target.relpath)[0]
  # add the .wasm target
  target.append(f'{ext_stripped}.wasm')
  return target, source


# Takes a bunch of static libraries and makes a shared library with them
def shared_lib_builder(env, platform=None):
  if platform is None:
    platform = get_platform(env)

  if platform == 'linux':
    action = '$LINK $LINKFLAGS -shared -o $TARGET -Wl,--whole-archive $SOURCES -Wl,--no-whole-archive'
    emitter = None
  elif platform == 'web':
    action = '$LINK $LINKFLAGS -o $TARGET -Wl,--whole-archive $SOURCES -Wl,--no-whole-archive'
    emitter = shared_lib_web_emitter
  else:
    raise RuntimeError(f'Unsupported platform `{platform}`.')

  return Builder(
    action=action,
    emitter=emitter,
    prefix='$SHLIBPREFIX',
    suffix='$SHLIBSUFFIX',
  )
