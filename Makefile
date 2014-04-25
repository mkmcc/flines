.PHONY: clean

all:    integrate plot
	@echo  build finished

integrate: force_look
	cd integrate/src/ ; make all ; cp flines ../../

plot: force_look
	cp plot/*.m ./

clean:
	/bin/rm *.m
	/bin/rm flines
	cd integrate/src/ ; make clean

force_look:
	true
