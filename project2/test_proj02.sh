#!/bin/bash
set -e

PROG=./proj02

echo "=== proj02 unit tests ==="

# Clean slate
rm -f infile.txt out*.txt

# Create test input
printf "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n" > infile.txt

# 1. Basic copy
$PROG infile.txt out_basic.txt
diff infile.txt out_basic.txt

# 2. Small buffer
$PROG -b 1 infile.txt out_b1.txt
diff infile.txt out_b1.txt

# 3. Large buffer
$PROG -b 1024 infile.txt out_b1024.txt
diff infile.txt out_b1024.txt

# 4. Append
echo "START" > out_append.txt
$PROG -a infile.txt out_append.txt
grep -q "START" out_append.txt
grep -q "ABCDEFGHIJKLMNOPQRSTUVWXYZ" out_append.txt

# 5. Truncate
echo "OLD DATA" > out_trunc.txt
$PROG -t infile.txt out_trunc.txt
diff infile.txt out_trunc.txt

# 6. Option order independence
$PROG infile.txt -b 8 -a out_order.txt
diff infile.txt out_order.txt

# 7. Both -a and -t (last wins)
echo "OLD" > out_conflict.txt
$PROG infile.txt -a -t out_conflict.txt
diff infile.txt out_conflict.txt

# 8. Destination auto-create
rm -f out_create.txt
$PROG infile.txt out_create.txt
diff infile.txt out_create.txt

# 9. Invalid buffer size
if $PROG -b abc infile.txt out_fail.txt 2>/dev/null; then
    echo "FAILED: invalid buffer accepted"
    exit 1
fi

# 10. Missing buffer argument
if $PROG -b infile.txt out_fail.txt 2>/dev/null; then
    echo "FAILED: missing buffer size accepted"
    exit 1
fi

# 11. Missing source file
if $PROG nosuchfile.txt out_fail.txt 2>/dev/null; then
    echo "FAILED: missing source accepted"
    exit 1
fi

echo "=== ALL TESTS PASSED ==="
