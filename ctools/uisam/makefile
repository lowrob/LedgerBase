normal:
	set CFLAGS=""; export CFLAGS
	make makelist
debug:
	set CFLAGS="-g"; export CFLAGS
	make makelist
profile:
	set CFLAGS="-qp"; export CFLAGS
	make makelist
test:
	set CFLAGS="-ql"; export CFLAGS
	make makelist
makelist:
	make -f makelib  -e 2>err
	make -f makeisco -e 2>>err
	make -f makeisch -e 2>>err
	make -f makeisrc -e 2>>err
clean:
	make -f makelib  clean
	make -f makeisco clean
	make -f makeisch clean
	make -f makeisrc clean
veryclean:
	make -f makelib  veryclean
	make -f makeisco veryclean
	make -f makeisch veryclean
	make -f makeisrc veryclean
