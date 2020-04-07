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
	make -f makerlib -e 2>err
	make -f makecrt  -e 2>>err
	make -f makerpnm -e 2>>err
	make -f makeform -e 2>>err
	make -f makerfom -e 2>>err
	make -f makerfrm -e 2>>err
	make -f makeprnt -e 2>>err
clean:
	make -f makerlib clean
	make -f makecrt  clean
	make -f makerpnm clean
	make -f makeform clean
	make -f makerfom clean
	make -f makerfrm clean
	make -f makeprnt clean
veryclean:
	make -f makerlib veryclean
	make -f makecrt  veryclean
	make -f makerpnm veryclean
	make -f makeform veryclean
	make -f makerfom veryclean
	make -f makerfrm veryclean
	make -f makeprnt veryclean
