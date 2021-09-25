#!/bin/bash

# dos2unix
find . -type f -name "*" -not -path "./nbproject/private/*" -not -path "./build/*" -not -path "./dist/*" -not -path "./.git/*" -print0 | xargs -0 dos2unix

# strip trailing whitespace
find . -type f -name "*" -not -path "./nbproject/private/*" -not -path "./build/*" -not -path "./dist/*" -not -path "./.git/*" -exec sed -i 's/[ \t]*$//' {} +

# replace tabs with 8 spaces
find . -type f -name "*" -not -path "./nbproject/private/*" -not -path "./build/*" -not -path "./dist/*" -not -path "./.git/*" -exec sed -i 's/\t/        /g' {} +
