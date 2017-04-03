#!/bin/sh
astyle  --max-code-length=80 --indent=tab ./*.h
astyle  --max-code-length=80 --indent=tab ./*.c


astyle  --max-code-length=80 --indent=tab ./test/*.h
astyle  --max-code-length=80 --indent=tab ./test/*.cpp
astyle  --max-code-length=80 --indent=tab ./test/*.c




