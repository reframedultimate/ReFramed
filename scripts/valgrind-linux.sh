#!/bin/sh

echo "Started valgrind..."
valgrind --num-callers=50 \
	--leak-resolution=high \
	--leak-check=full \
	--track-origins=yes \
	--time-stamp=yes \
    --gen-suppressions=all \
	./ReFramed 2>&1 | tee ReFramed.grind

