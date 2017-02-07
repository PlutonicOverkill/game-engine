@echo off
rem this uses glad to generate debug opengl headers

rem this should probably be part of cmakelists.txt
rem but I can't be bothered doing that right now

rem change to working directory
CD /D %~dp0

CD src/lib/glad

python -m glad --profile=core --out-path=./ --generator=c-debug --extensions=../../../GL_EXTENSIONS.txt --omit-khrplatform

CD ../../../
