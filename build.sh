#!/bin/bash

gcc server.c -o server -lSDL2 -lSDL2_image -lgif -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib


gcc client.c -o client -lSDL2 -lSDL2_image -lgif -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib



echo "Сборка завершена!"
