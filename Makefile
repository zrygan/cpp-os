.PHONY: b c r br

b:
	cmake -S . -B build
	cmake --build build

c:
	rm -rf build

r:
	./build/prosched

br:
	cmake -S . -B build
	cmake --build build
	./build/prosched