FILE(REMOVE_RECURSE
  "CMakeFiles/test.dir/src/main.c.o"
  "bin/linux/test.pdb"
  "bin/linux/test"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang C)
  INCLUDE(CMakeFiles/test.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
