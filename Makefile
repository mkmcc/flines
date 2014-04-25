.PHONY: clean

all:    dirs integrate scripts plot
	@echo  build finished

dirs:
	mkdir -p bin

integrate: force_look
	cd integrate/src/ ; $(MAKE) all ; cp flines ../../bin

plot: force_look
	cp plot/*.m ./bin/

scripts: force_look
	cp scripts/*.rb ./bin/
	cd scripts/src/ ; $(MAKE) all ; cp join_vtk.x ../../bin

clean:
	(if [ -d bin ]; then /bin/rm -r bin ; fi)
	(cd integrate/src/ ; $(MAKE) clean)
	(cd scripts/src/ ; $(MAKE) clean)

force_look:
	true
