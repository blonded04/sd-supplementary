CXX ?= g++
CFLAGS ?= -Iinclude --std=c++17 -Wall -Werror -Wextra -pedantic -pthread
CFLAGS_SANITIZE ?= $(CFLAGS) -fsanitize=thread -g3

all: tests tests_sanitize examples

tests/mcs_test.out:
	$(CXX) $(CFLAGS) tests/test_mcs.cpp -o $@
	
tests/mcs_test_sanitize.out:
	$(CXX) $(CFLAGS_SANITIZE) tests/test_mcs.cpp -o $@

tests/numaamm_test.out:
	$(CXX) $(CFLAGS) tests/test_numaamm.cpp -o $@

tests/numaamm_test_sanitize.out:
	$(CXX) $(CFLAGS_SANITIZE) tests/test_numaamm.cpp -o $@

tests/reentrant_test.out:
	$(CXX) $(CFLAGS) tests/test_reentrant.cpp -o $@

tests/reentrant_test_sanitize.out:
	$(CXX) $(CFLAGS_SANITIZE) tests/test_reentrant.cpp -o $@

tests/rw_test.out:
	$(CXX) $(CFLAGS) tests/test_rw.cpp -o $@

tests/rw_test_sanitize.out:
	$(CXX) $(CFLAGS_SANITIZE) tests/test_rw.cpp -o $@

tests/seq_test.out:
	$(CXX) $(CFLAGS) tests/test_seq.cpp -o $@

tests/seq_test_sanitize.out:
	$(CXX) $(CFLAGS_SANITIZE) tests/test_seq.cpp -o $@

tests/ttas_test.out:
	$(CXX) $(CFLAGS) tests/test_ttas.cpp -o $@

tests/ttas_test_sanitize.out:
	$(CXX) $(CFLAGS_SANITIZE) tests/test_ttas.cpp -o $@

tests: tests/mcs_test.out tests/numaamm_test.out tests/reentrant_test.out tests/rw_test.out tests/seq_test.out tests/ttas_test.out
	./*_test.out

tests_sanitize: tests/mcs_test_sanitize.out tests/numaamm_test_sanitize.out tests/reentrant_test_sanitize.out tests/rw_test_sanitize.out tests/seq_test_sanitize.out tests/ttas_test_sanitize.out
	./*_test_sanitize.out

examples: examples/rw_example.out

clean_tests: rm -f tests/*.out
clean_examples: rm -f examples/*.out
clean: clean_examples clean_tests

.PHONY: clean all
