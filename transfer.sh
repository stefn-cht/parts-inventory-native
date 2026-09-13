#!/bin/bash
rsync -avh --info=progress2 ./build/parts-inventory-1.0.0-Windows.exe
echo "EXE transferred!"
done
