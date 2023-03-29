import os
import platforms # site_scons

def wrapup_conscript(self, libs=[], headers=[]):
  return (libs, headers)

options = Variables('custom.py')
options.Add(
  'platform',
  f'Target platform: {",".join(platforms.target_platforms)}.',
  '',
)
options.Add(
  'js',
  'Command for the JavaScript interpreter.',
  'node',
)
options.Add(BoolVariable(
  'debug',
  'Build with debugging information. Will use `-g` if true.',
  True
))

env = Environment(variables=options)
env.AddMethod(wrapup_conscript)

# use user PATH
env.PrependENVPath('PATH', os.getenv('PATH'))

# allow for e.g. asdf to work
env['ENV']['HOME'] = os.getenv('HOME')
env.Tool('reflex')

if env.WhereIs('bison') is None:
  print('Required `bison` not found in PATH.')
  Exit(1)
if env.WhereIs('reflex') is None:
  print('Required `reflex` not found in PATH.')
  Exit(1)

platform = platforms.get_platform(env)
platforms.setup_for(env, platform)

variant_dir = Dir(f'#build/{platform}')

headers_dir = f'{variant_dir.abspath}/include'
private_libs_dir = f'{variant_dir.abspath}/privlib'
libs_dir = f'{variant_dir.abspath}/lib'

env.Append(LIBPATH=private_libs_dir)

print('Building for platform', platform, 'at', variant_dir.relpath)
print('Headers will be left at', headers_dir)
print('Libraries will be left at', libs_dir)

env['BUILDERS']['TokiwenStaticLibJoin'] = platforms.static_lib_join_builder(env, platform)
env['BUILDERS']['TokiwenSharedLib'] = platforms.shared_lib_builder(env, platform)

include = [
  variant_dir,
  '#thirdparty/RE-flex/include',
]

env.Append(CPPPATH=include)
env.Prepend(CXXFLAGS=['-std=gnu++2b', '-fPIC'])

if env['debug']:
  env.Prepend(CXXFLAGS=['-g'])

env.Tool("compilation_db")
env.CompilationDatabase()

Export('env', 'headers_dir', 'private_libs_dir', 'libs_dir')

libs = []
headers = []

thirdparty_variant_dir = f'{variant_dir}/thirdparty'

thirdparty_libs, thirdparty_headers = \
  SConscript('thirdparty/SConscript', variant_dir=thirdparty_variant_dir)

libs.extend(thirdparty_libs)
headers.extend(thirdparty_headers)

conscript_dirs = ['common', 'parser', 'synthesis']

for conscript_dir in conscript_dirs:
  conscript_file = f'{conscript_dir}/SConscript'
  variant_sub_dir = f'{variant_dir}/{conscript_dir}'

  new_libs, new_headers = SConscript(conscript_file, variant_dir=variant_sub_dir)
  libs.extend(new_libs)
  headers.extend(new_headers)

tokiwen_lib = env.TokiwenStaticLibJoin(
  f'{private_libs_dir}/tokiwen',
  libs,
)

tokiwen_shlib = env.TokiwenSharedLib(
  f'{private_libs_dir}/tokiwen',
  libs,
)

env.Install(headers_dir, headers)
env.Install(private_libs_dir, libs)
env.Install(libs_dir, tokiwen_shlib + tokiwen_lib)

tests_variant_dir = f'{variant_dir}/tests'
SConscript('tests/SConscript', variant_dir=tests_variant_dir)

print()
