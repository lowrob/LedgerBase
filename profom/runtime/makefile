normal:
	set -a CFLAGS=""
	make makelist
debug:
	set -a CFLAGS="-g"
	make makelist
profile:
	set -a CFLAGS="-qp"
	make makelist
test:
	set -a CFLAGS="-ql"
	make makelist
makelist:
	make -f makelfom -e 2>err
	make -f makeerfl -e 2>>err
	make -f maketcr  -e 2>>err
clean:
	make -f makelfom clean
	make -f makeerfl clean
	make -f maketcr  clean
veryclean:
	make -f makelfom veryclean
	make -f makeerfl veryclean
	make -f maketcr  veryclean
