PYTHON=python

yalibnkf_amalgamated.c: tools/amalgamator.py yalibnkf.c yalibnkf.h nkf/utf8tbl.c nkf/utf8tbl.h nkf/config.h nkf/nkf.c nkf/nkf.h
	$(PYTHON) tools/amalgamator.py --input-encoding=iso2022_jp --input-encoding=sjis --comment --blacklist=nkf/nkf32dll.c --blacklist=nkf/nkf32.h --out=yalibnkf_amalgamated.c yalibnkf.c
