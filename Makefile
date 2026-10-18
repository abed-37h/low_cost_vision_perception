.PHONY: build upload

build:
	./scripts/build.sh $(filter-out $@,$(MAKECMDGOALS))

upload:
	./scripts/upload.sh $(filter-out $@,$(MAKECMDGOALS))

%:
	@: