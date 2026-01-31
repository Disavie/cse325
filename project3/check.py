import sys
from pathlib import Path

if len(sys.argv) != 2:
    print("Usage: python3 check_log.py N=<number>[,with-debug]")
    sys.exit(1)

arg = sys.argv[1]

BASE = Path("P3/Testcases/Expected Outputs")
actual = Path("src/LOG.txt")

if not actual.exists():
    print("src/LOG.txt not found")
    sys.exit(1)

expected = BASE / f"{arg},LOG.txt"

if not expected.exists():
    if not expected.exists():
        print(f"Expected file not found: {expected}")
        sys.exit(1)

expected_text = expected.read_text().strip()
actual_text = actual.read_text().strip()

if expected_text == actual_text:
    print(f"✅ LOG.txt matches {expected.name}")
else:
    print(f"❌ LOG.txt does NOT match {expected.name}")
