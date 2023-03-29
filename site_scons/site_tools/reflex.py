def generate(env, **kwargs):
    env.Replace(LEX='reflex')
    env.Replace(LEXCOM='$LEX $LEXFLAGS $_LEX_HEADER $_LEX_TABLES --outfile=$TARGET $SOURCES')

def exists(env):
  return env.WhereIs('reflex') is not None
